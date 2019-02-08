#pragma once
#include <Windows.h>
#include <d3d11_1.h>

//Store swapchain and device here
extern IDXGISwapChain* swapChain;
extern ID3D11Device* device;
extern ID3D11DeviceContext* context;

//Which dx api from hw or sw was selected by device
extern D3D_FEATURE_LEVEL selectedFeatureLevel;

//Define vertex structure
typedef struct vertex {
	float x, y, z;
	float r, g, b;
} vertex;