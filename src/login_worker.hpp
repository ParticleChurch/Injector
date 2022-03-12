#pragma once
#include <QtCore>
#include "http.hpp"

class LoginWorkerThread : public QThread
{
    Q_OBJECT;

    std::string email, password;

public:
    LoginWorkerThread(QObject* parent, const std::string& email, const std::string& password) : QThread(parent)
    {
        this->email = email;
        this->password = password;
    }

    void run() override
    {
        auto response = HTTP::post(
            "sessions",
            {
                { "email", this->email },
                { "password", this->password },
                { "platform", "injector" },
            }
        );

        if (response.error)
        {
            return emit resultReady(false, "Unknown Error: " + response.error.message, response.status_code);
        }

        if (response.status_code == 200)
        {
            HTTP::JSON::json body = HTTP::JSON::json::parse(response.text);
            if (body.contains("session_id") && body.contains("user_id"))
            {
                return emit resultReady(true, body["session_id"], body["user_id"]);
            }
            else
            {
                return emit resultReady(false, "Invalid Server Response (missing required fields): " + response.text, response.status_code);
            }
        }
        else
        {
            return emit resultReady(false, "Bad Status Code: " + std::to_string(response.status_code), response.status_code);
        }
    }

signals:
    void resultReady(bool success, std::string error_or_sessionId, int statusCode_or_userId);
};