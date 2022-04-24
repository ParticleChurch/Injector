#pragma once

#include "encryption.hpp"
#include "http.hpp"

#include <QtCore/QtCore>
#include <QtWidgets/QWidget>

#include <Windows.h>

#include <psapi.h>
#include <string>
#include <vector>

// for some reason, you can't pass a HANDLE through a QT signal
// so we have to wrap it like this...
struct Process {
    DWORD id;
    HANDLE handle;
};

class OpenCSGOWorker : public QThread
{
    Q_OBJECT;

    bool processLikelyCSGO(DWORD pid) {
        HANDLE proc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
        if (!proc) return false;

        char szExeName[MAX_PATH + 1];
        GetModuleFileNameEx(proc, 0, szExeName, MAX_PATH);

        std::string exeName(szExeName);
        std::string csgo = "csgo.exe";

        if (exeName.length() < csgo.length()) return false;
        if (exeName.substr(exeName.size() - csgo.size()) != csgo) return false;

        return true;
    }

    DWORD findCSGOPID() {
        DWORD PIDs[2048];
        DWORD bytesUsed;

        if (!EnumProcesses(PIDs, sizeof(PIDs), &bytesUsed))
        {
            return 0;
        }

        int processCount = bytesUsed / sizeof(DWORD);
        for (int i = 0; i < processCount; i++)
        {
            DWORD& pid = PIDs[i];
            if (pid && processLikelyCSGO(pid))
                return pid;
        }

        return 0;
    }

public:
    using QThread::QThread;

    void run() override
    {
        emit status("Finding Process ID...");
        DWORD pid = this->findCSGOPID();
        while (!pid) {
            Sleep(1000);
            pid = this->findCSGOPID();
        }

        emit status("Hooking CS:GO...");
        HANDLE proc = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
        while (!proc) {
            Sleep(1000);
            proc = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
        }

        emit status("Initializing CS:GO...");
        // TODO: wait for DLLs to be loaded

        emit complete({ pid, proc });
    }

signals:
    void status(std::string status);
    void complete(Process x);
};

class DLLDownloadWorker : public QThread
{
    Q_OBJECT;

public:
    using QThread::QThread;

    void run() override
    {
        emit status("Downloading DLL...");

        Sleep(2000);

        emit complete({});
    }

signals:
    void status(std::string status);
    void complete(std::vector<char> data);
};

class DLLInjectionWorker : public QThread
{
    Q_OBJECT;

public:
    using QThread::QThread;

    void run() override
    {
        emit status("Decrypting DLL...", "Injecting DLL...");

        Sleep(2000);

        emit complete(nullptr);
    }

signals:
    void status(std::string status1, std::string status2);
    void complete(void* remoteAddress);
};

class EntryPointExecutionWorker : public QThread
{
    Q_OBJECT;

public:
    using QThread::QThread;

    void run() override
    {
        emit status("Executing...");

        Sleep(1000);

        emit complete();
    }

signals:
    void status(std::string status);
    void complete();
};

struct TaskStatus {
    std::string description = "/";
    bool complete = false;
};

class InjectionWorker : public QThread {
    Q_OBJECT;
    std::vector<TaskStatus> statuses = { {}, {}, {}, {}, {} };

    OpenCSGOWorker* task0;
    DLLDownloadWorker* task1;
    DLLInjectionWorker* task2;
    EntryPointExecutionWorker* task3;

    bool done = false;

public:
    InjectionWorker(QObject* parent = nullptr) : QThread(parent) {

        this->task0 = new OpenCSGOWorker(parent);
        this->connect(this->task0, &OpenCSGOWorker::status, this, &InjectionWorker::task0_status);
        this->connect(this->task0, &OpenCSGOWorker::complete, this, &InjectionWorker::task0_complete);
        this->connect(this->task0, &OpenCSGOWorker::finished, this->task0, &QObject::deleteLater);

        this->task1 = new DLLDownloadWorker(parent);
        this->connect(this->task1, &DLLDownloadWorker::status, this, &InjectionWorker::task1_status);
        this->connect(this->task1, &DLLDownloadWorker::complete, this, &InjectionWorker::task1_complete);
        this->connect(this->task1, &DLLDownloadWorker::finished, this->task1, &QObject::deleteLater);
        
        this->task2 = new DLLInjectionWorker(parent);
        this->connect(this->task2, &DLLInjectionWorker::status, this, &InjectionWorker::task2_status);
        this->connect(this->task2, &DLLInjectionWorker::complete, this, &InjectionWorker::task2_complete);
        this->connect(this->task2, &DLLInjectionWorker::finished, this->task2, &QObject::deleteLater);
        
        this->task3 = new EntryPointExecutionWorker(parent);
        this->connect(this->task3, &EntryPointExecutionWorker::status, this, &InjectionWorker::task3_status);
        this->connect(this->task3, &EntryPointExecutionWorker::complete, this, &InjectionWorker::task3_complete);
        this->connect(this->task3, &EntryPointExecutionWorker::finished, this->task3, &QObject::deleteLater);
    }

    void run() override {
        this->task0->start();

        while (!this->done) {
            emit this->status(this->statuses);
            Sleep(1000 / 144);
        }
        emit this->status(this->statuses);
        Sleep(3000);

        emit complete();
    }

public slots:
    void task0_status(std::string status) { this->statuses[0].description = status; };
    void task0_complete(Process x) { this->statuses[0].complete = true; this->task1->start(); };

    void task1_status(std::string status) { this->statuses[1].description = status; };
    void task1_complete(std::vector<char> data) { this->statuses[1].complete = true; this->task2->start(); };

    void task2_status(std::string status1, std::string status2) { this->statuses[2].description = status1; this->statuses[3].description = status2; };
    void task2_complete(void* remoteAddress) { this->statuses[2].complete = true; this->statuses[3].complete = true; this->task3->start(); };

    void task3_status(std::string status) { this->statuses[4].description = status;  };
    void task3_complete() { this->statuses[4].complete = true; this->done = true; };

signals:
    void status(std::vector<TaskStatus> statuses);
    void complete();
};