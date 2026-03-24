#pragma once

// built-in
#include <unordered_map>
#include <string>
#include <QWidget>

// external library
#include <QMainWindow>

// internal library
#include "EntryBaseClass.h"
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

	EntryBaseClass* entryBase;
	DatabaseSelection* databaseSelection;

	// variables
	std::unordered_map<std::string, QWidget*> widgets;
	dict_like_string defaultHeaders;

	// signals
	void onLoadDatabase(std::string db_name);
	void onUnloadDatabase();

	// Ui
	void setupUiDatabaseSelection();
	void setupSignals();
	void getConfig();

	// database
	void setupDefaultDatabase();
};
