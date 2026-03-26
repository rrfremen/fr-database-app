#pragma once

// built-in
#include <vector>
#include <string>
#include <unordered_map>

// external library
#include <QWidget>
#include "../external/sqlite3.h"

// internal library
#include "DatabaseSelection.h"

// custom commands
using dict_like_string = std::unordered_map<std::string, std::vector<std::string>>;

namespace Ui {
	class DatabaseSelection;
}

class DatabaseSelection : public QWidget
{
	Q_OBJECT

public:
	DatabaseSelection(QWidget* parent = nullptr);
	~DatabaseSelection();

	dict_like_string defaultHeaders;

signals:
	void signalLoadDB(sqlite3* pointerDB, std::string titleDB);
	void signalUnloadDB();

private:
	Ui::DatabaseSelection* ui;

	std::vector<std::string> listAvailDatabase;
	dict_like_string newHeaders;

	// database
	bool newDefault = false;
	sqlite3* currentDB = nullptr;
	std::string folderPathDB = "localDatabase";

	// functions
	void initUi();
	void setupConnections();
	void populateDatabaseComboBox();
	void onNewDatabase();
	void onLoadDatabase();
	void onUnloadDatabase();
	int openDatabase(std::string fileName);
	void createDatabase(std::string newTitle);
	void createDatabaseTables(std::string newTitle);
};