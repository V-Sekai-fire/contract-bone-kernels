import Bone.SlangCodegen.Sample
import Bone.SlangCodegen.Local
import Bone.SlangCodegen.Jump
import Bone.SlangCodegen.Skin

/-! The bone kernels, in dispatch order: sample every track, write local matrices, jump
⌈log₂(depth + 1)⌉ rounds, skin every bind. -/

namespace Bone.SlangCodegen

def kernels : List (String × LeanSlang.SlangShaderModule) :=
  [ ("bone_sample", Sample.shader)
  , ("bone_local",  Local.shader)
  , ("bone_jump",   Jump.shader)
  , ("bone_skin",   Skin.shader) ]

end Bone.SlangCodegen
