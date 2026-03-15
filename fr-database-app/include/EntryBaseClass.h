#pragma once

// built in
#include <string>

// external
#include <QWidget>
#include <QTableView>

class EntryBaseClass : public QWidget
{
	Q_OBJECT

public:
	EntryBaseClass(QWidget* parent = nullptr);

protected:
	void setupUiLocal(QTableView* table, const std::string& text);
};
