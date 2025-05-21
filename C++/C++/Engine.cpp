#include "Engine.h"

// Basic vertex structure
struct Vertex {
    XMFLOAT3 Position;
    XMFLOAT4 Color;
};

struct Light {
    XMFLOAT4 Position;
    XMFLOAT4 Color;
};

// Basic vertex shader
const char* vertexShaderSource = R"(
struct VS_INPUT {
    float3 Position : POSITION;
    float4 Color : COLOR;
};

struct VS_OUTPUT {
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
};

VS_OUTPUT main(VS_INPUT input) {
    VS_OUTPUT output;
    output.Position = float4(input.Position, 1.0f);
    output.Color = input.Color;
    return output;
})";

// Basic pixel shader
const char* pixelShaderSource = R"(
struct PS_INPUT {
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
};

float4 main(PS_INPUT input) : SV_TARGET {
    return input.Color;
})";

Engine::Engine(HWND hwnd) : hwnd_(hwnd) {
    lastTick = GetTickCount64();

    // Get window dimensions
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    width_ = clientRect.right - clientRect.left;
    height_ = clientRect.bottom - clientRect.top;
}

Engine::~Engine() {
    // DirectX will automatically clean up ComPtr resources
}

bool Engine::Initialize() {
    if (!InitializeDirectX()) return false;
    if (!CreateShaders()) return false;
    if (!CreateGeometry()) return false;
    return true;
}

bool Engine::InitializeDirectX() {
    // Create device and swap chain
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Width = width_;
    swapChainDesc.BufferDesc.Height = height_;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = hwnd_;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.Windowed = TRUE;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };
    UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    D3D_FEATURE_LEVEL featureLevel;
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags,
        featureLevels, numFeatureLevels, D3D11_SDK_VERSION,
        &swapChainDesc, swapChain_.GetAddressOf(), device_.GetAddressOf(),
        &featureLevel, context_.GetAddressOf());

    if (FAILED(hr)) {
        MessageBox(hwnd_, L"D3D11CreateDeviceAndSwapChain failed!", L"Error", MB_OK);
        return false;
    }

    // Create render target view
    ComPtr<ID3D11Texture2D> backBuffer;
    hr = swapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));
    if (FAILED(hr)) {
        MessageBox(hwnd_, L"SwapChain GetBuffer failed!", L"Error", MB_OK);
        return false;
    }

    hr = device_->CreateRenderTargetView(backBuffer.Get(), nullptr, renderTargetView_.GetAddressOf());
    if (FAILED(hr)) {
        MessageBox(hwnd_, L"CreateRenderTargetView failed!", L"Error", MB_OK);
        return false;
    }

    // Set render target and viewport
    context_->OMSetRenderTargets(1, renderTargetView_.GetAddressOf(), nullptr);

    D3D11_VIEWPORT viewport = {};
    viewport.Width = static_cast<float>(width_);
    viewport.Height = static_cast<float>(height_);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    context_->RSSetViewports(1, &viewport);

    return true;
}

bool Engine::CreateShaders() {
    // Compile vertex shader
    ComPtr<ID3DBlob> vsBlob;
    ComPtr<ID3DBlob> errorBlob;
    HRESULT hr = D3DCompile(vertexShaderSource, strlen(vertexShaderSource), "VS", nullptr, nullptr, "main",
        "vs_4_0", 0, 0, vsBlob.GetAddressOf(), errorBlob.GetAddressOf());
    if (FAILED(hr)) {
        if (errorBlob) {
            MessageBoxA(hwnd_, (char*)errorBlob->GetBufferPointer(), "Vertex Shader Compilation Error", MB_OK);
        }
        return false;
    }

    // Create vertex shader
    hr = device_->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, vertexShader_.GetAddressOf());
    if (FAILED(hr)) {
        MessageBox(hwnd_, L"CreateVertexShader failed!", L"Error", MB_OK);
        return false;
    }

    // Define input layout
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    UINT numElements = ARRAYSIZE(layout);

    // Create input layout
    hr = device_->CreateInputLayout(layout, numElements, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), inputLayout_.GetAddressOf());
    if (FAILED(hr)) {
        MessageBox(hwnd_, L"CreateInputLayout failed!", L"Error", MB_OK);
        return false;
    }

    // Compile pixel shader
    ComPtr<ID3DBlob> psBlob;
    hr = D3DCompile(pixelShaderSource, strlen(pixelShaderSource), "PS", nullptr, nullptr, "main",
        "ps_4_0", 0, 0, psBlob.GetAddressOf(), errorBlob.GetAddressOf());
    if (FAILED(hr)) {
        if (errorBlob) {
            MessageBoxA(hwnd_, (char*)errorBlob->GetBufferPointer(), "Pixel Shader Compilation Error", MB_OK);
        }
        return false;
    }

    // Create pixel shader
    hr = device_->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, pixelShader_.GetAddressOf());
    if (FAILED(hr)) {
        MessageBox(hwnd_, L"CreatePixelShader failed!", L"Error", MB_OK);
        return false;
    }

    return true;
}

bool Engine::CreateGeometry() {
    // Create a triangle (initially at 0,0)
    Vertex vertices[] = {
        { XMFLOAT3(-0.1f, 0.1f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
        { XMFLOAT3(0.1f, 0.1f, 0.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
        { XMFLOAT3(0.0f, -0.1f, 0.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) }
    };

    // Create vertex buffer
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.ByteWidth = sizeof(Vertex) * 3;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = vertices;

    HRESULT hr = device_->CreateBuffer(&bufferDesc, &initData, vertexBuffer_.GetAddressOf());
    if (FAILED(hr)) {
        MessageBox(hwnd_, L"CreateBuffer failed!", L"Error", MB_OK);
        return false;
    }

    return true;
}

void Engine::Update() {
    // Calculate delta time
    ULONGLONG currentTick = GetTickCount64();
    float deltaTime = (currentTick - lastTick) / 1000.0f;
    lastTick = currentTick;

    // Update position (normalized from 0 to 2 for screen width)
    xPos += speed * deltaTime / width_ * 2.0f;
    if (xPos > 2.0f) xPos = -0.2f; // Reset when it goes off screen

    // Update vertex buffer with new position
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT hr = context_->Map(vertexBuffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (SUCCEEDED(hr)) {
        Vertex* vertices = reinterpret_cast<Vertex*>(mappedResource.pData);

        // Transform triangle to current position (-1 to 1 NDC coordinates)
        vertices[0] = { XMFLOAT3(xPos - 1.0f, 0.1f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) };
        vertices[1] = { XMFLOAT3(xPos - 0.8f, 0.1f, 0.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) };
        vertices[2] = { XMFLOAT3(xPos - 0.9f, -0.1f, 0.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) };

        context_->Unmap(vertexBuffer_.Get(), 0);
    }

    // Force a repaint
    InvalidateRect(hwnd_, NULL, FALSE);
}

void Engine::Render() {
    // Clear the render target
    float clearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
    context_->ClearRenderTargetView(renderTargetView_.Get(), clearColor);

    // Set up the rendering pipeline
    context_->IASetInputLayout(inputLayout_.Get());
    context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    context_->IASetVertexBuffers(0, 1, vertexBuffer_.GetAddressOf(), &stride, &offset);

    // Set shaders
    context_->VSSetShader(vertexShader_.Get(), nullptr, 0);
    context_->PSSetShader(pixelShader_.Get(), nullptr, 0);

    // Draw the triangle
    context_->Draw(3, 0);

    // Present the frame
    swapChain_->Present(1, 0);
}

