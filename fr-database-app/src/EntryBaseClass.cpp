// built-in
#include <iostream>
#include <fstream>

// external libraries
#include "./external/nlohmann-json.hpp"
#include <QStandardItemModel>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>

// internal libraries
#include "../include/EntryBaseClass.h"
#include "../include/Logger.h"

EntryBaseClass::EntryBaseClass(QWidget* parent)
	: QWidget(parent) {}

// getting the headers from json
QStringList EntryBaseClass::getHeadersFromJson(const std::string& text) {
	std::ifstream file("./assets/config.json");
	nlohmann::json j;
	file >> j;

	// retrieve headers
	std::vector<std::string> headers = j["listCols"][text];
	QStringList qheaders;
	for (const auto& h : headers)
		qheaders << QString::fromStdString(h);

	return qheaders;
}

void EntryBaseClass::setupUiLocal(QTableView* table, const std::string& text) {
	QStringList headers = getHeadersFromJson(text);

	// set placeholder model
	QStandardItemModel* placeholderModel = new QStandardItemModel();
	placeholderModel->setHorizontalHeaderLabels(headers);
	table->setModel(placeholderModel);
	LOG("Placeholder model for " << text << " created");

	table->horizontalHeader()->setVisible(true);
	table->verticalHeader()->setVisible(false);
}

/*void EntryBaseClass::setupUiLocal(QTableView* table, const std::string& text) {
	std::ifstream file("./assets/config.json");
	nlohmann::json j;
	file >> j;

	// retrieve headers
	std::vector<std::string> headers = j["listCols"][text];
	QStringList qheaders;
	for (const auto& h : headers)
		qheaders << QString::fromStdString(h);

	// set placeholder model
	QStandardItemModel* placeholderModel = new QStandardItemModel();
	placeholderModel->setHorizontalHeaderLabels(qheaders);
	table->setModel(placeholderModel);
	LOG("Placeholder model for " << text << " created");

	table->horizontalHeader()->setVisible(true);
	table->verticalHeader()->setVisible(false);
}*/

// setting the database
void EntryBaseClass::setDatabase(sqlite3* database) {
	db = database;
}

void EntryBaseClass::addEntry(QPushButton* button, QTableView* view, const std::string& text, const std::string& tableName)
{
	connect(button, &QPushButton::clicked, this, [=]() { // show pop-up window, get input and insert into database

		// headers
		QStringList headers = getHeadersFromJson(text);
		if (headers.isEmpty())
			return;

		// pop-up window
		QDialog dialog(this);
		dialog.setWindowTitle("Add Entry");

		QVBoxLayout* mainLayout = new QVBoxLayout(&dialog);
		QList<QLineEdit*> inserts;

		for (const QString& header : headers)
		{
			QLabel* label = new QLabel(header, &dialog);
			QLineEdit* insert = new QLineEdit(&dialog);

			mainLayout->addWidget(label);
			mainLayout->addWidget(insert);

			inserts.append(insert);
		}
		
		// done and cancel button inside the pop-up window
		QPushButton* doneButton = new QPushButton("Done", &dialog);
		QPushButton* cancelButton = new QPushButton("Cancel", &dialog);

		QHBoxLayout* buttonLayout = new QHBoxLayout();
		buttonLayout->addWidget(doneButton);
		buttonLayout->addWidget(cancelButton);

		mainLayout->addLayout(buttonLayout);

		connect(doneButton, &QPushButton::clicked, &dialog, &QDialog::accept);
		connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

		if (dialog.exec() == QDialog::Accepted)
		{
			QStringList values;

			for (QLineEdit* insert : inserts)
			{
				QString value = insert->text().trimmed();
				if (value.isEmpty())
					return;

				value.replace("'", "''");
				values << "'" + value + "'";
			}

			std::string sql = "INSERT INTO " + tableName + " VALUES (";
			for (int i = 0; i < values.size(); ++i)
			{
				sql += values[i].toStdString();
				if (i < values.size() - 1)
					sql += ", ";
			}
			sql += ");";

			sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr);

			LOG("Successfully added new information");

			reloadTable(view, tableName, headers);
		}
		else {
			LOG("Canceled adding new information");
		}
	});
}

void EntryBaseClass::reloadTable(QTableView* view, const std::string& tableName, const QStringList& headers)
{
	QStandardItemModel* model = new QStandardItemModel(view);
	model->setHorizontalHeaderLabels(headers);

	std::string selectSQL = "SELECT * FROM " + tableName + ";";
	sqlite3_stmt* stmt = nullptr; // setting and holding the query from the table

	if (sqlite3_prepare_v2(db, selectSQL.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
		return;

	int row = 0;
	int colCount = headers.size();

	while (sqlite3_step(stmt) == SQLITE_ROW)
	{
		for (int col = 0; col < colCount; ++col)
		{
			const unsigned char* text = sqlite3_column_text(stmt, col);
			QString value = text ? QString(reinterpret_cast<const char*>(text)) : "";

			model->setItem(row, col, new QStandardItem(value));
		}
		row++;
	}

	sqlite3_finalize(stmt);

	view->setModel(model);
	view->horizontalHeader()->setVisible(true);
	view->verticalHeader()->setVisible(false);
}