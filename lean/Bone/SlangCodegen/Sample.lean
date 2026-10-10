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
6 makima-angle); 4 the track's `loop_wrap`.

Bindings (set 0):

  0  ConstantBuffer<BoneSampleParams> { uint T; float time; float length; uint loopMode; uint backward; }
     loopMode 0 none, 1 linear, 2 ping-pong
  1  StructuredBuffer<uint>    trackKeys  (2T)  first key, len
  2  StructuredBuffer<uint>    trackDst   (T)
  3  StructuredBuffer<uint>    trackKind  (T)
  4  StructuredBuffer<float>   keyTime    (K)
  5  StructuredBuffer<float>   keyVal     (4K)
  6  StructuredBuffer<float>   keyTrans   (K)
  7  RWStructuredBuffer<float> localPose  (10 per bone)
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

/-- `_fetch_key`: the unwrapped time for position `p`. -/
def fetchTime : SlangFunctionDecl :=
  fn fT "bk_fetch_time" [("p", iT), ("len", iT), ("wrap", bT), ("mode", uT), ("off", uT), ("L", fT)]
    [ if_ (not_ (v "wrap")) [ retE (at_ "keyTime" (v "off" + toU (call "clamp" [v "p", fi 0, v "len" - fi 1]))) ]
    , let_ iT "idx" (.bin "%" (v "p") (v "len"))
    , if_ (lt (v "idx") (fi 0)) [ setv "idx" (v "idx" + v "len") ]
    , let_ iT "cyc" ((v "p" - v "idx") / v "len")
    , if_ (or_ (eq (v "mode") (u 1)) (eq (.bin "%" (v "cyc") (fi 2)) (fi 0)))
        [ retE (toF (v "cyc") * v "L" + at_ "keyTime" (v "off" + toU (v "idx"))) ]
    , retE (toF (v "cyc" + fi 1) * v "L" - at_ "keyTime" (v "off" + toU (v "len" - fi 1 - v "idx"))) ]

/-- Neighbour slots: base − 2 … base + 3 (names m2 m1 n0 n1 n2 n3). -/
private def slots : List (String × Int) := [("m2", -2), ("m1", -1), ("n0", 0), ("n1", 1), ("n2", 2), ("n3", 3)]

private def kv (n : String) : E :=
  call "float4" ((List.range 4).map fun c => at_ "keyVal" ((v "off" + toU (v ("k" ++ n))) * u 4 + u c))

private def writeVec (e : E) : List St :=
  [ let_ f4T "res" e
  , if_ (eq (v "kind") (u 0))
      ((["x", "y", "z"].zipIdx).map fun (c, k) => setAt "localPose" (v "dst" + u k) (mem (v "res") c))
      ((["x", "y", "z", "w"].zipIdx).map fun (c, k) => setAt "localPose" (v "dst" + u k) (mem (v "res") c))
  , ret ]

private def vecCubic : E :=
  call "float4" ((["x", "y", "z"].map fun c =>
    call "bk_cubic" [mem (v "vn0") c, mem (v "vn1") c, mem (v "vm1") c, mem (v "vn2") c, v "w",
      v "tn1" - v "tn0", v "tm1" - v "tn0", v "tn2" - v "tn0"]) ++ [fl 0.0])

private def vecMakima : E :=
  call "float4" ((["x", "y", "z"].map fun c =>
    call "bk_makima" [mem (v "vn0") c, mem (v "vn1") c, mem (v "vm1") c, mem (v "vn2") c, mem (v "vm2") c, mem (v "vn3") c,
      v "w", v "tn1" - v "tn0", v "tm1" - v "tn0", v "tn2" - v "tn0", v "tm2" - v "tn0", v "tn3" - v "tn0"]) ++ [fl 0.0])

def shader : SlangShaderModule :=
  { structs := [ { name := "BoneSampleParams"
                 , fields := [fld "T" uT, fld "time" fT, fld "length" fT, fld "loopMode" uT, fld "backward" uT] } ]
  , globals :=
      [ paramsCB "BoneSampleParams", roU "trackKeys" 1, roU "trackDst" 2, roU "trackKind" 3,
        roF "keyTime" 4, roF "keyVal" 5, roF "keyTrans" 6, rwF "localPose" 7 ]
  , functions := Curves.all ++ [ fetchIdx, fetchTime,
      entry 64 [dtid]
          ([ let_ uT "i" (.member (v "tid") "x")
           , if_ (ge (v "i") (p "T")) [ ret ]
           , let_ uT "off" (at_ "trackKeys" (v "i" * u 2))
           , let_ iT "len" (toI (at_ "trackKeys" (v "i" * u 2 + u 1)))
           , let_ uT "dst" (at_ "trackDst" (v "i"))
           , let_ uT "flags" (at_ "trackKind" (v "i"))
           , let_ uT "kind" (.bin "&" (v "flags") (u 1))
           , let_ uT "interp" (.bin "&" (.bin ">>" (v "flags") (u 1)) (u 7))
           , let_ bT "wrap" (and_ (ne (.bin "&" (.bin ">>" (v "flags") (u 4)) (u 1)) (u 0)) (ne (p "loopMode") (u 0)))
           , let_ bT "back" (and_ (ne (p "backward") (u 0)) (eq (v "interp") (u 0)))
           , let_ fT "t" (p "time")
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
           , let_ iT "base" (sel (v "back") (fmin (v "mid") (v "len")) (fmin (v "mid") (v "len" - fi 1))) ] ++
           -- _fetch_keys: slot j is base + j (base − j backward)
           (slots.map fun (n, j) =>
             let_ iT ("p" ++ n) (sel (v "back") (v "base" - fi j) (v "base" + fi j))) ++
           (slots.map fun (n, _) =>
             let_ iT ("k" ++ n) (call "bk_fetch_idx" [v ("p" ++ n), v "len", v "wrap", p "loopMode"])) ++
           (slots.map fun (n, _) =>
             let_ fT ("t" ++ n) (call "bk_fetch_time" [v ("p" ++ n), v "len", v "wrap", p "loopMode", v "off", p "length"])) ++
           (slots.map fun (n, _) => let_ f4T ("v" ++ n) (kv n)) ++
           [ let_ fT "tr" (at_ "keyTrans" (v "off" + toU (v "kn0")))
           , if_ (eq (v "tr") (fl 0.0)) (writeVec (v "vn0"))
           , let_ fT "dlt" (v "tn1" - v "tn0")
           , let_ fT "w" (sel (lt (fabs (v "dlt")) eps) (fl 0.0) ((v "t" - v "tn0") / v "dlt"))
           , if_ (ne (v "tr") (fl 1.0)) [ setv "w" (call "bk_ease" [v "w", v "tr"]) ]
           , if_ (eq (v "interp") (u 0)) (writeVec (v "vn0"))
           , if_ (or_ (eq (v "interp") (u 1)) (eq (v "interp") (u 3)))
               [ if_ (eq (v "kind") (u 0))
                   (writeVec (v "vn0" + (v "vn1" - v "vn0") * v "w"))
                   (writeVec (call "bk_slerp" [v "vn0", v "vn1", v "w"])) ]
           , if_ (or_ (eq (v "interp") (u 2)) (eq (v "interp") (u 4)))
               [ if_ (eq (v "kind") (u 0))
                   (writeVec vecCubic)
                   (writeVec (call "bk_sq_cubic" [v "vn0", v "vn1", v "vm1", v "vn2", v "w",
                      v "tn1" - v "tn0", v "tm1" - v "tn0", v "tn2" - v "tn0"])) ]
           , if_ (eq (v "kind") (u 0))
               (writeVec vecMakima)
               (writeVec (call "bk_sq_makima" [v "vn0", v "vn1", v "vm1", v "vn2", v "vm2", v "vn3", v "w",
                  v "tn1" - v "tn0", v "tm1" - v "tn0", v "tn2" - v "tn0", v "tm2" - v "tn0", v "tn3" - v "tn0"])) ]) ] }

end Bone.SlangCodegen.Sample
