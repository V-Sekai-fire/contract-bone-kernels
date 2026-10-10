struct BoneSampleParams {
  uint T;
  float time;
  float length;
  uint loopMode;
  uint backward;
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
StructuredBuffer<float> keyTrans;
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

int bk_fetch_idx(int p, int len, bool wrap, uint mode) {
  if ((!wrap)) {
    return clamp(p, 0, (len - 1));
  }
  int idx = (p % len);
  if ((idx < 0)) {
    idx = (idx + len);
  }
  int cyc = ((p - idx) / len);
  if (((mode == 1u) || ((cyc % 2) == 0))) {
    return idx;
  }
  return ((len - 1) - idx);
}

float bk_fetch_time(int p, int len, bool wrap, uint mode, uint off, float L) {
  if ((!wrap)) {
    return keyTime[(off + uint(clamp(p, 0, (len - 1))))];
  }
  int idx = (p % len);
  if ((idx < 0)) {
    idx = (idx + len);
  }
  int cyc = ((p - idx) / len);
  if (((mode == 1u) || ((cyc % 2) == 0))) {
    return ((float(cyc) * L) + keyTime[(off + uint(idx))]);
  }
  return ((float((cyc + 1)) * L) - keyTime[(off + uint(((len - 1) - idx)))]);
}

[shader("compute")] [numthreads(64, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID) {
  uint i = tid.x;
  if ((i >= params.T)) {
    return;
  }
  uint off = trackKeys[(i * 2u)];
  int len = int(trackKeys[((i * 2u) + 1u)]);
  uint dst = trackDst[i];
  uint flags = trackKind[i];
  uint kind = (flags & 1u);
  uint interp = ((flags >> 1u) & 7u);
  bool wrap = ((((flags >> 4u) & 1u) != 0u) && (params.loopMode != 0u));
  bool back = ((params.backward != 0u) && (interp == 0u));
  float t = params.time;
  int k0 = 0;
  float4 vk0 = float4(keyVal[(((off + uint(k0)) * 4u) + 0u)], keyVal[(((off + uint(k0)) * 4u) + 1u)], keyVal[(((off + uint(k0)) * 4u) + 2u)], keyVal[(((off + uint(k0)) * 4u) + 3u)]);
  if ((len == 1)) {
    float4 res = vk0;
    if ((kind == 0u)) {
      localPose[(dst + 0u)] = res.x;
      localPose[(dst + 1u)] = res.y;
      localPose[(dst + 2u)] = res.z;
    } else {
      localPose[(dst + 0u)] = res.x;
      localPose[(dst + 1u)] = res.y;
      localPose[(dst + 2u)] = res.z;
      localPose[(dst + 3u)] = res.w;
    }
    return;
  }
  int low = 0;
  int high = (len - 1);
  int mid = 0;
  bool found = false;
  while (((low <= high) && (!found))) {
    mid = ((low + high) / 2);
    float km = keyTime[(off + uint(mid))];
    float tol = max((0.00001f * abs(t)), 0.00001f);
    if (((t == km) || (abs((t - km)) < tol))) {
      found = true;
    } else {
      if ((t < km)) {
        high = (mid - 1);
      } else {
        low = (mid + 1);
      }
    }
  }
  if ((!found)) {
    if ((!back)) {
      if ((keyTime[(off + uint(mid))] > t)) {
        mid = (mid - 1);
      }
    } else {
      if ((keyTime[(off + uint(mid))] < t)) {
        mid = (mid + 1);
      }
    }
  }
  int base = (back ? min(mid, len) : min(mid, (len - 1)));
  int pm2 = (back ? (base - (-2)) : (base + (-2)));
  int pm1 = (back ? (base - (-1)) : (base + (-1)));
  int pn0 = (back ? (base - 0) : (base + 0));
  int pn1 = (back ? (base - 1) : (base + 1));
  int pn2 = (back ? (base - 2) : (base + 2));
  int pn3 = (back ? (base - 3) : (base + 3));
  int km2 = bk_fetch_idx(pm2, len, wrap, params.loopMode);
  int km1 = bk_fetch_idx(pm1, len, wrap, params.loopMode);
  int kn0 = bk_fetch_idx(pn0, len, wrap, params.loopMode);
  int kn1 = bk_fetch_idx(pn1, len, wrap, params.loopMode);
  int kn2 = bk_fetch_idx(pn2, len, wrap, params.loopMode);
  int kn3 = bk_fetch_idx(pn3, len, wrap, params.loopMode);
  float tm2 = bk_fetch_time(pm2, len, wrap, params.loopMode, off, params.length);
  float tm1 = bk_fetch_time(pm1, len, wrap, params.loopMode, off, params.length);
  float tn0 = bk_fetch_time(pn0, len, wrap, params.loopMode, off, params.length);
  float tn1 = bk_fetch_time(pn1, len, wrap, params.loopMode, off, params.length);
  float tn2 = bk_fetch_time(pn2, len, wrap, params.loopMode, off, params.length);
  float tn3 = bk_fetch_time(pn3, len, wrap, params.loopMode, off, params.length);
  float4 vm2 = float4(keyVal[(((off + uint(km2)) * 4u) + 0u)], keyVal[(((off + uint(km2)) * 4u) + 1u)], keyVal[(((off + uint(km2)) * 4u) + 2u)], keyVal[(((off + uint(km2)) * 4u) + 3u)]);
  float4 vm1 = float4(keyVal[(((off + uint(km1)) * 4u) + 0u)], keyVal[(((off + uint(km1)) * 4u) + 1u)], keyVal[(((off + uint(km1)) * 4u) + 2u)], keyVal[(((off + uint(km1)) * 4u) + 3u)]);
  float4 vn0 = float4(keyVal[(((off + uint(kn0)) * 4u) + 0u)], keyVal[(((off + uint(kn0)) * 4u) + 1u)], keyVal[(((off + uint(kn0)) * 4u) + 2u)], keyVal[(((off + uint(kn0)) * 4u) + 3u)]);
  float4 vn1 = float4(keyVal[(((off + uint(kn1)) * 4u) + 0u)], keyVal[(((off + uint(kn1)) * 4u) + 1u)], keyVal[(((off + uint(kn1)) * 4u) + 2u)], keyVal[(((off + uint(kn1)) * 4u) + 3u)]);
  float4 vn2 = float4(keyVal[(((off + uint(kn2)) * 4u) + 0u)], keyVal[(((off + uint(kn2)) * 4u) + 1u)], keyVal[(((off + uint(kn2)) * 4u) + 2u)], keyVal[(((off + uint(kn2)) * 4u) + 3u)]);
  float4 vn3 = float4(keyVal[(((off + uint(kn3)) * 4u) + 0u)], keyVal[(((off + uint(kn3)) * 4u) + 1u)], keyVal[(((off + uint(kn3)) * 4u) + 2u)], keyVal[(((off + uint(kn3)) * 4u) + 3u)]);
  float tr = keyTrans[(off + uint(kn0))];
  if ((tr == 0.000000)) {
    float4 res = vn0;
    if ((kind == 0u)) {
      localPose[(dst + 0u)] = res.x;
      localPose[(dst + 1u)] = res.y;
      localPose[(dst + 2u)] = res.z;
    } else {
      localPose[(dst + 0u)] = res.x;
      localPose[(dst + 1u)] = res.y;
      localPose[(dst + 2u)] = res.z;
      localPose[(dst + 3u)] = res.w;
    }
    return;
  }
  float dlt = (tn1 - tn0);
  float w = ((abs(dlt) < 0.00001f) ? 0.000000 : ((t - tn0) / dlt));
  if ((tr != 1.000000)) {
    w = bk_ease(w, tr);
  }
  if ((interp == 0u)) {
    float4 res = vn0;
    if ((kind == 0u)) {
      localPose[(dst + 0u)] = res.x;
      localPose[(dst + 1u)] = res.y;
      localPose[(dst + 2u)] = res.z;
    } else {
      localPose[(dst + 0u)] = res.x;
      localPose[(dst + 1u)] = res.y;
      localPose[(dst + 2u)] = res.z;
      localPose[(dst + 3u)] = res.w;
    }
    return;
  }
  if (((interp == 1u) || (interp == 3u))) {
    if ((kind == 0u)) {
      float4 res = (vn0 + ((vn1 - vn0) * w));
      if ((kind == 0u)) {
        localPose[(dst + 0u)] = res.x;
        localPose[(dst + 1u)] = res.y;
        localPose[(dst + 2u)] = res.z;
      } else {
        localPose[(dst + 0u)] = res.x;
        localPose[(dst + 1u)] = res.y;
        localPose[(dst + 2u)] = res.z;
        localPose[(dst + 3u)] = res.w;
      }
      return;
    } else {
      float4 res = bk_slerp(vn0, vn1, w);
      if ((kind == 0u)) {
        localPose[(dst + 0u)] = res.x;
        localPose[(dst + 1u)] = res.y;
        localPose[(dst + 2u)] = res.z;
      } else {
        localPose[(dst + 0u)] = res.x;
        localPose[(dst + 1u)] = res.y;
        localPose[(dst + 2u)] = res.z;
        localPose[(dst + 3u)] = res.w;
      }
      return;
    }
  }
  if (((interp == 2u) || (interp == 4u))) {
    if ((kind == 0u)) {
      float4 res = float4(bk_cubic(vn0.x, vn1.x, vm1.x, vn2.x, w, (tn1 - tn0), (tm1 - tn0), (tn2 - tn0)), bk_cubic(vn0.y, vn1.y, vm1.y, vn2.y, w, (tn1 - tn0), (tm1 - tn0), (tn2 - tn0)), bk_cubic(vn0.z, vn1.z, vm1.z, vn2.z, w, (tn1 - tn0), (tm1 - tn0), (tn2 - tn0)), 0.000000);
      if ((kind == 0u)) {
        localPose[(dst + 0u)] = res.x;
        localPose[(dst + 1u)] = res.y;
        localPose[(dst + 2u)] = res.z;
      } else {
        localPose[(dst + 0u)] = res.x;
        localPose[(dst + 1u)] = res.y;
        localPose[(dst + 2u)] = res.z;
        localPose[(dst + 3u)] = res.w;
      }
      return;
    } else {
      float4 res = bk_sq_cubic(vn0, vn1, vm1, vn2, w, (tn1 - tn0), (tm1 - tn0), (tn2 - tn0));
      if ((kind == 0u)) {
        localPose[(dst + 0u)] = res.x;
        localPose[(dst + 1u)] = res.y;
        localPose[(dst + 2u)] = res.z;
      } else {
        localPose[(dst + 0u)] = res.x;
        localPose[(dst + 1u)] = res.y;
        localPose[(dst + 2u)] = res.z;
        localPose[(dst + 3u)] = res.w;
      }
      return;
    }
  }
  if ((kind == 0u)) {
    float4 res = float4(bk_makima(vn0.x, vn1.x, vm1.x, vn2.x, vm2.x, vn3.x, w, (tn1 - tn0), (tm1 - tn0), (tn2 - tn0), (tm2 - tn0), (tn3 - tn0)), bk_makima(vn0.y, vn1.y, vm1.y, vn2.y, vm2.y, vn3.y, w, (tn1 - tn0), (tm1 - tn0), (tn2 - tn0), (tm2 - tn0), (tn3 - tn0)), bk_makima(vn0.z, vn1.z, vm1.z, vn2.z, vm2.z, vn3.z, w, (tn1 - tn0), (tm1 - tn0), (tn2 - tn0), (tm2 - tn0), (tn3 - tn0)), 0.000000);
    if ((kind == 0u)) {
      localPose[(dst + 0u)] = res.x;
      localPose[(dst + 1u)] = res.y;
      localPose[(dst + 2u)] = res.z;
    } else {
      localPose[(dst + 0u)] = res.x;
      localPose[(dst + 1u)] = res.y;
      localPose[(dst + 2u)] = res.z;
      localPose[(dst + 3u)] = res.w;
    }
    return;
  } else {
    float4 res = bk_sq_makima(vn0, vn1, vm1, vn2, vm2, vn3, w, (tn1 - tn0), (tm1 - tn0), (tn2 - tn0), (tm2 - tn0), (tn3 - tn0));
    if ((kind == 0u)) {
      localPose[(dst + 0u)] = res.x;
      localPose[(dst + 1u)] = res.y;
      localPose[(dst + 2u)] = res.z;
    } else {
      localPose[(dst + 0u)] = res.x;
      localPose[(dst + 1u)] = res.y;
      localPose[(dst + 2u)] = res.z;
      localPose[(dst + 3u)] = res.w;
    }
    return;
  }
}
