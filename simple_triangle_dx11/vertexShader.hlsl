struct input {
	float3 position: POSITION;
	float3 color: COLOR;
};

struct output {
	float4 position: SV_POSITION;
	float3 color: COLOR;
};

output main(input verticesIn) {
	struct output verticesOut;
	verticesOut.position = float4(verticesIn.position.x, verticesIn.position.y, verticesIn.position.z, 1.f );
	verticesOut.color = float3( verticesIn.color.r, verticesIn.color.g, verticesIn.color.b);

	return verticesOut;
}
