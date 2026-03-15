// built-in
#include <iostream>
#include <fstream>

// external libraries
#include "./external/nlohmann-json.hpp"
#include <QStandardItemModel>

// internal libraries
#include "../include/EntryBaseClass.h"
#include "../include/Logger.h"


EntryBaseClass::EntryBaseClass(QWidget* parent)
	: QWidget(parent) {}

void EntryBaseClass::setupUiLocal(QTableView* table, const std::string& text) {
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
}
