#pragma once

// built-in
#include <vector>
#include <string>
#include <unordered_map>

// external library
#include <QWidget>
#include "../external/sqlite3.h"
#include "../external/nlohmann-json.hpp"

// internal library

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

	// functions for example database
	void ensureExampleDatabaseExists();
	bool createExampleDatabaseFromJson(const std::string& filePath);
	bool createTableFromDefinition(sqlite3* db, const std::string& tableName, const nlohmann::json& definition);
	bool insertRowsFromDefinition(sqlite3* db, const std::string& tableName, const nlohmann::json& definition, const nlohmann::json& content);
	std::string escapeSqlValue(const std::string& value); // protects sql query from breaking when the text contain ' (single quote)
};