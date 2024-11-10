struct PS_IN
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
};

Texture2D tex : register(t0);

SamplerState samplerState : register(s0);

PS_IN VSMain( uint id: SV_VERTEXID )
{
	PS_IN output = (PS_IN)0;
	
	output.uv = float2(id & 1, (id & 2) >> 1);
	output.pos = float4(output.uv * float2(2, -2) + float2(-1, 1), 0, 1);
	
	return output;
}

float4 PSMain( PS_IN input ) : SV_Target
{
    float3 pixPos = float3(input.uv, 0);
	float4 color = tex.Sample(samplerState, pixPos);
	return color;
}
