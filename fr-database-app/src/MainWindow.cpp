// built-in
#include <iostream>
#include <fstream>

// external libraries
#include "./external/nlohmann-json.hpp"

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

	setupSignals();
	getConfig();
	setupUiDatabaseSelection();
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