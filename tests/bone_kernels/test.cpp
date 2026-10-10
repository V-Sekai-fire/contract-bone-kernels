// Test of the bone kernels' C++ (server) emission against the engine. A fixture holds a clip's key
// tables, a skeleton and the skin matrices the engine's own CPU clip player and skeleton produced at
// fixture time (tests/bone_kernels/fixtures/<name>, meta.json says how). The test runs sample, local,
// pointer-jumping rounds until every bone reaches its root, and skin, then compares every skin float
// with the engine's, relative to max(1, |engine value|).
//   test <fixture_dir>            must pass (worst relative diff <= 1e-4)
//   test <fixture_dir> --control  samples 0.25 s late; must fail
#include <cmath>
#include <cstring>
#include <utility>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <regex>
#include <string>
#include <vector>

extern "C" void bone_sample_run(uint32_t, float, float, float, float, uint32_t, uint32_t, uint32_t *, uint32_t *, uint32_t *, float *, float *, size_t,
		float *, float *, float *, size_t);
// A double time as float hi + lo (df32), as the hosts split the engine's doubles.
static inline float df_hi(double x) { return float(x); }
static inline float df_lo(double x) { return float(x - double(float(x))); }
extern "C" void bone_local_run(uint32_t, uint32_t *, float *, float *, uint32_t *);
extern "C" void bone_jump_run(uint32_t, float *, uint32_t *, float *, uint32_t *);
extern "C" void bone_skin_run(uint32_t, uint32_t *, float *, float *, size_t, float *);

template <class T>
static std::vector<T> load(const std::string &path) {
	std::ifstream f(path, std::ios::binary);
	if (!f) {
		std::fprintf(stderr, "missing %s\n", path.c_str());
		std::exit(2);
	}
	std::vector<char> raw((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
	std::vector<T> v(raw.size() / sizeof(T));
	std::memcpy(v.data(), raw.data(), v.size() * sizeof(T));
	return v;
}

static double num(const std::string &json, const std::string &key) {
	std::smatch m;
	std::regex re("\"" + key + "\"\\s*:\\s*([-0-9.eE]+)");
	if (!std::regex_search(json, m, re)) {
		std::fprintf(stderr, "meta.json has no %s\n", key.c_str());
		std::exit(2);
	}
	return std::stod(m[1]);
}

int main(int argc, char **argv) {
	if (argc < 2) {
		std::fprintf(stderr, "usage: test <fixture_dir> [--control]\n");
		return 2;
	}
	const std::string d = argv[1];
	const bool control = argc > 2 && std::string(argv[2]) == "--control";
	std::ifstream mf(d + "/meta.json");
	const std::string meta((std::istreambuf_iterator<char>(mf)), std::istreambuf_iterator<char>());
	const float time = float(num(meta, "time")) + (control ? 0.25f : 0.0f);
	const float length = float(num(meta, "length"));
	const uint32_t loop = uint32_t(num(meta, "loop_mode"));
	const uint32_t T = uint32_t(num(meta, "tracks"));
	const size_t bones = size_t(num(meta, "bones"));
	const uint32_t B = uint32_t(num(meta, "binds"));
	auto trackKeys = load<uint32_t>(d + "/track_keys.bin");
	auto trackDst = load<uint32_t>(d + "/track_dst.bin");
	auto trackKind = load<uint32_t>(d + "/track_kind.bin");
	auto keyTime = load<float>(d + "/key_time.bin");
	auto keyVal = load<float>(d + "/key_val.bin");
	auto keyTrans = load<float>(d + "/key_trans.bin");
	std::vector<float> keyTimeLo(keyTime.size(), 0.0f); // these fixtures store key times as float32 only
	auto localPose = load<float>(d + "/local_pose.bin");
	auto parent = load<uint32_t>(d + "/parent.bin");
	auto bindBone = load<uint32_t>(d + "/bind_bone.bin");
	auto bindPose = load<float>(d + "/bind_pose.bin");
	auto expect = load<float>(d + "/expect_skin.bin");
	std::vector<float> globalPose(12 * bones, 0.0f), skin(12 * size_t(B), 0.0f);

	bone_sample_run(T, time, 0.0f, length, 0.0f, loop, 0, trackKeys.data(), trackDst.data(), trackKind.data(), keyTime.data(), keyTimeLo.data(),
			keyTime.size(), keyVal.data(), keyTrans.data(), localPose.data(), localPose.size());
	// local matrices, then pointer-jumping rounds until no bone has an ancestor left
	const uint32_t N = uint32_t(bones);
	std::vector<float> acc2(12 * bones, 0.0f);
	std::vector<uint32_t> anc(bones), anc2(bones);
	bone_local_run(N, parent.data(), localPose.data(), globalPose.data(), anc.data());
	float *accIn = globalPose.data(), *accOut = acc2.data();
	uint32_t *ancIn = anc.data(), *ancOut = anc2.data();
	for (int round = 0; round < 64; round++) {
		bool open = false;
		for (uint32_t a : std::vector<uint32_t>(ancIn, ancIn + bones)) {
			open = open || a != 0xFFFFFFFFu;
		}
		if (!open) {
			break;
		}
		bone_jump_run(N, accIn, ancIn, accOut, ancOut);
		std::swap(accIn, accOut);
		std::swap(ancIn, ancOut);
	}
	bone_skin_run(B, bindBone.data(), bindPose.data(), accIn, bones, skin.data());

	double worst = 0.0;
	size_t at = 0;
	for (size_t i = 0; i < expect.size() && i < skin.size(); i++) {
		const double e = std::fabs(double(skin[i]) - double(expect[i])) / std::fmax(1.0, std::fabs(double(expect[i])));
		if (e > worst || std::isnan(e)) {
			worst = std::isnan(e) ? INFINITY : e;
			at = i;
		}
	}
	const bool ok = worst <= 1e-4 && expect.size() == skin.size();
	std::printf("%s  %s, %s: C++ emission vs the engine's CPU clip player, %zu bones, %zu floats: worst relative diff %.3g (float %zu)\n",
			ok ? "PASS" : "FAIL", d.substr(d.find_last_of('/') + 1).c_str(), control ? "control (0.25 s late)" : "fixture time", bones,
			skin.size(), worst, at);
	return ok ? 0 : 1;
}
