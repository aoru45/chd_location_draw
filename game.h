#pragma once
#include <unordered_map>
#include "hook.h"
#include <Windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <dwmapi.h>
#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"
#include "memory.h"
class Gamer {
  using Ptr = std::shared_ptr<Gamer>;

 private:
  std::shared_ptr<MemoryManager> mgr_ = MemoryManager::Create();
  std::unordered_map<int, bool> use_map_;
  ImDrawList* draw_list_ = nullptr;

 public:
  Gamer() = default;
  ~Gamer() = default;
  static Ptr Create();
  virtual bool Init(const std::string& process_name,
                    const std::string& module_name);
  virtual void Run();
  virtual void SetDrawList(ImDrawList* draw_list);
  void DrawBox(int x, int y, int w, int h, ImVec4 color, float T) {
    draw_list_->AddRect(ImVec2(x, y), ImVec2(x + w, y + h),
                        ImGui::ColorConvertFloat4ToU32(color), 0, 0, T);
  }

  void DrawCircle(int x, int y, float r, ImVec4 color, float T) {
    draw_list_->AddCircle(ImVec2(x, y), r,
                          ImGui::ColorConvertFloat4ToU32(color), 0, T);
  }

  void DrawLine(int x1, int y1, int x2, int y2, ImVec4 color, float T) {
    draw_list_->AddLine(ImVec2(x1, y1), ImVec2(x2, y2),
                        ImGui::ColorConvertFloat4ToU32(color), T);
  }

  void DrawD3DText(int x, int y, const char* str, ImVec4 color) {
    draw_list_->AddText(ImVec2(x, y), ImGui::ColorConvertFloat4ToU32(color),
                        str);
  }

  void DrawFillRect(int x, int y, int w, int h, ImVec4 color) {
    draw_list_->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h),
                              ImGui::ColorConvertFloat4ToU32(color), 0, 0);
  }
};

class GUI {
  using Ptr = std::shared_ptr<GUI>;

 private:
  static std::shared_ptr<Hooker> hook_Endcene_;
  static std::shared_ptr<Hooker> hook_Reset_;
  static std::shared_ptr<Gamer> gamer_;

  static WNDPROC prev_func_;
  static bool EnableShow;

  static HRESULT __stdcall Reset_(
      IDirect3DDevice9* direct3ddevice9,
      D3DPRESENT_PARAMETERS* pPresentationParameters);
  static HRESULT __stdcall EndScene_(IDirect3DDevice9* direct3ddevice9);
  static HRESULT CALLBACK ProcFunc(HWND hWnd, UINT uMsg, WPARAM wParam,
                                   LPARAM lParam);

 public:
  GUI() = default;
  static Ptr Create();

  virtual ~GUI();
  virtual bool Init(const std::string& process_name,
                    const std::string& module_name);
};
