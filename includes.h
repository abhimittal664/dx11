#pragma once
#include <Windows.h>
#include <d3d11_1.h>
#include <tuple>
using namespace std;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
int INITD3D(HWND);
void cleanupDevice();
int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int);
HWND createWindow(HINSTANCE);
WNDCLASSEX windowSetup(HINSTANCE);
std::tuple<ID3D11RenderTargetView*, ID3D11RasterizerState*>  createRenderTargetView();
void swapBuffers();
void updateBackBuffer(ID3D11RenderTargetView*);