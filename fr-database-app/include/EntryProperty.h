#pragma once

// external libraries
#include "../external/sqlite3.h"

// internals libraries
#include <QWidget>
#include "EntryBaseClass.h"

namespace Ui {
	class EntryProperty;
}

class EntryProperty : public EntryBaseClass
{
	Q_OBJECT;

public:
	EntryProperty(sqlite3* database, QWidget* parent = nullptr);
	~EntryProperty();

private:
	Ui::EntryProperty* ui;
};
