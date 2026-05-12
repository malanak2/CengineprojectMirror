struct VSOutput {
    float4 pos : SV_Position;
    float3 color : COLOR;
  };

struct Data {
  uniform float4x4 proj;
  uniform float4x4 view;
  uniform float4x4 translate;
  uniform float4 u_color;
}

[vk::binding(0,0)]
ConstantBuffer<Data> data;
[shader("vertex")]
VSOutput mainVertex(float3 aPos : POSITION, uint vertexID : SV_VertexID)
{
  VSOutput output;
  output.pos =mul(data.proj, mul(data.view, mul(data.translate, float4(aPos, 1.0))));
  output.color = float3(1.0, 1.0, 1.0);
  return output;
}

[shader("pixel")]
float4 mainPixel(VSOutput out) : SV_Target
{

  return float4(data.u_color);
}
