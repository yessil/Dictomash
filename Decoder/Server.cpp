// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

// Settings common to both executables: determines whether
// we're using TCP/IP or real DDE.
#include "ipcsetup.h"

#if defined(__WXGTK__) || defined(__WXX11__) || defined(__WXMOTIF__) || defined(__WXMAC__)
    #include "mondrian.xpm"
#endif

#include "server.h"
#include "wx/datetime.h"


// ----------------------------------------------------------------------------
// MyServer
// ----------------------------------------------------------------------------

MyServer::MyServer() : wxServer()
{
    m_connection = NULL;
}

MyServer::~MyServer()
{
    Disconnect();
}

wxConnectionBase *MyServer::OnAcceptConnection(const wxString& topic)
{

    if ( topic == IPC_TOPIC )
    {
        m_connection = new MyConnection(this);
        return m_connection;
    }
    // unknown topic
    return NULL;
}

void MyServer::Disconnect()
{
    if (m_connection)
    {
        m_connection->Disconnect();
        delete m_connection;
        m_connection = NULL;
    }
}

void MyServer::Advise()
{
    if (CanAdvise())
    {
        wxString s = wxDateTime::Now().Format();
        m_connection->Advise(m_connection->m_sAdvise, (wxChar *)s.c_str());
        s = wxDateTime::Now().FormatTime() + _T(" ") + wxDateTime::Now().FormatDate();
        m_connection->Advise(m_connection->m_sAdvise, (wxChar *)s.c_str(), (s.Length() + 1) * sizeof(wxChar));

        char bytes[3];
        bytes[0] = '1'; bytes[1] = '2'; bytes[2] = '3';
        m_connection->Advise(m_connection->m_sAdvise, (wxChar *)bytes, 3, wxIPC_PRIVATE);
        // this works, but the log treats it as a string now
//        m_connection->Advise(m_connection->m_sAdvise, (wxChar *)bytes, 3, wxIPC_TEXT );
    }
}

// ----------------------------------------------------------------------------
// MyConnection
// ----------------------------------------------------------------------------

MyConnection::MyConnection(MyServer* s)
            : wxConnection()
{
	srv = s;
}

MyConnection::~MyConnection()
{
}

bool MyConnection::OnExecute(const wxString& topic,
    wxChar *data, int size, wxIPCFormat format)
{
    return true;
}

bool MyConnection::OnPoke(const wxString& topic,
    const wxString& item, wxChar *data, int size, wxIPCFormat format)
{
	srv->data = data;
    return wxConnection::OnPoke(topic, item, data, size, format);
}

wxChar *MyConnection::OnRequest(const wxString& topic,
    const wxString& item, int * size, wxIPCFormat format)
{
    wxChar *data;
    if (item == _T("Date"))
    {
        m_sRequestDate = wxDateTime::Now().Format();
        data = (wxChar *)m_sRequestDate.c_str();
        *size = -1;
    }    
    else if (item == _T("Date+len"))
    {
        m_sRequestDate = wxDateTime::Now().FormatTime() + _T(" ") + wxDateTime::Now().FormatDate();
        data = (wxChar *)m_sRequestDate.c_str();
        *size = (m_sRequestDate.Length() + 1) * sizeof(wxChar);
    }    
    else if (item == _T("bytes[3]"))
    {
        data = (wxChar *)m_achRequestBytes;
        m_achRequestBytes[0] = '1'; m_achRequestBytes[1] = '2'; m_achRequestBytes[2] = '3';
        *size = 3;
    }
    else
    {
        data = NULL;
        *size = 0;
    }
     Log(_T("OnRequest"), topic, item, data, *size, format);
    return data;
}

bool MyConnection::OnStartAdvise(const wxString& topic,
                                 const wxString& item)
{
    m_sAdvise = item;
    return true;
}

bool MyConnection::OnStopAdvise(const wxString& topic,
                                 const wxString& item)
{
    m_sAdvise.Empty();
    return true;
}

void MyConnection::Log(const wxString& command, const wxString& topic,
    const wxString& item, wxChar *data, int size, wxIPCFormat format)
{
    wxString s;
    if (topic.IsEmpty() && item.IsEmpty())
        s.Printf(_T("%s("), command.c_str());
    else if (topic.IsEmpty())
        s.Printf(_T("%s(\"%s\","), command.c_str(), item.c_str());
    else if (item.IsEmpty())
        s.Printf(_T("%s(\"%s\","), command.c_str(), topic.c_str());
    else
        s.Printf(_T("%s(\"%s\",\"%s\","), command.c_str(), topic.c_str(), item.c_str());

    if (format == wxIPC_TEXT || format == wxIPC_UNICODETEXT) 
        wxLogMessage(_T("%s\"%s\",%d)"), s.c_str(), data, size);
    else if (format == wxIPC_PRIVATE)
    {
        if (size == 3)
        {
            char *bytes = (char *)data;
            wxLogMessage(_T("%s'%c%c%c',%d)"), s.c_str(), bytes[0], bytes[1], bytes[2], size);
        }
        else
            wxLogMessage(_T("%s...,%d)"), s.c_str(), size);
    }
    else if (format == wxIPC_INVALID) 
        wxLogMessage(_T("%s[invalid data],%d)"), s.c_str(), size);
}

bool MyConnection::Advise(const wxString& item, wxChar *data, int size, wxIPCFormat format)
{
    return wxConnection::Advise(item, data, size, format);
}

bool MyConnection::OnDisconnect()
{
    wxLogMessage(_T("OnDisconnect()"));
    return true;
}


extern "C" bool StartServer(MyServer* srv, wxString port)	// wrapper function
	{
		return srv->Create(port);
	}
