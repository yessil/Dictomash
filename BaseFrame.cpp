﻿#include "BaseFrame.h"



BaseFrame::BaseFrame(wxLocale& locale, wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style) : wxFrame(parent, id, title, pos, size, style), m_locale(locale)
{
//	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	stop = false;
}

void BaseFrame::WriteText(const wxString& text) {};
void BaseFrame::SetStatusbarText(wxString msg){};
void BaseFrame::SetValue(int value) {};
void BaseFrame::Exec(wxString cmd) {};
void BaseFrame::SetFlag(bool flag) {};
void BaseFrame::ResetTimer(){};
void BaseFrame::StopTimer() {};
bool BaseFrame::IsStop() {return false; };