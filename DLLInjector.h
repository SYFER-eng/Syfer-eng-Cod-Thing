#pragma once

// Cross-platform compatibility
#ifdef _WIN32
    #include <Windows.h>
    #include <TlHelp32.h>
    #define WINDOWS_BUILD
#else
    // Linux/Unix typedefs for compatibility
    typedef unsigned long DWORD;
    typedef void* HANDLE;
    typedef void* LPVOID;
    typedef const char* LPCSTR;
    #define MAX_PATH 260
#endif

#include <string>
#include <vector>
#include <memory>
#include "Logger.h"

enum class InjectionError {
    SUCCESS = 0,
    PROCESS_NOT_FOUND = 1,
    PROCESS_OPEN_FAILED = 2,
    MEMORY_ALLOCATION_FAILED = 3,
    MEMORY_WRITE_FAILED = 4,
    KERNEL32_HANDLE_FAILED = 5,
    LOADLIBRARY_ADDR_FAILED = 6,
    REMOTE_THREAD_FAILED = 7,
    DLL_NOT_FOUND = 8,
    PLATFORM_NOT_SUPPORTED = 9,
    UNKNOWN_ERROR = 10
};

class DLLInjector {
public:
    DLLInjector(std::shared_ptr<Logger> logger);
    ~DLLInjector();

    // Find a process by name and return its ID
    DWORD GetProcessIdByName(const std::wstring& processName);
    
    // Get all processes with matching name
    std::vector<DWORD> GetAllProcessIdsByName(const std::wstring& processName);
    
    // Check if DLL file exists
    bool CheckDLLExists(const std::string& dllPath);
    
    // Inject DLL into a process
    InjectionError InjectDLL(DWORD processId, const std::string& dllPath);
    
    // Get error message for injection error
    static std::string GetErrorMessage(InjectionError error);
    
    // Get error details (Windows-specific on Windows, generic on other platforms)
    static std::string GetLastErrorAsString();
    
    // Check if current platform is supported for DLL injection
    static bool IsPlatformSupported();

private:
    std::shared_ptr<Logger> m_logger;
};
