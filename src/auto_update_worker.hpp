#pragma once
#include "encryption.hpp"
#include "http.hpp"
#include <QtCore/QtCore>
#include <QtWidgets/QWidget>

#include <Windows.h>

#define VERSION "0.0.1"

class AutoUpdateWorker : public QThread
{
    Q_OBJECT;

    HWND parentHandle = NULL;

public:
    using QThread::QThread;
    AutoUpdateWorker(QWidget* p) : QThread(p)
    {
        this->parentHandle = (HWND)p->winId();
    }

    void run() override
    {
        using namespace Encryption::Encoding;

        auto response = HTTP::get(
            "dist/injector/version",
            {
                { "local_version", VERSION },
            }
        );
        Sleep(5000);

        std::string errorMessage = "[An unknown error occurred]";

        if (response.error)
        {
            errorMessage = "Failed to establish a connection with https://api.particle.church (error: " + response.error.message + ").";
            goto error;
        }

        if (response.status_code == 200)
        {
            HTTP::JSON::json body = HTTP::JSON::json::parse(response.text);

            if (!body.contains("update_required") || !body["update_required"].is_boolean())
            {
                errorMessage =
                    "Invalid API response. Please report this error to a developer. " +
                    Base64::encode("`update_required` not found: " + response.text);
                goto error;
            }

            emit this->resultReady(body["update_required"]);
            goto end;
        }
        else
        {
            qDebug() << response.text.c_str();
            errorMessage = "Failed to check for updates (got HTTP status code " + std::to_string(response.status_code) + ").";
            goto error;
        }

        goto end;
    error:
        errorMessage += "\n" \
            "The injector will not work if it isn't up-to-date.\n" \
            "Press OK to visit https://particle.church and re-download.";
        switch (MessageBox(this->parentHandle, errorMessage.c_str(), "Update Failed", MB_ICONERROR | MB_OKCANCEL))
        {
        default:
        case IDOK:
            system("start https://particle.church");
            std::exit(0);
            break;
        case IDCANCEL:
            return;
        }

    end:;
    }

signals:
    void resultReady(bool updateRequired);
};