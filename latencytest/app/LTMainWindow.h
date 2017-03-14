#ifndef _LTMAINWINDOW_H_
#define _LTMAINWINDOW_H_

#include <QMainWindow>
#include <QGroupBox>
#include <QTimer>
#include <QSettings>
#include <QLabel>

#include "Windows.h"

#include "ui_LTMainWindowUI.h"

#define MAXFPSSAMPLES 120 // 2 seconds(ish)

namespace Ui {
class LTMainWindow;
}

class LTApplication;

class LTMainWindow : public QMainWindow, public Ui_LTMainWindow
{
    Q_OBJECT
    
public:
    LTMainWindow(LTApplication* pApp, QDateTime startupTime);
    virtual ~LTMainWindow();

    QString getLoadPath(void) { return m_sStartingLoadPath; }
    void setLoadPath(QString path) { m_sStartingLoadPath = path; }

    void updateFps(void);
    void updateStatusBar(void);
    float getTimeInSeconds();
    float getAvgFps() { return m_fAvgFps; }
    float getPrevFps();
    float getLastFrameDelta() { return m_fFrameDeltaInSeconds; }
 
	void saveSettings(QSettings *settings, bool saveAutoSettings, bool propigateSharedSettings);
	void loadSettings(QSettings *settings, bool reset, bool propigateSharedSettings);

    void setLoadComplete(void);
    bool getLoadComplete(void) { return m_bInitialPrefsLoadComplete; }

private:
    void initTimecounter();

    void initializeMidiInPanel(void);
    void initializeMidiOutPanel(void);
    void initializeAsioPanel(void);

public slots:
    void onClickHelpAbout(void);

    void onAppSaveSettings(void);
    void onAppCancelSettings(void);
    void onAppRestoreSettings(void);

    void onRefreshMIDIInPushed(void);
    void onRefreshMIDIOutPushed(void);

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

    int m_iCurFpsIdx;
    float m_fFps[MAXFPSSAMPLES];
    float m_fAvgFps;
    float m_fLastFrameTime;
    float m_fFrameDeltaInSeconds;

    LARGE_INTEGER m_iTimeAtStart;
    LARGE_INTEGER m_iTicksPerSecond;

    class LTWindowsMIDI *m_pWindowsMIDI;
    class LTWindowsASIO *m_pWindowsASIO;
 
protected:
	virtual bool event (QEvent* lEvent); 
    virtual bool eventFilter(QObject *obj, QEvent *ev);
};

#endif // _LTMAINWINDOW_H_
