#include "Prerequisites.h"
#include <sstream>
#include <fstream>
#include "BaseApp.h"
//
// main.cpp
//
// An example application based off of Microsoft's Direct3D 11 tutorial
// series (Tutorial 07).  This version demonstrates how to load an
// arbitrary OBJ model and render it using a simple metal/roughness
// physically based shading (PBR) approach.  Textures for albedo,
// metallic, roughness, ambient occlusion and normals are loaded
// using stb_image (implemented in stb_image.cpp using WIC).  The
// OBJ loader comes from the public domain project OBJ?Loader.
//
// Build settings for this project should follow the guidelines
// provided by the assignment, with output directories pointing to
// $(SolutionDir)bin/$(PlatformShortName) and intermediate files
// stored under $(SolutionDir)intermediate/...
//


#include "OBJ_Loader.h"
#include "stb_image.h"


// Vertex structure with position, normal, tangent, bitangent and UV
struct Vertex
{
  XMFLOAT3 Position;
  XMFLOAT3 Normal;
  XMFLOAT3 Tangent;
  XMFLOAT3 Bitangent;
  XMFLOAT2 Tex;
};

// Constant buffer used in the vertex and pixel shaders.  Align to
// 16?bytes as required by Direct3D constant buffers.
struct CBMain
{
  XMFLOAT4X4 World;
  XMFLOAT4X4 View;
  XMFLOAT4X4 Projection;
  XMFLOAT3   CameraPos;
  float      pad0;
  XMFLOAT3   LightDir;
  float      pad1;
  XMFLOAT3   LightColor;
  float      pad2;
};

// Helper function to compute tangents and bitangents for a mesh
static void ComputeTangents(const std::vector<Vertex>& inVerts,
  const std::vector<unsigned int>& indices,
  std::vector<Vertex>& outVerts)
{
  outVerts = inVerts;
  // Zero out tangents and bitangents
  for (auto& v : outVerts)
  {
    v.Tangent = XMFLOAT3(0, 0, 0);
    v.Bitangent = XMFLOAT3(0, 0, 0);
  }
  // Loop over each triangle and accumulate tangents/bitangents
  for (size_t i = 0; i < indices.size(); i += 3)
  {
    Vertex& v0 = outVerts[indices[i]];
    Vertex& v1 = outVerts[indices[i + 1]];
    Vertex& v2 = outVerts[indices[i + 2]];

    XMVECTOR p0 = XMLoadFloat3(&v0.Position);
    XMVECTOR p1 = XMLoadFloat3(&v1.Position);
    XMVECTOR p2 = XMLoadFloat3(&v2.Position);
    XMVECTOR uv0 = XMLoadFloat2(&v0.Tex);
    XMVECTOR uv1 = XMLoadFloat2(&v1.Tex);
    XMVECTOR uv2 = XMLoadFloat2(&v2.Tex);

    XMVECTOR edge1 = XMVectorSubtract(p1, p0);
    XMVECTOR edge2 = XMVectorSubtract(p2, p0);
    XMVECTOR deltaUV1 = XMVectorSubtract(uv1, uv0);
    XMVECTOR deltaUV2 = XMVectorSubtract(uv2, uv0);

    float f = XMVectorGetX(deltaUV1) * XMVectorGetY(deltaUV2) - XMVectorGetX(deltaUV2) * XMVectorGetY(deltaUV1);
    if (fabsf(f) < 1e-6f)
      f = 1.0f;
    else
      f = 1.0f / f;

    XMVECTOR tangent = XMVectorScale(
      XMVectorSubtract(
        XMVectorScale(edge1, XMVectorGetY(deltaUV2)),
        XMVectorScale(edge2, XMVectorGetY(deltaUV1))), f);
    // Note the bitangent uses deltaUV1.x * edge2 - deltaUV2.x * edge1
    XMVECTOR bitangent = XMVectorScale(
      XMVectorSubtract(
        XMVectorScale(edge2, XMVectorGetX(deltaUV1)),
        XMVectorScale(edge1, XMVectorGetX(deltaUV2))), f);

    XMFLOAT3 tan;
    XMFLOAT3 bitan;
    XMStoreFloat3(&tan, tangent);
    XMStoreFloat3(&bitan, bitangent);
    // Accumulate
    v0.Tangent.x += tan.x;
    v0.Tangent.y += tan.y;
    v0.Tangent.z += tan.z;
    v0.Bitangent.x += bitan.x;
    v0.Bitangent.y += bitan.y;
    v0.Bitangent.z += bitan.z;
    v1.Tangent.x += tan.x;
    v1.Tangent.y += tan.y;
    v1.Tangent.z += tan.z;
    v1.Bitangent.x += bitan.x;
    v1.Bitangent.y += bitan.y;
    v1.Bitangent.z += bitan.z;
    v2.Tangent.x += tan.x;
    v2.Tangent.y += tan.y;
    v2.Tangent.z += tan.z;
    v2.Bitangent.x += bitan.x;
    v2.Bitangent.y += bitan.y;
    v2.Bitangent.z += bitan.z;
  }
  // Normalize tangents and bitangents
  for (auto& v : outVerts)
  {
    XMVECTOR T = XMLoadFloat3(&v.Tangent);
    T = XMVector3Normalize(T);
    XMStoreFloat3(&v.Tangent, T);
    XMVECTOR B = XMLoadFloat3(&v.Bitangent);
    B = XMVector3Normalize(B);
    XMStoreFloat3(&v.Bitangent, B);
  }
}

// Safe release macro
template<class T>
static void SafeRelease(T*& ptr)
{
  if (ptr)
  {
    ptr->Release();
    ptr = nullptr;
  }
}

// Main application class encapsulates window creation and rendering loop
class PBRApp
{
public:
  PBRApp(HINSTANCE hInst)
    : m_hInstance(hInst), m_hWnd(nullptr),
    m_driverType(D3D_DRIVER_TYPE_HARDWARE), m_featureLevel(D3D_FEATURE_LEVEL_11_0),
    m_pd3dDevice(nullptr), m_pImmediateContext(nullptr),
    m_pSwapChain(nullptr), m_pRenderTargetView(nullptr), m_pDepthStencil(nullptr), m_pDepthStencilView(nullptr),
    m_pVertexShader(nullptr), m_pPixelShader(nullptr), m_pInputLayout(nullptr),
    m_pVertexBuffer(nullptr), m_pIndexBuffer(nullptr), m_pConstantBuffer(nullptr)
  {
  }

  ~PBRApp()
  {
    CleanupDevice();
  }

  int Run(int nCmdShow)
  {
    if (FAILED(InitWindow(nCmdShow)))
      return 0;
    if (FAILED(InitDevice()))
      return 0;
    // Main message loop
    MSG msg = {};
    LARGE_INTEGER freq, prev;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&prev);
    while (WM_QUIT != msg.message)
    {
      if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
      {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
      else
      {
        LARGE_INTEGER curr;
        QueryPerformanceCounter(&curr);
        float deltaTime = static_cast<float>(curr.QuadPart - prev.QuadPart) / freq.QuadPart;
        prev = curr;
        Update(deltaTime);
        Render();
      }
    }
    return (int)msg.wParam;
  }

private:
  HRESULT InitWindow(int nCmdShow)
  {
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = PBRApp::WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = m_hInstance;
    wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = "PBRAppWindowClass";
    wcex.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
    if (!RegisterClassEx(&wcex))
      return E_FAIL;

    // Create window
    RECT rc = { 0, 0, 1280, 720 };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    m_hWnd = CreateWindow("PBRAppWindowClass", "Direct3D 11 PBR Demo", WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top,
      nullptr, nullptr, m_hInstance, this);
    if (!m_hWnd)
      return E_FAIL;
    ShowWindow(m_hWnd, nCmdShow);
    return S_OK;
  }

  static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
  {
    PBRApp* pApp = reinterpret_cast<PBRApp*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    switch (message)
    {
    case WM_CREATE:
    {
      CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
      SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pCreate->lpCreateParams);
    }
    return 0;
    case WM_PAINT:
    {
      if (pApp) pApp->Render();
      PAINTSTRUCT ps;
      BeginPaint(hWnd, &ps);
      EndPaint(hWnd, &ps);
    }
    return 0;
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
  }

  HRESULT InitDevice()
  {
    HRESULT hr = S_OK;
    RECT rc;
    GetClientRect(m_hWnd, &rc);
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = m_hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    for (UINT driver = 0; driver < numDriverTypes; ++driver)
    {
      m_driverType = driverTypes[driver];
      hr = D3D11CreateDeviceAndSwapChain(nullptr, m_driverType, nullptr,
        createDeviceFlags, featureLevels,
        numFeatureLevels, D3D11_SDK_VERSION,
        &sd, &m_pSwapChain, &m_pd3dDevice,
        &m_featureLevel, &m_pImmediateContext);
      if (SUCCEEDED(hr))
        break;
    }
    if (FAILED(hr))
      return hr;

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
    if (FAILED(hr))
      return hr;
    hr = m_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_pRenderTargetView);
    pBackBuffer->Release();
    if (FAILED(hr))
      return hr;

    // Create depth stencil texture
    D3D11_TEXTURE2D_DESC depthDesc;
    ZeroMemory(&depthDesc, sizeof(depthDesc));
    depthDesc.Width = width;
    depthDesc.Height = height;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.SampleDesc.Quality = 0;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthDesc.CPUAccessFlags = 0;
    depthDesc.MiscFlags = 0;
    hr = m_pd3dDevice->CreateTexture2D(&depthDesc, nullptr, &m_pDepthStencil);
    if (FAILED(hr))
      return hr;
    hr = m_pd3dDevice->CreateDepthStencilView(m_pDepthStencil, nullptr, &m_pDepthStencilView);
    if (FAILED(hr))
      return hr;
    m_pImmediateContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

    // Setup viewport
    D3D11_VIEWPORT vp;
    vp.Width = static_cast<FLOAT>(width);
    vp.Height = static_cast<FLOAT>(height);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    m_pImmediateContext->RSSetViewports(1, &vp);

    // Load assets (model and textures) and create buffers/shaders
    if (FAILED(LoadAssets()))
      return E_FAIL;

    return S_OK;
  }

  HRESULT LoadAssets()
  {
    HRESULT hr = S_OK;
    // Load the OBJ model
    objl::Loader loader;
    bool loaded = loader.LoadFile("Models/drakefire_pistol_low.obj");
    if (!loaded)
    {
      MessageBoxA(nullptr, "Failed to load OBJ file", "Error", MB_OK);
      return E_FAIL;
    }
    // Flatten the meshes into one vertex/index list
    std::vector<Vertex> tempVerts;
    std::vector<unsigned int> indices;
    for (const auto& mesh : loader.LoadedMeshes)
    {
      for (size_t i = 0; i < mesh.Vertices.size(); ++i)
      {
        Vertex v;
        v.Position = XMFLOAT3(mesh.Vertices[i].Position.X, mesh.Vertices[i].Position.Y, mesh.Vertices[i].Position.Z);
        v.Normal = XMFLOAT3(mesh.Vertices[i].Normal.X, mesh.Vertices[i].Normal.Y, mesh.Vertices[i].Normal.Z);
        v.Tex = XMFLOAT2(mesh.Vertices[i].TextureCoordinate.X, mesh.Vertices[i].TextureCoordinate.Y);
        v.Tangent = XMFLOAT3(0, 0, 0);
        v.Bitangent = XMFLOAT3(0, 0, 0);
        tempVerts.push_back(v);
      }
      for (size_t idx : mesh.Indices)
      {
        indices.push_back(static_cast<unsigned int>(idx));
      }
    }
    // Compute tangents
    std::vector<Vertex> vertices;
    ComputeTangents(tempVerts, indices, vertices);

    m_indexCount = static_cast<UINT>(indices.size());
    // Create vertex buffer
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = UINT(sizeof(Vertex) * vertices.size());
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = vertices.data();
    hr = m_pd3dDevice->CreateBuffer(&bd, &InitData, &m_pVertexBuffer);
    if (FAILED(hr))
      return hr;
    // Create index buffer
    D3D11_BUFFER_DESC ibd;
    ZeroMemory(&ibd, sizeof(ibd));
    ibd.Usage = D3D11_USAGE_DEFAULT;
    ibd.ByteWidth = UINT(sizeof(unsigned int) * indices.size());
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA iInitData;
    ZeroMemory(&iInitData, sizeof(iInitData));
    iInitData.pSysMem = indices.data();
    hr = m_pd3dDevice->CreateBuffer(&ibd, &iInitData, &m_pIndexBuffer);
    if (FAILED(hr))
      return hr;

    // Load textures using stb_image
    stbi_set_flip_vertically_on_load(1);
    // Helper lambda to load a texture file
    auto LoadTexture = [&](const char* filename, ID3D11ShaderResourceView** srv) -> HRESULT
      {
        int texWidth = 0, texHeight = 0, texComp = 0;
        stbi_uc* imageData = stbi_load(filename, &texWidth, &texHeight, &texComp, 4);
        if (!imageData)
        {
          std::stringstream ss;
          ss << "Failed to load texture: " << filename;
          MessageBoxA(nullptr, ss.str().c_str(), "Error", MB_OK);
          return E_FAIL;
        }
        // Describe texture
        D3D11_TEXTURE2D_DESC texDesc;
        ZeroMemory(&texDesc, sizeof(texDesc));
        texDesc.Width = texWidth;
        texDesc.Height = texHeight;
        texDesc.MipLevels = 1;
        texDesc.ArraySize = 1;
        texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        texDesc.SampleDesc.Count = 1;
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        texDesc.CPUAccessFlags = 0;
        texDesc.MiscFlags = 0;
        D3D11_SUBRESOURCE_DATA texInit;
        texInit.pSysMem = imageData;
        texInit.SysMemPitch = texWidth * 4;
        texInit.SysMemSlicePitch = 0;
        ID3D11Texture2D* pTexture = nullptr;
        HRESULT hrTex = m_pd3dDevice->CreateTexture2D(&texDesc, &texInit, &pTexture);
        stbi_image_free(imageData);
        if (FAILED(hrTex))
          return hrTex;
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        ZeroMemory(&srvDesc, sizeof(srvDesc));
        srvDesc.Format = texDesc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
        srvDesc.Texture2D.MostDetailedMip = 0;
        hrTex = m_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, srv);
        pTexture->Release();
        return hrTex;
      };
    std::string assetPath = "Textures/";
    hr = LoadTexture((assetPath + "base_albedo.jpg").c_str(), &m_pAlbedoSRV); if (FAILED(hr)) return hr;
    hr = LoadTexture((assetPath + "base_metallic.jpg").c_str(), &m_pMetallicSRV); if (FAILED(hr)) return hr;
    hr = LoadTexture((assetPath + "base_roughness.jpg").c_str(), &m_pRoughnessSRV); if (FAILED(hr)) return hr;
    hr = LoadTexture((assetPath + "base_AO.jpg").c_str(), &m_pAOSRV); if (FAILED(hr)) return hr;
    hr = LoadTexture((assetPath + "base_normal.jpg").c_str(), &m_pNormalSRV); if (FAILED(hr)) return hr;

    // Create sampler state
    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = m_pd3dDevice->CreateSamplerState(&sampDesc, &m_pSampler);
    if (FAILED(hr)) return hr;

    // Compile shaders
    UINT flags = 0;
#ifdef _DEBUG
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
    ID3DBlob* vsBlob = nullptr;
    ID3DBlob* psBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;
    // Read shader file into string
    std::ifstream file("PBRShaders.hlsl");
    if (!file)
    {
      MessageBoxA(nullptr, "Could not open PBRShaders.hlsl", "Error", MB_OK);
      return E_FAIL;
    }
    std::string shaderStr((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    // Compile vertex shader
    hr = D3DCompile(shaderStr.c_str(), shaderStr.size(), nullptr, nullptr, nullptr,
      "VS", "vs_5_0", flags, 0, &vsBlob, &errorBlob);
    if (FAILED(hr))
    {
      if (errorBlob)
      {
        std::string errMsg((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize());
        MessageBoxA(nullptr, errMsg.c_str(), "Vertex Shader Error", MB_OK);
        errorBlob->Release();
      }
      return hr;
    }
    hr = m_pd3dDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_pVertexShader);
    if (FAILED(hr))
    {
      vsBlob->Release();
      return hr;
    }
    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,                            D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, Normal),   D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, Tangent),  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BITANGENT",0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, Bitangent),D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, offsetof(Vertex, Tex),      D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    UINT numElements = ARRAYSIZE(layout);
    hr = m_pd3dDevice->CreateInputLayout(layout, numElements, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_pInputLayout);
    vsBlob->Release();
    if (FAILED(hr)) return hr;
    // Compile pixel shader
    hr = D3DCompile(shaderStr.c_str(), shaderStr.size(), nullptr, nullptr, nullptr,
      "PS", "ps_5_0", flags, 0, &psBlob, &errorBlob);
    if (FAILED(hr))
    {
      if (errorBlob)
      {
        std::string errMsg((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize());
        MessageBoxA(nullptr, errMsg.c_str(), "Pixel Shader Error", MB_OK);
        errorBlob->Release();
      }
      return hr;
    }
    hr = m_pd3dDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_pPixelShader);
    psBlob->Release();
    if (FAILED(hr)) return hr;

    // Create constant buffer
    D3D11_BUFFER_DESC cbd;
    ZeroMemory(&cbd, sizeof(cbd));
    cbd.ByteWidth = sizeof(CBMain);
    cbd.Usage = D3D11_USAGE_DEFAULT;
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.CPUAccessFlags = 0;
    hr = m_pd3dDevice->CreateBuffer(&cbd, nullptr, &m_pConstantBuffer);
    if (FAILED(hr)) return hr;

    // Set primitive topology
    m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    return S_OK;
  }

  void CleanupDevice()
  {
    if (m_pImmediateContext) m_pImmediateContext->ClearState();
    SafeRelease(m_pConstantBuffer);
    SafeRelease(m_pSampler);
    SafeRelease(m_pAlbedoSRV);
    SafeRelease(m_pMetallicSRV);
    SafeRelease(m_pRoughnessSRV);
    SafeRelease(m_pAOSRV);
    SafeRelease(m_pNormalSRV);
    SafeRelease(m_pVertexBuffer);
    SafeRelease(m_pIndexBuffer);
    SafeRelease(m_pInputLayout);
    SafeRelease(m_pVertexShader);
    SafeRelease(m_pPixelShader);
    SafeRelease(m_pDepthStencilView);
    SafeRelease(m_pDepthStencil);
    SafeRelease(m_pRenderTargetView);
    SafeRelease(m_pSwapChain);
    SafeRelease(m_pImmediateContext);
    SafeRelease(m_pd3dDevice);
  }

  void Update(float deltaTime)
  {
    // Rotate the model over time
    m_rotateAngle += deltaTime * 0.5f;
    if (m_rotateAngle > XM_2PI)
      m_rotateAngle -= XM_2PI;
  }

  void Render()
  {
    // Clear the back buffer
    float ClearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
    m_pImmediateContext->ClearRenderTargetView(m_pRenderTargetView, ClearColor);
    m_pImmediateContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // Set input layout and shaders
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    m_pImmediateContext->IASetInputLayout(m_pInputLayout);
    m_pImmediateContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
    m_pImmediateContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    m_pImmediateContext->VSSetShader(m_pVertexShader, nullptr, 0);
    m_pImmediateContext->PSSetShader(m_pPixelShader, nullptr, 0);
    // Set texture SRVs and sampler
    ID3D11ShaderResourceView* srvs[] = { m_pAlbedoSRV, m_pNormalSRV, m_pMetallicSRV, m_pRoughnessSRV, m_pAOSRV };
    m_pImmediateContext->PSSetShaderResources(0, 5, srvs);
    m_pImmediateContext->PSSetSamplers(0, 1, &m_pSampler);

    // Update constant buffer
    CBMain cb;
    // Compute matrices
    XMMATRIX world = XMMatrixRotationY(m_rotateAngle) * XMMatrixScaling(1.0f, 1.0f, 1.0f);
    XMMATRIX view = XMMatrixLookAtLH(XMVectorSet(0.0f, 2.0f, -5.0f, 1.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
    RECT rc;
    GetClientRect(m_hWnd, &rc);
    float width = static_cast<float>(rc.right - rc.left);
    float height = static_cast<float>(rc.bottom - rc.top);
    XMMATRIX proj = XMMatrixPerspectiveFovLH(XM_PIDIV4, width / height, 0.1f, 100.0f);
    XMStoreFloat4x4(&cb.World, XMMatrixTranspose(world));
    XMStoreFloat4x4(&cb.View, XMMatrixTranspose(view));
    XMStoreFloat4x4(&cb.Projection, XMMatrixTranspose(proj));
    cb.CameraPos = XMFLOAT3(0.0f, 2.0f, -5.0f);
    cb.LightDir = XMFLOAT3(0.0f, -1.0f, 0.3f);
    cb.LightColor = XMFLOAT3(5.0f, 5.0f, 5.0f);
    m_pImmediateContext->UpdateSubresource(m_pConstantBuffer, 0, nullptr, &cb, 0, 0);
    m_pImmediateContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
    m_pImmediateContext->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);

    // Draw the mesh
    m_pImmediateContext->DrawIndexed(m_indexCount, 0, 0);

    // Present
    m_pSwapChain->Present(1, 0);
  }

private:
  HINSTANCE              m_hInstance;
  HWND                   m_hWnd;
  D3D_DRIVER_TYPE        m_driverType;
  D3D_FEATURE_LEVEL      m_featureLevel;
  ID3D11Device* m_pd3dDevice;
  ID3D11DeviceContext* m_pImmediateContext;
  IDXGISwapChain* m_pSwapChain;
  ID3D11RenderTargetView* m_pRenderTargetView;
  ID3D11Texture2D* m_pDepthStencil;
  ID3D11DepthStencilView* m_pDepthStencilView;
  ID3D11VertexShader* m_pVertexShader;
  ID3D11PixelShader* m_pPixelShader;
  ID3D11InputLayout* m_pInputLayout;
  ID3D11Buffer* m_pVertexBuffer;
  ID3D11Buffer* m_pIndexBuffer;
  ID3D11Buffer* m_pConstantBuffer;
  UINT                   m_indexCount;
  // Textures and sampler
  ID3D11ShaderResourceView* m_pAlbedoSRV;
  ID3D11ShaderResourceView* m_pMetallicSRV;
  ID3D11ShaderResourceView* m_pRoughnessSRV;
  ID3D11ShaderResourceView* m_pAOSRV;
  ID3D11ShaderResourceView* m_pNormalSRV;
  ID3D11SamplerState* m_pSampler;
  // Animation
  float m_rotateAngle = 0.0f;
};

// Entry point
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
  PBRApp app(hInstance);
  return app.Run(nCmdShow);
}