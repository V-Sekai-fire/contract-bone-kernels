struct BoneSkinParams {
  uint B;
};

[[vk::binding(0, 0)]]
ConstantBuffer<BoneSkinParams> params;
[[vk::binding(1, 0)]]
StructuredBuffer<uint> bindBone;
[[vk::binding(2, 0)]]
StructuredBuffer<float> bindPose;
[[vk::binding(3, 0)]]
StructuredBuffer<float> globalPose;
[[vk::binding(4, 0)]]
RWStructuredBuffer<float> skin;

[shader("compute")] [numthreads(64, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID) {
  uint i = tid.x;
  if ((i >= params.B)) {
    return;
  }
  uint gb = (bindBone[i] * 12u);
  uint bb = (i * 12u);
  float G0 = globalPose[(gb + 0u)];
  float G1 = globalPose[(gb + 1u)];
  float G2 = globalPose[(gb + 2u)];
  float G3 = globalPose[(gb + 4u)];
  float G4 = globalPose[(gb + 5u)];
  float G5 = globalPose[(gb + 6u)];
  float G6 = globalPose[(gb + 8u)];
  float G7 = globalPose[(gb + 9u)];
  float G8 = globalPose[(gb + 10u)];
  float g0 = globalPose[(gb + 3u)];
  float g1 = globalPose[(gb + 7u)];
  float g2 = globalPose[(gb + 11u)];
  float K0 = bindPose[(bb + 0u)];
  float K1 = bindPose[(bb + 1u)];
  float K2 = bindPose[(bb + 2u)];
  float K3 = bindPose[(bb + 4u)];
  float K4 = bindPose[(bb + 5u)];
  float K5 = bindPose[(bb + 6u)];
  float K6 = bindPose[(bb + 8u)];
  float K7 = bindPose[(bb + 9u)];
  float K8 = bindPose[(bb + 10u)];
  float k0 = bindPose[(bb + 3u)];
  float k1 = bindPose[(bb + 7u)];
  float k2 = bindPose[(bb + 11u)];
  float C0 = (((G0 * K0) + (G1 * K3)) + (G2 * K6));
  float C1 = (((G0 * K1) + (G1 * K4)) + (G2 * K7));
  float C2 = (((G0 * K2) + (G1 * K5)) + (G2 * K8));
  float C3 = (((G3 * K0) + (G4 * K3)) + (G5 * K6));
  float C4 = (((G3 * K1) + (G4 * K4)) + (G5 * K7));
  float C5 = (((G3 * K2) + (G4 * K5)) + (G5 * K8));
  float C6 = (((G6 * K0) + (G7 * K3)) + (G8 * K6));
  float C7 = (((G6 * K1) + (G7 * K4)) + (G8 * K7));
  float C8 = (((G6 * K2) + (G7 * K5)) + (G8 * K8));
  float Gk0 = (((G0 * k0) + (G1 * k1)) + (G2 * k2));
  float Gk1 = (((G3 * k0) + (G4 * k1)) + (G5 * k2));
  float Gk2 = (((G6 * k0) + (G7 * k1)) + (G8 * k2));
  float c0 = (Gk0 + g0);
  float c1 = (Gk1 + g1);
  float c2 = (Gk2 + g2);
  skin[(bb + 0u)] = C0;
  skin[(bb + 1u)] = C1;
  skin[(bb + 2u)] = C2;
  skin[(bb + 4u)] = C3;
  skin[(bb + 5u)] = C4;
  skin[(bb + 6u)] = C5;
  skin[(bb + 8u)] = C6;
  skin[(bb + 9u)] = C7;
  skin[(bb + 10u)] = C8;
  skin[(bb + 3u)] = c0;
  skin[(bb + 7u)] = c1;
  skin[(bb + 11u)] = c2;
}
