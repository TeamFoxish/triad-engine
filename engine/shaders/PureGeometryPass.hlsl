struct VS_IN
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD;
};

struct PS_IN
{
	float4 pos : SV_POSITION;
};

cbuffer VertexConstantBuffer : register(b0)
{
    float4x4 worldTransform;
	float4x4 viewProj;
}

PS_IN VSMain( VS_IN input )
{
	PS_IN output = (PS_IN)0;
	
	output.pos = mul(float4(input.pos, 1.0), mul(worldTransform, viewProj));
	
	return output;
}
