#ifndef _LTMIDIDEVICE_H_
#define _LTMIDIDEVICE_H_

#include <QString>

class LTMIDI
{
public:
    LTMIDI(void);
    virtual ~LTMIDI(void);

    virtual void InitializeMIDIIn(void) = 0;
    virtual void InitializeMIDIOut(void) = 0;

    int GetNumInitializedInDevices(void) { return m_iNumInitializedInDevs; }
    int GetNumInitializedOutDevices(void) { return m_iNumInitializedOutDevs; }

    class LTMIDIDevice* GetInDevice(int deviceID);
    class LTMIDIDevice* GetOutDevice(int deviceID);

protected:
    int m_iNumInitializedInDevs;
    int m_iNumInitializedOutDevs;

    class LTMIDIDevice* m_pInDevs;
    class LTMIDIDevice* m_pOutDevs;
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