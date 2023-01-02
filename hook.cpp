#include "hook.h"
Hooker::Ptr Hooker::Create() { return std::make_shared<Hooker>(); }

bool Hooker::Init(const ULL& addr_ori, const ULL& addr_tgt) {
  addr_ori_ = addr_ori;
  addr_tgt_ = addr_tgt;

  inst_tgt_[0] = '\xe9';
  int offset = addr_tgt_ - (addr_ori_ + INST_LEN);

  memcpy(&inst_tgt_[1], &offset, INST_LEN - 1);  // byte copy
  DWORD attr = ModAttr(addr_ori_);
  memcpy(inst_ori_, reinterpret_cast<void*>(addr_ori_), INST_LEN);
  return ModAttr(addr_ori_, attr);
}

DWORD Hooker::ModAttr(const ULL& addr, DWORD attr) {
  DWORD old_attr;
  VirtualProtect(reinterpret_cast<void*>(addr), INST_LEN, attr, &old_attr);
  return old_attr;
}

bool Hooker::Modify() {
  DWORD attr = ModAttr(addr_ori_);
  memcpy(reinterpret_cast<void*>(addr_ori_), inst_tgt_, INST_LEN);
  ModAttr(addr_ori_, attr);
  return true;
}
bool Hooker::Restore() {
  DWORD attributes = ModAttr(addr_ori_);
  memcpy(reinterpret_cast<void*>(addr_ori_), inst_ori_, INST_LEN);
  ModAttr(addr_ori_, attributes);
  return true;
}