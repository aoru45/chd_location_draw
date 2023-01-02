#include "game.h"
#include <process.h>
std::shared_ptr<GUI> gui = GUI::Create();
unsigned int __stdcall load(void* data) {
  gui->Init("LaTaleClient.exe", "LaTaleClient.exe");
  return 0;
};
void un_load() { ImGui::End(); }

int __stdcall DllMain(void* _DllHandle, unsigned long _Reason,
                      void* _Reserved) {
  if (_Reason == DLL_PROCESS_ATTACH) {
    _beginthreadex(nullptr, 0, load, nullptr, 0, nullptr);
  }

  if (_Reason == DLL_PROCESS_DETACH) un_load();

  return 1;
}
