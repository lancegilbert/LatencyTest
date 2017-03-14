#ifndef _LTAUDIODEVICE_H_
#define _LTAUDIODEVICE_H_

#include <QString>

class LTAudioDevice
{
public:
    LTAudioDevice();
    virtual ~LTAudioDevice();

    virtual bool Initialize(int deviceID, QString name);

    int GetDeviceID(void) { return m_iDeviceID; }
    QString GetName(void) { return m_sName; }

protected:
    int m_iDeviceID;
    QString m_sName;
};

#endif /* _LTAUDIODEVICE_H_ */