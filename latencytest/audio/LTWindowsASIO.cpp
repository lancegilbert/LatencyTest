#include "LTWindowsASIO.h"

#include "iasiodrv.h"

extern IASIO *theAsioDriver;

LTWindowsASIO::LTWindowsASIO(void)
    : m_iNumDevs(0)
    , m_pDevs(nullptr)
    , m_pAsioDrivers(nullptr)
{

}

LTWindowsASIO::~LTWindowsASIO(void)
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

    if (m_pAsioDrivers == nullptr)
    {
        m_pAsioDrivers = new AsioDrivers();
    }

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
        m_pDevs = new LTWindowsASIODriver[numDevs];

        for (UINT idx = 0; idx < numDevs; idx++)
        {
            m_pDevs[idx].Initialize(m_pAsioDrivers, idx, m_DriverNames.at(idx));
            m_iNumDevs++;
        }
    }
}

LTWindowsASIODriver* LTWindowsASIO::GetDriver(int driverID)
{
    if (m_pDevs == nullptr || m_iNumDevs == 0 || driverID >= m_iNumDevs)
    {
        return nullptr;
    }

    return &m_pDevs[driverID];
}

LTWindowsASIODriver::LTWindowsASIODriver()
    : LTAudioDriver()
    , m_pAsioDrivers(nullptr)
    , m_bLoaded(false)
{

}

LTWindowsASIODriver::~LTWindowsASIODriver(void)
{

}

bool LTWindowsASIODriver::Initialize(AsioDrivers* asioDrivers, int DriverID, QString name)
{
    m_pAsioDrivers = asioDrivers;

    return LTAudioDriver::Initialize(DriverID, name);
}

bool LTWindowsASIODriver::Load(void)
{
    m_pAsioDrivers->removeCurrentDriver();

    m_bLoaded = m_pAsioDrivers->loadDriver(GetName().toLatin1().data());

    if (!m_bLoaded)
    {
        return m_bLoaded;
    }

    ASIODriverInfo driverInfo;

    if (ASIOInit(&driverInfo) != ASE_OK)
    {
        m_bLoaded = false;
        return m_bLoaded;
    }

    long inputChannels;
    long outputChannels;

    if (ASIOGetChannels(&inputChannels, &outputChannels) != ASE_OK)
    {
        m_bLoaded = false;
        return m_bLoaded;
    }

    long minSize;
    long maxSize;
    long preferredSize;
    long granularity;

    if (ASIOGetBufferSize(&minSize, &maxSize, &preferredSize, &granularity) != ASE_OK)
    {
        m_bLoaded = false;
        return m_bLoaded;
    }

    long inputLatency;
    long outputLatency;

    // It is possible that there will not be valid latencies until after ASIO buffer creation
    if( ASIOGetLatencies(&inputLatency, &outputLatency) != ASE_OK)
    {
        m_bLoaded = false;
        return m_bLoaded;
    }

    double sampleRate;

    if (ASIOGetSampleRate(&sampleRate) != ASE_OK)
    {
        m_bLoaded = false;
        return m_bLoaded;
    }

    Open(inputChannels, outputChannels, minSize, maxSize, preferredSize, granularity, inputLatency, outputLatency, sampleRate);

    return m_bLoaded;
}