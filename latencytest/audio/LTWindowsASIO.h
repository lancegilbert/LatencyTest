#ifndef _LTWINDOWSASIO_H_
#define _LTWINDOWSASIO_H_

#include <QStringList>

#include <Windows.h>

#include "host/asiodrivers.h"
#include "common/asio.h"

#include "LTAudioDriver.h"

#define ASIO_MAX_DRIVERS 255
#define ASIO_MAX_DRIVER_NAME_LEN 32

class LTWindowsASIO
{
public:
    LTWindowsASIO(void);
    virtual ~LTWindowsASIO(void);

    virtual void Initialize(void);

    uint32_t GetNumDrivers(void) { return m_iNumDevs; }

    class LTWindowsASIODriver* GetDriver(int driverID);

private:
    UINT m_iNumDevs;
    class LTWindowsASIODriver* m_pDevs;
    QStringList m_DriverNames;

    AsioDrivers* m_pAsioDrivers;
};

class LTWindowsASIODriver : public LTAudioDriver
{
public:
    LTWindowsASIODriver(void);
    virtual ~LTWindowsASIODriver(void);

    virtual bool Initialize(AsioDrivers* asioDrivers, int driverID, QString name);
    virtual bool Load(void);

private:
    AsioDrivers* m_pAsioDrivers;
    bool m_bLoaded;
};

#endif /* _LTWINDOWSASIO_H_ */