#pragma once
#include "common.hpp"

class Worker : public QObject
{
    Q_OBJECT;

private:
    QThread* thread;

	bool die = false;

public:
    Worker(QObject* parent) : QObject(nullptr) // the worker itself has no parent
    {
        this->thread = new QThread(parent);

		this->connect(
			this, &Worker::finished,
			this->thread, &QThread::quit
		);
		this->connect(
			this->thread, &QThread::started,
			this, &Worker::work
		);

        this->moveToThread(this->thread);
        this->thread->start();
    }

	void cancel()
	{
		this->die = true;
		this->thread->quit();
		this->thread->wait();
	}

public slots:
    void work()
    {
		cpr::Response response = cpr::Get(
			cpr::Url{ "https://www.particle.church" }
		);

		emit this->setText(response.text);

		emit this->finished();
    }

signals:
	void finished();
	void setText(std::string text);
};