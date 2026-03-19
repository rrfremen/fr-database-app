// built-in
#include <iostream>
#include <fstream>

// external libraries
#include "../external/sqlite3.h"

// internal libraries
#include "../include/EntryProperty.h"
#include "ui_EntryProperty.h"
#include "../include/EntryBaseClass.h"

EntryProperty::EntryProperty(sqlite3* database, QWidget* parent)
	: EntryBaseClass(parent)
	, ui(new Ui::EntryProperty)
{
	ui->setupUi(this);
	setDatabase(database);
	setupUiLocal(ui->tableView_entry, "EntryProperty");
	addEntry(ui->pushButton_add, ui->tableView_entry, "EntryProperty", "properties");
	reloadTable(ui->tableView_entry, "EntryProperty", getHeadersFromJson("EntryProperty"));
}

EntryProperty::~EntryProperty()
{
	delete ui;
}