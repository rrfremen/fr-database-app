// built-in
#include <iostream>
#include <fstream>

// external libraries

// internal libraries
#include "../include/EntryProperty.h"
#include "ui_EntryProperty.h"

EntryProperty::EntryProperty(QWidget* parent)
	: EntryBaseClass(parent)
	, ui(new Ui::EntryProperty)
{
	ui->setupUi(this);
	setupUiLocal(ui->tableView_entry, "EntryProperty");
}

EntryProperty::~EntryProperty()
{
	delete ui;
}