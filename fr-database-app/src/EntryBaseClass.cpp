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
#include <QMessageBox>
#include <QModelIndex>

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

	table->setSelectionBehavior(QAbstractItemView::SelectRows); // whole row selected
	table->setSelectionMode(QAbstractItemView::SingleSelection); // only one row selected
}

// setting the database
void EntryBaseClass::setDatabase(sqlite3* database) {
	db = database;
}

// add new informations
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

				value.replace("'", "'");
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

			int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr); // save into database

			if (rc == SQLITE_OK)
			{
				LOG("Successfully inserted new informations");
				reloadTable(view, tableName, headers);
			}
			else
			{
				LOG("Insert failed: " << sqlite3_errmsg(db));
			}
		}
		else 
		{
			LOG("Canceled adding new information");
		}
	});
}

// reload the database each time the application is opened without overwrite the existing model
void EntryBaseClass::reloadTable(QTableView* view, const std::string& tableName, const QStringList& headers)
{
	QStandardItemModel* model = qobject_cast<QStandardItemModel*>(view->model());
	if (!model)
	{
		model = new QStandardItemModel(view);
		view->setModel(model);
	}

	model->clear();
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

	view->horizontalHeader()->setVisible(true);
	view->verticalHeader()->setVisible(false);
}

// remove informations from the table and database
void EntryBaseClass::removeEntry(QPushButton* button, QTableView* view, const std::string& text, const std::string& tableName) 
{
	connect(button, &QPushButton::clicked, this, [=]() {

		QStandardItemModel* model = qobject_cast<QStandardItemModel*>(view->model());
		if (!model)
			return;

		QModelIndex currentIndex = view->currentIndex();
		if (!currentIndex.isValid())
		{
			LOG("No row selected");
			return;
		}

		int row = currentIndex.row();
		QStringList headers = getHeadersFromJson(text);

		if (headers.isEmpty())
			return;

		QMessageBox::StandardButton message;
		message = QMessageBox::question(this, "Remove Entry", "Are you sure you want to delete this row?", QMessageBox::Yes | QMessageBox::No);

		if (message == QMessageBox::No)
		{

			LOG("Cancel removing a row");
			return;
		}

		std::string sql = "DELETE FROM " + tableName + " WHERE ";

		for (int col = 0; col < headers.size(); ++col)
		{
			QStandardItem* item = model->item(row, col);
			QString value = item ? item->text() : "";

			value.replace("'", "''");

			std::string columnName = headers[col].toStdString();

			for (char& c : columnName)
			{
				c = std::tolower(static_cast<unsigned char>(c)); // change the column name from json to lowercase to match the database
			}

			for (char& c : columnName)
			{
				if (c == ' ')
					c = '_'; // replace space with _ to match the database
			}
			
			std::string columnValue = value.toStdString();

			sql += columnName + " = '" + columnValue + "'";

			if (col < headers.size() - 1)
				sql += " AND ";
		}

		sql += ";";

		int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr); // delete the row from the database

		if (rc == SQLITE_OK) // delete the row from the tableView 
		{
			reloadTable(view, tableName, headers);
			view->clearSelection(); // removes higlights from previous row
			view->setCurrentIndex(QModelIndex()); // removes cursor focus
			LOG("Successfully removed a row");
		}
		else
		{
			LOG("Remove failed: " << sqlite3_errmsg(db));
		}
	});
}