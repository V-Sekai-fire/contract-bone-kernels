// The C++ (server) emission of the bone kernels, one translation unit each: each unit includes its
// kernel's emitted .cpu.cpp (Slang prelude included) and exports one C entry point. build.sh compiles
// this file once per kernel with -DBONE_KERNEL=<n> and keeps only that unit's *_run symbol global,
// so the kernels link into one test.
#include <cstddef>
#include <cstdint>

#if BONE_KERNEL == 1
#include "bone_sample.cpu.cpp"
extern "C" void bone_sample_run(uint32_t T, float time, float length, uint32_t loop, uint32_t *trackKeys, uint32_t *trackDst,
		uint32_t *trackKind, float *keyTime, size_t K, float *keyVal, float *localPose, size_t localCount) {
	BoneSampleParams_0 p{ T, time, length, loop };
	GlobalParams_0 g{};
	g.params_0 = &p;
	g.trackKeys_0.data = trackKeys; g.trackKeys_0.count = 2 * T;
	g.trackDst_0.data = trackDst; g.trackDst_0.count = T;
	g.trackKind_0.data = trackKind; g.trackKind_0.count = T;
	g.keyTime_0.data = keyTime; g.keyTime_0.count = K;
	g.keyVal_0.data = keyVal; g.keyVal_0.count = 4 * K;
	g.localPose_0.data = localPose; g.localPose_0.count = localCount;
	ComputeVaryingInput vi{};
	vi.startGroupID = uint3(0u, 0u, 0u);
	vi.endGroupID = uint3((T + 63u) / 64u, 1u, 1u);
	main_0(&vi, nullptr, &g);
}
#elif BONE_KERNEL == 2
#include "bone_local.cpu.cpp"
extern "C" void bone_local_run(uint32_t N, uint32_t *parent, float *localPose, float *acc, uint32_t *anc) {
	BoneLocalParams_0 p{ N };
	GlobalParams_0 g{};
	g.params_0 = &p;
	g.parent_0.data = parent; g.parent_0.count = N;
	g.localPose_0.data = localPose; g.localPose_0.count = 10 * size_t(N);
	g.acc_0.data = acc; g.acc_0.count = 12 * size_t(N);
	g.anc_0.data = anc; g.anc_0.count = N;
	ComputeVaryingInput vi{};
	vi.startGroupID = uint3(0u, 0u, 0u);
	vi.endGroupID = uint3((N + 63u) / 64u, 1u, 1u);
	main_0(&vi, nullptr, &g);
}
#elif BONE_KERNEL == 3
#include "bone_jump.cpu.cpp"
extern "C" void bone_jump_run(uint32_t N, float *accIn, uint32_t *ancIn, float *accOut, uint32_t *ancOut) {
	BoneJumpParams_0 p{ N };
	GlobalParams_0 g{};
	g.params_0 = &p;
	g.accIn_0.data = accIn; g.accIn_0.count = 12 * size_t(N);
	g.ancIn_0.data = ancIn; g.ancIn_0.count = N;
	g.accOut_0.data = accOut; g.accOut_0.count = 12 * size_t(N);
	g.ancOut_0.data = ancOut; g.ancOut_0.count = N;
	ComputeVaryingInput vi{};
	vi.startGroupID = uint3(0u, 0u, 0u);
	vi.endGroupID = uint3((N + 63u) / 64u, 1u, 1u);
	main_0(&vi, nullptr, &g);
}
#elif BONE_KERNEL == 4
#include "bone_skin.cpu.cpp"
extern "C" void bone_skin_run(uint32_t B, uint32_t *bindBone, float *bindPose, float *globalPose, size_t bones, float *skin) {
	BoneSkinParams_0 p{ B };
	GlobalParams_0 g{};
	g.params_0 = &p;
	g.bindBone_0.data = bindBone; g.bindBone_0.count = B;
	g.bindPose_0.data = bindPose; g.bindPose_0.count = 12 * B;
	g.globalPose_0.data = globalPose; g.globalPose_0.count = 12 * bones;
	g.skin_0.data = skin; g.skin_0.count = 12 * B;
	ComputeVaryingInput vi{};
	vi.startGroupID = uint3(0u, 0u, 0u);
	vi.endGroupID = uint3((B + 63u) / 64u, 1u, 1u);
	main_0(&vi, nullptr, &g);
}
#endif
