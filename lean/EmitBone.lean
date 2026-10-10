import LeanSlang
import Bone.SlangCodegen

/-!
# `emit_bone` — writes every bone kernel's Slang to `<outDir>/<name>.slang`

    lake exe emit_bone /path/to/output/dir
-/

open LeanSlang

def main (args : List String) : IO Unit := do
  let dir := args.headD "."
  IO.FS.createDirAll dir
  for (name, sh) in Bone.SlangCodegen.kernels do
    IO.FS.writeFile s!"{dir}/{name}.slang" (LeanSlang.emit sh ++ "\n")
    IO.println s!"{name}.slang"
