#!/bin/sh
# Build and run the test of the bone kernels' C++ (server) emission.
#
#   tests/bone_kernels/build.sh   build, then run every fixture (must pass) and its control (must fail)
#
# Compiles kernels/bone/cpp/*.cpu.cpp (committed; regenerate with kernels/bone/gen.sh), one
# translation unit per kernel with -ffp-contract=off, keeping one *_run global each, and links
# them with test.cpp. Each fixture in fixtures/ is the engine's own CPU result; its meta.json says
# how it was made.
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
OUT="$ROOT/build/bone_kernels_test"
CXX=${CXX:-c++}
FLAGS="-O2 -std=c++17 -ffp-contract=off -Wno-everything -I$ROOT/kernels/bone/cpp"
mkdir -p "$OUT"
n=1
for k in sample local jump skin blend; do
	"$CXX" $FLAGS -DBONE_KERNEL=$n -c "$HERE/kernel_tu.cpp" -o "$OUT/$k.full.o"
	# each emission defines its own main_0: keep only this unit's bone_<k>_run global
	if [ "$(uname)" = Darwin ]; then
		ld -r -exported_symbol "_bone_${k}_run" "$OUT/$k.full.o" -o "$OUT/$k.o"
	else
		objcopy --keep-global-symbol="bone_${k}_run" "$OUT/$k.full.o" "$OUT/$k.o"
	fi
	n=$((n + 1))
done
"$CXX" $FLAGS "$HERE/test.cpp" "$OUT/sample.o" "$OUT/local.o" "$OUT/jump.o" "$OUT/skin.o" -o "$OUT/bone_kernels_test"
"$CXX" $FLAGS "$HERE/sample_test.cpp" "$OUT/sample.o" -o "$OUT/sample_test"
"$CXX" $FLAGS "$HERE/blend_test.cpp" "$OUT/sample.o" "$OUT/blend.o" -o "$OUT/blend_test"
rc=0
for fx in "$HERE"/fixtures/*/; do
	fx=${fx%/}
	"$OUT/bone_kernels_test" "$fx" || rc=1
	if "$OUT/bone_kernels_test" "$fx" --control > /dev/null; then
		echo "control passed on $(basename "$fx"), so the test cannot tell a wrong time from a right one" >&2
		rc=1
	else
		echo "control failed on $(basename "$fx"), as it must"
	fi
done
for fx in "$HERE"/fixtures_sample/*/; do
	fx=${fx%/}
	"$OUT/sample_test" "$fx" || rc=1
	for c in --control-time --control-transition; do
		if "$OUT/sample_test" "$fx" $c > /dev/null; then
			echo "control $c passed on $(basename "$fx"), so the test cannot see that difference" >&2
			rc=1
		else
			echo "control $c failed on $(basename "$fx"), as it must"
		fi
	done
done
"$OUT/blend_test" "$HERE/fixtures_blend" || rc=1
for c in --control-order --control-deterministic --control-filter; do
	if "$OUT/blend_test" "$HERE/fixtures_blend" $c > /dev/null; then
		echo "control $c passed on blend, so the test cannot see that difference" >&2
		rc=1
	else
		echo "control $c failed on blend, as it must"
	fi
done
exit $rc
