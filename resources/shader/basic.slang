struct VSOutput {
    float4 pos : SV_Position;
    float3 color : COLOR;
  };

[shader("vertex")]
VSOutput mainVertex(float3 aPos : POSITION)
{
  VSOutput output;
  output.pos = float4(aPos, 1.0);
  output.color = float3(1.0, 1.0, 1.0);
  return output;
}

uniform float4 u_color;
[shader("pixel")]
float4 mainPixel(VSOutput out) : SV_Target
{
  return float4(u_color);
}
