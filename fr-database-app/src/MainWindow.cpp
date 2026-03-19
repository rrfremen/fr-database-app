// built-in
#include <iostream>
#include <fstream>

// external libraries
#include "../external/sqlite3.h"

// internal libraries
#include "../include/MainWindow.h"
#include "ui_MainWindow.h"
#include "../include/Logger.h"
#include "../include/EntryBuyer.h"
#include "../include/EntryProperty.h"
#include "../include/EntryBaseClass.h"

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	// calling database functions 
	createDatabase();
	connectionMessage();
	createTable();
	buyerMessage();
	propertiesMessage();

	setupUiLocal();
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::setupUiLocal() {
	ui->tabWidget_centralWidget->clear();

	entryBuyer = new EntryBuyer(db,this);
	entryProperty = new EntryProperty(db, this);

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
	if (rc != SQLITE_OK) {
		LOG("Cannot open database");
	}
	else {
		LOG("Database opened sucessfully");
	}
}

// create table for Buyer and Properties
void MainWindow::createTable()
{
	 const char* buyer =
		"CREATE TABLE IF NOT EXISTS buyer ("
		"name TEXT NOT NULL,"
		"contact_details INTEGER,"
		"address TEXT,"
		"budget INTEGER,"
		"keywords TEXT);";

	const char* properties =
		"CREATE TABLE IF NOT EXISTS properties ("
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
		LOG("Table Buyer was created!");
	}
	else 
	{
		LOG("Error creating table Buyer!");
	}
}

void MainWindow::propertiesMessage()
{
	errMsg = sqlite3_exec(db, properties, nullptr, nullptr, nullptr);
	if (errMsg == SQLITE_OK)
	{
		LOG("Table Properties was created!");
	}
	else {
		LOG("Error creating table Properties!");
	}
}