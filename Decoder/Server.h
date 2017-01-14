#include "wx/ipc.h"
class MyServer;


class MyConnection : public wxConnection
{
public:
    MyConnection(MyServer* srv);
    ~MyConnection();

    virtual bool OnExecute(const wxString& topic, wxChar *data, int size, wxIPCFormat format);
    virtual wxChar *OnRequest(const wxString& topic, const wxString& item, int *size, wxIPCFormat format);
    virtual bool OnPoke(const wxString& topic, const wxString& item, wxChar *data, int size, wxIPCFormat format);
    virtual bool OnStartAdvise(const wxString& topic, const wxString& item);
    virtual bool OnStopAdvise(const wxString& topic, const wxString& item);
    virtual bool Advise(const wxString& item, wxChar *data, int size = -1, wxIPCFormat format = wxIPC_TEXT);
    virtual bool OnDisconnect();
protected:
    void Log(const wxString& command, const wxString& topic, const wxString& item, wxChar *data, int size, wxIPCFormat format);
public:
    wxString        m_sAdvise;
protected:
    wxString        m_sRequestDate;
    char             m_achRequestBytes[3];
	MyServer*	srv;
};

class MyServer: public wxServer
{
public:
    MyServer();
    ~MyServer();
    void Disconnect();
    bool IsConnected() { return m_connection != NULL; };
    MyConnection *GetConnection() { return m_connection; };
    void Advise();
    bool CanAdvise() { return m_connection != NULL && !m_connection->m_sAdvise.IsEmpty(); };
    wxConnectionBase *OnAcceptConnection(const wxString& topic);
	wxChar*		data;

protected:
    MyConnection     *m_connection;
};
