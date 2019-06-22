struct input {
	float4 position: SV_POSITION;
	float3 color: COLOR;
};

float4 main(input verticesIn): SV_TARGET {
float4 verticesOut;
verticesOut		= float4(verticesIn.color.r/2.f, verticesIn.color.g/2.f, verticesIn.color.b/2.f, 1.0f);
//verticesOut.position = float4(verticesIn.position.x, verticesIn.position.y, verticesIn.position.z, 1.0f);
return verticesOut;

}
