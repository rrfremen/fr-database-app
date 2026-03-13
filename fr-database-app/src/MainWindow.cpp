#include "../include/MainWindow.h"
#include "ui_MainWindow.h"

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