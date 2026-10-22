
struct VSInput
{
    float3 aPos : POSITION;
    uint instanceID : SV_InstanceID;
};

struct VSOutput
{
    float4 pos   : SV_Position;
    float3 color : COLOR;
};

struct MaterialData
{
    float4x4 proj;    // Offset 0   (64 bytes)
    float4x4 view;    // Offset 64  (64 bytes)
    float4 u_color;   // Offset 128 (16 bytes)
    float4 u_color_trigger; // Offset 144 (16 bytes)
};                    // Total Size = 160 bytes

struct InstData
{
  float4x4 transform;
  uint4 flags;
};

[vk::binding(0, 0)]
ConstantBuffer<MaterialData> material;

// Instance SSBO at Binding Point 2
[vk::binding(2, 0)]
StructuredBuffer<InstData> instanceTransforms;

[shader("vertex")]
VSOutput mainVertex(VSInput input)
{
    VSOutput output;

    float4x4 modelMatrix = instanceTransforms[input.instanceID].transform;
    float4 worldPos = mul(modelMatrix, float4(input.aPos, 1.0));

    output.pos = mul(material.proj, mul(material.view, worldPos));
    bool isTrigger = (instanceTransforms[input.instanceID].flags.x != 0u);
    output.color = isTrigger ? material.u_color_trigger.rgb : material.u_color.rgb;

    return output;
}

[shader("pixel")]
float4 mainPixel(VSOutput pin) : SV_Target
{
    return float4(pin.color, material.u_color.a);
}
