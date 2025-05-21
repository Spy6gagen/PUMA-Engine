#pragma once
#include <windows.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <wrl/client.h>

// Link the DirectX libraries
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

using Microsoft::WRL::ComPtr;
using namespace DirectX;

class Engine {
public:
    Engine(HWND hwnd);
    ~Engine();
    void Update();
    void Render();
    bool Initialize();

private:
    // Window handle
    HWND hwnd_;

    // Game state variables
    float xPos = 0.0f;
    float speed = 100.0f; // pixels per second
    ULONGLONG lastTick;

    // DirectX objects
    ComPtr<ID3D11Device> device_;
    ComPtr<ID3D11DeviceContext> context_;
    ComPtr<IDXGISwapChain> swapChain_;
    ComPtr<ID3D11RenderTargetView> renderTargetView_;

    // Shaders and related objects
    ComPtr<ID3D11VertexShader> vertexShader_;
    ComPtr<ID3D11PixelShader> pixelShader_;
    ComPtr<ID3D11InputLayout> inputLayout_;
    ComPtr<ID3D11Buffer> vertexBuffer_;

    // Window dimensions
    UINT width_ = 800;
    UINT height_ = 600;

    // DirectX initialization functions
    bool InitializeDirectX();
    bool CreateShaders();
    bool CreateGeometry();
};