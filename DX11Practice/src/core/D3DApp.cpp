#include "core/D3DApp.h"
#include <thread>
#include <cassert>
#include <vector>
#include <sstream>

#include <WinUser.h>

#include "utils/D3dHelper.h"
#include "utils/DXTrace.h"

extern "C"
{
	// 在具有多显卡的硬件设备中，优先使用NVIDIA或AMD的显卡运行
	// 需要在.exe中使用
	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 0x00000001;
}

namespace
{
	D3DApp* g_app = nullptr;
}

LRESULT CALLBACK MyWndProc(HWND h_wnd, UINT msg, WPARAM w_param, LPARAM l_param)
{
	return g_app->msgProc(h_wnd, msg, w_param, l_param);
}

D3DApp::D3DApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight)
	: m_app_h_inst(hInstance)
	, m_main_wnd_caption(windowName)
	, m_client_width(initWidth)
	, m_client_height(initHeight)
{
	ZeroMemory(&m_screen_viewport, sizeof(m_screen_viewport));
	g_app = this;
}

int D3DApp::run()
{
	MSG msg{};
	m_timer.mark();
	while (true)
	{
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT)
		{
			break;
		}

		if (!m_app_paused)
		{
			calculateFrameStats();
			updateScene(m_timer.mark());
			drawScene();
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}

	return static_cast<int>(msg.wParam);
}

bool D3DApp::init()
{
	if (!initMainWindow())
	{
		return false;
	}

	if (!initDirect3D())
	{
		return false;
	}

	return true;
}

void D3DApp::onResize()
{
	assert(m_device);
	assert(m_context);
	assert(m_swapchain);

	if (m_device1)
	{
		assert(m_device1);
		assert(m_context1);
		assert(m_swapchain1);
	}

	// Release pipeline related resource.
	m_render_target_view.Reset();
	m_depth_stencil_view.Reset();
	m_depth_stencil_buffer.Reset();

	// Reset swapchain and recreate render target view.
	ComPtr<ID3D11Texture2D> back_buffer;
	HR(m_swapchain->ResizeBuffers(1, m_client_width, m_client_height, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
	HR(m_swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), &back_buffer));
	HR(m_device->CreateRenderTargetView(back_buffer.Get(), nullptr, &m_render_target_view));

	// Set debug object name.
	D3D11SetDebugObjectName(back_buffer.Get(), "BackBuffer[0]");
	back_buffer.Reset();


	// Create depth and stencil buffer and buffer view.
	{
		D3D11_TEXTURE2D_DESC desc{};
		desc.Width = m_client_width;
		desc.Height = m_client_height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		desc.SampleDesc.Count = m_enable_4x_msaa ? 4 : 1;
		desc.SampleDesc.Quality = m_enable_4x_msaa ? (m_4x_msaa_quality - 1) : 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		HR(m_device->CreateTexture2D(&desc, nullptr, &m_depth_stencil_buffer));
		HR(m_device->CreateDepthStencilView(m_depth_stencil_buffer.Get(), nullptr, &m_depth_stencil_view));
	}


	// Set depth and stencil view to the context.
	// Here m_render_target_view is used like an array, the dsv will be set to each element of the array.
	m_context->OMSetRenderTargets(1, m_render_target_view.GetAddressOf(), m_depth_stencil_view.Get());


	// Set viewport.
	m_screen_viewport.TopLeftX = 0;
	m_screen_viewport.TopLeftY = 0;
	m_screen_viewport.Width = static_cast<float>(m_client_width);
	m_screen_viewport.Height = static_cast<float>(m_client_height);
	m_screen_viewport.MinDepth = 0.0f;
	m_screen_viewport.MaxDepth = 1.0f;

	m_context->RSSetViewports(1, &m_screen_viewport);
}

LRESULT D3DApp::msgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;

	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			m_app_paused = true;
			m_timer.pause();
		}
		else
		{
			m_app_paused = false;
			m_timer.start();
		}
		return 0;

		// WM_SIZE is sent when the user resizes the window.  
	case WM_SIZE:
		// Save the new client area dimensions.
		m_client_width = LOWORD(lParam);
		m_client_height = HIWORD(lParam);
		if (m_device)
		{
			if (wParam == SIZE_MINIMIZED)
			{
				m_app_paused = true;
				m_minimized = true;
				m_maximized = false;
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				m_app_paused = false;
				m_minimized = false;
				m_maximized = true;
				onResize();
			}
			else if (wParam == SIZE_RESTORED)
			{

				// Restoring from minimized state?
				if (m_minimized)
				{
					m_app_paused = false;
					m_minimized = false;
					onResize();
				}

				// Restoring from maximized state?
				else if (m_maximized)
				{
					m_app_paused = false;
					m_maximized = false;
					onResize();
				}
				else if (m_resizing)
				{
					// If user is dragging the resize bars, we do not resize 
					// the buffers here because as the user continuously 
					// drags the resize bars, a stream of WM_SIZE messages are
					// sent to the window, and it would be pointless (and slow)
					// to resize for each WM_SIZE message received from dragging
					// the resize bars.  So instead, we reset after the user is 
					// done resizing the window and releases the resize bars, which 
					// sends a WM_EXITSIZEMOVE message.
				}
				else // API call such as SetWindowPos or m_pSwapChain->SetFullscreenState.
				{
					onResize();
				}
			}
		}
		return 0;

		// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
	case WM_ENTERSIZEMOVE:
		m_app_paused = true;
		m_resizing = true;
		m_timer.pause();
		return 0;

		// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
		// Here we reset everything based on the new window dimensions.
	case WM_EXITSIZEMOVE:
		m_app_paused = false;
		m_resizing = false;
		m_timer.start();
		onResize();
		return 0;

		// WM_DESTROY is sent when the window is being destroyed.
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

		// The WM_MENUCHAR message is sent when a menu is active and the user presses 
		// a key that does not correspond to any mnemonic or accelerator key. 
	case WM_MENUCHAR:
		// Don't beep when we alt-enter.
		return MAKELRESULT(0, 1); // #define MNC_CLOSE 1

		// Catch this message so to prevent the window from becoming too small.
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		return 0;
	case WM_MOUSEMOVE:
		return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool D3DApp::initMainWindow()
{
	static constexpr wchar_t k_wnd_class_name[] = L"DX11 Practice";

	WNDCLASS wc{};
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MyWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_app_h_inst;
	wc.hIcon = nullptr;
	wc.hCursor = nullptr;
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = k_wnd_class_name;

	if (!RegisterClass(&wc))
	{
		MessageBox(0, L"Register Class Failed.", 0, 0);
		return false;
	}


	RECT rect{ 0, 0, m_client_width, m_client_height };
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	m_main_h_wnd = CreateWindow(
		k_wnd_class_name,
		m_main_wnd_caption.c_str(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		width,
		height,
		nullptr,
		nullptr,
		m_app_h_inst,
		nullptr);
	if (!m_main_h_wnd)
	{
		auto hr = GetLastError();
		MessageBox(nullptr, L"Create Window Failed.", nullptr, 0);
		return false;
	}

	ShowWindow(m_main_h_wnd, SW_SHOW);
	UpdateWindow(m_main_h_wnd);

	return true;
}

bool D3DApp::initDirect3D()
{
	// Create Device and Context.
	{
		UINT create_device_flags = 0
			| (isDebugMode() ? D3D11_CREATE_DEVICE_DEBUG : 0)
			;

		std::vector<D3D_DRIVER_TYPE> driver_types
		{
			D3D_DRIVER_TYPE_HARDWARE,
			D3D_DRIVER_TYPE_WARP,
			D3D_DRIVER_TYPE_REFERENCE,
		};

		std::vector<D3D_FEATURE_LEVEL> feature_levels
		{
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
		};

		D3D_FEATURE_LEVEL feature_level{};

		HRESULT hr;
		for (size_t i = 0, n = driver_types.size(); i < n; ++i)
		{
			hr = D3D11CreateDevice(
				nullptr,
				driver_types[i],
				nullptr,
				create_device_flags,
				feature_levels.data(),
				static_cast<UINT>(feature_levels.size()),
				D3D11_SDK_VERSION,
				&m_device,
				&feature_level,
				&m_context
			);

			if (hr == E_INVALIDARG)
			{
				hr = D3D11CreateDevice(
					nullptr,
					driver_types[i],
					nullptr,
					create_device_flags,
					feature_levels.data() + 1,
					static_cast<UINT>(feature_levels.size() - 1),
					D3D11_SDK_VERSION,
					&m_device,
					&feature_level,
					&m_context
				);
			}

			if (SUCCEEDED(hr))
			{
				break;
			}
		}

		if (FAILED(hr))
		{
			MessageBox(0, L"D3D11CreateDevice Failed.", nullptr, 0);
			return false;
		}

		// Check if feature 11.0 or 11.1 is supported.
		if (feature_level != D3D_FEATURE_LEVEL_11_1 && feature_level != D3D_FEATURE_LEVEL_11_0)
		{
			MessageBox(0, L"Direct 3D Feature Level 11 is not supported.", nullptr, 0);
			return false;
		}


		// MSAA supporting checking.
		m_device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m_4x_msaa_quality);
		assert(m_4x_msaa_quality > 0);


		ComPtr<IDXGIDevice> dxgi_device;
		ComPtr<IDXGIAdapter> dxgi_adapter;
		ComPtr<IDXGIFactory1> dxgi_factory1;
		ComPtr<IDXGIFactory2> dxgi_factory2;

		// Get the dxgi factory of the d3d11 device.
		HR(m_device.As(&dxgi_device));
		HR(dxgi_device->GetAdapter(&dxgi_adapter));
		HR(dxgi_adapter->GetParent(__uuidof(IDXGIFactory1), &dxgi_factory1));

		dxgi_factory1.As(&dxgi_factory2);
		if (dxgi_factory2)
		{
			HR(m_device.As(&m_device1));
			HR(m_context.As(&m_context1));

			// Create Swapchain.
			DXGI_SWAP_CHAIN_DESC1 sd{};
			sd.Width = m_client_width;
			sd.Height = m_client_height;
			sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			//sd.Stereo;
			sd.SampleDesc.Count = m_enable_4x_msaa ? 4 : 1;
			sd.SampleDesc.Quality = m_enable_4x_msaa ? (m_4x_msaa_quality - 1) : 0;
			sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			sd.BufferCount = 1;
			//sd.Scaling;
			sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			//sd.AlphaMode;
			sd.Flags = 0;

			DXGI_SWAP_CHAIN_FULLSCREEN_DESC fd{};
			fd.RefreshRate.Numerator = 60;
			fd.RefreshRate.Denominator = 1;
			fd.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			fd.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
			fd.Windowed = TRUE;

			HR(dxgi_factory2->CreateSwapChainForHwnd(m_device.Get(), m_main_h_wnd, &sd, &fd, nullptr, &m_swapchain1));
			HR(m_swapchain1.As(&m_swapchain));
		}
		else
		{
			DXGI_SWAP_CHAIN_DESC sd{};
			sd.BufferDesc.Width = m_client_width;
			sd.BufferDesc.Height = m_client_height;
			sd.BufferDesc.RefreshRate.Numerator = 60;
			sd.BufferDesc.RefreshRate.Denominator = 1;
			sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
			sd.SampleDesc.Count = m_enable_4x_msaa ? 4 : 1;
			sd.SampleDesc.Quality = m_enable_4x_msaa ? (m_4x_msaa_quality - 1) : 0;
			sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			sd.BufferCount = 1;
			sd.OutputWindow = m_main_h_wnd;
			sd.Windowed = TRUE;
			sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			sd.Flags = 0;

			HR(dxgi_factory1->CreateSwapChain(m_device.Get(), &sd, &m_swapchain));
		}


		// Forbid alt + enter to full screen.
		dxgi_factory1->MakeWindowAssociation(m_main_h_wnd, DXGI_MWA_NO_ALT_ENTER | DXGI_MWA_NO_WINDOW_CHANGES);


		D3D11SetDebugObjectName(m_context.Get(), "Context");
		DXGISetDebugObjectName(m_swapchain.Get(), "SwapChain");

		onResize();

		return true;
	}
}

void D3DApp::calculateFrameStats()
{
	static size_t frame_count = 0;
	static float time_elapsed = 0.0f;

	++frame_count;
	float curr_time = m_timer.totalTime();
	if ((curr_time - time_elapsed) >= 1.0f)
	{
		float fps = static_cast<float>(frame_count);
		float mspf = 1000.0f / fps;

		std::wstringstream oss;
		oss << m_main_wnd_caption << L"\t  "
			<< L"FPS: " << fps << L"\t  "
			<< L"Frame Time: " << mspf << L" ms";
		SetWindowText(m_main_h_wnd, oss.str().c_str());

		frame_count = 0;
		time_elapsed = curr_time;
	}
}
