#include "core/GameApp.h"
#include <cassert>

GameApp::GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight)
	: D3DApp(hInstance, windowName, initWidth, initHeight)
{

}

bool GameApp::init()
{
	if (!D3DApp::init())
	{
		return false;
	}
	return true;
}

void GameApp::onResize()
{
	D3DApp::onResize();
}

void GameApp::updateScene(float dt)
{

}

void GameApp::drawScene()
{
	assert(m_context);
	assert(m_swapchain);

	static float color[4] = { 0.1f, 0.1f, 0.1f, 1.0f };

	m_context->ClearRenderTargetView(m_render_target_view.Get(), color);
	m_context->ClearDepthStencilView(m_depth_stencil_view.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	m_swapchain->Present(0, 0);
}
