#pragma once
#include <string>

#include "WinMin.h" // Should be the first Windows header to include
#include <wrl/client.h>
#include <d3d11_1.h>
#include <DirectXMath.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "D3DCompiler.lib")
#pragma comment(lib, "winmm.lib")

#include "utils/Timer.h"

class D3DApp
{
protected:
	template <class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

public:
	D3DApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight);
	D3DApp(const D3DApp&) = delete;
	D3DApp& operator=(const D3DApp&) = delete;
	virtual ~D3DApp() noexcept = default;

	HINSTANCE getAppHInstance() const { return m_app_h_inst; }  // 获取应用实例的句柄
	HWND      getMainHWND() const { return m_main_h_wnd; }      // 获取主窗口句柄
	float     getAspectRatio() const { return (float)m_client_width / (float)m_client_height; } // 获取屏幕宽高比

	int run();                                // 运行程序，进行游戏主循环

	// 框架方法。客户派生类需要重载这些方法以实现特定的应用需求
	virtual bool init();                      // 该父类方法需要初始化窗口和Direct3D部分
	virtual void onResize();                  // 该父类方法需要在窗口大小变动的时候调用
	virtual void updateScene(float dt) = 0;   // 子类需要实现该方法，完成每一帧的更新
	virtual void drawScene() = 0;             // 子类需要实现该方法，完成每一帧的绘制

	virtual LRESULT msgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);	// 窗口的消息回调函数

protected:
	bool initMainWindow();       // 窗口初始化
	bool initDirect3D();         // Direct3D初始化

	void calculateFrameStats();  // 计算每秒帧数并在窗口显示

protected:
	HINSTANCE m_app_h_inst;					// 应用实例句柄
	HWND      m_main_h_wnd = nullptr;		// 主窗口句柄
	bool      m_app_paused = false;			// 应用是否暂停
	bool      m_minimized = false;			// 应用是否最小化
	bool      m_maximized = false;			// 应用是否最大化
	bool      m_resizing = false;			// 窗口大小是否变化
	bool      m_enable_4x_msaa = true;		// 是否开启4倍多重采样
	UINT      m_4x_msaa_quality = 0;		// MSAA支持的质量等级


	Timer m_timer;           // 计时器

	
	// Direct3D 11
	ComPtr<ID3D11Device> m_device;                        // D3D11设备
	ComPtr<ID3D11DeviceContext> m_context;                // D3D11设备上下文
	ComPtr<IDXGISwapChain> m_swapchain;                   // D3D11交换链
	// Direct3D 11.1									  
	ComPtr<ID3D11Device1> m_device1;                      // D3D11.1设备
	ComPtr<ID3D11DeviceContext1> m_context1;              // D3D11.1设备上下文
	ComPtr<IDXGISwapChain1> m_swapchain1;                 // D3D11.1交换链
	// 常用资源
	ComPtr<ID3D11Texture2D> m_depth_stencil_buffer;       // 深度模板缓冲区
	ComPtr<ID3D11RenderTargetView> m_render_target_view;  // 渲染目标视图
	ComPtr<ID3D11DepthStencilView> m_depth_stencil_view;  // 深度模板视图
	D3D11_VIEWPORT m_screen_viewport;                     // 视口

	// 派生类应该在构造函数设置好这些自定义的初始参数
	std::wstring m_main_wnd_caption;                      // 主窗口标题
	int m_client_width;                                   // 视口宽度
	int m_client_height;                                  // 视口高度
};