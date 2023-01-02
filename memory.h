#pragma once
#include <string>
#include <Windows.h>
#include <iostream>
#include <chrono>
#include <thread>
using namespace std;

class MemoryManager {
  using Ptr = std::shared_ptr<MemoryManager>;
  using ULL = unsigned long long;

 private:
  std::string process_name_;
  std::string module_name_;
  ULL process_id_;
  ULL process_module_base_;
  HANDLE process_handle_;

 public:
  MemoryManager() = default;
  virtual ~MemoryManager();
  static Ptr Create();
  virtual bool Init(const std::string& process_name,
                    const std::string& module_name);
  virtual int ReadCoordWor(std::array<float, 2>& c_wor, int idx);
  virtual int ReadCoordScr(std::array<float, 2>& c_scr, int idx, int width,
                           int height);
  virtual int ReadEnermy();
  virtual int ReadMapId();
  virtual int ReadSelf(std::array<float, 2>& c_wor,
                       std::array<float, 2>& c_scr);

 private:
  template <typename ReadType>
  inline ReadType Readmemory(ULL addr) {
    ReadType buff;
    SIZE_T readSz;
    ReadProcessMemory(process_handle_, (LPVOID)addr, &buff, sizeof(ReadType),
                      &readSz);
    return buff;
  };
};