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
	updateHeaders();
	reloadTable();
}

EntryForm::~EntryForm()
{
	delete ui;
}

void EntryForm::setupConnections() {
	connect(ui->pushButton_add, &QPushButton::clicked,
			this, &EntryForm::addEntry);
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

	for (const QString& header : currentHeaders)
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

		sqlite3_exec(currentDB, sql.c_str(), nullptr, nullptr, nullptr);

		LOG("Successfully added new information");

		reloadTable();
	}
	else {
		LOG("Canceled adding new information");
	}
}

void EntryForm::reloadTable()
{
	QStandardItemModel* model = new QStandardItemModel(ui->tableView_entry);
	model->setHorizontalHeaderLabels(currentHeaders);

	std::string selectSQL = "SELECT * FROM " + tableName + ";";
	sqlite3_stmt* stmt = nullptr; // setting and holding the query from the table

	if (sqlite3_prepare_v2(currentDB, selectSQL.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
		return;

	int row = 0;
	int colCount = currentHeaders.size();

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

	ui->tableView_entry->setModel(model);
	ui->tableView_entry->horizontalHeader()->setVisible(true);
	ui->tableView_entry->verticalHeader()->setVisible(false);
}
