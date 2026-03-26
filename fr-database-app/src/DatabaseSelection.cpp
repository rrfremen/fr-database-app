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
#include "../include/Logger.h"


DatabaseSelection::DatabaseSelection(QWidget* parent)
	: QWidget(parent)
	, ui(new Ui::DatabaseSelection)
{
	ui->setupUi(this);
	initUi();
	setupConnections();
	populateDatabaseComboBox();
}

DatabaseSelection::~DatabaseSelection()
{
	delete ui;
}

void DatabaseSelection::initUi() {
	ui->pushButton_unloadDB->setEnabled(false);
}

void DatabaseSelection::setupConnections() {
	connect(ui->pushButton_loadDB, &QPushButton::clicked,
			this, &DatabaseSelection::onLoadDatabase);
	connect(ui->pushButton_unloadDB, &QPushButton::clicked,
			this, &DatabaseSelection::onUnloadDatabase);
	connect(ui->pushButton_newDB, &QPushButton::clicked,
			this, &DatabaseSelection::onNewDatabase);
}

void DatabaseSelection::populateDatabaseComboBox() {
	ui->comboBox_selectDatabase->clear();
	listAvailDatabase.clear();
	// get available database in the designated folder
	for (const auto& entry : fs::directory_iterator(folderPathDB)) {
		if (entry.path().extension() == ".db") {
			listAvailDatabase.push_back(entry.path().stem().string());
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

void DatabaseSelection::onNewDatabase() {
	// TODO: refactor !
	QDialog dialog(this);
	dialog.setWindowTitle("Configuration for new Database");

	QVBoxLayout* layoutMain = new QVBoxLayout(&dialog);

	QLabel* labelDBTitle = new QLabel("Name", &dialog);
	QLineEdit* insertDBTitle = new QLineEdit(&dialog);
	insertDBTitle->setPlaceholderText("Name of new database");
	layoutMain->addWidget(labelDBTitle);
	layoutMain->addWidget(insertDBTitle);

	QLabel* labelDBClientHeaders = new QLabel("Client Headers", &dialog);
	QLineEdit* insertDBClientHeaders = new QLineEdit(&dialog);
	insertDBClientHeaders->setPlaceholderText("Name, Age, num:Budget");
	layoutMain->addWidget(labelDBClientHeaders);
	layoutMain->addWidget(insertDBClientHeaders);

	QLabel* labelDBProductHeaders = new QLabel("Product Headers", &dialog);
	QLineEdit* insertDBProductHeaders = new QLineEdit(&dialog);
	insertDBProductHeaders->setPlaceholderText("Name, num:Price, Keywords");
	layoutMain->addWidget(labelDBProductHeaders);
	layoutMain->addWidget(insertDBProductHeaders);

	QPushButton* buttonCreate = new QPushButton("Create", &dialog);
	QPushButton* buttonCancel = new QPushButton("Cancel", &dialog);
	QPushButton* buttonDefault = new QPushButton("Default Headers", &dialog);

	QHBoxLayout* layoutButton = new QHBoxLayout();
	layoutButton->addWidget(buttonCreate);
	layoutButton->addWidget(buttonCancel);
	layoutButton->addWidget(buttonDefault);
	layoutMain->addLayout(layoutButton);

	connect(buttonCreate, &QPushButton::clicked,
			&dialog, &QDialog::accept);
	connect(buttonCancel, &QPushButton::clicked,
			&dialog, &QDialog::reject);
	connect(buttonDefault, &QPushButton::clicked,
			&dialog, [=] {
				QStringList clientHeaders;
				QStringList productHeaders;
				for (const auto& s : defaultHeaders["defaultHeadersClient"])
					clientHeaders << QString::fromStdString(s);
				for (const auto& s : defaultHeaders["defaultHeadersProduct"])
					productHeaders << QString::fromStdString(s);
				insertDBClientHeaders->setText(clientHeaders.join(","));
				insertDBProductHeaders->setText(productHeaders.join(","));
			});

	if (dialog.exec() == QDialog::Accepted) {
		{
			// TODO: check for forbidden words/characters and empty inputs not allowed
			std::string newTitle = insertDBTitle->text().trimmed().toStdString();
			newHeaders["newTitle"] = { newTitle };
			QStringList parts = insertDBClientHeaders->text().split(",", Qt::SkipEmptyParts);
			std::vector<std::string> holder;
			for (const QString& p : parts)
				holder.push_back(p.trimmed().toStdString());
			newHeaders[newTitle + "_Client"] = holder;
			holder.clear();
			parts = insertDBProductHeaders->text().split(",", Qt::SkipEmptyParts);
			for (const QString& p : parts)
				holder.push_back(p.trimmed().toStdString());
			newHeaders[newTitle + "_Product"] = holder;
			//LOG(newHeaders["newTitle"]);
			//LOG(newHeaders[newTitle + "_Client"]);
			//LOG(newHeaders[newTitle + "_Product"]);
			createDatabase(newTitle);
		}
	} else {
		LOG("Cancelled new Database");
	}
}

void DatabaseSelection::onLoadDatabase() {
	std::string currentTitle = ui->comboBox_selectDatabase->currentText().toStdString();
	if (openDatabase(currentTitle) == SQLITE_OK) { 
		emit signalLoadDB(currentDB, currentTitle); 
		ui->pushButton_loadDB->setEnabled(false);
		ui->pushButton_unloadDB->setEnabled(true);
		ui->comboBox_selectDatabase->setEnabled(false);
	} else { LOG("Error opening" + currentTitle); }
}

void DatabaseSelection::onUnloadDatabase() {
	emit signalUnloadDB();
	ui->pushButton_loadDB->setEnabled(true);
	ui->pushButton_unloadDB->setEnabled(false);
	ui->comboBox_selectDatabase->setEnabled(true);
}

int DatabaseSelection::openDatabase(std::string fileName) {
	std::string filePath = folderPathDB + "/" + fileName + ".db";
	LOG(filePath);
	int msg = sqlite3_open_v2(filePath.c_str(), &currentDB, SQLITE_OPEN_READWRITE, nullptr);
	return msg;
}

void DatabaseSelection::createDatabase(std::string newTitle) {
	std::string newPath = folderPathDB + "/" + newTitle + ".db";
	int msg = sqlite3_open(newPath.c_str(), &currentDB);
	if (msg == SQLITE_OK) {
		LOG("Database created successfully");
		createDatabaseTables(newTitle);
		sqlite3_close(currentDB);
		// re-populate ComboBox
		populateDatabaseComboBox();
	} else { LOG("Error creating Database"); }
}

void DatabaseSelection::createDatabaseTables(std::string newTitle) {
	// create client table
	auto& clientVector = newHeaders[newTitle + "_Client"];
	std::string client = "CREATE TABLE IF NOT EXISTS " + newTitle + "_Client (";
	client += "id INTEGER PRIMARY KEY, ";
	for (const std::string& value : clientVector) {
		std::string header = value;
		if (header.find("num:") != std::string::npos) {
			header.erase(0, 4);
			client += header + " INTEGER, ";
		}
		else {
			client += header + " TEXT, ";
		}
	}
	if (client.size() >= 2) {
		client.erase(client.size() - 2);
	}
	client += ");";
	
	// create product table
	auto& productVector = newHeaders[newTitle + "_Product"];
	std::string product = "CREATE TABLE IF NOT EXISTS " + newTitle + "_Product (";
	product += "id INTEGER PRIMARY KEY, ";
	for (const std::string& value : productVector) {
		std::string header = value;
		if (header.find("num:") != std::string::npos) {
			header.erase(0, 4);
			product += header + " INTEGER, ";
		}
		else {
			product += header + " TEXT, ";
		}
	}
	if (product.size() >= 2) {
		product.erase(product.size() - 2);
	}
	product += ");";
	int msg1 = sqlite3_exec(currentDB, client.c_str(), nullptr, nullptr, nullptr);
	int msg2 = sqlite3_exec(currentDB, product.c_str(), nullptr, nullptr, nullptr);
	if (msg1 == SQLITE_OK && msg2 == SQLITE_OK) { LOG("Tables successfully created"); } 
	else { LOG("Error creating tables"); }
}
