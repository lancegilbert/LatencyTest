#ifndef _LTAUDIODRIVER_H_
#define _LTAUDIODRIVER_H_

#include <QString>

class LTAudioDriver
{
public:
    LTAudioDriver();
    virtual ~LTAudioDriver();

    virtual bool Initialize(int driverID, QString name);
    virtual bool Open(int inputChannels, int outputChannels, int minSize, int maxSize, int preferredSize, int granularity, int inputLatency, int outputLatency, double sampleRate);

    bool GetIsOpen(void) { return m_bOpen; }

    int GetDriverID(void) { return m_iDriverID; }
    QString GetName(void) { return m_sName; }

    int GetNumInputChannels(void) { return m_iInputChannels; }
    int GetNumOutputChannels(void) { return m_iOutputChannels; }
    int GetMinSize(void) { return m_iMinSize; }
    int GetMaxSize(void) { return m_iMaxSize; }
    int GetPreferredSize(void) { return m_iPreferredSize; }
    int GetGranularity(void) { return m_iGranularity; }
    int GetInputLatency(void) { return m_iInputLatency; }
    int GetOutputLatency(void) { return m_iOutputLatency; }
    float GetSampleRate(void) { return m_fSampleRate; }

protected:
    bool m_bOpen;

    int m_iDriverID;
    QString m_sName;

    int m_iInputChannels;
    int m_iOutputChannels;
    int m_iMinSize;
    int m_iMaxSize;
    int m_iPreferredSize;
    int m_iGranularity;
    int m_iInputLatency;
    int m_iOutputLatency;
    double m_fSampleRate;
};

#endif /* _LTAUDIODRIVER_H_ */