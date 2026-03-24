#pragma once

// built-in
#include <string>

// external library
#include <QWidget>

// internal library
#include "EntryBaseClass.h"

namespace Ui {
	class EntryForm;
}

class EntryForm : public EntryBaseClass 
{
	Q_OBJECT

public:
	EntryForm(QWidget* parent = nullptr);
	~EntryForm();

private:
	Ui::EntryForm* ui;
};
