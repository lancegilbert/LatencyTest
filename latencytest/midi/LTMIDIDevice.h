#ifndef _LTMIDIDEVICE_H_
#define _LTMIDIDEVICE_H_

#include <QString>
#include <QList>

enum LTMIDI_Commands
{
    LTMIDI_Command_NoteOn = 0x0009,
    LTMIDI_Command_NoteOff = 0x0008,
    LTMIDI_Command_NoteOffRunning = 0x0109, // Because of "Running Mode" this needs to be the same command as NoteOn but still differentiatable
    LTMIDI_Command_INAVLID = 0xFFFF
};

enum LTMIDI_Notes
{
    LTMIDI_Note_C       = 0x0C,
    LTMIDI_Note_CSharp  = 0x0D,
    LTMIDI_Note_D       = 0x0E,
    LTMIDI_Note_EFlat   = 0x0F,
    LTMIDI_Note_E       = 0x10,
    LTMIDI_Note_F       = 0x11,
    LTMIDI_Note_FSharp  = 0x12,
    LTMIDI_Note_G       = 0x13,
    LTMIDI_Note_GSharp  = 0x14,
    LTMIDI_Note_A       = 0x15,
    LTMIDI_Note_BFlat   = 0x16,
    LTMIDI_Note_B       = 0x17,
    LTMIDI_Note_INVALID = 0xFF
};

class LTMIDI
{
public:
    LTMIDI(void);
    virtual ~LTMIDI(void);

    virtual void InitializeMIDIIn(void) = 0;
    virtual void InitializeMIDIOut(void) = 0;

    int GetNumInitializedInDevices(void) { return m_InDevs.count(); }
    int GetNumInitializedOutDevices(void) { return m_OutDevs.count(); }

    class LTMIDIInDevice* GetInDevice(int deviceID);
    class LTMIDIOutDevice* GetOutDevice(int deviceID);

protected:
    QList<class LTMIDIInDevice*> m_InDevs;
    QList<class LTMIDIOutDevice*> m_OutDevs;
};

class LTMIDIDevice
{
public:
    LTMIDIDevice(void);
    virtual ~LTMIDIDevice(void);

    virtual bool Initialize(int deviceID, int MID, int PID, int driverVersion, QString name);

    int GetDeviceID(void) { return m_iDeviceID; }
    int GetMID(void) { return m_iMID; }
    int GetPID(void) { return m_iPID; }
    int GetDriverVersion(void) { return m_iDriverVersion; }
    QString GetName(void) { return m_sName; }

    virtual bool OpenDevice(void) = 0;
    virtual bool CloseDevice(void) = 0;

protected:
    int m_iDeviceID;

    int m_iMID;
    int m_iPID;
    int m_iDriverVersion;
    QString m_sName;
};

class LTMIDIInDevice : public LTMIDIDevice
{
public:
    LTMIDIInDevice(void);
    virtual ~LTMIDIInDevice(void);

    virtual bool Initialize(int deviceID, int MID, int PID, int driverVersion, QString name);

protected:
};

class LTMIDIOutDevice : public LTMIDIDevice
{
public:
    LTMIDIOutDevice(void);
    virtual ~LTMIDIOutDevice(void);

    virtual bool Initialize(int deviceID, int MID, int PID, int driverVersion, QString name, int technology, int voices, int notes, int channelMask);
    virtual bool SendMIDIMessage(uint16_t low, uint16_t high) = 0;
    virtual bool SendMIDIStream(QByteArray buffer) = 0;

    bool TriggerMIDINote(uint8_t channel, LTMIDI_Notes Note, uint8_t octave, uint8_t velocity);
    bool SendMIDINote(LTMIDI_Commands command, uint8_t channel, LTMIDI_Notes note, uint8_t octave, uint8_t velocity);

    int GetTechnology(void) { return m_iTechnology; }
    int GetVoices(void) { return m_iVoices; }
    int GetNotes(void) { return m_iNotes; }
    int GetChannelMask(void) { return m_iChannelMask; }

protected:
    int m_iTechnology;
    int m_iVoices;
    int m_iNotes;
    int m_iChannelMask;
};
#endif /* _LTMIDIDEVICE_H_ */