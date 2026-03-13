#pragma once

#include <QMainWindow>

#include "EntryBuyer.h"
#include "EntryProperty.h"

namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget* parent = nullptr);
	~MainWindow();

private:
	Ui::MainWindow* ui;
	EntryBuyer* entryBuyer;
	EntryProperty* entryProperty;

	// functions
	void setupUiLocal();
};
