#include "windows.h"
#include "tlhelp32.h"
#include "tchar.h"

#define DEF_PROC_NAME (L"notepad.exe")
#define DEF_DLL_NAME (L"myhack.dll")

DWORD FindProcessID(LPCTSTR szProcessName)
{
    DWORD dwPID = 0xFFFFFFFF;
    HANDLE hSnapShot = INVALID_HANDLE_VALUE;
    PROCESSENTRY32 pe;

    //시스템의 스냅샷 가져오기
    pe.dwSize = sizeof(PROCESSENTRY32);
    hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);

    //프로세스 찾기
    Process32First(hSanpShot, &pe);
    do
    {
        if(!_tcsicmp(szProcessName, (LPCTSTR)pe.szExeFile))
        {
            dwPID = pe.th32ProcessID;
            break;
        }
    }
    while(Process32Next(hSanpShot, &pe));

    CloseHandle(hSanpShot);

    return dwPID:
}

BOOL SetPrivilege(LPCTSTR lpszPrivilege, BOOL bEnablePrivilege)
{
    TOKEN_PRIVILEGES tp;
    HANDLE hToke;
    LUID luid;

    if(!OpenProcessToken(GetCUrentProcess(),
        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
    {
        _tprintf(L"OpenProcessToken error: %u\n", GetLastError());
        return FALSE;
    }
    if(!LookupPrivilegeValue(NULL, //lookup privilege on local system
        lpszPrivilege, //privilege to lookup 
        &luid)) //receives LUID of privilege
    {
        _tprintf(L"LookupPrivilegeValue error: %u\n", GetLastError());
        return FALSE;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    if(bEnablePrivilege)
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    else
        tp.Privileges[0].Attributes = 0;

    //Enable the privilege or disable all privileges.k
    if(!AdjustTokenPrivileges(hToken, FALSE,&tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES) NULL, (POWORD)NULL))
    {
        _tprintf(L"AdjustTokenPrivileges error: %u\n", GetLastError());
        return FALSE;
    }

    if(GetLastError() == ERROR_NOT_ALL_ASSIGNED)
    {
        _tprintf(L"The token does not have the specified privilege. \n");
        return FALSE;
    }

    return TRUE;

}

BOOL EjectDll(DWORD dwPID, LPCTSTR szDllName)
{
    BOOL bMore = FALSE, bFound = FALSE;
    HANDLE hSnapshot, hProcess, hThread;
    HMODULE hModule = NULL;
    MODULEENTRY32 me = {sizeof(me)};
    LPTHREAD_START_ROUTINE pThreadProc;

    //dwPID = notepad 프로세스 ID
    //TH32CS_SNAPMODULE 파라미터를 이요애서 notepad 프로세스에 로딩된 DLL 이름을 얻음
    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);

    bMore = Module32First(hSnapshot, &me);
    for(;bMore;bMore = Module32NExt(hSnapshot, &me))
    {
        if(!_tcsicmp((LPCTSTR)me.szModule, szDllNmae) ||
        !_tcsicmp((LPCTSTR)me.szExePath, szDllNae))
        {
            bFound = TRUE;
            break;
        }
    }

    if(!bFound)
    {
        CloseHandle(hSnapshot);
        return FALSE;
    }

    if(!(hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE< dwPID)))
    {
        _tprintf(L"OpenProcess(%d) failed!!! [%d]\n", dwPID, GetLastError());
        return FALSE;
    }

    hModule = GetModuleHandle(L"kernel32.dll");
    pThreadProc = (LPTHREAD_START_ROUTINE)GetProcAddress(hModule, "FreeLibrary");
    hThread = CreateRemoteThread(hProcess, NULL, 0, pThreadProc, me.modBaseAddr, 0, NULL);
    WaitForSingleObject(hThread, INFINITE);

    CloseHandle(hThread);
    CloseHandle(hProcess);
    CloseHandle(hSnapshot);

    return TRUE;
}
