// built-in
#include <filesystem>
namespace fs = std::filesystem;
#include <iostream>

// external libraries
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>

// internal libraries
#include "../include/DatabaseSelection.h"
#include "ui_DatabaseSelection.h"


DatabaseSelection::DatabaseSelection(QWidget* parent)
	: QWidget(parent)
	, ui(new Ui::DatabaseSelection)
{
	ui->setupUi(this);
	setupUiSignals();
	populateDatabaseComboBox();
}

DatabaseSelection::~DatabaseSelection()
{
	delete ui;
}

void DatabaseSelection::setupUiSignals() {
	connect(ui->pushButton_loadDB, &QPushButton::clicked,
			this, &DatabaseSelection::onLoadDatabase);
	connect(ui->pushButton_unloadDB, &QPushButton::clicked,
			this, &DatabaseSelection::onUnloadDatabase);
}

void DatabaseSelection::populateDatabaseComboBox() {
	// get available database in the designated folder
	auto pathDBFolder = fs::current_path() / "localDatabase";
	for (const auto& entry : fs::directory_iterator(pathDBFolder)) {
		if (entry.path().extension() == ".db") {
			listAvailDatabase.push_back(entry.path().string());
		}
	}
	// if empty then fallbacks to default
	if (listAvailDatabase.empty()) {
		listAvailDatabase.push_back("Default");
		newDefault = true;
	}
	// populate ComboBox
	for (const auto& s : listAvailDatabase) {
		ui->comboBox_selectDatabase->addItem(QString::fromStdString(s));
	}
}

void DatabaseSelection::newDatabase() {
	
}

void DatabaseSelection::onLoadDatabase() {
	std::string currentText = ui->comboBox_selectDatabase->currentText().toStdString();
	if (currentText == "Default" && newDefault) {
		currentText += ":new";
		newDefault = false;
	}
	emit signalLoadDB(currentText);
}

void DatabaseSelection::onUnloadDatabase() {
	emit signalUnloadDB();
}
