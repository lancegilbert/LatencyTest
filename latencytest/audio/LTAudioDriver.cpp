#include "LTAudioDriver.h"

LTAudioDriver::LTAudioDriver(void)
    : m_bOpen(false)
    , m_iDriverID(-1)
    , m_sName("INVALID")
    , m_iInputChannels(0)
    , m_iOutputChannels(0)
    , m_iMinSize(0)
    , m_iMaxSize(0)
    , m_iPreferredSize(0)
    , m_iGranularity(0)
    , m_iInputLatency(0)
    , m_iOutputLatency(0)
    , m_fSampleRate(0)
{

}

LTAudioDriver::~LTAudioDriver(void)
{

}

bool LTAudioDriver::Initialize(int driverID, QString name)
{
    m_bOpen = false;

    m_iDriverID = driverID;
    m_sName = name;

    return true;
}

bool LTAudioDriver::Open(int inputChannels, int outputChannels, int minSize, int maxSize, int preferredSize, int granularity, int inputLatency, int outputLatency, double sampleRate)
{
    m_iInputChannels = inputChannels;
    m_iOutputChannels = outputChannels;
    m_iMinSize = minSize;
    m_iMaxSize = maxSize;
    m_iPreferredSize = preferredSize;
    m_iGranularity = granularity;
    m_iInputLatency = inputLatency;
    m_iOutputLatency = outputLatency;
    m_fSampleRate = sampleRate;

    m_bOpen = true;

    return m_bOpen;
}
