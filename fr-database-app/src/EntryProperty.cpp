#include "../include/EntryProperty.h"
#include "ui_EntryProperty.h"

EntryProperty::EntryProperty(QWidget* parent)
	: QWidget(parent)
	, ui(new Ui::EntryProperty)
{
	ui->setupUi(this);
}

EntryProperty::~EntryProperty()
{
	delete ui;
}