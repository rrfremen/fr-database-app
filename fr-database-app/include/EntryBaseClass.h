#pragma once

// built in
#include <string>
#include <vector>

// external
#include <QWidget>
#include <QStandardItemModel>
#include <QTableView>
#include <QPushButton>
#include <QStringList>
#include "./external/sqlite3.h"

class EntryBaseClass : public QWidget
{
	Q_OBJECT

public:
	EntryBaseClass(QWidget* parent = nullptr);
	
protected:
	sqlite3* db = nullptr; // for setDatabase
	QStringList getHeadersFromJson(const std::string& text); // for getting the headers

	void setupUiLocal(QTableView* table, const std::string& text);
	void setDatabase(sqlite3* database);
	void addEntry(QPushButton* button, QTableView* view, const std::string& text, const std::string& tableName);
	void reloadTable(QTableView* view, const std::string& text, const QStringList& headers);
};
