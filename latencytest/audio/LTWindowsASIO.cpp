#include "LTWindowsASIO.h"

#include "iasiodrv.h"

// The ASIO64toDouble macro is taken from the ASIO SDK 2.3 host sample
const double twoRaisedTo32 = 4294967296.;
#define ASIO64toDouble(a)  ((a).lo + (a).hi * twoRaisedTo32)

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
    , m_pChannelInfos(nullptr)
    , m_bLoaded(false)
    , m_fNanoSeconds(0.0)
    , m_fSamples(0.0)
    , m_fTcSamples(0.0)
    , m_uSystemRefrenceTime(0)
    , m_bPostOutput(false)
    , m_pInputSamples(nullptr)
    , m_pOutputSamples(nullptr)
    , m_iSignalDetectedTimerInputChannel(-1)
    , m_iNoiseFloorDetectChannel(-1)
{
    // Start this out locked, we unlock it once we have detected the noise floor in response to a request
    m_NoiseFloorDetectedMutex.lock();
}

LTWindowsASIODriver::~LTWindowsASIODriver(void)
{
    delete[] m_pBufferInfos;
    delete[] m_pChannelInfos;

    if (m_pAsioDrivers != nullptr)
    {
        delete m_pAsioDrivers;
    }

    if (m_pInputSamples != nullptr)
    {
        delete[] m_pInputSamples;
    }

    if (m_pOutputSamples != nullptr)
    {
        delete[] m_pOutputSamples;
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
    driverInfo.asioVersion = 2;
    driverInfo.sysRef = GetForegroundWindow();

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

    m_NoiseFloors.clear();

    int totalChannels = inputChannels + outputChannels;
    m_pBufferInfos = new ASIOBufferInfo[totalChannels];
    m_pChannelInfos = new ASIOChannelInfo[totalChannels];

    for (int idx = 0; idx < totalChannels; idx++)
    {
        if (idx < inputChannels)
        {
            m_pBufferInfos[idx].isInput = ASIOTrue;
            m_pBufferInfos[idx].channelNum = idx;
            m_NoiseFloors.append(0.005);
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
        return m_bLoaded;
    }

    for (int idx = 0; idx < totalChannels; idx++)
    {
        m_pChannelInfos[idx].channel = m_pBufferInfos[idx].channelNum;
        m_pChannelInfos[idx].isInput = m_pBufferInfos[idx].isInput;

        if (ASIOGetChannelInfo(&m_pChannelInfos[idx]) != ASE_OK)
        {
            m_bLoaded = false;
            return m_bLoaded;
        }
    }

    long inputLatency;
    long outputLatency;

    // It is possible that there will not be valid latencies until after ASIO buffer creation
    if (ASIOGetLatencies(&inputLatency, &outputLatency) != ASE_OK)
    {
        m_bLoaded = false;

        return m_bLoaded;
    }

    m_pInputSamples = new double[inputChannels * preferredSize];
    m_pOutputSamples = new double[outputChannels * preferredSize];

    Open(inputChannels, outputChannels, minSize, maxSize, preferredSize, granularity, inputLatency, outputLatency, sampleRate);

    if (ASIOStart() != ASE_OK)
    {
        m_bLoaded = false;

        return m_bLoaded;
    }

    return m_bLoaded;
}

uint64_t LTWindowsASIODriver::GetTime(void)
{
    return timeGetTime();
}

QString LTWindowsASIODriver::GetChannelName(int index)
{
    if (m_pChannelInfos == nullptr)
    {
        return QString("");
    }

    int displayChannel = m_pChannelInfos[index].channel + 1;
    
    const char* channelName = m_pChannelInfos[index].name;

    if (channelName == nullptr)
    {
        return QString("%1").arg(displayChannel);
    }

    return QString("%1: %2").arg(displayChannel).arg(channelName);
}

void LTWindowsASIODriver::CancelSignalDetection(void) 
{
    if(m_iSignalDetectedTimerInputChannel >= 0)
    {
        m_iSignalDetectedTimerInputChannel = -2;
    }

    if(m_iNoiseFloorDetectChannel >= 0)
    {
        m_iNoiseFloorDetectChannel = -2;
    }
}

void LTWindowsASIODriver::StartSignalDetectTimer(int inputChannel)
{
    m_SignalDetectedMutex.lock();

    m_iSignalDetectedTimerInputChannel = inputChannel;

    if (m_SignalDetectedTimer.isValid())
    {
        m_SignalDetectedTimer.restart();
    }
    else
    {
        m_SignalDetectedTimer.start();
    }
}

int64_t LTWindowsASIODriver::WaitForSignalDetected(void)
{
    m_SignalDetectedMutex.lock();

    m_SignalDetectedMutex.unlock();

    if (m_iSignalDetectedTimerInputChannel == -2)
    {
        m_iSignalDetectedTimerInputChannel = -1;
        return -1;
    }

    return m_iSignalDetectedNsecsElapsed;
}

bool LTWindowsASIODriver::DetectNoiseFloor(int inputChannel)
{
    if(m_iNoiseFloorDetectChannel >= 0)
    {
        return false;
    }

    m_NoiseFloorDetectionTimer.restart();

    m_iNoiseFloorDetectChannel = inputChannel;

    m_NoiseFloorDetectedMutex.lock();

    if(m_iNoiseFloorDetectChannel == -2)
    {
        m_iNoiseFloorDetectChannel = -1;
        return false;
    }

    return true;
}

void LTWindowsASIODriver::AsioCallbackBufferSwitch_Internal(long index, ASIOBool processNow)
{
    ASIOTime  timeInfo;
    memset(&timeInfo, 0, sizeof(timeInfo));

    if (ASIOGetSamplePosition(&timeInfo.timeInfo.samplePosition, &timeInfo.timeInfo.systemTime) == ASE_OK)
        timeInfo.timeInfo.flags = kSystemTimeValid | kSamplePositionValid;

    AsioCallbackbufferSwitchTimeInfo_Internal(&timeInfo, index, processNow);
}

ASIOTime* LTWindowsASIODriver::AsioCallbackbufferSwitchTimeInfo_Internal(ASIOTime* timeInfo, long index, ASIOBool processNow)
{
    m_TimeInfo = *timeInfo;

    if (timeInfo->timeInfo.flags & kSystemTimeValid)
    {
        m_fNanoSeconds = ASIO64toDouble(timeInfo->timeInfo.systemTime);
    }
    else
    {
        m_fNanoSeconds = 0;
    }

    if (timeInfo->timeInfo.flags & kSamplePositionValid)
    {
        m_fSamples = ASIO64toDouble(timeInfo->timeInfo.samplePosition);
    }
    else
    {
        m_fSamples = 0;
    }

    if (timeInfo->timeCode.flags & kTcValid)
    {
        m_fTcSamples = ASIO64toDouble(timeInfo->timeCode.timeCodeSamples);
    }
    else
    {
        m_fTcSamples = 0;
    }

    m_uSystemRefrenceTime = GetTime();

    //if (m_bPostOutput)
    //    ASIOOutputReady();

    if (processNow)
    {
        ProcessSignal(index);
    }

    return 0L;
}

void LTWindowsASIODriver::ProcessSignal(long index)
{
    if(m_iNoiseFloorDetectChannel >= 0)
    {
        ConvertSampleToNative(m_pChannelInfos[m_iNoiseFloorDetectChannel].type, m_pBufferInfos[m_iNoiseFloorDetectChannel].buffers[index], ASIOSTFloat64LSB, m_pInputSamples);

        for(int idx = 0; idx < GetPreferredSize(); idx++)
        {
            int inputChannel = m_iNoiseFloorDetectChannel;
            double currentNoiseFloor = m_NoiseFloors.at(inputChannel);
            m_NoiseFloors.replace(inputChannel, max(abs(currentNoiseFloor), abs(m_pInputSamples[idx])));
        }

        static float noiseFloorDetectTime = (1.0 * 1000.0); // 1 Second in milliseconds

        if(m_NoiseFloorDetectionTimer.elapsed() > noiseFloorDetectTime)
        {
            m_iNoiseFloorDetectChannel = -1;
            m_NoiseFloorDetectedMutex.unlock();
        }
    }
    else if (m_iNoiseFloorDetectChannel == -2)
    {
        m_NoiseFloorDetectedMutex.unlock();
    }
    else if (m_iSignalDetectedTimerInputChannel >= 0)
    {
        m_iSignalDetectedNsecsElapsed = m_SignalDetectedTimer.nsecsElapsed();

        ConvertSampleToNative(m_pChannelInfos[m_iSignalDetectedTimerInputChannel].type, m_pBufferInfos[m_iSignalDetectedTimerInputChannel].buffers[index], ASIOSTFloat64LSB, m_pInputSamples);

        double threshold = m_NoiseFloors.at(m_iSignalDetectedTimerInputChannel);

        static double margin = 0.25;

        threshold = threshold + (threshold * margin);

        for (int idx = 0; idx < GetPreferredSize(); idx++)
        {   
            if (abs(m_pInputSamples[idx]) > threshold)
            {
                m_iSignalDetectedTimerInputChannel = -1;

                m_SignalDetectedTimer.invalidate();
                m_SignalDetectedMutex.unlock();

                break;
            }
        }
    }
    else if(m_iSignalDetectedTimerInputChannel == -2)
    {
        m_SignalDetectedTimer.invalidate();
        m_SignalDetectedMutex.unlock();
    }
}

// ASIO SDK Callbacks
void LTWindowsASIODriver::AsioCallbackBufferSwitch(long index, ASIOBool processNow)
{
    LTWindowsASIO* ltWindowsAsio = LTWindowsASIO::GetLockedLTWindowsAsio();
    LTWindowsASIODriver* driver = ltWindowsAsio->GetDriver();

    driver->AsioCallbackBufferSwitch_Internal(index, processNow);

    LTWindowsASIO::UnlockLTWindowsAsio();
}

void LTWindowsASIODriver::AsioCallbackSampleRateDidChange(ASIOSampleRate sampleRate)
{
    Sleep(1);
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

ASIOTime* LTWindowsASIODriver::AsioCallbackbufferSwitchTimeInfo(ASIOTime* timeInfo, long index, ASIOBool processNow)
{
    LTWindowsASIO* ltWindowsAsio = LTWindowsASIO::GetLockedLTWindowsAsio();
    LTWindowsASIODriver* driver = ltWindowsAsio->GetDriver();

    ASIOTime* retTime = driver->AsioCallbackbufferSwitchTimeInfo_Internal(timeInfo, index, processNow);

    LTWindowsASIO::UnlockLTWindowsAsio();

    return retTime;
}

// The following two functions are taken from RTAudio with minor modifications
// It would be preferable to have our own version here, but for now it's more important to do it correctly using RTAudio's
void LTWindowsASIODriver::ConvertSampleToNative(ASIOSampleType inputType, void* inputBuffer, ASIOSampleType outputType, void* outputBuffer)
{
    switch (outputType)
    {
    case ASIOSTFloat64MSB:
    case ASIOSTFloat64LSB:
    {
        double scale;
        double *out = (double *)outputBuffer;

        switch (inputType)
        {
        case ASIOSTInt16MSB:
        case ASIOSTInt16LSB:
        {
            int16_t *in = (int16_t *)inputBuffer;
            scale = 1.0 / 32767.5;
            for (int i = 0; i<GetPreferredSize(); i++)
            {
                *out = *((double*)in);
                *out += 0.5;
                *out *= scale;

                in++;
                out++;
            }
            break;
        }
        case ASIOSTInt24MSB:
        case ASIOSTInt24LSB:
        {
            LTS24 *in = (LTS24 *)inputBuffer;
            scale = 1.0 / 8388607.5;
            for (int i = 0; i<GetPreferredSize(); i++)
            {
                *out = (double)in->asInt();
                *out += 0.5;
                *out *= scale;

                in++;
                out++;
            }

            break;
        }
        case ASIOSTInt32MSB:
        case ASIOSTInt32LSB:
        {
            int32_t *in = (int32_t *)inputBuffer;
            scale = 1.0 / 2147483647.5;
            for (int i = 0; i<GetPreferredSize(); i++)
            {
                *out = *in;
                *out += 0.5;
                *out *= scale;

                in++;
                out++;
            }

            break;
        }
        case ASIOSTFloat32MSB:
        case ASIOSTFloat32LSB:
        {
            float *in = (float *)inputBuffer;
            for (int i = 0; i<GetPreferredSize(); i++)
            {
                *out = *in;

                in++;
                out++;
            }

            break;
        }
        case ASIOSTFloat64MSB:
        case ASIOSTFloat64LSB:
        {
            // Channel compensation and/or (de)interleaving only.
            double *in = (double *)inputBuffer;
            for (int i = 0; i<GetPreferredSize(); i++) {
                *out = *in;

                in++;
                out++;
            }

            break;
        }
        }

        break;
    }
    case ASIOSTFloat32MSB:
    case ASIOSTFloat32LSB:
    {
        float scale;
        float *out = (float *)outputBuffer;

        switch (inputType)
        {
        case ASIOSTInt16MSB:
        case ASIOSTInt16LSB:
        {
            int16_t *in = (int16_t *)inputBuffer;
            scale = (float)(1.0 / 32767.5);
            for (int i = 0; i<GetPreferredSize(); i++)
            {
                *out = *in;
                *out += 0.5;
                *out *= scale;

                in++;
                out++;
            }

            break;
        }
        case ASIOSTInt24MSB:
        case ASIOSTInt24LSB:
        {
            LTS24 *in = (LTS24 *)inputBuffer;
            scale = (float)(1.0 / 8388607.5);
            for (int i = 0; i<GetPreferredSize(); i++)
            {
                *out = in->asInt();
                *out += 0.5;
                *out *= scale;

                in++;
                out++;
            }

            break;
        }
        case ASIOSTInt32MSB:
        case ASIOSTInt32LSB:
        {
            int32_t *in = (int32_t *)inputBuffer;
            scale = (float)(1.0 / 2147483647.5);
            for (int i = 0; i<GetPreferredSize(); i++)
            {
                *out = *in;
                *out += 0.5;
                *out *= scale;

                in++;
                out++;
            }

            break;
        }
        case ASIOSTFloat32MSB:
        case ASIOSTFloat32LSB:
        {
            // Channel compensation and/or (de)interleaving only.
            float *in = (float *)inputBuffer;
            for (int i = 0; i<GetPreferredSize(); i++)
            {
                *out = *in;

                in++;
                out++;
            }

            break;
        }
        case ASIOSTFloat64MSB:
        case ASIOSTFloat64LSB:
        {
            double *in = (double *)inputBuffer;
            for (int i = 0; i<GetPreferredSize(); i++)
            {
                *out = *in;

                in++;
                out++;
            }

            break;
        }
        }

        break;
    }
    case ASIOSTInt32MSB:
    case ASIOSTInt32LSB:
    {
        int32_t *out = (int32_t *)outputBuffer;

        switch (inputType)
        {
        case ASIOSTInt16MSB:
        case ASIOSTInt16LSB:
        {
            int16_t *in = (int16_t *)inputBuffer;
            for (int i = 0; i<GetPreferredSize(); i++)
            {
                *out = *in;
                *out <<= 16;

                in++;
                out++;
            }

            break;
        }
        case ASIOSTInt24MSB:
        case ASIOSTInt24LSB:
        {
            LTS24 *in = (LTS24 *)inputBuffer;
            for (int i = 0; i<GetPreferredSize(); i++)
            {
                *out = in->asInt();
                *out <<= 8;

                in++;
                out++;
            }

            break;
        }
        case ASIOSTInt32MSB:
        case ASIOSTInt32LSB:
        {
            // Channel compensation and/or (de)interleaving only.
            int32_t *in = (int32_t *)inputBuffer;
            for (int i = 0; i<GetPreferredSize(); i++)
            {
                *out = *in;

                in++;
                out++;
            }

            break;
        }
        case ASIOSTFloat32MSB:
        case ASIOSTFloat32LSB:
        {
            float *in = (float *)inputBuffer;
            for (int i = 0; i<GetPreferredSize(); i++)
            {
                *out = (int32_t)(*in * 2147483647.5 - 0.5);

                in++;
                out++;
            }

            break;
        }
        case ASIOSTFloat64MSB:
        case ASIOSTFloat64LSB:
        {
            double *in = (double *)inputBuffer;
            for (int i = 0; i<GetPreferredSize(); i++)
            {
                *out = (int32_t)(*in * 2147483647.5 - 0.5);

                in++;
                out++;
            }

            break;
        }
        }

        break;
    }
    case ASIOSTInt24MSB:
    case ASIOSTInt24LSB:
    {
        LTS24 *out = (LTS24 *)outputBuffer;

        switch (inputType)
        {
        case ASIOSTInt16MSB:
        case ASIOSTInt16LSB:
        {
            int16_t *in = (int16_t *)inputBuffer;
            for (int i = 0; i<GetPreferredSize(); i++)
            {
                *out = (int32_t)(*in << 8);
                //*out <<= 8;

                in++;
                out++;
            }

            break;
        }
        case ASIOSTInt24MSB:
        case ASIOSTInt24LSB:
        {
            // Channel compensation and/or (de)interleaving only.
            LTS24 *in = (LTS24 *)inputBuffer;
            for (int i = 0; i<GetPreferredSize(); i++)
            {
                *out = *in;

                in++;
                out++;
            }

            break;
        }
        case ASIOSTInt32MSB:
        case ASIOSTInt32LSB:
        {
            int32_t *in = (int32_t *)inputBuffer;
            for (int i = 0; i<GetPreferredSize(); i++)
            {
                *out = (int32_t)(*in >> 8);
                //*out >>= 8;

                in++;
                out++;
            }

            break;
        }
        case ASIOSTFloat32MSB:
        case ASIOSTFloat32LSB:
        {
            float *in = (float *)inputBuffer;
            for (int i = 0; i<GetPreferredSize(); i++)
            {
                *out = (int32_t)(*in * 8388607.5 - 0.5);

                in++;
                out++;
            }

            break;
        }
        case ASIOSTFloat64MSB:
        case ASIOSTFloat64LSB:
        {

            double *in = (double *)inputBuffer;
            for (int i = 0; i<GetPreferredSize(); i++)
            {
                *out = (int32_t)(*in * 8388607.5 - 0.5);

                in++;
                out++;
            }

            break;
        }
        }

        break;
    }
    case ASIOSTInt16MSB:
    case ASIOSTInt16LSB:
    {
        int16_t *out = (int16_t *)outputBuffer;

        switch (inputType)
        {
        case ASIOSTInt16MSB:
        case ASIOSTInt16LSB:
        {
            // Channel compensation and/or (de)interleaving only.
            int16_t *in = (int16_t *)inputBuffer;
            for (int i = 0; i<GetPreferredSize(); i++)
            {
                *out = *in;

                in++;
                out++;
            }

            break;
        }
        case ASIOSTInt24MSB:
        case ASIOSTInt24LSB:
        {
            LTS24 *in = (LTS24 *)inputBuffer;
            for (int i = 0; i<GetPreferredSize(); i++)
            {
                *out = (int16_t)(in->asInt() >> 8);

                in++;
                out++;
            }

            break;
        }
        case ASIOSTInt32MSB:
        case ASIOSTInt32LSB:
        {
            int32_t *in = (int32_t *)inputBuffer;
            for (int i = 0; i<GetPreferredSize(); i++)
            {
                *out = (int16_t)((*in >> 16) & 0x0000ffff);

                in++;
                out++;
            }

            break;
        }
        case ASIOSTFloat32MSB:
        case ASIOSTFloat32LSB:
        {
            float *in = (float *)inputBuffer;
            for (int i = 0; i<GetPreferredSize(); i++)
            {
                *out = (int16_t)(*in * 32767.5 - 0.5);

                in++;
                out++;
            }

            break;
        }
        case ASIOSTFloat64MSB:
        case ASIOSTFloat64LSB:
        {
            double *in = (double *)inputBuffer;
            for (int i = 0; i<GetPreferredSize(); i++) {
                *out = (int16_t)(*in * 32767.5 - 0.5);

                in++;
                out++;
            }

            break;
        }
        }

        break;
    }
    }

    return;
}

#if 0
void LTWindowsASIODriver::byteSwapBuffer(char *buffer, unsigned int samples, RtAudioFormat format)
{
    char val;
    char *ptr;

    ptr = buffer;
    if (format == RTAUDIO_Sint16_t) {
        for (unsigned int i = 0; i<samples; i++) {
            // Swap 1st and 2nd bytes.
            val = *(ptr);
            *(ptr) = *(ptr + 1);
            *(ptr + 1) = val;

            // Increment 2 bytes.
            ptr += 2;
        }
    }
    else if (format == RTAUDIO_Sint32_t ||
        format == RTAUDIO_float) {
        for (unsigned int i = 0; i<samples; i++) {
            // Swap 1st and 4th bytes.
            val = *(ptr);
            *(ptr) = *(ptr + 3);
            *(ptr + 3) = val;

            // Swap 2nd and 3rd bytes.
            ptr += 1;
            val = *(ptr);
            *(ptr) = *(ptr + 1);
            *(ptr + 1) = val;

            // Increment 3 more bytes.
            ptr += 3;
        }
    }
    else if (format == RTAUDIO_SLTS24) {
        for (unsigned int i = 0; i<samples; i++) {
            // Swap 1st and 3rd bytes.
            val = *(ptr);
            *(ptr) = *(ptr + 2);
            *(ptr + 2) = val;

            // Increment 2 more bytes.
            ptr += 2;
        }
    }
    else if (format == RTAUDIO_double) {
        for (unsigned int i = 0; i<samples; i++) {
            // Swap 1st and 8th bytes
            val = *(ptr);
            *(ptr) = *(ptr + 7);
            *(ptr + 7) = val;

            // Swap 2nd and 7th bytes
            ptr += 1;
            val = *(ptr);
            *(ptr) = *(ptr + 5);
            *(ptr + 5) = val;

            // Swap 3rd and 6th bytes
            ptr += 1;
            val = *(ptr);
            *(ptr) = *(ptr + 3);
            *(ptr + 3) = val;

            // Swap 4th and 5th bytes
            ptr += 1;
            val = *(ptr);
            *(ptr) = *(ptr + 1);
            *(ptr + 1) = val;

            // Increment 5 more bytes.
            ptr += 5;
        }
    }
}
#endif