#ifndef _LTWINDOWSMIDI_H_
#define _LTWINDOWSMIDI_H_

#include <Windows.h>

#include "LTMIDIDevice.h"

class LTWindowsMIDI
{
public:
    LTWindowsMIDI(void);
    virtual ~LTWindowsMIDI(void);

    virtual void InitializeMIDIIn(void);
    virtual void InitializeMIDIOut(void);

    uint32_t GetNumInitializedInDevices(void) { return m_iNumInitializedInDevs; }
    uint32_t GetNumInitializedOutDevices(void) { return m_iNumInitializedOutDevs; }

    class LTWindowsMIDIInDevice* GetInDevice(int deviceID);
    class LTWindowsMIDIOutDevice* GetOutDevice(int deviceID);

private:
    UINT m_iNumInitializedInDevs;
    UINT m_iNumInitializedOutDevs;

    class LTWindowsMIDIInDevice* m_pInDevs;
    class LTWindowsMIDIOutDevice* m_pOutDevs;
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