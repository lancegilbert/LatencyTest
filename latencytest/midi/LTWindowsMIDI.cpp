#include "LTWindowsMIDI.h"

LTWindowsMIDI::LTWindowsMIDI()
    : LTMIDI()
{

}

LTWindowsMIDI::~LTWindowsMIDI()
{

}

void LTWindowsMIDI::InitializeMIDIIn(void)
{
    if (m_pInDevs != nullptr)
    {
        delete[] m_pInDevs;
        m_pInDevs = nullptr;
        m_iNumInitializedInDevs = 0;
    }

    UINT numInDevs = midiInGetNumDevs();

    if(numInDevs > 0)
    {
        LTWindowsMIDIInDevice* inDevs = new LTWindowsMIDIInDevice[numInDevs];

        for (UINT idx = 0; idx < numInDevs; idx++)
        {
            inDevs[idx].Initialize(idx);
            m_iNumInitializedInDevs++;
        }

        m_pInDevs = inDevs;
    }
}

void LTWindowsMIDI::InitializeMIDIOut(void)
{
    if (m_pOutDevs != nullptr)
    {
        delete[] m_pOutDevs;
        m_pOutDevs = nullptr;
        m_iNumInitializedOutDevs = 0;
    }

    UINT numOutDevs = midiOutGetNumDevs();

    if (numOutDevs > 0)
    {
        LTWindowsMIDIInDevice* outDevs = new LTWindowsMIDIInDevice[numOutDevs];

        for (UINT idx = 0; idx < numOutDevs; idx++)
        {
            if (outDevs[idx].Initialize(idx))
            {
                m_iNumInitializedOutDevs++;
            }
        }

        m_pOutDevs = outDevs;
    }
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

