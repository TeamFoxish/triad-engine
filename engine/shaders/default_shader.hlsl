struct VS_IN
{
	float4 pos : POSITION0;
};

struct PS_IN
{
	float4 pos : SV_POSITION;
};

cbuffer PixelConstantBuffer : register(b0)
{
    float4 color;
}

cbuffer VertexConstantBuffer : register(b0)
{
    float4 offset;
    float4 size;
}

PS_IN VSMain( VS_IN input )
{
	PS_IN output = (PS_IN)0;
	
	output.pos = input.pos * float4(size.xy, 1, 1) + float4(offset.xy, 0, 0);
	
	return output;
}

float4 PSMain( PS_IN input ) : SV_Target
{
	return color;
}
