///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Apr 16 2008)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////
#include "ASRApp.h"
#include "ASRAppFrame.h"
#include "wx/fontenc.h"
#include "AudioDialog.h"
#include "wx/textfile.h"
#include "wx/msw/ole/automtn.h"
#include "wx/intl.h"
#include "defevent.h"
#include "psapi.h"
#include <wx/tokenzr.h>
#include <wx/process.h>
#include <wx/fontdlg.h>

///////////////////////////////////////////////////////////////////////////

MyFrame1::MyFrame1( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : BaseFrame( parent, id, title, pos, size, style )
{
	m_listCtrl = NULL;
	decoder = NULL;
	audioIO = NULL;
	m_taskBarIcon = NULL;
	m_cursor = NULL;
	pConfig = NULL;
	stop = false;
	NOISE_THRESHOLD  = 700;
	MAX_PAUSE_LENGTH = 10;
	SIGNAL_LEVEL = 2000;
	SERVER = _T("alphabet.kz");
	PORT = 3000;
	debug = false;
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	m_cursor = new wxCursor(wxCURSOR_WAIT);
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );
	wxLocale::AddCatalogLookupPathPrefix(wxT("./lang"));
	bool check =
	m_locale.Init(wxLANGUAGE_KAZAKH, wxLOCALE_CONV_ENCODING);
	check = m_locale.AddCatalog(wxT("Dictomash"));
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxVERTICAL );
	m_panel1 = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_textbox = new wxRichTextCtrl(m_panel1, RICHTEXT_CTRL, wxEmptyString, wxPoint(210,0), wxSize(200, 200), wxVSCROLL|wxHSCROLL|wxNO_BORDER|wxWANTS_CHARS);
	bSizer2->Add( m_textbox, 1, wxEXPAND | wxALL, 5 );
	m_panel1->SetSizer( bSizer2 );
	m_panel1->Layout();
	bSizer2->Fit( m_panel1 );
	wxFont font(24, wxROMAN, wxNORMAL, wxNORMAL, false, wxT("Times New Roman"), wxFONTENCODING_CP1251);
 	m_textbox->SetFont(font);

	bSizer1->Add( m_panel1, 1, wxEXPAND | wxALL, 5 );

  
#if wxUSE_MENUS
    // create a menu bar
    wxMenu *fileMenu = new wxMenu;

    // the "About" item should be in the help menu
    wxMenu *helpMenu = new wxMenu;
	helpMenu->Append(ASRApp_Help, _("&Help\tF1"), _("Instructions"));
    helpMenu->Append(ASRApp_About, _("&About program..."), _("Brief description"));

	fileMenu->Append(ASRApp_OpenFile, _("Process file\tF2"), _("Process file"));
    fileMenu->Append(ASRApp_Font, _("Font\tAlt-F"), _("Font"));
//	fileMenu->Append(ASRApp_Listen, _("Restart\tF3"), _("Restart"));
	fileMenu->Append(ASRApp_Play, _("Listen\tF6"), _("Listen to last phrase"));
	fileMenu->Append(ASRApp_Record, _("Record\tF7"), _("Record"));
//	fileMenu->Append(ASRApp_Decode, _("Add new words\tF8"), _("Add new words"));
	fileMenu->Append(ASRApp_AudioParams, _("Parameters\tF9"), _("Audio parameters"));
    fileMenu->Append(ASRApp_Clear, _("Clear\tF10"), _("Clear the screen"));
    fileMenu->Append(ASRApp_Hide, _("H&ide\tAlt-X"), _("Hide the screen"));
    fileMenu->Append(ASRApp_Shutdown, _("Stop the server\tF12"), _("Stop the server"));
	fileMenu->Append(ASRApp_SendKeyStroke, _("Output to any active app"), _("Output to any active app"), wxITEM_CHECK );

    // now append the freshly created menu to the menu bar...
    wxMenuBar *menuBar = new wxMenuBar();
    menuBar->Append(fileMenu, _("&Actions"));
    menuBar->Append(helpMenu, _("&Help"));

    // ... and attach this menu bar to the frame
    SetMenuBar(menuBar);
#endif // wxUSE_MENUS

#if wxUSE_STATUSBAR
	bar = new MyStatusBar(this);
	SetStatusBar(bar);
	PositionStatusBar();
	bar->Layout();
#endif // wxUSE_STATUSBAR

	this->SetSizer( bSizer1 );
	this->Layout();
	this->Center();
	pConfig = wxConfigBase::Get();
	log = fopen(".\\Dictomash.log","a");
	logchain = new wxLogChain(new wxLogStderr(log));
	LoadParams();
	InitDecoder();
	InitAudio();
	doModelUpdate = false;


	m_taskBarIcon = new MyTaskBarIcon();
	if (!m_taskBarIcon->SetIcon(wxICON(logo), PROGRAM_TITLE))
        wxMessageBox(wxT("Could not set icon."));
	sendKeyStroke = false;
}

MyFrame1::~MyFrame1()
{
	if (m_taskBarIcon!=NULL)
		delete m_taskBarIcon;
	delete m_cursor;
	free(launchdir);
	if (pConfig != NULL){
		SaveParams();
		delete pConfig;
		pConfig = NULL;
	}
	SetStatusBar(NULL);
	delete bar;
	m_cursor = NULL;
	m_taskBarIcon = NULL;
	delete wxLog::SetActiveTarget(NULL);
	if (log!=NULL)
		fclose(log);
}

// event handlers

void MyFrame1::Exec(wxString cmd){

		process = wxProcess::Open(cmd);

		if (!process){
			wxMessageBox(_("Exec"), _("Error"), wxOK | wxICON_ERROR);		
		} else {
			//long pid = process->GetPid();
			process->SetNextHandler(this);
		}

}
void MyFrame1::OnHide(wxCommandEvent& WXUNUSED(event))
{
	Hide();
}

void MyFrame1::OnFont(wxCommandEvent& WXUNUSED(event))
{
	wxFont font =::wxGetFontFromUser(this, wxNullFont, _("Font"));
	if (font.IsOk())
		m_textbox->SetFont(font);

}

void MyFrame1::OnSendKeyStroke(wxCommandEvent& event){
	sendKeyStroke = !sendKeyStroke;
}

void MyFrame1::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxMessageBox(wxString::Format(
		_("Developed by: Yessil Khassenov, Astana\nVersion: %s\nOperating system is %s."),
                    _("2.0"),
					wxGetOsDescription().c_str()
                 ),
                 _("Brief info"),
                 wxOK | wxICON_INFORMATION,
                 this);
	//m_textbox->Freeze();
	m_textbox->SetCursor(*m_cursor);
	//this->SetCursor(wxCURSOR_ARROWWAIT);
	
}

void MyFrame1::OnSize(wxSizeEvent& event)
{
   DoSize();
   event.Skip();
}

void MyFrame1::DoSize()
{
}

void MyFrame1::OnAudioParams(wxCommandEvent& WXUNUSED(event)){

	AudioParams();
}

void MyFrame1::InitWithListItems()
{
    for ( int i = 0; i < NUM_ITEMS; i++ )
    {
        m_listCtrl->InsertItem(i, wxString::Format(_("Item %d"), i));
    }
}

void MyFrame1::InitWithReportItems()
{

    // note that under MSW for SetColumnWidth() to work we need to create the
    // items with images initially even if we specify dummy image id
    wxListItem itemCol;
    itemCol.SetText(_("Звуковой файл"));
    itemCol.SetImage(-1);
    m_listCtrl->InsertColumn(0, itemCol);
	m_listCtrl->SetColumnWidth(0, 400);

    // to speed up inserting we hide the control temporarily
    m_listCtrl->Hide();
    wxString buf;
	wxFileInputStream input( _T("f:\\ASR\\ru\\task1\\etc\\task1_train.fileids") );
	wxTextInputStream text( input );

	for ( int i = 0; i < NUM_ITEMS; i++ )
    {
		text >> buf;
		if (buf.IsEmpty())
			break;
		//buf.Printf(_T("This is item %d"), i);
		long tmp = m_listCtrl->InsertItem(i, buf, 0);
		m_listCtrl->SetItemData(tmp, i);
	}

    m_listCtrl->Show();

    //m_listCtrl->SetColumnWidth( 0, wxLIST_AUTOSIZE );

}

void MyFrame1::OnListen(wxCommandEvent& WXUNUSED(event)){

	Listen();
}
void MyFrame1::OnRecord(wxCommandEvent& WXUNUSED(event)){

	Record();

}

void MyFrame1::OnPlay(wxCommandEvent& WXUNUSED(event)){

	wxString curAudioFile(_("wav\\lastfile.rw"));
	audioOut->StopStream();
	if (wxFile::Exists(curAudioFile)){
		audioOut->fileName = curAudioFile;//path;
		audioOut->StartStream(false);
	}
	else {
		WriteText(_(":(("));
	}


	//wxString cmd, path;
	//char buf[_MAX_PATH];
	//getcwd(buf, _MAX_PATH);
	//path = wxString::FromAscii(buf);
	//Exec( wxString::Format(_("cmd /c %s\\playit.bat"), wxString::FromAscii(launchdir)));
}

void writeFile(wxString);
void MyFrame1::OnDecode(wxCommandEvent& WXUNUSED(event)){

	//wxExecute(_("notepad lm\\text.txt"), wxEXEC_SYNC);
	static bool prev;
	wxString cmd, path;
	if (doModelUpdate){
		wxBusyCursor wait;
		::writeFile(m_textbox->GetValue().Trim().Trim(false).Upper());
		char buf[_MAX_PATH];
		getcwd(buf, _MAX_PATH);
		path = wxString::FromAscii(buf);
		process = wxProcess::Open(_T("cmd /c lm\\mklm.bat"));
		if (!process){
			wxMessageBox(_("Updating vocabulary..."), _("Error"), wxOK | wxICON_ERROR);		
		} else {
			//long pid = process->GetPid();
			process->SetNextHandler(this);
//			process->Detach();
//			while (wxProcess::Exists(pid));
		}
		decoder->ReStart();
		doModelUpdate = false;
		decoder->Resume();
		audioIO->Initialize();
		audioIO->recordIt = prev;
	} else {
		prev = audioIO->recordIt;
		audioIO->recordIt = false;
		wxThread::Sleep(1000);
		while (audioIO->recordIt);
		decoder->Pause();
		m_textbox->Clear();
		wxMessageBox(_("Updating vocabulary..."),
					 _("Enter new words"), wxOK | wxICON_INFORMATION);
		doModelUpdate = true;
	}
}

void MyFrame1::OnProcessTerm(wxProcessEvent& WXUNUSED(event))
{
	delete process;
    process = NULL;
}

void MyFrame1::OnQuit(wxCloseEvent& event){

	Quit();
	event.Skip();

}

void MyFrame1::Listen(){

	decoder->ReStart();
	CheckDecoderSrv();
}

wxString Test();
bool MyFrame1::Record(){

	audioIO->ToggleRecord();
	if (audioIO->recordIt){
		m_taskBarIcon->SetIcon(wxICON( mic_on), _("Microphone is on"));
		//Exec(_("cmd /c tell.bat intro"));
		SetStatusbarText(_("Microphone is on"));
	}
	else{
		m_taskBarIcon->SetIcon(wxICON(mic_off), _("Microphone is off"));
		SetStatusbarText(_("Microphone is off"));
		audioIO->ResetTimer();
	}

	return audioIO->recordIt;
}

void MyFrame1::WriteText(const wxString& text) { 

	wxColor color(_("BLACK"));
	m_textbox->SetForegroundColour(color);
	m_textbox->WriteText(text); 
}

void MyFrame1::AudioParams(){

//WriteText(_("Проверка\n"));

	AudioDialog* dlg = new AudioDialog(this, wxID_ANY, _("Parameters"), wxDefaultPosition, wxSize(320,230), wxDEFAULT_DIALOG_STYLE);
	dlg->SetValue(_T(""), debug);
	dlg->SetValue(NOISE_THRESHOLD_KEY, NOISE_THRESHOLD);
	dlg->SetValue(MAX_PAUSE_LENGTH_KEY, MAX_PAUSE_LENGTH);
	dlg->SetValue(SIGNAL_LEVEL_KEY, SIGNAL_LEVEL);
	wxString server_port = wxString::Format(_("%s:%d"), SERVER, PORT);
	dlg->SetStrValue(SERVER_KEY, server_port);

	if (dlg->ShowModal() == wxID_OK){
		NOISE_THRESHOLD= dlg->GetValue(NOISE_THRESHOLD_KEY);
		MAX_PAUSE_LENGTH = dlg->GetValue(MAX_PAUSE_LENGTH_KEY);
		SIGNAL_LEVEL = dlg->GetValue(SIGNAL_LEVEL_KEY);
		server_port = dlg->GetStrValue(SERVER_KEY);
		wxString tmp = server_port.BeforeFirst(':');
		if (!tmp.IsEmpty()) 
			SERVER = tmp;
		long l = PORT;
		tmp = server_port.AfterFirst(':');
		if (!tmp.IsEmpty())
			tmp.ToLong(&l);
		PORT = (int) l;
		GetMenuBar()->FindItem(ASRApp_Shutdown)->Enable(SERVER.Lower() == _("localhost"));

		debug = dlg->GetValue() >0;
		if (debug){
			wxString s;
			WriteText(wxString::Format(_("NOISE_LEVEL: %d\nMAX_PAUSE_LENGTH: %d\nSIGNAL_LEVEL: %d\nDEBUG %s\n"),\
				NOISE_THRESHOLD, MAX_PAUSE_LENGTH, SIGNAL_LEVEL, debug ? _("TRUE") : _("FALSE")));
		}
	}
	dlg->Destroy();
	SaveParams();
}
void MyFrame1::LoadParams(){

	if ( pConfig == NULL )
		return;
	NOISE_THRESHOLD = pConfig->Read(NOISE_THRESHOLD_KEY, NOISE_THRESHOLD);
	MAX_PAUSE_LENGTH = pConfig->Read(MAX_PAUSE_LENGTH_KEY, MAX_PAUSE_LENGTH);
	SIGNAL_LEVEL = pConfig->Read(SIGNAL_LEVEL_KEY, SIGNAL_LEVEL);
	wxString server_port = pConfig->Read(SERVER_KEY, SERVER);
	wxString tmp = server_port.BeforeFirst(':');
	if (!tmp.IsEmpty()) 
		SERVER = tmp;
	long l = PORT;
	tmp = server_port.AfterFirst(':');
	if (!tmp.IsEmpty())
		tmp.ToLong(&l);
	PORT = (int) l;
	GetMenuBar()->FindItem(ASRApp_Shutdown)->Enable(SERVER.Lower() == _T("localhost"));

}
void MyFrame1::SaveParams(){

	if ( pConfig == NULL )
		return;
	pConfig->Write(NOISE_THRESHOLD_KEY, NOISE_THRESHOLD);
	pConfig->Write(MAX_PAUSE_LENGTH_KEY, MAX_PAUSE_LENGTH);
	pConfig->Write(SIGNAL_LEVEL_KEY, SIGNAL_LEVEL);
	wxString server_port = wxString::Format(_("%s:%d"), SERVER, PORT);
	pConfig->Write(SERVER_KEY, server_port);

}
void MyFrame1::OnSelected(wxListEvent& event){


      curItem.m_itemId = event.m_itemIndex;
      curItem.m_col = 0;
      curItem.m_mask = wxLIST_MASK_TEXT;
      if ( !m_listCtrl->GetItem(curItem) )
       {
            wxFAIL_MSG(wxT("wxListCtrl::GetItem() failed"));
       }
}
void MyFrame1::InitAudio(){

	audioIO = new AudioThread(this);
//	AudioThread::RawToWav("wav\\recorded000.raw");
	audioIO->Create();
	audioIO->Initialize();
	audioIO->Run();
	
	audioOut = new ReadAudio(this);
	audioOut->Create();
	audioOut->Initialize();
	audioOut->Run();
	
}

void MyFrame1::InitDecoder(){
	
	char *temp = getenv("TEMP");
	launchdir = _getcwd(NULL, _MAX_PATH);

	SetStatusbarText(_("Wait..."));
	if (SERVER.Lower()== _T("localhost"))
		CheckDecoderSrv();

	_chdir(temp);
	_mkdir("wav");
	_mkdir("feat");

	decoder = new DecoderThread(this);
	decoder->Create();
	decoder->Initialize();
	SetStatusbarText(_(""));
	decoder->Run();
}

void MyFrame1::OnOpenFile(wxCommandEvent& WXUNUSED(event)){

    wxString path;
    wxString filename;
    wxArrayInt fileTypes;

    wxString filter = wxT("wav files (*.wav)|*.wav|");
    filter += wxT("All files (*.*)|*.*");

    wxFileDialog dialog(this,
        _("Choose the audio file"),
        path,
        filename,
        filter,
        wxFD_OPEN);
	decoder->Pause();

    if (dialog.ShowModal() == wxID_OK)
    {
        wxString path = dialog.GetPath();
        if (!path.empty())
        {
			wxFont font(15, wxROMAN, wxFONTSTYLE_ITALIC, wxNORMAL, false, wxT("Arial"), wxFONTENCODING_CP1251);
			wxFont curFont = m_textbox->GetFont();
			m_textbox->BeginFont(font);
			m_textbox->BeginTextColour(wxColor(_T("Brown")));
			wxCopyFile(path, _T("wav\\recorded.raw"));
			WriteText(_("\n") + path + _("\n"));
			m_textbox->EndTextColour();
			m_textbox->EndFont();

        }
    }
	decoder->Resume();
} 

void MyFrame1::Hide(){

	this->Show(false);
}
void MyFrame1::Quit(){
	if (audioIO !=NULL){
		audioIO->Stop();
		//audioIO->Delete();
		//audioIO = NULL;
	}
	decoder->Stop();
	audioOut->Stop();
	Sleep(1000);
	wxExit();
}

void MyFrame1::SetStatusbarText(wxString msg){

	SetStatusText(msg);
}

void writeFile(wxString text){

	wxString file(_("lm\\b.txt"));
	wxString line;
	wxTextFile f(file);
	
	if (f.Exists())
		wxRemoveFile(file);
	f.Create();
	wxStringTokenizer tkz(text, _T("\r\n"));
	while ( tkz.HasMoreTokens() )
	{
		line = tkz.GetNextToken();
		f.AddLine(line.Prepend(_T("<s> ")).Append(_T(" </s>")));
	}
	f.Write();
	f.Close();
}

void MyFrame1::OnProcessNewText(wxCommandEvent& event){
// generate keystrokes to send to active window

 	wxVariant  res;
	wxString text;
	int len = 0;
	KEYBDINPUT ki[1000];
	INPUT in[1000];

	text = event.GetString();
	len =  text.Length();
//TODO
	len = len > 1000 ? 1000 : len;
	if ( len == 0)
		return;
	for (int i=0; i<len; i++){
		ki[i].wVk = 0;
		ki[i].dwFlags = KEYEVENTF_UNICODE;
		ki[i].wScan = text.c_str()[i];
		in[i].type =  INPUT_KEYBOARD;
		in[i].ki = ki[i];
	}
	if (sendKeyStroke)
		::SendInput(len, in, sizeof(INPUT));
	else 
		WriteText(text);

}

void MyFrame1::SetValue(int value){

	bar->SetValue(value);
}

void MyFrame1::CheckDecoderSrv(){

	DWORD p[500];
	//char path[_MAX_PATH];
	DWORD np;
	HANDLE h;
	WCHAR fname[300];
 	wxString filename;
	int found = 1; // not found
	long res;

	EnumProcesses(p, 500, &np); 
	for (int i =0; i < 500; i++){
		h = OpenProcess(PROCESS_ALL_ACCESS, FALSE, p[i]);
		GetProcessImageFileName(h, fname, 100);
		filename.Printf(_T("%s"), fname);
		found = filename.AfterLast(_T('\\')).Cmp(_T("Decoder.exe"));
		if (found == 0)
			return;
	}
	wxSetWorkingDirectory(_(".\\Decoder"));
	filename.Printf(_T("%s\\Decoder.exe"), wxGetCwd());
//		wxLogMessage(filename);
	if ( !wxFileExists(filename) || (res=wxExecute(filename))==0)
		wxLogError(_("Failed to launch the server !"));

}

void MyFrame1::ProcessMessage(wxString msg){

		//wxString city = _("default");
		//wxString text = msg.Trim(true);
		//if (msg.IsSameAs(_("АЛМАТЫ")))
		//		city = _("ALA");
		//else if (msg.IsSameAs(_("ПЕТРОПАВЛОВСК")))
		//		city = _("PET");
		//else if (msg.IsSameAs(_("МАНГЫШЛАК")))
		//		city = _("MAN");
		//else if (msg.IsSameAs(_("ПАВЛОДАР")))
		//		city = _("PAV");
		//else if (msg.IsSameAs(_("УРУМЧИ")))
		//		city = _("URU");
		//else if (msg.IsSameAs(_("КОСТАНАЙ")))
		//		city = _("KOS");
		//else if (msg.IsSameAs(_("КЫЗЫЛОРДА")))
		//		city = _("KZL");
		//else if (msg.IsSameAs(_("КИЕВ")))
		//		city = _("URU");

		//if (city != _("default")){
		//	audioIO->ResetTimer();
		//	Exec(wxString::Format(_("cmd /c tell.bat %s"), city));
		//}
		////int t = audioIO->getTimer();
		//if ( audioIO->getTimer() > TIMEOUT ) {
		//	audioIO->ResetTimer();
		//	Exec(_("cmd /c tell.bat intro"));
		//}
}




void MyFrame1::SetFlag(bool flag){
	bar->SetFlag(flag);
}

void MyFrame1::ResetTimer(){ bar->ResetTimer(); }

void MyFrame1::StopTimer(){ bar->StopTimer(); }



void MyFrame1::OnClear(wxCommandEvent& WXUNUSED(event)){

	m_textbox->Clear();
}

void MyFrame1::OnShutdown(wxCommandEvent& WXUNUSED(event)){

	decoder->Stop();
} 

void MyFrame1::OnHelp(wxCommandEvent& WXUNUSED(event)){

	Exec( wxString::Format(_T("cmd /c \"%s\\help.chm\""), wxString::FromAscii(launchdir)));
} 

MyStatusBar::MyStatusBar(wxWindow *parent)
: wxStatusBar(parent, wxID_ANY), m_timer(this)
{
    static const int widths[5] = { -1, 100, 100, 100, 100 };

    SetFieldsCount(5);
    SetStatusWidths(5, widths);
	SetStatusText(_("Recording level: "), 1);
	recGauge = new wxGauge( this, 1000, 2000, wxPoint(4,4), wxSize(60, 10), wxGA_HORIZONTAL | wxNO_BORDER );
	recGauge->SetForegroundColour(*wxGREEN);
	m_statbmp = new wxBitmapButton(this, wxID_ANY, CreateBitmapForButton(true), wxPoint(4,4), wxDefaultSize, wxBU_EXACTFIT);
	time = 0;
	incr = 0;
	m_timer.Start(1000);
    UpdateClock();
}

#ifdef __VISUALC__
    #pragma warning(default: 4355)
#endif

MyStatusBar::~MyStatusBar()
{
  if ( m_timer.IsRunning() )
    {
        m_timer.Stop();
    }
}

void MyStatusBar::OnSize(wxSizeEvent& event)
{
    wxRect rect;
    GetFieldRect(2, rect);
    recGauge->SetSize(rect.x + 2, rect.y + 2, rect.width - 4, rect.height - 4);

	GetFieldRect(3, rect);
    wxSize size = m_statbmp->GetSize();

    m_statbmp->Move(rect.x + (rect.width - size.x) / 2,  rect.y + (rect.height - size.y) / 2);
    event.Skip();
}

void MyStatusBar::SetValue(int value){
	recGauge->SetValue(value);
}

void MyStatusBar::SetFlag(bool flag) { 
	m_statbmp->SetBitmapLabel(CreateBitmapForButton(flag));
	m_statbmp->Refresh();
}

wxBitmap MyStatusBar::CreateBitmapForButton(bool on)
{
    static const int BMP_BUTTON_SIZE_X = 10;
    static const int BMP_BUTTON_SIZE_Y = 9;

    wxBitmap bitmap(BMP_BUTTON_SIZE_X, BMP_BUTTON_SIZE_Y);
    wxMemoryDC dc;
    dc.SelectObject(bitmap);
    dc.SetBrush(on ? *wxGREEN_BRUSH : *wxRED_BRUSH);
    dc.SetBackground(*wxLIGHT_GREY_BRUSH);
    dc.Clear();
    dc.DrawEllipse(0, 0, BMP_BUTTON_SIZE_X, BMP_BUTTON_SIZE_Y);
    dc.SelectObject(wxNullBitmap);

    return bitmap;
}

void MyStatusBar::UpdateClock()
{
	time += incr;
	SetStatusText(wxString::Format(_("  %d sec."), time), 4);

}


void MyStatusBar::ResetTimer(){ 
	time = 0; 
	incr = 1;
}

void MyStatusBar::StopTimer() { 

	incr = 0;
}

