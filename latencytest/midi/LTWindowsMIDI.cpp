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
    while (m_InDevs.count() > 0)
    {
        delete m_InDevs.takeLast();
    }

    UINT numInDevs = midiInGetNumDevs();

    for (UINT idx = 0; idx < numInDevs; idx++)
    {
        LTWindowsMIDIInDevice* inDev = new LTWindowsMIDIInDevice();

        if (!inDev->Initialize(idx))
        {
            delete inDev;
        }
        else
        {
            m_InDevs.append(inDev);
        }
    }
}

void LTWindowsMIDI::InitializeMIDIOut(void)
{
    while (m_InDevs.count() > 0)
    {
        delete m_InDevs.takeLast();
    }

    UINT numOutDevs = midiOutGetNumDevs();

    for (UINT idx = 0; idx < numOutDevs; idx++)
    {
        LTWindowsMIDIOutDevice* outDev = new LTWindowsMIDIOutDevice();

        if (!outDev->Initialize(idx))
        {
            delete outDev;
        }
        else
        {
            m_OutDevs.append(outDev);
        }
    }
}

LTWindowsMIDIInDevice::LTWindowsMIDIInDevice()
    : LTMIDIInDevice()
    , m_bDeviceOpened(false)
    , m_bStreamOpened(false)
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


bool LTWindowsMIDIInDevice::OpenDevice(void)
{
    if (m_bDeviceOpened)
    {
        CloseDevice();
        m_bDeviceOpened = false;
    }

    MMRESULT result = midiOutOpen(&m_DeviceHandle, GetDeviceID(), NULL, NULL, CALLBACK_NULL);

    if (result != MMSYSERR_NOERROR)
    {
        m_bDeviceOpened = false;

        return m_bDeviceOpened;
    }

    m_bDeviceOpened = true;
    return m_bDeviceOpened;
}

bool LTWindowsMIDIInDevice::CloseDevice(void)
{
    MMRESULT result = midiOutClose(m_DeviceHandle);

    if (result != MMSYSERR_NOERROR)
    {
        return false;
    }

    m_bDeviceOpened = false;
    return true;
}


bool LTWindowsMIDIInDevice::OpenStream(void)
{
    if (m_bStreamOpened)
    {
        CloseDevice();
        m_bStreamOpened = false;
    }

    UINT deviceID;

    MMRESULT result = midiStreamOpen(&m_StreamHandle, &deviceID, 1, NULL, NULL, CALLBACK_NULL);

    if (result != MMSYSERR_NOERROR)
    {
        m_bStreamOpened = false;

        return m_bStreamOpened;
    }

    m_bStreamOpened = true;
    return m_bStreamOpened;
}

bool LTWindowsMIDIInDevice::CloseStream(void)
{
    MMRESULT result = midiStreamClose(m_StreamHandle);

    if (result != MMSYSERR_NOERROR)
    {
        return false;
    }

    m_bStreamOpened = false;
    return true;
}

LTWindowsMIDIOutDevice::LTWindowsMIDIOutDevice()
    : LTMIDIOutDevice()
    , m_bDeviceOpened(false)
    , m_bStreamOpened(false)
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

bool LTWindowsMIDIOutDevice::SendMIDIMessage(uint16_t low, uint16_t high)
{
    DWORD message = (low | (high << 16));
    
    MMRESULT result = midiOutShortMsg(m_DeviceHandle, message);

    if (result != MMSYSERR_NOERROR)
    {
        CloseDevice();
        return false;
    }

    return true;
}

bool LTWindowsMIDIOutDevice::SendMIDIStream(QByteArray buffer)
{
    return false;

// Not fully implemented yet
#if 0
    if (!OpenDevice())
    {
        return false;
    }

    MIDIHDR midiHeader;

    midiHeader.lpData = buffer.data_ptr()->data();
    midiHeader.dwBufferLength = buffer.data_ptr()->size;
    midiHeader.dwBytesRecorded = buffer.data_ptr()->size;
    midiHeader.dwUser = NULL;
    midiHeader.dwFlags = 0;


    MMRESULT result = midiOutPrepareHeader(m_DeviceHandle, &midiHeader, sizeof(MIDIHDR));

    if (result != MMSYSERR_NOERROR)
    {
        return false;
    }


    result = midiOutUnprepareHeader(m_DeviceHandle, &midiHeader, sizeof(MIDIHDR));

    CloseDevice();

    return true;
#endif
}

bool LTWindowsMIDIOutDevice::OpenDevice(void)
{
    if (m_bDeviceOpened)
    {
        CloseDevice();
        m_bDeviceOpened = false;
    }

    MMRESULT result = midiOutOpen(&m_DeviceHandle, GetDeviceID(), NULL, NULL, CALLBACK_NULL);

    if (result != MMSYSERR_NOERROR)
    {
        m_bDeviceOpened = false;

        return m_bDeviceOpened;
    }

    m_bDeviceOpened = true;
    return m_bDeviceOpened;
}

bool LTWindowsMIDIOutDevice::CloseDevice(void)
{
    MMRESULT result = midiOutClose(m_DeviceHandle);

    if (result != MMSYSERR_NOERROR)
    {
        return false;
    }

    m_bDeviceOpened = false;
    return true;
}


bool LTWindowsMIDIOutDevice::OpenStream(void)
{
    if (m_bStreamOpened)
    {
        CloseDevice();
        m_bStreamOpened = false;
    }

    UINT deviceID;

    MMRESULT result = midiStreamOpen(&m_StreamHandle, &deviceID, 1, NULL, NULL, CALLBACK_NULL);

    if (result != MMSYSERR_NOERROR)
    {
        m_bStreamOpened = false;

        return m_bStreamOpened;
    }

    m_bStreamOpened = true;
    return m_bStreamOpened;
}

bool LTWindowsMIDIOutDevice::CloseStream(void)
{
    MMRESULT result = midiStreamClose(m_StreamHandle);

    if (result != MMSYSERR_NOERROR)
    {
        return false;
    }

    m_bStreamOpened = false;
    return true;
}