import Drape.SlangCodegen.Dsl
import Anny.SlangCodegen.Common
import Bone.SlangCodegen.Curves

/-!
# `Bone.SlangCodegen.Sample` — keyframe track sampling, one thread per track

Each track samples one channel of one bone's local pose at the clip time:
position or scale (kind 0, three floats) or rotation (kind 1, a quaternion
`x y z w`). The steps restate `Animation::_interpolate` on the engine's
reference branch:

1. A track with one key returns it.
2. `_find`: binary search with `is_equal_approx` matching; forward, the key at
   or before the time (−1 before the first); backward, the key at or after it.
3. `_fetch_keys` / `_fetch_key`: the base key and its neighbours (base − 2 …
   base + 3, enough for every mode) as (key index, unwrapped time). Without
   wrapping (track `loop_wrap` off, or loop mode none) the index is clamped to
   the keys; with linear looping it wraps and adds `cycle · length`; with
   ping-pong, odd cycles mirror the index and time.
4. A `from` key with transition 0 holds its value.
5. Weighted time `(t − t₀) / (t₁ − t₀)` (0 when `|t₁ − t₀| < 1e-5`), eased by
   the `from` key's transition when it is not 1 (`Math::ease`).
6. The curve: nearest returns the `from` key (the key after, playing
   backward); linear and linear-angle lerp vectors and `slerp` quaternions;
   cubic and cubic-angle use Barry–Goldman per component and the expmap
   spherical cubic; makima and makima-angle the modified Akima spline per
   component and the expmap spherical makima. For position, rotation and
   scale the angle modes are the plain ones, as in the engine.

`len` is the engine's `_find(keys, length) + 1`: keys past the clip length are
ignored. The host uploads it; a track with no key in range is not uploaded.

Local pose layout, 10 floats per bone: position 0–2, rotation 3–6 (x y z w),
scale 7–9. `trackDst` is the first float the track writes.

`trackKind` bits: 0 kind (0 vector, 1 quaternion); 1–3 interpolation
(0 nearest, 1 linear, 2 cubic, 3 linear-angle, 4 cubic-angle, 5 makima,
6 makima-angle); 4 the track's `loop_wrap`; 5 scalar (a blend-shape track:
one float in `x`). For scalars the angle modes are the engine's angle forms
(`lerp_angle`, `cubic_interpolate_angle_in_time`,
`makima_interpolate_angle_in_time`), each wrapped by `fposmod(·, TAU)`, as the
engine does for float keys. Bit 6 marks a compressed track
(`Animation::_*_interpolate_compressed`): the host feeds it as linear with
wrap off and every key, including keys past the clip end; the kernel clamps
the time to the clip and takes the last key at or before it, and its
`keyTrans` holds, per key, how many keys back the engine's answer at exactly
that key's time lies (non-zero only where quantised key times coincide at
the start of a compression packet).

Bindings (set 0):

  0  ConstantBuffer<BoneSampleParams> { uint T; float time; float length; uint loopMode; uint backward;
                                        float timeLo; float lengthLo; }
     loopMode 0 none, 1 linear, 2 ping-pong
  1  StructuredBuffer<uint>    trackKeys  (2T)  first key, len
  2  StructuredBuffer<uint>    trackDst   (T)
  3  StructuredBuffer<uint>    trackKind  (T)
  4  StructuredBuffer<float>   keyTime    (K)
  5  StructuredBuffer<float>   keyVal     (4K)
  6  StructuredBuffer<float>   keyTrans   (K)
  7  RWStructuredBuffer<float> localPose  (10 per bone)
  8  StructuredBuffer<float>   keyTimeLo  (K)

Times are df32 (`Curves.lean`): a key time is `keyTime + keyTimeLo`, the clip
time `time + timeLo`, the length `length + lengthLo`, as the host splits the
engine's doubles. Every time difference, the weighted time and the curves'
time terms are taken in df32, so keys a millisecond apart do not turn float32
rounding of the times into error.
-/

namespace Bone.SlangCodegen.Sample

open LeanSlang
open Drape.SlangCodegen.Dsl
open Anny.SlangCodegen.Common
open Bone.SlangCodegen.Curves

private def toI (e : E) : E := .cast iT e

/-- `_fetch_key`: the key index for position `p`. -/
def fetchIdx : SlangFunctionDecl :=
  fn iT "bk_fetch_idx" [("p", iT), ("len", iT), ("wrap", bT), ("mode", uT)]
    [ if_ (not_ (v "wrap")) [ retE (call "clamp" [v "p", fi 0, v "len" - fi 1]) ]
    , let_ iT "idx" (.bin "%" (v "p") (v "len"))
    , if_ (lt (v "idx") (fi 0)) [ setv "idx" (v "idx" + v "len") ]
    , let_ iT "cyc" ((v "p" - v "idx") / v "len")
    , if_ (or_ (eq (v "mode") (u 1)) (eq (.bin "%" (v "cyc") (fi 2)) (fi 0))) [ retE (v "idx") ]
    , retE (v "len" - fi 1 - v "idx") ]

/-- `_fetch_key`: the unwrapped time for position `p`, as df32 (`cycle · length` added in df32). -/
def fetchTime : SlangFunctionDecl :=
  let key (i : E) : E := call "float2" [at_ "keyTime" (v "off" + toU i), at_ "keyTimeLo" (v "off" + toU i)]
  fn f2T "bk_fetch_time" [("p", iT), ("len", iT), ("wrap", bT), ("mode", uT), ("off", uT), ("L", f2T)]
    [ if_ (not_ (v "wrap")) [ retE (key (call "clamp" [v "p", fi 0, v "len" - fi 1])) ]
    , let_ iT "idx" (.bin "%" (v "p") (v "len"))
    , if_ (lt (v "idx") (fi 0)) [ setv "idx" (v "idx" + v "len") ]
    , let_ iT "cyc" ((v "p" - v "idx") / v "len")
    , if_ (or_ (eq (v "mode") (u 1)) (eq (.bin "%" (v "cyc") (fi 2)) (fi 0)))
        [ retE (call "bk_df_add" [call "bk_df_mulf" [v "L", toF (v "cyc")], key (v "idx")]) ]
    , retE (call "bk_df_sub" [call "bk_df_mulf" [v "L", toF (v "cyc" + fi 1)], key (v "len" - fi 1 - v "idx")]) ]

/-- Neighbour slots: base − 2 … base + 3 (names m2 m1 n0 n1 n2 n3). -/
private def slots : List (String × Int) := [("m2", -2), ("m1", -1), ("n0", 0), ("n1", 1), ("n2", 2), ("n3", 3)]

private def kv (n : String) : E :=
  call "float4" ((List.range 4).map fun c => at_ "keyVal" ((v "off" + toU (v ("k" ++ n))) * u 4 + u c))

private def writeVec (e : E) : List St := [ retE e ]

private def vecCubic : E :=
  call "float4" ((["x", "y", "z"].map fun c =>
    call "bk_cubic" [mem (v "vn0") c, mem (v "vn1") c, mem (v "vm1") c, mem (v "vn2") c, v "w",
      call "bk_df_sub" [v "tn1", v "tn0"], call "bk_df_sub" [v "tm1", v "tn0"], call "bk_df_sub" [v "tn2", v "tn0"]]) ++ [fl 0.0])

private def vecMakima : E :=
  call "float4" ((["x", "y", "z"].map fun c =>
    call "bk_makima" [mem (v "vn0") c, mem (v "vn1") c, mem (v "vm1") c, mem (v "vn2") c, mem (v "vm2") c, mem (v "vn3") c,
      v "w", call "bk_df_sub" [v "tn1", v "tn0"], call "bk_df_sub" [v "tm1", v "tn0"], call "bk_df_sub" [v "tn2", v "tn0"], call "bk_df_sub" [v "tm2", v "tn0"], call "bk_df_sub" [v "tn3", v "tn0"]]) ++ [fl 0.0])

/-- The engine's track sampler for track `i` at df32 time `tdf` (clip length `Ldf`, loop mode
`loopMode`, `backward` for nearest keys): the value as `float4` (vector in `xyz`, quaternion in
`xyzw`, scalar in `x`). `bone_sample` and `root_motion` both sample through it. -/
def sampleFn : SlangFunctionDecl :=
  fn f4T "bk_sample" [("i", uT), ("tdf", f2T), ("Ldf", f2T), ("loopMode", uT), ("backward", uT)]
          ([ let_ uT "off" (at_ "trackKeys" (v "i" * u 2))
           , let_ iT "len" (toI (at_ "trackKeys" (v "i" * u 2 + u 1)))
           , let_ uT "flags" (at_ "trackKind" (v "i"))
           , let_ uT "kind" (.bin "&" (v "flags") (u 1))
           , let_ uT "interp" (.bin "&" (.bin ">>" (v "flags") (u 1)) (u 7))
           , let_ bT "wrap" (and_ (ne (.bin "&" (.bin ">>" (v "flags") (u 4)) (u 1)) (u 0)) (ne (v "loopMode") (u 0)))
           , let_ bT "back" (and_ (ne (v "backward") (u 0)) (eq (v "interp") (u 0)))
           , let_ fT "t" (mem (v "tdf") "x")
           , let_ iT "k0" (fi 0)
           , let_ f4T "vk0" (kv "0") ] ++
           [ if_ (eq (v "len") (fi 1)) (writeVec (v "vk0")) ] ++
           -- _find
           [ let_ iT "low" (fi 0)
           , let_ iT "high" (v "len" - fi 1)
           , let_ iT "mid" (fi 0)
           , let_ bT "found" (.litBool false)
           , while_ (and_ (le (v "low") (v "high")) (not_ (v "found")))
               [ setv "mid" ((v "low" + v "high") / fi 2)
               , let_ fT "km" (at_ "keyTime" (v "off" + toU (v "mid")))
               , let_ fT "tol" (fmax (eps * fabs (v "t")) eps)
               , if_ (or_ (eq (v "t") (v "km")) (lt (fabs (v "t" - v "km")) (v "tol")))
                   [ setv "found" (.litBool true) ]
                   [ if_ (lt (v "t") (v "km")) [ setv "high" (v "mid" - fi 1) ] [ setv "low" (v "mid" + fi 1) ] ] ]
           , if_ (not_ (v "found"))
               [ if_ (not_ (v "back"))
                   [ if_ (gt (at_ "keyTime" (v "off" + toU (v "mid"))) (v "t")) [ setv "mid" (v "mid" - fi 1) ] ]
                   [ if_ (lt (at_ "keyTime" (v "off" + toU (v "mid"))) (v "t")) [ setv "mid" (v "mid" + fi 1) ] ] ]
           , let_ iT "base" (sel (v "back") (fmin (v "mid") (v "len")) (fmin (v "mid") (v "len" - fi 1)))
           -- A compressed track (bit 6) fetches as `_fetch_compressed`: the time clamped to the
           -- clip, then the last key at or before it, compared exactly (quantised key times can
           -- coincide: just after a shared time the later key wins, at exactly that time the first).
           , if_ (ne (.bin "&" (.bin ">>" (v "flags") (u 6)) (u 1)) (u 0))
               [ setv "tdf" (sel (lt (mem (v "tdf") "x") (fl 0.0)) (call "float2" [fl 0.0, fl 0.0]) (v "tdf"))
               , setv "tdf" (sel (gt (dfd (v "tdf") (v "Ldf")) (fl 0.0)) (v "Ldf") (v "tdf"))
               , let_ iT "clo" (fi 0)
               , let_ iT "chi" (v "len")
               , while_ (lt (v "clo") (v "chi"))
                   [ let_ iT "cm" ((v "clo" + v "chi") / fi 2)
                   , if_ (le (dfd (call "float2" [at_ "keyTime" (v "off" + toU (v "cm")), at_ "keyTimeLo" (v "off" + toU (v "cm"))]) (v "tdf")) (fl 0.0))
                       [ setv "clo" (v "cm" + fi 1) ] [ setv "chi" (v "cm") ] ]
               , setv "base" (v "clo" - fi 1)
               -- at exactly a shared key time the engine returns the first key of the packet that
               -- starts there; `keyTrans` of a compressed key holds how many keys back that is
               , if_ (ge (v "base") (fi 0))
                   [ let_ iT "ps" (v "base" - toI (at_ "keyTrans" (v "off" + toU (v "base"))))
                   , if_ (eq (dfd (call "float2" [at_ "keyTime" (v "off" + toU (v "ps")), at_ "keyTimeLo" (v "off" + toU (v "ps"))]) (v "tdf")) (fl 0.0))
                       [ setv "base" (v "ps") ] ] ] ] ++
           -- _fetch_keys: slot j is base + j (base − j backward)
           (slots.map fun (n, j) =>
             let_ iT ("p" ++ n) (sel (v "back") (v "base" - fi j) (v "base" + fi j))) ++
           (slots.map fun (n, _) =>
             let_ iT ("k" ++ n) (call "bk_fetch_idx" [v ("p" ++ n), v "len", v "wrap", v "loopMode"])) ++
           (slots.map fun (n, _) =>
             let_ f2T ("t" ++ n) (call "bk_fetch_time" [v ("p" ++ n), v "len", v "wrap", v "loopMode", v "off", v "Ldf"])) ++
           (slots.map fun (n, _) => let_ f4T ("v" ++ n) (kv n)) ++
           [ let_ fT "tr" (sel (ne (.bin "&" (.bin ">>" (v "flags") (u 6)) (u 1)) (u 0)) (fl 1.0) (at_ "keyTrans" (v "off" + toU (v "kn0"))))
           , if_ (eq (v "tr") (fl 0.0)) (writeVec (v "vn0"))
           , let_ fT "dlt" (dfd (v "tn1") (v "tn0"))
           , let_ fT "w" (sel (lt (fabs (v "dlt")) eps) (fl 0.0) (dfd (v "tdf") (v "tn0") / v "dlt"))
           , if_ (ne (v "tr") (fl 1.0)) [ setv "w" (call "bk_ease" [v "w", v "tr"]) ]
           , if_ (eq (v "interp") (u 0)) (writeVec (v "vn0"))
           , let_ bT "scalar" (ne (.bin "&" (.bin ">>" (v "flags") (u 5)) (u 1)) (u 0))
           , if_ (and_ (v "scalar") (eq (v "interp") (u 3)))
               (writeVec (call "float4" [call "bk_fposmod" [mem (v "vn0") "x" + call "bk_angle_diff" [mem (v "vn0") "x", mem (v "vn1") "x"] * v "w",
                  .litFloatExact 6.283185307179586], fl 0.0, fl 0.0, fl 0.0]))
           , if_ (and_ (v "scalar") (eq (v "interp") (u 4)))
               (writeVec (call "float4" [call "bk_fposmod" [call "bk_cubic_angle" [mem (v "vn0") "x", mem (v "vn1") "x", mem (v "vm1") "x", mem (v "vn2") "x",
                  v "w", call "bk_df_sub" [v "tn1", v "tn0"], call "bk_df_sub" [v "tm1", v "tn0"], call "bk_df_sub" [v "tn2", v "tn0"]], .litFloatExact 6.283185307179586], fl 0.0, fl 0.0, fl 0.0]))
           , if_ (and_ (v "scalar") (eq (v "interp") (u 6)))
               (writeVec (call "float4" [call "bk_fposmod" [call "bk_makima_angle" [mem (v "vn0") "x", mem (v "vn1") "x", mem (v "vm1") "x", mem (v "vn2") "x",
                  mem (v "vm2") "x", mem (v "vn3") "x", v "w", call "bk_df_sub" [v "tn1", v "tn0"], call "bk_df_sub" [v "tm1", v "tn0"], call "bk_df_sub" [v "tn2", v "tn0"], call "bk_df_sub" [v "tm2", v "tn0"], call "bk_df_sub" [v "tn3", v "tn0"]],
                  .litFloatExact 6.283185307179586], fl 0.0, fl 0.0, fl 0.0]))
           , if_ (or_ (eq (v "interp") (u 1)) (eq (v "interp") (u 3)))
               [ if_ (eq (v "kind") (u 0))
                   (writeVec (v "vn0" + (v "vn1" - v "vn0") * v "w"))
                   (writeVec (call "bk_slerp" [v "vn0", v "vn1", v "w"])) ]
           , if_ (or_ (eq (v "interp") (u 2)) (eq (v "interp") (u 4)))
               [ if_ (eq (v "kind") (u 0))
                   (writeVec vecCubic)
                   (writeVec (call "bk_sq_cubic" [v "vn0", v "vn1", v "vm1", v "vn2", v "w",
                      call "bk_df_sub" [v "tn1", v "tn0"], call "bk_df_sub" [v "tm1", v "tn0"], call "bk_df_sub" [v "tn2", v "tn0"]])) ]
           , if_ (eq (v "kind") (u 0))
               (writeVec vecMakima)
               (writeVec (call "bk_sq_makima" [v "vn0", v "vn1", v "vm1", v "vn2", v "vm2", v "vn3", v "w",
                  call "bk_df_sub" [v "tn1", v "tn0"], call "bk_df_sub" [v "tm1", v "tn0"], call "bk_df_sub" [v "tn2", v "tn0"], call "bk_df_sub" [v "tm2", v "tn0"], call "bk_df_sub" [v "tn3", v "tn0"]])) ])

/-- The helpers `bk_sample` needs, callees first. -/
def sampleFns : List SlangFunctionDecl := Curves.all ++ [ fetchIdx, fetchTime, sampleFn ]

/-- The globals `bk_sample` reads, at the given bindings: track keys, kinds, key times (hi, lo),
values and transitions. -/
def sampleGlobals (keys kind time val trans timeLo : Nat) : List SlangBinding :=
  [ roU "trackKeys" keys, roU "trackKind" kind, roF "keyTime" time, roF "keyVal" val, roF "keyTrans" trans, roF "keyTimeLo" timeLo ]

def shader : SlangShaderModule :=
  { structs := [ { name := "BoneSampleParams"
                 , fields := [fld "T" uT, fld "time" fT, fld "length" fT, fld "loopMode" uT, fld "backward" uT, fld "timeLo" fT, fld "lengthLo" fT] } ]
  , globals :=
      [ paramsCB "BoneSampleParams", roU "trackDst" 2, rwF "localPose" 7 ] ++ sampleGlobals 1 3 4 5 6 8
  , functions := sampleFns ++ [
      entry 64 [dtid]
          [ let_ uT "i" (.member (v "tid") "x")
          , if_ (ge (v "i") (p "T")) [ ret ]
          , let_ uT "dst" (at_ "trackDst" (v "i"))
          , let_ uT "flags" (at_ "trackKind" (v "i"))
          , let_ f4T "res" (call "bk_sample" [v "i", call "float2" [p "time", p "timeLo"], call "float2" [p "length", p "lengthLo"],
              p "loopMode", p "backward"])
          , if_ (eq (.bin "&" (v "flags") (u 1)) (u 0))
              ((["x", "y", "z"].zipIdx).map fun (c, k) => setAt "localPose" (v "dst" + u k) (mem (v "res") c))
              ((["x", "y", "z", "w"].zipIdx).map fun (c, k) => setAt "localPose" (v "dst" + u k) (mem (v "res") c)) ] ] }

end Bone.SlangCodegen.Sample
