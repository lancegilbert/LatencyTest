#ifndef _LTAPPLICATION_H_
#define _LTAPPLICATION_H_

#include <QApplication>
#include <QPixmap>
#include <QSettings>
#include <QAtomicInt>
#include <QHash>
#include <QDateTime>
#include <QMainWindow>

class LTApplication : public QApplication
{
    Q_OBJECT

public:
    LTApplication(int& argc, char *argv[], QString appName);
    virtual ~LTApplication(void);

    void start(void);
    virtual void setupMainWindow(void);

	bool startupFailed(void) { return m_bStartupFailed; }

    QSettings* getSettings() { return &m_settings; }
	virtual void saveSettings(QSettings *settings, bool saveAutoSettings, bool propigateSharedSettings, bool isOnShutdown = false);
	virtual void loadSettings(QSettings *settings, bool reset, bool propigateSharedSettings);

    virtual QMainWindow* getMainWindow(void);

protected:
    virtual bool eventFilter(QObject *obj, QEvent *ev);

public slots:
    void onShutdown(void);

signals:
    void setupUiCompleteSignal(void);
    void loadApplicationSettings(void);
    void saveApplicationSettings(void);
    void restoreApplicationDefaults(void);

private:
	bool m_bStartupFailed;

protected:
    QDateTime m_startupTime;    
	QSettings m_settings;
    class LTMainWindow* m_pMainWindow;

    QHash<int, QAtomicInt*> m_keyStates;
};

#endif // _LTAPPLICATION_H_
