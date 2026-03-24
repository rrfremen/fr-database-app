#pragma once

// built-in
#include <vector>
#include <string>
#include <unordered_map>

// external library
#include <QWidget>

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

	// variables
	

signals:
	void signalLoadDB(std::string db_name);
	void signalUnloadDB();

private:
	Ui::DatabaseSelection* ui;

	std::vector<std::string> listAvailDatabase;
	dict_like_string newHeaders;

	bool newDefault = false;

	// functions
	void setupUiSignals();
	void populateDatabaseComboBox();
	void newDatabase();
	void onLoadDatabase();
	void onUnloadDatabase();
};