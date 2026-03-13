#pragma once

#include <QWidget>

namespace Ui {
	class EntryProperty;
}

class EntryProperty : public QWidget
{
	Q_OBJECT;

public:
	EntryProperty(QWidget* parent = nullptr);
	~EntryProperty();

private:
	Ui::EntryProperty* ui;
};
