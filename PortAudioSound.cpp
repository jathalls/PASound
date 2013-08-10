#include "stdafx.h"
#include "PortAudioSound.h"


PortAudioSound::PortAudioSound()
{
	error(Pa_Initialize());
	
	iSampleRate = 192000;
	recordflag = false;
	
	if (!logFile.Open(L"C:/Bat Recordings/TestFileLog.txt", CFile::modeCreate | CFile::modeWrite | CFile::typeText)){
		OutputDebugString(L"Unable to open Log File\n");
	}

	numBuffers = 16;
	ReadIndex = 0;
	WriteIndex = 0;
	
	FFTin = (double *) fftw_malloc(sizeof(double) * 514);
	FFTout = (double *) fftw_malloc(sizeof(double) * 512);
	hamming = (double *) fftw_malloc(sizeof(double) * 512);
	CreateHammingProfile(hamming);
	FFTplan = fftw_plan_r2r_1d(512, FFTin, FFTout, FFTW_DHT, FFTW_ESTIMATE);
	ppPowSpect = (double **) fftw_malloc(sizeof(double *) * numBuffers);
	for (int i = 0; i < numBuffers; i++){
		ppPowSpect[i] = (double *) fftw_malloc(sizeof(double) * 513);
	}
	//FFTplan = fftw_plan_dft_r2c_1d(512, FFTin, FFTout,  FFTW_ESTIMATE);
	//Pspect = (double *) fftw_malloc(sizeof(double) * 512);

	ListDevices();

	PaStreamParameters inputParameters;

	inputParameters.device = 9;
	inputParameters.channelCount = 1;
	inputParameters.sampleFormat = paInt16;
	
	inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
	inputParameters.hostApiSpecificStreamInfo = NULL;
	int err = Pa_IsFormatSupported(&inputParameters, NULL, 192000);
	if (err==0){
		logFile.WriteString(L"Input format is supported\n");
	}
	else{
		logFile.WriteString(L"Input format NOT supported\n");
	}



	error(Pa_OpenStream(
		&pStream,
		&inputParameters, /* input channel*/
		NULL, /* output channel*/
		Pa_GetDeviceInfo(inputParameters.device)->defaultSampleRate,
		2048,
		paClipOff,
		&PortAudioSound::myPaCallback,
		this
		));

	logFile.WriteString(L"Stream opened\n");
	const PaDeviceInfo* info= Pa_GetDeviceInfo(9);
	CString lf;
	lf.Format(L"Max Input Channels=%d\n", info->maxInputChannels);
	logFile.WriteString(lf);
	lf.Format(L"Default sample rate=%f\n", info->defaultSampleRate);
	logFile.WriteString(lf);
	lf.Format(L"Default input device name=%s\n", CString((info->name)));
	logFile.WriteString(lf);
	lf.Format(L"Default HostApi=%s\n\n",CString((Pa_GetHostApiInfo(info->hostApi)->name)));
	logFile.WriteString(lf);
	lf.Format(L"Size of short is:-%d\n", sizeof(short) );
	logFile.WriteString(lf);


}

void PortAudioSound::CreateHammingProfile(double *hambuf){
	double max = 0.0;
	double alpha = 0.54;
	double beta = 1 - alpha;
	for (int i = 0; i < 512; i++){
		hambuf[i] = alpha - beta*cos((2 * 3.14159 * i) / 511);
		max = max(max, hambuf[i]);
	}

}

void PortAudioSound::ListDevices(){
	const PaDeviceInfo *deviceInfo;
	int numDevices = Pa_GetDeviceCount();
	for (int i = 0; i < numDevices; i++){
		CString desc = L"";
		deviceInfo = Pa_GetDeviceInfo(i);
		desc.Format(L"%d - %s by %s\n", i, CString(deviceInfo->name), CString((Pa_GetHostApiInfo(deviceInfo->hostApi)->name)));
		logFile.WriteString(desc);

	}
}

void PortAudioSound::error(int errorCode){
	if (errorCode != paNoError){
		CString err = L"";
		err.Format(L"\n\n********   Port Audio Error:-<%s>\n\n", CString(Pa_GetErrorText(errorCode)));
		OutputDebugString(err);
		//OutputDebugString(L(Pa_GetErrorText(errorCode)));
	}
}


PortAudioSound::~PortAudioSound()
{
	ReadIndex = WriteIndex = 0;
	int n = numBuffers;
	numBuffers = 0;
	for (int i = 0; i < n; i++){
		fftw_free(ppPowSpect[i]);
	}
	fftw_free(ppPowSpect);
	ppPowSpect = NULL;

	if (pStream != NULL){
		StopStream();
		error(Pa_CloseStream(pStream));
		logFile.WriteString(L"Stream Closed\n");
	}
	error(Pa_Terminate());
	logFile.WriteString(L"PortAudio terminated\n");

	//fftw_free(Pspect);
	fftw_destroy_plan(FFTplan);
	fftw_free(hamming);
	fftw_free(FFTin);
	fftw_free(FFTout);
}


// Starts the portaudio stream that was created by the constructor
int PortAudioSound::StartStream()
{
	int ierr = Pa_StartStream(pStream);
	error(ierr);
	logFile.WriteString(L"Stream started\n");
	return ierr;
}


// stops the portaudio stream
int PortAudioSound::StopStream()
{
	int ierr = paNoError;
	if (!Pa_IsStreamStopped(pStream)){
		ierr = Pa_StopStream(pStream);
		error(ierr);
		logFile.WriteString(L"Stream stopped\n");
	}
	return ierr;
}


// starts recording to a file as defined by the class parameters
int PortAudioSound::StartRecord()
{
	if (!recordflag){
		sfinfo.channels = 1;
		sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
		sfinfo.frames = 2048;
		sfinfo.samplerate = 192000;
		
		outfile = sf_open("c:/bat recordings/testfile.wav", SFM_WRITE, &sfinfo);
		if (outfile == NULL){
			int err = sf_error(NULL);
			const char *errstr = sf_error_number(err);
			logFile.WriteString((LPCWSTR) errstr);
		}
		recordflag = true;
		logFile.WriteString(L"Recording started\n");
	}
	return(0);
}


// stops recording to the file and closes it with appropriate log entries
int PortAudioSound::StopRecord()
{
	if (recordflag){
		recordflag = false;
		
		sf_close(outfile);
		logFile.WriteString(L"Recording stopped\n");
	}
	return 0;
}


// Records to a file for the specified number of minutes, with appropriate log entries
int PortAudioSound::RecordMinutes(int minutes)
{
	return 0;
}


// Turns on the built-in bat detector
int PortAudioSound::BatDetectorOn()
{
	return 0;
}


// turns off the built in bat detector
int PortAudioSound::BatDetectorOff()
{
	return 0;
}


// changes the bat detectcor tuning by the indicated amount (in kHz)
int PortAudioSound::Tune(int offset)
{
	return 0;
}




int PortAudioSound::myMemberCallback(const void * input, void * output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags)
{

	short *rdr = (short *) input;
	
	if (recordflag ){
		int written = (int)sf_writef_short(outfile, rdr, (sf_count_t)frameCount);
	}
	//int written=sf_writef_int(outfile, rdr, frameCount);
	int pos = 0;
	for (int i = 0; i < 8; i++){
		for (int j = 0; j < 512 && pos<2048; j++, pos++){
			FFTin[j] = hamming[j]*(double) rdr[pos];
		}
		pos = pos - 256;
		fftw_execute(FFTplan);

		WriteIndex = (WriteIndex + 1)%numBuffers;
		if (WriteIndex == ReadIndex) ReadIndex = (++ReadIndex%numBuffers);
		ppPowSpect[WriteIndex][0] = 0.0;


		for (int j = 0; j < 256; j++){
			//ppPowSpect[WriteIndex][j + 1] = log10(FFTout[j]);
			//ppPowSpect[WriteIndex][j + 1] = (FFTout[j] * FFTout[j]) /2000000.0;
			ppPowSpect[WriteIndex][j + 1] =log10( .000001+abs(FFTout[j]));
		}

		ppPowSpect[WriteIndex][0] = 1.0;

	}
	//displayWindow.PostMessageW(WM_USER, (WPARAM) WriteIndex);
	//SendMessage();
	PostMessage(ChildWindowHwnd, WM_USER, 0, 0L);
	return paContinue;
}


