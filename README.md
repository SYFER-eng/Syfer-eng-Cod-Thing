# Syfer DLL Injector for Call of Duty


![Syfer DLL Injector](https://raw.githubusercontent.com/SYFER-eng/Syfer-eng-Cod-Thing/refs/heads/main/Terminal.png)
```
[Cod Dll]()
```
## Features

- **Clean, Professional UI**: Console-based UI with color coding and clear status messages
- **Robust Error Handling**: Detailed error messages with Windows error code translation
- **Process Detection**: Automatically finds the game process
- **DLL Verification**: Checks that the DLL exists before attempting injection
- **Logging System**: Comprehensive logging for troubleshooting
- **Visual Feedback**: Loading spinners during detection and injection
- **Cross-Platform Compatible**: Core code works on Windows, macOS, and Linux (with Windows required for actual injection)

## Requirements

- Windows 10/11 (for actual DLL injection)
- Visual Studio 2019 or newer (for building from source)
- Administrative privileges (for process injection)

## Usage

1. Place your `Syfer-eng.dll` in the same directory as the injector
2. Launch Call of Duty
3. Run `SyferInjector.exe` as Administrator
4. The injector will automatically find the game and inject the DLL

## Building from Source

### Using Visual Studio (Windows)
1. Open `SyferInjector.sln` in Visual Studio
2. Select your build configuration (Debug/Release, x86/x64)-

### Using Command Line (All Platforms)
```bash
# Using the build script
chmod +x build.sh
./build.sh

# Or manually:
# Windows (MinGW)
g++ -std=c++17 main.cpp DLLInjector.cpp Logger.cpp -o SyferInjector.exe -lpsapi -luser32

# Linux
g++ -std=c++17 main.cpp DLLInjector.cpp Logger.cpp -o SyferInjector -pthread

# macOS
g++ -std=c++17 main.cpp DLLInjector.cpp Logger.cpp -o SyferInjector
```

## Technical Details

This DLL injector uses the Remote Thread Injection technique:

1. Opens the target process with `OpenProcess`
2. Allocates memory in the target process with `VirtualAllocEx`
3. Writes the DLL path to the allocated memory with `WriteProcessMemory`
4. Creates a remote thread with `CreateRemoteThread` that calls `LoadLibraryA`
5. Waits for the thread to complete and cleans up resources
