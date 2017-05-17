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
	, m_iSignalDetectThreadsRemaining(0)
 {
    setupUi(this);
 
    setWindowTitle(QString("LatencyTest - v%2").arg(sLTVersion));

    m_pApplication->processEvents();

    setTabPosition(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea, QTabWidget::North);

    m_pPermStatusLabel = new QLabel(QString("   "), this);
    m_pPermStatusLabel->setFrameStyle(QFrame::Plain | QFrame::NoFrame);
    mainWindowStatusBar->addPermanentWidget(m_pPermStatusLabel);

    m_pWindowsMIDI = new LTWindowsMIDI();

    connect(midiInRefreshButton, SIGNAL(clicked()), this, SLOT(onRefreshMIDIInPushed()));
    connect(midiOutRefreshButton, SIGNAL(clicked()), this, SLOT(onRefreshMIDIOutPushed()));
    connect(asioDeviceListComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onAsioCurrentIndexChanged(int)));
	connect(latencyTestAudioDeviceComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onLatencyTestAudioDeviceChanged(int)));
    connect(measureLatencyButton, SIGNAL(clicked()), this, SLOT(onLatencyTestMeasurePushed()));
    connect(addButton, SIGNAL(clicked()), this, SLOT(onAddLatencyTestPushed()));
    connect(panicButton, SIGNAL(clicked()), this, SLOT(onLatencyTestCancelPushed()));
    connect(saveSettingsButton, SIGNAL(clicked()), this, SLOT(onSaveSettingsPushed()));

	connect(&m_removeLatencyTestButtonSignalMapper, SIGNAL(mapped(int)), this, SLOT(onRemoveLatencyTestPushed(int)), Qt::DirectConnection);
  
    initializeMidiInPanel();
    initializeMidiOutPanel();
    initializeAsioPanel();
    initializeLatencyTestPanel();

    loadSettings(m_pApplication->getSettings(), false);

    panicButton->setText("Panic");
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

void LTMainWindow::onLatencyTestAudioDeviceChanged(int index)
{
	LTWindowsASIO* ltWindowsAsio = LTWindowsASIO::GetLockedLTWindowsAsio();

	latencyTestControls->setEnabled(false);

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

	if (!driver->Load(ltWindowsAsio))
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

	updateLatencyTests();

	bool signalsBlocked = asioDeviceListComboBox->blockSignals(true);
	asioDeviceListComboBox->setCurrentIndex(index);
	asioDeviceListComboBox->blockSignals(signalsBlocked);

	latencyTestControls->setEnabled(true);
}

void LTMainWindow::onAsioCurrentIndexChanged(int index)
{
	latencyTestAudioDeviceComboBox->setCurrentIndex(index);
}


void LTMainWindow::onLatencyTestCancelPushed(void)
{
    m_pWindowsMIDI->SendMIDIPanic(-1);
}

void LTMainWindow::onSaveSettingsPushed(void)
{
    saveSettings(m_pApplication->getSettings());
}

void LTMainWindow::onLatencyTestMeasurePushed(void)
{
	if (QString::compare("Cancel Test", measureLatencyButton->text()) == 0)
	{
		LTWindowsASIO* ltWindowsAsio = LTWindowsASIO::GetLockedLTWindowsAsio();
		LTWindowsASIODriver* driver = ltWindowsAsio->GetDriver();

		driver->CancelSignalDetection();

		LTWindowsASIO::UnlockLTWindowsAsio();

		m_pWindowsMIDI->SendMIDIPanic(-1);
	}
	else
	{
		panicButton->setEnabled(false);
		measureLatencyButton->setText("Cancel Test");

		progressBar->setValue(0);

		while (m_SignalDetectThreads.count() > 0)
		{
			LTSignalDetectThread* thread = m_SignalDetectThreads.takeFirst();
			if (thread != nullptr)
			{
				while (!thread->isFinished())
				{
					QThread::msleep(10);
				}

				thread->deleteLater();
			}
		}

		LTWindowsASIO* ltWindowsAsio = LTWindowsASIO::GetLockedLTWindowsAsio();
		LTWindowsASIODriver* driver = ltWindowsAsio->GetDriver();
		LTWindowsASIO::UnlockLTWindowsAsio();

		latencyTestListWidget->setEnabled(false);

		swapRowWidgetsLatencyTest(true);

		for (int idx = 0; idx < m_LTRowWidgets.count(); idx++)
		{
			LTRowWidget* curRow = m_LTRowWidgets.at(idx);

			if (curRow->enableCheckBox->isChecked())
			{
				LTSignalDetectThreadParameters params;
				params.parent = this;
				params.rowIdx = idx;
				params.midiDevice = m_pWindowsMIDI;
				params.audioDriver = driver;
				params.midiOutDeviceId = curRow->midiOutComboBox->currentIndex();;
				params.midiOutChannel = curRow->midiChannelSpinBox->value();
				params.asioInputChannel = curRow->asioInputChannelComboBox->currentIndex();
				params.testCount = testIterationsSpinBox->value();
				params.testsPerSecond = testsPerSecondSpinBox->value();

				curRow->progressBar->setMaximum(params.testCount);

				LTSignalDetectThread* thread = new LTSignalDetectThread(params);
				connect(thread, SIGNAL(IterationCompleted(LTSignalDetectThreadResult)), this, SLOT(onSignalDetectThreadIterationCompleted(LTSignalDetectThreadResult)));
				connect(thread, SIGNAL(Completed(LTSignalDetectThreadResult)), this, SLOT(onSignalDetectThreadCompleted(LTSignalDetectThreadResult)));

				m_SignalDetectThreads.append(thread);
				m_iSignalDetectThreadsRemaining++;
			}
		}

		if (m_SignalDetectThreads.count() > 0)
		{
			progressBar->setMaximum(m_SignalDetectThreads.count());
			m_SignalDetectThreads.first()->start();
		}
		else
		{
			latencyTestListWidget->setEnabled(true);
			swapRowWidgetsLatencyTest(false);

			panicButton->setEnabled(true);
			measureLatencyButton->setText("Measure Latency");
		}
	}
}

void LTMainWindow::swapRowWidgetsLatencyTest(bool progressBar)
{
	for (int idx = 1; idx < latencyTestGridLayout->count() && (idx - 1) < m_LTRowWidgets.count(); idx++)
	{
		QLayoutItem *layoutItem = latencyTestGridLayout->itemAtPosition(idx, 1);
		latencyTestGridLayout->removeItem(layoutItem);

		LTRowWidget* curRow = m_LTRowWidgets.at(idx - 1);

		curRow->progressBar->setValue(0);

		if (progressBar && curRow->enableCheckBox->isChecked())
		{
			latencyTestGridLayout->addWidget(curRow->progressBar, idx, 1);
			curRow->progressBar->show();
			curRow->enableCheckBox->hide();
		}
		else
		{
			latencyTestGridLayout->addWidget(curRow->enableCheckBox, idx, 1);
			curRow->progressBar->hide();
			curRow->enableCheckBox->show();
		}
	}
}

void LTMainWindow::onSignalDetectThreadIterationCompleted(LTSignalDetectThreadResult result)
{
	if (result.signalDetected && result.rowIdx < m_LTRowWidgets.count())
	{
		LTRowWidget* curRow = m_LTRowWidgets.at(result.rowIdx);

		curRow->midiLatencyLabel->setText(QString("%1ms").arg(result.midiLatency));
		curRow->totalLatencyLabel->setText(QString("%1ms").arg(result.totalLatency));

		curRow->progressBar->setValue(result.iterationsComplete);
	}
}

void LTMainWindow::onSignalDetectThreadCompleted(LTSignalDetectThreadResult result)
{
	if (result.signalDetected && result.rowIdx < m_LTRowWidgets.count())
	{
		LTRowWidget* curRow = m_LTRowWidgets.at(result.rowIdx);

		curRow->midiLatencyLabel->setText(QString("%1ms").arg(result.midiLatency));
		curRow->totalLatencyLabel->setText(QString("%1ms").arg(result.totalLatency));
	}

	m_iSignalDetectThreadsRemaining--;

	progressBar->setValue(progressBar->maximum() - m_iSignalDetectThreadsRemaining);

	LTSignalDetectThread* thread = m_SignalDetectThreads.takeFirst();
	if (thread != nullptr)
	{
		thread->deleteLater();
	}

	if (m_iSignalDetectThreadsRemaining > 0)
	{
		m_SignalDetectThreads.first()->start();
	}
	else
	{
		while (m_SignalDetectThreads.count() > 0)
		{
			LTSignalDetectThread* thread = m_SignalDetectThreads.takeFirst();
			if (thread != nullptr)
			{
				thread->deleteLater();
			}
		}

		latencyTestListWidget->setEnabled(true);
		swapRowWidgetsLatencyTest(false);

		panicButton->setEnabled(true);
		measureLatencyButton->setText("Measure Latency");
	}
}

void LTMainWindow::onAddLatencyTestPushed(void)
{
	addLatencyTest();
}

LTRowWidget* LTMainWindow::addLatencyTest(void)
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

	newRow->hide();

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

	m_removeLatencyTestButtonSignalMapper.removeMappings(newRow->removeButton);
	m_removeLatencyTestButtonSignalMapper.setMapping(newRow->removeButton, idx - 1);

	connect(newRow->removeButton, SIGNAL(clicked()), &m_removeLatencyTestButtonSignalMapper, SLOT(map()), Qt::DirectConnection);

	updateLatencyTest(newRow);

	int numEnabledTests = 0;

	for (int idx = 0; idx < m_LTRowWidgets.count(); idx++)
	{
		if (m_LTRowWidgets.at(idx)->enableCheckBox->isChecked())
		{
			numEnabledTests++;
		}
	}

	progressBar->setMaximum(numEnabledTests);

	return newRow;
}

void LTMainWindow::onRemoveLatencyTestPushed(int rowIdx)
{
	if(m_LTRowWidgets.count() > rowIdx)
	{
		LTRowWidget* curRow = m_LTRowWidgets.takeAt(rowIdx);
		m_removeLatencyTestButtonSignalMapper.removeMappings(curRow->removeButton);

		latencyTestGridLayout->removeWidget(curRow->removeButton);
		latencyTestGridLayout->removeWidget(curRow->progressBar);
		latencyTestGridLayout->removeWidget(curRow->enableCheckBox);
		latencyTestGridLayout->removeWidget(curRow->midiOutComboBox);
		latencyTestGridLayout->removeWidget(curRow->midiChannelSpinBox);
		latencyTestGridLayout->removeWidget(curRow->asioDriverLabel);
		latencyTestGridLayout->removeWidget(curRow->asioInputChannelComboBox);
		latencyTestGridLayout->removeWidget(curRow->midiLatencyLabel);
		latencyTestGridLayout->removeWidget(curRow->totalLatencyLabel);
		curRow->deleteLater();
	}

	for (int idx = 0; idx < m_LTRowWidgets.count(); idx++)
	{
		LTRowWidget* curRow = m_LTRowWidgets.at(idx);

		m_removeLatencyTestButtonSignalMapper.removeMappings(curRow->removeButton);
		m_removeLatencyTestButtonSignalMapper.setMapping(curRow->removeButton, idx);
	}

	progressBar->setMaximum(m_LTRowWidgets.count());
}

void LTMainWindow::updateLatencyTests(void)
{
	for (int idx = 0; idx < m_LTRowWidgets.count(); idx++)
	{
		LTRowWidget* curRow = m_LTRowWidgets.at(idx);

		updateLatencyTest(curRow);
	}
}

void LTMainWindow::updateLatencyTest(LTRowWidget* row)
{
    LTWindowsASIO* ltWindowsAsio = LTWindowsASIO::GetLockedLTWindowsAsio();
    LTWindowsASIODriver* driver = ltWindowsAsio->GetDriver();

    if (ltWindowsAsio == nullptr)
    {
		row->asioDriverLabel->setText("No ASIO Driver Selected");
		row->enableCheckBox->setEnabled(false);
		row->asioInputChannelComboBox->setEnabled(false);
		row->asioInputChannelComboBox->clear();
    }
    else
    {
		row->asioDriverLabel->setText(driver->GetName());
		row->enableCheckBox->setEnabled(true);
		row->asioInputChannelComboBox->setEnabled(true);

		row->asioInputChannelComboBox->clear();

        int numInputChannels = driver->GetNumInputChannels();

        for (int channelIdx = 0; channelIdx < numInputChannels; channelIdx++)
        {
            QString channelName = driver->GetChannelName(channelIdx);

			row->asioInputChannelComboBox->addItem(channelName);
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
	latencyTestAudioDeviceComboBox->clear();

    // Signals for this widget must be blocked otherwise onAsioCurrentIndexChanged will be called immediately resulting in a deadlock
    bool asioDeviceListComboBoxSigsBlocked = asioDeviceListComboBox->blockSignals(true);
	bool latencyTestAudioDeviceComboBoxSigsBlocked = latencyTestAudioDeviceComboBox->blockSignals(true);

    for (int idx = 0; idx < numDrivers; idx++)
    {
        asioDeviceListComboBox->addItem(ltWindowsAsio->GetDriverName(idx));
		latencyTestAudioDeviceComboBox->addItem(ltWindowsAsio->GetDriverName(idx));
    }

    asioDeviceListComboBox->blockSignals(asioDeviceListComboBoxSigsBlocked);
	latencyTestAudioDeviceComboBox->blockSignals(latencyTestAudioDeviceComboBoxSigsBlocked);

    LTWindowsASIO::UnlockLTWindowsAsio();
}

void LTMainWindow::initializeLatencyTestPanel(void)
{
	while (m_LTRowWidgets.count() > 0)
	{
		onRemoveLatencyTestPushed(0);
	}
}

void LTMainWindow::loadSettings(QSettings *settings, bool reset)
{
	bool immediateSave = false;

    settings->beginGroup("Misc");
	{
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
	}
    settings->endGroup();

	loadLatencyTestSettings(settings);

    if(immediateSave)
    {
        saveSettings(settings);
    }
}

void LTMainWindow::loadLatencyTestSettings(QSettings *settings)
{
	initializeLatencyTestPanel();

	settings->beginGroup("LatencyTest");
	{
		testIterationsSpinBox->setValue(settings->value("TestCount", testIterationsSpinBox->value()).toInt());
		testsPerSecondSpinBox->setValue(settings->value("TestsPerSecond", testsPerSecondSpinBox->value()).toInt());

		if (settings->contains("DeviceName"))
		{
			for (int idx = 0; idx < latencyTestAudioDeviceComboBox->count(); idx++)
			{
				if (QString::compare(latencyTestAudioDeviceComboBox->itemText(idx), settings->value("DeviceName").toString()) == 0)
				{
					if (latencyTestAudioDeviceComboBox->currentIndex() != idx)
					{
						latencyTestAudioDeviceComboBox->setCurrentIndex(idx);
					}

					break;
				}
			}
		}

		settings->beginGroup("Tests");
		{
			QStringList tests = settings->childGroups();

			for (int idx = 0; idx < tests.count(); idx++)
			{
				LTRowWidget* newRow = addLatencyTest();

				settings->beginGroup(tests.at(idx));
				{
					newRow->enableCheckBox->setChecked(settings->value("Enabled", newRow->enableCheckBox->isChecked()).toBool());
					newRow->midiOutComboBox->setCurrentText(settings->value("MidiOutDevice", newRow->midiOutComboBox->currentText()).toString());
					newRow->midiChannelSpinBox->setValue(settings->value("MidiOutChannel", newRow->midiChannelSpinBox->value()).toInt());
					newRow->asioInputChannelComboBox->setCurrentIndex(settings->value("AsioInputChannel", newRow->asioInputChannelComboBox->currentIndex()).toInt());
				}
				settings->endGroup();
			}
		}
		settings->endGroup();
	}
	settings->endGroup();

	if (m_LTRowWidgets.count() > 0)
	{
		int numEnabledTests = 0;

		for (int idx = 0; idx < m_LTRowWidgets.count(); idx++)
		{
			if (m_LTRowWidgets.at(idx)->enableCheckBox->isChecked())
			{
				numEnabledTests++;
			}
		}

		progressBar->setMaximum(numEnabledTests);
	}
	else
	{
		onAddLatencyTestPushed();
	}
}

void LTMainWindow::setLoadComplete(void)
{ 
    m_bInitialPrefsLoadComplete = true; 
}

void LTMainWindow::saveSettings(QSettings *settings)
{
    settings->beginGroup("Misc");
	{
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
	}
	settings->endGroup();

	saveLatencyTestSettings(settings);
}

void LTMainWindow::saveLatencyTestSettings(QSettings *settings)
{
	settings->beginGroup("LatencyTest");
	{
		settings->setValue("TestCount", testIterationsSpinBox->value());
		settings->setValue("TestsPerSecond", testsPerSecondSpinBox->value());
		settings->setValue("DeviceName", latencyTestAudioDeviceComboBox->currentText());

		settings->beginGroup("Tests");
		{
			settings->remove("");

			for (int idx = 0; idx < m_LTRowWidgets.count(); idx++)
			{
				LTRowWidget* curRow = m_LTRowWidgets.at(idx);

				settings->beginGroup(QString("%1").arg(idx));
				{
					settings->setValue("Enabled", curRow->enableCheckBox->isChecked());
					settings->setValue("MidiOutDevice", curRow->midiOutComboBox->currentText());
					settings->setValue("MidiOutChannel", curRow->midiChannelSpinBox->value());
					settings->setValue("AsioInputChannel", curRow->asioInputChannelComboBox->currentIndex());
				}
				settings->endGroup();
			}
		}
		settings->endGroup();
	}
	settings->endGroup();
}

void LTMainWindow::updateStatusBar(void)
{
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
