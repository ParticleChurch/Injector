#pragma once

#include <memory>
#include <fstream>
#include <vector>
#include <map>

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
#include "update_workers.hpp"

#include "qt_loading_spinner.hpp"

#include "controlled_manual_mapper.hpp"

#include "injection_workers.hpp"

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
	QLoadingSpinner* spinner;
	bool enabled = true;
	bool loading = false;

public:
	Button(QWidget* parent = nullptr) : QPushButton(parent)
	{
		this->setFocusPolicy(Qt::FocusPolicy::NoFocus);

		this->spinner = new QLoadingSpinner(this, 0.5);

		this->restyle();

		this->connect(new HoverWatcher(this), &HoverWatcher::hoverChanged, this, &Button::restyle);

		if (parent) parent->installEventFilter(this);

		this->setLoading(false);
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
			this->spinner->recenter();
			break;
		}

		return false;
	}

	void setEnabled(bool enabled)
	{
		((QPushButton*)this)->setEnabled(enabled);
		this->enabled = enabled;
		this->restyle();
	}

	void setLoading(bool loading)
	{
		this->loading = loading;

		if (loading) this->spinner->show();
		else this->spinner->hide();

		QPushButton::setText(loading ? "" : this->text);
		this->setEnabled(!loading);
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
		bool invalid = this->invalid;

		QString textColor = empty ? "rgb(117, 117, 117)" : "rgb(235, 235, 235)";
		QString border = focus ? "1px solid rgb(120, 120, 120)" : invalid ? "2px solid rgb(166, 61, 61)" : "none";
		QString padding = focus ? "9px" : invalid ? "8px" : "10px";

		this->setStyleSheet(
			"background-color: rgb(55, 57, 60);"
			"border-radius: 5px; "
			"padding: " + padding + ";"
			"font-size: 16px;"
			"color: " + textColor + ";"
			"border: " + border + ";"
		);
	}

	void setInvalid(bool invalid)
	{
		this->invalid = invalid;
		this->restyle();
	}
};

class H1 : public QLabel {
public:
	H1(QString text, QWidget* parent = nullptr, QFont font = QFont()): QLabel(text, parent) {
		this->setStyleSheet(
			"font-size: 24px;"
			"color: rgb(255, 255, 255);"
			"background-color: rgba(0,0,0,0);"
		);
		this->setFont(font);
	}
};

class H2 : public QLabel {
public:
	H2(QString text, QWidget* parent = nullptr, QFont font = QFont()) : QLabel(text, parent) {
		this->setStyleSheet(
			"font-size: 18px;"
			"color: rgb(255, 255, 255);"
			"background-color: rgba(0,0,0,0);"
		);
		this->setFont(font);
	}
};

class H3 : public QLabel {
public:
	H3(QString text, QWidget* parent = nullptr, QFont font = QFont()) : QLabel(text, parent) {
		this->setStyleSheet(
			"font-size: 14px;"
			"color: rgb(150, 150, 150);"
			"background-color: rgba(0,0,0,0);"
		);
		this->setFont(font);
	}
};

class Task : public QWidget {
	QLabel* title;
	QLabel* subtitle;
	QWidget* icon;
	QLoadingSpinner* spinnerIcon;

public:
	Task(QWidget* parent, QString title, QFont titleFont, QString subtitle, QFont subtitleFont): QWidget(parent) {
		this->title = new H2(title, parent, titleFont);
		this->subtitle = new H3(subtitle, parent, subtitleFont);
		this->icon = new QWidget(this);
		this->spinnerIcon = new QLoadingSpinner(this->icon, 0.6f, 0.5f);

		this->geometryChanged();

		this->installEventFilter(this);
	}

	void geometryChanged() {
		static int lastw, lasth, lastx, lasty;
		int w = this->width(), h = this->height();
		int x = this->x(), y = this->y();
		int p = 5;

		this->icon->setGeometry(0, 0, h, h); // no clue why tf this is relative but nothing else is
		this->title->setGeometry(x + h + p, y, w - h - p, h * 0.6f);
		this->subtitle->setGeometry(x + h + p, y + this->title->height(), w - h - p, h * 0.4f + 0.5f);
		this->spinnerIcon->recenter();

		this->repaint();
	}

	virtual bool eventFilter(QObject* obj, QEvent* event) override
	{
		switch (event->type())
		{
		case QEvent::Resize:
		case QEvent::Move:
			this->geometryChanged();
			break;
		}

		return false;
	}

	void setStatus(const TaskStatus& status) {
		
		if (status.complete) {
			this->setSubtitle("Done!");
			this->spinnerIcon->setCheckmark(true);
		}
		else {
			this->setSubtitle(status.description);
			this->spinnerIcon->setCheckmark(false);
		}
	}

	void setSubtitle(std::string subtitle) {
		this->subtitle->setText(subtitle.c_str());
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
	std::unique_ptr<H1> emailLabel;
	std::unique_ptr<H1> passwordLabel;

	std::unique_ptr<H1> welcomeLabel;
	std::map<std::string, std::unique_ptr<Task>> tasks;

	std::unique_ptr<QFont> OpenSans400;
	std::unique_ptr<QFont> OpenSans600;
	std::unique_ptr<QFont> OpenSans700;

public:
	explicit MainWindow(QWidget* parent = nullptr) : QWidget(parent)
	{
		// version check
		UpdateCheckWorker* t = new UpdateCheckWorker(this);
		this->connect(t, &UpdateCheckWorker::resultReady, this, &MainWindow::onAutoUpdateResult);
		this->connect(t, &UpdateCheckWorker::finished, t, &QObject::deleteLater);
		t->start();

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

		this->emailLabel = std::make_unique<H1>("Email", this, *this->OpenSans700);
		this->passwordLabel = std::make_unique<H1>("Password", this, *this->OpenSans700);
		this->emailLabel->setGeometry(24, 12, 292, 40);
		this->passwordLabel->setGeometry(24, 107, 292, 40);

		this->playAuth->setText("Log In");
		this->playAnon->setText("Play Anonymously");

		this->playAuth->setGeometry(24, 214, 292, 42);
		this->playAnon->setGeometry(24, 280, 292, 42);

		this->playAuth->setFont(*this->OpenSans400);
		this->playAnon->setFont(*this->OpenSans400);

		// the buttons are loading until AutoUpdate finishes
		this->playAnon->setLoading(true);
		this->playAuth->setLoading(true);

		this->loadLoginInfo();
		this->connect(this->playAuth.get(), &QPushButton::clicked, this, &MainWindow::onLogin);
		this->connect(this->playAnon.get(), &QPushButton::clicked, this, &MainWindow::onPlayAnon);

		// injection screen
		this->welcomeLabel = std::make_unique<H1>("Welcome, Guest", this, *this->OpenSans700);
		this->welcomeLabel->setGeometry(24, 12, 292, 40);

		this->tasks["wait_for_csgo"] = std::make_unique<Task>(
			this,
			"Waiting for CS:GO", *this->OpenSans600,
			"step 1 subtitle", *this->OpenSans400
		);

		this->tasks["dll_download"] = std::make_unique<Task>(
			this,
			"Updating DLL", *this->OpenSans600,
			"step 2 subtitle", *this->OpenSans400
		);

		this->tasks["decrypt"] = std::make_unique<Task>(
			this,
			"Decrypting", *this->OpenSans600,
			"step 3 subtitle", *this->OpenSans400
		);

		this->tasks["inject"] = std::make_unique<Task>(
			this,
			"Injecting", *this->OpenSans600,
			"step 4 subtitle", *this->OpenSans400
		);

		this->tasks["start"] = std::make_unique<Task>(
			this,
			"Starting", *this->OpenSans600,
			"step 4 subtitle", *this->OpenSans400
		);

		int h = 60;
		for (const auto& key : { "wait_for_csgo", "dll_download", "decrypt", "inject", "start" }) {
			const auto& task = this->tasks[key];
			task->setGeometry(18, h, 292, 40);
			h += 56;
		}

		// setup
		this->shiftInjectionScreen();
	}

private:
	void loadFonts()
	{
		{
			int id = QFontDatabase::addApplicationFont(":/font/OpenSans/400.otf");
			QString family = QFontDatabase::applicationFontFamilies(id).at(0);
			this->OpenSans400 = std::make_unique<QFont>(family);
			this->OpenSans400->setStyleStrategy(QFont::StyleStrategy(QFont::PreferAntialias | QFont::PreferQuality));
			this->OpenSans400->setHintingPreference(QFont::HintingPreference::PreferNoHinting);
		}
		{
			int id = QFontDatabase::addApplicationFont(":/font/OpenSans/600.otf");
			QString family = QFontDatabase::applicationFontFamilies(id).at(0);
			this->OpenSans600 = std::make_unique<QFont>(family);
			this->OpenSans600->setStyleStrategy(QFont::StyleStrategy(QFont::PreferAntialias | QFont::PreferQuality));
			this->OpenSans600->setHintingPreference(QFont::HintingPreference::PreferNoHinting);
		}
		{
			int id = QFontDatabase::addApplicationFont(":/font/OpenSans/700.otf");
			QString family = QFontDatabase::applicationFontFamilies(id).at(0);
			this->OpenSans700 = std::make_unique<QFont>(family);
			this->OpenSans600->setStyleStrategy(QFont::StyleStrategy(QFont::PreferAntialias | QFont::PreferQuality));
			this->OpenSans700->setHintingPreference(QFont::HintingPreference::PreferNoHinting);
		}
	}

	void loadLoginInfo()
	{
		using namespace Encryption::Encoding;

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

			std::string email = Base16::decode(decrypted.substr(0, comma));
			std::string password = Base16::decode(decrypted.substr(comma + 1));

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
		using namespace Encryption::Encoding;

		std::string temp = std::getenv("TEMP");
		std::ofstream file(temp + "\\particle.church.login", std::ios::binary);

		if (!file || !file.is_open()) return;

		std::string email = this->email->text().toStdString();
		std::string password = this->password->text().toStdString();

		std::string encryptedData = Encryption::encrypt(Base16::encode(email) + "," + Base16::encode(password));

		file.write(encryptedData.data(), encryptedData.size());
		file.close();
	}

	virtual void mousePressEvent(QMouseEvent* evt)
	{
		this->email->clearFocus();
		this->password->clearFocus();

		QWidget::mousePressEvent(evt);
	}

	void transitionToLoadingScreen()
	{
		int duration = 1000;

		std::vector<QWidget*> transition = {
			this->email.get(),
			this->password.get(),
			this->playAuth.get(),
			this->playAnon.get(),
			this->emailLabel.get(),
			this->passwordLabel.get(),
			this->welcomeLabel.get(),
		};
		for (const auto& [k, v] : this->tasks) {
			transition.push_back(v.get());
		}

		for (auto element : transition) {
			QPropertyAnimation* a = new QPropertyAnimation(element, "geometry", this);
			a->setEasingCurve(QEasingCurve::InOutQuad);
			a->setDuration(duration);
			a->setEndValue(element->geometry().adjusted(this->width(), 0, this->width(), 0));
			a->start();
		}
	}

	void shiftInjectionScreen()
	{
		std::vector<QWidget*> shift = {
			this->welcomeLabel.get(),
		};
		for (const auto& [k, v] : this->tasks) {
			shift.push_back(v.get());
		}

		for (auto element : shift) {
			element->setGeometry(
				element->geometry().adjusted(-this->width(), 0, -this->width(), 0)
			);
		}
	}

	void inject()
	{
		// disable all interactions on login page
		this->playAnon->setLoading(false);
		this->playAnon->setLoading(false);
		this->playAnon->setEnabled(false);
		this->playAnon->setEnabled(false);
		this->email->setReadOnly(true);
		this->password->setReadOnly(true);

		// transition to injection page
		this->transitionToLoadingScreen();

		// start injection thread
		auto w = new InjectionWorker(this);
		this->connect(w, &InjectionWorker::status, this, &MainWindow::onInjectionStatus);
		this->connect(w, &InjectionWorker::complete, this, &MainWindow::onInjectionComplete);
		this->connect(w, &InjectionWorker::complete, this, &QObject::deleteLater);
		w->start();
	}

signals:

public slots:
	void onInjectionComplete() {
		this->close();
		std::exit(0);
	}

	void onInjectionStatus(std::vector<TaskStatus> statuses) {
		int i = 0;
		for (const auto& key : { "wait_for_csgo", "dll_download", "decrypt", "inject", "start" }) {
			const auto& task = this->tasks[key];
			const auto& status = statuses[i++];
			task->setStatus(status);
		}
	}

	void onPlayAnon()
	{
		this->welcomeLabel->setText("Welcome, Guest");
		this->inject();
	}

	void onLogin()
	{
		this->playAnon->setEnabled(false);
		this->playAuth->setLoading(true);

		this->saveLoginInfo();

		LoginWorkerThread* t = new LoginWorkerThread(this, this->email->text().toStdString(), this->password->text().toStdString());

		this->connect(t, &LoginWorkerThread::resultReady, this, &MainWindow::onLoginResult);
		this->connect(t, &LoginWorkerThread::finished, t, &QObject::deleteLater);
		t->start();
	}

	void onLoginResult(bool success, std::string error_or_sessionId, int statusCode_or_userId)
	{
		this->playAuth->setLoading(false);
		this->playAnon->setEnabled(true);

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

			this->welcomeLabel->setText(("Welcome, User " + std::to_string(userId)).c_str());
			this->inject();
		}
	}

	void onAutoUpdateResult(bool updateRequired)
	{
		if (!updateRequired)
		{
			this->playAuth->setLoading(false);
			this->playAnon->setLoading(false);
		}
		else
		{
			UpdateWorker* t = new UpdateWorker(this);
			this->connect(t, &UpdateWorker::finished, t, &QObject::deleteLater);
			t->start();
			MessageBox((HWND)this->winId(), "Please wait while the particle.church injector is updated.", "Updating...", MB_ICONINFORMATION | MB_OK);
		}
	}
};