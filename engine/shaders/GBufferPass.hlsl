struct VS_IN
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD;
};

struct PS_IN
{
	float4 pos : SV_POSITION;
	float4 normal : NORMAL;
	float2 uv : TEXCOORD;
};

struct PS_OUTPUT
{
    float4 albedoSpec: SV_Target0;
    float3 normal: SV_Target1;
    float4 lightAcc: SV_Target2;
	uint entityId: SV_Target3;
};

cbuffer VertexConstantBuffer : register(b0)
{
    float4x4 worldTransform;
	float4x4 viewProj;
}

cbuffer PixelConstantBuffer : register(b0)
{
	float4 ambientColor;
	int isTextureSet;
	uint entityId;
}

cbuffer MaterialCB : register(b1)
{
	float4 color;
    float uSpecPower;
	float uShininess;
}

Texture2D tex : register(t0);
SamplerState samplerState : register(s0);

PS_IN VSMain( VS_IN input )
{
	PS_IN output = (PS_IN)0;
	
	output.pos = mul(float4(input.pos, 1.0), mul(worldTransform, viewProj));
	output.normal = mul(float4(input.normal, 0.0), worldTransform); // TODO: apply invert matrix to fix normals direction
	output.uv = input.uv;
	
	return output;
}

PS_OUTPUT PSMain( PS_IN input ) : SV_Target
{
    PS_OUTPUT output;

	float3 N = normalize(input.normal.xyz);

    output.normal = N;
    output.albedoSpec = isTextureSet * tex.Sample(samplerState, input.uv) + !isTextureSet * float4(color.xyz, 0.0);
    output.albedoSpec.a = uSpecPower;
	output.lightAcc = float4(output.albedoSpec.rgb * ambientColor.rgb, 1.0f);
	output.entityId = entityId;

    return output;
}
