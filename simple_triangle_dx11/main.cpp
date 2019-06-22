#include "globals.h"
#include "includes.h"
#include "draw_utils.h"

//The main window display function
int WINAPI wWinMain(
	HINSTANCE	hInstance,
	HINSTANCE	hPreviousInstance,
	LPWSTR		lpCmdLine,
	int			nCmdShow
) {
	WNDCLASSEX wcex;
	HWND hwnd;
	ID3D11RenderTargetView* renderTargetView;	//For texture back buffer
	ID3D11VertexShader* vertexShader;
	ID3D11PixelShader* pixelShader ;
	ID3D11InputLayout* inputLayout;
	ID3D11Buffer* vertexBufferObject = nullptr;
	ID3D11RasterizerState* rasterizerState;

	struct vertex vertices[3];

	//Setup Window
	wcex = windowSetup(hInstance);

	//Create window
	hwnd = createWindow(hInstance);

	//Call init function
	(void)INITD3D(hwnd);

	//Create RenderTarget View
	std::tie(renderTargetView, rasterizerState) = createRenderTargetView();

	//Create Shaders
	std::tie(vertexShader, inputLayout) = loadVertexShader();
	pixelShader = loadPixelShader();

	//Create vertices
	vertexBufferObject = createVertices(vertices, vertexBufferObject);
	if (vertexBufferObject == nullptr) exit(-8);

	//Show actual window
	ShowWindow(hwnd, nCmdShow);

	//Main window message loop
	MSG msg = { 0 };
	while (WM_QUIT != msg.message) {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			//Main loop
			//Update back buffer
			context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
			updateBackBuffer(renderTargetView);

			//Draw to back buffer
			if (vertexShader == nullptr || inputLayout == nullptr || pixelShader == nullptr) exit(-7);
			prepareGeometry(vertexShader, pixelShader, inputLayout, vertexBufferObject, vertices);
			context->Draw(3, 0);

			//Swapping back buffer
			swapBuffers();

		}
	}
	//Cleanup device resources
	cleanupDevice();
	return 0;
}

