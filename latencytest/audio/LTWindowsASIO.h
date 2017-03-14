#ifndef _LTWINDOWSASIO_H_
#define _LTWINDOWSASIO_H_

#include <QStringList>

#include <Windows.h>

#include "host/asiodrivers.h"

#include "LTAudioDevice.h"

#define ASIO_MAX_DRIVERS 255
#define ASIO_MAX_DRIVER_NAME_LEN 32

class LTWindowsASIO
{
public:
    LTWindowsASIO();
    virtual ~LTWindowsASIO();

    virtual void Initialize(void);

    uint32_t GetNumDevices() { return m_iNumDevs; }

    class LTWindowsASIODevice* GetDevice(int deviceID);

private:
    UINT m_iNumDevs;
    class LTWindowsASIODevice* m_pDevs;
    QStringList m_DriverNames;

    AsioDrivers* m_pAsioDrivers;
};

class LTWindowsASIODevice : public LTAudioDevice
{
public:
    LTWindowsASIODevice();
    virtual ~LTWindowsASIODevice();

    virtual bool Initialize(int deviceID, QString name);

private:
    
};

#endif /* _LTWINDOWSASIO_H_ */