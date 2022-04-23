#include <QtWidgets/QApplication>
#include "main_window.hpp"

#include <filesystem>
namespace filesystem = std::filesystem;

#include <Windows.h>


int main(int argc, char** argv)
{
	if (argc > 1)
	{
		try {
			// give some time for that process to exit
			Sleep(2000);

			filesystem::path p = argv[1];
			filesystem::remove(p);
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