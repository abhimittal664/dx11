#include "globals.h"
#include <fstream>
#include <vector>
#include <tuple>
using namespace std;

ID3D11Buffer* createVertices(struct vertex* vertices, ID3D11Buffer* vertexBufferObject) {

	vertices[1] = { -1.f, -1.f, 0.f, 1.f, 0.f, 0.f };
	vertices[0] = { 1.f, -1.f, 0.f, 0.f, 1.f, 0.f };
	vertices[2] = { 0.f, 1.f, 0.f, 0.f, 0.f, 1.f };


	//Create Vertex buffer Descriptor
	auto vertexBufferDesc = CD3D11_BUFFER_DESC(sizeof(vertex)*3, D3D11_BIND_VERTEX_BUFFER);
	D3D11_SUBRESOURCE_DATA vertexData = { 0 };
	vertexData.pSysMem = vertices;

	//Create vertex buffer object
	HRESULT hr = device->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBufferObject);
	if (FAILED(hr)) exit(-10);
	return vertexBufferObject;
}

//Functions for loading vertex and pixel shaders
std::tuple<ID3D11VertexShader*, ID3D11InputLayout*> loadVertexShader() {
	ID3D11VertexShader* vertexShader;
	ID3D11InputLayout* inputLayout;

	//Define buffer layout
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 3*sizeof(float), D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	ifstream vertexShaderFile("vertexShader.cso", ios::binary);

	vector<char> vsData = { istreambuf_iterator<char>(vertexShaderFile), istreambuf_iterator<char>() };

	device->CreateVertexShader(vsData.data(), vsData.size(), nullptr, &vertexShader);
	
	device->CreateInputLayout(layout, ARRAYSIZE(layout), vsData.data(), vsData.size(), &inputLayout);

	//Bind layout
	context->IASetInputLayout(inputLayout);

	//Create Shader
	context->VSSetShader(vertexShader, nullptr, 0);

	return std::make_tuple(vertexShader, inputLayout);
}

//Functions for loading vertex and pixel shaders
ID3D11PixelShader* loadPixelShader() {
	ID3D11PixelShader* pixelShader;

	ifstream pixelShaderFile("pixelShader.cso", ios::binary);

	vector<char> psData = { istreambuf_iterator<char>(pixelShaderFile), istreambuf_iterator<char>() };

	device->CreatePixelShader(psData.data(), psData.size(), nullptr, &pixelShader);

	//Create Shader
	context->PSSetShader(pixelShader, nullptr, 0);

	return pixelShader;
}

void prepareGeometry(ID3D11VertexShader* vertexShader, ID3D11PixelShader* pixelShader, ID3D11InputLayout* inputLayout, ID3D11Buffer* vertexBufferObject, struct vertex* vertices) {
	//Define vertices
	UINT stride;
	UINT offset;

	//Bind vertex buffer object
	stride = sizeof(vertex);
	offset = 0;
	context->IASetVertexBuffers(0, 1, &vertexBufferObject, &stride, &offset);

}