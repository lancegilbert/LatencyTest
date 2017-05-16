#ifndef _LTMAINWINDOW_H_
#define _LTMAINWINDOW_H_

#include <QMainWindow>
#include <QGroupBox>
#include <QTimer>
#include <QSettings>
#include <QLabel>
#include <QList>
#include <QSignalMapper>

#include "ui_LTMainWindowUI.h"

#include "audio/LTAudioThreads.h"

namespace Ui {
class LTMainWindow;
}

class LTApplication;
class LTRowWidget;

class LTMainWindow : public QMainWindow, public Ui_LTMainWindow
{
    Q_OBJECT
    
public:
    LTMainWindow(LTApplication* pApp, QDateTime startupTime);
    virtual ~LTMainWindow(void);

    QString getLoadPath(void) { return m_sStartingLoadPath; }
    void setLoadPath(QString path) { m_sStartingLoadPath = path; }

    void updateStatusBar(void);
 
	void saveSettings(QSettings *settings);
	void loadSettings(QSettings *settings, bool reset);

    void setLoadComplete(void);
    bool getLoadComplete(void) { return m_bInitialPrefsLoadComplete; }

private:
    void updateLatencyTests(void);
	void updateLatencyTest(LTRowWidget* row);
	void swapRowWidgetsLatencyTest(bool progressBar);

    void initializeMidiInPanel(void);
    void initializeMidiOutPanel(void);
    void initializeAsioPanel(void);
    void initializeLatencyTestPanel(void);

	void loadLatencyTestSettings(QSettings *settings);
	void saveLatencyTestSettings(QSettings *settings);

	LTRowWidget* addLatencyTest(void);

public slots:
    void onClickHelpAbout(void);

    void onRefreshMIDIInPushed(void);
    void onRefreshMIDIOutPushed(void);
    void onAsioCurrentIndexChanged(int index);
    void onLatencyTestMeasurePushed(void);
	void onSignalDetectThreadIterationCompleted(LTSignalDetectThreadResult result);
	void onSignalDetectThreadCompleted(LTSignalDetectThreadResult result);
    void onLatencyTestCancelPushed(void);
    void onSaveSettingsPushed(void);

	void onAddLatencyTestPushed(void);
    void onRemoveLatencyTestPushed(int rowIdx);

public:

public slots:


private:
    LTApplication* m_pApplication;
 
    struct LTStatusBarEntry
    {
        QString m_sString;
        float m_fTimeout;
    };

    QHash<QString, LTStatusBarEntry> m_statusBarMessages;
    QLabel *m_pPermStatusLabel;

    QString m_sStartingLoadPath;

    QString m_sLoadedFilePath;

    bool m_bInitialPrefsLoadComplete;

    class LTWindowsMIDI *m_pWindowsMIDI;
 
    QList<LTRowWidget*> m_LTRowWidgets;

    QSignalMapper m_removeLatencyTestButtonSignalMapper;

	QList<LTSignalDetectThread*> m_SignalDetectThreads;

	int m_iSignalDetectThreadsRemaining;

protected:
	virtual bool event (QEvent* lEvent); 
    virtual bool eventFilter(QObject *obj, QEvent *ev);
};

#endif // _LTMAINWINDOW_H_
