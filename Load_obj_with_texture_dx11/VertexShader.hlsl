struct VS_INPUT
{
	float4 Position: POSITION;
	float2 inTexCoord : TEXCOORD;
};

struct VS_OUTPUT
{
	float4 Position: SV_Position;
	float2 outTexCoord : TEXCOORD;
	float3 vertexNormal : NORMAL;
};

cbuffer mvp {
	float4x4 mvp_matrices;
};

VS_OUTPUT main(VS_INPUT IN)
{
	VS_OUTPUT OUT = (VS_OUTPUT)0;

	OUT.Position = mul(IN.Position, mvp_matrices) ;
	OUT.outTexCoord = IN.inTexCoord;

	return OUT;
}