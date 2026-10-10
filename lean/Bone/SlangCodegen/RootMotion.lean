import Drape.SlangCodegen.Dsl
import Anny.SlangCodegen.Common
import Bone.SlangCodegen.Curves
import Bone.SlangCodegen.Sample

/-!
# `Bone.SlangCodegen.RootMotion` — the mixer's root motion, one thread

Restates the root-motion arms of the engine mixer's `_blend_process` and the
root-motion lines of `_blend_apply` for the one root-motion track of a mixer.
For each animation instance, in order, with `blend` its weight (divided by the
total when not deterministic; skipped when `|blend| < 1e-5`) and only when the
instance does root calculation (`!seeked || is_external_seeking`):

1. `prev = time − delta`. Playing forward and `prev < start` (approx): loop
   none clamps to `start`, linear wraps by `fposmod`, ping-pong by `pingpong`
   into `[start, end]`. Backward, the same against `end`.
2. For each root channel the instance has, if the playback crossed the loop
   boundary (forward: `prev > time`; backward: `prev < time`), the motion from
   `prev` to the boundary (`end` forward, `start` backward) is added, then
   `prev` moves to the other boundary. Then the motion from `prev` to `time`.
3. The motion from `a` to `b`:
     position `loc += (P(b) − P(a)) · blend`; in local mode
       `loc += R(b)⁻¹ · (P(b) − P(a)) · blend` with `R` the clip's rotation
       track on the same path (identity when that track cannot be sampled);
     rotation `rot = normalize(rot · slerp(1, R(a)⁻¹ · R(b), blend))`
       (`Animation::interpolate_via_rest(rot, R(b), blend, R(a))`);
     scale    `scale += (S(b) − S(a)) · blend`.
4. Output: position, rotation, and `scale − 1`, as `get_root_motion_*`
   returns them. The accumulators are the root target's ordinary blend
   (`bone_blend`).

Times are df32. Samples go through `bk_sample` over a track table holding
only the root tracks.

Bindings (set 0):

  0  ConstantBuffer<RootMotionParams> { uint I; uint deterministic; uint local; }
  1–6  the root track table (`Sample.sampleGlobals`)
  7  StructuredBuffer<float>   instF   (10 · I)  time, delta, start, end, length; each hi, lo
  8  StructuredBuffer<uint>    instU   (6 · I)   loop mode; flags (0 calc root, 1 backward,
                                                 3 any enabled root track); position, rotation,
                                                 scale track (0xFFFFFFFF none); local rotation
                                                 track (0xFFFFFFFF none, 0xFFFFFFFE identity)
  9  StructuredBuffer<float>   weight  (I)
 10  RWStructuredBuffer<float> rootMotion (10)
-/

namespace Bone.SlangCodegen.RootMotion

open LeanSlang
open Drape.SlangCodegen.Dsl
open Anny.SlangCodegen.Common
open Bone.SlangCodegen.Curves

private def NONE : E := u 4294967295
private def IDENT : E := u 4294967294

/-- `Quaternion::xform_inv(v)` for a unit `q`. -/
def xformInv : SlangFunctionDecl :=
  fn f3T "bk_xform_inv" [("q", f4T), ("v", f3T)]
    [ let_ f4T "qi" (call "bk_qinv" [v "q"])
    , retE (mem (call "bk_qmul" [call "bk_qmul" [v "qi", call "float4" [v "v", fl 0.0]], v "q"]) "xyz") ]

/-- `Math::is_equal_approx(a, b)` on df32 times. -/
def dfEq : SlangFunctionDecl :=
  fn bT "bk_df_eq" [("a", f2T), ("b", f2T)]
    [ let_ fT "d" (dfd (v "a") (v "b"))
    , retE (or_ (eq (v "d") (fl 0.0)) (lt (fabs (v "d")) (fmax (eps * fabs (dfv (v "a"))) eps))) ]

/-- `Animation::is_less_approx(a, b)` on df32 times. -/
def dfLess : SlangFunctionDecl :=
  fn bT "bk_df_less" [("a", f2T), ("b", f2T)]
    [ retE (and_ (lt (dfd (v "a") (v "b")) (fl 0.0)) (not_ (call "bk_df_eq" [v "a", v "b"]))) ]

/-- `Animation::is_greater_approx(a, b)` on df32 times. -/
def dfGreater : SlangFunctionDecl :=
  fn bT "bk_df_greater" [("a", f2T), ("b", f2T)]
    [ retE (and_ (gt (dfd (v "a") (v "b")) (fl 0.0)) (not_ (call "bk_df_eq" [v "a", v "b"]))) ]

/-- `start + wrap(prev − start, end − start)`: `fposmod` (linear) or `pingpong` (ping-pong). -/
def wrapPrev : SlangFunctionDecl :=
  fn f2T "bk_wrap_prev" [("prev", f2T), ("st", f2T), ("en", f2T), ("lm", uT)]
    [ let_ fT "x" (dfd (v "prev") (v "st"))
    , let_ fT "L" (dfd (v "en") (v "st"))
    , let_ fT "r" (call "bk_fposmod" [v "x", v "L"])
    , if_ (eq (v "lm") (u 2))
        [ setv "r" (sel (ne (v "L") (fl 0.0))
            (fabs ((((v "x" - v "L") / (v "L" * fl 2.0)) - call "floor" [(v "x" - v "L") / (v "L" * fl 2.0)]) * v "L" * fl 2.0 - v "L"))
            (fl 0.0)) ]
    , retE (call "bk_df_add" [v "st", call "float2" [v "r", fl 0.0]]) ]

/-- The motion of one channel from time `a` to `b`, folded into the cache. -/
private def motion (a b : String) : List St :=
  let smp (tk t : E) : E := call "bk_sample" [tk, t, v "Ldf", v "lm", u 0]
  [ if_ (eq (v "ch") (u 0))
      [ let_ f3T "d" (mem (smp (v "tk") (v b)) "xyz" - mem (smp (v "tk") (v a)) "xyz")
      , if_ (and_ (ne (p "local") (u 0)) (ne (v "lrt") NONE))
          [ let_ f4T "lr" (sel (eq (v "lrt") IDENT) (call "float4" [fl 0.0, fl 0.0, fl 0.0, fl 1.0]) (smp (v "lrt") (v b)))
          , setv "d" (call "bk_xform_inv" [v "lr", v "d"]) ]
      , setv "loc" (v "loc" + v "d" * v "bl") ]
  , if_ (eq (v "ch") (u 1))
      [ let_ f4T "r0" (smp (v "tk") (v a))
      , let_ f4T "r1" (smp (v "tk") (v b))
      , let_ f4T "nr" (call "bk_qmul" [v "rot", call "bk_slerp" [call "float4" [fl 0.0, fl 0.0, fl 0.0, fl 1.0],
          call "bk_qmul" [call "bk_qinv" [v "r0"], v "r1"], v "bl"]])
      , setv "rot" (v "nr" / fmax (call "length" [v "nr"]) (.litFloatExact 1e-20)) ]
  , if_ (eq (v "ch") (u 2))
      [ setv "sc" (v "sc" + (mem (smp (v "tk") (v b)) "xyz" - mem (smp (v "tk") (v a)) "xyz") * v "bl") ] ]

private def f2at (k : Nat) : E := call "float2" [at_ "instF" (v "i" * u 10 + u (2 * k)), at_ "instF" (v "i" * u 10 + u (2 * k + 1))]

def shader : SlangShaderModule :=
  { structs := [ { name := "RootMotionParams", fields := [fld "I" uT, fld "deterministic" uT, fld "local" uT] } ]
  , globals :=
      [ paramsCB "RootMotionParams" ] ++ Sample.sampleGlobals 1 2 3 4 5 6 ++
      [ roF "instF" 7, roU "instU" 8, roF "weight" 9, rwF "rootMotion" 10 ]
  , functions := Sample.sampleFns ++ [ xformInv, dfEq, dfLess, dfGreater, wrapPrev,
      entry 1 [dtid]
          [ if_ (ne (.member (v "tid") "x") (u 0)) [ ret ]
          , let_ bT "det" (ne (p "deterministic") (u 0))
          , let_ fT "total" (fl 0.0)
          , for_ "i" (u 0) (p "I")
              [ if_ (ne (.bin "&" (at_ "instU" (v "i" * u 6 + u 1)) (u 8)) (u 0)) [ setv "total" (v "total" + at_ "weight" (v "i")) ] ]
          , let_ f3T "loc" (call "float3" [fl 0.0, fl 0.0, fl 0.0])
          , let_ f4T "rot" (call "float4" [fl 0.0, fl 0.0, fl 0.0, fl 1.0])
          , let_ f3T "sc" (call "float3" [fl 1.0, fl 1.0, fl 1.0])
          , for_ "i" (u 0) (p "I")
              [ let_ uT "fl" (at_ "instU" (v "i" * u 6 + u 1))
              , let_ fT "bl" (at_ "weight" (v "i"))
              , if_ (not_ (v "det")) [ setv "bl" (sel (lt (fabs (v "total")) eps) (fl 0.0) (v "bl" / v "total")) ]
              , if_ (and_ (and_ (ne (.bin "&" (v "fl") (u 8)) (u 0)) (ne (.bin "&" (v "fl") (u 1)) (u 0))) (ge (fabs (v "bl")) eps))
                  [ let_ uT "lm" (at_ "instU" (v "i" * u 6))
                  , let_ bT "back" (ne (.bin "&" (v "fl") (u 2)) (u 0))
                  , let_ f2T "tt" (f2at 0), let_ f2T "dl" (f2at 1), let_ f2T "st" (f2at 2), let_ f2T "en" (f2at 3), let_ f2T "Ldf" (f2at 4)
                  , let_ uT "lrt" (at_ "instU" (v "i" * u 6 + u 5))
                  , let_ f2T "prev0" (call "bk_df_sub" [v "tt", v "dl"])
                  , if_ (not_ (v "back"))
                      [ if_ (call "bk_df_less" [v "prev0", v "st"])
                          [ setv "prev0" (sel (eq (v "lm") (u 0)) (v "st") (call "bk_wrap_prev" [v "prev0", v "st", v "en", v "lm"])) ] ]
                      [ if_ (call "bk_df_greater" [v "prev0", v "en"])
                          [ setv "prev0" (sel (eq (v "lm") (u 0)) (v "en") (call "bk_wrap_prev" [v "prev0", v "st", v "en", v "lm"])) ] ]
                  , for_ "ch" (u 0) (u 3)
                      [ let_ uT "tk" (at_ "instU" (v "i" * u 6 + u 2 + v "ch"))
                      , if_ (ne (v "tk") NONE)
                          ([ let_ f2T "pv" (v "prev0") ] ++
                           [ if_ (and_ (not_ (v "back")) (call "bk_df_greater" [v "pv", v "tt"]))
                               (motion "pv" "en" ++ [ setv "pv" (v "st") ])
                           , if_ (and_ (v "back") (call "bk_df_less" [v "pv", v "tt"]))
                               (motion "pv" "st" ++ [ setv "pv" (v "en") ]) ] ++
                           motion "pv" "tt") ] ] ]
          , setAt "rootMotion" (u 0) (mem (v "loc") "x"), setAt "rootMotion" (u 1) (mem (v "loc") "y"), setAt "rootMotion" (u 2) (mem (v "loc") "z")
          , setAt "rootMotion" (u 3) (mem (v "rot") "x"), setAt "rootMotion" (u 4) (mem (v "rot") "y")
          , setAt "rootMotion" (u 5) (mem (v "rot") "z"), setAt "rootMotion" (u 6) (mem (v "rot") "w")
          , setAt "rootMotion" (u 7) (mem (v "sc") "x" - fl 1.0), setAt "rootMotion" (u 8) (mem (v "sc") "y" - fl 1.0)
          , setAt "rootMotion" (u 9) (mem (v "sc") "z" - fl 1.0) ] ] }

end Bone.SlangCodegen.RootMotion
