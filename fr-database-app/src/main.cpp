// built-in
#include <iostream>

// built-in - windows specific
#ifdef _WIN32
#include <windows.h>
#endif

// external
#include <QApplication>

// internal
#include "../include/MainWindow.h"

void SetUpWindowsConsole() {
#ifdef _WIN32
	AllocConsole();
	FILE* f;
	freopen_s(&f, "CONOUT$", "w", stdout);
#endif
}


int main(int argc, char* argv[])
{
	SetUpWindowsConsole();
	std::cout << "Starting Application" << std::endl;
	QApplication app(argc, argv);

	// uncomment once functionalities with threading are added
	// qRegisterMetaType<sqlite3*>("sqlite3*");

	MainWindow window;
	window.show();

	return app.exec();
}