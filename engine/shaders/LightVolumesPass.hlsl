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

cbuffer PixelConstantBuffer : register(b0)
{
	struct DirectionalLight
	{
		float4 mDirection;
		float4 mDiffuseColor;
		float4 mSpecColor;
	} dirLight;
	struct PointLight {
    	float4 position;
    	float4 diffuse;
    	float4 specular;
		float constant;
    	float lin;
    	float quadratic;
		float _dummy;
	} pointLight;
	float4x4 viewMatr;
	float uShininess; // TODO: remove?
}

cbuffer ScreenToViewParams : register(b1)
{
    float4x4 InverseProjection;
    float2 ScreenDimensions;
}

// Convert clip space coordinates to view space
float4 ClipToView( float4 clip )
{
    // View space position.
    float4 view = mul( clip, InverseProjection );
    // Perspective projection.
    view = view / view.w;
 
    return view;
}
 
// Convert screen space coordinates to view space.
float4 ScreenToView( float4 screen )
{
    // Convert to normalized texture coordinates
    float2 texCoord = screen.xy / ScreenDimensions;
 
    // Convert to clip space
    float4 clip = float4( float2( texCoord.x, 1.0f - texCoord.y ) * 2.0f - 1.0f, screen.z, screen.w );
 
    return ClipToView( clip );
}

Texture2D albedoSpecTex : register(t0);
Texture2D normalsTex : register(t1);
Texture2D depthStencilTex : register(t2);

SamplerState samplerState : register(s0);

float4 CalcPointLight(PointLight light, float3 texVal, float3 normal, float3 fragPos, float3 viewDir, float specPower)
{
	float4 lightPosViewSpace = mul(float4(light.position.xyz, 1.0f), viewMatr);
    float3 lightDir = normalize(lightPosViewSpace.xyz - fragPos);
	normal = mul(float4(normal, 0.0f), viewMatr).xyz;
    // диффузное освещение
    float3 diff = max(dot(normal, lightDir), 0.0);
    // освещение зеркальных бликов
    float3 reflectDir = reflect(-lightDir, normal);
    float spec = specPower * pow(max(dot(viewDir, reflectDir), 0.0), uShininess);
    // затухание
    float distance = length(lightPosViewSpace.xyz - fragPos);
    float attenuation = 1.0 / (light.constant + light.lin * distance + 
  			     light.quadratic * (distance * distance));
    // комбинируем результаты
    float3 diffuse = light.diffuse.xyz * diff * texVal;
    float3 specular = light.specular.xyz * spec * texVal;
    diffuse  *= attenuation;
    specular *= attenuation;
    return float4(diffuse + specular, 1.0);
} 

PS_IN VSMain( VS_IN input )
{
	PS_IN output = (PS_IN)0;
	
	output.pos = mul(float4(input.pos, 1.0), mul(worldTransform, viewProj));

    return output;
}

float4 PSMain( PS_IN input ) : SV_Target
{
	// Everything is in view space.
    float4 eyePos = { 0, 0, 0, 1 };
 
    int2 texCoord = input.pos.xy;
    float depth = depthStencilTex.Load( int3( texCoord, 0 ) ).r;
 
	// point in view space
    float4 P = ScreenToView( float4( texCoord, depth, 1.0f ) );

	float4 normal = normalsTex.Load(int3( texCoord, 0 ));
	float4 albedoSpec = albedoSpecTex.Load(int3( texCoord, 0 ));

	// Surface normal
	float3 N = normalize(normal.xyz);
	// Vector from surface to light
	float3 L = normalize(-dirLight.mDirection.xyz);
	// Vector from surface to camera
	float3 V = normalize(eyePos.xyz - P.xyz);
	// Reflection of -L about N
	float3 R = normalize(reflect(-L, N));

	return CalcPointLight(pointLight, float4(albedoSpec.xyz, 1.0f), N, P.xyz, V, albedoSpec.w);
}
