#undef UNICODE
#include <string>
#include <vector>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
using std::string;
using std::vector;
#include <Tlhelp32.h>

int main(void)
{
    vector<string> processNames;
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    HANDLE hTool32 = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    BOOL bProcess = Process32First(hTool32, &pe32);
    if (bProcess == TRUE) {
        while ((Process32Next(hTool32, &pe32)) == TRUE) {
            processNames.push_back(pe32.szExeFile);
            if (strcmp(pe32.szExeFile, "Among Us.exe") == 0) {
                char* DirPath = new char[MAX_PATH];
                char* FullPath = new char[MAX_PATH];
                GetCurrentDirectory(MAX_PATH, DirPath);
                sprintf_s(FullPath, MAX_PATH, "%s\\spacemafia.dll", DirPath);
                HANDLE hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE, FALSE, pe32.th32ProcessID);
                LPVOID LoadLibraryAddr = (LPVOID)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");
                LPVOID LLParam = (LPVOID)VirtualAllocEx(hProcess, NULL, strlen(FullPath), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
                WriteProcessMemory(hProcess, LLParam, FullPath, strlen(FullPath), NULL);
                CreateRemoteThread(hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibraryAddr, LLParam, NULL, NULL);
                CloseHandle(hProcess);
                delete[] DirPath;
                delete[] FullPath;
            }
        }
    }
    CloseHandle(hTool32);
    return 0;
}
