struct PS_IN
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
};

#define NR_CASCADES 4
cbuffer PixelConstantBuffer : register(b0)
{
	struct DirectionalLight
	{
		float4 mDirection;
		float4 mDiffuseColor;
		float4 mSpecColor;
	} dirLight;
    
    float4x4 inverseViewMatr;
	float4x4 viewMatr;
	float4 uCameraPos;
	float4 uAmbientLight;
	//float cascadePlaneDistances[NR_CASCADES]; doesn't work for whatever reason
	float4 cascadePlaneDistances;
	float uShininess; // TODO: remove from lighting
}

/*
cbuffer Cascades : register(b2) 
{
	float4x4 lightSpaceMatrices[NR_CASCADES];
}
*/

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
Texture2D lightAccTex : register(t3);
//Texture2DArray shadowMap : register(t4);

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
	// Everything is in view space.
    float4 eyePos = { 0, 0, 0, 1 };
 
    int2 texCoord = input.pos.xy;
    float depth = depthStencilTex.Load( int3( texCoord, 0 ) ).r;
 
	// point in view space
    float4 P = ScreenToView( float4( texCoord, depth, 1.0f ) );
	float4 worldPos = mul(P, inverseViewMatr);
	float3 pixPos = float3(input.uv, 0);
	float4 normal = normalsTex.Sample(samplerState, pixPos);
	float4 albedoSpec = albedoSpecTex.Sample(samplerState, pixPos);
	float4 lightAcc = lightAccTex.Sample(samplerState, pixPos);
	
	/*
	// select cascade layer
	float4 fragPosViewSpace = mul(float4(worldPos.xyz, 1.0), viewMatr);
	float depthValue = abs(fragPosViewSpace.z);
	//return float4(depthValue / 100.0, depthValue / 100.0, depthValue / 100.0, 1.0);
	
	int layer = 4;
	for (int i = 0; i < NR_CASCADES; ++i)
	{
		if (depthValue < cascadePlaneDistances[i])
		{
			layer = i;
			break;
		}
	}
	*/

	// Surface normal
	float3 N = normalize(normal.xyz);
	// Vector from surface to light
	float3 L = normalize(-dirLight.mDirection.xyz);
	// Vector from surface to camera
	float3 V = normalize(uCameraPos.xyz - worldPos.xyz);
	// Reflection of -L about N
	float3 R = normalize(reflect(-L, N));

	/*
	float4 fragPosLightSpace = mul(float4(worldPos.xyz, 1.0), lightSpaceMatrices[layer]);
	// perform perspective divide
	float3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	// transform to [0,1] range
	projCoords.xy = projCoords.xy * 0.5 + 0.5;
	//projCoords = projCoords * 0.5 + 0.5;
	projCoords.y = 1.0 - projCoords.y;
		
	// PCF
	float shadow = 0.0;
	// get depth of current fragment from light's perspective
	float currentDepth = projCoords.z;
	//return float4(currentDepth, currentDepth, currentDepth, 1.0);
	//return float4(currentDepth, currentDepth, currentDepth, 1.0);
	// calculate bias (based on depth map resolution and slope)
	float bias = max(0.05 * (1.0 - dot(N, L)), 0.005);
	if (layer == NR_CASCADES)
	{
		float farPlane = 100.0;
		bias *= 1 / (farPlane * 0.5f);
	}
	else
	{
		bias *= 1 / (cascadePlaneDistances[layer] * 0.5f);
	}

	
	//float2 texelSize = 1.0 / float2(textureSize(shadowMap, 0));
	float2 texelSize = 1.0 / float2(2000.0, 2000.0);
	//return float4(projCoords.xz, 0.0, 1.0);
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = shadowMap.Sample(samplerState,
						float3(projCoords.xy + float2(x, y) * texelSize,
						layer)
						).r;
			shadow += (currentDepth - bias) > pcfDepth ? 1.0 : 0.0;        
		}    
	}
	shadow /= 9.0;
	//return float4(shadow, shadow, shadow, 1.0);
		
	// keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
	if(projCoords.z > 1.0)
	{
		shadow = 0.0;
	}
	
	//return float4((float)layer / 5.0, (float)layer / 5.0, (float)layer / 5.0, 1.0);
	*/

	// Compute phong reflection
	float3 Phong = uAmbientLight.xyz;
	float NdotL = dot(N, L);
	if (NdotL > 0)
	{
		float3 Diffuse = albedoSpec.rgb * dirLight.mDiffuseColor.xyz * NdotL;
		float3 Specular = albedoSpec.w * dirLight.mSpecColor.xyz * pow(max(0.0, dot(R, V)), uShininess);
		float shadow = 0.0; // temp
		Phong += (1.0 - shadow) * (Diffuse + Specular);
	}
	
	float4 col = float4(Phong, 1.0);
	
	// Final color is texture color times phong light (alpha = 1)
	//col *= float4(Phong, 1.0f);
	col += lightAcc;

	return col;
}
