#ifndef _LTMIDIDEVICE_H_
#define _LTMIDIDEVICE_H_

#include <QString>

class LTMIDIDevice
{
public:
    LTMIDIDevice();
    virtual ~LTMIDIDevice();

    virtual bool Initialize(int deviceID, int MID, int PID, int driverVersion, QString name);

    int GetDeviceID(void) { return m_iDeviceID; }
    int GetMID(void) { return m_iMID; }
    int GetPID(void) { return m_iPID; }
    int GetDriverVersion(void) { return m_iDriverVersion; }
    QString GetName(void) { return m_sName; }

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
    LTMIDIInDevice();
    virtual ~LTMIDIInDevice();

    virtual bool Initialize(int deviceID, int MID, int PID, int driverVersion, QString name);

protected:
};

class LTMIDIOutDevice : public LTMIDIDevice
{
public:
    LTMIDIOutDevice();
    virtual ~LTMIDIOutDevice();

    virtual bool Initialize(int deviceID, int MID, int PID, int driverVersion, QString name, int technology, int voices, int notes, int channelMask);

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