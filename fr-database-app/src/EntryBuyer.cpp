// built-in
#include <iostream>
#include <fstream>

// external libraries
#include "../external/sqlite3.h"

// internal libraries
#include "../include/EntryBuyer.h"
#include "ui_EntryBuyer.h"
#include "../include/EntryBaseClass.h"

EntryBuyer::EntryBuyer(sqlite3* database, QWidget* parent)
	: EntryBaseClass(parent)
	, ui(new Ui::EntryBuyer)
{
	ui->setupUi(this);
	setDatabase(database);
	setupUiLocal(ui->tableView_entry, "EntryBuyer");
	addEntry(ui->pushButton_add, ui->tableView_entry, "EntryBuyer", "buyer");
	reloadTable(ui->tableView_entry, "EntryBuyer", getHeadersFromJson("EntryBuyer"));
}

EntryBuyer::~EntryBuyer()
{
	delete ui;
}
