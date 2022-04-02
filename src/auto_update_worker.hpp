#pragma once
#include "http.hpp"
#include <QtCore/QtCore>

#include <Windows.h>

#define VERSION "0.0.1"

class AutoUpdateWorker : public QThread
{
    Q_OBJECT;

public:
    using QThread::QThread;

    void run() override
    {
        auto response = HTTP::get(
            "injector/version",
            {
                { "my_version", VERSION },
            }
        );

        if (response.error)
        {
            MessageBox(NULL, "Hello", "Goodbye", MB_ICONERROR);
        }

        if (response.status_code == 200)
        {
            HTTP::JSON::json body = HTTP::JSON::json::parse(response.text);
            if (body.contains("session_id") && body.contains("user_id"))
            {
                
            }
            else
            {
                
            }
        }
        else
        {
            
        }
    }
};