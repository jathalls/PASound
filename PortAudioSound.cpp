#include "stdafx.h"
#include "PortAudioSound.h"


PortAudioSound::PortAudioSound()
{
	error(Pa_Initialize());
	
	iSampleRate = 192000;
	sampleRate = (double) iSampleRate;
	framesPerBuffer = 2048;
	FFTSize = 512;
	FFTsPerBuffer = framesPerBuffer/FFTSize;
	recordflag = false;
	Processing = FALSE;
	overlap = FALSE;
	
	if (!logFile.Open(L"C:/Bat Recordings/TestFileLog.txt", CFile::modeCreate | CFile::modeWrite | CFile::typeText)){
		OutputDebugString(L"Unable to open Log File\n");
	}

	numBuffers = 16;
	ReadIndex = 0;
	WriteIndex = 0;
	currentColumn = 0;
	currentRow = 0;
	maxi = 6.0;
	mini = -6.0;
	
	FFTin = (double *) fftw_malloc(sizeof(double) * (FFTSize+2));
	FFTout = (double *) fftw_malloc(sizeof(double) * FFTSize);
	hamming = (double *) fftw_malloc(sizeof(double) * FFTSize);
	CreateHammingProfile(hamming);
	FFTplan = fftw_plan_r2r_1d(FFTSize, FFTin, FFTout, FFTW_DHT, FFTW_ESTIMATE);
	ppPowSpect = (double **) fftw_malloc(sizeof(double *) * numBuffers);
	ppShade = (BYTE **) fftw_malloc(sizeof(BYTE *) *numBuffers);
	for (int i = 0; i < numBuffers; i++){
		ppPowSpect[i] = (double *) fftw_malloc(sizeof(double) * (FFTSize+1));
		ppShade[i] = (BYTE *) fftw_malloc(sizeof(BYTE) * (FFTSize + 1));
	}
	//FFTplan = fftw_plan_dft_r2c_1d(512, FFTin, FFTout,  FFTW_ESTIMATE);
	//Pspect = (double *) fftw_malloc(sizeof(double) * 512);

	ListDevices();

	PaStreamParameters inputParameters;

	inputParameters.device = 1;
	inputParameters.channelCount = 1;
	inputParameters.sampleFormat = paInt16;
	
	inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultHighInputLatency;
	inputParameters.hostApiSpecificStreamInfo = NULL;
	int err = Pa_IsFormatSupported(&inputParameters, NULL, sampleRate);
	if (err==0){
		logFile.WriteString(L"Input format is supported\n");
	}
	else{
		logFile.WriteString(L"Input format NOT supported\n");
	}


	
	//error(Pa_OpenStream(
	//	&pStream,
	//	&inputParameters, /* input channel*/
	//	NULL, /* output channel*/
	//	Pa_GetDeviceInfo(inputParameters.device)->defaultSampleRate,
	//	2048,
	//	paClipOff,
	//	&PortAudioSound::myPaCallback,
	//	this
	//	));
		

		error(Pa_OpenStream(
		&pStream,
		&inputParameters, /* input channel*/
		NULL, /* output channel*/
		sampleRate,
		framesPerBuffer,
		paClipOff,
		&PortAudioSound::myPaCallback,
		this
		));

	logFile.WriteString(L"Stream opened\n");
	const PaDeviceInfo* info= Pa_GetDeviceInfo(1);
	CString lf;
	lf.Format(L"Max Input Channels=%d\n", info->maxInputChannels);
	logFile.WriteString(lf);
	lf.Format(L"Default sample rate=%f\n", info->defaultSampleRate);
	logFile.WriteString(lf);
	lf.Format(L"Default input device name=%s\n", CString((info->name)));
	logFile.WriteString(lf);
	lf.Format(L"Default HostApi=%s\n\n",CString((Pa_GetHostApiInfo(info->hostApi)->name)));
	logFile.WriteString(lf);
	


}

void PortAudioSound::CreateHammingProfile(double *hambuf){
	double max = 0.0;
	double alpha = 0.54;
	double beta = 1 - alpha;
	for (int i = 0; i < FFTSize; i++){
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

		PaStreamParameters inputParameters;

		inputParameters.device = i;
		inputParameters.channelCount = 1;
		inputParameters.sampleFormat = paInt16;

		inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultHighInputLatency;
		inputParameters.hostApiSpecificStreamInfo = NULL;
		int err = Pa_IsFormatSupported(&inputParameters, NULL, sampleRate);
		CString str;
		if (err == 0) str = "";
		else str = L"not ";

		desc.Format(L"%d - %s by %s - desired format is %ssupported\n", i, CString(deviceInfo->name), CString((Pa_GetHostApiInfo(deviceInfo->hostApi)->name)),str);
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
		fftw_free(ppShade[i]);
	}
	fftw_free(ppPowSpect);
	fftw_free(ppShade);
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
	int ierr = paNoError;
	//if (!Pa_IsStreamStopped(pStream)){
		ierr = Pa_StartStream(pStream);
		error(ierr);
		logFile.WriteString(L"Stream started\n");
	//}
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
		startRecording = TRUE;
		recordedBlocks = 0L;
		stopTime = -1.0;
		sfinfo.channels = 1;
		sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
		sfinfo.frames = framesPerBuffer;
		sfinfo.samplerate = iSampleRate;
		timeStr = "";
		
		outfile = sf_open("c:/bat recordings/testfile.wav", SFM_WRITE, &sfinfo);
		if (outfile == NULL){
			int err = sf_error(NULL);
			const char *errstr = sf_error_number(err);
			logFile.WriteString((LPCWSTR) errstr);
		}
		logFile.WriteString(L"Recording started\n");
		startTime = (double) Pa_GetStreamTime(pStream);
		recordflag = true;
		
	}
	return(0);
}


// stops recording to the file and closes it with appropriate log entries
int PortAudioSound::StopRecord()
{
	
	if (recordflag){
		recordflag = false;
		startRecording = FALSE;
		stopTime = (double) Pa_GetStreamTime(pStream);
		sf_close(outfile);
		
		CString str;
		str.Format(L"Min=%f, Max=%f after recording %ld blocks \n", mini, maxi,recordedBlocks);
		logFile.WriteString(str);
		str.Format(L"Duration of recording= %f to %f = %f seconds\n",(double)startTime,(double)stopTime, (double)stopTime - (double)startTime);
		logFile.WriteString(str);
		logFile.WriteString(L"Recording stopped\n\n");
		logFile.WriteString(timeStr);
		Pa_Sleep(2000);
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
	if (!Processing){
		Processing = TRUE;
		double t = Pa_GetStreamTime(pStream);
		short *rdr = (short *) input;
		CString tstr;
	/*	if (startRecording){
			startRecording = FALSE;
			startTime = (double)timeInfo->inputBufferAdcTime;
		}*/
		if (recordflag){
			int written = (int) sf_writef_short(outfile, rdr, (sf_count_t) frameCount);
			recordedBlocks++;
			tstr.Format(L"%f\n", t);

			timeStr = timeStr + tstr;
			
			//stopTime = (double)timeInfo->inputBufferAdcTime;
		}
		
		int pos = 0;
		int DisplayFramesPerBuffer;
		if (overlap){
			DisplayFramesPerBuffer = (FFTsPerBuffer * 2) - 1;
		}
		else{
			DisplayFramesPerBuffer = FFTsPerBuffer;
		}
		for (int i = 0; i < (FFTsPerBuffer*2)-1; i++){
			for (int j = 0; j < FFTSize && pos < framesPerBuffer; j++, pos++){
				FFTin[j] = hamming[j] * (double) rdr[pos];
			}
			if (overlap){// only move the read pointer back for overlap
				pos = pos - FFTSize / 2;
			}
			
			fftw_execute(FFTplan);

			WriteIndex = (WriteIndex + 1)%numBuffers;
			if (WriteIndex == ReadIndex) ReadIndex = (++ReadIndex%numBuffers);
			//ppPowSpect[WriteIndex][0] = 0.0;
			ppShade[WriteIndex][0] = 0;


			for (int j = 0; j < FFTSize/2; j++){
				//ppPowSpect[WriteIndex][j + 1] = log10(FFTout[j]);
				//ppPowSpect[WriteIndex][j + 1] = (FFTout[j] * FFTout[j]) /2000000.0;
				//ppPowSpect[WriteIndex][j + 1] = log10(.000001 + abs(FFTout[j]));

				ppShade[WriteIndex][j + 1] = 255-(BYTE) (((log10(.000001 + abs(FFTout[j]))+6)/6)*255);
			}

			//ppPowSpect[WriteIndex][0] = 1.0;
			ppShade[WriteIndex][0] = 1;

		}
		//displayWindow.PostMessageW(WM_USER, (WPARAM) WriteIndex);
		//SendMessage();
		PostMessage(ChildWindowHwnd, WM_USER, 0, 0L);
		Processing = FALSE;
	}
	else{
		logFile.WriteString(L"*");
	}
	
	return paContinue;
}

/*
	DrawSpectrogram - called from the view that is to display the spectrogram
	passing a *deviceContext for the view and a pointer to the rectangle to drawn
	into (this function only accesses the width and height of the rectangle)

	The function is part of the PortAudioSound class since it is specifically for
	use with data held within that class but is called on the UI thread in response to
	WM_USER message that is Posted to the UI thread from the callback function.
	*/
bool PortAudioSound::DrawSpectrogram(CDC *pDC,CRect *pDrawing_area){

	if (ReadIndex != WriteIndex){

		int bufSize = FFTSize / 2;
		CString str;





		int readIndex = ReadIndex;
		//while (ppPowSpect[readIndex][0] != 0.0){
		while (ppShade[readIndex][0]!=0){



			for (int x = 0; x < bufSize; x++){
				

					//maxi = max(ppPowSpect[readIndex][bufSize - x], maxi);
					//mini = min(ppPowSpect[readIndex][bufSize - x], mini);
				
				//BYTE shade = 255 - (BYTE) (((ppPowSpect[readIndex][bufSize - x] +6.0) / 6.0)*255.0);

				COLORREF bcolor = RGB(ppShade[readIndex][bufSize - x], ppShade[readIndex][bufSize - x], ppShade[readIndex][bufSize - x]);
				pDC->SetPixel(currentColumn, x + (currentRow * 300), bcolor);
			}
			currentColumn = ++currentColumn;
			if (currentColumn >= pDrawing_area->Width()){
				currentColumn = 0;
				currentRow = ++currentRow % 2;
			}

			//ppPowSpect[readIndex][0] = 0.0;
			ppShade[readIndex][0] = 0;
			ReadIndex = ++readIndex%numBuffers;
			readIndex = ReadIndex;
			if (ReadIndex == WriteIndex) break;
		}
		ReadIndex = readIndex;
		//str.Format(L"%d", readIndex);
		//pDC->TextOutW(10, 280, str);
		
		return(TRUE);
	}
	return(FALSE);
}


