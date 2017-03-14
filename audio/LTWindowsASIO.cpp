#include "LTWindowsASIO.h"

LTWindowsASIO::LTWindowsASIO()
    : m_iNumInitializedDevs(0)
    , m_pDevs(nullptr)
{

}

LTWindowsASIO::~LTWindowsASIO()
{
    if (m_pDevs != nullptr)
    {
        delete[] m_pDevs;
        m_iNumInitializedDevs = 0;
    }
}

void LTWindowsASIO::Initialize(void)
{
    if (m_pDevs != nullptr)
    {
        delete[] m_pDevs;
        m_iNumInitializedDevs = 0;
    }

    UINT numDevs = midiInGetNumDevs();

    if(numDevs > 0)
    {
        m_pDevs = new LTWindowsASIODevice[numDevs];

        for (UINT idx = 0; idx < numDevs; idx++)
        {
            m_pDevs[idx].Initialize(idx);
            m_iNumInitializedDevs++;
        }
    }
}

LTWindowsASIODevice* LTWindowsASIO::GetDevice(int deviceID)
{
    if (m_pDevs == nullptr || m_iNumInitializedDevs == 0 || deviceID >= m_iNumInitializedDevs)
    {
        return nullptr;
    }

    return &m_pDevs[deviceID];
}

LTWindowsASIODevice::LTWindowsASIODevice()
    : LTAudioDevice()
{

}

LTWindowsASIODevice::~LTWindowsASIODevice()
{

}

bool LTWindowsASIODevice::Initialize(int deviceID)
{
    m_Result = midiInGetDevCaps(deviceID, &m_InCaps, sizeof(MIDIINCAPS));

    if (m_Result != MMSYSERR_NOERROR)
    {
        return false;
    }

    QString name;
    name = QString::fromWCharArray(m_InCaps.szPname);

    return LTAudioDevice::Initialize(deviceID, m_InCaps.wMid, m_InCaps.wPid, m_InCaps.vDriverVersion, name);
}
