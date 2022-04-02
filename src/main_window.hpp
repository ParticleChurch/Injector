#pragma once

#include <memory>
#include <fstream>

#include <QtGui/QFontDatabase>
#include <QtGui/QFont>
#include <QtCore/QEvent>
#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtGui/QMovie>

#include "title_bar_theme.hpp"
#include "encryption.hpp"

#include "login_worker.hpp"

#include "controlled_manual_mapper.hpp"

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

class Button : public QPushButton {
	QString text = "Click Me!";
	std::unique_ptr<QMovie> movie;
	std::unique_ptr<QLabel> movieContainer;
	bool enabled = true;
	bool loading = false;

	void resizeMovie()
	{
		const int w = this->width(), h = this->height();
		const int sz = std::min(w, h) * 0.7;

		this->movieContainer->setGeometry(0, 0, w, h);
		this->movie->setScaledSize({ sz, sz });
	}

public:
	Button(QWidget* parent = nullptr) : QPushButton(parent)
	{

		this->movieContainer = std::make_unique<QLabel>(this);
		this->movieContainer->setAttribute(Qt::WA_TranslucentBackground);
		this->movieContainer->setAlignment(Qt::AlignCenter);
		this->movieContainer->setGeometry(0, 0, this->width(), this->height());

		this->movie = std::make_unique<QMovie>(":/img/loading.gif");
		this->resizeMovie();
		this->movie->start();

		this->restyle();

		this->connect(new HoverWatcher(this), &HoverWatcher::hoverChanged, this, &Button::restyle);

		if (parent) parent->installEventFilter(this);
	}

	void restyle()
	{
		QString opacity = (this->enabled && this->underMouse()) ? "0.8" : "1";

		this->setStyleSheet(
			"border-radius: 5px;"
			"color: rgba(255, 255, 255, " + opacity + ");"
			"font-size: 16px;"
			"background-color: rgba(58, 121, 199, " + opacity + ");"
		);

		this->setCursor(this->enabled ? QCursor(Qt::PointingHandCursor) : QCursor());
	}

	void setText(const QString& text)
	{
		this->text = text;

		if (!this->loading) QPushButton::setText(this->text);
	}

	virtual bool eventFilter(QObject* obj, QEvent* event) override
	{
		switch (event->type())
		{
		case QEvent::Resize:
			this->resizeMovie();
			break;
		}

		return false;
	}

	void enable()
	{
		this->enabled = true;
		this->restyle();
	}

	void disable()
	{
		this->enabled = false;
		this->restyle();
	}

	void startLoading()
	{
		this->loading = true;
		this->movieContainer->setMovie(this->movie.get());
		QPushButton::setText("");
	}

	void stopLoading()
	{
		this->loading = false;
		this->movieContainer->setMovie(nullptr);
		QPushButton::setText(this->text);
	}
};

class Input : public QLineEdit {
	bool invalid = false;

public:
	Input(QWidget* parent = nullptr) : QLineEdit(parent)
	{
		this->connect(new FocusWatcher(this), &FocusWatcher::focusChanged, this, &Input::restyle);
		this->connect(this, &Input::textChanged, this, &Input::restyle);
	}

	void restyle()
	{
		bool empty = this->text() == "";
		bool focus = this->hasFocus();

		QString opacity = "1";
		if (empty) opacity = focus ? "0" : "0.6";

		QString border = this->invalid ? "2px solid rgb(166, 61, 61)" : "none";
		QString padding = this->invalid ? "8px" : "10px";

		this->setStyleSheet(
			"background-color: rgb(26, 29, 32);"
			"border-radius: 5px; "
			"padding: " + padding + ";"
			"font-size: 16px;"
			"color: rgba(255, 255, 255, " + opacity + ");"
			"border: " + border + ";"
		);
	}

	void setInvalid(bool invalid)
	{
		this->invalid = invalid;
		this->restyle();
	}
};

class MainWindow : public QWidget
{
	Q_OBJECT;

private:
	std::unique_ptr<Input> email;
	std::unique_ptr<Input> password;
	std::unique_ptr<Button> playAnon;
	std::unique_ptr<Button> playAuth;

	std::unique_ptr<QFont> OpenSans400;
	std::unique_ptr<QFont> OpenSans600;
	std::unique_ptr<QFont> OpenSans700;

public:
	explicit MainWindow(QWidget* parent = nullptr) : QWidget(parent)
	{
		ControlledManualMapper::function();

		/*
			window styles
		*/
		this->setStyleSheet("background-color: rgb(40, 42, 44); border: none; outline: none; color: white;");
		this->setFixedSize(340, 346);
		TitleBarTheme::update((HWND)this->winId());

		/*
			children
		*/
		this->email = std::make_unique<Input>(this);
		this->password = std::make_unique<Input>(this);
		this->playAnon = std::make_unique<Button>(this);
		this->playAuth = std::make_unique<Button>(this);

		/*
			children styles
		*/
		this->loadFonts();

		this->email->setPlaceholderText("example@particle.church");
		this->password->setPlaceholderText("Password");

		this->email->setGeometry(24, 53, 292, 42);
		this->password->setGeometry(24, 148, 292, 42);

		this->email->restyle();
		this->password->restyle();

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

		this->loadLoginInfo();
		this->connect(this->playAuth.get(), &QPushButton::clicked, this, &MainWindow::onLogin);

		// fix placeholder garbage
		this->setFocus();
	}

private:
	void loadFonts()
	{
		{
			int id = QFontDatabase::addApplicationFont(":/font/OpenSans/400.otf");
			QString family = QFontDatabase::applicationFontFamilies(id).at(0);
			this->OpenSans400 = std::make_unique<QFont>(family);
			this->OpenSans400->setStyleStrategy(QFont::PreferAntialias);
		}
		{
			int id = QFontDatabase::addApplicationFont(":/font/OpenSans/600.otf");
			QString family = QFontDatabase::applicationFontFamilies(id).at(0);
			this->OpenSans600 = std::make_unique<QFont>(family);
			this->OpenSans600->setStyleStrategy(QFont::PreferAntialias);
		}
		{
			int id = QFontDatabase::addApplicationFont(":/font/OpenSans/700.otf");
			QString family = QFontDatabase::applicationFontFamilies(id).at(0);
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
			this->email->restyle();
			this->password->restyle();
		}
		catch (...) {
			return file.close();
		}

		file.close();
	}

	void saveLoginInfo()
	{
		using Encryption::Util::hex, Encryption::encrypt;

		std::string temp = std::getenv("TEMP");
		std::ofstream file(temp + "\\particle.church.login", std::ios::binary);

		if (!file || !file.is_open()) return;

		std::string email = this->email->text().toStdString();
		std::string password = this->password->text().toStdString();

		std::string encryptedData = encrypt(hex(email) + "," + hex(password));

		file.write(encryptedData.data(), encryptedData.size());
		file.close();
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
		this->email->restyle();
		this->password->restyle();
	}

	void onLogin()
	{
		this->playAnon->disable();
		this->playAuth->disable();
		this->playAuth->startLoading();

		this->saveLoginInfo();

		LoginWorkerThread* t = new LoginWorkerThread(this, this->email->text().toStdString(), this->password->text().toStdString());
		this->connect(t, &LoginWorkerThread::resultReady, this, &MainWindow::onLoginResult);
		this->connect(t, &LoginWorkerThread::finished, t, &QObject::deleteLater);
		t->start();
	}

	void onLoginResult(bool success, std::string error_or_sessionId, int statusCode_or_userId)
	{
		this->playAuth->stopLoading();
		this->playAnon->enable();
		this->playAuth->enable();

		if (!success)
		{
			const std::string& error = error_or_sessionId;
			const int& statusCode = statusCode_or_userId;

			this->email->setInvalid(statusCode == 400 || statusCode == 404);
			this->password->setInvalid(statusCode == 400 || statusCode == 401);
		} 
		else
		{
			const std::string& sessionId = error_or_sessionId;
			const int& userId = statusCode_or_userId;

			this->email->setInvalid(false);
			this->password->setInvalid(false);

			MessageBoxA(MB_OK, ("LOGGED IN AS USER " + std::to_string(userId)).c_str(), NULL, NULL);
		}
	}
};