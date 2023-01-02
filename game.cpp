#include "game.h"
#include <array>

std::shared_ptr<Hooker> GUI::hook_Endcene_;
std::shared_ptr<Hooker> GUI::hook_Reset_;
std::shared_ptr<Gamer> GUI::gamer_;
WNDPROC GUI::prev_func_;
bool GUI::EnableShow;
void get_window_size(HWND target, int& x, int& y, int& width, int& height) {
  RECT rect;
  GetWindowRect(target, &rect);
  x = rect.left;
  y = rect.top;
  width = rect.right - rect.left;
  height = rect.bottom - rect.top;
  if (GetWindowLongA(target, GWL_STYLE) & WS_CAPTION) {
    x += 8;
    width -= 8;
    y += 30;
    height -= 30;
  }
}

GUI::Ptr GUI::Create() {
  GUI::hook_Endcene_ = Hooker::Create();
  GUI::hook_Reset_ = Hooker::Create();
  GUI::gamer_ = Gamer::Create();
  prev_func_ = nullptr;
  EnableShow = true;
  return std::make_shared<GUI>();
}
Gamer::Ptr Gamer::Create() { return std::make_shared<Gamer>(); }
bool Gamer::Init(const std::string& process_name,
                 const std::string& module_name) {
  return mgr_->Init(process_name, module_name);
}
void Gamer::SetDrawList(ImDrawList* draw_list) { draw_list_ = draw_list; }
void Gamer::Run() {
  int map_id = mgr_->ReadMapId();
  if (use_map_.find(map_id) == use_map_.end()) {
    int have_enermy = mgr_->ReadEnermy();
    if (0 == have_enermy) {
      use_map_[map_id] = true;
    }
  } else {
    std::array<float, 2> p_wor;
    std::array<float, 2> p_scr;
    if (0 == mgr_->ReadSelf(p_wor, p_scr)) {
      DrawBox(p_scr[0] - 65, p_scr[1] - 180, 130, 230, ImVec4(0, 255, 0, 255),
              4);
    }
    for (int i = 1; i < 50; ++i) {
      std::array<float, 2> c_wor, c_scr;
      if (0 == mgr_->ReadCoordWor(c_wor, i)) {
        c_scr[0] = c_wor[0] + p_scr[0] - p_wor[0];
        c_scr[1] = c_wor[1] + p_scr[1] - p_wor[1];
        DrawBox(c_scr[0] - 60, c_scr[1] - 100, 120, 140, ImVec4(255, 0, 0, 255),
                4);
        DrawLine(c_scr[0], c_scr[1], p_scr[0], p_scr[1], ImVec4(0, 0, 255, 255),
                 4);
      }
    }
  }
}

GUI::~GUI() {
  hook_Endcene_->Restore();
  hook_Reset_->Restore();
}
bool GUI::Init(const std::string& process_name,
               const std::string& module_name) {
  D3DPRESENT_PARAMETERS i_present;
  HWND game_hwnd = FindWindow(NULL, "LaTale Client");
  // HWND overlay_hwnd = create_transparent_window(game_hwnd);
  int x, y, width, height;
  get_window_size(game_hwnd, x, y, width, height);
  IDirect3D9* i_direct3d9 = Direct3DCreate9(D3D_SDK_VERSION);
  IDirect3DDevice9* i_direct3ddevice9 = nullptr;
  memset(&i_present, 0, sizeof(i_present));
  i_present.Windowed = true;
  i_present.SwapEffect = D3DSWAPEFFECT_DISCARD;
  i_present.BackBufferFormat = D3DFMT_A8R8G8B8;
  i_present.BackBufferWidth = width;
  i_present.BackBufferHeight = height;
  i_present.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

  HRESULT result = i_direct3d9->CreateDevice(
      D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, game_hwnd,
      D3DCREATE_SOFTWARE_VERTEXPROCESSING, &i_present, &i_direct3ddevice9);

  ULL* direct3d9_table = (ULL*)*(ULL*)i_direct3d9;
  ULL* direct3ddevice9_table = (ULL*)*(ULL*)i_direct3ddevice9;
  hook_Reset_->Init(direct3ddevice9_table[16], ULL(Reset_));
  hook_Endcene_->Init(direct3ddevice9_table[42], ULL(EndScene_));
  hook_Reset_->Modify();
  hook_Endcene_->Modify();
  gamer_->Init(process_name, module_name);

  // mgr = new MemoryManager("LaTaleClient.exe", "LaTaleClient.exe");
  // auto pid = mgr->getPIDByName("LaTaleClient.exe");
  // mgr->init();

  // std::string title =
  //     "chdyz v2.8.70                           PID: " + to_string(pid);
  // plugin_hwnd = FindWindow(NULL, title.c_str());
  // plugin_hwnd = GetWindow(plugin_hwnd, GW_CHILD);
  return true;
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg,
                                              WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK GUI::ProcFunc(HWND hWnd, UINT uMsg, WPARAM wParam,
                               LPARAM lParam) {
  if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam)) return true;

  return CallWindowProcW(prev_func_, hWnd, uMsg, wParam, lParam);
}

HRESULT __stdcall GUI::Reset_(IDirect3DDevice9* direct3ddevice9,
                              D3DPRESENT_PARAMETERS* pPresentationParameters) {
  hook_Reset_->Restore();
  ImGui_ImplDX9_InvalidateDeviceObjects();
  HRESULT result = direct3ddevice9->Reset(pPresentationParameters);
  ImGui_ImplDX9_CreateDeviceObjects();
  hook_Reset_->Modify();
  return result;
}
HRESULT __stdcall GUI::EndScene_(IDirect3DDevice9* direct3ddevice9) {
  static bool is_first_call_ = true;
  if (is_first_call_) {
    is_first_call_ = false;
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    io.Fonts->AddFontFromFileTTF(
        "C:/Users/Administrator/AppData/Local/Microsoft/Windows/Fonts/"
        "1640582872194027.otf",
        14.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
    ImGui_ImplWin32_Init(FindWindow(NULL, "LaTale Client"));
    ImGui_ImplDX9_Init(direct3ddevice9);

    prev_func_ = (WNDPROC)SetWindowLongA(FindWindow(NULL, "LaTale Client"),
                                         GWL_WNDPROC, (ULL)ProcFunc);
  }
  hook_Endcene_->Restore();

  ImGui_ImplDX9_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();
  ImGui::Begin(u8"彩虹ar", nullptr,
               ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar |
                   ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse |
                   ImGuiWindowFlags_NoMove);
  ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);
  ImGui::Text(u8"彩虹ar");
  ImGui::Checkbox(u8"传送门透视", &EnableShow);
  ImGui::End();
  auto draw_list_ptr = ImGui::GetBackgroundDrawList();
  gamer_->SetDrawList(draw_list_ptr);

  if (EnableShow) {
    gamer_->Run();
  }
  ImGui::EndFrame();
  ImGui::Render();

  ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

  HRESULT result = direct3ddevice9->EndScene();
  hook_Endcene_->Modify();

  return result;
}
