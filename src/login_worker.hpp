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
                { "password", this->password }
            }
        );

        if (response.error)
        {
            return emit resultReady(false, "Unknown Error: " + response.error.message, "");
        }

        if (response.status_code == 200)
        {
            HTTP::JSON::json body = response.text;
            if (body.contains("session_id"))
            {
                return emit resultReady(true, "success", body["session_id"]);
            }
            else
            {
                return emit resultReady(false, "Server response did not contain a session id: " + response.text, "");
            }
        }
        else
        {
            return emit resultReady(false, "Request failed with static code: " + std::to_string(response.status_code), "");
        }
    }

signals:
    void resultReady(bool success, std::string error, std::string sessionId);
};