//
// Created by stukenvitalii on 20.05.2024.
//

#include "consts.hpp"
HANDLE CreateNewProcess(const string&, const string&);

int main(){
    HANDLE writeSemaphores[pageCount],
            readSemaphores[pageCount],
            IOMutex = CreateMutex(
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

    LPVOID fileView = MapViewOfFile(
            mapFile,
            FILE_MAP_ALL_ACCESS,
            0,
            0,
            pageSize * pageCount);

    cout << "---Begin---\n";
    for (int i = 0; i < pageCount; i++){
        string semaphoreName = "writeSemaphore #" + to_string(i);
        writeSemaphores[i] = CreateSemaphore(
                nullptr,
                1,
                1,
                semaphoreName.c_str());
        semaphoreName = "readSemaphore #" + to_string(i);
        readSemaphores[i] = CreateSemaphore(
                nullptr,
                0,
                1,
                semaphoreName.c_str());
    }

    VirtualLock(fileView, pageSize * pageCount);

    for (int i = 0; i < hProcessCount; i++) {
        string logName = R"(D:\tanki\University\OS\lab4\cmake-build-debug\logs\writelogs\writeLog #)" + to_string(i) + ".txt";
        processHandles[i] = CreateNewProcess("writer.exe", logName);
    }

    for (int i = 0; i < hProcessCount; i++) {
        string logName = R"(D:\tanki\University\OS\lab4\cmake-build-debug\logs\readlogs\readLog #)" + to_string(i) + ".txt";
        processHandles[i + hProcessCount] = CreateNewProcess("reader.exe", logName);
    }

    cout << "Now waiting...\n";
    WaitForMultipleObjects(
            hProcessCount * 2,
            processHandles,
            true,
            INFINITE);
    cout << "---Finish---\n";

    VirtualUnlock(fileView, pageSize * pageCount);
    UnmapViewOfFile(fileView);
    CloseHandle(IOMutex);
    CloseHandle(mapFile);
    CloseHandle(fileHandle);
    for (int i = 0; i < pageCount; i++){
        CloseHandle(writeSemaphores[i]);
        CloseHandle(readSemaphores[i]);
    }
    cout << "\nDone. Press ENTER to close the program.\n";
    system("pause");
}

HANDLE CreateNewProcess(const string& exePath, const string& logName){
    SECURITY_ATTRIBUTES securityAttributes = {
            sizeof(securityAttributes),
            nullptr,
            true};

    HANDLE logFileHandle = CreateFile(
            logName.c_str(),
            GENERIC_WRITE | GENERIC_READ,
            FILE_SHARE_WRITE,
            &securityAttributes,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            nullptr);

    STARTUPINFO startupInfo;
    PROCESS_INFORMATION processInformation;

    ZeroMemory(&startupInfo, sizeof(startupInfo));
    ZeroMemory(&processInformation, sizeof(processInformation));

    startupInfo.cb = sizeof(startupInfo);
    startupInfo.dwFlags |= STARTF_USESTDHANDLES;
    startupInfo.hStdInput = nullptr;
    startupInfo.hStdError = nullptr;
    startupInfo.hStdOutput = logFileHandle;

    if (CreateProcess(exePath.c_str(),
                      nullptr,
                      nullptr,
                      nullptr,
                      true,
                      0,
                      nullptr,
                      nullptr,
                      &startupInfo,
                      &processInformation))
        return processInformation.hProcess;
    return nullptr;
}