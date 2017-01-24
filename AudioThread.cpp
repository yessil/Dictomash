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
	nzc = 0;
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
	tmp.Printf(_T("\nError: %d - %s"), errNum);
	WriteText(tmp);
	err = 0; // reset error
}

int AudioThread::Initialize(){


	silence = frame->ZERO_MARGIN; // ����� ������
	sil_cutoff = frame->SILENCE_CUTOFF; // ����� �����
	speech = frame->SPEECH_LENGTH; // ����� ����
	zc = 0;
	nzc = 0;
	ResetFiles();
	fileNum = 0;
	timeout = 0;

	return err;
}

void *AudioThread::Entry(){

	while(!TestDestroy()){
		Sleep(10);
		timeout++;
		silence = frame->ZERO_MARGIN;
		sil_cutoff = frame->SILENCE_CUTOFF;
		speech = frame->SPEECH_LENGTH;
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
			frame->WriteText(_T("�� ��������� �������� :(\n"));
			Sleep(3000);
			return;
		}
	//	if (!in_ad->recording)
			ad_start_rec(in_ad);

	frame->SetStatusbarText(_T("������ ������..."));

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

void AudioThread::Record2(){

		uint32 num_frames;
		char fname[20];
		static int m = 0;

		//if (!in_ad->recording)
			ad_start_rec(in_ad);

		sprintf(fname, "wav\\recorded");
		if (dump == NULL)
			if ((dump = fopen(fname, "wb")) == 0) {
				E_ERROR("Cannot open dump file %s\n", fname);
				return;
			}

		for(int i=0; i<FRAMES_PER_BUFFER; i++)
			frames[i]=0;
		num_frames = ad_read(in_ad, frames, FRAMES_PER_BUFFER);
		zc=0;
		for (int i=0; i<num_frames; i++){
			if (abs(frames[i])< silence){
				zc++;
			}
		}
/*		if (zc == num_frames){
			return;
		}*/
		if (m++ == 2){
			SetValue((abs(frames[0])+abs(frames[1])+abs(frames[2])) / 3); // level indicator on status panel
			m =0;
		}
		if (fwrite(frames, sizeof(int16), num_frames, dump) < num_frames) {
			E_ERROR("Error writing audio to dump file.\n");
		}
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
	double noiseLevel;
	static int pauseLenght = 0;
	static double noiseThreshold = 1000; // = 1/ noiseThreshold actually
	static int m = 0;

	static bool speechDetected = false;

	if (!OpenFile(fname))
		return;

	if (!in_ad->recording)
		ad_start_rec(in_ad);
	
	num_frames = ad_read(in_ad, frames, FRAMES_PER_BUFFER);
    if (num_frames > 0) {

		zc = nzc = 1;
		for (int i = 0; i<num_frames; i++){
			if (abs(frames[i])< silence){
				zc++;
			}
			else {
				nzc++;
			}
		}

		noiseLevel = double(nzc * 1.0) / double((zc *1.0));
		//if (noiseThresholdCounter <= MAX_NOISE_LEVEL_COUNT){
		//	sumnoiseThreshold += noiseLevel;
		//	if (noiseThresholdCounter++ == MAX_NOISE_LEVEL_COUNT){
		//		noiseThreshold = sumnoiseThreshold / MAX_NOISE_LEVEL_COUNT;
		//	}
		//	return;
		//}
		noiseThreshold = speech *1e-3;// sil_noiseLeveloff  *1e-3;
		if (debug)
			frame->SetStatusbarText(wxString::Format(_T("noiseLevel: %5.3f noise: %5.3f"), noiseLevel, noiseThreshold));
		if (noiseLevel < noiseThreshold ){
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
				frame->SetStatusbarText(wxString::Format(_T("noiseLevel: %5.3f noise: %5.3f"), noiseLevel, noiseThreshold));

		if (pauseLenght > sil_cutoff){ //close  current dump file and open the new one
			if (debug)
				frame->SetStatusbarText(wxString::Format(_T("noiseLevel: %5f.3 !"), noiseLevel));

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