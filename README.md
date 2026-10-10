# contract-bone-kernels

The animation mixer's per-frame bone work as Lean kernels. The kernels are emitted as Slang, then compiled for GPU clients and for servers without a GPU.

## What it is for

Four compute kernels do what the engine's clip player and skeleton do on the CPU each frame:

1. `bone_sample` samples every keyframe track at the clip time. Position and scale use linear interpolation, and rotation uses spherical linear interpolation. Time wraps as in the linear loop mode.
2. `bone_local` writes each bone's local pose as a 3×4 matrix.
3. `bone_jump` does one pointer-jumping round: each bone multiplies in its current ancestor's transform, then points at that ancestor's ancestor. After ⌈log₂(depth + 1)⌉ rounds, every bone holds its skeleton-space pose. This works for any tree shape, so a chain of 4,096 bones takes 12 rounds.
4. `bone_skin` multiplies each pose by its bind pose. It writes the 3×4 row layout that the skinning pass reads.

The Lean tree in `lean/` emits the Slang in `kernels/bone/slang/`. `gen.sh` compiles that Slang to GPU bytecode in `kernels/bone/spv/` and to C++ in `kernels/bone/cpp/`. An engine vendors only those emitted files.

The kernels play one clip at full weight. Blending between clips is not supported yet.

## Build

```sh
kernels/bone/gen.sh            # emit from Lean, then compile both targets
kernels/bone/gen.sh --check    # re-emit and diff against the committed Slang (exit 1 on drift)
tests/bone_kernels/build.sh    # C++ emission vs the engine's own CPU result, plus a control per fixture that must fail
```

The fixtures in `tests/bone_kernels/fixtures/` are the engine's CPU results:

- `fan`: 201 bones under a shared root.
- `chain`: one chain of 512 bones.
- `random`: 4,096 bones with random parents, depth 16.

The chain and random fixtures include non-uniform scale, so the bones below a scaled bone are sheared. Each `meta.json` says how its fixture was made.

The build finds the repositories it needs as sibling checkouts, at their paths in the goal manifest.

## Licence

MIT. See [LICENSE](LICENSE). `kernels/bone/CITATION.cff` names the engine code whose semantics the kernels restate.
