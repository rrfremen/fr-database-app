// built-in
#include <iostream>
#include <fstream>

// external libraries
#include "./external/nlohmann-json.hpp"
#include "../external/sqlite3.h"

// internal libraries
#include "../include/MainWindow.h"
#include "../include/Logger.h"
#include "../include/EntryBaseClass.h"

// internal Ui libraries
#include "ui_MainWindow.h"


MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	ui->tabWidget_centralWidget->clear();

	setupSignals();
	getConfig();
	setupUiDatabaseSelection();

	// calling database functions 
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

void MainWindow::setupUiDatabaseSelection() {
	databaseSelection = new DatabaseSelection();
	ui->tabWidget_centralWidget->addTab(databaseSelection, "Database");
	connect(databaseSelection, &DatabaseSelection::signalLoadDB,
			this, &MainWindow::onLoadDatabase);
	connect(databaseSelection, &DatabaseSelection::signalUnloadDB,
			this, &MainWindow::onUnloadDatabase);
}

void MainWindow::setupSignals() {

}

void MainWindow::getConfig() {
	std::ifstream file("./assets/config.json");
	nlohmann::json config;
	file >> config;

	// get default headers
	for (auto& [key, value] : config["defaultDatabase"].items()) {
		if (key.find("Definition") != std::string::npos) {
			defaultHeaders[key] = value.get<std::vector<std::string>>();
		}
	}
}

void MainWindow::onLoadDatabase(std::string db_name) {
	LOG(db_name);
	if (db_name == "Default:new") {
		setupDefaultDatabase();
	}
}

void MainWindow::onUnloadDatabase() {
	int index;
	for (auto& [key, widget] : widgets) {
		index = ui->tabWidget_centralWidget->indexOf(widget);
		ui->tabWidget_centralWidget->removeTab(index);
		widget->deleteLater();
		LOG("removed " << key);
	}
}

void MainWindow::setupDefaultDatabase() {
	widgets["defaultClient"] = new EntryForm(this);
	widgets["defaultProduct"] = new EntryForm(this);

	ui->tabWidget_centralWidget->addTab(widgets["defaultClient"], "Client");
	ui->tabWidget_centralWidget->addTab(widgets["defaultProduct"], "Product");
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