#include "consts.hpp"
#include <ctime>
#include <windows.h>
#include <string>
#include <iostream>

HANDLE CreateNewProcess(const std::string&, const std::string&);

int main() {
    HANDLE writeSemaphores[pageCount],
            readSemaphores[pageCount],
            ioMutex = CreateMutex(
            nullptr,
            false,
            mutexName.c_str()),
            fileHandle = CreateFile(
            fileName.c_str(),
            GENERIC_WRITE | GENERIC_READ,
            0, nullptr,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            nullptr),

            mapFile = CreateFileMapping(
            fileHandle,
            nullptr,
            PAGE_READWRITE,
            0,
            pageSize * pageCount,
            mapName.c_str()),

            processHandles[hProcessCount * 2];

    // Map the file to the memory
    LPVOID fileView = MapViewOfFile(
            mapFile,
            FILE_MAP_ALL_ACCESS,
            0,
            0,
            pageSize * pageCount);

    // Print start message
    std::cout << "---Begin---\n";
    for (int i = 0; i < pageCount; i++) {
        // Create write and read semaphores for each page
        std::string writeSemaphoreName = "writeSemaphore #" + std::to_string(i);
        writeSemaphores[i] = CreateSemaphore(
                nullptr,
                1,
                1,
                writeSemaphoreName.c_str());
        if (writeSemaphores[i] == NULL) {
            std::cerr << "Error creating semaphore: " << writeSemaphoreName << std::endl;
            ExitProcess(1);
        }

        std::string readSemaphoreName = "readSemaphore #" + std::to_string(i);
        readSemaphores[i] = CreateSemaphore(
                nullptr,
                0,
                1,
                readSemaphoreName.c_str());
        if (readSemaphores[i] == NULL) {
            std::cerr << "Error creating semaphore: " << readSemaphoreName << std::endl;
            ExitProcess(1);
        }
    }

    // Lock the file view in memory
    VirtualLock(fileView, pageSize * pageCount);

    // Create writer and reader processes
    for (int i = 0; i < hProcessCount; i++) {
        std::string logName = R"(D:\tanki\University\OS\lab4\cmake-build-debug\logs\writelogs\writeLog #)" + std::to_string(i) + ".txt";
        processHandles[i] = CreateNewProcess("writer.exe", logName);
    }

    for (int i = 0; i < hProcessCount; i++) {
        std::string logName = R"(D:\tanki\University\OS\lab4\cmake-build-debug\logs\readlogs\readLog #)" + std::to_string(i) + ".txt";
        processHandles[i + hProcessCount] = CreateNewProcess("reader.exe", logName);
    }

    // Wait for all processes to finish
    std::cout << "Now waiting...\n";
    DWORD result = WaitForMultipleObjects(
            hProcessCount * 2,
            processHandles,
            true,
            INFINITE);
    if (result == WAIT_FAILED) {
        std::cerr << "Error waiting for processes: " << GetLastError() << std::endl;
        ExitProcess(1);
    }
    std::cout << "---End---\n";

    // Unlock the file view and close handles
    VirtualUnlock(fileView, pageSize * pageCount);
    UnmapViewOfFile(fileView);
    CloseHandle(mapFile);
    CloseHandle(fileHandle);
    for (int i = 0; i < pageCount; i++) {
        CloseHandle(writeSemaphores[i]);
        CloseHandle(readSemaphores[i]);
    }
    CloseHandle(ioMutex);
    return 0;
}

HANDLE CreateNewProcess(const std::string& exePath, const std::string& logName) {
    // Create the process with specified path to the executable file and arguments
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    char buffer[512];
    strcpy_s(buffer, exePath.c_str());
    std::string commandLine = exePath + " " + logName;
    if (!CreateProcessA(
            exePath.c_str(),
            const_cast<char*>(commandLine.c_str()),
            nullptr,
            nullptr,
            false,
            0,
            nullptr,
            nullptr,
            &si,
            &pi)) {
        std::cerr << "Error creating process: " << GetLastError() << std::endl;
        ExitProcess(1);
    }
    CloseHandle(pi.hThread);
    return pi.hProcess;
}