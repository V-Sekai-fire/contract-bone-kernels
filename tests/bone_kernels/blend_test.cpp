// Test of bone_sample + bone_blend's C++ (server) emission against the engine's own AnimationTree
// mixer. The fixture (fixtures_blend/, meta.json says how it was made) holds three clips' track
// tables, each clip's per-bone weight per frame (Blend2 with a filter, then Add2), the bones' init
// (rest) pose, motion scale, used channels, the pose before each frame, and the engine's pose after.
// Rotations compare up to sign; differences are relative to max(1, |engine value|).
//   blend_test <dir>                          must pass (worst relative diff <= 1e-4)
//   blend_test <dir> --control-order          blends the clips in reverse order; must fail
//   blend_test <dir> --control-deterministic  flips the deterministic flag; must fail
//   blend_test <dir> --control-filter         ignores the filter (every weight from unfiltered bones); must fail
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include <regex>
#include <string>
#include <vector>

extern "C" void bone_sample_run(uint32_t, float, float, uint32_t, uint32_t, uint32_t *, uint32_t *, uint32_t *, float *, size_t, float *, float *, float *,
		size_t);
extern "C" void bone_blend_run(uint32_t, uint32_t, uint32_t, float *, uint32_t *, float *, float *, float *, uint32_t *, float *);

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

static std::vector<double> nums(const std::string &s) {
	std::vector<double> out;
	std::regex n("[-0-9.eE]+");
	for (auto it = std::sregex_iterator(s.begin(), s.end(), n); it != std::sregex_iterator(); ++it) {
		out.push_back(std::stod(it->str()));
	}
	return out;
}

static std::vector<double> list(const std::string &json, const std::string &key) {
	std::smatch m;
	if (!std::regex_search(json, m, std::regex("\"" + key + "\"\\s*:\\s*\\[([^\\]]*)\\]"))) {
		std::fprintf(stderr, "meta.json has no %s\n", key.c_str());
		std::exit(2);
	}
	return nums(m[1]);
}

struct Clip {
	std::vector<uint32_t> keys, dst, kind;
	std::vector<float> time, val, trans;
};

int main(int argc, char **argv) {
	if (argc < 2) {
		std::fprintf(stderr, "usage: blend_test <dir> [--control-order|--control-deterministic|--control-filter]\n");
		return 2;
	}
	const std::string d = argv[1];
	const std::string mode = argc > 2 ? argv[2] : "";
	std::ifstream mf(d + "/meta.json");
	const std::string meta((std::istreambuf_iterator<char>(mf)), std::istreambuf_iterator<char>());
	std::smatch m;
	std::regex_search(meta, m, std::regex("\"bones\"\\s*:\\s*([0-9]+)"));
	const uint32_t N = uint32_t(std::stoul(m[1]));
	const auto lengths = list(meta, "lengths");
	const auto loops = list(meta, "loop_modes");
	const uint32_t I = uint32_t(lengths.size());
	// frames: [{"deterministic":d,"times":[a,b,c]}, ...] in either key order
	std::vector<std::vector<double>> ftimes;
	std::vector<uint32_t> fdet;
	{
		std::regex fr("\\{[^{}]*\\}");
		std::string s = meta.substr(meta.find("\"frames\""));
		s = s.substr(0, s.find("]}") == std::string::npos ? s.size() : s.find("}]") + 1);
		for (auto it = std::sregex_iterator(s.begin(), s.end(), fr); it != std::sregex_iterator(); ++it) {
			const std::string o = it->str();
			std::smatch a, b;
			std::regex_search(o, a, std::regex("\"times\"\\s*:\\s*\\[([^\\]]*)\\]"));
			std::regex_search(o, b, std::regex("\"deterministic\"\\s*:\\s*([0-9])"));
			ftimes.push_back(nums(a[1]));
			fdet.push_back(uint32_t(std::stoul(b[1])));
		}
	}
	const size_t F = ftimes.size();
	std::vector<Clip> clips(I);
	for (uint32_t c = 0; c < I; c++) {
		const std::string p = d + "/clip" + std::to_string(c) + "_";
		clips[c] = { load<uint32_t>(p + "track_keys.bin"), load<uint32_t>(p + "track_dst.bin"), load<uint32_t>(p + "track_kind.bin"),
			load<float>(p + "key_time.bin"), load<float>(p + "key_val.bin"), load<float>(p + "key_trans.bin") };
	}
	auto present = load<uint32_t>(d + "/present.bin");
	auto init = load<float>(d + "/init.bin");
	auto ms = load<float>(d + "/motion_scale.bin");
	auto used = load<uint32_t>(d + "/used.bin");
	auto weight = load<float>(d + "/weight.bin");
	auto pre = load<float>(d + "/pre_pose.bin");
	auto expect = load<float>(d + "/expect.bin");

	double worst = 0.0;
	size_t wf = 0, wb = 0;
	for (size_t f = 0; f < F; f++) {
		std::vector<float> samp(10 * size_t(I) * N, 0.0f);
		for (uint32_t c = 0; c < I; c++) {
			Clip &k = clips[c];
			const uint32_t T = uint32_t(k.dst.size());
			if (T > 0) {
				bone_sample_run(T, float(ftimes[f][c]), float(lengths[c]), uint32_t(loops[c]), 0, k.keys.data(), k.dst.data(), k.kind.data(),
						k.time.data(), k.time.size(), k.val.data(), k.trans.data(), samp.data(), samp.size());
			}
		}
		std::vector<float> w(weight.begin() + f * I * N, weight.begin() + (f + 1) * I * N);
		std::vector<uint32_t> pr = present;
		if (mode == "--control-filter") {
			// Every bone takes the weights the unfiltered bones get: the first bone's (bone 0 is not filtered).
			for (uint32_t c = 0; c < I; c++) {
				for (uint32_t b = 0; b < N; b++) {
					w[c * N + b] = weight[f * I * N + c * N + 0];
				}
			}
		}
		if (mode == "--control-order") {
			// Reverse the clip order: swap clip 0 and clip I-1 in samples, flags and weights.
			const uint32_t a = 0, z = I - 1;
			for (uint32_t b = 0; b < N; b++) {
				std::swap(w[a * N + b], w[z * N + b]);
				std::swap(pr[a * N + b], pr[z * N + b]);
				for (int q = 0; q < 10; q++) {
					std::swap(samp[(a * N + b) * 10 + q], samp[(z * N + b) * 10 + q]);
				}
			}
		}
		const uint32_t det = mode == "--control-deterministic" ? 1u - fdet[f] : fdet[f];
		std::vector<float> pose(pre.begin() + f * N * 10, pre.begin() + (f + 1) * N * 10);
		bone_blend_run(N, I, det, samp.data(), pr.data(), w.data(), init.data(), ms.data(), used.data(), pose.data());
		const float *o = &expect[f * N * 10];
		for (uint32_t b = 0; b < N; b++) {
			const int groups[3][2] = { { 0, 3 }, { 3, 7 }, { 7, 10 } };
			for (auto &g : groups) {
				double e = 0.0, en = 0.0;
				for (int c = g[0]; c < g[1]; c++) {
					const double sc = std::fmax(1.0, std::fabs(double(o[b * 10 + c])));
					e = std::fmax(e, std::fabs(double(pose[b * 10 + c]) - o[b * 10 + c]) / sc);
					en = std::fmax(en, std::fabs(double(pose[b * 10 + c]) + o[b * 10 + c]) / sc);
				}
				if (g[0] == 3) {
					e = std::fmin(e, en);
				}
				if (std::isnan(e) || e > worst) {
					worst = std::isnan(e) ? INFINITY : e;
					wf = f;
					wb = b;
				}
			}
		}
	}
	const bool ok = worst <= 1e-4;
	std::printf("%s  blend%s: bone_sample + bone_blend C++ emission vs the engine's AnimationTree, %u clips, %u bones, %zu frames: worst relative diff %.3g (frame %zu, bone %zu)\n",
			ok ? "PASS" : "FAIL", mode.empty() ? "" : (" " + mode).c_str(), I, N, F, worst, wf, wb);
	return ok ? 0 : 1;
}
