#ifndef LTROWWIDGET_H
#define LTROWWIDGET_H

#include <QWidget>

#include "ui_LTRowWidgetUI.h"

namespace Ui {
class LTRowWidget;
}

class LTRowWidget : public QWidget, public Ui_LTRowWidget
{
    Q_OBJECT

public:
    LTRowWidget(QWidget *parent = nullptr);
    virtual ~LTRowWidget(void);
};

#endif // LTROWWIDGET_H
