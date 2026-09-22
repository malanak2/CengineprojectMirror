struct VSOutput {
    float4 pos : SV_Position;
    float3 color : COLOR;
    float3 normal: NORMAL;
    float2 uv    : TEXCOORD0;
};

struct Data {
  uniform float4x4 proj;
  uniform float4x4 view;
}

[vk::binding(0,0)]
ConstantBuffer<Data> data;

[vk::binding(1,0)]
uniform Sampler2D texture;

[vk::binding(2,0)]
StructuredBuffer<float4x4> instanceTransforms;

[shader("vertex")]
VSOutput mainVertex(float3 aPos : POSITION, float3 aNormal : NORMAL, float2 aUV : TEXCOORD0, uint instanceID : SV_InstanceID)
{
  VSOutput output;
  float4x4 modelMatrix = instanceTransforms[instanceID];
  output.pos = mul(data.proj, mul(data.view, mul(modelMatrix, float4(aPos, 1.0))));
  output.color = float3(1.0, 1.0, 1.0);
  output.normal = aNormal;
  output.uv = aUV;
  return output;
}

[shader("pixel")]
float4 mainPixel(VSOutput out) : SV_Target
{
    float4 texColor = texture.Sample(out.uv);
    if (texColor.a < 0.05f) {
      discard;
    }
    return texColor * float4(out.color, 1.0);
}
