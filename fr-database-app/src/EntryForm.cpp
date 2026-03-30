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
#include "../include/EntryForm.h"
#include "ui_EntryForm.h"
#include "../include/Logger.h"

EntryForm::EntryForm(QWidget* parent, std::string tabName, sqlite3* database)
	: QWidget(parent)
	, ui(new Ui::EntryForm)
{
	ui->setupUi(this);
	tableName = tabName;
	currentDB = database;

	setupConnections();
	initUi();
	updateHeaders();
	loadSearchHeaders();
	reloadTable();
}

EntryForm::~EntryForm()
{
	delete ui;
}

void EntryForm::initUi() {
	ui->tableView_entry->horizontalHeader()->setVisible(true);
	ui->tableView_entry->verticalHeader()->setVisible(false);
	ui->tableView_entry->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->tableView_entry->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->tableView_entry->setEditTriggers(QAbstractItemView::NoEditTriggers); // no direct editing from mouse or keyboard
}

void EntryForm::setupConnections() {
	connect(ui->pushButton_add, &QPushButton::clicked,
			this, &EntryForm::addEntry);
	connect(ui->pushButton_remove, &QPushButton::clicked,
			this, &EntryForm::removeEntry);
	connect(ui->pushButton_edit, &QPushButton::clicked,
			this, &EntryForm::editEntry);
	connect(ui->pushButton_search, &QPushButton::clicked,
			this, &EntryForm::searchEntry);
	connect(ui->lineEdit_searchBar, &QLineEdit::textChanged,
			this, &EntryForm::onSearchTextChanged);
}

void EntryForm::updateHeaders() {
	currentHeaders.clear();
	LOG(tableName);
	sqlite3_stmt* stmt;
	std::string selectSQL = "SELECT * FROM " + tableName + " LIMIT 1;";
	sqlite3_prepare_v2(currentDB, selectSQL.c_str(), -1, &stmt, nullptr);
	int col_count = sqlite3_column_count(stmt);

	for (int i = 0; i < col_count; i++) {
		currentHeaders << QString::fromUtf8(sqlite3_column_name(stmt, i));
	}
	sqlite3_finalize(stmt);
}

void EntryForm::addEntry()
{
	// headers
	if (currentHeaders.isEmpty())
		return;

	// pop-up window
	QDialog dialog(this);
	dialog.setWindowTitle("Add Entry");

	QVBoxLayout* mainLayout = new QVBoxLayout(&dialog);
	QList<QLineEdit*> inserts;
	QStringList insertHeaders;

	for (const QString& header : currentHeaders)
	{
		if (header.compare("id", Qt::CaseInsensitive) == 0) // skip id column in pop-up window
			continue;

		QLabel* label = new QLabel(header, &dialog);
		QLineEdit* insert = new QLineEdit(&dialog);

		mainLayout->addWidget(label);
		mainLayout->addWidget(insert);

		inserts.append(insert);
		insertHeaders.append(header);
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
		QStringList quotedHeaders; // for the quoted headers, where headers contains space in between ex: "Date of Birth"

		for (int i = 0; i < inserts.size(); ++i)
		{
			QString value = inserts[i]->text().trimmed();
			if (value.isEmpty())
				return;

			value.replace("'", "''");
			values << "'" + value + "'";
			quotedHeaders << "\"" + insertHeaders[i] + "\"";
		}

		std::string sql = "INSERT INTO " + tableName + " (";
		for (int i = 0; i < quotedHeaders.size(); ++i)
		{
			sql += quotedHeaders[i].toStdString();
			if (i < quotedHeaders.size() - 1)
				sql += ", ";
		}
		sql += ") VALUES (";

		for (int i = 0; i < values.size(); ++i)
		{
			sql += values[i].toStdString();
			if (i < values.size() - 1)
				sql += ", ";
		}
		sql += ");";

		int rc = sqlite3_exec(currentDB, sql.c_str(), nullptr, nullptr, nullptr);

		if (rc == SQLITE_OK) {
			LOG("Successfully added new information");
			reloadTable();
		} else {
			LOG("Insert failed: " << sqlite3_errmsg(currentDB));
		}
	}
	else {
		LOG("Canceled adding new information");
	}
}

void EntryForm::removeEntry()
{
	QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui->tableView_entry->model());
	if (!model)
		return;

	QModelIndex currentIndex = ui->tableView_entry->currentIndex();
	if (!currentIndex.isValid())
	{
		LOG("No row selected");
		return;
	}

	int row = currentIndex.row();

	if (currentHeaders.isEmpty())
		return;

	QMessageBox::StandardButton message;
	message = QMessageBox::question(this, "Remove Entry", "Are you sure you want to delete this row?", QMessageBox::Yes | QMessageBox::No);

	if (message == QMessageBox::No)
	{

		LOG("Cancel removing a row");
		return;
	}

	std::string sql = "DELETE FROM " + tableName + " WHERE ";

	for (int col = 0; col < currentHeaders.size(); ++col)
	{
		QStandardItem* item = model->item(row, col);
		QString value = item ? item->text() : "";

		value.replace("'", "''");

		std::string columnName = currentHeaders[col].toStdString();
		std::string columnValue = value.toStdString();

		sql += "\"" + columnName + "\" = '" + columnValue + "'";

		if (col < currentHeaders.size() - 1)
			sql += " AND ";
	}

	sql += ";";

	int rc = sqlite3_exec(currentDB, sql.c_str(), nullptr, nullptr, nullptr); // delete the row from the database

	if (rc == SQLITE_OK) // delete the row from the tableView 
	{
		reloadTable();
		ui->tableView_entry->clearSelection(); // removes higlights from previous row
		ui->tableView_entry->setCurrentIndex(QModelIndex()); // removes cursor focus
		LOG("Successfully removed a row");
	}
	else
	{
		LOG("Remove failed: " << sqlite3_errmsg(currentDB));
	}
}

// load table from query 
void EntryForm::loadTableFromQuery(const std::string& sql)
{
	QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui->tableView_entry->model());
	if (!model) {
		model = new QStandardItemModel(ui->tableView_entry);
		ui->tableView_entry->setModel(model);
	}
	model->clear();
	model->setHorizontalHeaderLabels(currentHeaders);

	sqlite3_stmt* stmt = nullptr; // setting and holding the query from the table

	if (sqlite3_prepare_v2(currentDB, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
		LOG("Query failed: " << sqlite3_errmsg(currentDB));
		return;
	}

	int row = 0;

	while (sqlite3_step(stmt) == SQLITE_ROW)
	{
		int colCount = sqlite3_column_count(stmt);

		for (int col = 0; col < colCount; ++col)
		{
			const unsigned char* text = sqlite3_column_text(stmt, col);
			QString value = text ? QString(reinterpret_cast<const char*>(text)) : "";

			model->setItem(row, col, new QStandardItem(value));
		}
		row++;
	}
	sqlite3_finalize(stmt);
	ui->tableView_entry->resizeColumnsToContents(); // allowing the table to adjust to the contents of the table
}

void EntryForm::reloadTable()
{
	std::string selectSQL = "SELECT * FROM " + tableName + ";";
	loadTableFromQuery(selectSQL);
}

void EntryForm::editEntry()
{
	QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui->tableView_entry->model());
	if (!model)
		return;

	QModelIndex currentIndex = ui->tableView_entry->currentIndex();
	if (!currentIndex.isValid())
	{
		LOG("No row selected");
		return;
	}

	int row = currentIndex.row();

	QStandardItem* idItem = model->item(row, 0);
	if (!idItem)
	{
		LOG("No id found for selected row");
		return;
	}

	QString idValue = idItem->text();

	if (currentHeaders.isEmpty())
		return;

	QDialog dialog(this);
	dialog.setWindowTitle("Edit Entry");

	QVBoxLayout* mainLayout = new QVBoxLayout(&dialog);
	QList<QLineEdit*> edits;
	QStringList editableHeaders;

	for (int col = 0; col < currentHeaders.size(); ++col) 
	{
		QString header = currentHeaders[col];

		if (header.compare("id", Qt::CaseInsensitive) == 0) 
			continue;

		QStandardItem* item = model->item(row, col);
		QString currentValue = item ? item->text() : "";

		QLabel* label = new QLabel(header, &dialog);
		QLineEdit* edit = new QLineEdit(&dialog);
		edit->setText(currentValue);

		mainLayout->addWidget(label);
		mainLayout->addWidget(edit);

		edits.append(edit);
		editableHeaders.append(header);
	}

	QPushButton* saveButton = new QPushButton("Save", &dialog);
	QPushButton* cancelButton = new QPushButton("Cancel", &dialog);

	QHBoxLayout* buttonLayout = new QHBoxLayout();
	buttonLayout->addWidget(saveButton);
	buttonLayout->addWidget(cancelButton);
	mainLayout->addLayout(buttonLayout);

	connect(saveButton, &QPushButton::clicked, &dialog, &QDialog::accept);
	connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

	if (dialog.exec() == QDialog::Accepted)
	{
		QStringList newValues;

		for (QLineEdit* edit : edits)
		{
			QString value = edit->text().trimmed();
			if (value.isEmpty())
				return;
			
			value.replace("'", "''");
			newValues << value;
		}

		std::string sql = "UPDATE " + tableName + " SET";

		for (int i = 0; i < editableHeaders.size(); ++i)
		{
			std::string columnName = editableHeaders[i].toStdString();
			std::string columnValue = newValues[i].toStdString();

			sql += "\"" + columnName + "\" = '" + columnValue + "'";

			if (i < editableHeaders.size() - 1)
				sql += ", ";
		}

		sql += " WHERE id = " + idValue.toStdString() + ";";

		int rc = sqlite3_exec(currentDB, sql.c_str(), nullptr, nullptr, nullptr);
		if (rc == SQLITE_OK)
		{
			LOG("Sucessfully edited a row");
			reloadTable();
			ui->tableView_entry->clearSelection();
			ui->tableView_entry->setCurrentIndex(QModelIndex());
		}
		else {
			LOG("Edit failed: " << sqlite3_errmsg(currentDB));
		}
	}
	else {
		LOG("Cancelled editing a row");
	}
}

// search combo box
void EntryForm::loadSearchHeaders()
{
	ui->comboBox_searchColumn->clear();

	for (const QString& header : currentHeaders)
	{
		ui->comboBox_searchColumn->addItem(header);
	}
}

// execute search button 
void EntryForm::searchEntry()
{
	QString selectedHeader = ui->comboBox_searchColumn->currentText();
	QString searchText = ui->lineEdit_searchBar->text().trimmed();

	if (selectedHeader.isEmpty())
	{
		LOG("No header selected for search");
		return;
	}

	if (searchText.isEmpty())
	{
		reloadTable(); 
		return;
	}
	
	searchText.replace("'", "''");

	std::string selectSQL = "SELECT * FROM " + tableName +
		" WHERE \"" + selectedHeader.toStdString() +
		"\" LIKE '%" + searchText.toStdString() + "%';";

	loadTableFromQuery(selectSQL);
}

// live searching while typing and auto reloading full table when the search bar is cleared
void EntryForm::onSearchTextChanged(const QString& text)
{
	if (text.trimmed().isEmpty())
	{
		reloadTable(); // reset table and show everything again
	}
	else {
		searchEntry(); // search the information 
	}
}



