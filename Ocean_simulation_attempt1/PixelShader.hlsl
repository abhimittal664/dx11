Texture2D inTexture[3];
SamplerState sso;

typedef struct {
	float4 ambientLightColor;
	float4 pointLightColor;
	float4 pointLightLoc;
} t_LightStruct;

typedef struct {
	float4 kAmbient;
	float4 kDiffuse;
	float4 kSpecular;
	float4 ExpSpecular;
	int uses_ka; //Texcoords are used only if material is mapped
	int uses_kd;	
	int uses_ks;
	int dummy;
} t_objectMaterial;

//Add constant buffer for per pixel lighting data
cbuffer light : register(b0){
	t_LightStruct lightData;
};

//Add constant buffer for object material properties
cbuffer objectMaterial : register(b1){
	t_objectMaterial objectMaterialData;
};

struct VS_OUTPUT
{
	float4 Position: SV_Position;
	float2 inTexCoord : TEXCOORD;
	float3 pixelNorm  : NORMAL;
};

float4 main(VS_OUTPUT IN) : SV_TARGET
{
	//Sample only if enabled
	float3 ambientBaseColor, diffuseBaseColor, specularBaseColor;
	//Assign defaults as black
	ambientBaseColor = float3(1.f, 1.f, 1.f);
	diffuseBaseColor = float3(1.f, 1.f, 1.f);
	specularBaseColor = float3(1.f, 1.f, 1.f);

	//Ambient
	if (objectMaterialData.uses_ka){
		ambientBaseColor = inTexture[0].Sample(sso, IN.inTexCoord);
	}

	//Diffuse
	if (objectMaterialData.uses_kd){
		diffuseBaseColor = inTexture[1].Sample(sso, IN.inTexCoord);
	}

	//Specular
	if (objectMaterialData.uses_ks){
		specularBaseColor = inTexture[2].Sample(sso, IN.inTexCoord);
	}
		
	//Calculate ambient light color
	float3 ambientColor = ambientBaseColor.xyz * lightData.ambientLightColor.xyz * objectMaterialData.kAmbient.xyz;

		//Calcualte diffused color I_d = K_d * I * (L dot N)N
		float3 lightVec = lightData.pointLightLoc.xyz - normalize(IN.Position.xyz); //First calculate L vector
		float LdotN = dot(lightVec.xyz, IN.pixelNorm.xyz); //Calculate the dot product
		float3 diffuseColor;
		if (LdotN > 0){
			diffuseColor = diffuseBaseColor.xyz * lightData.pointLightColor.xyz * objectMaterialData.kDiffuse.xyz * LdotN;
		}
		else {
			diffuseColor = diffuseBaseColor.xyz;
		}

		//Calculate Specular color

		//Calculate final Color
		float3 finalColor = diffuseBaseColor;// +ambientColor;

	return float4(1.f,1.f,1.f, 1.0f);
}