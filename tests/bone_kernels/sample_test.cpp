// Test of bone_sample's C++ (server) emission against the engine's own track samplers
// (Animation.position/rotation/scale_track_interpolate on the reference branch). A sampling fixture
// (fixtures_sample/<loop mode>) holds 42 tracks: 7 interpolation modes x loop_wrap on/off x
// position/rotation/scale, with mixed key transitions, and the engine's value of every track at
// every (time, direction) in meta.json. Rotations compare up to sign (q and -q are one rotation).
// Differences are relative to max(1, |engine value|).
//   sample_test <dir>                       must pass (worst relative diff <= 1e-4)
//   sample_test <dir> --control-time        samples 0.05 s late; must fail
//   sample_test <dir> --control-transition  ignores key transitions; must fail
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
	if (!std::regex_search(json, m, std::regex("\"" + key + "\"\\s*:\\s*([-0-9.eE]+)"))) {
		std::fprintf(stderr, "meta.json has no %s\n", key.c_str());
		std::exit(2);
	}
	return std::stod(m[1]);
}

static std::vector<double> list(const std::string &json, const std::string &key) {
	std::smatch m;
	std::regex_search(json, m, std::regex("\"" + key + "\"\\s*:\\s*\\[([^\\]]*)\\]"));
	std::string s = m[1];
	std::vector<double> out;
	std::regex n("[-0-9.eE]+");
	for (auto it = std::sregex_iterator(s.begin(), s.end(), n); it != std::sregex_iterator(); ++it) {
		out.push_back(std::stod(it->str()));
	}
	return out;
}

int main(int argc, char **argv) {
	if (argc < 2) {
		std::fprintf(stderr, "usage: sample_test <dir> [--control-time|--control-transition]\n");
		return 2;
	}
	const std::string d = argv[1];
	const std::string mode = argc > 2 ? argv[2] : "";
	std::ifstream mf(d + "/meta.json");
	const std::string meta((std::istreambuf_iterator<char>(mf)), std::istreambuf_iterator<char>());
	const float length = float(num(meta, "length"));
	const uint32_t loopMode = uint32_t(num(meta, "loop_mode"));
	const uint32_t T = uint32_t(num(meta, "tracks"));
	const auto times = list(meta, "times");
	const auto backward = list(meta, "backward");
	auto trackKeys = load<uint32_t>(d + "/track_keys.bin");
	auto trackDst = load<uint32_t>(d + "/track_dst.bin");
	auto trackKind = load<uint32_t>(d + "/track_kind.bin");
	auto keyTime = load<float>(d + "/key_time.bin");
	auto keyVal = load<float>(d + "/key_val.bin");
	auto keyTrans = load<float>(d + "/key_trans.bin");
	auto expect = load<float>(d + "/expect.bin");
	if (mode == "--control-transition") {
		for (float &x : keyTrans) {
			x = 1.0f;
		}
	}
	const float shift = mode == "--control-time" ? 0.05f : 0.0f;
	std::vector<float> pose(10 * size_t(T));
	double worst = 0.0;
	size_t worstRun = 0, worstTrack = 0;
	for (size_t r = 0; r < times.size(); r++) {
		std::fill(pose.begin(), pose.end(), 0.0f);
		bone_sample_run(T, float(times[r]) + shift, length, loopMode, uint32_t(backward[r]), trackKeys.data(), trackDst.data(), trackKind.data(),
				keyTime.data(), keyTime.size(), keyVal.data(), keyTrans.data(), pose.data(), pose.size());
		for (uint32_t t = 0; t < T; t++) {
			const bool rot = (trackKind[t] & 1u) != 0;
			const float *o = &expect[(r * T + t) * 4];
			double e = 0.0, en = 0.0;
			for (int c = 0; c < (rot ? 4 : 3); c++) {
				const double g = pose[trackDst[t] + c];
				const double sc = std::fmax(1.0, std::fabs(double(o[c])));
				e = std::fmax(e, std::fabs(g - o[c]) / sc);
				en = std::fmax(en, std::fabs(g + o[c]) / sc);
			}
			if (rot) {
				e = std::fmin(e, en);
			}
			if (std::isnan(e) || e > worst) {
				worst = std::isnan(e) ? INFINITY : e;
				worstRun = r;
				worstTrack = t;
			}
		}
	}
	const bool ok = worst <= 1e-4;
	std::printf("%s  %s%s: bone_sample C++ emission vs the engine's track samplers, %u tracks x %zu runs: worst relative diff %.3g (track %zu, t %.4f%s)\n",
			ok ? "PASS" : "FAIL", d.substr(d.find_last_of('/') + 1).c_str(), mode.empty() ? "" : (" " + mode).c_str(), T, times.size(), worst, worstTrack,
			times[worstRun], backward[worstRun] != 0.0 ? ", backward" : "");
	return ok ? 0 : 1;
}
