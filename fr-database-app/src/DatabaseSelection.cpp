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
#include <QFileDialog>
#include <QComboBox>
#include <QTableWidget>
#include <QTableWidgetItem>

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
	connect(ui->pushButton_importCSV, &QPushButton::clicked,
		this, &DatabaseSelection::onImportCsv);
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
				for (const auto& s : config.at("headersClient"))
					clientHeaders << QString::fromStdString(s);
				for (const auto& s : config.at("headersProduct"))
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

// get the headers and rows from the CSV file
std::pair<std::vector<std::string>, std::vector<std::vector<std::string>>> DatabaseSelection::parseCsv(const std::string& filePath) {
	std::vector<std::string> headers;
	std::vector<std::vector<std::string>> rows;

	std::ifstream file(filePath);
	if (!file.is_open()) {
		LOG("Could not open CSV file: " + filePath);
		return {};
	}

	auto parseLine = [](const std::string& line, char delimiter) {
		std::vector<std::string> fields;
		std::string field;
		bool inQuotes = false;

		for (size_t i = 0; i < line.size(); ++i) {
			char c = line[i];
			if (c == '"') {
				if (inQuotes && i + 1 < line.size() && line[i + 1] == '"') {
					field += '"';
					++i;
				}
				else {
					inQuotes = !inQuotes;
				}
			}
			else if (c == delimiter && !inQuotes) {
				fields.push_back(field);
				field.clear();
			}
			else {
				field += c;
			}
		}
		fields.push_back(field);
		return fields;
		};

	// detect delimiter from header line
	std::string headerLine;
	if (!std::getline(file, headerLine)) return {};
	if (!headerLine.empty() && headerLine.back() == '\r')
		headerLine.pop_back();

	char delimiter = ','; // default
	int commaCount = std::count(headerLine.begin(), headerLine.end(), ',');
	int semicolonCount = std::count(headerLine.begin(), headerLine.end(), ';');
	int tabCount = std::count(headerLine.begin(), headerLine.end(), '\t');

	if (semicolonCount > commaCount && semicolonCount >= tabCount)
		delimiter = ';';
	else if (tabCount > commaCount && tabCount >= semicolonCount)
		delimiter = '\t';

	LOG("CSV delimiter detected: " + std::string(1, delimiter));

	headers = parseLine(headerLine, delimiter);
	for (auto& h : headers)
		if (!h.empty() && h.back() == '\r') h.pop_back();

	std::string line;
	while (std::getline(file, line)) {
		if (line.empty() || line == "\r") continue;
		auto fields = parseLine(line, delimiter);
		for (auto& f : fields)
			if (!f.empty() && f.back() == '\r') f.pop_back();
		rows.push_back(fields);
	}
	return { 
		headers, rows 
	};
}

void DatabaseSelection::onImportCsv()
{
	// pick the CSV file
	QString filePath = QFileDialog::getOpenFileName(this, "Import CSV", QString(), "CSV Files (*.csv)");
	if (filePath.isEmpty()) return;

	auto [headers, rows] = parseCsv(filePath.toStdString());
	if (headers.empty()) {
		QMessageBox::warning(this, "Import Failed",
			"CSV file is empty or could not be read.");
		return;
	}

	// build new dialog for importing csv file 
	QDialog dialog(this);
	dialog.setWindowTitle("Import CSV");
	QVBoxLayout* layout = new QVBoxLayout(&dialog);

	// enter the DB name and show in combo box a combination from existing DBs + free-type for new ones
	layout->addWidget(new QLabel("Database name:", &dialog));
	QComboBox* editDbName = new QComboBox(&dialog);
	editDbName->setEditable(true);
	editDbName->setInsertPolicy(QComboBox::NoInsert); // only type in the name of the database
	editDbName->setPlaceholderText("Select existing or type new name");
	for (const auto& db : listAvailDatabase)
		editDbName->addItem(QString::fromStdString(db));
	editDbName->setCurrentText("");
	layout->addWidget(editDbName);

	// show status label updates when the user picks a DB name
	QLabel* statusLabel = new QLabel(&dialog);
	statusLabel->setWordWrap(true);
	layout->addWidget(statusLabel);

	// table type
	layout->addWidget(new QLabel("Import as:", &dialog));
	QComboBox* comboType = new QComboBox(&dialog);
	comboType->addItems({ "Client", "Product" });
	layout->addWidget(comboType);

	// check which tables already exist in a given .db file
	auto getExistingTables = [&](const std::string& dbName) -> std::vector<std::string> {
		std::string path = folderPathDB + "/" + dbName + ".db";
		if (!fs::exists(path)) return {};

		sqlite3* db = nullptr;
		if (sqlite3_open_v2(path.c_str(), &db, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK)
			return {};

		std::vector<std::string> tables;
		std::string sql = "SELECT name FROM sqlite_master WHERE type='table';";
		sqlite3_exec(db, sql.c_str(), [](void* data, int, char** argv, char**) {
			reinterpret_cast<std::vector<std::string>*>(data)->push_back(argv[0]);
			return 0;
			}, &tables, nullptr);

		sqlite3_close(db);
		return tables;
		};

	// refresh the status label + disable table type if already present
	auto refreshStatus = [&]() {
		QString dbName = editDbName->currentText().trimmed();
		if (dbName.isEmpty()) {
			statusLabel->setText("");
			return;
		}

		auto existing = getExistingTables(dbName.toStdString());
		bool hasClient = std::find(existing.begin(), existing.end(),
			dbName.toStdString() + "_Client") != existing.end();
		bool hasProduct = std::find(existing.begin(), existing.end(),
			dbName.toStdString() + "_Product") != existing.end();

		if (existing.empty()) {
			statusLabel->setText("New database will be created.");
			comboType->setEnabled(true);
		}
		else if (hasClient && hasProduct) {
			statusLabel->setText(
				"This database already has both tables. "
				"Importing will add rows to the selected table.");
			comboType->setEnabled(true);
		}
		else if (hasClient) {
			statusLabel->setText("Client table exists, now importing as Product.");
			comboType->setCurrentText("Product");
			comboType->setEnabled(false);   // lock to the missing one
		}
		else if (hasProduct) {
			statusLabel->setText("Product table exists, now importing as Client.");
			comboType->setCurrentText("Client");
			comboType->setEnabled(false);
		}
		else {
			statusLabel->setText("Existing database detected.");
			comboType->setEnabled(true);
		}
		};

	connect(editDbName, &QComboBox::currentTextChanged,
		&dialog, [&](const QString&) { refreshStatus(); });
	refreshStatus(); // run once on open

	// preview
	layout->addWidget(new QLabel(
		QString("Preview (%1 columns, %2 rows):")
		.arg(headers.size()).arg(rows.size()), &dialog));

	QTableWidget* preview = new QTableWidget(&dialog);
	preview->setEditTriggers(QAbstractItemView::NoEditTriggers);
	preview->setColumnCount(static_cast<int>(headers.size()));

	QStringList qHeaders;
	for (const auto& h : headers) qHeaders << QString::fromStdString(h);
	preview->setHorizontalHeaderLabels(qHeaders);

	int previewRows = std::min(static_cast<int>(rows.size()), 5);
	preview->setRowCount(previewRows);
	for (int r = 0; r < previewRows; ++r) {
		for (int c = 0; c < static_cast<int>(headers.size()); ++c) {
			QString val = (c < static_cast<int>(rows[r].size()))
				? QString::fromStdString(rows[r][c]) : "";
			preview->setItem(r, c, new QTableWidgetItem(val));
		}
	}
	//preview->resizeColumnsToContents();
	//preview->setMinimumHeight(160);
	layout->addWidget(preview);

	// buttons
	QHBoxLayout* btnLayout = new QHBoxLayout();
	QPushButton* btnImport = new QPushButton("Import", &dialog);
	QPushButton* btnCancel = new QPushButton("Cancel", &dialog);
	btnLayout->addWidget(btnImport);
	btnLayout->addWidget(btnCancel);
	layout->addLayout(btnLayout);

	connect(btnCancel, &QPushButton::clicked, &dialog, &QDialog::reject);
	connect(btnImport, &QPushButton::clicked, &dialog, [&] {
		QString dbName = editDbName->currentText().trimmed();
		if (!userInputValid(QStringList(dbName), config.at("validTitle")[0])) return;
		dialog.accept();
		});

	if (dialog.exec() != QDialog::Accepted) return;

	// import the database
	std::string dbTitle = editDbName->currentText().trimmed().toStdString();
	std::string tableType = comboType->currentText().toStdString();

	if (importCsvToDatabase(dbTitle, tableType, headers, rows)) {
		// check if the DB now has both tables and show information to user
		auto existing = getExistingTables(dbTitle);
		bool hasClient = std::find(existing.begin(), existing.end(),
			dbTitle + "_Client") != existing.end();
		bool hasProduct = std::find(existing.begin(), existing.end(),
			dbTitle + "_Product") != existing.end();

		QString msg = QString("'%1_%2' imported successfully.\n")
			.arg(QString::fromStdString(dbTitle))
			.arg(QString::fromStdString(tableType));

		if (hasClient && hasProduct)
			msg += "Database is complete and both tables present.";
		else if (hasClient)
			msg += "Still missing: Product table. Import another CSV to complete it.";
		else
			msg += "Still missing: Client table. Import another CSV to complete it.";

		QMessageBox::information(this, "Import Successful", msg);
		populateDatabaseComboBox();
	}
	else {
		QMessageBox::critical(this, "Import Failed",
			"Could not write the database. Check logs for details.");
	}
}

bool DatabaseSelection::importCsvToDatabase(const std::string& dbTitle, const std::string& tableType, const std::vector<std::string>& headers, const std::vector<std::vector<std::string>>& rows) {
	std::string dbPath = folderPathDB + "/" + dbTitle + ".db";
	sqlite3* db = nullptr;

	if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
		LOG("Could not open/create database: " + dbPath);
		return false;
	}

	std::string tableName = dbTitle + "_" + tableType;
	std::string createSql = "CREATE TABLE IF NOT EXISTS \"" + tableName + "\" (";
	createSql += "\"id\" INTEGER PRIMARY KEY, ";

	for (const auto& h : headers) {
		std::string col = h;
		bool isNum = (col.find("num:") == 0);
		if (isNum) col.erase(0, 4);
		createSql += "\"" + col + "\" " + (isNum ? "INTEGER" : "TEXT") + ", ";
	}
	createSql.erase(createSql.size() - 2);
	createSql += ");";

	if (sqlite3_exec(db, createSql.c_str(), nullptr, nullptr, nullptr) != SQLITE_OK) {
		LOG("Error creating table: " + tableName);
		sqlite3_close(db);
		return false;
	}

	// insert rows
	for (const auto& row : rows) {
		std::string insertSql = "INSERT INTO \"" + tableName + "\" (";
		std::string valuesPart = "VALUES (";

		for (size_t i = 0; i < headers.size(); ++i) {
			std::string col = headers[i];
			bool isNum = (col.find("num:") == 0);
			if (isNum) col.erase(0, 4);

			insertSql += "\"" + col + "\"";

			std::string val = (i < row.size()) ? row[i] : "";
			if (isNum) {
				valuesPart += val.empty() ? "NULL" : val;
			}
			else {
				valuesPart += "'" + escapeSqlValue(val) + "'";
			}

			if (i != headers.size() - 1) {
				insertSql += ", ";
				valuesPart += ", ";
			}
		}
		insertSql += ") " + valuesPart + ");";

		if (sqlite3_exec(db, insertSql.c_str(), nullptr, nullptr, nullptr) != SQLITE_OK) {
			LOG("Error inserting row into: " + tableName);
			sqlite3_close(db);
			return false;
		}
	}

	sqlite3_close(db);
	LOG("CSV file imported successfully into: " + tableName);
	return true;
}