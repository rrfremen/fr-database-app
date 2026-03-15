#pragma once

#include <QWidget>
#include "EntryBaseClass.h"

namespace Ui {
	class EntryProperty;
}

class EntryProperty : public EntryBaseClass
{
	Q_OBJECT;

public:
	EntryProperty(QWidget* parent = nullptr);
	~EntryProperty();

private:
	Ui::EntryProperty* ui;
};
