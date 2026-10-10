// Test of root_motion's C++ (server) emission against the engine's get_root_motion_position, _rotation
// and _scale (fixtures_root/, meta.json says how it was made). Each step holds the three instances'
// PlaybackInfo (time, delta, start, end, length as df32 pairs), loop modes, flags, root track indices,
// weights, the deterministic and local flags, and the engine's result. The accumulators are the root
// target's ordinary blend, which blend_test covers.
//   root_test <dir>                    must pass (worst relative diff <= 1e-4)
//   root_test <dir> --control-local    flips local mode; must fail
//   root_test <dir> --control-noloop   treats every clip as not looping; must fail
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

extern "C" void root_motion_run(uint32_t I, uint32_t deterministic, uint32_t local, uint32_t *trackKeys, uint32_t *trackKind, float *keyTime,
		float *keyVal, float *keyTrans, float *keyTimeLo, size_t K, uint32_t T, float *instF, uint32_t *instU, float *weight, float *out);

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

static std::vector<double> list(const std::string &json, const std::string &key) {
	std::smatch m;
	std::vector<double> out;
	if (!std::regex_search(json, m, std::regex("\"" + key + "\"\\s*:\\s*\\[([^\\]]*)\\]"))) {
		return out;
	}
	std::string s = m[1];
	std::regex n("[-0-9.eE]+");
	for (auto it = std::sregex_iterator(s.begin(), s.end(), n); it != std::sregex_iterator(); ++it) {
		out.push_back(std::stod(it->str()));
	}
	return out;
}

static double num(const std::string &json, const std::string &key) {
	std::smatch m;
	std::regex_search(json, m, std::regex("\"" + key + "\"\\s*:\\s*([-0-9.eE]+)"));
	return std::stod(m[1]);
}

int main(int argc, char **argv) {
	if (argc < 2) {
		std::fprintf(stderr, "usage: root_test <dir> [--control-local|--control-noloop]\n");
		return 2;
	}
	const std::string d = argv[1];
	const std::string mode = argc > 2 ? argv[2] : "";
	std::ifstream mf(d + "/meta.json");
	const std::string meta((std::istreambuf_iterator<char>(mf)), std::istreambuf_iterator<char>());
	auto trackKeys = load<uint32_t>(d + "/track_keys.bin");
	auto trackKind = load<uint32_t>(d + "/track_kind.bin");
	auto keyTime = load<float>(d + "/key_time.bin");
	auto keyVal = load<float>(d + "/key_val.bin");
	auto keyTrans = load<float>(d + "/key_trans.bin");
	auto keyTimeLo = load<float>(d + "/key_time_lo.bin");
	const uint32_t T = uint32_t(trackKind.size());
	const uint32_t I = uint32_t(num(meta, "instances"));
	const std::string steps = meta.substr(meta.find("\"steps\""));
	std::regex obj("\\{[^{}]*\\}");
	double worst = 0.0;
	size_t ws = 0, nsteps = 0;
	for (auto it = std::sregex_iterator(steps.begin(), steps.end(), obj); it != std::sregex_iterator(); ++it, nsteps++) {
		const std::string o = it->str();
		auto fd = list(o, "inst_f"), ud = list(o, "inst_u"), wd = list(o, "weight"), ex = list(o, "expect");
		std::vector<float> instF(fd.begin(), fd.end()), weight(wd.begin(), wd.end());
		std::vector<uint32_t> instU;
		for (double x : ud) {
			instU.push_back(uint32_t(int64_t(x)));
		}
		uint32_t det = uint32_t(num(o, "deterministic")), local = uint32_t(num(o, "local"));
		if (mode == "--control-local") {
			local = 1 - local;
		}
		if (mode == "--control-noloop") {
			for (uint32_t i = 0; i < I; i++) {
				instU[i * 6] = 0;
			}
		}
		float out[10] = {};
		root_motion_run(I, det, local, trackKeys.data(), trackKind.data(), keyTime.data(), keyVal.data(), keyTrans.data(), keyTimeLo.data(),
				keyTime.size(), T, instF.data(), instU.data(), weight.data(), out);
		const int groups[3][2] = { { 0, 3 }, { 3, 7 }, { 7, 10 } };
		for (auto &g : groups) {
			double e = 0.0, en = 0.0;
			for (int c = g[0]; c < g[1]; c++) {
				const double sc = std::fmax(1.0, std::fabs(ex[c]));
				e = std::fmax(e, std::fabs(out[c] - ex[c]) / sc);
				en = std::fmax(en, std::fabs(out[c] + ex[c]) / sc);
			}
			if (g[0] == 3) {
				e = std::fmin(e, en);
			}
			if (std::isnan(e) || e > worst) {
				worst = std::isnan(e) ? INFINITY : e;
				ws = nsteps;
			}
		}
	}
	const bool ok = worst <= 1e-4 && nsteps > 0;
	std::printf("%s  root%s: root_motion C++ emission vs the engine's get_root_motion_*, %u instances, %zu steps: worst relative diff %.3g (step %zu)\n",
			ok ? "PASS" : "FAIL", mode.empty() ? "" : (" " + mode).c_str(), I, nsteps, worst, ws);
	return ok ? 0 : 1;
}
