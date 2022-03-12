#pragma once
#include <QtCore>

class LoginWorker : public QObject
{
    Q_OBJECT;

    std::string email, password;

public:
    LoginWorker(QObject* parent, std::string email, std::string password) : QObject(parent)
    {
        this->email = email;
        this->password = password;
    }

public slots:
    void start()
    {
        
    }

signals:
    void resultReady(bool success, std::string error, std::string sessionId);
};