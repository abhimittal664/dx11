Texture2D inTexture;
SamplerState sso;

struct VS_OUTPUT
{
	float4 Position: SV_Position;
	float2 inTexCoord : TEXCOORD;
	float3 pixelNorm  : NORMAL;
};

float4 main(VS_OUTPUT IN) : SV_TARGET
{
	float3 pixelColor = inTexture.Sample(sso, IN.inTexCoord);
	return float4(pixelColor, 1.0f);
}