#pragma once

#include <QMainWindow>

#include "EntryBuyer.h"
#include "EntryProperty.h"

// external libraries
#include "../external/sqlite3.h"

namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget* parent = nullptr);
	~MainWindow();

	// declare the database
	sqlite3* db;
	int rc;
	const char* buyer;
	const char* properties;
	int errMsg;

private:
	Ui::MainWindow* ui;
	EntryBuyer* entryBuyer;
	EntryProperty* entryProperty;

		// functions
	void setupUiLocal();
	void createDatabase();
	void connectionMessage();
	void createTable();
	void buyerMessage();
	void propertiesMessage();

};
