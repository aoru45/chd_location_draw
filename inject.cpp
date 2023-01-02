#include <cstdio>
#include <Windows.h>
#include <TlHelp32.h>

#define PROCESS_NAME "LaTaleClient.exe"
#define DLL_NAME "chdar.dll"

BOOL InjectDll(DWORD dwPid, CHAR szDllName[]);
DWORD GetPID(PCHAR pProName);
VOID ShowError(PCHAR msg);
BOOL EnbalePrivileges(HANDLE hProcess, char *pszPrivilegesName);

int main() {
  CHAR szDllPath[MAX_PATH] = {0};
  DWORD dwPID = 0;
  if (!EnbalePrivileges(GetCurrentProcess(), SE_DEBUG_NAME)) {
    printf("failed to open device");
  }

  dwPID = GetPID(PROCESS_NAME);
  if (dwPID == 0) {
    printf("failed to get pid");
    goto exit;
  }

  GetCurrentDirectory(MAX_PATH, szDllPath);
  strcat(szDllPath, "\\");
  strcat(szDllPath, DLL_NAME);

  if (InjectDll(dwPID, szDllPath)) {
    printf("Dll load success");
  }
exit:
  system("pause");

  return 0;
}
BOOL InjectDll(DWORD dwPid, CHAR szDllName[]) {
  BOOL bRet = TRUE;
  HANDLE hProcess = NULL, hRemoteThread = NULL;
  HMODULE hKernel32 = NULL;
  DWORD dwSize = 0;
  LPVOID pDllPathAddr = NULL;
  PVOID pLoadLibraryAddr = NULL;

  hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid);
  if (NULL == hProcess) {
    ShowError("OpenProcess");
    bRet = FALSE;
    goto exit;
  }

  dwSize = 1 + strlen(szDllName);
  pDllPathAddr =
      VirtualAllocEx(hProcess, NULL, dwSize, MEM_COMMIT, PAGE_READWRITE);
  if (!pDllPathAddr) {
    ShowError("VirtualAllocEx");
    bRet = FALSE;
    goto exit;
  }

  if (!WriteProcessMemory(hProcess, pDllPathAddr, szDllName, dwSize, NULL)) {
    ShowError("WriteProcessMemory");
    bRet = FALSE;
    goto exit;
  }

  hKernel32 = LoadLibrary("kernel32.dll");
  if (hKernel32 == NULL) {
    ShowError("LoadLibrary");
    bRet = FALSE;
    goto exit;
  }

  pLoadLibraryAddr = GetProcAddress(hKernel32, "LoadLibraryA");
  if (pLoadLibraryAddr == NULL) {
    ShowError("GetProcAddress ");
    bRet = FALSE;
    goto exit;
  }

  hRemoteThread = CreateRemoteThread(hProcess, NULL, 0,
                                     (LPTHREAD_START_ROUTINE)pLoadLibraryAddr,
                                     pDllPathAddr, 0, NULL);
  if (hRemoteThread == NULL) {
    ShowError("CreateRemoteThread");
    bRet = FALSE;
    goto exit;
  }

exit:
  if (hKernel32) FreeLibrary(hKernel32);
  if (hProcess) CloseHandle(hProcess);
  if (hRemoteThread) CloseHandle(hRemoteThread);

  return bRet;
}

DWORD GetPID(PCHAR pProName) {
  PROCESSENTRY32 pe32 = {0};
  HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  BOOL bRet = FALSE;
  DWORD dwPID = 0;

  if (hSnap == INVALID_HANDLE_VALUE) {
    printf("CreateToolhelp32Snapshot process %d\n", GetLastError());
    goto exit;
  }

  pe32.dwSize = sizeof(pe32);
  bRet = Process32First(hSnap, &pe32);
  while (bRet) {
    if (lstrcmp(pe32.szExeFile, pProName) == 0) {
      dwPID = pe32.th32ProcessID;
      break;
    }
    bRet = Process32Next(hSnap, &pe32);
  }

  CloseHandle(hSnap);
exit:
  return dwPID;
}

VOID ShowError(PCHAR msg) { printf("%s Error %d\n", msg, GetLastError()); }

BOOL EnbalePrivileges(HANDLE hProcess, char *pszPrivilegesName) {
  HANDLE hToken = NULL;
  LUID luidValue = {0};
  TOKEN_PRIVILEGES tokenPrivileges = {0};
  BOOL bRet = FALSE;
  DWORD dwRet = 0;

  if (!OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES, &hToken)) {
    ShowError("OpenProcessToken");
    goto exit;
  }

  if (!LookupPrivilegeValue(NULL, pszPrivilegesName, &luidValue)) {
    ShowError("LookupPrivilegeValue");
    goto exit;
  }

  tokenPrivileges.PrivilegeCount = 1;
  tokenPrivileges.Privileges[0].Luid = luidValue;
  tokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
  if (!AdjustTokenPrivileges(hToken, FALSE, &tokenPrivileges, 0, NULL, NULL)) {
    ShowError("AdjustTokenPrivileges");
    goto exit;
  } else {
    dwRet = ::GetLastError();
    if (ERROR_SUCCESS == dwRet) {
      bRet = TRUE;
      goto exit;
    } else if (ERROR_NOT_ALL_ASSIGNED == dwRet) {
      ShowError("ERROR_NOT_ALL_ASSIGNED");
      goto exit;
    }
  }
exit:
  return bRet;
}