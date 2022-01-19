#pragma once
#include "common.hpp"
#include "injector.hpp"

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
			cpr::Url{ "https://api.particle.church/dll/" }
		);

		if (response.error.code != cpr::ErrorCode::OK) {
			MessageBox(0, L"Failed to download.", L"Injection Failed", MB_OK);
			std::exit(0);
		}

		char* dll = response.text.data();
		size_t dllSize = response.text.size();

		Utils::mapXOR((unsigned char*)dll, dllSize);

		Injector injector{};
		injector.openCSGO();


		injector.inject(dll, dllSize);

		std::exit(0);
		emit this->finished();
    }

signals:
	void finished();
	void setText(std::string text);
};