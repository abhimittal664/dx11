struct VS_INPUT
{
	float4 Position: POSITION;
	float2 inTexCoord : TEXCOORD;
	float3 vertexNormal : NORMAL;
};

struct VS_OUTPUT
{
	float4 Position: SV_Position;
	float2 outTexCoord : TEXCOORD;
	float3 vertexNormal : NORMAL;
};

//Wave constants
struct t_wave_consts {
	float		k;
	float		w;
	float		amp;	//Amplitude
	float		phase;	//Starting phase
	float4	direction;	//Direction of the wave
} ;

cbuffer mvp : register(cb[0]) {
	float4x4 mvp_matrices;
};

cbuffer waveData : register(cb[1]){
	float4 wave_const[2];
};

cbuffer scaledTime : register(cb[2]){
	float4 sTime;
}

float4 sine_func(float4 pos : POSITION){
	float4 curr_pos;
	float4 pos1, pos2;
	float k_x = (mul(wave_const[0].x, pos.x));
	float k_x2 = mul(mul(wave_const[1].x, 0.3f), pos.z);
	float w_t = (mul(wave_const[0].y, sTime.x));
	float w_t2 = mul(mul(wave_const[1].y, 3.f), sTime.x);
	pos1.y = mul(wave_const[0].z, sin(k_x - w_t));
	pos2.y = mul(wave_const[1].z, sin(k_x2 - w_t2));
	curr_pos = float4(pos.x, pos1.y + pos2.y, pos.z, 1.f);
	return curr_pos;
}

VS_OUTPUT main(VS_INPUT IN)
{
	VS_OUTPUT OUT = (VS_OUTPUT)0;
	float4 pos = sine_func((IN.Position));
	OUT.Position = mul(normalize(pos), mvp_matrices) ;
	OUT.outTexCoord = (IN.inTexCoord);
	OUT.vertexNormal =(IN.vertexNormal);

	return OUT;
}

