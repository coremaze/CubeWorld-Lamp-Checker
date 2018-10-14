#include <iostream>
#include <windows.h>

using namespace std;

int main()
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    //si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    if (!CreateProcess(NULL,
                  "Server.exe",
                  NULL,
                  NULL,
                  FALSE,
                  CREATE_SUSPENDED,
                  NULL,
                  NULL,
                  &si,
                  &pi))
    {
        cout << "Failed to create process.";
        return 1;
    }

    char dllName[] = "CWSeeds.dll";

    LPVOID load_library = (LPVOID) GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "LoadLibraryA");
	LPVOID remote_string = (LPVOID) VirtualAllocEx(pi.hProcess, NULL, strlen(dllName) + 1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    WriteProcessMemory(pi.hProcess, remote_string, dllName, strlen(dllName) + 1, NULL);

    HANDLE thread = CreateRemoteThread(pi.hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE) load_library, remote_string, CREATE_SUSPENDED, NULL);

    ResumeThread(pi.hThread);
    ResumeThread(thread);

    CloseHandle(pi.hProcess);

    CloseHandle(thread);

    WaitForSingleObject(pi.hThread, INFINITE);

    return 0;
}
