#include "globals.h"
#include<tuple>
using namespace std;


//To handle events and clicks from the window
LRESULT CALLBACK WndProc(
	HWND	hWnd,
	UINT	message,
	WPARAM	wParam,
	LPARAM	lParam
) {
	switch (message) {
	case WM_DESTROY: PostQuitMessage(0); break;
	default: return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

WNDCLASSEX windowSetup(HINSTANCE hInstance) {
	WNDCLASSEX wcex;

	//Set few window parameters
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = "MainWindow";
	wcex.hIconSm = NULL;

	//Exit if unsuccessful
	if (!RegisterClassEx(&wcex)) { exit(-1); }

	return wcex;

}
HWND createWindow(HINSTANCE hInstance) {

	//Create actual window
	HWND hwnd;

	RECT rc = { 0,0,800,600 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
		hwnd = CreateWindow(
		"MainWindow",
		"Window Title",
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZE,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rc.right - rc.left,
		rc.bottom - rc.top,
		nullptr,
		nullptr,
		nullptr,
		nullptr);

		return hwnd;
}

int INITD3D(HWND hwnd) {
	DXGI_SWAP_CHAIN_DESC swapChainDesc;

	//Select directx support level api
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0
	};

	//Set defauts to 0
	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));	

	//Compute back buffer size - Pick from window size
	swapChainDesc.BufferDesc.Width = 0;
	swapChainDesc.BufferDesc.Height = 0;

	//Set refresh rate. Zero means pick up from GPU
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;

	//Set Pixel format
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;

	//Scanline Format
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER::DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

	//Set scaling type - Useful for full screen apps using stretched window
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING::DXGI_MODE_SCALING_UNSPECIFIED;

	//Specify number of pixels for multisampling. 1 means no multisampling
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	//Misc
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	//Set number of back buffers
	swapChainDesc.BufferCount = 1;

	//Attach descriptor to window handle
	swapChainDesc.OutputWindow = hwnd;

	//Set window mode instead of full screen mode
	swapChainDesc.Windowed = TRUE;

	//Set back buffer swapping method- Discard the back buffer after it has been swapped
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	//No additional flags
	swapChainDesc.Flags = 0;

	//Create device and swap chain
	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DEBUG,
		featureLevels,
		ARRAYSIZE(featureLevels),
		D3D11_SDK_VERSION,
		&swapChainDesc,
		&swapChain,
		&device,
		&selectedFeatureLevel,
		&context

	);

	return 0;
}

//Create Render Target view here
std::tuple<ID3D11RenderTargetView*, ID3D11RasterizerState*> createRenderTargetView() {
	//Create texture buffer
	ID3D11Texture2D* textureBuffer;
	ID3D11RenderTargetView* renderTargetView;
	ID3D11RasterizerState* rasterizerState;

	//Bind it to swapchain
	swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&textureBuffer);

	//Create Render target view from device
	device->CreateRenderTargetView(textureBuffer, nullptr, &renderTargetView);
	textureBuffer->Release();
	//if (textureBuffer == NULL) exit(-1);

	//Set render target as backbuffer
	//context->OMSetRenderTargets(1, &renderTargetView, NULL);

	//Create Rasterizer state
	auto rasterizerDesc = CD3D11_RASTERIZER_DESC(D3D11_FILL_SOLID, D3D11_CULL_NONE, FALSE, 0, 0, 0, FALSE,
		FALSE, FALSE, FALSE);
	HRESULT hr = device->CreateRasterizerState(&rasterizerDesc, &rasterizerState);
	if (FAILED(hr)) exit(-11);

	return std::make_tuple(renderTargetView, rasterizerState);

}

//Update the back buffer
void updateBackBuffer(ID3D11RenderTargetView* renderTargetView) {
	//set render view
	context->OMSetRenderTargets(1, &renderTargetView, nullptr);

	//Set viewport
	auto viewport = CD3D11_VIEWPORT(0.f, 0.f, 800.f, 600.f);
	context->RSSetViewports(1, &viewport);

	float defaultColor[] = { 1.f, 1.f, 1.f, 1 };
	context->ClearRenderTargetView(renderTargetView, defaultColor);

}

//Draw by swapping buffers
void swapBuffers() {
	swapChain->Present(1, 0);
}

//Cleanup code
void cleanupDevice() {
	swapChain = 0;
	device = 0;
	context = 0;
	if (swapChain) swapChain->Release();
	if (device) device->Release();
	if (context) context->Release();
}
