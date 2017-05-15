#include "LTAudioThreads.h"

#include "audio/LTWindowsASIO.h"
#include "midi/LTMIDIDevice.h"

LTSignalDetectThread::LTSignalDetectThread(LTSignalDetectThreadParameters params)
	: QThread(params.parent)
	, m_iRowIdx(params.rowIdx)
	, m_pMidiDevice(params.midiDevice)
	, m_pAudioDriver(params.audioDriver)
	, m_iMidiOutDeviceId(params.midiOutDeviceId)
	, m_iMidiOutChannel(params.midiOutChannel)
	, m_iAsioInputChannel(params.asioInputChannel)
	, m_iTestCount(params.testCount)
	, m_fTestsPerSecond(params.testsPerSecond)
{
	static bool metaTypesRegistered = false;
	if(!metaTypesRegistered)
	{
		qRegisterMetaType<LTSignalDetectThreadResult>();
		metaTypesRegistered = true;
	}
} 

// Taken from http://stackoverflow.com/a/19695285
template <typename It>
typename std::iterator_traits<It>::value_type Median(It begin, It end)
{
	auto size = std::distance(begin, end);
	std::nth_element(begin, begin + size / 2, end);
	return *std::next(begin, size / 2);
}

void LTSignalDetectThread::run()
{
	LTSignalDetectThreadResult result;
	result.rowIdx = m_iRowIdx;
	result.signalDetected = false;
	result.iterationsComplete = 0;
	
	LTMIDIOutDevice* device = m_pMidiDevice->GetOutDevice(m_iMidiOutDeviceId);

	if (!device->OpenDevice())
	{
		emit Completed(result);
		return;
	}
	
	if (!m_pAudioDriver->DetectNoiseFloor(m_iAsioInputChannel))
	{
		device->CloseDevice();
		emit Completed(result);
		return;
	}
	else
	{
		std::vector<double> elapsedValues;

		double inputLatency = (m_pAudioDriver->GetInputLatency() / m_pAudioDriver->GetSampleRate()) * 1000.0f;

		for (int idx = 0; idx < m_iTestCount; idx++)
		{
			m_pAudioDriver->StartSignalDetectTimer(m_iAsioInputChannel);

			if (!device->SendMIDINote(LTMIDI_Command_NoteOn, m_iMidiOutChannel, LTMIDI_Note_C, 3, 0x40))
			{
				device->CloseDevice();
				emit Completed(result);
				return;
			}

			int64_t nsecsElapsed = m_pAudioDriver->WaitForSignalDetected();

			if (nsecsElapsed == -1)
			{
				device->CloseDevice();
				emit Completed(result);
				return;
			}

			if (!device->SendMIDINote(LTMIDI_Command_NoteOffRunning, m_iMidiOutChannel, LTMIDI_Note_C, 3, 0x00))
			{
				device->CloseDevice();
				emit Completed(result);
				return;
			}

			double msecsElapsed = (double)nsecsElapsed / 1000000.0;

			elapsedValues.push_back(msecsElapsed);

			// Make sure to clamp delay to a value above 0.0f to prevent msleep deadlock while debugging due to long msecsElapsed
			float delay = max(((1.0f / m_fTestsPerSecond) * 1000.0f) - msecsElapsed, 0.0f);

			double averageMsecsElapsed = Median(elapsedValues.begin(), elapsedValues.end());
			double midiLatency = averageMsecsElapsed - inputLatency;

			result.signalDetected = true;
			result.midiLatency = midiLatency;
			result.totalLatency = averageMsecsElapsed;
			result.iterationsComplete++;

			emit IterationCompleted(result);

			result.signalDetected = false;

			QThread::msleep((unsigned long)delay);
		}

		device->CloseDevice();

		double averageMsecsElapsed = Median(elapsedValues.begin(), elapsedValues.end());
		double midiLatency = averageMsecsElapsed - inputLatency;

		result.signalDetected = true;
		result.midiLatency = midiLatency;
		result.totalLatency = averageMsecsElapsed;
	}

	emit Completed(result);
}