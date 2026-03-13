#pragma once

#include <QWidget>

namespace Ui {
	class EntryBuyer;
}

class EntryBuyer : public QWidget
{
	Q_OBJECT

public:
	EntryBuyer(QWidget* parent = nullptr);
	~EntryBuyer();

private:
	Ui::EntryBuyer* ui;
};
