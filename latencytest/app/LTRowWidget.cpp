#include "LTRowWidget.h"

LTRowWidget::LTRowWidget(QWidget *parent /* = nullptr */)
    : QWidget(parent)
{
    setupUi(this);
}

LTRowWidget::~LTRowWidget(void)
{
    delete enableCheckBox;
    delete midiOutLabel;
    delete midiChannelSpinBox;
    delete asioDriverLabel;
    delete asioInputChannelSpinBox;
    delete latencyLabel;
}
