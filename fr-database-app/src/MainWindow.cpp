// built-in
#include <iostream>
#include <fstream>

// external libraries
#include "./external/nlohmann-json.hpp"
#include "./external/sqlite3.h"

// internal libraries
#include "../include/MainWindow.h"
#include "../include/Logger.h"

// internal Ui libraries
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	ui->tabWidget_centralWidget->clear();

	setupConnections();
	getConfig();
	setupUiDatabaseSelection();
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::setupUiDatabaseSelection() {
	databaseSelection = new DatabaseSelection(this, config);
	ui->tabWidget_centralWidget->addTab(databaseSelection, "Database");

	// pass default headers from config to database selection
	connect(databaseSelection, &DatabaseSelection::signalLoadDB,
			this, &MainWindow::receiveLoadDatabase);
	connect(databaseSelection, &DatabaseSelection::signalUnloadDB,
			this, &MainWindow::receiveUnloadDatabase);
}

void MainWindow::setupConnections() {

}

void MainWindow::getConfig() {
	std::ifstream file("./assets/config.json");
	nlohmann::json config_json;
	file >> config_json;

	// get default headers
	for (auto& [key, value] : config_json["defaultDatabase"].items()) {
		if (key.find("definition") != std::string::npos) {
			config[key] = value.get<std::vector<std::string>>();
		}
	}
	// entry rules
	for (auto& [key, value] : config_json["entryRules"].items()) {
		config[key] = value.get<std::vector<std::string>>();
	}

}

void MainWindow::receiveLoadDatabase(sqlite3* pointerDB, std::string dbTitle) {
	if (pointerDB != nullptr) {
		currentDB = pointerDB;
		currentDBTitle = dbTitle;
		setupDatabase();
	}
}

void MainWindow::receiveUnloadDatabase() {
	int index;
	for (auto& [key, widget] : widgets) {
		index = ui->tabWidget_centralWidget->indexOf(widget);
		ui->tabWidget_centralWidget->removeTab(index);
		widget->deleteLater();
		LOG("removed " << key);
	}
}

void MainWindow::setupDatabase() {
	widgets["mainClient"] = new EntryForm(this, currentDBTitle + "_Client", currentDB);
	widgets["mainProduct"] = new EntryForm(this, currentDBTitle + "_Product", currentDB);

	ui->tabWidget_centralWidget->addTab(widgets["mainClient"], "Client");
	ui->tabWidget_centralWidget->addTab(widgets["mainProduct"], "Product");
}
