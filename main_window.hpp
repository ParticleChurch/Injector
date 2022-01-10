#pragma once
#include "common.hpp"
#include "worker.hpp"
#include "title_bar_theme.hpp"

class MainWindow : public QWidget
{
	Q_OBJECT;

private:
	Worker* worker;
	QLabel* label;

public:
	explicit MainWindow(QWidget* parent = nullptr) : QWidget(parent)
	{
		/*
			window styles
		*/
		this->setStyleSheet("background-color: rgb(57, 60, 63);");
		this->setFixedSize(400, 200);
		TitleBarTheme::update((HWND)this->winId());

		/*
			children
		*/
		this->label = new QLabel(this);
		this->label->setGeometry(0, 0, 400, 200);
		this->label->setStyleSheet("color: white;");
		this->label->setWordWrap(true);
		this->label->setAlignment(Qt::AlignCenter);
		this->label->setText("Loading...");
		this->label->show();

		/*
			start worker
		*/
		this->worker = new Worker(this);
		this->connect(
			this->worker, &Worker::setText,
			this, &MainWindow::setText
		);
	}

signals:

public slots:
	void closed()
	{
		this->worker->cancel();
	};
	void setText(std::string text)
	{
		QString str = text.c_str();
		this->label->setText(str.toHtmlEscaped());
	}
};