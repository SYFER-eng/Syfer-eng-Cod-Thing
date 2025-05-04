#include <iostream>
#include <string>
#include <memory>
#include <thread>
#include <chrono>
#include <filesystem>
#include <fstream>
#include "DLLInjector.h"
#include "Logger.h"

// Cross-platform compatibility
// First define the common color constants
#define FOREGROUND_BLUE 1
#define FOREGROUND_GREEN 2
#define FOREGROUND_RED 4
#define FOREGROUND_INTENSITY 8
#define FOREGROUND_CYAN (FOREGROUND_BLUE | FOREGROUND_GREEN)
#define FOREGROUND_YELLOW (FOREGROUND_RED | FOREGROUND_GREEN)

#ifdef _WIN32
    #include <Windows.h>
    #define WINDOWS_BUILD
    
    // Windows-specific console color functions
    void SetConsoleColor(WORD color) {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, color);
    }
    
    void ResetConsoleColor() {
        SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    }
    
    void SetConsoleTitle(const std::string& title) {
        SetConsoleTitleA(title.c_str());
    }
    
    std::string GetExecutablePath() {
        char buffer[MAX_PATH];
        GetModuleFileNameA(NULL, buffer, MAX_PATH);
        std::string::size_type pos = std::string(buffer).find_last_of("\\/");
        return std::string(buffer).substr(0, pos);
    }
    
    void Sleep(int milliseconds) {
        ::Sleep(milliseconds);
    }
#else
    // Non-Windows platform stubs
    typedef int WORD; // Use int instead of the enum for Linux
    
    void SetConsoleColor(WORD color) {
        // ANSI color codes for Linux/Unix terminals
        if (color == (FOREGROUND_RED | FOREGROUND_INTENSITY)) {
            std::cout << "\033[1;31m"; // Bright Red
        }
        else if (color == (FOREGROUND_GREEN | FOREGROUND_INTENSITY)) {
            std::cout << "\033[1;32m"; // Bright Green
        }
        else if (color == (FOREGROUND_BLUE | FOREGROUND_INTENSITY)) {
            std::cout << "\033[1;34m"; // Bright Blue
        }
        else if (color == (FOREGROUND_CYAN | FOREGROUND_INTENSITY)) {
            std::cout << "\033[1;36m"; // Bright Cyan
        }
        else if (color == (FOREGROUND_YELLOW | FOREGROUND_INTENSITY)) {
            std::cout << "\033[1;33m"; // Bright Yellow
        }
        else {
            std::cout << "\033[0m"; // Reset
        }
    }
    
    void ResetConsoleColor() {
        std::cout << "\033[0m"; // Reset color
    }
    
    void SetConsoleTitle(const std::string& title) {
        // Some terminals support this escape sequence
        std::cout << "\033]0;" << title << "\007";
    }
    
    std::string GetExecutablePath() {
        return std::filesystem::current_path().string();
    }
    
    void Sleep(int milliseconds) {
        std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    }
#endif

void PrintTitle(const std::string& title) {
    SetConsoleColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    std::cout << "\n" << title << std::endl;
    std::string underline(title.length(), '=');
    std::cout << underline << std::endl;
    ResetConsoleColor();
}

void PrintError(const std::string& error) {
    SetConsoleColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
    std::cerr << "[ERROR] " << error << std::endl;
    ResetConsoleColor();
}

void PrintSuccess(const std::string& message) {
    SetConsoleColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    std::cout << "[SUCCESS] " << message << std::endl;
    ResetConsoleColor();
}

void PrintInfo(const std::string& message) {
    SetConsoleColor(FOREGROUND_CYAN | FOREGROUND_INTENSITY);
    std::cout << "[INFO] " << message << std::endl;
    ResetConsoleColor();
}

void PrintStatus(const std::string& status) {
    SetConsoleColor(FOREGROUND_YELLOW | FOREGROUND_INTENSITY);
    std::cout << status << std::endl;
    ResetConsoleColor();
}

void PrintBanner() {
    SetConsoleColor(FOREGROUND_CYAN | FOREGROUND_INTENSITY);
    std::cout << R"(
.d8888. db    db d88888b d88888b d8888b.        d88888b d8b   db  d888b  
88'  YP `8b  d8' 88'     88'     88  `8D        88'     888o  88 88' Y8b 
`8bo.    `8bd8'  88ooo   88ooooo 88oobY'        88ooooo 88V8o 88 88      
  `Y8b.    88    88~~~   88~~~~~ 88`8b   C8888D 88~~~~~ 88 V8o88 88  ooo 
db   8D    88    88      88.     88 `88.        88.     88  V888 88. ~8~ 
`8888Y'    YP    YP      Y88888P 88   YD        Y88888P VP   V8P  Y888P  
)" << std::endl;
    
    SetConsoleColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    std::cout << "              DLL INJECTOR v1.0\n";
    std::cout << "           FOR CALL OF DUTY: MW\n";
    std::cout << "============================================\n";
    ResetConsoleColor();
}

// Show spinner animation
void ShowSpinner(const std::string& message, std::shared_ptr<Logger> logger, bool& running) {
    const char spinner[] = {'|', '/', '-', '\\'};
    int counter = 0;
    
    while (running) {
        std::cout << "\r" << message << " " << spinner[counter % 4] << std::flush;
        counter++;
        Sleep(100);
    }
    std::cout << "\r" << std::string(message.length() + 2, ' ') << "\r";
}

int main() {
    // Set console title
    SetConsoleTitle("Syfer Injector v1.0 for CoD");
    
    // Create logger
    auto logger = std::make_shared<Logger>("SyferInjector.log");
    
    // Print banner
    PrintBanner();
    logger->Log("Application started");
    
    // Check if platform is supported for actual DLL injection
    if (!DLLInjector::IsPlatformSupported()) {
        PrintInfo("Running in demonstration mode - actual DLL injection requires Windows");
        logger->LogWarning("Running in demonstration mode (not on Windows)");
    }
    
    // Create DLL injector
    DLLInjector injector(logger);
    
    // Path to the DLL - use current directory
    std::string exePath = GetExecutablePath();
    
    // Use appropriate path separator based on platform
    std::string separator = "/";
    #ifdef WINDOWS_BUILD
        separator = "\\";
    #endif
    
    std::string dllPath = exePath + separator + "Syfer-eng.dll";
    
    // Process name for CoD: MW III
    const std::wstring processName = L"cod.exe";
    
    PrintInfo("DLL Path: " + dllPath);
    PrintInfo("Target Process: cod.exe");
    
    // Check if DLL exists before proceeding
    if (!injector.CheckDLLExists(dllPath)) {
        PrintError("Could not find the DLL file: " + dllPath);
        PrintInfo("Please make sure the DLL is in the same directory as this program.");
        PrintStatus("Press any key to exit...");
        std::cin.get();
        return 1;
    }
    
    PrintTitle("PROCESS DETECTION");
    PrintStatus("Searching for game process...");
    
    // Start a thread to show a spinner while searching
    bool searchRunning = true;
    std::thread spinnerThread([&]() {
        ShowSpinner("Searching for cod.exe", logger, searchRunning);
    });
    
    // Get the process ID
    DWORD processId = injector.GetProcessIdByName(processName);
    searchRunning = false;
    spinnerThread.join();
    
    if (processId == 0) {
        PrintError("Game process not found!");
        PrintInfo("Make sure Call of Duty: Modern Warfare III is running.");
        PrintInfo("Try running this injector as Administrator if the game is running.");
        PrintStatus("Press any key to exit...");
        std::cin.get();
        return 1;
    }
    
    PrintSuccess("Game process found!");
    std::cout << "Process ID: " << processId << std::endl << std::endl;
    
    // Inject the DLL
    PrintTitle("DLL INJECTION");
    PrintStatus("Injecting DLL into the game...");
    
    // Start a thread to show a spinner while injecting
    bool injectionRunning = true;
    std::thread injectionSpinnerThread([&]() {
        ShowSpinner("Injecting DLL", logger, injectionRunning);
    });
    
    InjectionError result = injector.InjectDLL(processId, dllPath);
    injectionRunning = false;
    injectionSpinnerThread.join();
    
    if (result != InjectionError::SUCCESS) {
        PrintError("DLL injection failed: " + injector.GetErrorMessage(result));
        
        if (result != InjectionError::PLATFORM_NOT_SUPPORTED) {
            PrintError("System Error: " + injector.GetLastErrorAsString());
            PrintInfo("Try running this program as Administrator.");
        }
        
        PrintStatus("Press any key to exit...");
        std::cin.get();
        return 1;
    }
    
    PrintSuccess("DLL injected successfully!");
    PrintInfo("The mod should now be active in your game.");
    
    logger->Log("Application completed successfully");
    PrintStatus("\nPress any key to exit...");
    std::cin.get();
    
    return 0;
}
