#pragma once

#include <QWidget>
#include "EntryBaseClass.h"

namespace Ui {
	class EntryBuyer;
}

class EntryBuyer : public EntryBaseClass
{
	Q_OBJECT

public:
	EntryBuyer(QWidget* parent = nullptr);
	~EntryBuyer();

private:
	Ui::EntryBuyer* ui;
};
