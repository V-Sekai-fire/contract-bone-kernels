struct BoneBlendParams {
  uint N;
  uint I;
  uint deterministic;
};

[[vk::binding(0, 0)]]
ConstantBuffer<BoneBlendParams> params;
[[vk::binding(1, 0)]]
StructuredBuffer<float> samp;
[[vk::binding(2, 0)]]
StructuredBuffer<uint> present;
[[vk::binding(3, 0)]]
StructuredBuffer<float> weight;
[[vk::binding(4, 0)]]
StructuredBuffer<float> init;
[[vk::binding(5, 0)]]
StructuredBuffer<float> motionScale;
[[vk::binding(6, 0)]]
StructuredBuffer<uint> used;
[[vk::binding(7, 0)]]
RWStructuredBuffer<float> localPose;

float bk_ease(float x0, float c) {
  float x = min(max(x0, 0.000000), 1.000000);
  if ((c > 0.000000)) {
    if ((c < 1.000000)) {
      return (1.000000 - pow((1.000000 - x), (1.000000 / c)));
    } else {
      return pow(x, c);
    }
  } else {
    if ((c < 0.000000)) {
      if ((x < 0.500000)) {
        return (pow((x * 2.000000), (-c)) * 0.500000);
      } else {
        return (((1.000000 - pow((1.000000 - ((x - 0.500000) * 2.000000)), (-c))) * 0.500000) + 0.500000);
      }
    }
  }
  return 0.000000;
}

float bk_cubic(float from, float to, float pre, float post, float w, float toT, float preT, float postT) {
  float t = (0.000000 + (w * (toT - 0.000000)));
  float a1 = (pre + (((preT == 0.000000) ? 0.000000 : ((t - preT) / (-preT))) * (from - pre)));
  float a2 = (from + (((toT == 0.000000) ? 0.500000 : (t / toT)) * (to - from)));
  float a3 = (to + ((((postT - toT) == 0.000000) ? 1.000000 : ((t - toT) / (postT - toT))) * (post - to)));
  float b1 = (a1 + ((((toT - preT) == 0.000000) ? 0.000000 : ((t - preT) / (toT - preT))) * (a2 - a1)));
  float b2 = (a2 + (((postT == 0.000000) ? 1.000000 : (t / postT)) * (a3 - a2)));
  return (b1 + (((toT == 0.000000) ? 0.500000 : (t / toT)) * (b2 - b1)));
}

float bk_makima_tan(float p0, float p1, float p2, float p3) {
  float w1 = (abs((p3 - p2)) + (abs((p3 + p2)) * 0.500000));
  float w2 = (abs((p1 - p0)) + (abs((p1 + p0)) * 0.500000));
  if (((w1 + w2) == 0.000000)) {
    return ((p1 + p2) * 0.500000);
  }
  return (((w1 * p1) + (w2 * p2)) / (w1 + w2));
}

float bk_makima(float from, float to, float pre, float post, float pp, float qq, float w, float toT, float preT, float postT, float ppT, float qqT) {
  bool cTo = (toT == 0.000000);
  bool cPre = (preT == 0.000000);
  bool cPost = ((postT - toT) == 0.000000);
  bool cPp = ((preT - ppT) == 0.000000);
  bool cQq = ((qqT - postT) == 0.000000);
  float mFrom = (cTo ? 0.000000 : ((to - from) / toT));
  float mPre = (cPre ? 0.000000 : ((from - pre) / (-preT)));
  float mPost = (cPost ? 0.000000 : ((post - to) / (postT - toT)));
  if (cPre) {
    mPre = (cPost ? mFrom : ((2.000000 * mFrom) - mPost));
  }
  if (cPost) {
    mPost = (cPre ? mFrom : ((2.000000 * mFrom) - mPre));
  }
  float mPp = (cPp ? ((2.000000 * mPre) - mFrom) : ((pre - pp) / (preT - ppT)));
  float mQq = (cQq ? ((2.000000 * mPost) - mFrom) : ((qq - post) / (qqT - postT)));
  float tFrom = bk_makima_tan(mPp, mPre, mFrom, mPost);
  float tTo = bk_makima_tan(mPre, mFrom, mPost, mQq);
  float s2 = (w * w);
  float s3 = (s2 * w);
  return (((((((2.000000 * s3) - (3.000000 * s2)) + 1.000000) * from) + ((((s3 - (2.000000 * s2)) + w) * toT) * tFrom)) + (((-2.000000 * s3) + (3.000000 * s2)) * to)) + (((s3 - s2) * toT) * tTo));
}

float4 bk_qmul(float4 a, float4 b) {
  return float4(((((a.w * b.x) + (a.x * b.w)) + (a.y * b.z)) - (a.z * b.y)), ((((a.w * b.y) + (a.y * b.w)) + (a.z * b.x)) - (a.x * b.z)), ((((a.w * b.z) + (a.z * b.w)) + (a.x * b.y)) - (a.y * b.x)), ((((a.w * b.w) - (a.x * b.x)) - (a.y * b.y)) - (a.z * b.z)));
}

float4 bk_qinv(float4 a) {
  return float4((-a.x), (-a.y), (-a.z), a.w);
}

float3 bk_qlog(float4 a) {
  if ((abs(a.w) > (1.000000 - 0.00001f))) {
    return (a.xyz * 2.000000);
  }
  float r = (1.000000 / sqrt((1.000000 - (a.w * a.w))));
  return ((a.xyz * r) * (2.000000 * acos(a.w)));
}

float4 bk_qexp(float3 e) {
  float th = length(e);
  if ((th < 0.00001f)) {
    return float4(0.000000, 0.000000, 0.000000, 1.000000);
  }
  float3 ax = (e / th);
  return float4((ax * sin((th * 0.500000))), cos((th * 0.500000)));
}

float4 bk_slerp(float4 a, float4 b0, float w) {
  float4 b = b0;
  float c = dot(a, b);
  if ((c < 0.000000)) {
    c = (-c);
    b = (-b);
  }
  float s0 = (1.000000 - w);
  float s1 = w;
  if (((1.000000 - c) > 0.00001f)) {
    float om = acos(c);
    float si = sin(om);
    s0 = (sin(((1.000000 - w) * om)) / si);
    s1 = (sin((w * om)) / si);
  }
  float4 r = ((a * s0) + (b * s1));
  return (r / max(length(r), 1.0e-20f));
}

float3 bk_nearest_log(float4 a, float3 nb) {
  float phase = (2.000000 * 6.2831855f);
  float3 ax = a.xyz;
  float ang = (2.000000 * atan2(length(ax), a.w));
  bool axZero = (((abs(ax.x) < 0.00001f) && (abs(ax.y) < 0.00001f)) && (abs(ax.z) < 0.00001f));
  float nl = length(nb);
  if (axZero) {
    ax = ((nl == 0.000000) ? nb : (nb / nl));
  } else {
    ax = (ax / length(ax));
  }
  float ph = round(((dot(nb, ax) - ang) / phase));
  return (ax * (ang + (ph * phase)));
}

float4 bk_blend_log(float4 fq, float3 fromLn, float3 toLn0, float3 tif, float w) {
  float3 toLn = toLn0;
  float th = length(tif);
  float3 cr = cross(tif, toLn);
  toLn = ((toLn + tif) + (cr * 0.500000));
  if ((th >= 0.00001f)) {
    float c = (1.000000 / (th * th));
    if ((abs((th - 3.1415927f)) >= max((0.00001f * abs(th)), 0.00001f))) {
      c = (c - ((1.000000 + cos(th)) / ((2.000000 * th) * sin(th))));
    }
    toLn = (toLn + (cross(tif, cr) * c));
  }
  float3 bl = (fromLn + ((toLn - fromLn) * w));
  return bk_qmul(fq, bk_qexp(bl));
}

float4 bk_sq_cubic(float4 a, float4 b0, float4 pa0, float4 pb0, float w, float toT, float preT, float postT) {
  float4 pa = pa0;
  float4 b = b0;
  float4 pb = pb0;
  bool f_pa = (dot(a, pa) < 0.000000);
  if (f_pa) {
    pa = (-pa);
  }
  bool f_b = (dot(a, b) < 0.000000);
  if (f_b) {
    b = (-b);
  }
  bool f_pb = (f_b ? (dot(b, pb) <= 0.000000) : (dot(b, pb) < 0.000000));
  if (f_pb) {
    pb = (-pb);
  }
  float3 z0 = float3(0.000000, 0.000000, 0.000000);
  float3 aTo = bk_qlog(bk_qmul(bk_qinv(a), b));
  float3 aPre = bk_qlog(bk_qmul(bk_qinv(a), pa));
  float3 aPost = bk_qlog(bk_qmul(bk_qinv(a), pb));
  float3 fromLn = float3(bk_cubic(z0.x, aTo.x, aPre.x, aPost.x, w, toT, preT, postT), bk_cubic(z0.y, aTo.y, aPre.y, aPost.y, w, toT, preT, postT), bk_cubic(z0.z, aTo.z, aPre.z, aPost.z, w, toT, preT, postT));
  float3 bFrom = bk_qlog(bk_qmul(bk_qinv(b), a));
  float3 bPre = bk_qlog(bk_qmul(bk_qinv(b), pa));
  float3 bPost = bk_qlog(bk_qmul(bk_qinv(b), pb));
  float3 toLn = float3(bk_cubic(bFrom.x, z0.x, bPre.x, bPost.x, w, toT, preT, postT), bk_cubic(bFrom.y, z0.y, bPre.y, bPost.y, w, toT, preT, postT), bk_cubic(bFrom.z, z0.z, bPre.z, bPost.z, w, toT, preT, postT));
  return bk_blend_log(a, fromLn, toLn, aTo, w);
}

float4 bk_sq_makima(float4 a, float4 b0, float4 pa0, float4 pb0, float4 pp0, float4 qq0, float w, float toT, float preT, float postT, float ppT, float qqT) {
  float4 pa = pa0;
  float4 b = b0;
  float4 pb = pb0;
  float4 pp = pp0;
  float4 qq = qq0;
  bool f_pa = (dot(a, pa) < 0.000000);
  if (f_pa) {
    pa = (-pa);
  }
  bool f_pp = (f_pa ? (dot(pa, pp) <= 0.000000) : (dot(pa, pp) < 0.000000));
  if (f_pp) {
    pp = (-pp);
  }
  bool f_b = (dot(a, b) < 0.000000);
  if (f_b) {
    b = (-b);
  }
  bool f_pb = (f_b ? (dot(b, pb) <= 0.000000) : (dot(b, pb) < 0.000000));
  if (f_pb) {
    pb = (-pb);
  }
  bool f_qq = (f_pb ? (dot(pb, qq) <= 0.000000) : (dot(pb, qq) < 0.000000));
  if (f_qq) {
    qq = (-qq);
  }
  float3 z0 = float3(0.000000, 0.000000, 0.000000);
  float3 aTo = bk_qlog(bk_qmul(bk_qinv(a), b));
  float3 aPre = bk_qlog(bk_qmul(bk_qinv(a), pa));
  float3 aPost = bk_qlog(bk_qmul(bk_qinv(a), pb));
  float3 aPp = bk_nearest_log(bk_qmul(bk_qinv(a), pp), aPre);
  float3 aQq = bk_nearest_log(bk_qmul(bk_qinv(a), qq), aPost);
  float3 fromLn = float3(bk_makima(z0.x, aTo.x, aPre.x, aPost.x, aPp.x, aQq.x, w, toT, preT, postT, ppT, qqT), bk_makima(z0.y, aTo.y, aPre.y, aPost.y, aPp.y, aQq.y, w, toT, preT, postT, ppT, qqT), bk_makima(z0.z, aTo.z, aPre.z, aPost.z, aPp.z, aQq.z, w, toT, preT, postT, ppT, qqT));
  float3 bFrom = bk_qlog(bk_qmul(bk_qinv(b), a));
  float3 bPre = bk_qlog(bk_qmul(bk_qinv(b), pa));
  float3 bPost = bk_qlog(bk_qmul(bk_qinv(b), pb));
  float3 bPp = bk_nearest_log(bk_qmul(bk_qinv(b), pp), bPre);
  float3 bQq = bk_nearest_log(bk_qmul(bk_qinv(b), qq), bPost);
  float3 toLn = float3(bk_makima(bFrom.x, z0.x, bPre.x, bPost.x, bPp.x, bQq.x, w, toT, preT, postT, ppT, qqT), bk_makima(bFrom.y, z0.y, bPre.y, bPost.y, bPp.y, bQq.y, w, toT, preT, postT, ppT, qqT), bk_makima(bFrom.z, z0.z, bPre.z, bPost.z, bPp.z, bQq.z, w, toT, preT, postT, ppT, qqT));
  return bk_blend_log(a, fromLn, toLn, aTo, w);
}

[shader("compute")] [numthreads(64, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID) {
  uint b = tid.x;
  if ((b >= params.N)) {
    return;
  }
  bool det = (params.deterministic != 0u);
  float total = 0.000000;
  for (uint i = 0u; i < params.I; ++i) {
    if (((present[((i * params.N) + b)] & 8u) != 0u)) {
      total = (total + weight[((i * params.N) + b)]);
    }
  }
  if (((!det) && (abs(total) < 0.00001f))) {
    return;
  }
  uint ib = (b * 10u);
  float3 iLoc = float3(init[ib], init[(ib + 1u)], init[(ib + 2u)]);
  float4 iRot = float4(init[(ib + 3u)], init[((ib + 3u) + 1u)], init[((ib + 3u) + 2u)], init[((ib + 3u) + 3u)]);
  float3 iSc = float3(init[(ib + 7u)], init[((ib + 7u) + 1u)], init[((ib + 7u) + 2u)]);
  float4 iRotInv = bk_qinv(iRot);
  float3 loc = iLoc;
  float4 rot = iRot;
  float3 sc = iSc;
  float ms = motionScale[b];
  for (uint i = 0u; i < params.I; ++i) {
    uint k = ((i * params.N) + b);
    uint m = present[k];
    if (((m & 8u) != 0u)) {
      float bl = weight[k];
      if ((!det)) {
        bl = (bl / total);
      }
      if ((abs(bl) >= 0.00001f)) {
        uint o = (k * 10u);
        if (((m & 1u) != 0u)) {
          loc = (loc + (((float3(samp[o], samp[(o + 1u)], samp[(o + 2u)]) * ms) - iLoc) * bl));
        }
        if (((m & 2u) != 0u)) {
          float4 rel = bk_qmul(iRotInv, float4(samp[(o + 3u)], samp[((o + 3u) + 1u)], samp[((o + 3u) + 2u)], samp[((o + 3u) + 3u)]));
          float4 r = bk_qmul(rot, bk_slerp(float4(0.000000, 0.000000, 0.000000, 1.000000), rel, bl));
          rot = (r / max(length(r), 1.0e-20f));
        }
        if (((m & 4u) != 0u)) {
          sc = (sc + ((float3(samp[(o + 7u)], samp[((o + 7u) + 1u)], samp[((o + 7u) + 2u)]) - iSc) * bl));
        }
      }
    }
  }
  uint us = used[b];
  if (((us & 1u) != 0u)) {
    localPose[(ib + 0u)] = loc.x;
    localPose[(ib + 1u)] = loc.y;
    localPose[(ib + 2u)] = loc.z;
  }
  if (((us & 2u) != 0u)) {
    localPose[(ib + 3u)] = rot.x;
    localPose[(ib + 4u)] = rot.y;
    localPose[(ib + 5u)] = rot.z;
    localPose[(ib + 6u)] = rot.w;
  }
  if (((us & 4u) != 0u)) {
    localPose[(ib + 7u)] = sc.x;
    localPose[(ib + 8u)] = sc.y;
    localPose[(ib + 9u)] = sc.z;
  }
}
