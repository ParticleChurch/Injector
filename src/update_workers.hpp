#pragma once
#include <regex>
#include <filesystem>
#include <fstream>
namespace filesystem = std::filesystem;

#include "encryption.hpp"
#include "http.hpp"

#include <QtCore/QtCore>
#include <QtWidgets/QWidget>

#include <Windows.h>

#define VERSION "0.9.0"

class UpdateCheckWorker : public QThread
{
    Q_OBJECT;

    HWND parentHandle = NULL;

public:
    using QThread::QThread;
    UpdateCheckWorker(QWidget* p) : QThread(p)
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
            errorMessage = "Failed to check for updates (got HTTP status code " + std::to_string(response.status_code) + ").";
            goto error;
        }

        goto end;
    error:
        errorMessage = \
            "~ Update Failed ~\n" \
            "The injector will not work if it isn't up-to-date.\n" \
            "Press OK to visit https://particle.church and re-download.\n\nError message:\n" +
            errorMessage;

        MessageBox(this->parentHandle, errorMessage.c_str(), "Update Failed", MB_ICONERROR | MB_OK);
        system("start https://particle.church");
        std::exit(0);

    end:;
    }

signals:
    void resultReady(bool updateRequired);
};

class UpdateWorker : public QThread
{
    Q_OBJECT;

    HWND parentHandle = NULL;

    static std::string determineFilename(const std::string contentDisposition) {
        static std::regex re("filename=\"([^\"]+)\"");
        
        std::smatch m;
        if (!std::regex_search(contentDisposition, m, re)) {
            return "injector_v0.0.0.exe";
        }

        return m[1].str();
    }

public:
    using QThread::QThread;
    UpdateWorker(QWidget* p) : QThread(p)
    {
        this->parentHandle = (HWND)p->winId();
    }

    void run() override
    {
        using namespace Encryption::Encoding;

        auto response = HTTP::get(
            "dist/injector",
            {
                { "updating_from", VERSION },
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
            filesystem::path oldFile;

            {
                char myExeName[MAX_PATH];
                GetModuleFileName(NULL, myExeName, MAX_PATH);

                oldFile = std::string(myExeName, MAX_PATH).c_str();
            }

            filesystem::path newFile = oldFile.parent_path() / determineFilename(response.header["content-disposition"]);

            std::ofstream f(newFile, std::ios::binary);
            if (!f.is_open())
            {
                errorMessage = "Failed to open " + newFile.string() + " for writing.";
                goto error;
            }

            f.write(response.text.data(), response.text.size());

            f.close();

            std::string command = "start /B /MIN \"\" \"" + newFile.string() + "\" \"" + oldFile.string() + "\"";
            system(command.c_str());
            std::exit(0);

            goto end;
        }
        else
        {
            errorMessage = "Failed to download new injector (got HTTP status code " + std::to_string(response.status_code) + ").";
            goto error;
        }

        goto end;
    error:
        errorMessage = \
            "~ Update Failed ~\n" \
            "The injector will not work if it isn't up-to-date.\n" \
            "Press OK to visit https://particle.church and re-download.\n\nError message:\n" +
            errorMessage;

        MessageBox(this->parentHandle, errorMessage.c_str(), "Update Failed", MB_ICONERROR | MB_OK);
        system("start https://particle.church");
        std::exit(0);

    end:;
    }
};
