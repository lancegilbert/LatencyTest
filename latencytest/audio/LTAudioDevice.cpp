#include "LTAudioDevice.h"

LTAudioDevice::LTAudioDevice()
    : m_iDeviceID(-1)
    , m_iMID(-1)
    , m_iPID(-1)
    , m_iDriverVersion(-1)
    , m_sName("INVALID")
{

}

LTAudioDevice::~LTAudioDevice()
{

}

bool LTAudioDevice::Initialize(int deviceID, int MID, int PID, int driverVersion, QString name)
{
    m_iDeviceID = deviceID;
    m_iMID = MID;
    m_iPID = PID;
    m_iDriverVersion = driverVersion;
    m_sName = name;

    return true;
}
