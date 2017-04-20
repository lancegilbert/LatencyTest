#ifndef _LTWINDOWSASIO_H_
#define _LTWINDOWSASIO_H_

#include <QStringList>
#include <QList>
#include <QMutex>
#include <QThread>
#include <QElapsedTimer>

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

    uint64_t GetTime(void);
    QString GetChannelName(int index);

    void CancelSignalDetection(void);

    void StartSignalDetectTimer(int inputChannel);

    static void AsioCallbackBufferSwitch(long index, ASIOBool processNow);
    static void AsioCallbackSampleRateDidChange(ASIOSampleRate sampleRate);
    static long AsioCallbackAsioMessages(long selector, long value, void* message, double* opt);
    static ASIOTime* AsioCallbackbufferSwitchTimeInfo(ASIOTime* timeInfo, long index, ASIOBool processNow);

    int64_t WaitForSignalDetected(void);

    bool DetectNoiseFloor(int inputChannel);

private:
    void AsioCallbackBufferSwitch_Internal(long index, ASIOBool processNow);
    ASIOTime* AsioCallbackbufferSwitchTimeInfo_Internal(ASIOTime* timeInfo, long index, ASIOBool processNow);

    void ConvertSampleToNative(ASIOSampleType inputType, void* inputBuffer, ASIOSampleType outputType, void* outputBuffer);

    void ProcessSignal(long index);

private:
    AsioDrivers* m_pAsioDrivers;
    ASIOBufferInfo* m_pBufferInfos;
    ASIOChannelInfo* m_pChannelInfos;

    ASIOTime m_TimeInfo;
    double m_fNanoSeconds;
    double m_fSamples;
    double m_fTcSamples;
    uint32_t m_uSystemRefrenceTime;
    bool m_bPostOutput;

    QMutex m_SignalDetectedMutex;
    QMutex m_NoiseFloorDetectedMutex;
    QElapsedTimer m_SignalDetectedTimer;
    QAtomicInt m_iSignalDetectedTimerInputChannel;
    int64_t m_iSignalDetectedNsecsElapsed;

    double *m_pInputSamples;
    double *m_pOutputSamples;

    QAtomicInt m_iNoiseFloorDetectChannel;
    QList<double> m_NoiseFloors;
    QElapsedTimer m_NoiseFloorDetectionTimer;

    bool m_bLoaded;
};

// This is taken directly from RTAudio.h until time is taken to write a new implementation
#pragma pack(push, 1)
class LTS24 {

protected:
    unsigned char c3[3];

public:
    LTS24() {}

    LTS24& operator = (const int& i) {
        c3[0] = (i & 0x000000ff);
        c3[1] = (i & 0x0000ff00) >> 8;
        c3[2] = (i & 0x00ff0000) >> 16;
        return *this;
    }

    LTS24(const LTS24& v) { *this = v; }
    LTS24(const double& d) { *this = (int)d; }
    LTS24(const float& f) { *this = (int)f; }
    LTS24(const signed short& s) { *this = (int)s; }
    LTS24(const char& c) { *this = (int)c; }

    int asInt() {
        int i = c3[0] | (c3[1] << 8) | (c3[2] << 16);
        if (i & 0x800000) i |= ~0xffffff;
        return i;
    }
};
#pragma pack(pop)

#endif /* _LTWINDOWSASIO_H_ */