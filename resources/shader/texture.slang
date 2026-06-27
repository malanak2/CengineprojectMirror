struct VSOutput {
    float4 pos : SV_Position;
    float3 color : COLOR;
    float2 uv    : TEXCOORD0;
  };

struct Data {
  uniform float4x4 proj;
  uniform float4x4 view;
  uniform float4x4 translate;
  uniform Sampler2D texture;
}

[vk::binding(0,0)]
ConstantBuffer<Data> data;
[shader("vertex")]
VSOutput mainVertex(float3 aPos : POSITION,float2 aUV : TEXCOORD0)
{
  VSOutput output;
  output.pos =mul(data.proj, mul(data.view, mul(data.translate, float4(aPos, 1.0))));
  output.color = float3(1.0, 1.0, 1.0);
  output.uv = aUV;
  return output;
}

[shader("pixel")]
float4 mainPixel(VSOutput out) : SV_Target
{
    float4 texColor = data.texture.Sample(out.uv);
    return texColor * float4(out.color, 1.0);
}
