import Drape.SlangCodegen.Dsl
import Anny.SlangCodegen.Common

/-!
# `Bone.SlangCodegen.Jump` — one pointer-jumping round over the skeleton, one thread per bone

Each bone holds `acc_b`, the product of the local poses from `anc_b`'s child
down to `b`, and the ancestor `anc_b` that product starts below. One round
replaces it by `acc_{anc_b} · acc_b` and jumps to `anc_{anc_b}`; a bone with
no ancestor left keeps its transform. After ⌈log₂(depth + 1)⌉ rounds every
`acc_b` is bone `b`'s skeleton-space pose, for any tree shape: a chain of N
bones takes ⌈log₂ N⌉ rounds instead of N level dispatches. Reads one buffer
pair and writes the other (the host alternates them), so a round sees no
partial writes.

Bindings (set 0):

  0  ConstantBuffer<BoneJumpParams> { uint N; }
  1  StructuredBuffer<float>   accIn   (12 per bone)
  2  StructuredBuffer<uint>    ancIn   (N)
  3  RWStructuredBuffer<float> accOut  (12 per bone)
  4  RWStructuredBuffer<uint>  ancOut  (N)
-/

namespace Bone.SlangCodegen.Jump

open LeanSlang
open Drape.SlangCodegen.Dsl
open Anny.SlangCodegen.Common

def shader : SlangShaderModule :=
  { structs := [ { name := "BoneJumpParams", fields := [fld "N" uT] } ]
  , globals := [ paramsCB "BoneJumpParams", roF "accIn" 1, roU "ancIn" 2, rwF "accOut" 3, rwU "ancOut" 4 ]
  , functions :=
      [ entry 64 [dtid]
          ([ let_ uT "b" (.member (v "tid") "x")
           , if_ (ge (v "b") (p "N")) [ ret ]
           , let_ uT "a" (at_ "ancIn" (v "b"))
           , let_ uT "ob" (v "b" * u 12) ] ++
           loadRot12 "A" "accIn" (v "ob") ++ loadT12 "at" "accIn" (v "ob") ++
           [ if_ (eq (v "a") (u 4294967295))
               (store12 "accOut" (v "ob") "A" "at" ++ [ setAt "ancOut" (v "b") (u 4294967295) ])
               ([ let_ uT "oa" (v "a" * u 12) ] ++
                loadRot12 "P" "accIn" (v "oa") ++ loadT12 "pt" "accIn" (v "oa") ++
                matMul "C" "P" "A" ++
                matVec "Pa" "P" "at" ++
                vec "c" (fun i => s "Pa" i + s "pt" i) ++
                store12 "accOut" (v "ob") "C" "c" ++
                [ setAt "ancOut" (v "b") (at_ "ancIn" (v "a")) ]) ]) ] }

end Bone.SlangCodegen.Jump
