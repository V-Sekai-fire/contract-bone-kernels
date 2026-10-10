import Drape.SlangCodegen.Dsl
import Anny.SlangCodegen.Common

/-!
# `Bone.SlangCodegen.Skin` — skin transforms, one thread per bind

`skin_i = global_{bone_i} · bind_i`, the transform the skinning pass reads
for bind `i`, written in the skeleton buffer's layout (12 floats, row r at
`4r … 4r+3`, translation in column 3).

Bindings (set 0):

  0  ConstantBuffer<BoneSkinParams> { uint B; }
  1  StructuredBuffer<uint>    bindBone    (B)
  2  StructuredBuffer<float>   bindPose    (12 per bind)
  3  StructuredBuffer<float>   globalPose  (12 per bone)
  4  RWStructuredBuffer<float> skin        (12 per bind)
-/

namespace Bone.SlangCodegen.Skin

open LeanSlang
open Drape.SlangCodegen.Dsl
open Anny.SlangCodegen.Common

def shader : SlangShaderModule :=
  { structs := [ { name := "BoneSkinParams", fields := [fld "B" uT] } ]
  , globals := [ paramsCB "BoneSkinParams", roU "bindBone" 1, roF "bindPose" 2, roF "globalPose" 3, rwF "skin" 4 ]
  , functions :=
      [ entry 64 [dtid]
          ([ let_ uT "i" (.member (v "tid") "x")
           , if_ (ge (v "i") (p "B")) [ ret ]
           , let_ uT "gb" (at_ "bindBone" (v "i") * u 12)
           , let_ uT "bb" (v "i" * u 12) ] ++
           loadRot12 "G" "globalPose" (v "gb") ++ loadT12 "g" "globalPose" (v "gb") ++
           loadRot12 "K" "bindPose" (v "bb") ++ loadT12 "k" "bindPose" (v "bb") ++
           matMul "C" "G" "K" ++
           matVec "Gk" "G" "k" ++
           vec "c" (fun i => s "Gk" i + s "g" i) ++
           store12 "skin" (v "bb") "C" "c") ] }

end Bone.SlangCodegen.Skin
