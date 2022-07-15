#pragma once

#include "core/D3DApp.h"

class GameApp : public D3DApp
{
public:
	GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight);
	GameApp(const GameApp&) = delete;
	GameApp& operator=(const GameApp&) = delete;
	~GameApp() noexcept = default;

	bool init() override;
	void onResize() override;
	void updateScene(float dt) override;
	void drawScene() override;
};