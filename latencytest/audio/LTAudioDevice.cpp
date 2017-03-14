#include "LTAudioDevice.h"

LTAudioDevice::LTAudioDevice()
    : m_iDeviceID(-1)
    , m_sName("INVALID")
{

}

LTAudioDevice::~LTAudioDevice()
{

}

bool LTAudioDevice::Initialize(int deviceID, QString name)
{
    m_iDeviceID = deviceID;
    m_sName = name;

    return true;
}
