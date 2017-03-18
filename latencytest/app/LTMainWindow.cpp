#include <QApplication>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSpinBox>
#include <QSlider>
#include <QPushButton>
#include <QMessageBox>
#include <QSizePolicy>
#include <QTimer>
#include <QGroupBox>
#include <QComboBox>
#include <QScrollArea>
#include <QFileDialog>
#include <QStatusBar>
#include <QRadioButton>
#include <QDockWidget>
#include <QToolTip>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QCloseEvent>
#include <QTableWidget>
#include <time.h>

#include "LTMainWindow.h"
#include "LTApplication.h"
#include "LTRowWidget.h"

#include "midi/LTWindowsMIDI.h"
#include "audio/LTWindowsASIO.h"

static QString sLTVersion("1.0.0");
static const float sLTSettingsVersion = 1.0f;

LTMainWindow::LTMainWindow(LTApplication* pApp, QDateTime startupTime)
    : QMainWindow(NULL)
    , m_pApplication(pApp)
    , m_pPermStatusLabel(NULL)
    , m_sLoadedFilePath("UNKNOWN")
    , m_bInitialPrefsLoadComplete(false)
    , m_pWindowsMIDI(nullptr)
 {
    setupUi(this);
 
    setWindowTitle(QString("LatencyTest - v%2").arg(sLTVersion));

    m_pApplication->processEvents();

    setTabPosition(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea, QTabWidget::North);

    initTimecounter();

    m_pPermStatusLabel = new QLabel(QString("   "), this);
    m_pPermStatusLabel->setFrameStyle(QFrame::Plain | QFrame::NoFrame);
    mainWindowStatusBar->addPermanentWidget(m_pPermStatusLabel);

    m_pWindowsMIDI = new LTWindowsMIDI();

    connect(midiInRefreshButton, SIGNAL(clicked()), this, SLOT(onRefreshMIDIInPushed()));
    connect(midiOutRefreshButton, SIGNAL(clicked()), this, SLOT(onRefreshMIDIOutPushed()));
    connect(asioDeviceListComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onAsioCurrentIndexChanged(int)));
    connect(measureLatencyButton, SIGNAL(clicked()), this, SLOT(onLatencyTestMeasurePushed()));
    connect(addButton, SIGNAL(clicked()), this, SLOT(onAddLatencyTestPushed()));

    initializeMidiInPanel();
    initializeMidiOutPanel();
    initializeAsioPanel();
    initializeLatencyTestPanel();
}

LTMainWindow::~LTMainWindow(void)
{
    if(m_pPermStatusLabel)
    {
        delete m_pPermStatusLabel;
        m_pPermStatusLabel = NULL;
    }
}

void LTMainWindow::onRefreshMIDIInPushed(void)
{
    initializeMidiInPanel();
}

void LTMainWindow::onRefreshMIDIOutPushed(void)
{
    initializeMidiOutPanel();
}

void LTMainWindow::onAsioCurrentIndexChanged(int index)
{
    LTWindowsASIO* ltWindowsAsio = LTWindowsASIO::GetLockedLTWindowsAsio();

    int numDrivers = ltWindowsAsio->GetNumDrivers();

    if (numDrivers <= index)
    {
        LTWindowsASIO::UnlockLTWindowsAsio();
        return;
    }

    LTWindowsASIODriver* driver = ltWindowsAsio->GetDriver();

    if (driver == nullptr)
    {
        LTWindowsASIO::UnlockLTWindowsAsio();
        return;
    }

    if (!driver->Initialize(index, ltWindowsAsio->GetDriverName(index)))
    {
        LTWindowsASIO::UnlockLTWindowsAsio();
        return;
    }

    if (!driver->Load())
    {
        LTWindowsASIO::UnlockLTWindowsAsio();
        return;
    }

    inputChannelsValueLabel->setText(tr("%1").arg(driver->GetNumInputChannels()));
    outputChannelsValueLabel->setText(tr("%1").arg(driver->GetNumOutputChannels()));
    minSizeValueLabel->setText(tr("%1").arg(driver->GetMinSize()));
    maxSizeValueLabel->setText(tr("%1").arg(driver->GetMaxSize()));
    preferredSizeValueLabel->setText(tr("%1").arg(driver->GetPreferredSize()));
    granularityValueLabel->setText(tr("%1").arg(driver->GetGranularity()));
    inputLatencyValueLabel->setText(tr("%1").arg(driver->GetInputLatency()));
    outputLatencyValueLabel->setText(tr("%1").arg(driver->GetOutputLatency()));
    sampleRateValueLabel->setText(tr("%1").arg(driver->GetSampleRate()));

    LTWindowsASIO::UnlockLTWindowsAsio();

    UpdateLatencyTestAsio();

}

// Taken from http://stackoverflow.com/a/19695285
template <typename It>
typename std::iterator_traits<It>::value_type Median(It begin, It end)
{
    auto size = std::distance(begin, end);
        std::nth_element(begin, begin + size / 2, end);
    return *std::next(begin, size / 2);
}

void LTMainWindow::onLatencyTestMeasurePushed(void)
{
    LTWindowsASIO* ltWindowsAsio = LTWindowsASIO::GetLockedLTWindowsAsio();
    LTWindowsASIODriver* driver = ltWindowsAsio->GetDriver();
    LTWindowsASIO::UnlockLTWindowsAsio();

    for (int idx = 0; idx < m_LTRowWidgets.count(); idx++)
    {
        LTRowWidget* curRow = m_LTRowWidgets.at(idx);

        if (curRow->enableCheckBox->isChecked())
        {
            LTMIDIOutDevice* device = m_pWindowsMIDI->GetOutDevice(curRow->midiOutComboBox->currentIndex());

            if (!device->OpenDevice())
            {
                return;
            }

            int testCount = testIterationsSpinBox->value();
            float testsPerSecond = testsPerSecondSpinBox->value();

            std::vector<double> elapsedValues;

            for(int idx = 0; idx < testCount; idx++)
            {
                driver->StartSignalDetectTimer(curRow->asioInputChannelComboBox->currentIndex());

                int midiChannel = curRow->midiChannelSpinBox->value();

                if (!device->SendMIDINote(LTMIDI_Command_NoteOn, midiChannel, LTMIDI_Note_C, 3, 0x40))
                {
                    return;
                }

                int64_t nsecsElapsed = driver->WaitForSignalDetected();

                if (!device->SendMIDINote(LTMIDI_Command_NoteOffRunning, midiChannel, LTMIDI_Note_C, 3, 0x00))
                {
                    return;
                }

                double msecsElapsed = (double)nsecsElapsed / 1000000.0;

                float delay = ((1.0f / testsPerSecond) * 1000.0f) - msecsElapsed;

                QTime dieTime = QTime::currentTime().addMSecs(delay);
                while (QTime::currentTime() < dieTime)
                {
                    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
                }

                elapsedValues.push_back(msecsElapsed);
            }

            device->CloseDevice();

            double averageMsecsElapsed = Median(elapsedValues.begin(), elapsedValues.end());

            double inputLatency = (driver->GetInputLatency() / driver->GetSampleRate()) * 1000.0f;
            curRow->midiLatencyLabel->setText(QString("%1ms").arg(averageMsecsElapsed - inputLatency));
            curRow->totalLatencyLabel->setText(QString("%1ms").arg(averageMsecsElapsed));

            //latencyTestGridLayout->removeWidget(curRow->asioDriverLabel);
            //latencyTestGridLayout->removeWidget(curRow->asioInputChannelSpinBox);
            //latencyTestGridLayout->removeWidget(curRow->latencyLabel);
        }
    }
}

void LTMainWindow::onAddLatencyTestPushed(void)
{
    LTRowWidget* newRow = new LTRowWidget();
    m_LTRowWidgets.append(newRow);

    int numOutDevs = m_pWindowsMIDI->GetNumInitializedOutDevices();

    latencyTestGridLayout->removeWidget(addButton);
    latencyTestGridLayout->removeItem(latencyTestVertSpacer);

    for (int idx = 0; idx < numOutDevs; idx++)
    {
        LTMIDIOutDevice* device = m_pWindowsMIDI->GetOutDevice(idx);

        if (device != nullptr)
        {
            newRow->midiOutComboBox->addItem(device->GetName());
        }
    }

    int idx = m_LTRowWidgets.count();

    latencyTestGridLayout->addWidget(newRow->removeButton, idx, 0);
    latencyTestGridLayout->addWidget(newRow->enableCheckBox, idx, 1);
    latencyTestGridLayout->addWidget(newRow->midiOutComboBox, idx, 2);
    latencyTestGridLayout->addWidget(newRow->midiChannelSpinBox, idx, 3);
    latencyTestGridLayout->addWidget(newRow->asioDriverLabel, idx, 4);
    latencyTestGridLayout->addWidget(newRow->asioInputChannelComboBox, idx, 5);
    latencyTestGridLayout->addWidget(newRow->midiLatencyLabel, idx, 6);
    latencyTestGridLayout->addWidget(newRow->totalLatencyLabel, idx, 7);

    latencyTestGridLayout->addWidget(addButton, idx + 1, 0);
    latencyTestGridLayout->addItem(latencyTestVertSpacer, idx + 2, 1);

    UpdateLatencyTestAsio();
}

void LTMainWindow::onRemoveLatencyTestPushed(int rowIdx)
{
    LTRowWidget* curRow = m_LTRowWidgets.takeAt(rowIdx);
    latencyTestGridLayout->removeWidget(curRow->enableCheckBox);
    latencyTestGridLayout->removeWidget(curRow->midiOutComboBox);
    latencyTestGridLayout->removeWidget(curRow->midiChannelSpinBox);
    latencyTestGridLayout->removeWidget(curRow->asioDriverLabel);
    latencyTestGridLayout->removeWidget(curRow->asioInputChannelComboBox);
    latencyTestGridLayout->removeWidget(curRow->midiLatencyLabel);
    latencyTestGridLayout->removeWidget(curRow->totalLatencyLabel);
    curRow->deleteLater();
}

void LTMainWindow::UpdateLatencyTestAsio(void)
{
    LTWindowsASIO* ltWindowsAsio = LTWindowsASIO::GetLockedLTWindowsAsio();
    LTWindowsASIODriver* driver = ltWindowsAsio->GetDriver();

    if (ltWindowsAsio == nullptr)
    {
        for (int idx = 0; idx < m_LTRowWidgets.count(); idx++)
        {
            LTRowWidget* curRow = m_LTRowWidgets.at(idx);

            curRow->asioDriverLabel->setText("No ASIO Driver Selected");
            curRow->enableCheckBox->setEnabled(false);
            curRow->asioInputChannelComboBox->setEnabled(false);
            curRow->asioInputChannelComboBox->clear();
        }
    }
    else
    {
        for (int idx = 0; idx < m_LTRowWidgets.count(); idx++)
        {
            LTRowWidget* curRow = m_LTRowWidgets.at(idx);

            curRow->asioDriverLabel->setText(driver->GetName());
            curRow->enableCheckBox->setEnabled(true);
            curRow->asioInputChannelComboBox->setEnabled(true);

            curRow->asioInputChannelComboBox->clear();
        }

        int numInputChannels = driver->GetNumInputChannels();

        for (int channelIdx = 0; channelIdx < numInputChannels; channelIdx++)
        {
            QString channelName = driver->GetChannelName(channelIdx);

            for (int idx = 0; idx < m_LTRowWidgets.count(); idx++)
            {
                LTRowWidget* curRow = m_LTRowWidgets.at(idx);
                curRow->asioInputChannelComboBox->addItem(channelName);
            }
        }
    }


    LTWindowsASIO::UnlockLTWindowsAsio();
}

void LTMainWindow::initializeMidiInPanel(void)
{
    m_pWindowsMIDI->InitializeMIDIIn();

    int numInDevs = m_pWindowsMIDI->GetNumInitializedInDevices();

    // Disable sorting to prevent the idx from changing during item insertion
    midiInTable->setSortingEnabled(false);
    midiInTable->clearContents();

    for(int idx = 0; idx < numInDevs; idx++)
    {
        LTMIDIInDevice* device = m_pWindowsMIDI->GetInDevice(idx);

        if(device != nullptr)
        {
            midiInTable->insertRow(idx);

            QTableWidgetItem *newItem = nullptr;
            
            newItem = new QTableWidgetItem(tr("%1").arg(device->GetDeviceID()));
            midiInTable->setItem(idx, 0, newItem);

            newItem = new QTableWidgetItem(tr("%1").arg(device->GetMID()));
            midiInTable->setItem(idx, 1, newItem);

            newItem = new QTableWidgetItem(tr("%1").arg(device->GetPID()));
            midiInTable->setItem(idx, 2, newItem);

            newItem = new QTableWidgetItem(tr("%1").arg(device->GetDriverVersion()));
            midiInTable->setItem(idx, 3, newItem);

            newItem = new QTableWidgetItem(device->GetName());
            midiInTable->setItem(idx, 4, newItem);
        }
    }

    midiInTable->setSortingEnabled(true);
}

void LTMainWindow::initializeMidiOutPanel(void)
{
    m_pWindowsMIDI->InitializeMIDIOut();

    int numOutDevs = m_pWindowsMIDI->GetNumInitializedOutDevices();

    // Disable sorting to prevent the idx from changing during item insertion
    midiOutTable->setSortingEnabled(false);
    midiOutTable->clearContents();

    for (int idx = 0; idx < numOutDevs; idx++)
    {
        LTMIDIOutDevice* device = m_pWindowsMIDI->GetOutDevice(idx);

        if (device != nullptr)
        {
            midiOutTable->insertRow(idx);

            QTableWidgetItem *newItem = nullptr;

            newItem = new QTableWidgetItem(tr("%1").arg(device->GetDeviceID()));
            midiOutTable->setItem(idx, 0, newItem);

            newItem = new QTableWidgetItem(tr("%1").arg(device->GetMID()));
            midiOutTable->setItem(idx, 1, newItem);

            newItem = new QTableWidgetItem(tr("%1").arg(device->GetPID()));
            midiOutTable->setItem(idx, 2, newItem);

            newItem = new QTableWidgetItem(tr("%1").arg(device->GetDriverVersion()));
            midiOutTable->setItem(idx, 3, newItem);

            newItem = new QTableWidgetItem(tr("%1").arg(device->GetTechnology()));
            midiOutTable->setItem(idx, 4, newItem);

            newItem = new QTableWidgetItem(tr("%1").arg(device->GetVoices()));
            midiOutTable->setItem(idx, 5, newItem);

            newItem = new QTableWidgetItem(tr("%1").arg(device->GetNotes()));
            midiOutTable->setItem(idx, 6, newItem);

            newItem = new QTableWidgetItem(tr("%1").arg(device->GetChannelMask()));
            midiOutTable->setItem(idx, 7, newItem);

            newItem = new QTableWidgetItem(device->GetName());
            midiOutTable->setItem(idx, 8, newItem);
        }
    }

    midiOutTable->setSortingEnabled(true);
}

void LTMainWindow::initializeAsioPanel(void)
{
    LTWindowsASIO* ltWindowsAsio = LTWindowsASIO::GetLockedLTWindowsAsio();

    ltWindowsAsio->Initialize();

    int numDrivers = ltWindowsAsio->GetNumDrivers();

    asioDeviceListComboBox->clear();

    // Signals for this widget must be blocked otherwise onAsioCurrentIndexChanged will be called immediately resulting in a deadlock
    bool sigsBlocked = asioDeviceListComboBox->blockSignals(true);

    for (int idx = 0; idx < numDrivers; idx++)
    {
        asioDeviceListComboBox->addItem(ltWindowsAsio->GetDriverName(idx));
    }

    asioDeviceListComboBox->blockSignals(sigsBlocked);

    LTWindowsASIO::UnlockLTWindowsAsio();
}

void LTMainWindow::initializeLatencyTestPanel(void)
{
    onAddLatencyTestPushed();
}

void LTMainWindow::loadSettings(QSettings *settings, bool reset, bool propigateSharedSettings)
{
    settings->beginGroup("Misc");
    float savedSettingsVersion = 0.0f;
    savedSettingsVersion = settings->value("SettingsVersion", 0.0f).value<float>();

    if(savedSettingsVersion == 0.0f)
    {
        settings->endGroup();
        settings->beginGroup("Common");
        savedSettingsVersion = settings->value("SettingsVersion", 0.0f).value<float>();
        settings->endGroup();
        settings->beginGroup("Misc");
    }

    float savedSettingsMajorVersion = floorf(savedSettingsVersion);
    float ltSettingsMajorVersion = floorf(sLTSettingsVersion);
    bool immediateSave = false;

    if(savedSettingsMajorVersion < 1.0f)
    {
        settings->clear();
        immediateSave = true;
    }
    else if(ltSettingsMajorVersion > savedSettingsMajorVersion)
    {
        // CHRISNOTE: Our saved settings are now no longer supported
        // Right now we will simply nuke the settings, but in the future we should migrate them
        //LTError::warningDialog("Your current settings file is from an older version of LatencyTest and is no longer supported due to product enhancements in this version.\n\nThe settings file will now be cleared and recreated automatically.\n\nAny previously saved changes will be lost and LatencyTest will be reverted to its default settings.\n", false, false);
        settings->clear();
        immediateSave = true;
    }

	if(settings->contains("size"))
	{
        QSize savedSize = settings->value("size").toSize();
        // CHRISNOTE: Enforce a minimum value to avoid erroneous restoration
        if(savedSize.height() >= 64 && savedSize.width() >= 64)
        {
            resize(savedSize);
        }
	}

	if(settings->contains("position"))
	{
        QPoint savedPosition = settings->value("position").toPoint();
        if(QApplication::desktop()->geometry().contains(savedPosition))
        {
		    move(savedPosition);
        }
	}

	if(settings->contains("State"))
	{
		restoreState(settings->value("State").value<QByteArray>(), sLTSettingsVersion);
	}

	if(settings->contains("Maximized"))
	{
		if(settings->value("Maximized").value<bool>())
		{
			showMaximized();
		}
	}

    QDir defaultDir(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));

    QString defaultPath = defaultDir.absolutePath();

    m_sStartingLoadPath = settings->value("LoadPath", defaultPath).value<QString>();

    settings->endGroup();

    if(immediateSave)
    {
        saveSettings(settings, true, propigateSharedSettings);
    }
}

void LTMainWindow::setLoadComplete(void)
{ 
    m_bInitialPrefsLoadComplete = true; 
}

void LTMainWindow::saveSettings(QSettings *settings, bool saveAutoSettings, bool propigateSharedSettings)
{
    settings->beginGroup("Misc");

    settings->setValue("LTVersion", sLTVersion);
    settings->setValue("SettingsVersion", sLTSettingsVersion);

	settings->setValue("size", size());

    QPoint currentPosition = pos();

    if(currentPosition.x() < 0)
    {
        currentPosition.setX(0);
    }

    if(currentPosition.y() < 0)
    {
        currentPosition.setY(0);
    }

    settings->setValue("position", currentPosition);
	settings->setValue("maximized", isMaximized());

    settings->setValue("LoadPath", getLoadPath());

    settings->endGroup();

    if(saveAutoSettings)
    {

    }
}

void LTMainWindow::onAppSaveSettings(void)
{
    QSettings *settings = ((LTApplication*)LTApplication::instance())->getSettings();
    saveSettings(settings, true, false);
}

void LTMainWindow::onAppCancelSettings(void)
{
    QSettings *settings = ((LTApplication*)LTApplication::instance())->getSettings();
    loadSettings(settings, false, false);
}

void LTMainWindow::onAppRestoreSettings(void)
{
    QSettings *settings = ((LTApplication*)LTApplication::instance())->getSettings();
    loadSettings(settings, true, false);
}

void LTMainWindow::initTimecounter(void)
{
    QueryPerformanceFrequency(&m_iTicksPerSecond);
    QueryPerformanceCounter(&m_iTimeAtStart);
}

float LTMainWindow::getTimeInSeconds(void)
{
    LARGE_INTEGER curTime;
    // This is the number of clock ticks since start
    QueryPerformanceCounter(&curTime);
    
    // Divide by frequency to get the time in seconds
    LARGE_INTEGER deltaTime;
    deltaTime.QuadPart = curTime.QuadPart - m_iTimeAtStart.QuadPart;
    // Convert to mircoseconds
    deltaTime.QuadPart *= 1000000;
    deltaTime.QuadPart /= m_iTicksPerSecond.QuadPart;

    double deltaTimeInSeconds = (double)deltaTime.QuadPart / 1000000;

    return deltaTimeInSeconds;
}


void LTMainWindow::updateFps(void)
{
    float currentFrameTime = getTimeInSeconds();
    float delta = (currentFrameTime - m_fLastFrameTime);
    if (delta > 0.001f)
    {
        m_fFrameDeltaInSeconds = delta;

        m_fFps[m_iCurFpsIdx] = 1.0f / delta;
        m_iCurFpsIdx++;

        if (m_iCurFpsIdx >= MAXFPSSAMPLES)
        {
            m_iCurFpsIdx = 0;
        }

        float fTotalFps = 0;
        int numSamples = 0;
        for (int i = 0; i < MAXFPSSAMPLES; i++)
        {
            if (m_fFps[i] >= 0.0f)
            {
                fTotalFps += m_fFps[i];
                numSamples++;
            }
        }

        m_fAvgFps = fTotalFps / (float)numSamples;

        m_fLastFrameTime = currentFrameTime;
    }
}

float LTMainWindow::getPrevFps(void)
{
    // CHRISNOTE: 4 Frame Moving Average
    float fpsTotal = 0.0f;
    if (m_iCurFpsIdx == 0)
    {
        fpsTotal += m_fFps[(MAXFPSSAMPLES - 1)];
        fpsTotal += m_fFps[(MAXFPSSAMPLES - 2)];
        fpsTotal += m_fFps[(MAXFPSSAMPLES - 3)];
        fpsTotal += m_fFps[(MAXFPSSAMPLES - 4)];
    }
    else if (m_iCurFpsIdx == 1)
    {
        fpsTotal += m_fFps[0];
        fpsTotal += m_fFps[(MAXFPSSAMPLES - 1)];
        fpsTotal += m_fFps[(MAXFPSSAMPLES - 2)];
        fpsTotal += m_fFps[(MAXFPSSAMPLES - 3)];
    }
    else if (m_iCurFpsIdx == 2)
    {
        fpsTotal += m_fFps[1];
        fpsTotal += m_fFps[0];
        fpsTotal += m_fFps[(MAXFPSSAMPLES - 1)];
        fpsTotal += m_fFps[(MAXFPSSAMPLES - 2)];
    }
    else if (m_iCurFpsIdx == 3)
    {
        fpsTotal += m_fFps[2];
        fpsTotal += m_fFps[1];
        fpsTotal += m_fFps[0];
        fpsTotal += m_fFps[(MAXFPSSAMPLES - 1)];
    }
    else
    {
        fpsTotal += m_fFps[m_iCurFpsIdx - 1];
        fpsTotal += m_fFps[m_iCurFpsIdx - 2];
        fpsTotal += m_fFps[m_iCurFpsIdx - 3];
        fpsTotal += m_fFps[m_iCurFpsIdx - 4];
    }

    return (fpsTotal / 4.0f);
}

void LTMainWindow::updateStatusBar(void)
{
    QString statusBarMessage;
    bool firstMessage = false;

    QHashIterator<QString, LTStatusBarEntry> itr(m_statusBarMessages);
    while (itr.hasNext()) 
    {
        itr.next();

        LTStatusBarEntry entry = itr.value();

        if(!firstMessage)
        {
            firstMessage = true;
        }
        else
        {
            statusBarMessage.append(" | ");
        }

        statusBarMessage.append(entry.m_sString);

        if(entry.m_fTimeout > 0.0f)
        {
            entry.m_fTimeout -= m_fFrameDeltaInSeconds;

            if(entry.m_fTimeout > 0.0f)
            {
                m_statusBarMessages[itr.key()] = entry;
            }
        }

        if(entry.m_fTimeout <= 0.0f)
        {
            m_statusBarMessages.remove(itr.key());
        }
    }

    mainWindowStatusBar->showMessage(statusBarMessage);

    QString permMsgString = QString("");

    float curFps = m_fAvgFps;

    if(curFps > 1.0f && curFps < 1000.0f)
    {
        QString fpsString;
        fpsString.setNum(curFps, 'f', 1);
        permMsgString.append(fpsString);
        permMsgString.append("fps");
        permMsgString.append("   ");

        m_pPermStatusLabel->setText(permMsgString);
    }
    else
    {
        m_pPermStatusLabel->setText("   ");
    }
}

void LTMainWindow::onClickHelpAbout(void)
{

}

bool LTMainWindow::event(QEvent* lEvent)
{
	if(lEvent->type() == QEvent::Close)
	{
		QCloseEvent* closeEvent = (QCloseEvent*)lEvent;
		closeEvent->accept();
		return true;
	}

	return QMainWindow::event(lEvent);
}

bool LTMainWindow::eventFilter(QObject *obj, QEvent *ev)
{
    if (ev->type() == QEvent::Polish)
    {
        QWidget* widget = (QWidget*)obj;
        widget->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    }
    
    return QMainWindow::eventFilter(obj, ev);
}
