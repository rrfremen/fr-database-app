// built-in
#include <iostream>
#include <fstream>

// external libraries

// internal libraries
#include "../include/MainWindow.h"
#include "ui_MainWindow.h"
#include "../include/Logger.h"

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	setupUiLocal();
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::setupUiLocal() {
	ui->tabWidget_centralWidget->clear();

	entryBuyer = new EntryBuyer();
	entryProperty = new EntryProperty();

	ui->tabWidget_centralWidget->addTab(entryBuyer, "Buyer");
	ui->tabWidget_centralWidget->addTab(entryProperty, "Properties");
}