#include "LTMIDIDevice.h"

LTMIDIDevice::LTMIDIDevice()
    : m_iDeviceID(-1)
    , m_iMID(-1)
    , m_iPID(-1)
    , m_iDriverVersion(-1)
    , m_sName("INVALID")
{

}

LTMIDIDevice::~LTMIDIDevice()
{

}

bool LTMIDIDevice::Initialize(int deviceID, int MID, int PID, int driverVersion, QString name)
{
    m_iDeviceID = deviceID;
    m_iMID = MID;
    m_iPID = PID;
    m_iDriverVersion = driverVersion;
    m_sName = name;

    return true;
}

LTMIDIInDevice::LTMIDIInDevice()
    : LTMIDIDevice()
{

}

LTMIDIInDevice::~LTMIDIInDevice()
{

}

bool LTMIDIInDevice::Initialize(int deviceID, int MID, int PID, int driverVersion, QString name)
{
    return LTMIDIDevice::Initialize(deviceID, MID, PID, driverVersion, name);
}

LTMIDIOutDevice::LTMIDIOutDevice()
    : LTMIDIDevice()
    , m_iTechnology(-1)
    , m_iVoices(-1)
    , m_iNotes(-1)
    , m_iChannelMask(-1)
{

}

LTMIDIOutDevice::~LTMIDIOutDevice()
{

}

bool LTMIDIOutDevice::Initialize(int deviceID, int MID, int PID, int driverVersion, QString name, int technology, int voices, int notes, int channelMask)
{
    m_iTechnology = technology;
    m_iVoices = voices;
    m_iNotes = notes;
    m_iChannelMask = channelMask;

    return LTMIDIDevice::Initialize(deviceID, MID, PID, driverVersion, name);
}