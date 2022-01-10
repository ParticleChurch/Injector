#include "common.hpp"
#include "main_window.hpp"

int main(int argc, char** argv)
{
	QApplication app(argc, argv);

	MainWindow mainWindow{};
	mainWindow.show();

	mainWindow.connect(
		&app, SIGNAL(aboutToQuit()),
		&mainWindow, SLOT(closed())
	);

	return app.exec();
}