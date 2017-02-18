#ifndef AUDIOTHREAD_H
#include "AudioThread.h"
#endif
//#include "sndfile.h"
//#include "BaseFrame.h"
//#include "wx/dir.h"
//#include "err.h"


/*******************************************************************/

AudioThread::AudioThread(void):wxThread()
{
	playIt = false;
	recordIt = false;
	stop = false;
	err = 0;

}

AudioThread::AudioThread(BaseFrame *f){

	playIt = false;
	recordIt = false;
	stop = false;
	err = 0;
	frame = f;
	in_ad = 0;
	dump = NULL;
	fileNum = 0;
	timeout = 0;
}

AudioThread::~AudioThread(void)
{
	if (dump!=NULL){
		fclose(dump);
		dump = NULL;
	}

}

void AudioThread::WriteText(const wxString& text){

		wxString msg;
		// before doing any GUI calls we must ensure that this thread is the only
		// one doing it!
		wxMutexGuiEnter();
		msg << text;
		((BaseFrame*)frame)->WriteText(msg);
		wxMutexGuiLeave();
}

void AudioThread::SetValue(int value){

		wxMutexGuiEnter();
		if (stop)
			return;
		((BaseFrame*)frame)->SetValue(value);
		wxMutexGuiLeave();
}


void AudioThread::WriteText(const wxString& text, int errNum){

	wxString tmp = text;
	tmp.Printf(_("\nError: %d - %s"), errNum);
	WriteText(tmp);
	err = 0; // reset error
}

int AudioThread::Initialize(){


	ResetFiles();
	fileNum = 0;
	timeout = 0;

	return err;
}

void *AudioThread::Entry(){

	while(!TestDestroy()){
		Sleep(10);
		timeout++;
		noiseThreshold = frame->NOISE_THRESHOLD; // Порог тишины
		maxPauseLength = frame->MAX_PAUSE_LENGTH; // Длина паузы
		signalLevel = frame->SIGNAL_LEVEL *1E-3; // Условный порог речи
		debug = frame->debug;


		if (stop ){
			StopStream();
			Exit();
			continue;
		}

		if (recordIt){
			if (in_ad == 0 )
				StartStream();
			else 
				Record();
		} else {
			SetValue(0);
			//SaveFile();
			if (in_ad!=0 && in_ad->recording)
				ad_stop_rec(in_ad);
			//StopStream();
		}
	}
	return NULL;
}

void AudioThread::StartStream(){

	if (in_ad == 0)
		if ((in_ad = ad_open_sps(SAMPLE_RATE)) == NULL) {
			frame->WriteText(_("Не подключен микрофон :(\n"));
			Sleep(3000);
			return;
		}
	//	if (!in_ad->recording)
			ad_start_rec(in_ad);

	frame->SetStatusbarText(_("Начало записи..."));

}

void AudioThread::StopStream()
{
	if (in_ad == 0)
		return;
	ad_stop_rec(in_ad);
    ad_close(in_ad);
}



void AudioThread::ResetFiles(){

	wxArrayString *files = new wxArrayString();
	wxDir::GetAllFiles(_T("."), files, _T("*.*"));
	int n = (*files).Count();
	if (dump !=NULL)
		fclose(dump);
	for (int i=0; i < n; i++)
		remove((char*)(*files)[i].char_str());
	delete files;

}
void AudioThread::Stop(){

	stop = true;
}

void AudioThread::ToggleRecord(){

	recordIt = ! recordIt;
}


void AudioThread::SaveFile(){

		char fname[20];
		char newname[20];
		int fsize = 0;

		sprintf(fname, "wav\\recorded");

		if (dump == NULL)
			return;
		fsize = 1;//TODO !!!! dump->_cnt;
		fclose(dump);
		if (fsize > 0){
			strcpy(newname, fname);
			rename(fname, strcat(newname, ".raw"));
		}
		dump = NULL;
		ad_stop_rec(in_ad);
}

bool AudioThread::OpenFile(char* filename){

	sprintf(filename, "wav\\recorded%03d", fileNum);
	if (dump != NULL) // is already opened
		return  true;
	if ((dump = fopen(filename, "wb")) == 0) {
		E_ERROR("Cannot open dump file %s\n", filename);
		return false;
	}
	return true;
}

void AudioThread::Record(){

	uint32 num_frames;
	char fname[FILENAME_LENGHT];
	char newname[FILENAME_LENGHT];
	int zc, nzc;

	double noiseLevel = 0;
	static int pauseLenght = 0;
	static int m = 0;

	static bool speechDetected = false;

	if (!OpenFile(fname))
		return;

	if (!in_ad->recording)
		ad_start_rec(in_ad);
	
	num_frames = ad_read(in_ad, frames, FRAMES_PER_BUFFER);
	if (debug)
		frame->SetStatusbarText(wxString::Format(_T("num_frames: %d noiseLevel: %5.3f signalLevel: %5.3f"), num_frames, noiseLevel, signalLevel));

	if (num_frames > 0) {

		zc = nzc = 1;
		for (int i = 0; i<num_frames; i++){
			if (abs(frames[i])< noiseThreshold){
				zc++;
			}
			else {
				nzc++;
			}
		}

		noiseLevel = double(nzc * 1.0) / double((zc *1.0));
		if (debug)
			frame->SetStatusbarText(wxString::Format(_T("num_frames: %d noiseLevel: %5.3f signalLevel: %5.3f"), num_frames, noiseLevel, signalLevel));
		if (noiseLevel < signalLevel){
			if (!speechDetected)
				return;
			else 
				pauseLenght++;
		}
		else {
			pauseLenght = 0;
		}

		speechDetected = true;

		if (fwrite(frames, sizeof(int16), num_frames, dump) < num_frames) {
			E_ERROR("Error writing audio to dump file.\n");
			return;
		}

		if (m++ == 2){
			SetValue((abs(frames[0])+abs(frames[1])+abs(frames[2])) / 3); // level indicator on status panel
			m =0;
		}

		if (debug)
			frame->SetStatusbarText(wxString::Format(_T("num_frames: %d noiseLevel: %5.3f signalLevel: %5.3f"), num_frames, noiseLevel, signalLevel));

		if (pauseLenght > maxPauseLength){ //close  current dump file and open the new one

			fclose(dump);
			dump = NULL;
			strcpy(newname, fname);
			rename(fname, strcat(newname, ".raw"));
			sprintf(fname, "wav\\recorded%03d", ++fileNum);
			pauseLenght = 0;
			speechDetected = false;
		}
    }
	else {

		debug = debug;
	}
}

 int AudioThread::getTimer(){ return timeout; }
 void AudioThread::ResetTimer(){ 

	 if (dump!=NULL){
		 SaveFile();
		 return;
		//fclose(dump);
	 }
	 dump = NULL;
	 ResetFiles();
	 timeout = 0;
 }