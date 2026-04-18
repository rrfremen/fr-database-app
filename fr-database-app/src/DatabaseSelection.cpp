// built-in
#include <filesystem>
#include <fstream>
namespace fs = std::filesystem;
#include <iostream>

// external libraries
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>

// internal libraries
#include "../include/DatabaseSelection.h"
#include "ui_DatabaseSelection.h"
#include "../include/Logger.h"

DatabaseSelection::DatabaseSelection(QWidget* parent, const dict_like_string& cfg)
	: QWidget(parent)
	, ui(new Ui::DatabaseSelection)
	, config(cfg)
{
	ui->setupUi(this);
	initUi();
	setupConnections();
	ensureExampleDatabaseExists();
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

	if (!fs::exists(folderPathDB)) {
		fs::create_directories(folderPathDB);
	}

	ensureExampleDatabaseExists();

	// get available database in the designated folder
	for (const auto& entry : fs::directory_iterator(folderPathDB)) {
		if (entry.path().extension() == ".db") {
			listAvailDatabase.push_back(entry.path().stem().string());
		}
	}

	// populate ComboBox
	for (const auto& s : listAvailDatabase) {
		ui->comboBox_selectDatabase->addItem(QString::fromStdString(s));
	}

	ui->pushButton_loadDB->setEnabled(ui->comboBox_selectDatabase->count() > 0);

	int exampleIndex = ui->comboBox_selectDatabase->findText("Example");
	if (exampleIndex >= 0) {
		ui->comboBox_selectDatabase->setCurrentIndex(exampleIndex);
	}

	// if empty then fallbacks to default
	if (listAvailDatabase.empty()) {
		listAvailDatabase.push_back("Default");
		newDefault = true;
	}
}

void DatabaseSelection::onNewDatabase() {
	// TODO: refactor !
	QDialog dialog(this);
	dialog.setWindowTitle("Configuration for new Database");
	QString newTitle;
	QStringList clientParts;
	QStringList productParts;

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
			&dialog, [&] {
				newTitle = insertDBTitle->text().trimmed();
				clientParts = insertDBClientHeaders->text().split(",", Qt::SkipEmptyParts);
				productParts = insertDBProductHeaders->text().split(",", Qt::SkipEmptyParts);

				if (!userInputValid(QStringList(newTitle), config.at("validTitle")[0])) { return; };
				if (!userInputValid(clientParts, config.at("validHeaders")[0])) { return; };
				if (!userInputValid(productParts, config.at("validHeaders")[0])) { return; };

				dialog.accept();
			});
	connect(buttonCancel, &QPushButton::clicked,
			&dialog, &QDialog::reject);
	connect(buttonDefault, &QPushButton::clicked,
			&dialog, [=] {
				QStringList clientHeaders;
				QStringList productHeaders;
				for (const auto& s : config.at("definitionClient"))
					clientHeaders << QString::fromStdString(s);
				for (const auto& s : config.at("definitionProduct"))
					productHeaders << QString::fromStdString(s);
				insertDBClientHeaders->setText(clientHeaders.join(","));
				insertDBProductHeaders->setText(productHeaders.join(","));
			});

	if (dialog.exec() == QDialog::Accepted) {
		{
			std::vector<std::string> holder;
			for (const QString& p : clientParts)
				holder.push_back(p.trimmed().toStdString());
			holder.clear();
			for (const QString& p : productParts)
				holder.push_back(p.trimmed().toStdString());

			std::string newTitleString = newTitle.toStdString();
			newHeaders["newTitle"] = { newTitleString };
			newHeaders[newTitleString + "_Client"] = holder;
			newHeaders[newTitleString + "_Product"] = holder;
			//LOG(newHeaders["newTitle"]);
			//LOG(newHeaders[newTitle + "_Client"]);
			//LOG(newHeaders[newTitle + "_Product"]);
			createDatabase(newTitleString);
		}
	} else {
		LOG("Cancelled new Database");
	}
}

void DatabaseSelection::onLoadDatabase() {
	std::string currentTitle = ui->comboBox_selectDatabase->currentText().trimmed().toStdString();

	if (currentTitle.empty()) {
		LOG("No Database selected");
		return;
	}

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
		currentDB = nullptr;
		// re-populate ComboBox
		populateDatabaseComboBox();
	} else { LOG("Error creating Database"); }
}

void DatabaseSelection::createDatabaseTables(std::string newTitle) {
	// create client table
	auto& clientVector = newHeaders[newTitle + "_Client"];
	std::string client = "CREATE TABLE IF NOT EXISTS \"" + newTitle + "_Client\" (";
	client += "\"id\" INTEGER PRIMARY KEY, ";
	for (const std::string& value : clientVector) {
		std::string header = value;
		if (header.find("num:") == 0) { // allowing the headers name to have space, ex: Date of Birth 
			header.erase(0, 4);
			client += "\"" + header + "\" INTEGER, ";
		}
		else {
			client += "\"" + header + "\" TEXT, ";
		}
	}
	if (client.size() >= 2) {
		client.erase(client.size() - 2);
	}
	client += ");";
	
	// create product table
	auto& productVector = newHeaders[newTitle + "_Product"];
	std::string product = "CREATE TABLE IF NOT EXISTS \"" + newTitle + "_Product\" (";
	product += "\"id\" INTEGER PRIMARY KEY, ";
	for (const std::string& value : productVector) {
		std::string header = value;
		if (header.find("num:") == 0) { // allowing the headers name to have space, ex: Date of Buying
			header.erase(0, 4);
			product += "\"" + header + "\" INTEGER, ";
		}
		else {
			product += "\"" + header + "\" TEXT, ";
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

bool DatabaseSelection::userInputValid(QStringList inputs, std::string inputRules) {
	QStringList invalidHeaders;
	QRegularExpression validHeaders(
		QString::fromStdString(config.at("validHeaders")[0])
	);
	for (const QString& part : inputs) {
		QString p = part.trimmed();

		if (!validHeaders.match(p).hasMatch()) {
			invalidHeaders << p;
		}
	}

	if (!invalidHeaders.empty()) {
		QString message = "Invalid input:\n" + invalidHeaders.join("\n");
		QMessageBox::warning(this, "Warning", message);
		return false;
	}
	else {
		return true;
	}
}

// make sure the example database tables from json file exists
void DatabaseSelection::ensureExampleDatabaseExists() {
	if (!fs::exists(folderPathDB)) {
		fs::create_directories(folderPathDB);
	}

	std::string exampleDBPath = folderPathDB + "/example.db";

	if (fs::exists(exampleDBPath)) {
		return;
	}

	if (createExampleDatabaseFromJson("./assets/example_database.json")) {
		LOG("Example.db created successfully");
	}
	else {
		LOG("Failed to create Example.db from JSON");
	}
}

// create example database tables from json file
bool DatabaseSelection::createExampleDatabaseFromJson(const std::string& filePath) {
	std::ifstream file(filePath);
	if (!file.is_open()) {
		LOG("Could not open example_database.json: " + filePath);
		return false;
	}

	nlohmann::json root;
	file >> root;

	if (!root.contains("title") ||
		!root.contains("definitionClient") ||
		!root.contains("definitionProduct") ||
		!root.contains("contentClient") ||
		!root.contains("contentProduct")) {
		LOG("Invalid JSON format in example_database.json");
		return false;
	}

	std::string dbTitle = root["title"].get<std::string>();
	nlohmann::json definitionClient = root["definitionClient"];
	nlohmann::json definitionProduct = root["definitionProduct"];
	nlohmann::json contentClient = root["contentClient"];
	nlohmann::json contentProduct = root["contentProduct"];

	if (dbTitle.empty()) {
		LOG("JSON title is empty");
		return false;
	}

	std::string dbPath = folderPathDB + "/" + dbTitle + ".db";
	sqlite3* db = nullptr;

	if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
		LOG("Could not create or open example database");
		return false;
	}

	bool okClientTable = createTableFromDefinition(db, dbTitle + "_Client", definitionClient);
	bool okProductTable = createTableFromDefinition(db, dbTitle + "_Product", definitionProduct);

	bool okClientRows = false;
	bool okProductRows = false;

	if (okClientTable) {
		okClientRows = insertRowsFromDefinition(db, dbTitle + "_Client", definitionClient, contentClient);
	}
	if (okProductTable) {
		okProductRows = insertRowsFromDefinition(db, dbTitle + "_Product", definitionProduct, contentProduct);
	}

	sqlite3_close(db);

	if (okClientTable && okProductTable && okClientRows && okProductRows) {
		return true;
	}
	return false;
}

// create the tables and show tables
bool DatabaseSelection::createTableFromDefinition(sqlite3* db, const std::string& tableName, const nlohmann::json& definition) {
	if (!definition.is_array() || definition.empty()) {
		LOG("Invalid table definition for " + tableName);
		return false;
	}

	std::string createSql = "CREATE TABLE IF NOT EXISTS \"" + tableName + "\" (";
	createSql += "\"id\" INTEGER PRIMARY KEY, ";

	for (const auto& entry : definition) {
		if (!entry.is_string()) {
			continue;
		}

		std::string header = entry.get<std::string>();
		bool isNumber = false;

		if (header.find("num:") == 0) {
			header.erase(0, 4);
			isNumber = true;
		}

		createSql += "\"" + header + "\" ";
		createSql += isNumber ? "INTEGER, " : "TEXT, ";
	}

	if (createSql.size() >= 2) {
		createSql.erase(createSql.size() - 2);
	}

	createSql += ");";

	int msg = sqlite3_exec(db, createSql.c_str(), nullptr, nullptr, nullptr);
	if (msg != SQLITE_OK) {
		LOG("Error creating table: " + tableName);
		return false;
	}
	return true;
}

// insert the rows from example database tables 
bool DatabaseSelection::insertRowsFromDefinition(sqlite3* db, const std::string& tableName, const nlohmann::json& definition, const nlohmann::json& content) {
	if (!definition.is_array() || !content.is_array()) {
		LOG("Invalid insert definition for " + tableName);
		return false;
	}

	for (const auto& row : content) {
		if (!row.is_array()) {
			continue;
		}

		std::string insertSql = "INSERT INTO \"" + tableName + "\" (";
		std::string valuesPart = "VALUES (";

		size_t columnCount = definition.size();
		for (size_t i = 0; i < columnCount; ++i) {
			if (!definition[i].is_string()) {
				continue;
			}

			std::string header = definition[i].get<std::string>();
			bool isNumber = false;

			if (header.find("num:") == 0) {
				header.erase(0, 4);
				isNumber = true;
			}

			insertSql += "\"" + header + "\"";

			if (i < row.size()) {
				if (row[i].is_null()) {
					valuesPart += "NULL";
				}
				else if (isNumber) {
					if (row[i].is_number()) {
						valuesPart += row[i].dump();
					}
					else if (row[i].is_string()) {
						std::string numericValue = row[i].get<std::string>();
						valuesPart += numericValue.empty() ? "NULL" : numericValue;
					}
					else {
						valuesPart += "0";
					}
				}
				else {
					std::string textValue = row[i].is_string()
						? row[i].get<std::string>()
						: row[i].dump();
					valuesPart += "'" + escapeSqlValue(textValue) + "'";
				}
			}
			else {
				valuesPart += "NULL";
			}

			if (i != columnCount - 1) {
				insertSql += ", ";
				valuesPart += ", ";
			}
		}

		insertSql += ") ";
		valuesPart += ");";
		insertSql += valuesPart;

		int msg = sqlite3_exec(db, insertSql.c_str(), nullptr, nullptr, nullptr);
		if (msg != SQLITE_OK) {
			LOG("Error inserting row into table: " + tableName);
			return false;
		}
	}
	return true;
}

// protects sql query from breaking when the text contain ' (single quote)
std::string DatabaseSelection::escapeSqlValue(const std::string& value) {
	std::string escaped = value;
	size_t pos = 0;

	while ((pos = escaped.find('\'', pos)) != std::string::npos) {
		escaped.insert(pos, "'");
		pos += 2;
	}
	return escaped;
}
