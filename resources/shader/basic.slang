struct VSOutput {
    float4 pos : SV_Position;
  };

[shader("vertex")]
VSOutput main(float3 aPos : POSITION)
{
  VSOutput output;
    output.pos = float4(aPos, 1.0);
  return output;
}
