#include "consts.hpp"
#include <ctime>
#include <windows.h>
#include <string>
#include <iostream>

// Function to write log messages
void LogWrite(const std::string& message) {
    WriteFile(
            GetStdHandle(STD_OUTPUT_HANDLE),
            message.c_str(),
            message.length(),
            nullptr,
            nullptr);
}

// Function to open a semaphore with error handling
HANDLE OpenSemaphoreWithErrorCheck(DWORD accessMode, BOOL inheritHandle, const std::string& semaphoreName) {
    HANDLE semaphore = OpenSemaphore(accessMode, inheritHandle, semaphoreName.c_str());
    if (semaphore == nullptr) {
        std::cerr << "Error opening semaphore: " << semaphoreName << std::endl;
        ExitProcess(1);
    }
    return semaphore;
}

int main() {
    srand(time(nullptr));

    // Open handles to semaphores and mutex
    HANDLE writeSemaphores[pageCount], readSemaphores[pageCount];
    HANDLE ioMutex = OpenMutex(
            MUTEX_MODIFY_STATE | SYNCHRONIZE,
            false,
            mutexName.c_str());
    HANDLE mappedFile = OpenFileMapping(
            GENERIC_READ,
            false,
            mapName.c_str());

    // Open handles to standard output and semaphores
    HANDLE stdOutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    for (int i = 0; i < pageCount; i++) {
        std::string writeSemaphoreName = "writeSemaphore #" + std::to_string(i);
        writeSemaphores[i] = OpenSemaphoreWithErrorCheck(SEMAPHORE_MODIFY_STATE | SYNCHRONIZE,
                                                         FALSE,
                                                         writeSemaphoreName);

        std::string readSemaphoreName = "readSemaphore #" + std::to_string(i);
        readSemaphores[i] = OpenSemaphoreWithErrorCheck(SEMAPHORE_MODIFY_STATE | SYNCHRONIZE,
                                                        FALSE,
                                                        readSemaphoreName);
    }

    // Perform write operations
    for (int i = 0; i < 3; i++) {
        LogWrite("Wait | Semaphore | " + std::to_string(GetTickCount()) + "\n");

        // Wait for any available page
        DWORD page = WaitForMultipleObjects(
                pageCount,
                writeSemaphores,
                FALSE,
                INFINITE);
        LogWrite("Take | Semaphore | " + std::to_string(GetTickCount()) + "\n");

        // Wait for the I/O mutex
        WaitForSingleObject(
                ioMutex,
                INFINITE);
        LogWrite("Take | Mutex | " + std::to_string(GetTickCount()) + "\n");

        Sleep(500 + rand() % 1000);
        LogWrite("Write | Page: " + std::to_string(page) + " | " + std::to_string(GetTickCount()) + "\n");

        // Release the I/O mutex
        ReleaseMutex(ioMutex);
        LogWrite("Free | Mutex | " + std::to_string(GetTickCount()) + "\n");

        // Release the read semaphore for the written page
        ReleaseSemaphore(readSemaphores[page], 1, nullptr);
        LogWrite("Free | Semaphore | " + std::to_string(GetTickCount()) + "\n\n");
    }

    // Close handles
    for (int i = 0; i < pageCount; i++) {
        CloseHandle(writeSemaphores[i]);
        CloseHandle(readSemaphores[i]);
    }
    CloseHandle(ioMutex);
    CloseHandle(mappedFile);

    return 0;
}