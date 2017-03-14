#include "LTWindowsMIDI.h"

LTWindowsMIDI::LTWindowsMIDI()
    : m_iNumInitializedInDevs(0)
    , m_iNumInitializedOutDevs(0)
    , m_pInDevs(nullptr)
    , m_pOutDevs(nullptr)
{

}

LTWindowsMIDI::~LTWindowsMIDI()
{
    if (m_pInDevs != nullptr)
    {
        delete[] m_pInDevs;
        m_iNumInitializedInDevs = 0;
    }

    if (m_pOutDevs != nullptr)
    {
        delete[] m_pOutDevs;
        m_iNumInitializedOutDevs = 0;
    }
}

void LTWindowsMIDI::InitializeMIDIIn(void)
{
    if (m_pInDevs != nullptr)
    {
        delete[] m_pInDevs;
        m_iNumInitializedInDevs = 0;
    }

    UINT numInDevs = midiInGetNumDevs();

    if(numInDevs > 0)
    {
        m_pInDevs = new LTWindowsMIDIInDevice[numInDevs];

        for (UINT idx = 0; idx < numInDevs; idx++)
        {
            m_pInDevs[idx].Initialize(idx);
            m_iNumInitializedInDevs++;
        }
    }
}

void LTWindowsMIDI::InitializeMIDIOut(void)
{
    if (m_pOutDevs != nullptr)
    {
        delete[] m_pOutDevs;
        m_iNumInitializedOutDevs = 0;
    }

    UINT numOutDevs = midiOutGetNumDevs();

    if (numOutDevs > 0)
    {
        m_pOutDevs = new LTWindowsMIDIOutDevice[numOutDevs];

        for (UINT idx = 0; idx < numOutDevs; idx++)
        {
            if (m_pOutDevs[idx].Initialize(idx))
            {
                m_iNumInitializedOutDevs++;
            }
        }
    }
}

LTWindowsMIDIInDevice* LTWindowsMIDI::GetInDevice(int deviceID)
{
    if (m_pInDevs == nullptr || m_iNumInitializedInDevs == 0 || deviceID >= m_iNumInitializedInDevs)
    {
        return nullptr;
    }

    return &m_pInDevs[deviceID];
}

LTWindowsMIDIOutDevice* LTWindowsMIDI::GetOutDevice(int deviceID)
{
    if (m_pOutDevs == nullptr || m_iNumInitializedOutDevs == 0 || deviceID >= m_iNumInitializedOutDevs)
    {
        return nullptr;
    }

    return &m_pOutDevs[deviceID];
}

LTWindowsMIDIInDevice::LTWindowsMIDIInDevice()
    : LTMIDIInDevice()
{

}

LTWindowsMIDIInDevice::~LTWindowsMIDIInDevice()
{

}

bool LTWindowsMIDIInDevice::Initialize(int deviceID)
{
    m_Result = midiInGetDevCaps(deviceID, &m_InCaps, sizeof(MIDIINCAPS));

    if (m_Result != MMSYSERR_NOERROR)
    {
        return false;
    }

    QString name;
    name = QString::fromWCharArray(m_InCaps.szPname);

    return LTMIDIInDevice::Initialize(deviceID, m_InCaps.wMid, m_InCaps.wPid, m_InCaps.vDriverVersion, name);
}

LTWindowsMIDIOutDevice::LTWindowsMIDIOutDevice()
    : LTMIDIOutDevice()
{

}

LTWindowsMIDIOutDevice::~LTWindowsMIDIOutDevice()
{

}

bool LTWindowsMIDIOutDevice::Initialize(int deviceID)
{
    m_Result = midiOutGetDevCaps(deviceID, &m_OutCaps, sizeof(MIDIOUTCAPS));

    if (m_Result != MMSYSERR_NOERROR)
    {
        return false;
    }

    QString name;
    name = QString::fromWCharArray(m_OutCaps.szPname);

    return LTMIDIOutDevice::Initialize(deviceID, m_OutCaps.wMid, m_OutCaps.wPid, m_OutCaps.vDriverVersion, name, m_OutCaps.wTechnology, m_OutCaps.wVoices, m_OutCaps.wNotes, m_OutCaps.wChannelMask);
}

