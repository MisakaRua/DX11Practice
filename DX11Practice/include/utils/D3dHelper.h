#pragma once
#include <string>
#include <vector>

#include "core/WinMin.h"
#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <DirectXColors.h>
#include <d3dcompiler.h>

static constexpr bool isDebugMode()
{
#if (defined(DEBUG) || defined(_DEBUG))
	return true;
#else
	return false;
#endif
}

#ifndef ENABLE_GFX_DBG_OBJ_NAME
#define ENABLE_GFX_DBG_OBJ_NAME 1
#endif
static constexpr bool enabledGraphicsDebugObjectName()
{
	return ENABLE_GFX_DBG_OBJ_NAME;
}

template <typename T>
static void safeRelease(Microsoft::WRL::ComPtr<T>& ptr)
{
	if (ptr)
	{
		ptr->Release();
		ptr = nullptr;
	}
}

template <size_t N>
static void D3D11SetDebugObjectName(_In_ ID3D11DeviceChild* resource, _In_ const char(&name)[N])
{
	if constexpr (isDebugMode() && enabledGraphicsDebugObjectName())
	{
		resource->SetPrivateData(WKPDID_D3DDebugObjectName, N - 1, name);
	}
	else
	{
		UNREFERENCED_PARAMETER(resource);
		UNREFERENCED_PARAMETER(name);
	}
}

static void D3D11SetDebugObjectName(_In_ ID3D11DeviceChild* resource, _In_ LPCSTR name, _In_ size_t length)
{
	if constexpr (isDebugMode() && enabledGraphicsDebugObjectName())
	{
		resource->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(length), name);
	}
	else
	{
		UNREFERENCED_PARAMETER(resource);
		UNREFERENCED_PARAMETER(name);
		UNREFERENCED_PARAMETER(length);
	}
}

static void D3D11SetDebugObjectName(_In_ ID3D11DeviceChild* resource, _In_ const std::string& name)
{
	if constexpr (isDebugMode() && enabledGraphicsDebugObjectName())
	{
		resource->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(name.size()), name.c_str());
	}
	else
	{
		UNREFERENCED_PARAMETER(resource);
		UNREFERENCED_PARAMETER(name);
	}
}

static void D3D11SetDebugObjectName(_In_ ID3D11DeviceChild* resource, _In_ const std::nullptr_t)
{
	if constexpr (isDebugMode() && enabledGraphicsDebugObjectName())
	{
		resource->SetPrivateData(WKPDID_D3DDebugObjectName, 0, nullptr);
	}
	else
	{
		UNREFERENCED_PARAMETER(resource);
	}
}

template <size_t N>
static void DXGISetDebugObjectName(_In_ IDXGIObject* resource, _In_ const char(&name)[N])
{
	if constexpr (isDebugMode() && enabledGraphicsDebugObjectName())
	{
		resource->SetPrivateData(WKPDID_D3DDebugObjectName, N - 1, name);
	}
	else
	{
		UNREFERENCED_PARAMETER(resource);
		UNREFERENCED_PARAMETER(name);
	}
}

static void DXGISetDebugObjectName(_In_ IDXGIObject* resource, _In_ LPCSTR name, _In_ size_t length)
{
	if constexpr (isDebugMode() && enabledGraphicsDebugObjectName())
	{
		resource->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(length), name);
	}
	else
	{
		UNREFERENCED_PARAMETER(resource);
		UNREFERENCED_PARAMETER(name);
		UNREFERENCED_PARAMETER(length);
	}
}

static void DXGISetDebugObjectName(_In_ IDXGIObject* resource, _In_ const std::string& name)
{
	if constexpr (isDebugMode() && enabledGraphicsDebugObjectName())
	{
		resource->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(name.size()), name.c_str());
	}
	else
	{
		UNREFERENCED_PARAMETER(resource);
		UNREFERENCED_PARAMETER(name);
	}
}

static void DXGISetDebugObjectName(_In_ IDXGIObject* resource, _In_ const std::nullptr_t)
{
	if constexpr (isDebugMode() && enabledGraphicsDebugObjectName())
	{
		resource->SetPrivateData(WKPDID_D3DDebugObjectName, 0, nullptr);
	}
	else
	{
		UNREFERENCED_PARAMETER(resource);
	}
}