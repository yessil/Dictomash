///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Apr 16 2008)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "DecoderFrame.h"
extern "C" void decode();

///////////////////////////////////////////////////////////////////////////

DecoderFrame::DecoderFrame( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );
	
	this->SetSizer( bSizer1 );
	this->Layout();
	m_menubar1 = new wxMenuBar( 0 );
	m_menu1 = new wxMenu();
	wxMenuItem* m_menuStart;
	m_menuStart = new wxMenuItem( m_menu1, wxID_ANY, wxString( wxT("Start") ) , wxEmptyString, wxITEM_NORMAL );
	m_menu1->Append( m_menuStart );
	
	m_menubar1->Append( m_menu1, wxT("Do") );
	
	this->SetMenuBar( m_menubar1 );
	
	
	// Connect Events
	this->Connect( m_menuStart->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( DecoderFrame::Start ) );
}

DecoderFrame::~DecoderFrame()
{
	// Disconnect Events
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( DecoderFrame::Start ) );
}

void DecoderFrame::Start(wxCommandEvent& WXUNUSED(event)){

	decode();
}
