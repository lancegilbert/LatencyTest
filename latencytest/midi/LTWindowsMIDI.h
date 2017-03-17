#ifndef _LTWINDOWSMIDI_H_
#define _LTWINDOWSMIDI_H_

#include <Windows.h>

#include "LTMIDIDevice.h"

class LTWindowsMIDI : public LTMIDI
{
public:
    LTWindowsMIDI(void);
    virtual ~LTWindowsMIDI(void);

    virtual void InitializeMIDIIn(void);
    virtual void InitializeMIDIOut(void);

private:
};

class LTWindowsMIDIInDevice : public LTMIDIInDevice
{
public:
    LTWindowsMIDIInDevice(void);
    virtual ~LTWindowsMIDIInDevice(void);

    virtual bool Initialize(int deviceID);

    virtual bool OpenDevice(void);
    virtual bool CloseDevice(void);

    bool OpenStream(void);
    bool CloseStream(void);

private:
    MIDIINCAPS m_InCaps;
    MMRESULT m_Result;

    HMIDIOUT m_DeviceHandle;
    HMIDISTRM m_StreamHandle;

    bool m_bDeviceOpened;
    bool m_bStreamOpened;
};

class LTWindowsMIDIOutDevice : public LTMIDIOutDevice
{
public:
    LTWindowsMIDIOutDevice(void);
    virtual ~LTWindowsMIDIOutDevice(void);

    virtual bool Initialize(int deviceID);
    virtual bool SendMIDIMessage(uint16_t low, uint16_t high);
    virtual bool SendMIDIStream(QByteArray buffer);

    virtual bool OpenDevice(void);
    virtual bool CloseDevice(void);

    bool OpenStream(void);
    bool CloseStream(void);

private:
    MIDIOUTCAPS m_OutCaps;
    MMRESULT m_Result;
    HMIDIOUT m_DeviceHandle;
    HMIDISTRM m_StreamHandle;

    bool m_bDeviceOpened;
    bool m_bStreamOpened;
};

#endif /* _LTWINDOWSMIDI_H_ */