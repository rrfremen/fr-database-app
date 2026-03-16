// built-in
#include <iostream>
#include <fstream>

// external libraries
#include "../external/sqlite3.h"

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
	createDatabase();
	connectionMessage();
	createTable();
	buyerMessage();
	propertiesMessage();
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

// connect to database
void MainWindow::createDatabase()
{ 
	rc = sqlite3_open("database.db", &db);

}

// show the connection to database on console window
void MainWindow::connectionMessage()
{
	if (rc) {
		std::cout << "Cannot open database\n";
	}
	else {
		std::cout << "Database open sucessfully\n";
	}
}

// create table for Buyer and Properties
void MainWindow::createTable()
{
	 buyer =
		"CREATE TABLE IF NOT EXISTS buyer ("
		"name TEXT NOT NULL,"
		"contact_details INTEGER,"
		"address TEXT,"
		"budget INTEGER,"
		"keywords TEXT);";

	properties =
		"CREATE TABLE IF NOT EXISTS buyer ("
		"id INTEGER NOT NULL,"
		"date TEXT,"
		"address TEXT,"
		"price INTEGER,"
		"rooms INTEGER,"
		"area TEXT,"
		"keywords TEXT);";
}

void MainWindow::buyerMessage()
{
	errMsg = sqlite3_exec(db, buyer, nullptr, nullptr, nullptr);
	if (errMsg == SQLITE_OK)
	{
		std::cout << "Table Buyer was created!\n";
	}
	else {
		std::cout << "Error creating table Buyer!\n";
	}
}

void MainWindow::propertiesMessage()
{
	errMsg = sqlite3_exec(db, properties, nullptr, nullptr, nullptr);
	if (errMsg == SQLITE_OK)
	{
		std::cout << "Table Properties was created!\n";
	}
	else {
		std::cout << "Error creating table Properties!\n";
	}
}