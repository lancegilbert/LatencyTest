#include "LTWindowsASIO.h"

#include "iasiodrv.h"

extern IASIO *theAsioDriver;

QMutex LTWindowsASIO::s_LTWindowsAsioMutex;
LTWindowsASIO* LTWindowsASIO::s_pLTWindowsAsio = nullptr;

LTWindowsASIO* LTWindowsASIO::GetLockedLTWindowsAsio(void)
{
    if (s_pLTWindowsAsio == nullptr)
    {
        s_pLTWindowsAsio = new LTWindowsASIO();
    }

    s_LTWindowsAsioMutex.lock();

    return s_pLTWindowsAsio;
}

void LTWindowsASIO::UnlockLTWindowsAsio(void)
{
    s_LTWindowsAsioMutex.unlock();
}

LTWindowsASIO::LTWindowsASIO(void)
    : m_pDriver(nullptr)
{
}

LTWindowsASIO::~LTWindowsASIO(void)
{
    if (m_pDriver != nullptr)
    {
        delete m_pDriver;
        m_pDriver = nullptr;
    }
}

void LTWindowsASIO::Initialize(void)
{
    if (m_pDriver != nullptr)
    {
        delete m_pDriver;
    }

    m_DriverNames.clear();

    AsioDrivers* asioDrivers = new AsioDrivers();

    char* asioDriverNames[ASIO_MAX_DRIVERS];

    for (int idx = 0; idx < ASIO_MAX_DRIVERS; idx++)
    {
        asioDriverNames[idx] = new char[ASIO_MAX_DRIVER_NAME_LEN];
        asioDriverNames[idx][0] = '\0';
    }

    asioDrivers->getDriverNames(asioDriverNames, ASIO_MAX_DRIVERS);

    for (int idx = 0; idx < ASIO_MAX_DRIVERS; idx++)
    {
        if(asioDriverNames[idx][0] != '\0')
        {
            m_DriverNames.append(asioDriverNames[idx]);
        }

        delete[] asioDriverNames[idx];
    }

    m_pDriver = new LTWindowsASIODriver(asioDrivers);
}

LTWindowsASIODriver::LTWindowsASIODriver(AsioDrivers* asioDrivers)
    : LTAudioDriver()
    , m_pAsioDrivers(asioDrivers)
    , m_pBufferInfos(nullptr)
    , m_bLoaded(false)
    , m_fNanoSeconds(0.0)
    , m_fSamples(0.0)
    , m_fTcSamples(0.0)
    , m_uSystemRefrenceTime(0)
    , m_bPostOutput(false)
{
    
}

LTWindowsASIODriver::~LTWindowsASIODriver(void)
{
    delete[] m_pBufferInfos;

    if (m_pAsioDrivers != nullptr)
    {
        delete m_pAsioDrivers;
    }
}

bool LTWindowsASIODriver::Initialize(int DriverID, QString name)
{
    return LTAudioDriver::Initialize(DriverID, name);
}

bool LTWindowsASIODriver::Load(void)
{
    m_pAsioDrivers->removeCurrentDriver();

    delete[] m_pBufferInfos;

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

    int totalChannels = inputChannels + outputChannels;
    m_pBufferInfos = new ASIOBufferInfo[totalChannels];

    for (int idx = 0; idx < totalChannels; idx++)
    {
        if (idx < inputChannels)
        {
            m_pBufferInfos[idx].isInput = ASIOTrue;
            m_pBufferInfos[idx].channelNum = idx;
        }
        else
        {
            m_pBufferInfos[idx].isInput = ASIOFalse;
            m_pBufferInfos[idx].channelNum = (idx - inputChannels);
        }

        m_pBufferInfos[idx].buffers[0] = m_pBufferInfos[idx].buffers[1] = 0;
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

    double sampleRate;

    if (ASIOGetSampleRate(&sampleRate) != ASE_OK)
    {
        m_bLoaded = false;

        return m_bLoaded;
    }

    if (ASIOOutputReady() == ASE_OK)
    {
        m_bPostOutput = true;
    }
    else
    {
        m_bPostOutput = false;
    }

    ASIOCallbacks asioCallbacks;
    asioCallbacks.bufferSwitch = &LTWindowsASIODriver::AsioCallbackBufferSwitch;
    asioCallbacks.sampleRateDidChange = &LTWindowsASIODriver::AsioCallbackSampleRateDidChange;
    asioCallbacks.asioMessage = &LTWindowsASIODriver::AsioCallbackAsioMessages;
    asioCallbacks.bufferSwitchTimeInfo = &LTWindowsASIODriver::AsioCallbackbufferSwitchTimeInfo;

    if (ASIOCreateBuffers(m_pBufferInfos, totalChannels, preferredSize, &asioCallbacks) != ASE_OK)
    {
        m_bLoaded = false;
    }

    long inputLatency;
    long outputLatency;

    // It is possible that there will not be valid latencies until after ASIO buffer creation
    if (ASIOGetLatencies(&inputLatency, &outputLatency) != ASE_OK)
    {
        m_bLoaded = false;

        return m_bLoaded;
    }

    Open(inputChannels, outputChannels, minSize, maxSize, preferredSize, granularity, inputLatency, outputLatency, sampleRate);

    return m_bLoaded;
}

uint64_t LTWindowsASIODriver::GetTime(void)
{
    return timeGetTime();
}

// ASIO SDK Callbacks
void LTWindowsASIODriver::AsioCallbackBufferSwitch(long index, ASIOBool processNow)
{
    ASIOTime  timeInfo;
    memset(&timeInfo, 0, sizeof(timeInfo));

    LTWindowsASIO::GetLockedLTWindowsAsio();

    if (ASIOGetSamplePosition(&timeInfo.timeInfo.samplePosition, &timeInfo.timeInfo.systemTime) == ASE_OK)
        timeInfo.timeInfo.flags = kSystemTimeValid | kSamplePositionValid;

    LTWindowsASIO::UnlockLTWindowsAsio();

    AsioCallbackbufferSwitchTimeInfo(&timeInfo, index, processNow);
}

void LTWindowsASIODriver::AsioCallbackSampleRateDidChange(ASIOSampleRate sampleRate)
{

}

// This function is largely taken from the ASIO SDK 2.3 host sample to preserve documentation
long LTWindowsASIODriver::AsioCallbackAsioMessages(long selector, long value, void* message, double* opt)
{
    long ret = 0;
    switch (selector)
    {
        case kAsioSelectorSupported:
            if (value == kAsioResetRequest
                || value == kAsioEngineVersion
                || value == kAsioResyncRequest
                || value == kAsioLatenciesChanged
                // the following three were added for ASIO 2.0, you don't necessarily have to support them
                || value == kAsioSupportsTimeInfo
                || value == kAsioSupportsTimeCode
                || value == kAsioSupportsInputMonitor)
                ret = 1L;
            break;
        case kAsioResetRequest:
            // defer the task and perform the reset of the driver during the next "safe" situation
            // You cannot reset the driver right now, as this code is called from the driver.
            // Reset the driver is done by completely destruct is. I.e. ASIOStop(), ASIODisposeBuffers(), Destruction
            // Afterwards you initialize the driver again.
            ret = 1L;
            break;
        case kAsioResyncRequest:
            // This informs the application, that the driver encountered some non fatal data loss.
            // It is used for synchronization purposes of different media.
            // Added mainly to work around the Win16Mutex problems in Windows 95/98 with the
            // Windows Multimedia system, which could loose data because the Mutex was hold too long
            // by another thread.
            // However a driver can issue it in other situations, too.
            ret = 1L;
            break;
        case kAsioLatenciesChanged:
            // This will inform the host application that the drivers were latencies changed.
            // Beware, it this does not mean that the buffer sizes have changed!
            // You might need to update internal delay data.
            ret = 1L;
            break;
        case kAsioEngineVersion:
            // return the supported ASIO version of the host application
            // If a host applications does not implement this selector, ASIO 1.0 is assumed
            // by the driver
            ret = 2L;
            break;
        case kAsioSupportsTimeInfo:
            // informs the driver wether the asioCallbacks.bufferSwitchTimeInfo() callback
            // is supported.
            // For compatibility with ASIO 1.0 drivers the host application should always support
            // the "old" bufferSwitch method, too.
            ret = 1;
            break;
        case kAsioSupportsTimeCode:
            // informs the driver wether application is interested in time code info.
            // If an application does not need to know about time code, the driver has less work
            // to do.
            ret = 0;
            break;
    }

    return ret;
}

// The ASIO64toDouble macro is taken from the ASIO SDK 2.3 host sample
const double twoRaisedTo32 = 4294967296.;
#define ASIO64toDouble(a)  ((a).lo + (a).hi * twoRaisedTo32)

ASIOTime* LTWindowsASIODriver::AsioCallbackbufferSwitchTimeInfo(ASIOTime* timeInfo, long index, ASIOBool processNow)
{
    LTWindowsASIO* ltWindowsAsio = LTWindowsASIO::GetLockedLTWindowsAsio();
    LTWindowsASIODriver* driver = ltWindowsAsio->GetDriver();

    driver->m_TimeInfo = *timeInfo;

    if (timeInfo->timeInfo.flags & kSystemTimeValid)
    {
        driver->m_fNanoSeconds = ASIO64toDouble(timeInfo->timeInfo.systemTime);
    }
    else
    {
        driver->m_fNanoSeconds = 0;
    }

    if (timeInfo->timeInfo.flags & kSamplePositionValid)
    {
        driver->m_fSamples = ASIO64toDouble(timeInfo->timeInfo.samplePosition);
    }
    else
    {
        driver->m_fSamples = 0;
    }

    if (timeInfo->timeCode.flags & kTcValid)
    {
        driver->m_fTcSamples = ASIO64toDouble(timeInfo->timeCode.timeCodeSamples);
    }
    else
    {
        driver->m_fTcSamples = 0;
    }

    driver->m_uSystemRefrenceTime = driver->GetTime();

    // finally if the driver supports the ASIOOutputReady() optimization, do it here, all data are in place
    if (driver->m_bPostOutput)
        ASIOOutputReady();

    LTWindowsASIO::UnlockLTWindowsAsio();

    return 0;
}

