// built-in
#include <iostream>
#include <fstream>

// external libraries

// internal libraries
#include "../include/EntryForm.h"
#include "ui_EntryForm.h"

EntryForm::EntryForm(QWidget* parent)
	: EntryBaseClass(parent)
	, ui(new Ui::EntryForm)
{
	ui->setupUi(this);
}

EntryForm::~EntryForm()
{
	delete ui;
}