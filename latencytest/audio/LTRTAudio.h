#ifndef _LTRTAUDIO_H_
#define _LTRTAUDIO_H_

#include <QStringList>
#include <QList>
#include <QMutex>
#include <QThread>
#include <QElapsedTimer>

#include <Windows.h>

#include "RtAudio.h"

#include "LTAudioDriver.h"

enum LTRTAudioDriverType 
{
    LTRT_AudioDriverType_MACOSX_CORE = 0x00,
    LTRT_AudioDriverType_WINDOWS_ASIO,
    LTRT_AudioDriverType_WINDOWS_DS,
    LTRT_AudioDriverType_WINDOWS_WASAPI,
    LTRT_AudioDriverType_UNIX_JACK,
    LTRT_AudioDriverType_LINUX_ALSA,
    LTRT_AudioDriverType_LINUX_PULSE,
    LTRT_AudioDriverType_LINUX_OSS,
    LTRT_AudioDriverType_DUMMY,
    LTRT_AudioDriverType_INVALID = 0xFF
};

class LTRTAudioDriver : public LTAudioDriver
{
public:
    LTRTAudioDriver(void);
    virtual ~LTRTAudioDriver(void);

    virtual bool Initialize(LTRTAudioDriverType type);
    virtual bool Load(void);

    QString GetChannelName(int index);

    void StartSignalDetectTimer(int inputChannel);

    int64_t WaitForSignalDetected(void);

private:
    void ProcessSignal(long index);

private:
    RtAudio m_RtAudio;
    RtAudio::DeviceInfo m_DeviceInfo;

    QMutex m_SignalDetectedMutex;
    QElapsedTimer m_SignalDetectedTimer;
    int m_iSignalDetectedTimerInputChannel;
    int64_t m_iSignalDetectedNsecsElapsed;

    bool m_bLoaded;
};

#endif /* _LTRTAUDIO_H_ */