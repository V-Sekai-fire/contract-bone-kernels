import Lake
open Lake DSL

package Bone where

require LeanSlang from git
  "https://github.com/V-Sekai-fire/contract-lean-slang.git" @ "60532aef8ed70cc669ecab481182d0636c9e1ac3"

require Drape from "../../lbfgsb/lean"

require Anny from "../../anny-kernels/lean"

@[default_target] lean_lib Bone

lean_exe emit_bone where
  root := `EmitBone
