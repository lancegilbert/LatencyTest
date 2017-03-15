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

private:
    MIDIINCAPS m_InCaps;
    MMRESULT m_Result;
};

class LTWindowsMIDIOutDevice : public LTMIDIOutDevice
{
public:
    LTWindowsMIDIOutDevice(void);
    virtual ~LTWindowsMIDIOutDevice(void);

    virtual bool Initialize(int deviceID);

private:
    MIDIOUTCAPS m_OutCaps;
    MMRESULT m_Result;
};

#endif /* _LTWINDOWSMIDI_H_ */