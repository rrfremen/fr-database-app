#pragma once

// external libraries
#include "../external/sqlite3.h"

// internal libraries 
#include <QMainWindow>
#include "EntryBuyer.h"
#include "EntryProperty.h"

namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget* parent = nullptr);
	~MainWindow();

private:
	Ui::MainWindow* ui;
	EntryBuyer* entryBuyer = nullptr;
	EntryProperty* entryProperty = nullptr;

	// declare the database
	sqlite3* db = nullptr;
	int rc = 0;
	int errMsg = 0;

	const char* buyer = nullptr;
	const char* properties = nullptr;

	// functions
	void setupUiLocal();
	void createDatabase();
	void connectionMessage();
	void createTable();
	void buyerMessage();
	void propertiesMessage();

};
