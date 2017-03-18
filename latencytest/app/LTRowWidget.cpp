#include "LTRowWidget.h"

LTRowWidget::LTRowWidget(QWidget *parent /* = nullptr */)
    : QWidget(parent)
{
    setupUi(this);
}

LTRowWidget::~LTRowWidget(void)
{
    delete removeButton;
    delete enableCheckBox;
    delete midiOutComboBox;
    delete midiChannelSpinBox;
    delete asioDriverLabel;
    delete asioInputChannelComboBox;
    delete midiLatencyLabel;
    delete totalLatencyLabel;
}
