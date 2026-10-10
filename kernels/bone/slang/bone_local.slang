struct BoneLocalParams {
  uint N;
};

[[vk::binding(0, 0)]]
ConstantBuffer<BoneLocalParams> params;
[[vk::binding(1, 0)]]
StructuredBuffer<uint> parent;
[[vk::binding(2, 0)]]
StructuredBuffer<float> localPose;
[[vk::binding(3, 0)]]
RWStructuredBuffer<float> acc;
[[vk::binding(4, 0)]]
RWStructuredBuffer<uint> anc;

[shader("compute")] [numthreads(64, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID) {
  uint b = tid.x;
  if ((b >= params.N)) {
    return;
  }
  uint lb = (b * 10u);
  float qx = localPose[(lb + 3u)];
  float qy = localPose[(lb + 4u)];
  float qz = localPose[(lb + 5u)];
  float qw = localPose[(lb + 6u)];
  float qd = ((((qx * qx) + (qy * qy)) + (qz * qz)) + (qw * qw));
  float qs = (2.000000 / max(qd, 1.0e-20f));
  float xs = (qx * qs);
  float ys = (qy * qs);
  float zs = (qz * qs);
  float wx = (qw * xs);
  float wy = (qw * ys);
  float wz = (qw * zs);
  float xx = (qx * xs);
  float xy = (qx * ys);
  float xz = (qx * zs);
  float yy = (qy * ys);
  float yz = (qy * zs);
  float zz = (qz * zs);
  float S0 = localPose[(lb + 7u)];
  float S1 = localPose[(lb + 8u)];
  float S2 = localPose[(lb + 9u)];
  float lp0 = localPose[(lb + 0u)];
  float lp1 = localPose[(lb + 1u)];
  float lp2 = localPose[(lb + 2u)];
  float R0 = (1.000000 - (yy + zz));
  float R1 = (xy - wz);
  float R2 = (xz + wy);
  float R3 = (xy + wz);
  float R4 = (1.000000 - (xx + zz));
  float R5 = (yz - wx);
  float R6 = (xz - wy);
  float R7 = (yz + wx);
  float R8 = (1.000000 - (xx + yy));
  float M0 = (R0 * S0);
  float M1 = (R1 * S1);
  float M2 = (R2 * S2);
  float M3 = (R3 * S0);
  float M4 = (R4 * S1);
  float M5 = (R5 * S2);
  float M6 = (R6 * S0);
  float M7 = (R7 * S1);
  float M8 = (R8 * S2);
  acc[((b * 12u) + 0u)] = M0;
  acc[((b * 12u) + 1u)] = M1;
  acc[((b * 12u) + 2u)] = M2;
  acc[((b * 12u) + 4u)] = M3;
  acc[((b * 12u) + 5u)] = M4;
  acc[((b * 12u) + 6u)] = M5;
  acc[((b * 12u) + 8u)] = M6;
  acc[((b * 12u) + 9u)] = M7;
  acc[((b * 12u) + 10u)] = M8;
  acc[((b * 12u) + 3u)] = lp0;
  acc[((b * 12u) + 7u)] = lp1;
  acc[((b * 12u) + 11u)] = lp2;
  anc[b] = parent[b];
}
