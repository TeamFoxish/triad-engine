struct VS_IN
{
	float3 pos : POSITION;
	float4 color : COLOR;
};

struct PS_IN
{
	float4 pos : SV_POSITION;
    float4 color : COLOR;
};

struct PS_OUTPUT
{
    float4 color: SV_Target0;
};

cbuffer VertexConstantBuffer : register(b0)
{
    float4x4 worldTransform;
	float4x4 viewProj;
}

PS_IN VSMain( VS_IN input )
{
	PS_IN output = (PS_IN)0;
	
	output.pos = mul(float4(input.pos, 1.0), viewProj);
    output.color = input.color;
	
	return output;
}

PS_OUTPUT PSMain( PS_IN input ) : SV_Target
{
    PS_OUTPUT output;
    output.color = input.color;
	return output;
}
