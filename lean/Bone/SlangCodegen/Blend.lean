import Drape.SlangCodegen.Dsl
import Anny.SlangCodegen.Common
import Bone.SlangCodegen.Curves

/-!
# `Bone.SlangCodegen.Blend` — the mixer's per-bone blend, one thread per bone

Restates, for position, rotation and scale tracks on skeleton bones, the
engine mixer's `_blend_init`, `_blend_calc_total_weight`, the bone arms of
`_blend_process`, and `_blend_apply`:

1. Start from the init pose (the host fills `init`). For a skeleton bone it is
   the rest. The same kernel serves the mixer's other blended targets, with
   motion scale 1: a Node3D transform starts from identity, except the one
   channel of the track that creates its cache (the first track for that node
   in the mixer's animation and track order), which takes the RESET
   animation's first key when RESET has that track; a blend shape (a scalar
   in the position slot's `x`) starts from RESET's key, else 0.
2. `total` is the sum over animation instances of the bone's weight, counted
   once per instance that has any enabled track for the bone.
3. Not deterministic and `|total| < 1e-5`: the bone is not written.
4. For each instance in order, `blend` is its weight (instance weight × the
   bone's filter weight; the host multiplies them), divided by `total` when not
   deterministic; `|blend| < 1e-5` skips the instance. Then
     position `loc += (sample · motionScale − initLoc) · blend`,
     rotation `rot = normalize(rot · slerp(1, initRot⁻¹ · sample, blend))`
       (`Animation::interpolate_via_rest`),
     scale    `scale += (sample − initScale) · blend`,
   each only when the instance sampled that channel.
5. Write the channels any animation in the mixer animates for the bone (`used`).

`samp` holds `bone_sample`'s output for each instance, 10 floats per
(instance, bone) in the local-pose layout. `present` bits per (instance, bone):
0 position sampled, 1 rotation sampled, 2 scale sampled, 3 the instance has an
enabled track for the bone (a track with no key in range counts for the total
but is not sampled).

Bindings (set 0):

  0  ConstantBuffer<BoneBlendParams> { uint N; uint I; uint deterministic; }
  1  StructuredBuffer<float>   samp         (10 · I · N)
  2  StructuredBuffer<uint>    present      (I · N)
  3  StructuredBuffer<float>   weight       (I · N)
  4  StructuredBuffer<float>   init         (10 · N)
  5  StructuredBuffer<float>   motionScale  (N)
  6  StructuredBuffer<uint>    used         (N)   bits: 0 position, 1 rotation, 2 scale
  7  RWStructuredBuffer<float> localPose    (10 · N)
-/

namespace Bone.SlangCodegen.Blend

open LeanSlang
open Drape.SlangCodegen.Dsl
open Anny.SlangCodegen.Common
open Bone.SlangCodegen.Curves

private def f3 (buf : String) (base : E) : E :=
  call "float3" [at_ buf base, at_ buf (base + u 1), at_ buf (base + u 2)]

private def f4 (buf : String) (base : E) : E :=
  call "float4" [at_ buf base, at_ buf (base + u 1), at_ buf (base + u 2), at_ buf (base + u 3)]

private def bit (e : E) (k : Nat) : E := ne (.bin "&" e (u (2 ^ k))) (u 0)

def shader : SlangShaderModule :=
  { structs := [ { name := "BoneBlendParams", fields := [fld "N" uT, fld "I" uT, fld "deterministic" uT] } ]
  , globals :=
      [ paramsCB "BoneBlendParams", roF "samp" 1, roU "present" 2, roF "weight" 3, roF "init" 4,
        roF "motionScale" 5, roU "used" 6, rwF "localPose" 7 ]
  , functions := Curves.all ++ [
      entry 64 [dtid]
          [ let_ uT "b" (.member (v "tid") "x")
          , if_ (ge (v "b") (p "N")) [ ret ]
          , let_ bT "det" (ne (p "deterministic") (u 0))
          , let_ fT "total" (fl 0.0)
          , for_ "i" (u 0) (p "I")
              [ if_ (bit (at_ "present" (v "i" * p "N" + v "b")) 3)
                  [ setv "total" (v "total" + at_ "weight" (v "i" * p "N" + v "b")) ] ]
          , if_ (and_ (not_ (v "det")) (lt (fabs (v "total")) eps)) [ ret ]
          , let_ uT "ib" (v "b" * u 10)
          , let_ f3T "iLoc" (f3 "init" (v "ib"))
          , let_ f4T "iRot" (f4 "init" (v "ib" + u 3))
          , let_ f3T "iSc" (f3 "init" (v "ib" + u 7))
          , let_ f4T "iRotInv" (call "bk_qinv" [v "iRot"])
          , let_ f3T "loc" (v "iLoc")
          , let_ f4T "rot" (v "iRot")
          , let_ f3T "sc" (v "iSc")
          , let_ fT "ms" (at_ "motionScale" (v "b"))
          , for_ "i" (u 0) (p "I")
              [ let_ uT "k" (v "i" * p "N" + v "b")
              , let_ uT "m" (at_ "present" (v "k"))
              , if_ (bit (v "m") 3)
                  [ let_ fT "bl" (at_ "weight" (v "k"))
                  , if_ (not_ (v "det")) [ setv "bl" (v "bl" / v "total") ]
                  , if_ (ge (fabs (v "bl")) eps)
                      [ let_ uT "o" (v "k" * u 10)
                      , if_ (bit (v "m") 0)
                          [ setv "loc" (v "loc" + (f3 "samp" (v "o") * v "ms" - v "iLoc") * v "bl") ]
                      , if_ (bit (v "m") 1)
                          [ let_ f4T "rel" (call "bk_qmul" [v "iRotInv", f4 "samp" (v "o" + u 3)])
                          , let_ f4T "r" (call "bk_qmul" [v "rot",
                              call "bk_slerp" [call "float4" [fl 0.0, fl 0.0, fl 0.0, fl 1.0], v "rel", v "bl"]])
                          , setv "rot" (v "r" / fmax (call "length" [v "r"]) (.litFloatExact 1e-20)) ]
                      , if_ (bit (v "m") 2)
                          [ setv "sc" (v "sc" + (f3 "samp" (v "o" + u 7) - v "iSc") * v "bl") ] ] ] ]
          , let_ uT "us" (at_ "used" (v "b"))
          , if_ (bit (v "us") 0)
              ((["x", "y", "z"].zipIdx).map fun (c, k) => setAt "localPose" (v "ib" + u k) (mem (v "loc") c))
          , if_ (bit (v "us") 1)
              ((["x", "y", "z", "w"].zipIdx).map fun (c, k) => setAt "localPose" (v "ib" + u (3 + k)) (mem (v "rot") c))
          , if_ (bit (v "us") 2)
              ((["x", "y", "z"].zipIdx).map fun (c, k) => setAt "localPose" (v "ib" + u (7 + k)) (mem (v "sc") c)) ] ] }

end Bone.SlangCodegen.Blend
