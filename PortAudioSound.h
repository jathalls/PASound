#pragma once
#include <portaudio.h>
#include <sndfile.hh>
#include <string.h>
#include <iostream>
#include <stdio.h>
#include "fftw3.h"
#include <math.h>
#include "ChildView.h"

using namespace std;

class PortAudioSound
{

protected:
	void error(int errorCode);
	SNDFILE *outfile;
	SF_INFO sfinfo;
	bool recordflag;
	CStdioFile logFile;
	void ListDevices();
	double *FFTin;
	double *FFTout;
	double *hamming;
	fftw_plan FFTplan;
	void CreateHammingProfile(double *buffer);

	double mini;
	double maxi;
	int currentColumn;
	int currentRow;
	

public:
	PortAudioSound();
	~PortAudioSound();

	HWND ChildWindowHwnd;
	double **ppPowSpect;
	int numBuffers;
	int ReadIndex;
	int WriteIndex;
	// Starts the portaudio stream that was created by the constructor
	int StartStream();
	// stops the portaudio stream
	int StopStream();
	// starts recording to a file as defined by the class parameters
	int StartRecord();
	// stops recording to the file and closes it with appropriate log entries
	int StopRecord();
	// Records to a file for the specified number of minutes, with appropriate log entries
	int RecordMinutes(int minutes);
	// Turns on the built-in bat detector
	int BatDetectorOn();
	// turns off the built in bat detector
	int BatDetectorOff();
	// changes the bat detectcor tuning by the indicated amount (in kHz)
	int Tune(int offset);
	static int myPaCallback(const void * input, void * output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void * userData){
		return ((PortAudioSound*) userData)->myMemberCallback(input, output, frameCount, timeInfo, statusFlags);
	}
	int myMemberCallback(const void * input, void * output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags);
	bool PortAudioSound::DrawSpectrogram(CDC *pDC,CRect *pDrawing_Area);

protected:
	PaStream* pStream;
	int iSampleRate;
};

