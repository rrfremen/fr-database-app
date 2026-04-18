#pragma once

// built-in
#include <string>
#include <vector>

// external library
#include <QWidget>
#include <QStandardItemModel>
#include <QTableView>
#include <QPushButton>
#include <QStringList>
#include "./external/sqlite3.h"


namespace Ui {
	class EntryForm;
}

class EntryForm : public QWidget 
{
	Q_OBJECT

public:
	EntryForm(QWidget* parent = nullptr, std::string tabName = "", sqlite3* database = nullptr);
	~EntryForm();

private:
	Ui::EntryForm* ui;
	std::string tableName;

	// database
	sqlite3* currentDB = nullptr;
	QStringList currentHeaders;

	void updateHeaders();
	void initUi();
	void setupConnections();

	void addEntry();
	void removeEntry();
	void editEntry();
	void loadTableFromQuery(const std::string& sql);
	void searchEntry();
	void loadSearchHeaders();
	void onSearchTextChanged(const QString& text);
	void reloadTable();
};
