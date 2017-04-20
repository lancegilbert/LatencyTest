#ifndef _LTAUDIOTHREADS_H_
#define _LTAUDIOTHREADS_H_

#include <QThread>
#include <QTime>

struct LTSignalDetectThreadResult
{
	int rowIdx;
	bool signalDetected;
	double midiLatency;
	double totalLatency;
};

struct LTSignalDetectThreadParameters
{
	QObject *parent;
	int rowIdx;
	class LTMIDI* midiDevice;
	class LTWindowsASIODriver* audioDriver;
	int midiOutDeviceId;
	int midiOutChannel;
	int asioInputChannel;
	int testCount;
	float testsPerSecond;
};

class LTSignalDetectThread : public QThread
{
	Q_OBJECT
public:
	explicit LTSignalDetectThread(LTSignalDetectThreadParameters params);

protected:
	virtual void run();

signals:
	void Completed(LTSignalDetectThreadResult result);

private:
	int m_iRowIdx;
	class LTMIDI* m_pMidiDevice;
	class LTWindowsASIODriver* m_pAudioDriver;
	int m_iMidiOutDeviceId;
	int m_iMidiOutChannel;
	int m_iAsioInputChannel;
	int m_iTestCount;
	float m_fTestsPerSecond;
};

#endif /* _LTAUDIOTHREADS_H_ */
