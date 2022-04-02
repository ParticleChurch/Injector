#pragma once
#include "encryption.hpp"
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
        using namespace Encryption::Encoding;

        auto response = HTTP::get(
            "injector/version",
            {
                { "my_version", VERSION },
            }
        );

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

            bool updateRequired = body["update_required"];
            if (!updateRequired) goto end;

            MessageBox(NULL, "Update Required", "TODO", MB_ICONINFORMATION | MB_OK);
        }
        else
        {
            errorMessage = "Failed to check for updates (got HTTP status code " + std::to_string(response.status_code) + ").";
            goto error;
        }

        goto end;
    error:
        errorMessage += "\n" \
            "The injector will not work if it isn't up-to-date.\n" \
            "Press OK to visit https://particle.church and re-download.";
        switch (MessageBox(NULL, errorMessage.c_str(), "Update Failed", MB_ICONERROR | MB_OKCANCEL))
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
};