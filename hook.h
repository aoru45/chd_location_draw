#pragma once
#include "Windows.h"
#include <iostream>
#include <memory>
using ULL = unsigned long long;

class Hooker {
  using Ptr = std::shared_ptr<Hooker>;
  static const int INST_LEN = 5;

 private:
  u_char inst_ori_[INST_LEN];
  u_char inst_tgt_[INST_LEN];
  ULL addr_ori_ = 0;
  ULL addr_tgt_ = 0;

  DWORD ModAttr(const ULL& addr, DWORD attr = PAGE_EXECUTE_READWRITE);

 public:
  Hooker() = default;
  ~Hooker() = default;
  static Ptr Create();
  virtual bool Init(const ULL& addr_ori, const ULL& addr_tgt);
  virtual bool Modify();
  virtual bool Restore();
};