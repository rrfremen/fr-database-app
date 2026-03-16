// built-in
#include <iostream>
#include <fstream>

// external libraries
#include "../external/sqlite3.h"

// internal libraries
#include "../include/EntryBuyer.h"
#include "ui_EntryBuyer.h"

EntryBuyer::EntryBuyer(QWidget* parent)
	: EntryBaseClass(parent)
	, ui(new Ui::EntryBuyer)
{
	ui->setupUi(this);
	setupUiLocal(ui->tableView_entry, "EntryBuyer");
}

EntryBuyer::~EntryBuyer()
{
	delete ui;
}
