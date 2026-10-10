struct BoneSampleParams {
  uint T;
  float time;
  float length;
  uint loop;
};

[[vk::binding(0, 0)]]
ConstantBuffer<BoneSampleParams> params;
[[vk::binding(1, 0)]]
StructuredBuffer<uint> trackKeys;
[[vk::binding(2, 0)]]
StructuredBuffer<uint> trackDst;
[[vk::binding(3, 0)]]
StructuredBuffer<uint> trackKind;
[[vk::binding(4, 0)]]
StructuredBuffer<float> keyTime;
[[vk::binding(5, 0)]]
StructuredBuffer<float> keyVal;
[[vk::binding(6, 0)]]
RWStructuredBuffer<float> localPose;

[shader("compute")] [numthreads(64, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID) {
  uint i = tid.x;
  if ((i >= params.T)) {
    return;
  }
  uint off = trackKeys[(i * 2u)];
  uint cnt = trackKeys[((i * 2u) + 1u)];
  uint dst = trackDst[i];
  uint last = ((off + cnt) - 1u);
  bool looping = (params.loop != 0u);
  float L = params.length;
  float t = params.time;
  if ((looping && (L > 0.000000))) {
    t = (t - (L * floor((t / L))));
  }
  float t0 = keyTime[off];
  float t1 = keyTime[last];
  float wrap = ((t0 + L) - t1);
  uint a = off;
  uint b = off;
  float w = 0.000000;
  if ((cnt > 1u)) {
    if ((t >= t1)) {
      a = last;
      if ((looping && (wrap > 0.000000))) {
        b = off;
        w = ((t - t1) / wrap);
      } else {
        b = last;
      }
    } else {
      if ((t < t0)) {
        if ((looping && (wrap > 0.000000))) {
          a = last;
          b = off;
          w = (((t + L) - t1) / wrap);
        }
      } else {
        uint lo = off;
        uint hi = last;
        while (((hi - lo) > 1u)) {
          uint mid = ((lo + hi) / 2u);
          if ((keyTime[mid] <= t)) {
            lo = mid;
          } else {
            hi = mid;
          }
        }
        a = lo;
        b = hi;
        w = ((t - keyTime[lo]) / (keyTime[hi] - keyTime[lo]));
      }
    }
  }
  float qa0 = keyVal[((a * 4u) + 0u)];
  float qa1 = keyVal[((a * 4u) + 1u)];
  float qa2 = keyVal[((a * 4u) + 2u)];
  float qa3 = keyVal[((a * 4u) + 3u)];
  float qb0 = keyVal[((b * 4u) + 0u)];
  float qb1 = keyVal[((b * 4u) + 1u)];
  float qb2 = keyVal[((b * 4u) + 2u)];
  float qb3 = keyVal[((b * 4u) + 3u)];
  if ((trackKind[i] == 0u)) {
    localPose[(dst + 0u)] = (qa0 + ((qb0 - qa0) * w));
    localPose[(dst + 1u)] = (qa1 + ((qb1 - qa1) * w));
    localPose[(dst + 2u)] = (qa2 + ((qb2 - qa2) * w));
  } else {
    float c = ((((qa0 * qb0) + (qa1 * qb1)) + (qa2 * qb2)) + (qa3 * qb3));
    float sg = ((c < 0.000000) ? -1.000000 : 1.000000);
    c = (c * sg);
    float s0 = (1.000000 - w);
    float s1 = w;
    if (((1.000000 - c) > 0.00001f)) {
      float om = acos(c);
      float si = sin(om);
      s0 = (sin(((1.000000 - w) * om)) / si);
      s1 = (sin((w * om)) / si);
    }
    s1 = (s1 * sg);
    float q0 = ((s0 * qa0) + (s1 * qb0));
    float q1 = ((s0 * qa1) + (s1 * qb1));
    float q2 = ((s0 * qa2) + (s1 * qb2));
    float q3 = ((s0 * qa3) + (s1 * qb3));
    float n = sqrt(((((q0 * q0) + (q1 * q1)) + (q2 * q2)) + (q3 * q3)));
    n = (1.000000 / max(n, 1.0e-20f));
    localPose[(dst + 0u)] = (q0 * n);
    localPose[(dst + 1u)] = (q1 * n);
    localPose[(dst + 2u)] = (q2 * n);
    localPose[(dst + 3u)] = (q3 * n);
  }
}
