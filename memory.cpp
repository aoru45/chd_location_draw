#include "memory.h"
#include <TlHelp32.h>
#include <vector>
#include <array>
#include <Psapi.h>
MemoryManager::Ptr MemoryManager::Create() {
  return std::make_shared<MemoryManager>();
}

bool MemoryManager::Init(const std::string& process_name,
                         const std::string& module_name) {
  process_name_ = process_name;
  module_name_ = module_name;
  process_id_ = 0;

  // get pid
  HANDLE hProcessSnap;
  PROCESSENTRY32 pe32;
  hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hProcessSnap == INVALID_HANDLE_VALUE) {
    return (FALSE);
  }
  pe32.dwSize = sizeof(PROCESSENTRY32);
  if (!Process32First(hProcessSnap, &pe32)) {
    CloseHandle(hProcessSnap);
    return (FALSE);
  }
  BOOL getPID = FALSE;
  do {
    if (!strcmp(process_name_.c_str(), pe32.szExeFile)) {
      process_id_ = pe32.th32ProcessID;
      getPID = TRUE;
      break;
    }

  } while (Process32Next(hProcessSnap, &pe32));
  CloseHandle(hProcessSnap);
  if (!getPID) {
    throw exception("PID error");
  }

  // get handle
  process_handle_ = OpenProcess(PROCESS_ALL_ACCESS, false, process_id_);

  // get module_base
  HANDLE hModuleSnap = INVALID_HANDLE_VALUE;
  MODULEENTRY32 me32;
  hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, process_id_);
  if (hModuleSnap == INVALID_HANDLE_VALUE) {
    throw exception("[ERROR] Failed to CreateToolhelp32Snapshot");
  }
  me32.dwSize = sizeof(MODULEENTRY32);
  if (!Module32First(hModuleSnap, &me32)) {
    throw exception("[ERROR] Failed to Module32First");
  }
  do {
    if (!strcmp(me32.szModule, module_name_.c_str())) {
      CloseHandle(hModuleSnap);
      process_module_base_ = (DWORD)me32.modBaseAddr;
      return true;
    }
  } while (Module32Next(hModuleSnap, &me32));
  CloseHandle(hModuleSnap);

  return true;
}

MemoryManager::~MemoryManager() { CloseHandle(process_handle_); }

int MemoryManager::ReadSelf(std::array<float, 2>& p_wor,
                            std::array<float, 2>& p_scr) {
  DWORD caddr = process_module_base_;
  std::vector<DWORD> coffsets{(DWORD)0x565254, (DWORD)0x7C, (DWORD)0x104,
                              (DWORD)0x32C};
  for (int i = 0; i < coffsets.size() - 1; ++i) {
    // std::cout << hex << caddr << std::endl;
    caddr = Readmemory<DWORD>(caddr + coffsets[i]);
    if (0 == caddr) return -1;
  }
  DWORD cdx = Readmemory<DWORD>(caddr + coffsets.back());
  DWORD cdy = Readmemory<DWORD>(caddr + coffsets.back() + 0x04);
  p_scr[0] = cdx;
  p_scr[1] = cdy;

  DWORD waddr = process_module_base_;
  std::vector<DWORD> woffsets{(DWORD)0x565254, (DWORD)0x7C, (DWORD)0xF4,
                              (DWORD)0x08};
  for (int i = 0; i < woffsets.size() - 1; ++i) {
    // std::cout << hex << addrx << std::endl;
    waddr = Readmemory<DWORD>(waddr + woffsets[i]);
    if (0 == waddr) return -1;
  }
  float wdx = Readmemory<float>(waddr + woffsets.back());
  float wdy = Readmemory<float>(waddr + woffsets.back() + 0x04);
  p_wor[0] = wdx;
  p_wor[1] = wdy;
  return 0;
}
int MemoryManager::ReadCoordWor(std::array<float, 2>& c_wor, int idx) {
  DWORD waddr = process_module_base_;
  std::vector<DWORD> woffsets{
      (DWORD)0x565254, (DWORD)0x34, (DWORD)(0x100000000 - idx * 4),
      (DWORD)0x4C,     (DWORD)0xB8, (DWORD)0x244,
      (DWORD)0xBC};

  for (int i = 0; i < woffsets.size() - 1; ++i) {
    // std::cout << hex << addrx << std::endl;
    waddr = Readmemory<DWORD>(waddr + woffsets[i]);
    if (0 == waddr) return -1;
  }

  float wdx = Readmemory<float>(waddr + woffsets.back());
  if (wdx < 32.f || wdx >= 10000 || isnan(wdx)) return -1;
  float wdy = Readmemory<float>(waddr + woffsets.back() + 0x04);
  if (wdy < 32.f || wdy >= 10000 || isnan(wdy)) return -1;

  DWORD is_entrance = Readmemory<DWORD>(waddr + woffsets.back() + 0x20);
  if (1 != is_entrance) return -1;

  c_wor[0] = wdx;
  c_wor[1] = wdy;
  return 0;
}
int MemoryManager::ReadCoordScr(std::array<float, 2>& c_scr, int idx,
                                int scr_width, int scr_height) {
  DWORD saddr = process_module_base_;
  std::vector<DWORD> soffsets{(DWORD)0x565254, (DWORD)0x30,
                              (DWORD)0x00 + idx * 4, (DWORD)0x208, (DWORD)0x20};
  for (int i = 0; i < soffsets.size() - 1; ++i) {
    // std::cout << hex << addrx << std::endl;
    saddr = Readmemory<DWORD>(saddr + soffsets[i]);
    if (0 == saddr) return -1;
  }
  float sdx = Readmemory<float>(saddr + soffsets.back());
  if (sdx < 32 || sdx > scr_width || isnan(sdx)) return -1;
  float sdy = Readmemory<float>(saddr + soffsets.back() + 0x04);
  if (sdy < 32 || sdy >= scr_height || isnan(sdy)) return -1;
  DWORD is_entrance = Readmemory<DWORD>(saddr + soffsets.back() + 0x18);
  // DWORD is_entrance_2 = Readmemory<DWORD>(saddr + soffsets.back() + 0x2C);
  // DWORD is_entrance_3 = Readmemory<DWORD>(saddr + soffsets.back() + 0x58);
  if (0 != is_entrance) return -1;
  // if (0 == is_entrance_2) return -1;
  // if (0 == is_entrance_3) return -1;
  c_scr[0] = sdx;
  c_scr[1] = sdy;
  return 0;
}

int MemoryManager::ReadEnermy() {
  DWORD waddr = process_module_base_;
  std::vector<DWORD> woffsets{(DWORD)0x565254, (DWORD)0x24, (DWORD)(0x0),
                              (DWORD)0x68, (DWORD)0x08};

  for (int i = 0; i < woffsets.size() - 1; ++i) {
    // std::cout << hex << addrx << std::endl;
    waddr = Readmemory<DWORD>(waddr + woffsets[i]);
    if (0 == waddr) return -1;
  }

  float wdx = Readmemory<float>(waddr + woffsets.back());
  if (wdx < 32.f || wdx >= 10000 || isnan(wdx)) return -1;
  DWORD is_good = Readmemory<DWORD>(waddr + woffsets.back() + 0x18);
  // std::cout << is_good << std::endl;
  if (0 == is_good) return -1;
  return 0;
}
int MemoryManager::ReadMapId() {
  DWORD map_id = Readmemory<DWORD>(0x0095F050);
  return map_id;
}
