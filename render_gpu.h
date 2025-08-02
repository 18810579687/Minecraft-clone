#ifndef RENDER_GPU_H
#define RENDER_GPU_H

#include <d3d9.h>
#include <d3dx9.h>
#include <dxgi.h>     // 添加DXGI头文件
#include <vector>
#include <string>
#include <iostream>
#include <windows.h>
#include <algorithm>
#include "math3d.h"

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "dxgi.lib")  // 添加DXGI库

// 强制使用NVIDIA GPU
extern "C" {
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

// 调试日志宏
#define DEBUG_LOG(msg) { std::cout << "[DEBUG] " << msg << std::endl; }

// DXGI适配器信息结构
struct DXGIAdapterInfo {
    std::wstring name;
    UINT vendorId;
    UINT deviceId;
    SIZE_T dedicatedVideoMemory;
    bool isDiscrete;
    bool isNVIDIA;
    bool isAMD;
    bool isIntel;
    bool isSoftware;
    LUID adapterLuid;
};

// 适配器信息结构
struct AdapterInfo {
    UINT adapterIndex;
    D3DADAPTER_IDENTIFIER9 identifier;
    UINT vramMB;       // VRAM大小(MB)
    bool isDiscrete;   // 是否为独立GPU
    bool isNVIDIA;     // 是否为NVIDIA GPU
    bool isAMD;        // 是否为AMD GPU
    bool isIntel;      // 是否为Intel GPU
    
    AdapterInfo() : adapterIndex(0), vramMB(0), isDiscrete(false), isNVIDIA(false), isAMD(false), isIntel(false) {
        ZeroMemory(&identifier, sizeof(identifier));
    }
};

// 顶点结构
struct VertexPositionColor {
    float x, y, z;    // 位置
    DWORD color;      // 颜色
    
    static const DWORD FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;
};

// GPU渲染器类
class GPURenderer {
private:
    HWND hwnd;                           // 窗口句柄
    LPDIRECT3D9 d3d;                     // Direct3D对象
    LPDIRECT3DDEVICE9 d3dDevice;         // Direct3D设备
    LPDIRECT3DVERTEXBUFFER9 vertexBuffer; // 顶点缓冲区
    LPDIRECT3DINDEXBUFFER9 indexBuffer;  // 索引缓冲区
    D3DPRESENT_PARAMETERS d3dpp;         // 设备参数
    
    int width;                           // 渲染宽度
    int height;                          // 渲染高度
    AdapterInfo bestAdapter;             // 最佳适配器信息
    
    LPD3DXFONT font;                     // DirectX字体
    
    // 渲染统计信息
    int trianglesRendered;
    int drawCalls;
    
    // 查找最佳GPU适配器
    UINT FindBestAdapter() {
        try {
            // 尝试使用DXGI获取更准确的显存信息
            std::vector<DXGIAdapterInfo> dxgiAdapters = EnumerateDXGIAdapters();
            
            // 创建临时Direct3D对象用于枚举适配器
            LPDIRECT3D9 tempD3d = Direct3DCreate9(D3D_SDK_VERSION);
            if (!tempD3d) {
                MessageBox(hwnd, "Failed to create Direct3D object for adapter enumeration", "Error", MB_OK);
                return D3DADAPTER_DEFAULT;
            }
            
            // 获取适配器数量
            UINT adapterCount = tempD3d->GetAdapterCount();
            if (adapterCount == 0) {
                tempD3d->Release();
                MessageBox(hwnd, "No GPU adapters found", "Error", MB_OK);
                return D3DADAPTER_DEFAULT;
            }
            
            std::vector<AdapterInfo> adapters;
            
            // 枚举所有适配器
            for (UINT i = 0; i < adapterCount; i++) {
                AdapterInfo adapter;
                ZeroMemory(&adapter, sizeof(AdapterInfo)); // 确保完全初始化
                
                adapter.adapterIndex = i;
                
                // 获取适配器标识符
                if (FAILED(tempD3d->GetAdapterIdentifier(i, 0, &adapter.identifier))) {
                    continue; // 跳过获取失败的适配器
                }
                
                // 确定适配器类型
                adapter.isNVIDIA = (adapter.identifier.VendorId == 0x10DE);    // NVIDIA
                adapter.isAMD = (adapter.identifier.VendorId == 0x1002);       // AMD
                adapter.isIntel = (adapter.identifier.VendorId == 0x8086);     // Intel
                
                // 非Intel通常是独立显卡
                adapter.isDiscrete = !adapter.isIntel;
                
                // 尝试从DXGI获取更准确的显存信息
                bool foundInDXGI = false;
                for (const auto& dxgiAdapter : dxgiAdapters) {
                    // 匹配相同的VendorId和DeviceId
                    if (dxgiAdapter.vendorId == adapter.identifier.VendorId && 
                        dxgiAdapter.deviceId == adapter.identifier.DeviceId) {
                        // 使用DXGI提供的更准确的显存信息
                        adapter.vramMB = static_cast<UINT>(dxgiAdapter.dedicatedVideoMemory / (1024 * 1024));
                        foundInDXGI = true;
                        break;
                    }
                }
                
                // 如果没有匹配的DXGI适配器，尝试创建设备获取可用纹理内存
                if (!foundInDXGI) {
                    // 获取可用纹理内存
                    D3DDISPLAYMODE displayMode;
                    if (SUCCEEDED(tempD3d->GetAdapterDisplayMode(i, &displayMode))) {
                        D3DPRESENT_PARAMETERS d3dpp_memcheck = {0};
                        d3dpp_memcheck.Windowed = TRUE;
                        d3dpp_memcheck.SwapEffect = D3DSWAPEFFECT_DISCARD;
                        d3dpp_memcheck.BackBufferFormat = displayMode.Format;
                        d3dpp_memcheck.BackBufferWidth = 1;
                        d3dpp_memcheck.BackBufferHeight = 1;
                        d3dpp_memcheck.EnableAutoDepthStencil = FALSE;
                        d3dpp_memcheck.hDeviceWindow = hwnd;

                        LPDIRECT3DDEVICE9 tempDevice = NULL;
                        if (SUCCEEDED(tempD3d->CreateDevice(i, D3DDEVTYPE_HAL, hwnd,
                                                        D3DCREATE_HARDWARE_VERTEXPROCESSING,
                                                        &d3dpp_memcheck, &tempDevice))) {
                            adapter.vramMB = tempDevice->GetAvailableTextureMem() / (1024 * 1024);
                            tempDevice->Release();
                            tempDevice = NULL;
                        } else {
                            // Try with SOFTWARE_VERTEXPROCESSING if HARDWARE fails for mem check
                            if (SUCCEEDED(tempD3d->CreateDevice(i, D3DDEVTYPE_HAL, hwnd,
                                                            D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                                                            &d3dpp_memcheck, &tempDevice))) {
                                adapter.vramMB = tempDevice->GetAvailableTextureMem() / (1024 * 1024);
                                tempDevice->Release();
                                tempDevice = NULL;
                            } else {
                                adapter.vramMB = 0; // Default to 0 if device creation fails
                            }
                        }
                    } else {
                        adapter.vramMB = 0; // Default to 0 if display mode fetch fails
                    }
                }
                
                // 打印调试信息
                char debugInfo[256];
                sprintf_s(debugInfo, "Detected GPU Adapter #%u: %s, VendorID: 0x%x, VRAM: %u MB, Discrete: %s",
                        i, adapter.identifier.Description, adapter.identifier.VendorId, adapter.vramMB, adapter.isDiscrete ? "Yes" : "No");
                OutputDebugStringA(debugInfo);
                
                // 添加到适配器列表
                adapters.push_back(adapter);
            }
            
            // 释放临时Direct3D对象
            tempD3d->Release();
            
            if (adapters.empty()) {
                MessageBox(hwnd, "No usable GPU adapters found after enumeration.", "Warning", MB_OK);
                return D3DADAPTER_DEFAULT;
            }
            
            // 首先尝试查找NVIDIA GPU（优先级最高）
            for (const auto& adapter : adapters) {
                if (adapter.isNVIDIA) {
                    bestAdapter = adapter;
                    return adapter.adapterIndex;
                }
            }
            
            // 如果没有NVIDIA GPU，查找AMD GPU
            for (const auto& adapter : adapters) {
                if (adapter.isAMD) {
                    bestAdapter = adapter;
                    return adapter.adapterIndex;
                }
            }
            
            // 如果没有NVIDIA或AMD GPU，使用第一个可用适配器
            bestAdapter = adapters[0];
            return bestAdapter.adapterIndex;
        }
        catch(const std::exception& e) {
            char errorMsg[256];
            sprintf_s(errorMsg, "GPU adapter detection exception: %s", e.what());
            MessageBox(hwnd, errorMsg, "Error", MB_OK);
            return D3DADAPTER_DEFAULT;
        }
    }
    
    // 使用DXGI枚举适配器
    std::vector<DXGIAdapterInfo> EnumerateDXGIAdapters() {
        std::vector<DXGIAdapterInfo> adapters;
        
        try {
            // 初始化COM
            HRESULT hrCom = CoInitialize(NULL);
            if (FAILED(hrCom)) {
                std::cout << "COM initialization failed" << std::endl;
                return adapters;
            }
            
            // 创建DXGI Factory
            IDXGIFactory* factory = nullptr;
            HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
            
            if (SUCCEEDED(hr)) {
                // 枚举适配器
                IDXGIAdapter* adapter = nullptr;
                for (UINT i = 0; factory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i) {
                    DXGI_ADAPTER_DESC desc;
                    adapter->GetDesc(&desc);
                    
                    DXGIAdapterInfo adapterInfo;
                    adapterInfo.name = desc.Description;
                    adapterInfo.vendorId = desc.VendorId;
                    adapterInfo.deviceId = desc.DeviceId;
                    adapterInfo.dedicatedVideoMemory = desc.DedicatedVideoMemory;
                    adapterInfo.adapterLuid = desc.AdapterLuid;
                    
                    // 识别供应商
                    adapterInfo.isNVIDIA = (desc.VendorId == 0x10DE);
                    adapterInfo.isAMD = (desc.VendorId == 0x1002);
                    adapterInfo.isIntel = (desc.VendorId == 0x8086);
                    
                    // 软件适配器检查
                    adapterInfo.isSoftware = (wcscmp(desc.Description, L"Microsoft Basic Render Driver") == 0) ||
                                            (wcsstr(desc.Description, L"Remote") != nullptr);
                    
                    // 判断是否为独立显卡
                    adapterInfo.isDiscrete = adapterInfo.isNVIDIA || adapterInfo.isAMD;
                    
                    // 添加到列表
                    adapters.push_back(adapterInfo);
                    
                    // 打印适配器信息
                    char nameBuffer[128];
                    wcstombs(nameBuffer, desc.Description, sizeof(nameBuffer));
                    std::cout << "DXGI Adapter " << i << ": " << nameBuffer << 
                                ", VRAM: " << (desc.DedicatedVideoMemory / (1024 * 1024)) << "MB" <<
                                ", Vendor ID: 0x" << std::hex << desc.VendorId << std::dec << std::endl;
                    
                    adapter->Release();
                }
                
                factory->Release();
            }
            
            // 释放COM
            CoUninitialize();
        } catch (const std::exception& e) {
            std::cout << "Exception in EnumerateDXGIAdapters: " << e.what() << std::endl;
        }
        
        return adapters;
    }
    
    // 释放所有DirectX资源
    void ReleaseResources() {
        if (font) {
            font->Release();
            font = NULL;
        }
        
        if (vertexBuffer) {
            vertexBuffer->Release();
            vertexBuffer = NULL;
        }
        
        if (indexBuffer) {
            indexBuffer->Release();
            indexBuffer = NULL;
        }
        
        if (d3dDevice) {
            d3dDevice->Release();
            d3dDevice = NULL;
        }
        
        if (d3d) {
            d3d->Release();
            d3d = NULL;
        }
    }
    
public:
    GPURenderer() : hwnd(NULL), d3d(NULL), d3dDevice(NULL), vertexBuffer(NULL),
                   indexBuffer(NULL), width(0), height(0), font(NULL),
                   trianglesRendered(0), drawCalls(0) {
        ZeroMemory(&d3dpp, sizeof(d3dpp));
        // 初始化bestAdapter
        ZeroMemory(&bestAdapter, sizeof(AdapterInfo));
    }
    
    ~GPURenderer() {
        ReleaseResources();
    }
    
    // 初始化DirectX
    bool Initialize(HWND hwnd, int width, int height) {
        this->hwnd = hwnd;
        this->width = width;
        this->height = height;
        
        // 创建Direct3D对象
        d3d = Direct3DCreate9(D3D_SDK_VERSION);
        if (!d3d) {
            MessageBox(hwnd, "Failed to create Direct3D object", "Error", MB_OK);
            return false;
        }
        
        // 查找最佳适配器
        UINT selectedAdapter = FindBestAdapter();
        
        // 设置呈现参数
        ZeroMemory(&d3dpp, sizeof(d3dpp));
        d3dpp.Windowed = TRUE;
        d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
        d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
        d3dpp.EnableAutoDepthStencil = TRUE;
        d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
        d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE; // 垂直同步
        d3dpp.hDeviceWindow = hwnd;
        
        // 尝试创建具有硬件顶点处理功能的设备
        HRESULT hr = d3d->CreateDevice(
            selectedAdapter, D3DDEVTYPE_HAL, hwnd,
            D3DCREATE_HARDWARE_VERTEXPROCESSING,
            &d3dpp, &d3dDevice);
            
        // 如果失败，尝试软件顶点处理
        if (FAILED(hr)) {
            hr = d3d->CreateDevice(
                selectedAdapter, D3DDEVTYPE_HAL, hwnd,
                D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                &d3dpp, &d3dDevice);
                
            // 如果仍然失败，尝试参考设备
            if (FAILED(hr)) {
                hr = d3d->CreateDevice(
                    selectedAdapter, D3DDEVTYPE_REF, hwnd,
                    D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                    &d3dpp, &d3dDevice);
                    
                if (FAILED(hr)) {
                    MessageBox(hwnd, "Failed to create Direct3D device", "Error", MB_OK);
                    return false;
                }
            }
        }
        
        // 设置渲染状态
        d3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);            // 启用深度测试
        d3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);          // 禁用灯光
        d3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);   // 不剔除任何面
        
        // 启用alpha混合
        d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        
        // 启用Alpha测试，用于处理透明纹理
        d3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
        d3dDevice->SetRenderState(D3DRS_ALPHAREF, 0x08);
        d3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
        
        // 设置纹理过滤
        d3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
        d3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
        
        // 创建字体
        hr = D3DXCreateFont(d3dDevice, 16, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET,
                           OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
                           "Arial", &font);
        if (FAILED(hr)) {
            MessageBox(hwnd, "Failed to create font", "Error", MB_OK);
            // 不返回失败，因为字体不是关键功能
        }
        
        return true;
    }
    
    // 调整渲染器大小
    void Resize(int width, int height) {
        this->width = width;
        this->height = height;
        
        if (d3dDevice) {
            // 释放需要重新创建的资源
            if (font) {
                font->Release();
                font = NULL;
            }
            
            // 重置设备
            d3dpp.BackBufferWidth = width;
            d3dpp.BackBufferHeight = height;
            
            HRESULT hr = d3dDevice->Reset(&d3dpp);
            if (SUCCEEDED(hr)) {
                // 重新创建字体
                D3DXCreateFont(d3dDevice, 16, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET,
                              OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
                              "Arial", &font);
                
                // 重置渲染状态
                d3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
                d3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
                d3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
                
                // 重新设置Alpha混合
                d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
                d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
                d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
                
                // 重新设置Alpha测试
                d3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
                d3dDevice->SetRenderState(D3DRS_ALPHAREF, 0x08);
                d3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
                
                // 重新设置纹理过滤
                d3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
                d3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
            }
        }
    }
    
    // 开始场景渲染
    bool BeginScene(const Color& clearColor = Color(135, 206, 235)) { // 默认天空蓝色
        if (!d3dDevice) return false;
        
        // 清除统计信息
        trianglesRendered = 0;
        drawCalls = 0;
        
        // 清空缓冲区，使用传入的颜色
        D3DCOLOR bgColor = D3DCOLOR_RGBA(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        HRESULT hr = d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, bgColor, 1.0f, 0);
        
        if (FAILED(hr)) return false;
        
        // 开始场景
        hr = d3dDevice->BeginScene();
        if (FAILED(hr)) return false;
        
        // 设置基本渲染状态
        d3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);           // 启用深度测试
        d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);      // 启用深度写入
        d3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);         // 禁用灯光
        d3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);  // 暂时禁用面剔除，排查渲染问题
        
        // 设置正确的深度比较函数 - DirectX默认是D3DCMP_LESSEQUAL
        d3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
        
        // 纹理过滤设置 - 使用点采样以获得清晰的像素化效果
        d3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
        d3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
        d3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
        
        // 启用Alpha混合
        d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        
        // 设置Alpha测试 - 丢弃低于阈值的Alpha像素
        d3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
        d3dDevice->SetRenderState(D3DRS_ALPHAREF, 0x08);
        d3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);

        // 设置单位世界矩阵作为默认值
        D3DXMATRIX identity;
        D3DXMatrixIdentity(&identity);
        d3dDevice->SetTransform(D3DTS_WORLD, &identity);
        
        return true;
    }
    
    // 结束场景渲染并呈现
    void EndScene() {
        if (!d3dDevice) return;
        
        d3dDevice->EndScene();
        d3dDevice->Present(NULL, NULL, NULL, NULL);
    }
    
    // 设置变换矩阵
    void SetTransform(const Mat4& viewMatrix, const Mat4& projectionMatrix) {
        if (!d3dDevice) return;
        
        // 转换为D3DXMATRIX
        D3DXMATRIX view, projection;
        
        // 复制视图矩阵到D3DXMATRIX
        view._11 = viewMatrix.m[0][0]; view._12 = viewMatrix.m[0][1]; view._13 = viewMatrix.m[0][2]; view._14 = viewMatrix.m[0][3];
        view._21 = viewMatrix.m[1][0]; view._22 = viewMatrix.m[1][1]; view._23 = viewMatrix.m[1][2]; view._24 = viewMatrix.m[1][3];
        view._31 = viewMatrix.m[2][0]; view._32 = viewMatrix.m[2][1]; view._33 = viewMatrix.m[2][2]; view._34 = viewMatrix.m[2][3];
        view._41 = viewMatrix.m[3][0]; view._42 = viewMatrix.m[3][1]; view._43 = viewMatrix.m[3][2]; view._44 = viewMatrix.m[3][3];
        
        // 复制投影矩阵到D3DXMATRIX - 现在已经是DirectX格式
        projection._11 = projectionMatrix.m[0][0]; projection._12 = projectionMatrix.m[0][1]; projection._13 = projectionMatrix.m[0][2]; projection._14 = projectionMatrix.m[0][3];
        projection._21 = projectionMatrix.m[1][0]; projection._22 = projectionMatrix.m[1][1]; projection._23 = projectionMatrix.m[1][2]; projection._24 = projectionMatrix.m[1][3];
        projection._31 = projectionMatrix.m[2][0]; projection._32 = projectionMatrix.m[2][1]; projection._33 = projectionMatrix.m[2][2]; projection._34 = projectionMatrix.m[2][3];
        projection._41 = projectionMatrix.m[3][0]; projection._42 = projectionMatrix.m[3][1]; projection._43 = projectionMatrix.m[3][2]; projection._44 = projectionMatrix.m[3][3];
        
        // 设置矩阵
        d3dDevice->SetTransform(D3DTS_VIEW, &view);
        d3dDevice->SetTransform(D3DTS_PROJECTION, &projection);
    }
    
    // 设置世界矩阵
    void SetWorldMatrix(const Mat4& worldMatrix) {
        if (!d3dDevice) return;
        
        D3DXMATRIX world;
        world._11 = worldMatrix.m[0][0]; world._12 = worldMatrix.m[0][1]; world._13 = worldMatrix.m[0][2]; world._14 = worldMatrix.m[0][3];
        world._21 = worldMatrix.m[1][0]; world._22 = worldMatrix.m[1][1]; world._23 = worldMatrix.m[1][2]; world._24 = worldMatrix.m[1][3];
        world._31 = worldMatrix.m[2][0]; world._32 = worldMatrix.m[2][1]; world._33 = worldMatrix.m[2][2]; world._34 = worldMatrix.m[2][3];
        world._41 = worldMatrix.m[3][0]; world._42 = worldMatrix.m[3][1]; world._43 = worldMatrix.m[3][2]; world._44 = worldMatrix.m[3][3];
        
        d3dDevice->SetTransform(D3DTS_WORLD, &world);
    }
    
    // 绘制方块面
    void DrawBlockFace(const Vec3& position, int face, const Color& color) {
        if (!d3dDevice) return;
        
        // 确保DirectX状态正确设置
        d3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
        d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
        d3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE); // 暂时禁用面剔除，排查渲染问题
        
        // 临时设置单位世界矩阵，确保方块在正确位置
        D3DXMATRIX identityWorld;
        D3DXMatrixIdentity(&identityWorld);
        d3dDevice->SetTransform(D3DTS_WORLD, &identityWorld);
        
        // 定义方块面的顶点位置（单位方块）
        Vec3 vertices[4];
        
        // 根据面的类型确定四个顶点位置
        switch (face) {
            case 0: // FACE_FRONT (Z+ 面)
                vertices[0] = Vec3(position.x + 0.0f, position.y + 0.0f, position.z + 1.0f);
                vertices[1] = Vec3(position.x + 1.0f, position.y + 0.0f, position.z + 1.0f);
                vertices[2] = Vec3(position.x + 1.0f, position.y + 1.0f, position.z + 1.0f);
                vertices[3] = Vec3(position.x + 0.0f, position.y + 1.0f, position.z + 1.0f);
                break;
            case 1: // FACE_BACK (Z- 面)
                vertices[0] = Vec3(position.x + 1.0f, position.y + 0.0f, position.z + 0.0f);
                vertices[1] = Vec3(position.x + 0.0f, position.y + 0.0f, position.z + 0.0f);
                vertices[2] = Vec3(position.x + 0.0f, position.y + 1.0f, position.z + 0.0f);
                vertices[3] = Vec3(position.x + 1.0f, position.y + 1.0f, position.z + 0.0f);
                break;
            case 2: // FACE_LEFT (X- 面)
                vertices[0] = Vec3(position.x + 0.0f, position.y + 0.0f, position.z + 0.0f);
                vertices[1] = Vec3(position.x + 0.0f, position.y + 0.0f, position.z + 1.0f);
                vertices[2] = Vec3(position.x + 0.0f, position.y + 1.0f, position.z + 1.0f);
                vertices[3] = Vec3(position.x + 0.0f, position.y + 1.0f, position.z + 0.0f);
                break;
            case 3: // FACE_RIGHT (X+ 面)
                vertices[0] = Vec3(position.x + 1.0f, position.y + 0.0f, position.z + 1.0f);
                vertices[1] = Vec3(position.x + 1.0f, position.y + 0.0f, position.z + 0.0f);
                vertices[2] = Vec3(position.x + 1.0f, position.y + 1.0f, position.z + 0.0f);
                vertices[3] = Vec3(position.x + 1.0f, position.y + 1.0f, position.z + 1.0f);
                break;
            case 4: // FACE_TOP (Y+ 面)
                vertices[0] = Vec3(position.x + 0.0f, position.y + 1.0f, position.z + 1.0f);
                vertices[1] = Vec3(position.x + 1.0f, position.y + 1.0f, position.z + 1.0f);
                vertices[2] = Vec3(position.x + 1.0f, position.y + 1.0f, position.z + 0.0f);
                vertices[3] = Vec3(position.x + 0.0f, position.y + 1.0f, position.z + 0.0f);
                break;
            case 5: // FACE_BOTTOM (Y- 面)
                vertices[0] = Vec3(position.x + 0.0f, position.y + 0.0f, position.z + 0.0f);
                vertices[1] = Vec3(position.x + 1.0f, position.y + 0.0f, position.z + 0.0f);
                vertices[2] = Vec3(position.x + 1.0f, position.y + 0.0f, position.z + 1.0f);
                vertices[3] = Vec3(position.x + 0.0f, position.y + 0.0f, position.z + 1.0f);
                break;
        }
        
        // 输出调试信息，观察方块顶点坐标
        char debug_msg[256];
        sprintf_s(debug_msg, "Render block face %d at postion (%.1f, %.1f, %.1f)", face, position.x, position.y, position.z);
        OutputDebugStringA(debug_msg);
        
        // 应用简单的面光照 - 根据面的方向调整颜色亮度
        D3DCOLOR faceColor;
        
        switch (face) {
            case 4: // 顶面最亮
                faceColor = D3DCOLOR_RGBA(color.r, color.g, color.b, color.a);
                break;
            case 5: // 底面最暗
                faceColor = D3DCOLOR_RGBA(
                    static_cast<BYTE>(color.r * 0.6f),
                    static_cast<BYTE>(color.g * 0.6f),
                    static_cast<BYTE>(color.b * 0.6f),
                    color.a
                );
                break;
            case 0: // 前面
            case 1: // 后面
                faceColor = D3DCOLOR_RGBA(
                    static_cast<BYTE>(color.r * 0.8f),
                    static_cast<BYTE>(color.g * 0.8f),
                    static_cast<BYTE>(color.b * 0.8f),
                    color.a
                );
                break;
            case 2: // 左面
            case 3: // 右面
                faceColor = D3DCOLOR_RGBA(
                    static_cast<BYTE>(color.r * 0.7f),
                    static_cast<BYTE>(color.g * 0.7f),
                    static_cast<BYTE>(color.b * 0.7f),
                    color.a
                );
                break;
        }
        
        // 对于透明方块（如水、树叶等），确保Alpha混合正确设置
        bool useAlpha = (color.a < 255);
        if (useAlpha) {
            d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
            d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
            d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        } else {
            d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        }
        
        // 创建顶点数据
        VertexPositionColor verts[6];
        
        // 两个三角形组成一个面
        verts[0].x = vertices[0].x; verts[0].y = vertices[0].y; verts[0].z = vertices[0].z; verts[0].color = faceColor;
        verts[1].x = vertices[1].x; verts[1].y = vertices[1].y; verts[1].z = vertices[1].z; verts[1].color = faceColor;
        verts[2].x = vertices[2].x; verts[2].y = vertices[2].y; verts[2].z = vertices[2].z; verts[2].color = faceColor;
        
        verts[3].x = vertices[0].x; verts[3].y = vertices[0].y; verts[3].z = vertices[0].z; verts[3].color = faceColor;
        verts[4].x = vertices[2].x; verts[4].y = vertices[2].y; verts[4].z = vertices[2].z; verts[4].color = faceColor;
        verts[5].x = vertices[3].x; verts[5].y = vertices[3].y; verts[5].z = vertices[3].z; verts[5].color = faceColor;
        
        // 设置顶点格式
        d3dDevice->SetFVF(VertexPositionColor::FVF);
        
        // 使用DrawPrimitiveUP绘制三角形列表
        d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 2, verts, sizeof(VertexPositionColor));
        
        drawCalls++;
        trianglesRendered += 2;
    }
    
    // 绘制简单形状 - 横线
    void DrawLine(int x1, int y1, int x2, int y2, const Color& color) {
        if (!d3dDevice || !font) return;
        
        // 使用ID3DXLine绘制线条会更高效，但为简单起见，我们使用Direct3D的普通绘制功能
        D3DXVECTOR2 vertices[2];
        vertices[0] = D3DXVECTOR2(static_cast<float>(x1), static_cast<float>(y1));
        vertices[1] = D3DXVECTOR2(static_cast<float>(x2), static_cast<float>(y2));
        
        // 设置正交投影矩阵以便绘制2D
        D3DXMATRIX orthoProj;
        D3DXMatrixOrthoLH(&orthoProj, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f);
        
        D3DXMATRIX oldView, oldProj, oldWorld;
        d3dDevice->GetTransform(D3DTS_VIEW, &oldView);
        d3dDevice->GetTransform(D3DTS_PROJECTION, &oldProj);
        d3dDevice->GetTransform(D3DTS_WORLD, &oldWorld);
        
        D3DXMATRIX identity;
        D3DXMatrixIdentity(&identity);
        
        d3dDevice->SetTransform(D3DTS_WORLD, &identity);
        d3dDevice->SetTransform(D3DTS_VIEW, &identity);
        d3dDevice->SetTransform(D3DTS_PROJECTION, &orthoProj);
        
        d3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        
        // 创建顶点数据
        struct Line2DVertex {
            float x, y, z, rhw;
            DWORD color;
        };
        
        Line2DVertex lineVerts[2] = {
            { static_cast<float>(x1), static_cast<float>(y1), 0.5f, 1.0f, D3DCOLOR_RGBA(color.r, color.g, color.b, color.a) },
            { static_cast<float>(x2), static_cast<float>(y2), 0.5f, 1.0f, D3DCOLOR_RGBA(color.r, color.g, color.b, color.a) }
        };
        
        d3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
        d3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, 1, lineVerts, sizeof(Line2DVertex));
        
        // 恢复之前的状态
        d3dDevice->SetTransform(D3DTS_WORLD, &oldWorld);
        d3dDevice->SetTransform(D3DTS_VIEW, &oldView);
        d3dDevice->SetTransform(D3DTS_PROJECTION, &oldProj);
        d3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
        
        drawCalls++;
    }
    
    // 绘制矩形
    void DrawRect(int x, int y, int width, int height, const Color& color) {
        if (!d3dDevice) return;
        
        // 设置正交投影矩阵以便绘制2D
        D3DXMATRIX orthoProj;
        D3DXMatrixOrthoLH(&orthoProj, static_cast<float>(this->width), static_cast<float>(this->height), 0.0f, 1.0f);
        
        D3DXMATRIX oldView, oldProj, oldWorld;
        d3dDevice->GetTransform(D3DTS_VIEW, &oldView);
        d3dDevice->GetTransform(D3DTS_PROJECTION, &oldProj);
        d3dDevice->GetTransform(D3DTS_WORLD, &oldWorld);
        
        D3DXMATRIX identity;
        D3DXMatrixIdentity(&identity);
        
        d3dDevice->SetTransform(D3DTS_WORLD, &identity);
        d3dDevice->SetTransform(D3DTS_VIEW, &identity);
        d3dDevice->SetTransform(D3DTS_PROJECTION, &orthoProj);
        
        d3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        
        // 如果需要透明度，设置混合状态
        if (color.a < 255) {
            d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
            d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
            d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        }
        
        // 创建顶点数据
        struct Rect2DVertex {
            float x, y, z, rhw;
            DWORD color;
        };
        
        // 矩形的两个三角形
        Rect2DVertex rectVerts[6] = {
            { static_cast<float>(x), static_cast<float>(y), 0.5f, 1.0f, D3DCOLOR_RGBA(color.r, color.g, color.b, color.a) },
            { static_cast<float>(x + width), static_cast<float>(y), 0.5f, 1.0f, D3DCOLOR_RGBA(color.r, color.g, color.b, color.a) },
            { static_cast<float>(x + width), static_cast<float>(y + height), 0.5f, 1.0f, D3DCOLOR_RGBA(color.r, color.g, color.b, color.a) },
            
            { static_cast<float>(x), static_cast<float>(y), 0.5f, 1.0f, D3DCOLOR_RGBA(color.r, color.g, color.b, color.a) },
            { static_cast<float>(x + width), static_cast<float>(y + height), 0.5f, 1.0f, D3DCOLOR_RGBA(color.r, color.g, color.b, color.a) },
            { static_cast<float>(x), static_cast<float>(y + height), 0.5f, 1.0f, D3DCOLOR_RGBA(color.r, color.g, color.b, color.a) }
        };
        
        d3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
        d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 2, rectVerts, sizeof(Rect2DVertex));
        
        // 恢复状态
        if (color.a < 255) {
            d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        }
        
        d3dDevice->SetTransform(D3DTS_WORLD, &oldWorld);
        d3dDevice->SetTransform(D3DTS_VIEW, &oldView);
        d3dDevice->SetTransform(D3DTS_PROJECTION, &oldProj);
        d3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
        
        drawCalls++;
    }
    
    // 绘制矩形边框
    void DrawRectOutline(int x, int y, int width, int height, const Color& color) {
        // 绘制四条线
        DrawLine(x, y, x + width - 1, y, color);                     // 上边
        DrawLine(x + width - 1, y, x + width - 1, y + height - 1, color); // 右边
        DrawLine(x, y + height - 1, x + width - 1, y + height - 1, color);   // 下边
        DrawLine(x, y, x, y + height - 1, color);                       // 左边
    }
    
    // 绘制文本
    void DrawText(int x, int y, const std::string& text, const Color& color) {
        if (!font) return;
        
        // 定义文本矩形
        RECT rect;
        rect.left = x;
        rect.top = y;
        rect.right = width;
        rect.bottom = height;
        
        // 绘制文本
        font->DrawTextA(NULL, text.c_str(), -1, &rect, DT_LEFT | DT_NOCLIP,
                       D3DCOLOR_RGBA(color.r, color.g, color.b, color.a));
        
        drawCalls++;
    }
    
    // 获取当前使用的适配器信息
    const AdapterInfo& GetAdapterInfo() const {
        return bestAdapter;
    }
    
    // 获取三角形和绘制调用次数
    int GetTrianglesRendered() const { return trianglesRendered; }
    int GetDrawCalls() const { return drawCalls; }
    
    // 绘制云面
    void DrawCloudFace(Vec3 vertices[4], const Color& color) {
        if (!d3dDevice) return;
        
        // 设置透明混合状态
        d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        
        // 禁用深度写入但保持深度测试
        d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
        
        // 准备顶点数据
        VertexPositionColor verts[4];
        D3DCOLOR cloudColor = D3DCOLOR_ARGB(color.a, color.r, color.g, color.b);
        
        // 转换顶点到屏幕空间
        for (int i = 0; i < 4; i++) {
            verts[i].x = vertices[i].x;
            verts[i].y = vertices[i].y;
            verts[i].z = vertices[i].z;
            verts[i].color = cloudColor;
        }
        
        // 绘制两个三角形组成的四边形
        d3dDevice->SetFVF(VertexPositionColor::FVF);
        d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, verts, sizeof(VertexPositionColor));
        
        // 恢复渲染状态
        d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
        
        // 更新统计信息
        trianglesRendered += 2;
        drawCalls++;
    }
    
    // 绘制太阳
    void DrawSun(Vec3 vertices[4], const Color& color) {
        if (!d3dDevice) return;
        
        // 保持深度测试启用，这样太阳会被方块遮挡
        d3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
        d3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
        
        // 设置透明混合状态（使太阳边缘更柔和）
        d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        
        // 准备顶点数据
        VertexPositionColor verts[4];
        D3DCOLOR sunColor = D3DCOLOR_ARGB(color.a, color.r, color.g, color.b);
        
        // 转换顶点到屏幕空间
        for (int i = 0; i < 4; i++) {
            verts[i].x = vertices[i].x;
            verts[i].y = vertices[i].y;
            verts[i].z = vertices[i].z;
            verts[i].color = sunColor;
        }
        
        // 绘制两个三角形组成的四边形
        d3dDevice->SetFVF(VertexPositionColor::FVF);
        d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, verts, sizeof(VertexPositionColor));
        
        // 恢复渲染状态
        d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        
        // 更新统计信息
        trianglesRendered += 2;
        drawCalls++;
    }
};

#endif // RENDER_GPU_H 