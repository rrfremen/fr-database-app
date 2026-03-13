#include "../include/EntryBuyer.h"
#include "ui_EntryBuyer.h"

EntryBuyer::EntryBuyer(QWidget* parent)
	: QWidget(parent)
	, ui(new Ui::EntryBuyer)
{
	ui->setupUi(this);
}

EntryBuyer::~EntryBuyer()
{
	delete ui;
}