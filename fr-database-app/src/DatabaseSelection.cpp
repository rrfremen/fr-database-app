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
	connect(ui->pushButton_newDB, &QPushButton::clicked,
			this, &DatabaseSelection::onNewDatabase);
}

void DatabaseSelection::populateDatabaseComboBox() {
	ui->comboBox_selectDatabase->clear();
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

void DatabaseSelection::onNewDatabase() {
	QDialog dialog(this);
	dialog.setWindowTitle("Configuration for new Database");

	QVBoxLayout* layoutMain = new QVBoxLayout(&dialog);

	QLabel* labelDBTitle = new QLabel("Name", &dialog);
	QLineEdit* insertDBTitle = new QLineEdit(&dialog);
	layoutMain->addWidget(labelDBTitle);
	layoutMain->addWidget(insertDBTitle);

	QLabel* labelDBClientHeaders = new QLabel("Client Headers", &dialog);
	QLineEdit* insertDBClientHeaders = new QLineEdit(&dialog);
	layoutMain->addWidget(labelDBClientHeaders);
	layoutMain->addWidget(insertDBClientHeaders);

	QLabel* labelDBProductHeaders = new QLabel("Product Headers", &dialog);
	QLineEdit* insertDBProductHeaders = new QLineEdit(&dialog);
	layoutMain->addWidget(labelDBProductHeaders);
	layoutMain->addWidget(insertDBProductHeaders);

	QPushButton* buttonCreate = new QPushButton("Create", &dialog);
	QPushButton* buttonCancel = new QPushButton("Cancel", &dialog);

	QHBoxLayout* layoutButton = new QHBoxLayout();
	layoutButton->addWidget(buttonCreate);
	layoutButton->addWidget(buttonCancel);
	layoutMain->addLayout(layoutButton);

	connect(buttonCreate, &QPushButton::clicked,
			&dialog, &QDialog::accept);
	connect(buttonCancel, &QPushButton::clicked,
			&dialog, &QDialog::reject);

	if (dialog.exec() == QDialog::Accepted) {
		{
			newHeaders["newTitle"] = { insertDBTitle->text().trimmed().toStdString() };
			QStringList parts = insertDBClientHeaders->text().split(",", Qt::SkipEmptyParts);
			std::vector<std::string> holder;
			for (const QString& p : parts)
				holder.push_back(p.trimmed().toStdString());
			newHeaders["headersClient"] = holder;
			holder.clear();
			parts = insertDBProductHeaders->text().split(",", Qt::SkipEmptyParts);
			for (const QString& p : parts)
				holder.push_back(p.trimmed().toStdString());
			newHeaders["headersProduct"] = holder;
			LOG(newHeaders["newTitle"]);
			LOG(newHeaders["headersClient"]);
			LOG(newHeaders["headersProduct"]);
			createDatabase();
		}
	} else {
		LOG("Cancelled new Database");
	}
}

void DatabaseSelection::onLoadDatabase() {
	// TODO: eventually sends database pointer instead
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

void DatabaseSelection::createDatabase() {
	// TODO: create DB here

	// re-populate ComboBox
	populateDatabaseComboBox();
}
