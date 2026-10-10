# contract-bone-kernels

The animation mixer's per-frame bone work as Lean kernels. The kernels are emitted as Slang, then compiled for GPU clients and for servers without a GPU.

## What it is for

Five compute kernels do what the engine's animation mixer and skeleton do on the CPU each frame:

1. `bone_sample` samples every keyframe track at the clip time, as the engine's `Animation` track samplers do. It covers all seven interpolation modes (nearest, linear, cubic, makima and their angle variants), the none, linear and ping-pong loop modes with each track's loop-wrap setting, key transitions (easing, and 0 to hold a key), and backward playback for nearest keys. Blend-shape tracks are sampled the same way; for them the angle modes are the engine's angle forms. Times are carried in df32, a pair of floats: the engine keeps key times in double, and with keys a millisecond apart the cubic and makima curves magnify float32 time rounding hundreds of times. Both targets compile with `-fp-mode precise` so the compiler keeps the df32 arithmetic as written.
2. `bone_blend` blends the sampled clips per bone as the mixer does: weights with per-bone filter weights (so AnimationTree blend, add and filter nodes reduce to it), deterministic or normalised weighting, rotations applied in clip order, and the skeleton's motion scale. Bones start from their rest pose.
3. `bone_local` writes each bone's local pose as a 3×4 matrix.
4. `bone_jump` does one pointer-jumping round: each bone multiplies in its current ancestor's transform, then points at that ancestor's ancestor. After ⌈log₂(depth + 1)⌉ rounds, every bone holds its skeleton-space pose. This works for any tree shape, so a chain of 4,096 bones takes 12 rounds.
5. `bone_skin` multiplies each pose by its bind pose. It writes the 3×4 row layout that the skinning pass reads.

The Lean tree in `lean/` emits the Slang in `kernels/bone/slang/`. `gen.sh` compiles that Slang to GPU bytecode in `kernels/bone/spv/` and to C++ in `kernels/bone/cpp/`. An engine vendors only those emitted files.

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

The fixtures in `tests/bone_kernels/fixtures_sample/` hold the engine's own track samples, one directory per loop mode. Each has 56 tracks: 7 interpolation modes × loop-wrap on and off × position, rotation, scale and blend shape, with mixed key transitions. The engine's value is stored at 103 times, forward and backward. `sample_test` checks `bone_sample` against them. It has two controls that must fail: one samples 0.05 s late, and one ignores key transitions.

The build finds the repositories it needs as sibling checkouts, at their paths in the goal manifest.

`tests/bone_kernels/fixtures_blend/` is the engine's own AnimationTree output for 24 bones over 10 frames. Three clips are mixed by a Blend2 node with a bone filter, then an Add2 node. `blend_test` checks `bone_sample` and `bone_blend` against it. Its controls must fail: one blends the clips in reverse order, one flips the deterministic flag, and one ignores the filter.

In the engine, blend-shape tracks and transform tracks on other 3D nodes blend by the same rule, but start from the RESET animation (not the rest) and have no motion scale. Feeding them to `bone_blend` that way is not yet tested against the engine. Not covered yet: root motion, and value, bezier, method, audio and animation tracks.

## Licence

MIT. See [LICENSE](LICENSE). `kernels/bone/CITATION.cff` names the engine code whose semantics the kernels restate.
