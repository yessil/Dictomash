#pragma once
#include "wx/frame.h"
#include "wx/intl.h"
#include "wx/log.h"
#define FILENAME_LENGHT 30

class BaseFrame :
	public wxFrame
{
public:
	BaseFrame(wxLocale& locale, wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,300 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
	virtual void WriteText(const wxString& text);
	virtual void SetStatusbarText(wxString msg);
	virtual void SetValue(int value);
	virtual void SetFlag(bool flag);
	virtual void ResetTimer();
	virtual void StopTimer();
	bool stop;
	virtual bool IsStop();
	int NOISE_THRESHOLD;
	int MAX_PAUSE_LENGTH;
	int SIGNAL_LEVEL;
	wxString SERVER;
	int PORT;
	bool debug;
	virtual void Exec(wxString cmd);
	wxLogChain *logchain;
	FILE *log;
	wxLocale& m_locale;

};
