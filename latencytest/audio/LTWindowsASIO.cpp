#include "LTWindowsASIO.h"



LTWindowsASIO::LTWindowsASIO()
    : m_iNumDevs(0)
    , m_pDevs(nullptr)
    , m_pAsioDrivers(nullptr)
{

}

LTWindowsASIO::~LTWindowsASIO()
{
    if (m_pDevs != nullptr)
    {
        delete[] m_pDevs;
        m_pDevs = nullptr;

        m_iNumDevs = 0;
    }

    if (m_pAsioDrivers != nullptr)
    {
        delete m_pAsioDrivers;
        m_pAsioDrivers = nullptr;
    }
}

void LTWindowsASIO::Initialize(void)
{
    if (m_pDevs != nullptr)
    {
        delete[] m_pDevs;
        m_iNumDevs = 0;
    }

    m_DriverNames.clear();

    m_pAsioDrivers = new AsioDrivers();

    char* asioDriverNames[ASIO_MAX_DRIVERS];

    for (int idx = 0; idx < ASIO_MAX_DRIVERS; idx++)
    {
        asioDriverNames[idx] = new char[ASIO_MAX_DRIVER_NAME_LEN];
        asioDriverNames[idx][0] = '\0';
    }

    m_pAsioDrivers->getDriverNames(asioDriverNames, ASIO_MAX_DRIVERS);

    for (int idx = 0; idx < ASIO_MAX_DRIVERS; idx++)
    {
        if(asioDriverNames[idx][0] != '\0')
        {
            m_DriverNames.append(asioDriverNames[idx]);
        }

        delete[] asioDriverNames[idx];
    }

    UINT numDevs = m_DriverNames.count();

    if(numDevs > 0)
    {
        m_pDevs = new LTWindowsASIODevice[numDevs];

        for (UINT idx = 0; idx < numDevs; idx++)
        {
            m_pDevs[idx].Initialize(idx, m_DriverNames.at(idx));
            m_iNumDevs++;
        }
    }
}

LTWindowsASIODevice* LTWindowsASIO::GetDevice(int deviceID)
{
    if (m_pDevs == nullptr || m_iNumDevs == 0 || deviceID >= m_iNumDevs)
    {
        return nullptr;
    }

    return &m_pDevs[deviceID];
}

LTWindowsASIODevice::LTWindowsASIODevice()
    : LTAudioDevice()
{

}

LTWindowsASIODevice::~LTWindowsASIODevice()
{

}

bool LTWindowsASIODevice::Initialize(int deviceID, QString name)
{
    return LTAudioDevice::Initialize(deviceID, name);
}
