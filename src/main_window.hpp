#pragma once

#include <memory>
#include <fstream>

#include <QFontDatabase>
#include <QFont>
#include <QEvent>
#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>

#include "title_bar_theme.hpp"
#include "encryption.hpp"

class FocusWatcher : public QObject
{
	Q_OBJECT
public:
	explicit FocusWatcher(QObject* parent = nullptr) : QObject(parent)
	{
		if (parent)
			parent->installEventFilter(this);
	}

	virtual bool eventFilter(QObject* obj, QEvent* event) override
	{
		switch (event->type())
		{
		case QEvent::FocusIn:
		case QEvent::FocusOut:
			emit focusChanged();
			break;
		}

		return false;
	}

Q_SIGNALS:
	void focusChanged();
};

class HoverWatcher : public QObject
{
	Q_OBJECT
public:
	explicit HoverWatcher(QObject* parent = nullptr) : QObject(parent)
	{
		if (parent)
			parent->installEventFilter(this);
	}

	virtual bool eventFilter(QObject* obj, QEvent* event) override
	{
		switch (event->type())
		{
		case QEvent::HoverEnter:
		case QEvent::HoverLeave:
			emit hoverChanged();
			break;
		}

		return false;
	}

Q_SIGNALS:
	void hoverChanged();
};

class MainWindow : public QWidget
{
	Q_OBJECT;

private:
	std::unique_ptr<QLineEdit> email;
	std::unique_ptr<QLineEdit> password;
	std::unique_ptr<QPushButton> playAnon;
	std::unique_ptr<QPushButton> playAuth;

	std::unique_ptr<QFont> OpenSans400;
	std::unique_ptr<QFont> OpenSans600;
	std::unique_ptr<QFont> OpenSans700;

public:
	explicit MainWindow(QWidget* parent = nullptr) : QWidget(parent)
	{
		/*
			window styles
		*/
		this->setStyleSheet("background-color: rgb(40, 42, 44); border: none; outline: none; color: white;");
		this->setFixedSize(340, 346);
		TitleBarTheme::update((HWND)this->winId());

		/*
			children
		*/
		this->email = std::make_unique<QLineEdit>(this);
		this->password = std::make_unique<QLineEdit>(this);
		this->playAnon = std::make_unique<QPushButton>(this);
		this->playAuth = std::make_unique<QPushButton>(this);

		/*
			children styles
		*/
		this->loadFonts();

		this->email->setPlaceholderText("example@particle.church");
		this->password->setPlaceholderText("Password");

		this->email->setGeometry(24, 53, 292, 42);
		this->password->setGeometry(24, 148, 292, 42);

		this->stylePlaceholder(this->email.get());
		this->stylePlaceholder(this->password.get());

		this->email->setFont(*this->OpenSans600);
		this->password->setFont(*this->OpenSans600);
		this->password->setEchoMode(QLineEdit::Password);

		{
			auto email = new QLabel("Email", this);
			auto password = new QLabel("Password", this);

			email->setStyleSheet(
				"font-size: 24px;"
				"color: white;"
				"background-color: rgba(0,0,0,0);"
			);
			password->setStyleSheet(email->styleSheet());

			email->setFont(*this->OpenSans700);
			password->setFont(*this->OpenSans700);

			email->setGeometry(24, 12, 292, 40);
			password->setGeometry(24, 107, 292, 40);
		}

		this->playAuth->setText("Log In");
		this->playAnon->setText("Play Anonymously");

		this->playAuth->setGeometry(24, 214, 292, 42);
		this->playAnon->setGeometry(24, 280, 292, 42);

		this->playAuth->setFont(*this->OpenSans400);
		this->playAnon->setFont(*this->OpenSans400);

		this->playAuth->setCursor(QCursor(Qt::PointingHandCursor));
		this->playAnon->setCursor(QCursor(Qt::PointingHandCursor));

		this->styleButton(this->playAuth.get());
		this->styleButton(this->playAnon.get());
		
		this->connect(new HoverWatcher(this->playAuth.get()), &HoverWatcher::hoverChanged, this, &MainWindow::recalculateHover);
		this->connect(new HoverWatcher(this->playAnon.get()), &HoverWatcher::hoverChanged, this, &MainWindow::recalculateHover);

		this->loadLoginInfo();
		this->connect(this->playAuth.get(), &QPushButton::clicked, this, &MainWindow::onLogin);

		// fix placeholder garbage
		this->connect(new FocusWatcher(this->email.get()), &FocusWatcher::focusChanged, this, &MainWindow::recalculatePlaceholders);
		this->connect(new FocusWatcher(this->password.get()), &FocusWatcher::focusChanged, this, &MainWindow::recalculatePlaceholders);
		this->connect(this->email.get(), &QLineEdit::textChanged, this, &MainWindow::recalculatePlaceholders);
		this->connect(this->password.get(), &QLineEdit::textChanged, this, &MainWindow::recalculatePlaceholders);
		this->setFocus();
	}

private:
	void loadFonts()
	{
		{
			int id = QFontDatabase::addApplicationFont(":/fonts/OpenSans/400.otf");
			auto family = QFontDatabase::applicationFontFamilies(id).at(0);
			this->OpenSans400 = std::make_unique<QFont>(family);
			this->OpenSans400->setStyleStrategy(QFont::PreferAntialias);
		}
		{
			int id = QFontDatabase::addApplicationFont(":/fonts/OpenSans/600.otf");
			auto family = QFontDatabase::applicationFontFamilies(id).at(0);
			this->OpenSans600 = std::make_unique<QFont>(family);
			this->OpenSans600->setStyleStrategy(QFont::PreferAntialias);
		}
		{
			int id = QFontDatabase::addApplicationFont(":/fonts/OpenSans/700.otf");
			auto family = QFontDatabase::applicationFontFamilies(id).at(0);
			this->OpenSans700 = std::make_unique<QFont>(family);
			this->OpenSans700->setStyleStrategy(QFont::PreferAntialias);
		}
	}

	void loadLoginInfo()
	{
		std::string filepath = std::string(std::getenv("TEMP")) + "\\particle.church.login";
		std::ifstream file(filepath, std::ios::binary | std::ios::ate);

		if (!file || !file.is_open()) return;

		size_t fileEnd = file.tellg();
		file.seekg(0, std::ios::beg);
		size_t fileBegin = file.tellg();

		std::vector<char> fileDataVector(fileEnd - fileBegin);
		if (!file.read((char*)fileDataVector.data(), fileDataVector.capacity())) return file.close();

		std::string fileData(fileDataVector.begin(), fileDataVector.end());

		try {
			std::string decrypted = Encryption::decrypt(fileData);
			
			size_t comma = decrypted.find_first_of(',');
			if (comma >= decrypted.size()) return;

			std::string email = Encryption::unhex(decrypted.substr(0, comma));
			std::string password = Encryption::unhex(decrypted.substr(comma + 1));

			this->email->setText(email.c_str());
			this->password->setText(password.c_str());
			this->stylePlaceholder(this->email.get());
			this->stylePlaceholder(this->password.get());
		}
		catch (...) {
			return file.close();
		}

		file.close();
	}

	void saveLoginInfo()
	{
		std::string temp = std::getenv("TEMP");
		std::ofstream file(temp + "\\particle.church.login", std::ios::binary);

		if (!file || !file.is_open()) return;

		std::string data = Encryption::hex(this->email->text().toStdString()) + "," + Encryption::hex(this->password->text().toStdString());
		std::string encrypted = Encryption::encrypt(data);
		file.write(encrypted.data(), encrypted.size());
		file.close();
	}

	void styleButton(QPushButton* q)
	{
		QString opacity = q->underMouse() ? "0.8" : "1";

		q->setStyleSheet(
			"border-radius: 5px;"
			"color: rgba(255, 255, 255, " + opacity + ");"
			"font-size: 16px;"
			"background-color: rgba(58, 121, 199, " + opacity + ");"
		);
	}

	void stylePlaceholder(QLineEdit* q)
	{
		bool empty = q->text() == "";
		bool focus = q->hasFocus();

		QString opacity = "1";
		if (empty)
			opacity = focus ? "0" : "0.6";
		
		q->setStyleSheet(
			"background-color: rgb(26, 29, 32);"
			"border-radius: 5px; "
			"padding: 10px;"
			"font-size: 16px;"
			"color: rgba(255, 255, 255, " + opacity + ");"
		);
	}

	virtual void mousePressEvent(QMouseEvent* evt)
	{
		this->email->clearFocus();
		this->password->clearFocus();

		QWidget::mousePressEvent(evt);
	}

signals:

public slots:

	void recalculatePlaceholders()
	{
		this->stylePlaceholder(this->email.get());
		this->stylePlaceholder(this->password.get());
	}

	void recalculateHover()
	{
		this->styleButton(this->playAuth.get());
		this->styleButton(this->playAnon.get());
	}

	void onLogin()
	{
		this->saveLoginInfo();
	}
};