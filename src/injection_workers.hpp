#pragma once

#include "encryption.hpp"
#include "http.hpp"

#include <QtCore/QtCore>
#include <QtWidgets/QWidget>

#include <Windows.h>

#include "process_finder.hpp"
#include <string>
#include <vector>

class OpenCSGOWorker : public QThread
{
    Q_OBJECT;

public:
    using QThread::QThread;

    void run() override
    {
        emit status("Finding Process ID...");

        DWORD pid = ProcessFinder::findCSGOPID();
        while (!pid) {
            Sleep(1000);
            pid = ProcessFinder::findCSGOPID();
        }

        emit status("Hooking CS:GO...");
        HANDLE proc = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
        while (!proc) {
            Sleep(1000);
            proc = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
        }

        emit status("Initializing CS:GO...");
        // TODO: wait for DLLs to be loaded

        emit ready(proc);
    }

signals:
    void ready(HANDLE handle);
    void status(std::string status);
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

        emit ready({});
    }

signals:
    void ready(std::vector<char> data);
    void status(std::string status);
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

        emit ready(nullptr);
    }

signals:
    void ready(void* remoteAddress);
    void status(std::string status1, std::string status2);
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

        emit ready();
    }

signals:
    void ready();
    void status(std::string status);
};

class InjectionWorker : public QThread {
    Q_OBJECT;
    std::vector<std::string> statuses = { "/", "/", "/", "/", "/" };

    OpenCSGOWorker* task0;
    DLLDownloadWorker* task1;
    DLLInjectionWorker* task2;
    EntryPointExecutionWorker* task3;

public:
    InjectionWorker(QObject* parent = nullptr) : QThread(parent) {

        this->task0 = new OpenCSGOWorker(parent);
        this->connect(this->task0, &OpenCSGOWorker::status, this, &InjectionWorker::task0_status);
        this->connect(this->task0, &OpenCSGOWorker::ready, this, &InjectionWorker::task0_ready);
        this->connect(this->task0, &OpenCSGOWorker::finished, this->task0, &QObject::deleteLater);

        this->task1 = new DLLDownloadWorker(parent);
        this->connect(this->task1, &DLLDownloadWorker::status, this, &InjectionWorker::task1_status);
        this->connect(this->task1, &DLLDownloadWorker::ready, this, &InjectionWorker::task1_ready);
        this->connect(this->task1, &DLLDownloadWorker::finished, this->task1, &QObject::deleteLater);

        this->task2 = new DLLInjectionWorker(parent);
        this->connect(this->task2, &DLLInjectionWorker::status, this, &InjectionWorker::task2_status);
        this->connect(this->task2, &DLLInjectionWorker::ready, this, &InjectionWorker::task2_ready);
        this->connect(this->task2, &DLLInjectionWorker::finished, this->task2, &QObject::deleteLater);

        this->task3 = new EntryPointExecutionWorker(parent);
        this->connect(this->task3, &EntryPointExecutionWorker::status, this, &InjectionWorker::task3_status);
        this->connect(this->task3, &EntryPointExecutionWorker::ready, this, &InjectionWorker::task3_ready);
        this->connect(this->task3, &EntryPointExecutionWorker::finished, this->task3, &QObject::deleteLater);

        this->task0->start();
    }

    void run() override {

    }

public slots:
    void task0_status(std::string status) { this->statuses[0] = status; emit this->status(this->statuses); };
    void task0_ready(HANDLE handle) { this->task1->start(); };

    void task1_status(std::string status) { this->statuses[1] = status; emit this->status(this->statuses); };
    void task1_ready(std::vector<char> data) { this->task2->start(); };

    void task2_status(std::string status1, std::string status2) { this->statuses[2] = status1; this->statuses[3] = status2; emit this->status(this->statuses); };
    void task2_ready(void* remoteAddress) { this->task3->start(); };

    void task3_status(std::string status) { this->statuses[4] = status; emit this->status(this->statuses); };
    void task3_ready() { emit ready(); };

signals:
    void status(std::vector<std::string> statuses);
    void ready();
};