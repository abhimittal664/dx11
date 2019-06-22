#pragma once
#include <Windows.h>
#include <d3d11_1.h>
#include <vector>
#include <tuple>
using namespace std;

void prepareGeometry(ID3D11VertexShader*, ID3D11PixelShader*, ID3D11InputLayout*, ID3D11Buffer*, struct vertex*);
std::tuple<ID3D11VertexShader*, ID3D11InputLayout*>  loadVertexShader();
ID3D11PixelShader* loadPixelShader();
ID3D11Buffer* createVertices(struct vertex*, ID3D11Buffer*);
