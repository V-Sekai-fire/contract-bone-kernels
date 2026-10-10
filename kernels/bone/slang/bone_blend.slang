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

float2 bk_two_sum(float a, float b) {
  precise float s = (a + b);
  precise float bb = (s - a);
  precise float e = ((a - (s - bb)) + (b - bb));
  return float2(s, e);
}

float2 bk_split(float a) {
  precise float c = (4097.000000 * a);
  precise float hi = (c - (c - a));
  precise float lo = (a - hi);
  return float2(hi, lo);
}

float2 bk_two_prod(float a, float b) {
  precise float pr = (a * b);
  precise float2 sa = bk_split(a);
  precise float2 sb = bk_split(b);
  precise float e = (((((sa.x * sb.x) - pr) + (sa.x * sb.y)) + (sa.y * sb.x)) + (sa.y * sb.y));
  return float2(pr, e);
}

float2 bk_df_add(float2 a, float2 b) {
  precise float2 s = bk_two_sum(a.x, b.x);
  precise float e = ((s.y + a.y) + b.y);
  precise float h = (s.x + e);
  precise float l = (e - (h - s.x));
  return float2(h, l);
}

float2 bk_df_sub(float2 a, float2 b) {
  return bk_df_add(a, (-b));
}

float2 bk_df_mulf(float2 a, float b) {
  precise float2 pr = bk_two_prod(a.x, b);
  precise float e = (pr.y + (a.y * b));
  precise float h = (pr.x + e);
  precise float l = (e - (h - pr.x));
  return float2(h, l);
}

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

float bk_cubic(float from, float to, float pre, float post, float w, float2 toT, float2 preT, float2 postT) {
  precise float2 t = bk_df_mulf(toT, w);
  float fTo = (toT.x + toT.y);
  float fPre = (preT.x + preT.y);
  float fPost = (postT.x + postT.y);
  float dPostTo = (bk_df_sub(postT, toT).x + bk_df_sub(postT, toT).y);
  float dToPre = (bk_df_sub(toT, preT).x + bk_df_sub(toT, preT).y);
  float tPre = (bk_df_sub(t, preT).x + bk_df_sub(t, preT).y);
  float tTo = (bk_df_sub(t, toT).x + bk_df_sub(t, toT).y);
  float tt = (t.x + t.y);
  float a1 = (pre + (((fPre == 0.000000) ? 0.000000 : (tPre / (-fPre))) * (from - pre)));
  float a2 = (from + (((fTo == 0.000000) ? 0.500000 : (tt / fTo)) * (to - from)));
  float a3 = (to + (((dPostTo == 0.000000) ? 1.000000 : (tTo / dPostTo)) * (post - to)));
  float b1 = (a1 + (((dToPre == 0.000000) ? 0.000000 : (tPre / dToPre)) * (a2 - a1)));
  float b2 = (a2 + (((fPost == 0.000000) ? 1.000000 : (tt / fPost)) * (a3 - a2)));
  return (b1 + (((fTo == 0.000000) ? 0.500000 : (tt / fTo)) * (b2 - b1)));
}

float bk_makima_tan(float p0, float p1, float p2, float p3) {
  float w1 = (abs((p3 - p2)) + (abs((p3 + p2)) * 0.500000));
  float w2 = (abs((p1 - p0)) + (abs((p1 + p0)) * 0.500000));
  if (((w1 + w2) == 0.000000)) {
    return ((p1 + p2) * 0.500000);
  }
  return (((w1 * p1) + (w2 * p2)) / (w1 + w2));
}

float bk_makima(float from, float to, float pre, float post, float pp, float qq, float w, float2 toT, float2 preT, float2 postT, float2 ppT, float2 qqT) {
  float fTo = (toT.x + toT.y);
  float fPre = (preT.x + preT.y);
  float dPost = (bk_df_sub(postT, toT).x + bk_df_sub(postT, toT).y);
  float dPp = (bk_df_sub(preT, ppT).x + bk_df_sub(preT, ppT).y);
  float dQq = (bk_df_sub(qqT, postT).x + bk_df_sub(qqT, postT).y);
  bool cTo = (fTo == 0.000000);
  bool cPre = (fPre == 0.000000);
  bool cPost = (dPost == 0.000000);
  bool cPp = (dPp == 0.000000);
  bool cQq = (dQq == 0.000000);
  float mFrom = (cTo ? 0.000000 : ((to - from) / fTo));
  float mPre = (cPre ? 0.000000 : ((from - pre) / (-fPre)));
  float mPost = (cPost ? 0.000000 : ((post - to) / dPost));
  if (cPre) {
    mPre = (cPost ? mFrom : ((2.000000 * mFrom) - mPost));
  }
  if (cPost) {
    mPost = (cPre ? mFrom : ((2.000000 * mFrom) - mPre));
  }
  float mPp = (cPp ? ((2.000000 * mPre) - mFrom) : ((pre - pp) / dPp));
  float mQq = (cQq ? ((2.000000 * mPost) - mFrom) : ((qq - post) / dQq));
  float tFrom = bk_makima_tan(mPp, mPre, mFrom, mPost);
  float tTo = bk_makima_tan(mPre, mFrom, mPost, mQq);
  float s2 = (w * w);
  float s3 = (s2 * w);
  return (((((((2.000000 * s3) - (3.000000 * s2)) + 1.000000) * from) + ((((s3 - (2.000000 * s2)) + w) * fTo) * tFrom)) + (((-2.000000 * s3) + (3.000000 * s2)) * to)) + (((s3 - s2) * fTo) * tTo));
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

float4 bk_sq_cubic(float4 a, float4 b0, float4 pa0, float4 pb0, float w, float2 toT, float2 preT, float2 postT) {
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

float4 bk_sq_makima(float4 a, float4 b0, float4 pa0, float4 pb0, float4 pp0, float4 qq0, float w, float2 toT, float2 preT, float2 postT, float2 ppT, float2 qqT) {
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

float bk_fposmod(float x, float y) {
  float r = fmod(x, y);
  if ((((r < 0.000000) && (y > 0.000000)) || ((r > 0.000000) && (y < 0.000000)))) {
    r = (r + y);
  }
  return r;
}

float bk_angle_diff(float from, float to) {
  float d = fmod((to - from), 6.2831855f);
  return (fmod((2.000000 * d), 6.2831855f) - d);
}

float bk_cubic_angle(float from, float to, float pre, float post, float w, float2 toT, float2 preT, float2 postT) {
  float fr = fmod(from, 6.2831855f);
  float pr = ((fr + fmod((2.000000 * fmod((pre - fr), 6.2831855f)), 6.2831855f)) - fmod((pre - fr), 6.2831855f));
  float tr = ((fr + fmod((2.000000 * fmod((to - fr), 6.2831855f)), 6.2831855f)) - fmod((to - fr), 6.2831855f));
  float po = ((tr + fmod((2.000000 * fmod((post - tr), 6.2831855f)), 6.2831855f)) - fmod((post - tr), 6.2831855f));
  return bk_cubic(fr, tr, pr, po, w, toT, preT, postT);
}

float bk_makima_angle(float from, float to, float pre, float post, float pp, float qq, float w, float2 toT, float2 preT, float2 postT, float2 ppT, float2 qqT) {
  float fr = fmod(from, 6.2831855f);
  float pr = ((fr + fmod((2.000000 * fmod((pre - fr), 6.2831855f)), 6.2831855f)) - fmod((pre - fr), 6.2831855f));
  float ppr = ((pr + fmod((2.000000 * fmod((pp - pr), 6.2831855f)), 6.2831855f)) - fmod((pp - pr), 6.2831855f));
  float tr = ((fr + fmod((2.000000 * fmod((to - fr), 6.2831855f)), 6.2831855f)) - fmod((to - fr), 6.2831855f));
  float po = ((tr + fmod((2.000000 * fmod((post - tr), 6.2831855f)), 6.2831855f)) - fmod((post - tr), 6.2831855f));
  float qqr = ((po + fmod((2.000000 * fmod((qq - po), 6.2831855f)), 6.2831855f)) - fmod((qq - po), 6.2831855f));
  return bk_makima(fr, tr, pr, po, ppr, qqr, w, toT, preT, postT, ppT, qqT);
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
