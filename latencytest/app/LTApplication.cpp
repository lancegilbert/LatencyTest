#include <QFile>
#include <QString>
#include <QTextStream>
#include <QTimer>
#include <QStandardPaths>
#include <QMessageBox>

#include <stdio.h>

#ifdef _WIN32
#include <process.h>
#endif

#include "LTApplication.h"
#include "LTMainWindow.h"

LTApplication::LTApplication(int& argc, char *argv[], QString appName)
    : QApplication(argc, argv)
	, m_bStartupFailed(false)
    , m_startupTime(QDateTime::currentDateTimeUtc())
#ifdef _DEBUG
    , m_settings(QString("%1.ini").arg(appName), QSettings::IniFormat)
#else
	, m_settings(QSettings::IniFormat, QSettings::UserScope, "LatencyTest", appName)
#endif
{
    QCoreApplication::setOrganizationName("LatencyTest");
    QCoreApplication::setOrganizationDomain("lancegilbert.us");
    QCoreApplication::setApplicationName(appName);
}

LTApplication::~LTApplication(void)
{
    disconnect();
}

void LTApplication::start(void)
{
#if 0
	if(!loadStyleSheet("./styles/dark.qss"))
	{
		m_bStartupFailed = true;
		return;
	}
#endif

    setupMainWindow();

    processEvents();
}

void LTApplication::onShutdown(void)
{
    quit();
}

bool LTApplication::eventFilter(QObject *obj, QEvent *ev)
{
    if (ev->type() == QEvent::Polish)
    {
        QWidget* widget = (QWidget*)obj;
        widget->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    }
    
    return QApplication::eventFilter(obj, ev);
}

QMainWindow* LTApplication::getMainWindow(void)
{
    return m_pMainWindow;
}


void LTApplication::setupMainWindow(void)
{
    m_pMainWindow = new LTMainWindow(this, m_startupTime);

    emit setupUiCompleteSignal();

    installEventFilter(m_pMainWindow);

    connect(this, SIGNAL(applicationStateChanged(Qt::ApplicationState)), this, SLOT(onApplicationStateChanged(Qt::ApplicationState)));

    m_pMainWindow->show();

    if (QCoreApplication::closingDown() == true)
    {
        return;
    }

    m_pMainWindow->hide();

    m_pMainWindow->show();

    loadSettings(getSettings(), false);

    processEvents();

    m_pMainWindow->setLoadComplete();

    processEvents();
}


void LTApplication::loadSettings(QSettings *settings, bool reset)
{
    if (m_pMainWindow)
    {
        m_pMainWindow->loadSettings(settings, reset);
    }
}

void LTApplication::saveSettings(QSettings *settings, bool isOnShutdown /* = false */)
{
    if (m_pMainWindow)
    {
        if (isOnShutdown)
        {
            if (0)
            {
                QMessageBox::StandardButtons buttons = (QMessageBox::Yes | QMessageBox::No);
                QMessageBox notificationDialog(QMessageBox::Question, "LatencyTest Settings", "Would you like to save the changes made to your settings before closing? Any unsaved changes will be lost.", buttons);

                notificationDialog.setDefaultButton(QMessageBox::Yes);

                switch (notificationDialog.exec())
                {
                    case QMessageBox::Yes:
                    {
                        m_pMainWindow->saveSettings(settings);
                    }
                    case QMessageBox::No:
                    default:
                    {

                    }
                }
            }
        }
    }
}

int main(int argc, char *argv[])
{
    qputenv("QT_HIDPI_AWARE", "1");
    qputenv("QT_HIGHDPI_AWARE", "1");

    QString AppName("LatencyTest");

    LTApplication *pLTApp = new LTApplication(argc, argv, AppName);

    QCoreApplication::setOrganizationName("LatencyTest");
    QCoreApplication::setOrganizationDomain("lancegilbert.us");
    QCoreApplication::setApplicationName(AppName);

    if (pLTApp->startupFailed())
    {
        //LTError::fatalErrorDialog("Error loading LatencyTest resources. Please check working directory");
    }
    else
    {
        pLTApp->start();
    }

    if (pLTApp->startupFailed())
    {
        //LTError::fatalErrorDialog("Error Starting LatencyTest.");
    }

    int retVal = pLTApp->exec();

    pLTApp->saveSettings(pLTApp->getSettings(), true);

    delete pLTApp;

    return retVal;
}
