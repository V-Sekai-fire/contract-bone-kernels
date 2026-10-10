import Drape.SlangCodegen.Dsl
import Anny.SlangCodegen.Common
import Bone.SlangCodegen.Compose

/-!
# `Bone.SlangCodegen.Local` — local pose matrices and the first jump links, one thread per bone

Writes each bone's local pose as a 12-float affine (`Compose.localMatrix`:
`R(q) · diag(scale)`, translated by the position) into `acc`, and its parent
into `anc` (`0xFFFFFFFF` for a root). `bone_jump` then folds ancestors in.

Bindings (set 0):

  0  ConstantBuffer<BoneLocalParams> { uint N; }
  1  StructuredBuffer<uint>    parent     (N)
  2  StructuredBuffer<float>   localPose  (10 per bone)
  3  RWStructuredBuffer<float> acc        (12 per bone)
  4  RWStructuredBuffer<uint>  anc        (N)
-/

namespace Bone.SlangCodegen.Local

open LeanSlang
open Drape.SlangCodegen.Dsl
open Anny.SlangCodegen.Common

def shader : SlangShaderModule :=
  { structs := [ { name := "BoneLocalParams", fields := [fld "N" uT] } ]
  , globals := [ paramsCB "BoneLocalParams", roU "parent" 1, roF "localPose" 2, rwF "acc" 3, rwU "anc" 4 ]
  , functions :=
      [ entry 64 [dtid]
          ([ let_ uT "b" (.member (v "tid") "x")
           , if_ (ge (v "b") (p "N")) [ ret ] ] ++
           Compose.localMatrix (v "b") ++
           store12 "acc" (v "b" * u 12) "M" "lp" ++
           [ setAt "anc" (v "b") (at_ "parent" (v "b")) ]) ] }

end Bone.SlangCodegen.Local
