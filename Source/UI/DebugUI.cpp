#include "pch.h"
#include "DebugUI.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "Utill/console.h"

std::map<const char*, std::any> DebugUI::gDatas;

/// <summary>
/// UI를 구현하는 함수입니다.
/// </summary>
void ShowDebugUI();

void DebugUI::Init(const HWND& hWnd, ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dContext)
{
	//*** imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(d3dDevice, d3dContext);
}

void DebugUI::Render()
{
	ImGui_ImplWin32_NewFrame();
	ImGui_ImplDX11_NewFrame();

	ImGui::NewFrame();
	ShowDebugUI();

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void DebugUI::Release()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

// https://pthom.github.io/imgui_manual_online/manual/imgui_manual.html
// 여기서 UI를 구현합니다.
void ShowDebugUI()
{
	IM_ASSERT(ImGui::GetCurrentContext() != NULL && "Missing Dear ImGui context. Refer to examples app!");

	IMGUI_CHECKVERSION();

	ImGui::Begin("DebugUI");
	ImGui::Text("FPS (%d)", DebugUI::GetData<int>("FPS"));
	ImGui::End();
}

