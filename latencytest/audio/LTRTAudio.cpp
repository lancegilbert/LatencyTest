#include "LTRTAudio.h"

LTRTAudioDriver::LTRTAudioDriver(void)
    : LTAudioDriver()
    , m_iSignalDetectedTimerInputChannel(0)
{
    
}

LTRTAudioDriver::~LTRTAudioDriver(void)
{
}

bool LTRTAudioDriver::Initialize(LTRTAudioDriverType type)
{
    //m_RtAudio.openRtApi(type);

    //return LTAudioDriver::Initialize(DriverID, name);
    return false;
}

bool LTRTAudioDriver::Load(void)
{
    //Open(inputChannels, outputChannels, minSize, maxSize, preferredSize, granularity, inputLatency, outputLatency, sampleRate);

    return m_bLoaded;
}

QString LTRTAudioDriver::GetChannelName(int index)
{
    return QString("");
}

void LTRTAudioDriver::StartSignalDetectTimer(int inputChannel)
{
    m_SignalDetectedMutex.lock();

    m_iSignalDetectedTimerInputChannel = inputChannel;

    if (m_SignalDetectedTimer.isValid())
    {
        m_SignalDetectedTimer.restart();
    }
    else
    {
        m_SignalDetectedTimer.start();
    }
}

int64_t LTRTAudioDriver::WaitForSignalDetected(void)
{
    m_SignalDetectedMutex.lock();
    m_SignalDetectedMutex.unlock();

    return m_iSignalDetectedNsecsElapsed;
}

void LTRTAudioDriver::ProcessSignal(long index)
{
    if (m_SignalDetectedTimer.isValid())
    {
        m_iSignalDetectedNsecsElapsed = m_SignalDetectedTimer.nsecsElapsed();

        double sample = 0.0f; //ConvertSampleToNative(m_pChannelInfos[m_iSignalDetectedTimerInputChannel].type, m_pBufferInfos[m_iSignalDetectedTimerInputChannel].buffers[index^1]);

        static double edgeThreshold = 10000000000.0f;

        if (sample > edgeThreshold)
        {
            m_SignalDetectedTimer.invalidate();
            m_SignalDetectedMutex.unlock();
        }
    }
}

