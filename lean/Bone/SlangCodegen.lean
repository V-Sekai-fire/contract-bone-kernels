import Bone.SlangCodegen.Sample
import Bone.SlangCodegen.Blend
import Bone.SlangCodegen.RootMotion
import Bone.SlangCodegen.Local
import Bone.SlangCodegen.Jump
import Bone.SlangCodegen.Skin

/-! The bone kernels, in dispatch order: sample every track (once per animation instance),
blend the instances per bone, write local matrices, jump ⌈log₂(depth + 1)⌉ rounds, skin every
bind. -/

namespace Bone.SlangCodegen

def kernels : List (String × LeanSlang.SlangShaderModule) :=
  [ ("bone_sample", Sample.shader)
  , ("bone_blend",  Blend.shader)
  , ("root_motion", RootMotion.shader)
  , ("bone_local",  Local.shader)
  , ("bone_jump",   Jump.shader)
  , ("bone_skin",   Skin.shader) ]

end Bone.SlangCodegen
