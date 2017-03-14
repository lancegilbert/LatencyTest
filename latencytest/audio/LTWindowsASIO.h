#ifndef _LTWINDOWSASIO_H_
#define _LTWINDOWSASIO_H_

#include <Windows.h>

#include "LTAudioDevice.h"

class LTWindowsASIO
{
public:
    LTWindowsASIO();
    virtual ~LTWindowsASIO();

    virtual void Initialize(void);

    uint32_t GetNumInitializedDevices() { return m_iNumInitializedDevs; }

    class LTWindowsASIODevice* GetDevice(int deviceID);

private:
    UINT m_iNumInitializedDevs;

    class LTWindowsASIODevice* m_pDevs;
};

class LTWindowsASIODevice : public LTAudioDevice
{
public:
    LTWindowsASIODevice();
    virtual ~LTWindowsASIODevice();

    virtual bool Initialize(int deviceID);

private:
    MIDIINCAPS m_InCaps;
    MMRESULT m_Result;
};

#endif /* _LTWINDOWSASIO_H_ */