
//https://github.com/Enoticx1/CS2-DirectX-Chams

#pragma once
#include <Windows.h>
#include <vector>
#include <d3d11.h>
#include <dxgi.h>
#include <D3Dcompiler.h>
#pragma comment(lib, "D3dcompiler.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment( lib, "dxguid.lib")
#pragma comment(lib, "winmm.lib")
#include "MinHook/include/MinHook.h"
#include "ImGui\imgui.h"
#include "main.h"
#include "ImGui\backends\imgui_impl_win32.h"
#include "ImGui\backends\imgui_impl_dx11.h"
#include <DirectXMath.h>
using namespace DirectX;

#pragma warning( disable : 4244 )


typedef HRESULT(__stdcall* D3D11PresentHook) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
typedef HRESULT(__stdcall* D3D11ResizeBuffersHook) (IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);

typedef void(__stdcall* D3D11DrawIndexedHook) (ID3D11DeviceContext* pContext, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation);
typedef void(__stdcall* D3D11DrawIndexedInstancedHook) (ID3D11DeviceContext* pContext, UINT IndexCount, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation);
typedef void(__stdcall* D3D11DrawHook) (ID3D11DeviceContext* pContext, UINT IndexCount, UINT StartVertexLocation);
D3D11PresentHook phookD3D11Present = NULL;
D3D11ResizeBuffersHook phookD3D11ResizeBuffers = NULL;
D3D11DrawIndexedHook phookD3D11DrawIndexed = NULL;
D3D11DrawIndexedInstancedHook phookD3D11DrawIndexedInstanced = NULL;
D3D11DrawHook phookD3D11Draw = NULL;
ID3D11Device* pDevice = NULL;
ID3D11DeviceContext* pContext = NULL;
DWORD_PTR* pSwapChainVtable = NULL;
DWORD_PTR* pContextVTable = NULL;
DWORD_PTR* pDeviceVTable = NULL;
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
WNDPROC OWndProc = nullptr;

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (showmenu && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
	{
		ImGui::GetIO().MouseDrawCursor = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);
		return true;
	}

	if (bInit)
		ImGui::GetIO().MouseDrawCursor = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);

	if (uMsg == WM_CLOSE)
	{

		SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)OWndProc);
		TerminateProcess(GetCurrentProcess(), 0);
	}

	if (ImGui::GetIO().WantCaptureMouse)
	{
		if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
			return true;
		return false;
	}
	return CallWindowProc(OWndProc, hWnd, uMsg, wParam, lParam);
}

void InitImGuiD3D11()
{
	OWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.IniFilename = nullptr;
	io.Fonts->AddFontDefault();

	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(window);


	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX11_Init(pDevice, pContext);

	bInit = TRUE;
}



HRESULT __stdcall hookD3D11ResizeBuffers(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
	ImGui_ImplDX11_InvalidateDeviceObjects();
	if (nullptr != mainRenderTargetViewD3D11) { mainRenderTargetViewD3D11->Release(); mainRenderTargetViewD3D11 = nullptr; }

	HRESULT toReturn = phookD3D11ResizeBuffers(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);

	ImGui_ImplDX11_CreateDeviceObjects();

	return phookD3D11ResizeBuffers(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
}



HRESULT __stdcall hookD3D11Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	if (!initonce)
	{
		if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice)))
		{
			pDevice->GetImmediateContext(&pContext);
			DXGI_SWAP_CHAIN_DESC sd;
			pSwapChain->GetDesc(&sd);
			window = sd.OutputWindow;
			ID3D11Texture2D* pBackBuffer;
			pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
			pDevice->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetViewD3D11);
			pBackBuffer->Release();

			InitImGuiD3D11();


			D3D11_RASTERIZER_DESC Desc;
			Desc.AntialiasedLineEnable = 0;
			Desc.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
			Desc.DepthBiasClamp = 0.f;
			Desc.ScissorEnable = FALSE;
			Desc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
			Desc.DepthBias = LONG_MIN;
			Desc.DepthClipEnable = TRUE;
			Desc.FrontCounterClockwise = FALSE;
			pDevice->CreateRasterizerState(&Desc, &CustomState);

			D3D11_RASTERIZER_DESC WireframeDesc = Desc;
			WireframeDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_WIREFRAME;
			pDevice->CreateRasterizerState(&WireframeDesc, &WireframeState);


			D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
			depthStencilDesc.DepthEnable = FALSE;
			depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
			depthStencilDesc.StencilEnable = FALSE;
			depthStencilDesc.StencilReadMask = 0xFF;
			depthStencilDesc.StencilWriteMask = 0xFF;
			pDevice->CreateDepthStencilState(&depthStencilDesc, &DepthStencilState_FALSE);


			LoadCfg();

			initonce = true;
		}
		else
			return phookD3D11Present(pSwapChain, SyncInterval, Flags);
	}


	if (!sRed)
		GenerateShader(pDevice, &sRed, 1.0f, 0.0f, 0.0f);

	if (!sRedDark)
		GenerateShader(pDevice, &sRedDark, 0.2f, 0.0f, 0.0f);

	if (!sGreen)
		GenerateShader(pDevice, &sGreen, 0.0f, 1.0f, 0.0f);

	if (!sGreenDark)
		GenerateShader(pDevice, &sGreenDark, 0.0f, 0.2f, 0.0f);

	if (!sBlue)
		GenerateShader(pDevice, &sBlue, 0.0f, 0.0f, 1.0f);

	if (!sYellow)
		GenerateShader(pDevice, &sYellow, 1.0f, 1.0f, 0.0f);

	if (!sMagenta)
		GenerateShader(pDevice, &sMagenta, 1.0f, 0.0f, 1.0f);

	if (!sGrey)
		GenerateShader(pDevice, &sGrey, 0.3f, 0.3f, 0.3f);

	if (!sUniformColorVisible)
		GenerateShader(pDevice, &sUniformColorVisible, chams_color_visible[0], chams_color_visible[1], chams_color_visible[2]);
	
	if (!sUniformColorHidden)
		GenerateShader(pDevice, &sUniformColorHidden, chams_color_hidden[0], chams_color_hidden[1], chams_color_hidden[2]);
	
	if (!sUniformColorHands)
		GenerateShader(pDevice, &sUniformColorHands, chams_color_hands[0], chams_color_hands[1], chams_color_hands[2]);

	static float last_color_visible[3] = { -1.0f, -1.0f, -1.0f };
	static float last_color_hidden[3] = { -1.0f, -1.0f, -1.0f };
	static float last_color_hands[3] = { -1.0f, -1.0f, -1.0f };
	
	if (last_color_visible[0] != chams_color_visible[0] || last_color_visible[1] != chams_color_visible[1] || last_color_visible[2] != chams_color_visible[2])
	{
		if (sUniformColorVisible) {
			sUniformColorVisible->Release();
			sUniformColorVisible = NULL;
		}
		GenerateShader(pDevice, &sUniformColorVisible, chams_color_visible[0], chams_color_visible[1], chams_color_visible[2]);
		last_color_visible[0] = chams_color_visible[0];
		last_color_visible[1] = chams_color_visible[1];
		last_color_visible[2] = chams_color_visible[2];
	}
	
	if (last_color_hidden[0] != chams_color_hidden[0] || last_color_hidden[1] != chams_color_hidden[1] || last_color_hidden[2] != chams_color_hidden[2])
	{
		if (sUniformColorHidden) {
			sUniformColorHidden->Release();
			sUniformColorHidden = NULL;
		}
		GenerateShader(pDevice, &sUniformColorHidden, chams_color_hidden[0], chams_color_hidden[1], chams_color_hidden[2]);
		last_color_hidden[0] = chams_color_hidden[0];
		last_color_hidden[1] = chams_color_hidden[1];
		last_color_hidden[2] = chams_color_hidden[2];
	}
	
	if (last_color_hands[0] != chams_color_hands[0] || last_color_hands[1] != chams_color_hands[1] || last_color_hands[2] != chams_color_hands[2])
	{
		if (sUniformColorHands) {
			sUniformColorHands->Release();
			sUniformColorHands = NULL;
		}
		GenerateShader(pDevice, &sUniformColorHands, chams_color_hands[0], chams_color_hands[1], chams_color_hands[2]);
		last_color_hands[0] = chams_color_hands[0];
		last_color_hands[1] = chams_color_hands[1];
		last_color_hands[2] = chams_color_hands[2];
	}


	if (mainRenderTargetViewD3D11 == NULL)
	{
		ID3D11Texture2D* pBackBuffer = NULL;
		pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
		pDevice->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetViewD3D11);
		pBackBuffer->Release();
	}




	if (GetAsyncKeyState(VK_INSERT) & 1) {
		SaveCfg();
		showmenu = !showmenu;
	}


	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (showmenu)
	{
		ImGui::Begin("https://github.com/Enoticx1/CS2-DirectX-Chams", &showmenu);
		ImGui::SetWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
		if (vWindowPos.x != 0.0f && vWindowPos.y != 0.0f && vWindowSize.x != 0.0f && vWindowSize.y != 0.0f && bSetPos)
		{
			ImGui::SetWindowPos(vWindowPos);
			ImGui::SetWindowSize(vWindowSize);
			bSetPos = false;
		}
		if (bSetPos == false)
		{
			vWindowPos = { ImGui::GetWindowPos().x, ImGui::GetWindowPos().y };
			vWindowSize = { ImGui::GetWindowSize().x, ImGui::GetWindowSize().y };
		}
		ImGui::Checkbox("Wallhack", &wallhack);
		ImGui::Checkbox("Chams", &chams);
		ImGui::ColorEdit3("Visible Color", chams_color_visible, ImGuiColorEditFlags_NoInputs);
		ImGui::ColorEdit3("Hidden Color", chams_color_hidden, ImGuiColorEditFlags_NoInputs);
		ImGui::ColorEdit3("Hands Color", chams_color_hands, ImGuiColorEditFlags_NoInputs);
		const char* render_modes[] = { "Default", "Wireframe" };
		ImGui::Combo("Render Mode", &render_mode, render_modes, IM_ARRAYSIZE(render_modes));
		ImGui::End();
	}

	ImGui::EndFrame();
	ImGui::Render();
	pContext->OMSetRenderTargets(1, &mainRenderTargetViewD3D11, NULL);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	return phookD3D11Present(pSwapChain, SyncInterval, Flags);
}


void __stdcall hookD3D11DrawIndexed(ID3D11DeviceContext* pContext, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation)
{
	if (IndexCount > 0)
		check_drawindexed_result = 1;

	return phookD3D11DrawIndexed(pContext, IndexCount, StartIndexLocation, BaseVertexLocation);
}



void __stdcall hookD3D11DrawIndexedInstanced(ID3D11DeviceContext* pContext, UINT IndexCount, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation)
{
	if (IndexCount > 0)
		check_drawindexedinstanced_result = 1;


	ID3D11Buffer* veBuffer;
	UINT veWidth, Stride, veBufferOffset;
	D3D11_BUFFER_DESC veDesc;
	pContext->IAGetVertexBuffers(0, 1, &veBuffer, &Stride, &veBufferOffset);
	if (veBuffer) {
		veBuffer->GetDesc(&veDesc);
		veWidth = veDesc.ByteWidth;
	}
	if (NULL != veBuffer) {
		veBuffer->Release();
		veBuffer = NULL;
	}


	ID3D11PixelShader* pPixelShader = nullptr;
	pContext->PSGetShader(&pPixelShader, nullptr, nullptr);
	std::string debugName = GetDebugName(pPixelShader);
	if (pPixelShader)
	{
		pPixelShader->Release();

		ID3D11ShaderResourceView* pSRV = nullptr;
		pContext->PSGetShaderResources(0, 1, &pSRV);
		if (pSRV)
		{
			ID3D11Resource* pResource = nullptr;
			pSRV->GetResource(&pResource);
			if (pResource)
			{
				std::string debugName = GetDebugName(pResource);
				pResource->Release();
			}
			pSRV->Release();
		}
	}






#define T_Models (IndexCount == 3312 || IndexCount == 7218 || IndexCount == 8994 || IndexCount == 14184 || IndexCount == 19437)
#define CT_Models (IndexCount == 8160 || IndexCount == 8898 || IndexCount == 10242 || IndexCount == 21495)


	if (wallhack == 1 || chams == 1 || teamchams == 1)
		if (debugName == "csgo_character.vfx_ps")
		{
			if (render_mode == 1)
			{
				pContext->RSGetState(&OldState);
				pContext->RSSetState(WireframeState);
			}

			if (wallhack == 1)
			{
				pContext->OMGetDepthStencilState(&DepthStencilState_ORIG, &stencilRef);
				pContext->OMSetDepthStencilState(DepthStencilState_FALSE, stencilRef);
			}

			if (Stride != 28 && chams == 1)
			{
				pContext->PSGetShader(&oPixelShaderA, 0, 0);
				pContext->PSSetShader(sUniformColorHidden, NULL, NULL);
			}

			if (T_Models && Stride != 28 && teamchams == 1)
			{
				pContext->PSGetShader(&oPixelShaderA, 0, 0);
				pContext->PSSetShader(sUniformColorHidden, NULL, NULL);
			}

			if (CT_Models && Stride != 28 && teamchams == 1)
			{
				pContext->PSGetShader(&oPixelShaderA, 0, 0);
				pContext->PSSetShader(sUniformColorHidden, NULL, NULL);
			}

			if (Stride == 28 && chams == 1)
			{
				pContext->PSGetShader(&oPixelShaderA, 0, 0);
				pContext->PSSetShader(sUniformColorHands, NULL, NULL);
			}

			phookD3D11DrawIndexedInstanced(pContext, IndexCount, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);

			if (wallhack == 1)
			{
				pContext->OMSetDepthStencilState(DepthStencilState_ORIG, stencilRef);
			}

			if (Stride != 28 && chams == 1)
			{
				pContext->PSGetShader(&oPixelShaderA, 0, 0);
				pContext->PSSetShader(sUniformColorVisible, NULL, NULL);
			}

			if (T_Models && Stride != 28 && teamchams == 1)
			{
				pContext->PSGetShader(&oPixelShaderA, 0, 0);
				pContext->PSSetShader(sUniformColorVisible, NULL, NULL);
			}

			if (CT_Models && Stride != 28 && teamchams == 1)
			{
				pContext->PSGetShader(&oPixelShaderA, 0, 0);
				pContext->PSSetShader(sUniformColorVisible, NULL, NULL);
			}

			if (Stride == 28 && chams == 1)
			{
				pContext->PSGetShader(&oPixelShaderA, 0, 0);
				pContext->PSSetShader(sUniformColorHands, NULL, NULL);
			}

			phookD3D11DrawIndexedInstanced(pContext, IndexCount, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);

			if (render_mode == 1 && OldState)
			{
				pContext->RSSetState(OldState);
				OldState->Release();
			}
		}






	return phookD3D11DrawIndexedInstanced(pContext, IndexCount, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
}



void __stdcall hookD3D11Draw(ID3D11DeviceContext* pContext, UINT IndexCount, UINT StartVertexLocation)
{
	if (IndexCount > 0)
		check_draw_result = 1;

	return phookD3D11Draw(pContext, IndexCount, StartVertexLocation);
}



LRESULT CALLBACK DXGIMsgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) { return DefWindowProc(hwnd, uMsg, wParam, lParam); }
DWORD __stdcall InitializeHook(LPVOID)
{
	HMODULE hDXGIDLL = 0;
	do
	{
		hDXGIDLL = GetModuleHandle(L"dxgi.dll");
		Sleep(4000);
	} while (!hDXGIDLL);
	Sleep(100);



	IDXGISwapChain* pSwapChain;

	WNDCLASSEXA wc = { sizeof(WNDCLASSEX), CS_CLASSDC, DXGIMsgProc, 0L, 0L, GetModuleHandleA(NULL), NULL, NULL, NULL, NULL, "DX", NULL };
	RegisterClassExA(&wc);
	HWND hWnd = CreateWindowA("DX", NULL, WS_OVERLAPPEDWINDOW, 100, 100, 300, 300, NULL, NULL, wc.hInstance, NULL);

	D3D_FEATURE_LEVEL requestedLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1 };
	D3D_FEATURE_LEVEL obtainedLevel;
	ID3D11Device* d3dDevice = nullptr;
	ID3D11DeviceContext* d3dContext = nullptr;

	DXGI_SWAP_CHAIN_DESC scd;
	ZeroMemory(&scd, sizeof(scd));
	scd.BufferCount = 1;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	scd.OutputWindow = hWnd;
	scd.SampleDesc.Count = 1;
	scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scd.Windowed = ((GetWindowLongPtr(hWnd, GWL_STYLE) & WS_POPUP) != 0) ? false : true;

	scd.BufferDesc.Width = 1;
	scd.BufferDesc.Height = 1;
	scd.BufferDesc.RefreshRate.Numerator = 0;
	scd.BufferDesc.RefreshRate.Denominator = 1;

	UINT createFlags = 0;
#ifdef _DEBUG

	createFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	IDXGISwapChain* d3dSwapChain = 0;

	if (FAILED(D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		createFlags,
		requestedLevels,
		sizeof(requestedLevels) / sizeof(D3D_FEATURE_LEVEL),
		D3D11_SDK_VERSION,
		&scd,
		&pSwapChain,
		&pDevice,
		&obtainedLevel,
		&pContext)))
	{
		MessageBoxA(hWnd, "Failed to create DirectX Device and SwapChain!", "Error", MB_ICONERROR);
		return NULL;
	}

	pSwapChainVtable = (DWORD_PTR*)pSwapChain;
	pSwapChainVtable = (DWORD_PTR*)pSwapChainVtable[0];

	pContextVTable = (DWORD_PTR*)pContext;
	pContextVTable = (DWORD_PTR*)pContextVTable[0];

	pDeviceVTable = (DWORD_PTR*)pDevice;
	pDeviceVTable = (DWORD_PTR*)pDeviceVTable[0];

	if (MH_Initialize() != MH_OK) { return 1; }
	if (MH_CreateHook((DWORD_PTR*)pSwapChainVtable[8], hookD3D11Present, reinterpret_cast<void**>(&phookD3D11Present)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)pSwapChainVtable[8]) != MH_OK) { return 1; }
	if (MH_CreateHook((DWORD_PTR*)pSwapChainVtable[13], hookD3D11ResizeBuffers, reinterpret_cast<void**>(&phookD3D11ResizeBuffers)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)pSwapChainVtable[13]) != MH_OK) { return 1; }



	if (MH_CreateHook((DWORD_PTR*)pContextVTable[20], hookD3D11DrawIndexedInstanced, reinterpret_cast<void**>(&phookD3D11DrawIndexedInstanced)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)pContextVTable[20]) != MH_OK) { return 1; }







	DWORD dwOld;
	VirtualProtect(phookD3D11Present, 2, PAGE_EXECUTE_READWRITE, &dwOld);

	while (true) {
		Sleep(10);
	}

	pDevice->Release();
	pContext->Release();
	pSwapChain->Release();

	return NULL;
}



BOOL __stdcall DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		GetModuleFileNameW(hModule, (LPWSTR)dlldir, 512);
		for (size_t i = wcslen((wchar_t*)dlldir); i > 0; i--) { if (((wchar_t*)dlldir)[i] == L'\\') { ((wchar_t*)dlldir)[i + 1] = 0; break; } }
		CreateThread(NULL, 0, InitializeHook, NULL, 0, NULL);
		break;

	case DLL_PROCESS_DETACH:
		if (MH_Uninitialize() != MH_OK) { return 1; }
		if (MH_DisableHook((DWORD_PTR*)pSwapChainVtable[8]) != MH_OK) { return 1; }
		if (MH_DisableHook((DWORD_PTR*)pSwapChainVtable[13]) != MH_OK) { return 1; }


		if (MH_DisableHook((DWORD_PTR*)pContextVTable[20]) != MH_OK) { return 1; }


		break;
	}
	return TRUE;
}
