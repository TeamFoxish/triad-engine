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
	float4 worldPos : POSITION;
	float2 uv : TEXCOORD;
};

struct PS_OUTPUT
{
    float4 color: SV_Target0;
    uint entityId: SV_Target1;
};

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
	};
	#define NR_POINT_LIGHTS 4  
	PointLight pointLights[NR_POINT_LIGHTS];
	float4 uCameraPos;
	float4 uAmbientLight;
	int spotLightsNum;
	int isTextureSet;
	uint entityId;
}

cbuffer MaterialCB : register(b1)
{
	float4 color;
    float uSpecPower;
	float uShininess;
}

cbuffer VertexConstantBuffer : register(b0)
{
    float4x4 worldTransform;
	float4x4 viewProj;
}

Texture2D tex : register(t0);
SamplerState samplerState : register(s0);

float4 CalcPointLight(PointLight light, float3 texVal, float3 normal, float3 fragPos, float3 viewDir)
{
    float3 lightDir = normalize(light.position.xyz - fragPos);
    // диффузное освещение
    float3 diff = max(dot(normal, lightDir), 0.0);
    // освещение зеркальных бликов
    float3 reflectDir = reflect(-lightDir, normal);
    float spec = uSpecPower * pow(max(dot(viewDir, reflectDir), 0.0), uShininess);
    // затухание
    float distance = length(light.position.xyz - fragPos);
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
	
	output.worldPos = mul(float4(input.pos, 1.0), worldTransform);
	output.pos = mul(float4(input.pos, 1.0), mul(worldTransform, viewProj));
	output.normal = mul(float4(input.normal, 0.0), worldTransform);
	output.uv = input.uv;
	
	return output;
}

PS_OUTPUT PSMain( PS_IN input ) : SV_Target
{
	// Surface normal
	float3 N = normalize(input.normal.xyz);
	// Vector from surface to light
	float3 L = normalize(-dirLight.mDirection.xyz);
	// Vector from surface to camera
	float3 V = normalize(uCameraPos.xyz - input.worldPos.xyz);
	// Reflection of -L about N
	float3 R = normalize(reflect(-L, N));

	// Compute phong reflection
	float3 Phong = uAmbientLight.xyz;
	float NdotL = dot(N, L);
	if (NdotL > 0)
	{
		float3 Diffuse = dirLight.mDiffuseColor.xyz * NdotL;
		float3 Specular = uSpecPower * dirLight.mSpecColor.xyz * pow(max(0.0, dot(R, V)), uShininess);
		Phong += Diffuse + Specular;
	}
	
	PS_OUTPUT pix;
	pix.color = color;
	float4 texVal = 0;
	if (isTextureSet == 1) {
		pix.color = texVal = tex.Sample(samplerState, input.uv);
	}
	
	// Final color is texture color times phong light (alpha = 1)
	pix.color *= float4(Phong, 1.0f);

	int spotLightsNumClipped = min(spotLightsNum, NR_POINT_LIGHTS);
	for (int i = 0; i < spotLightsNum; i++) {
		pix.color += CalcPointLight(pointLights[i], texVal, N, input.worldPos.xyz, V); 
	}

	pix.entityId = entityId;

    return pix;
}
