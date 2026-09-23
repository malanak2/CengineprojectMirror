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

static const int MAX_BONES = 500;
static const int MAX_BONE_INFLUENCE = 4;

[vk::binding(3, 0)]
StructuredBuffer<float4x4> finalBonesMatrices;

[shader("vertex")]
VSOutput mainVertex(float3 aPos : POSITION,
  float3 aNormal                : NORMAL,
  float2 aUV                    : TEXCOORD0,
  int4   boneIds                : BLENDINDICES,
  float4 weights                : BLENDWEIGHT,
  uint   instanceID             : SV_InstanceID)
{
    VSOutput output;

    float4 totalPosition = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float3 totalNormal   = float3(0.0f, 0.0f, 0.0f);
    float  totalWeight   = 0.0f;

    for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
    {
        int boneId = boneIds[i];

        if (boneId < 0)
            continue;

        if (boneId >= MAX_BONES)
        {
            totalPosition = float4(aPos, 1.0f);
            totalNormal   = aNormal;
            totalWeight   = 1.0f;
            break;
        }

        float w = weights[i];
        float4x4 boneMat = finalBonesMatrices[boneId];

        totalPosition += mul(boneMat, float4(aPos, 1.0f)) * w;
        totalNormal   += mul((float3x3)boneMat, aNormal) * w;

        totalWeight += w;
    }

    if (totalWeight <= 0.0f)
    {
        totalPosition = float4(aPos, 1.0f);
        totalNormal   = aNormal;
    }

    float4x4 modelMatrix = instanceTransforms[instanceID];
    float4 worldPos = mul(modelMatrix, totalPosition);

    output.pos    = mul(data.proj, mul(data.view, worldPos));
    output.normal = normalize(mul((float3x3)modelMatrix, totalNormal));
    output.color  = float3(1.0, 1.0, 1.0);
    output.uv     = aUV;

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
