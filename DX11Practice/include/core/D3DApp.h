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

	HINSTANCE getAppHInstance() const { return m_app_h_inst; }  // ��ȡӦ��ʵ���ľ��
	HWND      getMainHWND() const { return m_main_h_wnd; }      // ��ȡ�����ھ��
	float     getAspectRatio() const { return (float)m_client_width / (float)m_client_height; } // ��ȡ��Ļ��߱�

	int run();                                // ���г��򣬽�����Ϸ��ѭ��

	// ��ܷ������ͻ���������Ҫ������Щ������ʵ���ض���Ӧ������
	virtual bool init();                      // �ø��෽����Ҫ��ʼ�����ں�Direct3D����
	virtual void onResize();                  // �ø��෽����Ҫ�ڴ��ڴ�С�䶯��ʱ�����
	virtual void updateScene(float dt) = 0;   // ������Ҫʵ�ָ÷��������ÿһ֡�ĸ���
	virtual void drawScene() = 0;             // ������Ҫʵ�ָ÷��������ÿһ֡�Ļ���

	virtual LRESULT msgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);	// ���ڵ���Ϣ�ص�����

protected:
	bool initMainWindow();       // ���ڳ�ʼ��
	bool initDirect3D();         // Direct3D��ʼ��

	void calculateFrameStats();  // ����ÿ��֡�����ڴ�����ʾ

protected:
	HINSTANCE m_app_h_inst;					// Ӧ��ʵ�����
	HWND      m_main_h_wnd = nullptr;		// �����ھ��
	bool      m_app_paused = false;			// Ӧ���Ƿ���ͣ
	bool      m_minimized = false;			// Ӧ���Ƿ���С��
	bool      m_maximized = false;			// Ӧ���Ƿ����
	bool      m_resizing = false;			// ���ڴ�С�Ƿ�仯
	bool      m_enable_4x_msaa = true;		// �Ƿ���4�����ز���
	UINT      m_4x_msaa_quality = 0;		// MSAA֧�ֵ������ȼ�


	Timer m_timer;           // ��ʱ��

	
	// Direct3D 11
	ComPtr<ID3D11Device> m_device;                        // D3D11�豸
	ComPtr<ID3D11DeviceContext> m_context;                // D3D11�豸������
	ComPtr<IDXGISwapChain> m_swapchain;                   // D3D11������
	// Direct3D 11.1									  
	ComPtr<ID3D11Device1> m_device1;                      // D3D11.1�豸
	ComPtr<ID3D11DeviceContext1> m_context1;              // D3D11.1�豸������
	ComPtr<IDXGISwapChain1> m_swapchain1;                 // D3D11.1������
	// ������Դ
	ComPtr<ID3D11Texture2D> m_depth_stencil_buffer;       // ���ģ�建����
	ComPtr<ID3D11RenderTargetView> m_render_target_view;  // ��ȾĿ����ͼ
	ComPtr<ID3D11DepthStencilView> m_depth_stencil_view;  // ���ģ����ͼ
	D3D11_VIEWPORT m_screen_viewport;                     // �ӿ�

	// ������Ӧ���ڹ��캯�����ú���Щ�Զ���ĳ�ʼ����
	std::wstring m_main_wnd_caption;                      // �����ڱ���
	int m_client_width;                                   // �ӿڿ��
	int m_client_height;                                  // �ӿڸ߶�
};