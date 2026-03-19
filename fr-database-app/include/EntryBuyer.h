#pragma once

// external libraries
#include "../external/sqlite3.h"

// internal libraries
#include <QWidget>
#include "EntryBaseClass.h"

namespace Ui {
	class EntryBuyer;
}

class EntryBuyer : public EntryBaseClass
{
	Q_OBJECT

public:
	EntryBuyer(sqlite3* database, QWidget* parent = nullptr);
	~EntryBuyer();

private:
	Ui::EntryBuyer* ui;
};
