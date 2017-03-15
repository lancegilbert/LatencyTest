#ifndef _LTWINDOWSASIO_H_
#define _LTWINDOWSASIO_H_

#include <QStringList>
#include <QList>
#include <QMutex>

#include <Windows.h>

#include "host/asiodrivers.h"
#include "common/asio.h"

#include "LTAudioDriver.h"

#define ASIO_MAX_DRIVERS 255
#define ASIO_MAX_DRIVER_NAME_LEN 32

class LTWindowsASIO
{
public:
    LTWindowsASIO(void);
    virtual ~LTWindowsASIO(void);

    static LTWindowsASIO* GetLockedLTWindowsAsio(void);
    static void UnlockLTWindowsAsio(void);

    virtual void Initialize(void);

    uint32_t GetNumDrivers(void) { return m_DriverNames.count(); }
    QString GetDriverName(int index) { return m_DriverNames.at(index); }

    class LTWindowsASIODriver* GetDriver(void) { return m_pDriver; }

private:
    class LTWindowsASIODriver* m_pDriver;
    QStringList m_DriverNames;

    static QMutex s_LTWindowsAsioMutex;
    static LTWindowsASIO* s_pLTWindowsAsio;
};

class LTWindowsASIODriver : public LTAudioDriver
{
public:
    LTWindowsASIODriver(AsioDrivers* asioDrivers);
    virtual ~LTWindowsASIODriver(void);

    virtual bool Initialize(int driverID, QString name);
    virtual bool Load(void);
    virtual bool StartRead(int inputChannel);

    uint64_t GetTime(void);

    void AsioCallbackBufferSwitch(long index, ASIOBool processNow);
    void AsioCallbackSampleRateDidChange(ASIOSampleRate sampleRate);
    long AsioCallbackAsioMessages(long selector, long value, void* message, double* opt);
    ASIOTime* AsioCallbackbufferSwitchTimeInfo(ASIOTime* timeInfo, long index, ASIOBool processNow);

private:
    AsioDrivers* m_pAsioDrivers;

    QList<ASIOBufferInfo*> m_InputBufferInfos;
    QList<ASIOBufferInfo*> m_OutputBufferInfos;
    ASIOTime m_TimeInfo;
    double m_fNanoSeconds;
    double m_fSamples;
    double m_fTcSamples;
    uint32_t m_uSystemRefrenceTime;
    bool m_bPostOutput;

    bool m_bLoaded;
};

#endif /* _LTWINDOWSASIO_H_ */