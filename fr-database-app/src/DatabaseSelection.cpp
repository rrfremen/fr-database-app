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
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray> 
#include <QCoreApplication>
#include <QDir>

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
	std::string client = "CREATE TABLE IF NOT EXISTS \"" + newTitle + "_Client\" (";
	client += "\"id\" INTEGER PRIMARY KEY, ";
	for (const std::string& value : clientVector) {
		std::string header = value;
		if (header.find("num:") != std::string::npos) {
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
		if (header.find("num:") != std::string::npos) {
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

void DatabaseSelection::ensureExampleDatabaseExists()
{
	if (!fs::exists(folderPathDB)){
		fs::create_directories(folderPathDB);
	}

	std::string exampleDBPath = folderPathDB + "/example.db";

	if (fs::exists(exampleDBPath)) {
		return;
	}

	QString jsonPath = QDir(QCoreApplication::applicationDirPath())
		.filePath("assets/example_database.json");

	LOG("JSON path: " << jsonPath.toStdString());
	LOG("DB path: " << exampleDBPath);

	if (createExampleDatabaseFromJson("assets/example_database.json")) {
		LOG("example.db created successfully");
	}
	else {
		LOG("Failed to create example.db from JSON");
	}
}

bool DatabaseSelection::createExampleDatabaseFromJson(const QString& filePath)
{
	// check if the jsom file exist
	QFile file(filePath);
	if (!file.exists()) {
		LOG("JSON file does not exist: " << filePath.toStdString());
		return false;
	}

	// if the file object of the json file not exist, this code fails
	if (!file.open(QIODevice::ReadOnly)) {
		LOG("Could not open example_database.json: " << filePath.toStdString());
		return false;
	}

	// takes all contents from json file 
	QByteArray contents = file.readAll();
	file.close();

	// to convert the text into json and contain the parsed json
	QJsonParseError parseError;
	QJsonDocument doc = QJsonDocument::fromJson(contents, &parseError);

	// make sure whether json is valid
	if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
		LOG("Invalid JSON format in example_database.json");
		return false;
	}

	// get the tables array
	QJsonObject rootObj = doc.object();
	QJsonArray tablesArray = rootObj["tables"].toArray();

	if (tablesArray.isEmpty()) {
		LOG("No tables found in example_database.json");
		return false;
	}

	std::string dbPath = folderPathDB + "/example.db";
	sqlite3* db = nullptr;

	if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
		LOG("Could not create or open example.db");
		return false;
	}

	for (const QJsonValue& tableValue : tablesArray) {
		if (!tableValue.isObject()) {
			continue;
		}

		QJsonObject tableObj = tableValue.toObject();

		if (!createTableFromJson(db, tableObj)) {
			sqlite3_close(db);
			return false;
		}
		
		if (!insertRowsFromJson(db, tableObj)) {
			sqlite3_close(db);
			return false;
		}
	}

	sqlite3_close(db);
	return true;
}

// create tables from json 
bool DatabaseSelection::createTableFromJson(sqlite3* db, const QJsonObject& tableObj)
{
	QString tableName = tableObj["name"].toString().trimmed();
	QJsonArray columnsArray = tableObj["columns"].toArray();

	if (tableName.isEmpty() || columnsArray.isEmpty()) {
		LOG("Invalid table definition in JSON");
		return false;
	}

	std::string createSql = "CREATE TABLE IF NOT EXISTS \"" + tableName.toStdString() + "\" (";
	createSql += "\"id\" INTEGER PRIMARY KEY, ";

	for (const QJsonValue& colValue : columnsArray) {
		if (!colValue.isObject()) {
			continue;
		}

		QJsonObject colObj = colValue.toObject();
		QString colName = colObj["name"].toString().trimmed();
		QString colType = colObj["type"].toString().trimmed().toUpper();

		if (colName.isEmpty() || colType.isEmpty()) {
			continue;
		}

		createSql += "\"" + colName.toStdString() + "\" " + colType.toStdString() + ", ";
	}

	// remove the last comma
	if (createSql.size() >= 2) {
		createSql.erase(createSql.size() - 2);
	}

	createSql += ");";

	int msg = sqlite3_exec(db, createSql.c_str(), nullptr, nullptr, nullptr);
	if (msg != SQLITE_OK) {
		LOG("Error creating table: " << tableName.toStdString());
		return false;
	}

	return true;
}

// take tables from json and show in UI
bool DatabaseSelection::insertRowsFromJson(sqlite3* db, const QJsonObject& tableObj)
{
	QString tableName = tableObj["name"].toString().trimmed();
	QJsonArray columnsArray = tableObj["columns"].toArray();
	QJsonArray rowsArray = tableObj["rows"].toArray();

	if (tableName.isEmpty() || columnsArray.isEmpty()) {
		LOG("Invalid insert definition in JSON");
		return false;
	}

	for (const QJsonValue& rowValue : rowsArray) {
		if (!rowValue.isObject()) {
			continue;
		}

		QJsonObject rowObj = rowValue.toObject();

		std::string insertSql = "INSERT INTO \"" + tableName.toStdString() + "\" (";
		std::string valuesPart = "VALUES (";

		for (int i = 0; i < columnsArray.size(); ++i) {
			QJsonObject colObj = columnsArray[i].toObject();
			QString colName = colObj["name"].toString().trimmed();
			QString colType = colObj["type"].toString().trimmed().toUpper();

			insertSql += "\"" + colName.toStdString() + "\"";

			QJsonValue cellValue = rowObj.value(colName);

			if (colType == "INTEGER") {
				valuesPart += std::to_string(cellValue.toInt());
			}
			else {
				std::string textValue = cellValue.toString().toStdString();
				valuesPart += "'" + escapeSqlValue(textValue) + "'";
			}

			if (i != columnsArray.size() - 1) {
				insertSql += ", ";
				valuesPart += ", ";
			}
		}

		insertSql += ") ";
		valuesPart += ");";
		insertSql += valuesPart;

		int msg = sqlite3_exec(db, insertSql.c_str(), nullptr, nullptr, nullptr);
		if (msg != SQLITE_OK) {
			LOG("Error inserting row into table: " << tableName.toStdString());
			return false;
		}
	}

	return true;
}

// protects sql query from breaking when the text contain ' (single quote)
std::string DatabaseSelection::escapeSqlValue(const std::string& value)
{
	std::string escaped = value;
	size_t pos = 0;

	// find the single quote
	while ((pos = escaped.find('\'', pos)) != std::string::npos) {
		escaped.insert(pos, "'");
		pos += 2;
	}

	return escaped;
}