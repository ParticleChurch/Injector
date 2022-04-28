#include <QtWidgets/QApplication>

#include "main_window.hpp"
#include "os.hpp"

int main(int argc, char** argv)
{
	// get csgo running if it isn't already
	OS::system("start steam://run/730");

	if (argc > 1)
	{
		try {
			// give some time for that process to exit
			Sleep(2000);

			OS::filesystem::path p = argv[1];
			OS::filesystem::remove(p);
		}
		catch (...) {
			// if it doesn't work, then honestly who cares
		}
	}

	QApplication app(argc, argv);
	MainWindow mainWindow{};

	// BEG microsoft to foreground my window!!
	mainWindow.show();
	mainWindow.setWindowState((mainWindow.windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
	mainWindow.setFocus();
	mainWindow.activateWindow();
	mainWindow.raise();

	return app.exec();
}