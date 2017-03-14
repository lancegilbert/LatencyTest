#ifndef _LTAUDIODEVICE_H_
#define _LTAUDIODEVICE_H_

#include <QString>

class LTAudioDevice
{
public:
    LTAudioDevice();
    virtual ~LTAudioDevice();

    virtual bool Initialize(int deviceID, int MID, int PID, int driverVersion, QString name);

    int GetDeviceID(void) { return m_iDeviceID; }
    int GetMID(void) { return m_iMID; }
    int GetPID(void) { return m_iPID; }
    int GetDriverVersion(void) { return m_iDriverVersion; }
    QString GetName(void) { return m_sName; }

protected:
    int m_iDeviceID;

    int m_iMID;
    int m_iPID;
    int m_iDriverVersion;
    QString m_sName;
};

#endif /* _LTAUDIODEVICE_H_ */