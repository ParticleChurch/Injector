#include <QtWidgets/QApplication>
#include "main_window.hpp"

int main(int argc, char** argv)
{
	QApplication app(argc, argv);

	app.setAttribute(Qt::AA_EnableHighDpiScaling);
	app.setAttribute(Qt::AA_UseHighDpiPixmaps);

	MainWindow mainWindow{};
	mainWindow.show();

	return app.exec();
}