#include "DLLInjector.h"
#include <iostream>
#include <sstream>
#include <filesystem>
#include <thread>
#include <chrono>

DLLInjector::DLLInjector(std::shared_ptr<Logger> logger) : m_logger(logger) {
    m_logger->Log("DLLInjector initialized");
    
    if (!IsPlatformSupported()) {
        m_logger->LogWarning("Current platform is not Windows. Limited functionality available.");
    }
}

DLLInjector::~DLLInjector() {
    m_logger->Log("DLLInjector destroyed");
}

bool DLLInjector::IsPlatformSupported() {
    #ifdef WINDOWS_BUILD
        return true;
    #else
        return false;
    #endif
}

DWORD DLLInjector::GetProcessIdByName(const std::wstring& processName) {
    m_logger->Log("Searching for process: " + std::string(processName.begin(), processName.end()));
    
    #ifdef WINDOWS_BUILD
        DWORD processId = 0;
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

        if (snapshot == INVALID_HANDLE_VALUE) {
            m_logger->LogError("Failed to create process snapshot: " + GetLastErrorAsString());
            return 0;
        }

        PROCESSENTRY32W processEntry;
        processEntry.dwSize = sizeof(PROCESSENTRY32W);

        if (Process32FirstW(snapshot, &processEntry)) {
            do {
                if (_wcsicmp(processEntry.szExeFile, processName.c_str()) == 0) {
                    processId = processEntry.th32ProcessID;
                    std::ostringstream log;
                    log << "Process found with ID: " << processId;
                    m_logger->Log(log.str());
                    break;
                }
            } while (Process32NextW(snapshot, &processEntry));
        }
        else {
            m_logger->LogError("Failed to get first process: " + GetLastErrorAsString());
        }

        CloseHandle(snapshot);
        
        if (processId == 0) {
            m_logger->LogError("Process not found: " + std::string(processName.begin(), processName.end()));
        }
        
        return processId;
    #else
        m_logger->LogWarning("GetProcessIdByName is only supported on Windows. Using demo mode.");
        // For demonstration, return a fake process ID
        return 12345;
    #endif
}

std::vector<DWORD> DLLInjector::GetAllProcessIdsByName(const std::wstring& processName) {
    std::vector<DWORD> processIds;
    
    #ifdef WINDOWS_BUILD
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

        if (snapshot == INVALID_HANDLE_VALUE) {
            m_logger->LogError("Failed to create process snapshot: " + GetLastErrorAsString());
            return processIds;
        }

        PROCESSENTRY32W processEntry;
        processEntry.dwSize = sizeof(PROCESSENTRY32W);

        if (Process32FirstW(snapshot, &processEntry)) {
            do {
                if (_wcsicmp(processEntry.szExeFile, processName.c_str()) == 0) {
                    processIds.push_back(processEntry.th32ProcessID);
                    std::ostringstream log;
                    log << "Found process instance with ID: " << processEntry.th32ProcessID;
                    m_logger->Log(log.str());
                }
            } while (Process32NextW(snapshot, &processEntry));
        }

        CloseHandle(snapshot);
    #else
        m_logger->LogWarning("GetAllProcessIdsByName is only supported on Windows. Using demo mode.");
        // For demonstration, return fake process IDs
        processIds.push_back(12345);
        processIds.push_back(12346);
    #endif
    
    return processIds;
}

bool DLLInjector::CheckDLLExists(const std::string& dllPath) {
    bool exists = std::filesystem::exists(dllPath);
    if (exists) {
        m_logger->Log("DLL file found: " + dllPath);
    }
    else {
        m_logger->LogError("DLL file not found: " + dllPath);
    }
    return exists;
}

InjectionError DLLInjector::InjectDLL(DWORD processId, const std::string& dllPath) {
    // Check if platform is supported
    if (!IsPlatformSupported()) {
        m_logger->LogError("DLL injection is only supported on Windows");
        return InjectionError::PLATFORM_NOT_SUPPORTED;
    }
    
    // Check if DLL file exists
    if (!CheckDLLExists(dllPath)) {
        return InjectionError::DLL_NOT_FOUND;
    }

    m_logger->Log("Attempting to inject DLL: " + dllPath + " into process ID: " + std::to_string(processId));

    #ifdef WINDOWS_BUILD
        // Open the process
        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
        if (hProcess == NULL) {
            m_logger->LogError("Failed to open process: " + GetLastErrorAsString());
            return InjectionError::PROCESS_OPEN_FAILED;
        }

        // Allocate memory in the process for the DLL path
        LPVOID dllPathAddr = VirtualAllocEx(hProcess, NULL, dllPath.length() + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (dllPathAddr == NULL) {
            m_logger->LogError("Failed to allocate memory in process: " + GetLastErrorAsString());
            CloseHandle(hProcess);
            return InjectionError::MEMORY_ALLOCATION_FAILED;
        }

        // Write the DLL path to the allocated memory
        if (!WriteProcessMemory(hProcess, dllPathAddr, dllPath.c_str(), dllPath.length() + 1, NULL)) {
            m_logger->LogError("Failed to write to process memory: " + GetLastErrorAsString());
            VirtualFreeEx(hProcess, dllPathAddr, 0, MEM_RELEASE);
            CloseHandle(hProcess);
            return InjectionError::MEMORY_WRITE_FAILED;
        }

        // Get the address of LoadLibraryA
        HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
        if (hKernel32 == NULL) {
            m_logger->LogError("Failed to get kernel32.dll handle: " + GetLastErrorAsString());
            VirtualFreeEx(hProcess, dllPathAddr, 0, MEM_RELEASE);
            CloseHandle(hProcess);
            return InjectionError::KERNEL32_HANDLE_FAILED;
        }

        LPVOID loadLibraryAddr = (LPVOID)GetProcAddress(hKernel32, "LoadLibraryA");
        if (loadLibraryAddr == NULL) {
            m_logger->LogError("Failed to get LoadLibraryA address: " + GetLastErrorAsString());
            VirtualFreeEx(hProcess, dllPathAddr, 0, MEM_RELEASE);
            CloseHandle(hProcess);
            return InjectionError::LOADLIBRARY_ADDR_FAILED;
        }

        // Create a remote thread that calls LoadLibraryA with the DLL path as argument
        HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)loadLibraryAddr, dllPathAddr, 0, NULL);
        if (hThread == NULL) {
            m_logger->LogError("Failed to create remote thread: " + GetLastErrorAsString());
            VirtualFreeEx(hProcess, dllPathAddr, 0, MEM_RELEASE);
            CloseHandle(hProcess);
            return InjectionError::REMOTE_THREAD_FAILED;
        }

        // Wait for the thread to finish
        m_logger->Log("Waiting for injection thread to complete...");
        WaitForSingleObject(hThread, INFINITE);
        
        // Get thread exit code to verify success
        DWORD exitCode = 0;
        GetExitCodeThread(hThread, &exitCode);
        
        if (exitCode == 0) {
            m_logger->LogError("DLL injection may have failed (thread exit code is 0)");
        } else {
            m_logger->Log("DLL injection thread completed successfully");
        }

        // Clean up
        CloseHandle(hThread);
        VirtualFreeEx(hProcess, dllPathAddr, 0, MEM_RELEASE);
        CloseHandle(hProcess);

        return InjectionError::SUCCESS;
    #else
        // This code is never executed on Windows platforms
        m_logger->Log("Simulating DLL injection in demo mode...");
        // Sleep for a short time to simulate the injection process
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        m_logger->Log("Simulation complete");
        return InjectionError::SUCCESS;
    #endif
}

std::string DLLInjector::GetErrorMessage(InjectionError error) {
    switch (error) {
        case InjectionError::SUCCESS:
            return "Success";
        case InjectionError::PROCESS_NOT_FOUND:
            return "Process not found";
        case InjectionError::PROCESS_OPEN_FAILED:
            return "Failed to open process";
        case InjectionError::MEMORY_ALLOCATION_FAILED:
            return "Failed to allocate memory in the process";
        case InjectionError::MEMORY_WRITE_FAILED:
            return "Failed to write to process memory";
        case InjectionError::KERNEL32_HANDLE_FAILED:
            return "Failed to get kernel32.dll handle";
        case InjectionError::LOADLIBRARY_ADDR_FAILED:
            return "Failed to get LoadLibraryA address";
        case InjectionError::REMOTE_THREAD_FAILED:
            return "Failed to create remote thread";
        case InjectionError::DLL_NOT_FOUND:
            return "DLL file not found";
        case InjectionError::PLATFORM_NOT_SUPPORTED:
            return "Platform not supported (Windows required)";
        case InjectionError::UNKNOWN_ERROR:
        default:
            return "Unknown error";
    }
}

std::string DLLInjector::GetLastErrorAsString() {
    #ifdef WINDOWS_BUILD
        DWORD errorCode = GetLastError();
        if (errorCode == 0) {
            return "No error";
        }

        LPSTR messageBuffer = nullptr;
        size_t size = FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

        std::string message(messageBuffer, size);
        LocalFree(messageBuffer);

        return "[Error " + std::to_string(errorCode) + "] " + message;
    #else
        return "Error information not available on this platform";
    #endif
}
