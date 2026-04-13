#pragma once

// built-in
#include <unordered_map>
#include <string>
#include <QWidget>

// external library
#include <QMainWindow>
#include "../external/sqlite3.h"

// internal library
#include "DatabaseSelection.h"
#include "EntryForm.h"

// custom commands
using dict_like_string = std::unordered_map<std::string, std::vector<std::string>>;


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

	DatabaseSelection* databaseSelection;

	// variables
	std::unordered_map<std::string, QWidget*> widgets;
	dict_like_string config;

	// signals
	void receiveLoadDatabase(sqlite3* pointerDB, std::string dbTitle);
	void receiveUnloadDatabase();

	// Ui
	void setupUiDatabaseSelection();
	void setupConnections();
	void getConfig();

	// database
	sqlite3* currentDB = nullptr;
	std::string currentDBTitle;
	void setupDatabase();
};
