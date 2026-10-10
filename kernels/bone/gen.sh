#!/usr/bin/env bash
# The bone kernels, from Lean to both targets (the kernels/anny/gen.sh shape).
#
#   kernels/bone/gen.sh             # emit from Lean, then cpp + spirv
#   kernels/bone/gen.sh --no-emit   # use the committed slang/ (no lake)
#   kernels/bone/gen.sh --check     # re-emit into a temp dir and diff against slang/ (CI; exit 1 on drift)
#
#   Lean (lean/, `lake exe emit_bone`)  ->  slang/<k>.slang       (committed)
#     slangc -target cpp                ->  cpp/<k>.cpu.cpp       (committed, the server path)
#     slangc -target spirv              ->  spv/<k>.spv           (committed, the GPU path)
#
# Both targets compile with -fp-mode precise. The df32 time arithmetic (Curves.lean) needs the
# compiler not to reassociate or fuse float operations; `precise` on its locals alone was not enough
# for the GPU path (makima drifted by ~3e-2 against the engine until the whole module was precise).
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
SLANGC="${SLANGC:-slangc}"
KERNELS="bone_sample bone_blend bone_local bone_jump bone_skin"

if [ "${1:-}" = "--check" ]; then
	T="$(mktemp -d)"
	( cd "$ROOT/lean" && lake exe emit_bone "$T" >/dev/null )
	diff -ru "$HERE/slang" "$T" && echo "== slang/ matches the Lean emission ==" || { echo "== slang/ drifted from the Lean emission ==" >&2; exit 1; }
	exit 0
fi
if [ "${1:-}" != "--no-emit" ]; then
	( cd "$ROOT/lean" && lake exe emit_bone "$HERE/slang" >/dev/null )
	echo "== emitted $(echo $KERNELS | wc -w | tr -d ' ') kernels into slang/ =="
fi
mkdir -p "$HERE/cpp" "$HERE/spv"
cd "$HERE"
for k in $KERNELS; do
	"$SLANGC" "slang/$k.slang" -target cpp -stage compute -entry main -fp-mode precise -o "cpp/$k.cpu.cpp"
	"$SLANGC" "slang/$k.slang" -target spirv -stage compute -entry main -fp-mode precise -o "spv/$k.spv"
	echo "$k: cpp $(wc -c < cpp/$k.cpu.cpp | tr -d ' ') B, spirv $(wc -c < spv/$k.spv | tr -d ' ') B"
done
