#include "LTMIDIDevice.h"

LTMIDI::LTMIDI()
{

}

LTMIDI::~LTMIDI()
{
    while (m_InDevs.count() > 0)
    {
        delete m_InDevs.takeLast();
    }

    while (m_OutDevs.count() > 0)
    {
        delete m_OutDevs.takeLast();
    }
}

LTMIDIInDevice* LTMIDI::GetInDevice(int deviceID)
{
    if (m_InDevs.count() <= deviceID)
    {
        return nullptr;
    }

    return m_InDevs.at(deviceID);
}

LTMIDIOutDevice* LTMIDI::GetOutDevice(int deviceID)
{
    if (m_OutDevs.count() <= deviceID)
    {
        return nullptr;
    }

    return m_OutDevs.at(deviceID);
}

void LTMIDI::SendMIDIPanic(int deviceID)
{
    if (deviceID < 0)
    {
        int numOutDevs = GetNumInitializedOutDevices();

        for (int idx = 0; idx < numOutDevs; idx++)
        {
            // Recursively call this function with valid device indexes 
            SendMIDIPanic(idx);
        }
    }
    else
    {
        LTMIDIOutDevice* device = GetOutDevice(deviceID);

        if (device != nullptr)
        {
            device->OpenDevice();

            //for(int channelIdx = 0; channelIdx < 16; channelIdx++)
            int channelIdx = 0;
            {
                //for(int octaveIdx = -1; octaveIdx < 4; octaveIdx++)
                int octaveIdx = 3;
                {
                    device->SendMIDINote(LTMIDI_Command_NoteOff, channelIdx, LTMIDI_Note_C, octaveIdx, 0x00);
#if 0
                    device->SendMIDINote(LTMIDI_Command_NoteOff, channelIdx, LTMIDI_Note_CSharp, octaveIdx, 0x00);
                    device->SendMIDINote(LTMIDI_Command_NoteOff, channelIdx, LTMIDI_Note_D, octaveIdx, 0x00);
                    device->SendMIDINote(LTMIDI_Command_NoteOff, channelIdx, LTMIDI_Note_EFlat, octaveIdx, 0x00);
                    device->SendMIDINote(LTMIDI_Command_NoteOff, channelIdx, LTMIDI_Note_E, octaveIdx, 0x00);
                    device->SendMIDINote(LTMIDI_Command_NoteOff, channelIdx, LTMIDI_Note_F, octaveIdx, 0x00);
                    device->SendMIDINote(LTMIDI_Command_NoteOff, channelIdx, LTMIDI_Note_FSharp, octaveIdx, 0x00);
                    device->SendMIDINote(LTMIDI_Command_NoteOff, channelIdx, LTMIDI_Note_G, octaveIdx, 0x00);
                    device->SendMIDINote(LTMIDI_Command_NoteOff, channelIdx, LTMIDI_Note_GSharp, octaveIdx, 0x00);
                    device->SendMIDINote(LTMIDI_Command_NoteOff, channelIdx, LTMIDI_Note_A, octaveIdx, 0x00);
                    device->SendMIDINote(LTMIDI_Command_NoteOff, channelIdx, LTMIDI_Note_BFlat, octaveIdx, 0x00);
                    device->SendMIDINote(LTMIDI_Command_NoteOff, channelIdx, LTMIDI_Note_B, octaveIdx, 0x00);
#endif
                }
            }

            device->CloseDevice();
        }
    }
}

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

bool LTMIDIOutDevice::TriggerMIDINote(uint8_t channel, LTMIDI_Notes Note, uint8_t octave, uint8_t velocity)
{
    if (!SendMIDINote(LTMIDI_Command_NoteOn, channel, Note, octave, velocity))
    {
        return false;
    }

    // todo: Add wait logic here once callbacks are in place

    return SendMIDINote(LTMIDI_Command_NoteOff, channel, Note, octave, velocity);
}

bool LTMIDIOutDevice::SendMIDINote(LTMIDI_Commands command, uint8_t channel, LTMIDI_Notes note, uint8_t octave, uint8_t velocity)
{
    uint16_t low = 0;
    uint16_t high = 0;

    if (command == LTMIDI_Command_NoteOn)
    {
        // Mask to compliant velocity range (0-127)
        velocity &= 0x7F;

        high = velocity;
    }

    uint8_t filteredCommand = (command & 0x000F);

    // Transpose channel (1-16 -> 0-15) and mask compliant channel range (0-15)
    channel = ((channel - 1) & 0x0F);
    
    uint8_t transposedNote = note + (LTMIDI_Note_C * octave);

    // Mask to compliant note range (0-127)
    transposedNote &= 0x7F;

    low = (channel) | (filteredCommand << 4) | (transposedNote << 8);

    return SendMIDIMessage(low, high);
}