/*
 *      ircsocket.cpp
 *
 *      Copyright 2008 David Vachulka <david@konstrukce-cad.com>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */

#include "ircsocket.h"
#include "config.h"
#include "i18n.h"
#include "utils.h"
#include "irctabitem.h"
#ifdef HAVE_OPENSSL
#include <openssl/err.h>
#include <openssl/rand.h>
#include <fstream>
#include <ios>
#endif

#define ETIMEOUT 666 //timeout for event fire

FXDEFMAP(IrcSocket) IrcSocketMap[] = {
    FXMAPFUNC(SEL_IO_READ,      IrcSocket::ID_SOCKET,   IrcSocket::onIORead),
    FXMAPFUNC(SEL_TIMEOUT,      IrcSocket::ID_SSLTIME,  IrcSocket::onIORead),
    FXMAPFUNC(SEL_IO_WRITE,     IrcSocket::ID_SOCKET,   IrcSocket::onIOWrite),
    FXMAPFUNC(SEL_TIMEOUT,      IrcSocket::ID_RTIME,    IrcSocket::onReconnectTimeout),
    FXMAPFUNC(SEL_TIMEOUT,      IrcSocket::ID_PTIME,    IrcSocket::onPositionTimeout),
    FXMAPFUNC(SEL_TIMEOUT,      IrcSocket::ID_CTIME,    IrcSocket::onCloseTimeout),
    FXMAPFUNC(SEL_TIMEOUT,      IrcSocket::ID_ETIME,    IrcSocket::onEventTimeout)
};

FXIMPLEMENT(IrcSocket, FXObject, IrcSocketMap, ARRAYNUMBER(IrcSocketMap))

IrcSocket::IrcSocket(FXApp *app, FXObject *tgt, FXString channels, FXString commands)
    : m_application(app), m_startChannels(utils::instance().removeSpaces(channels)), m_startCommands(commands)
#ifdef HAVE_OPENSSL
        ,m_ctx(NULL), m_ssl(NULL)
#endif
{
    m_targets.append(tgt);
    m_serverName = "localhost";
    m_realServerName = "";
    m_serverPort = 6667;
    m_serverPassword = "";
    m_nickName = "xxx";
    m_userName = m_nickName;
    m_realName = m_nickName;
    m_receiveRest = "";
    m_connected = FALSE;
    m_connecting = FALSE;
    m_endmotd = FALSE;
    m_ignoreUserHost = FALSE;
    m_dccType = DCC_NONE;
    m_dccNick = "";
    m_dccIP = utils::instance().getStringIniEntry("SETTINGS", "dccIP");
    FXRex rex("\\l");
    if(m_dccIP.contains('.')!=3 || rex.match(m_dccIP))
        m_dccIP = "";
    m_dccPortD = utils::instance().getIntIniEntry("SETTINGS", "dccPortD");
    if(m_dccPortD<0 || m_dccPortD>65536) m_dccPortD = 0;
    m_dccPortH = utils::instance().getIntIniEntry("SETTINGS", "dccPortH");
    if(m_dccPortH<0 || m_dccPortH>65536) m_dccPortH = 0;
    if(m_dccPortH<m_dccPortD) m_dccPortH = m_dccPortD;
    m_dccTimeout = utils::instance().getIntIniEntry("SETTINGS", "dccTimeout", 66);
    m_dccFile = DccFile();
    m_chanTypes = "#&+!";
    m_adminPrefix = '!';
    m_ownerPrefix = '*';
    m_opPrefix = '@';
    m_voicePrefix = '+';
    m_halfopPrefix = '%';
    m_attempts = 0;
    m_numberAttempt = 1;
    m_nickLen = 460;
    m_topicLen = 460;
    m_kickLen = 460;
    m_awayLen = 460;
    m_thread = new ConnectThread(this);
}

IrcSocket::~IrcSocket()
{
#ifdef HAVE_OPENSSL
    if(m_ssl)
    {
        SSL_free(m_ssl);
    }
#endif
    delete m_thread;
}

void IrcSocket::setUserName(const FXString& user)
{
    //needed for freenode
    m_userName = utils::instance().removeNonalphanumeric(user);
}

long IrcSocket::onReconnectTimeout(FXObject*, FXSelector, void*)
{
    if(m_attempts < m_numberAttempt && !m_connected)
    {
        if(m_thread->running())
            m_application->addTimeout(this, ID_RTIME, 1000);
        else
        {
            sendEvents();
            startConnection();
            m_application->addTimeout(this, ID_RTIME, m_delayAttempt*1000);
        }
    }
    else
        clearAttempts();
    return 1;
}

long IrcSocket::onPositionTimeout(FXObject*, FXSelector, void*)
{
    sendEvent(IRC_DCCPOSITION, m_dccFile);
    if(m_dccFile.currentPosition < m_dccFile.size && m_connected)
        m_application->addTimeout(this, ID_PTIME, 1000);
    return 1;
}

//for offered connectin to me
long IrcSocket::onCloseTimeout(FXObject*, FXSelector, void*)
{
    if(m_dccType != DCC_CHATOUT)
    {
        m_dccFile.canceled = TRUE;
        sendEvent(IRC_DCCPOSITION, m_dccFile);
    }
    if(!m_connected)
    {
#ifdef WIN32
        shutdown(m_serverid, SD_BOTH);
        closesocket(m_serverid);
#else
#ifndef SHUT_RDWR
#define SHUT_RDWR 2
#endif
        shutdown(m_serverid, SHUT_RDWR);
        close(m_serverid);
#endif
        sendEvent(IRC_DISCONNECT, _("Connection closed. Client didn't connect in given timeout"));
        closeConnection(TRUE);
    }
    return 1;
}

//fired events stored during thread
long IrcSocket::onEventTimeout(FXObject*, FXSelector, void*)
{
    sendEvents();
    return 1;
}

long IrcSocket::onIORead(FXObject *, FXSelector, void *)
{
#ifdef WIN32
    if(m_useSsl)
        m_application->addTimeout(this, ID_SSLTIME, 100);
#endif
    if(m_connected)
    {
        if(m_dccType == DCC_IN || m_dccType == DCC_PIN)
            readFileData();
        else
            readData();
    }
    return 1;
}

long IrcSocket::onIOWrite(FXObject*, FXSelector, void*)
{
    sendFile();
    return 1;
}

void IrcSocket::startConnection()
{
    FXASSERT(m_thread);
    utils::instance().debugLine("StartConnection");
    utils::instance().debugLine(FXStringFormat("Attempts on %s-%d-%s: %d", m_serverName.text(), m_serverPort, m_nickName.text(), m_attempts));
    m_connecting = TRUE;
    sendEvent(IRC_CONNECT, FXStringFormat(_("Connecting to %s"), m_serverName.text()));
    if(!m_thread->running())
    {
        m_attempts++;
        m_thread->start();
    }
}

void IrcSocket::startListening(const  FXString &nick, IrcSocket *server)
{
    FXASSERT(m_thread);
    utils::instance().debugLine("StartListening");
    m_connecting = TRUE;
    m_dccNick = nick;
    m_dccParent = server;
    if(!m_thread->running())
        m_thread->start();
}

FXint IrcSocket::connectIRC()
{
    utils::instance().debugLine("Connect");
    utils::instance().debugLine(FXStringFormat("startChannels: %s", m_startChannels.text()));
    utils::instance().debugLine(FXStringFormat("startCommands: %s", m_startCommands.text()));
    if(m_connected)
    {
        m_application->addTimeout(this, ID_ETIME, ETIMEOUT);
        return 1;
    }
    m_endmotd = FALSE;
#ifdef WIN32
    WORD wVersionRequested = MAKEWORD(2,0);
    WSADATA data;
#endif
    hostent *host;
#ifdef WIN32
    if (WSAStartup(wVersionRequested, &data) != 0)
    {
        sendEvent(IRC_ERROR, _("Unable to initiliaze socket"));
        clearChannelsCommands(FALSE);
        m_connecting = FALSE;
        m_application->addTimeout(this, ID_ETIME, ETIMEOUT);
        resetReconnect();
        return -1;
    }
#endif
    if ((host = gethostbyname(m_serverName.text())) == NULL)
    {
        sendEvent(IRC_ERROR, FXStringFormat(_("Bad host: %s"), m_serverName.text()));
        clearChannelsCommands(FALSE);
        m_connecting = FALSE;
        m_application->addTimeout(this, ID_ETIME, ETIMEOUT);
        resetReconnect();
        return -1;
    }
    if ((m_serverid = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    {
        sendEvent(IRC_ERROR, _("Unable to create socket"));
        clearChannelsCommands(FALSE);
#ifdef WIN32
        WSACleanup();
#endif
        m_connecting = FALSE;
        m_application->addTimeout(this, ID_ETIME, ETIMEOUT);
        resetReconnect();
        return -1;
    }
    if(m_dccType == DCC_IN)
    {
        
        if(m_dccFile.currentPosition)
        {
            m_receivedFile.open(FXString(m_dccFile.path+".part").text(), std::ios_base::binary|std::ios_base::ate|std::ios_base::app);
        }
        else
            m_receivedFile.open(FXString(m_dccFile.path+".part").text(), std::ios_base::binary);
    }
    m_serverSock.sin_family = AF_INET;
    m_serverSock.sin_port = htons(m_serverPort);
    memcpy(&(m_serverSock.sin_addr), host->h_addr, host->h_length);
    if (connect(m_serverid, (sockaddr *)&m_serverSock, sizeof(m_serverSock)) == -1)
    {
        sendEvent(IRC_ERROR, FXStringFormat(_("Unable to connect to: %s"), m_serverName.text()));
        clearChannelsCommands(FALSE);
#ifdef WIN32
        closesocket(m_serverid);
#else
        close(m_serverid);
#endif
        m_connecting = FALSE;
        m_receivedFile.close();
        m_application->addTimeout(this, ID_ETIME, ETIMEOUT);
        resetReconnect();
        return -1;
    }
#ifdef WIN32
    m_event = WSACreateEvent();
    WSAEventSelect(m_serverid, m_event, FD_CONNECT|FD_READ|FD_CLOSE); // sets non-blocking!!
    if(m_dccType == DCC_POUT)
        m_application->addInput((FXInputHandle)m_event, INPUT_READ|INPUT_WRITE, this, ID_SOCKET);
    else
        m_application->addInput((FXInputHandle)m_event, INPUT_READ, this, ID_SOCKET);
#else
    if(m_dccType == DCC_POUT)
        m_application->addInput((FXInputHandle)m_serverid, INPUT_READ|INPUT_WRITE, this, ID_SOCKET);
    else
        m_application->addInput((FXInputHandle)m_serverid, INPUT_READ, this, ID_SOCKET);
#endif
    m_connected = TRUE;
    m_connecting = FALSE;
    if(m_dccType == DCC_IN) m_application->addTimeout(this, ID_PTIME, 1000);
    if(m_dccType == DCC_POUT)
    {
        m_sentFile.open(m_dccFile.path.text(), std::ios_base::binary);
        if(m_dccFile.currentPosition) m_sentFile.seekg(m_dccFile.currentPosition);
        sendFile();
        m_application->addTimeout(this, ID_PTIME, 1000);
    }
    clearAttempts();
    sendEvent(IRC_CONNECT, FXStringFormat(_("Connected to %s"), m_serverName.text()));
    if(!m_serverPassword.empty() && m_dccType != DCC_CHATIN && m_dccType != DCC_IN && m_dccType != DCC_POUT)
        sendLine("PASS "+m_serverPassword+"\r\n");
    if(m_dccType != DCC_CHATIN && m_dccType != DCC_IN && m_dccType != DCC_POUT)
        sendLine("NICK "+m_nickName+"\r\n");
    if(m_dccType != DCC_CHATIN && m_dccType != DCC_IN && m_dccType != DCC_POUT)
        sendLine("USER "+m_userName+" 0 * :"+m_realName+"\r\n");
    m_application->addTimeout(this, ID_ETIME, ETIMEOUT);
    return 1;
}

FXint IrcSocket::connectSSL()
{
    utils::instance().debugLine("ConnectSSL");
    utils::instance().debugLine(FXStringFormat("startChannels: %s", m_startChannels.text()));
    utils::instance().debugLine(FXStringFormat("startCommands: %s", m_startCommands.text()));
    if(m_connected)
    {
        m_application->addTimeout(this, ID_ETIME, ETIMEOUT);
        return 1;
    }
    m_endmotd = FALSE;
#ifdef HAVE_OPENSSL
#ifdef WIN32
    WORD wVersionRequested = MAKEWORD(2,0);
    WSADATA data;
#endif
    hostent *host;
#ifdef WIN32
    if (WSAStartup(wVersionRequested, &data) != 0)
    {
        sendEvent(IRC_ERROR, _("Unable to initiliaze socket"));
        clearChannelsCommands(FALSE);
        m_connecting = FALSE;
        m_application->addTimeout(this, ID_ETIME, ETIMEOUT);
        resetReconnect();
        return -1;
    }
#endif
    if ((host = gethostbyname(m_serverName.text())) == NULL)
    {
        sendEvent(IRC_ERROR, FXStringFormat(_("Bad host: %s"), m_serverName.text()));
        clearChannelsCommands(FALSE);
        m_connecting = FALSE;
        m_application->addTimeout(this, ID_ETIME, ETIMEOUT);
        resetReconnect();
        return -1;
    }
    if ((m_serverid = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    {
        sendEvent(IRC_ERROR, _("Unable to create socket"));
        clearChannelsCommands(FALSE);
#ifdef WIN32
        WSACleanup();
#endif
        m_connecting = FALSE;
        m_application->addTimeout(this, ID_ETIME, ETIMEOUT);
        resetReconnect();
        return -1;
    }
    m_serverSock.sin_family = AF_INET;
    m_serverSock.sin_port = htons(m_serverPort);
    memcpy(&(m_serverSock.sin_addr), host->h_addr, host->h_length);
    if (connect(m_serverid, (sockaddr *)&m_serverSock, sizeof(m_serverSock)) == -1)
    {
        sendEvent(IRC_ERROR, FXStringFormat(_("Unable to connect to: %s"), m_serverName.text()));
        clearChannelsCommands(FALSE);
#ifdef WIN32
        closesocket(m_serverid);
#else
        close(m_serverid);
#endif
        m_connecting = FALSE;
        m_application->addTimeout(this, ID_ETIME, ETIMEOUT);
        resetReconnect();
        return -1;
    }
    SSLeay_add_ssl_algorithms();
    SSL_load_error_strings();    
    m_ctx = SSL_CTX_new(SSLv3_client_method());
    SSL_CTX_set_options(m_ctx, SSL_OP_ALL);
#ifdef WIN32
    int i,r;
    for(i=0; i<128; i++)
    {
        r = rand();
        RAND_seed((unsigned char *)&r, sizeof(r));
    }
#endif
    m_ssl = SSL_new(m_ctx);
    if(!m_ssl)
    {
#ifdef WIN32
        shutdown(m_serverid, SD_BOTH);
        closesocket(m_serverid);
#else
#ifndef SHUT_RDWR
#define SHUT_RDWR 2
#endif
        shutdown(m_serverid, SHUT_RDWR);
        close(m_serverid);
#endif
        sendEvent(IRC_ERROR, _("SSL creation error"));
        m_connecting = FALSE;
        m_application->addTimeout(this, ID_ETIME, ETIMEOUT);
        resetReconnect();
        return -1;
    }
    SSL_set_fd(m_ssl, m_serverid);
    m_err = SSL_connect(m_ssl);
    if(!m_err)
    {
#ifdef WIN32
        shutdown(m_serverid, SD_BOTH);
        closesocket(m_serverid);
#else
#ifndef SHUT_RDWR
#define SHUT_RDWR 2
#endif
        shutdown(m_serverid, SHUT_RDWR);
        close(m_serverid);
#endif
        sendEvent(IRC_ERROR, FXStringFormat(_("SSL connect error %d"), SSL_get_error(m_ssl, m_err)));
        m_connecting = FALSE;
        m_application->addTimeout(this, ID_ETIME, ETIMEOUT);
        resetReconnect();
        return -1;
    }
    m_connected = TRUE;
    clearAttempts();
    sendEvent(IRC_CONNECT, FXStringFormat(_("Connected to %s"), m_serverName.text()));
    if (!m_serverPassword.empty()) sendLine("PASS "+m_serverPassword+"\r\n");
    sendLine("NICK "+m_nickName+"\r\n");
    sendLine("USER "+m_userName+" 0 * :"+m_realName+"\r\n");
#ifdef WIN32
    m_event = WSACreateEvent();
    WSAEventSelect(m_serverid, m_event, FD_CONNECT|FD_READ|FD_CLOSE); // sets non-blocking!!
    /* now here 'dirty' solution with timeout
    application->addInput((FXInputHandle)m_event, INPUT_READ, this, ID_READ);
    */
    m_application->addTimeout(this, ID_SSLTIME, 100);
#else
    m_application->addInput((FXInputHandle)m_serverid, INPUT_READ, this, ID_SOCKET);
#endif
#else
    m_connected = FALSE;
#endif //HAVE_OPENSSL    
    m_connecting = FALSE;
    m_application->addTimeout(this, ID_ETIME, ETIMEOUT);
    return 1;
}

//Start listening for DCC
FXint IrcSocket::listenIRC()
{
    utils::instance().debugLine("Listen");
    if(m_connected)
    {
        m_connecting = FALSE;
        m_application->addTimeout(this, ID_ETIME, ETIMEOUT);
        return 1;
    }
    FXint i, bindResult = -1;
#ifdef WIN32
    WORD wVersionRequested = MAKEWORD(2,0);
    WSADATA data;
    FXint addrlen;
#else
    socklen_t addrlen;
#endif    
#ifdef WIN32
    if (WSAStartup(wVersionRequested, &data) != 0)
    {
        sendEvent(IRC_ERROR, _("Unable to initiliaze socket"));
        clearChannelsCommands(FALSE);
        m_connecting = FALSE;
        m_application->addTimeout(this, ID_ETIME, ETIMEOUT);
        return -1;
    }
#endif
    if ((m_serverid = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    {
        sendEvent(IRC_ERROR, _("Unable to create socket"));
        clearChannelsCommands(FALSE);
#ifdef WIN32
        WSACleanup();
#endif
        m_connecting = FALSE;
        m_application->addTimeout(this, ID_ETIME, ETIMEOUT);
        return -1;
    }
    m_serverSock.sin_family = AF_INET;
    m_serverSock.sin_addr.s_addr = INADDR_ANY;
    if(m_dccPortD > 0)
    {
        m_serverSock.sin_port = 0;
        i = 0;
        while(m_dccPortH > ntohs(m_serverSock.sin_port) && bindResult == -1)
        {
            m_serverSock.sin_port = htons(m_dccPortD+i);
            i++;
            bindResult = bind(m_serverid, (sockaddr *)&m_serverSock, sizeof(m_serverSock));
        }
        if (bindResult == -1)
        {
            sendEvent(IRC_ERROR, _("Unable to bind socket"));
#ifdef WIN32
            WSACleanup();
#endif
            m_connecting = FALSE;
            m_application->addTimeout(this, ID_ETIME, ETIMEOUT);
            return -1;
        }
    }
    else
    {
        m_serverSock.sin_port = 0;
        if (bind(m_serverid, (sockaddr *)&m_serverSock, sizeof(m_serverSock)) == -1)
        {
            sendEvent(IRC_ERROR, _("Unable to bind socket"));
#ifdef WIN32
            WSACleanup();
#endif
            m_connecting = FALSE;
            m_application->addTimeout(this, ID_ETIME, ETIMEOUT);
            return -1;
        }
    }
#ifdef WIN32
    FXint len = sizeof(m_serverSock);
#else
    socklen_t len = sizeof(m_serverSock);
#endif
    getsockname(m_serverid, (sockaddr *)&m_serverSock, &len);
    m_serverPort = ntohs(m_serverSock.sin_port);
    sendEvent(IRC_CONNECT, FXStringFormat(_("Listening on %s-%d"), m_serverName.text(), m_serverPort));
    if(!isRoutableIP(stringIPToBinary(m_serverName)))
    {
        sendEvent(IRC_ERROR, FXStringFormat(_("%s isn't routable."), m_serverName.text()));
        if(!m_dccParent->getDccIP().empty() || !m_dccIP.empty())
        {
            if(!m_dccIP.empty())
            {
                m_serverName = m_dccIP;
                sendEvent(IRC_CONNECT, FXStringFormat(_("Trying IP from settings %s"), m_serverName.text()));
            }
            else
            {
                m_serverName = m_dccParent->getDccIP();
                sendEvent(IRC_CONNECT, FXStringFormat(_("Trying IP from server %s"), m_serverName.text()));
            }
            if(!isRoutableIP(stringIPToBinary(m_serverName)))
                sendEvent(IRC_ERROR, FXStringFormat(_("%s isn't routable too, but will be used"), m_serverName.text()));
        }
        else
            sendEvent(IRC_ERROR, _("But better doesn't exist."));
    }
    if(m_dccParent->getConnected())
    {
        if(m_dccType == DCC_CHATOUT) m_dccParent->sendCtcp(m_dccNick, "DCC CHAT chat "+FXStringVal(stringIPToBinary(m_serverName))+" "+FXStringVal(m_serverPort));
        else if(m_dccType == DCC_OUT)
        {
            m_dccParent->sendCtcp(m_dccNick, "DCC SEND "+utils::instance().removeSpaces(m_dccFile.path.rafter(PATHSEP))+" "+FXStringVal(stringIPToBinary(m_serverName))+" "+FXStringVal(m_serverPort)+" "+FXStringVal(m_dccFile.size));
            m_dccFile.ip = m_serverName;
            m_dccFile.port = m_serverPort;
            sendEvent(IRC_DCCPOSITION, m_dccFile);
        }
        else if(m_dccType == DCC_PIN)
        {
            m_dccParent->sendCtcp(m_dccNick, "DCC SEND "+utils::instance().removeSpaces(m_dccFile.path.rafter(PATHSEP))+" "+FXStringVal(stringIPToBinary(m_serverName))+" "+FXStringVal(m_serverPort)+" "+FXStringVal(m_dccFile.size)+" "+FXStringVal(m_dccFile.token));
            m_dccFile.ip = m_serverName;
            m_dccFile.port = m_serverPort;
            sendEvent(IRC_DCCPOSITION, m_dccFile);
        }
    }
    else
    {
        m_connecting = FALSE;
        m_application->addTimeout(this, ID_ETIME, ETIMEOUT);
        return -1;
    }
    if(listen(m_serverid, 1) == -1)
    {
        sendEvent(IRC_ERROR, _("Unable to listen"));
#ifdef WIN32
        WSACleanup();
#endif
        m_connecting = FALSE;
        m_application->addTimeout(this, ID_ETIME, ETIMEOUT);
        return -1;
    }
    m_application->addTimeout(this, ID_CTIME, m_dccTimeout*1000);
    addrlen = sizeof(m_clientSock);
    m_clientid = accept(m_serverid, (sockaddr *)&m_clientSock, &addrlen);
    if(m_clientid == -1)
    {
        sendEvent(IRC_ERROR, _("Unable to accept connection"));
#ifdef WIN32
        WSACleanup();
#endif
        m_connecting = FALSE;
        m_application->addTimeout(this, ID_ETIME, ETIMEOUT);
        return -1;
    }
#ifdef WIN32
    shutdown(m_serverid, SD_BOTH);
    closesocket(m_serverid);
#else
#ifndef SHUT_RDWR
#define SHUT_RDWR 2
#endif
    shutdown(m_serverid, SHUT_RDWR);
    close(m_serverid);
#endif
    sendEvent(IRC_CONNECT, FXStringFormat(_("Someone connected from %s"), inet_ntoa((in_addr)m_clientSock.sin_addr)));
#ifdef WIN32
    m_event = WSACreateEvent();
    WSAEventSelect(m_clientid, m_event, FD_CONNECT|FD_READ|FD_WRITE|FD_CLOSE); // sets non-blocking!!
    if(m_dccType == DCC_CHATOUT || m_dccType == DCC_PIN) m_application->addInput((FXInputHandle)m_event, INPUT_READ, this, ID_SOCKET);
    else m_application->addInput((FXInputHandle)m_event, INPUT_READ|INPUT_WRITE, this, ID_SOCKET);
#else
    if(m_dccType == DCC_CHATOUT || m_dccType == DCC_PIN) m_application->addInput((FXInputHandle)m_clientid, INPUT_READ, this, ID_SOCKET);
    else m_application->addInput((FXInputHandle)m_clientid, INPUT_READ|INPUT_WRITE, this, ID_SOCKET);
#endif
    m_connected = TRUE;
    m_connecting = FALSE;
    m_application->removeTimeout(this, ID_CTIME);
    if(m_dccType == DCC_OUT)
    {
        m_sentFile.open(m_dccFile.path.text(), std::ios_base::binary);
        if(m_dccFile.currentPosition) m_sentFile.seekg(m_dccFile.currentPosition);
        sendFile();
        m_application->addTimeout(this, ID_PTIME, 1000);
    }
    if(m_dccType == DCC_PIN)
    {
        m_receivedFile.open(FXString(m_dccFile.path+".part").text(), std::ios_base::binary);
        m_application->addTimeout(this, ID_PTIME, 1000);
    }
    m_application->addTimeout(this, ID_ETIME, ETIMEOUT);
    return 1;
}

void IrcSocket::disconnect()
{
    utils::instance().debugLine(FXStringFormat("Quit: %s-%d-%s", m_serverName.text(), m_serverPort, m_nickName.text()));
    if(m_connected) sendLine(m_dccType == DCC_CHATIN ? "QUIT\n" : "QUIT\r\n");
    closeConnection(TRUE);
}

void IrcSocket::disconnect(const FXString& reason)
{
    utils::instance().debugLine(FXStringFormat("Quit: %s-%d-%s", m_serverName.text(), m_serverPort, m_nickName.text()));
    if(m_connected) sendLine("QUIT :"+reason+(m_dccType == DCC_CHATIN ? "\n" : "\r\n"));
    closeConnection(TRUE);
}

void IrcSocket::closeConnection(FXbool disableReconnect)
{
    sendEvents();
    FXbool client = m_dccType == DCC_CHATOUT || m_dccType == DCC_OUT || m_dccType == DCC_PIN;
    clearChannelsCommands(disableReconnect);
    if(!m_connected && disableReconnect)
    {
        m_application->removeTimeout(this, ID_RTIME);
        return;
    }
    m_connected = FALSE;
#ifdef WIN32
    shutdown(client ? m_clientid : m_serverid, SD_BOTH);
    closesocket(client ? m_clientid : m_serverid);
    if(m_event)
    {
        m_application->removeInput((FXInputHandle)m_event, INPUT_READ);
        if(m_dccType == DCC_OUT || m_dccType == DCC_POUT) m_application->removeInput((FXInputHandle)m_event, INPUT_WRITE);
        WSACloseEvent(m_event);
        m_event = NULL;
    }
#else
#ifndef SHUT_RDWR
#define SHUT_RDWR 2
#endif
    shutdown(client ? m_clientid : m_serverid, SHUT_RDWR);
    close(client ? m_clientid : m_serverid);
    m_application->removeInput(client ? m_clientid : m_serverid, INPUT_READ);
    if(m_dccType == DCC_OUT || m_dccType == DCC_POUT) m_application->removeInput(client ? m_clientid : m_serverid, INPUT_WRITE);
#endif
#ifdef HAVE_OPENSSL
    if(m_useSsl && m_ssl)
    {
        SSL_shutdown(m_ssl);
        if(m_ssl)
        {            
            SSL_free(m_ssl);
            m_ssl = NULL;
        }
    }
#endif
    if(m_dccType == DCC_CHATOUT)
    {
        sendEvent(IRC_DISCONNECT, _("Client was disconnected"));
        return;
    }
    if(m_dccType == DCC_IN)
    {
        sendEvent(IRC_DISCONNECT, FXStringFormat(_("%s closed DCC file %s connection %s-%d"), m_dccFile.nick.text(), m_dccFile.path.text(), m_serverName.text(), m_serverPort));
        return;
    }
    if(m_reconnect && m_attempts < m_numberAttempt && !disableReconnect)
    {
        sendEvent(IRC_RECONNECT, FXStringFormat(_("Server %s was disconnected"), m_serverName.text()));
        m_application->addTimeout(this, ID_RTIME, m_delayAttempt*1000);
    }
    else
    {
        m_application->removeTimeout(this, ID_RTIME);
        sendEvent(IRC_DISCONNECT, FXStringFormat(_("Server %s was disconnected"), m_serverName.text()));
    }
}

void IrcSocket::closeDccfileConnection(DccFile file)
{
    if(m_dccFile == file)
    {
        if(m_dccType == DCC_IN)
        {
            if(m_dccFile.currentPosition < m_dccFile.size)
                m_dccFile.canceled = TRUE;
            m_receivedFile.close();
            if(m_dccFile.currentPosition >= m_dccFile.size)
                FXFile::rename(FXString(m_dccFile.path+".part"), m_dccFile.path);
        }
        if(m_dccType == DCC_OUT)
        {
            if(m_dccFile.finishedPosition < m_dccFile.size)
                m_dccFile.canceled = TRUE;
            m_sentFile.close();
        }
        if(m_dccType == DCC_PIN)
        {
            if(m_dccFile.finishedPosition < m_dccFile.size)
                m_dccFile.canceled = TRUE;
            m_receivedFile.close();
            if(m_dccFile.finishedPosition >= m_dccFile.size)
                FXFile::rename(FXString(m_dccFile.path+".part"), m_dccFile.path);
        }
        if(m_dccType == DCC_POUT)
        {
            if(m_dccFile.currentPosition < m_dccFile.size)
                m_dccFile.canceled = TRUE;
            m_sentFile.close();
        }
        sendEvent(IRC_DCCPOSITION, m_dccFile);
        closeConnection(TRUE);
    }
}

//change position in file mainly for resumed file
void IrcSocket::setCurrentPostion(FXulong position)
{
    m_dccFile.currentPosition = position;
    m_dccFile.previousPostion = position;
    m_dccFile.finishedPosition = position;
}

//check dccfile for resume
FXbool IrcSocket::isForResume(const FXString& nick, const FXString& name, FXint port)
{
    return m_dccFile.nick == nick && FXPath::name(m_dccFile.path) == name
                && m_dccFile.port == port;
}

//check dccfile for resume
FXbool IrcSocket::isForResume(FXint token)
{
    return m_dccFile.token == token;
}

void IrcSocket::readFileData()
{
    FXchar buffer[4096];
    int size = 0;
    FXbool client = m_dccType == DCC_PIN;
#ifdef WIN32
    WSANETWORKEVENTS network_events;
    WSAEnumNetworkEvents(client? m_clientid : m_serverid, m_event, &network_events);
    if (network_events.lNetworkEvents&FD_READ)
    {
        size = recv(client? m_clientid : m_serverid, buffer, 4095, 0);
        if (size > 0)
        {
            m_dccFile.currentPosition += size;
            if(m_receivedFile.good())
                m_receivedFile.write(buffer, size);
            FXlong pos = htonl(m_dccFile.currentPosition);
            send(client? m_clientid : m_serverid, reinterpret_cast<char *>(&pos), 4, 0);
            if(m_dccFile.currentPosition >= m_dccFile.size)
            {
                m_receivedFile.close();
                FXFile::rename(FXString(m_dccFile.path+".part"), m_dccFile.path);
                closeConnection(TRUE);
            }
        }
        else if (size < 0)
        {
            sendEvent(IRC_ERROR, FXStringFormat(_("Error in reading DCC data from %s"), m_serverName.text()));
            m_application->removeTimeout(this, ID_PTIME);
            m_dccFile.canceled = TRUE;
            sendEvent(IRC_DCCPOSITION, m_dccFile);
            m_receivedFile.close();
            closeConnection(TRUE);
        }
        else
        {
            m_application->removeTimeout(this, ID_PTIME);
            m_dccFile.canceled = TRUE;
            sendEvent(IRC_DCCPOSITION, m_dccFile);
            m_receivedFile.close();
            closeConnection(TRUE);
        }
    }
    //else if (network_events.lNetworkEvents&FD_CONNECT) ;
    else if (network_events.lNetworkEvents&FD_CLOSE)
    {
        m_application->removeTimeout(this, ID_PTIME);
        m_dccFile.canceled = TRUE;
        sendEvent(IRC_DCCPOSITION, m_dccFile);
        m_receivedFile.close();
        closeConnection(TRUE);
    }
#else
    size = recv(client? m_clientid : m_serverid, buffer, 4095, 0);
    if (size > 0)
    {
        m_dccFile.currentPosition += size;
        if(m_receivedFile.good())
            m_receivedFile.write(buffer, size);
        FXulong pos = htonl(m_dccFile.currentPosition);
        if(m_connected) send(client? m_clientid : m_serverid, reinterpret_cast<char *>(&pos), 4, 0);
        if(m_dccFile.currentPosition >= m_dccFile.size)
        {
            m_receivedFile.close();
            FXFile::rename(FXString(m_dccFile.path+".part"), m_dccFile.path);
            closeConnection(TRUE);
        }
    }
    else if (size < 0)
    {
        sendEvent(IRC_ERROR, FXStringFormat(_("Error in reading DCC data from %s"), m_serverName.text()));
        m_application->removeTimeout(this, ID_PTIME);
        m_dccFile.canceled = TRUE;
        sendEvent(IRC_DCCPOSITION, m_dccFile);
        m_receivedFile.close();
        closeConnection(TRUE);
    }
    else
    {
        m_application->removeTimeout(this, ID_PTIME);
        m_dccFile.canceled = TRUE;
        sendEvent(IRC_DCCPOSITION, m_dccFile);
        m_receivedFile.close();
        closeConnection(TRUE);
    }
#endif
}

void IrcSocket::sendFile()
{
    FXbool client = m_dccType == DCC_OUT;
    FXchar buf[1024];
    if(m_sentFile.good())
        m_sentFile.read(buf, 1024);
    int readedChars = (int)m_sentFile.gcount();
    int size = 0;
    if(m_connected && m_dccFile.currentPosition < m_dccFile.size)
    {
        if((size = send(client ? m_clientid : m_serverid, buf, FXMIN(1024, m_dccFile.size-m_dccFile.currentPosition), 0)) == -1)
        {
            if(m_dccFile.finishedPosition < m_dccFile.size)
                m_dccFile.canceled = TRUE;
            m_application->removeTimeout(this, ID_PTIME);
            sendEvent(IRC_DCCPOSITION, m_dccFile);
            m_sentFile.close();
#ifdef WIN32
            m_application->removeInput((FXInputHandle)m_event, INPUT_WRITE);
#else
            m_application->removeInput(client ? m_clientid : m_serverid, INPUT_WRITE);
#endif
            closeConnection(TRUE);
        }
        else
        {
            m_dccFile.currentPosition += readedChars;
            if(m_dccFile.currentPosition >= m_dccFile.size)
            {
                m_sentFile.close();
#ifdef WIN32
                m_application->removeInput((FXInputHandle)m_event, INPUT_WRITE);
#else
                m_application->removeInput(client ? m_clientid : m_serverid, INPUT_WRITE);
#endif
                return;
            }
        }
    }
}

void IrcSocket::readData()
{
    FXchar buffer[1024];
    int size = 0;

    FXString data = m_receiveRest;
#ifdef HAVE_OPENSSL
    if(m_useSsl)
    {
        size = SSL_read(m_ssl, buffer, 1023);
        if(size == -1)
        {
            size = SSL_get_error(m_ssl, size);
            switch (size)
            {
            case SSL_ERROR_NONE:
            case SSL_ERROR_WANT_READ:
            case SSL_ERROR_WANT_WRITE:
                break;
            case SSL_ERROR_ZERO_RETURN:
                sendEvent(IRC_ERROR, _("SSL_read() returns zero - closing socket"));
                closeConnection();
                break;
            default:
                sendEvent(IRC_ERROR, FXStringFormat(_("SSL read problem, errcode = %d"), size));
                closeConnection();
            }
        }
        else if(!size)
        {
            closeConnection();
        }
        else if(size > 0 && size <= 1023)
        {
            buffer[size] = '\0';
            if (utils::instance().isUtf8(buffer, size)) data.append(buffer);
            else data.append(utils::instance().localeToUtf8(buffer));
            while (data.contains('\n'))
            {
                parseLine(data.before('\n').before('\r'));
                data = data.after('\n');
            }
            m_receiveRest = data;
        }
        else
        {
            sendEvent(IRC_ERROR, _("Abnormal value from SSL read"));
        }
    }
    else
#endif
    {
#ifdef WIN32
        WSANETWORKEVENTS network_events;
        if(m_dccType == DCC_CHATOUT || m_dccType == DCC_OUT) WSAEnumNetworkEvents(m_clientid, m_event, &network_events);
        else WSAEnumNetworkEvents(m_serverid, m_event, &network_events);
        if (network_events.lNetworkEvents&FD_READ)
        {
            if(m_dccType == DCC_CHATOUT) size = recv(m_clientid, buffer, 1023, 0);
            else if(m_dccType == DCC_OUT || m_dccType == DCC_POUT)
            {
                FXulong pos;
                recv(m_dccType == DCC_OUT ? m_clientid : m_serverid, reinterpret_cast<char *>(&pos), 4, 0);
                pos = ntohl(pos);
                m_dccFile.finishedPosition = pos;
                if(m_dccFile.finishedPosition >= m_dccFile.size)
                {
                    m_application->removeTimeout(this, ID_PTIME);
                    sendEvent(IRC_DCCPOSITION, m_dccFile);
                    closeConnection(TRUE);
                }
                return;
            }
            else size = recv(m_serverid, buffer, 1023, 0);
            if (size > 0)
            {
                buffer[size] = '\0';
                if (utils::instance().isUtf8(buffer, size)) data.append(buffer);
                else data.append(utils::instance().localeToUtf8(buffer));
                while (data.contains('\n'))
                {
                    parseLine(data.before('\n').before('\r'));
                    data = data.after('\n');
                }
                m_receiveRest = data;
            }
            else if (size < 0)
            {
                sendEvent(IRC_ERROR, FXStringFormat(_("Error in reading data from %s"), m_serverName.text()));
                closeConnection();
            }
            else closeConnection();
        }
        //else if (network_events.lNetworkEvents&FD_CONNECT) ;
        else if (network_events.lNetworkEvents&FD_CLOSE) closeConnection();
#else
        if(m_dccType == DCC_CHATOUT) size = recv(m_clientid, buffer, 1023, 0);
        else if(m_dccType == DCC_OUT || m_dccType == DCC_POUT)
        {
            FXulong pos;
            recv(m_dccType == DCC_OUT ? m_clientid : m_serverid, reinterpret_cast<char *>(&pos), 4, 0);
            pos = ntohl(pos);
            m_dccFile.finishedPosition = pos;
            if(m_dccFile.finishedPosition >= m_dccFile.size)
            {
                m_application->removeTimeout(this, ID_PTIME);
                sendEvent(IRC_DCCPOSITION, m_dccFile);
                closeConnection(TRUE);
            }
            return;
        }
        else size = recv(m_serverid, buffer, 1023, 0);
        if (size > 0)
        {
            buffer[size] = '\0';
            if (utils::instance().isUtf8(buffer, size)) data.append(buffer);
            else data.append(utils::instance().localeToUtf8(buffer));
            while (data.contains('\n'))
            {
                parseLine(data.before('\n').before('\r'));
                data = data.after('\n');
            }
            m_receiveRest = data;
        }
        else if (size < 0)
        {
            sendEvent(IRC_ERROR, FXStringFormat(_("Error in reading data from %s"), m_serverName.text()));
            closeConnection();
        }
        else closeConnection();
#endif
    }
}

void IrcSocket::parseLine(const FXString &line)
{
    utils::instance().debugLine(FXStringFormat("<< %s",line.text()));
    FXString from, command, params;
    if (line[0] == ':')
    {
        from = utils::instance().getParam(line, 1, FALSE).after(':');
        command = utils::instance().getParam(line, 2, FALSE).upper();
        params = utils::instance().getParam(line, 3, TRUE);
    }
    else
    {
        from = "";
        command = utils::instance().getParam(line, 1, FALSE).upper();
        params = utils::instance().getParam(line, 2, TRUE);
    }

    if(m_dccType == DCC_CHATIN || m_dccType == DCC_CHATOUT)
    {
        dccMsg(line);
        return;
    }
    if(m_dccType == DCC_OUT)
    {

    }
    if(command.length() == 3)
    {
        m_realServerName = from;
        numeric(FXIntVal(command), params);
    }
    else if(command == "PRIVMSG") privmsg(from, params);
    else if(command == "JOIN") join(from, params);
    else if(command == "PART") part(from, params);
    else if(command == "QUIT") quitirc(from, params);
    else if(command == "PING") ping(params);
    else if(command == "PONG") pong(from, params);
    else if(command == "MODE") mode(from, params);
    else if(command == "TOPIC") topic(from, params);
    else if(command == "NOTICE") notice(from, params);
    else if(command == "KICK") kick(from, params);
    else if(command == "NICK") nick(from, params);
    else if(command == "INVITE") invite(from, params);
    else if(command == "ERROR") error(params);
    else unknown(command, params);
}

void IrcSocket::numeric(const FXint &command, const FXString &params)
{
    switch(command) {
        case 5: //RPL_ISUPPORT
        {
            sendEvent(IRC_SERVERREPLY, utils::instance().getParam(params, 2, TRUE));
            parseRplsupport(params);
        }break;
        case 219: //RPL_ENDOFSTATS
        {
            sendEvent(IRC_SERVERREPLY, _("End of STATS report"));
        }break;
        case 242: //RPL_STATSUPTIME
        {
            sendEvent(IRC_SERVERREPLY, FXStringFormat(_("Server Up %s days %s"), utils::instance().getParam(params, 4, FALSE).text(), utils::instance().getParam(params, 6, TRUE).text()));
        }break;
        case 301: //RPL_AWAY
        {
            sendEvent(IRC_301, utils::instance().getParam(params, 2, FALSE), utils::instance().getParam(params, 3, TRUE).after(':'));
        }break;
        case 302: //RPL_USERHOST
        {
            if(!m_ignoreUserHost)
                sendEvent(IRC_SERVERREPLY, utils::instance().getParam(params, 2, TRUE).after(':'));
            else
            {
                m_ignoreUserHost = FALSE;
                if(m_dccIP.empty())
                    m_dccIP = utils::instance().getParam(params, 2, TRUE).after('@');
            }

        }break;
        case 305: //RPL_UNAWAY
        {
            sendEvent(IRC_305, _("You are no longer marked as being away"));
            for(FXint i=0; i < m_nicks.no(); i++)
            {
                if(m_nicks[i].nick == m_nickName)
                {
                    m_nicks[i].away = FALSE;
                    break;
                }
            }
        }break;
        case 306: //RPL_NOWAWAY
        {
            sendEvent(IRC_306, _("You have been marked as being away"));
            for(FXint i=0; i < m_nicks.no(); i++)
            {
                if(m_nicks[i].nick == m_nickName)
                {
                    m_nicks[i].away = TRUE;
                    break;
                }
            }
        }break;
        case 311: //RPL_WHOISUSER
        {
            sendEvent(IRC_SERVERREPLY, _("Start of WHOIS"));
            sendEvent(IRC_SERVERREPLY, utils::instance().getParam(params, 2, FALSE)+" ["+utils::instance().getParam(params, 3, FALSE)+"@"+utils::instance().getParam(params, 4, FALSE)+"]");
            addNick(utils::instance().getParam(params, 2, FALSE), utils::instance().getParam(params, 3, FALSE), utils::instance().getParam(params, 6, TRUE).after(':'), utils::instance().getParam(params, 4, FALSE));
            sendEvent(IRC_SERVERREPLY, _("Realname: ")+utils::instance().getParam(params, 6, TRUE).after(':'));
        }break;
        case 312: //RPL_WHOISSERVER
        {
            sendEvent(IRC_SERVERREPLY, _("Server: ")+utils::instance().getParam(params, 3, FALSE)+" ["+utils::instance().getParam(params, 4, TRUE).after(':')+"]");
        }break;
        case 313: //RPL_WHOISOPERATOR
        {
            sendEvent(IRC_SERVERREPLY, FXStringFormat(_("%s is an IRC operator"), utils::instance().getParam(params, 2, FALSE).text()));
        }break;
        case 315: //RPL_ENDOFWHO
        {
            FXbool ignoreEvent = FALSE;
            for(FXint i=0; i < m_ignoreCommands.no(); i++)
            {
                if((utils::instance().getParam(m_ignoreCommands[i], 1, FALSE) == "who") && comparecase(utils::instance().getParam(m_ignoreCommands[i], 2, TRUE), utils::instance().getParam(params, 2, FALSE)) == 0)
                {
                    m_ignoreCommands.erase(i);
                    ignoreEvent = TRUE;
                    sendEvent(IRC_AWAY, utils::instance().getParam(params, 2, FALSE));
                    break;
                }
            }
            if(!ignoreEvent) sendEvent(IRC_SERVERREPLY, _("End of WHO list"));
        }break;
        case 317: //RPL_WHOISIDLE
        {
            FXlong idle = FXLongVal(utils::instance().getParam(params, 3, FALSE));
            sendEvent(IRC_SERVERREPLY, _("Idle: ")+FXStringVal(idle/3600)+":"+FXStringVal((idle/60)%60)+":"+FXStringVal(idle%60));
            FXString datestr = utils::instance().getParam(params, 4, FALSE);
            FXlong time = FXLongVal(datestr);
            datestr = FXSystem::time(_("%x %X"), time);
            sendEvent(IRC_SERVERREPLY, _("Signon: ")+datestr);
        }break;
        case 318: //RPL_ENDOFWHOIS
        {
            sendEvent(IRC_SERVERREPLY, _("End of WHOIS"));
        }break;
        case 319: //RPL_WHOISCHANNELS
        {
            sendEvent(IRC_SERVERREPLY, _("Channels: ")+utils::instance().getParam(params, 3, TRUE).after(':'));
        }break;
        case 320: //RPL_IDENTIFIED
        {
            if(params.contains("is identified to services")) sendEvent(IRC_SERVERREPLY, FXStringFormat(_("%s : is identified to services"), utils::instance().getParam(params, 2, FALSE).text()));
            else if(params.contains("is signed on as account")) sendEvent(IRC_SERVERREPLY, FXStringFormat(_("%s : is signed on as account %s"), utils::instance().getParam(params, 2, FALSE).text(), utils::instance().getParam(params, 8, FALSE).text()));
            else sendEvent(IRC_SERVERREPLY, utils::instance().getParam(params, 2, TRUE));
        }break;
        case 324: //RPL_CHANNELMODEIS
        {
            sendEvent(IRC_CHMODE, utils::instance().getParam(params, 2, FALSE), utils::instance().getParam(params, 3, FALSE));
        }break;
        case 329: //RPL of channel mode (creation time)
        {
            //now nothing action
        }break;
        case 331: //RPL_NOTOPIC
        {
            sendEvent(IRC_331, utils::instance().getParam(params, 2, FALSE), _("No topic is set"));
        }break;
        case 332: //RPL_TOPIC
        {
            sendEvent(IRC_332, utils::instance().getParam(params, 2, FALSE), FXStringFormat(_("Topic for %s : %s"), utils::instance().getParam(params, 2, FALSE).text(), utils::instance().getParam(params, 3, TRUE).after(':').text()));
        }break;
        case 333: //RPL_TOPICSETBY
        {
            FXString datestr = utils::instance().getParam(params, 4, FALSE);
            FXlong time = FXLongVal(datestr);
            datestr = FXSystem::time(_("%x %X"), time);
            sendEvent(IRC_333, utils::instance().getParam(params, 2, FALSE), FXStringFormat(_("Set %s %s"), utils::instance().getParam(params, 3, FALSE).before('!').text(), datestr.text()));
        }break;
        case 341: //RPL_INVITING
        {
            sendEvent(IRC_SERVERREPLY, FXStringFormat(_("You invite %s to %s"), utils::instance().getParam(params, 2, FALSE).text(), utils::instance().getParam(params, 3, FALSE).text()));
        }break;
        case 342: //RPL_SUMMONING
        {
            sendEvent(IRC_SERVERREPLY, FXStringFormat(_("%s :Summoning user to IRC"), utils::instance().getParam(params, 2, FALSE).text()));
        }break;
        case 352: //RPL_WHOREPLY
        {
            FXbool ignoreEvent = FALSE;
            for(FXint i=0; i < m_ignoreCommands.no(); i++)
            {
                if((utils::instance().getParam(m_ignoreCommands[i], 1, FALSE) == "who") && comparecase(utils::instance().getParam(m_ignoreCommands[i], 2, TRUE), utils::instance().getParam(params, 2, FALSE)) == 0)
                {
                    //param:xxx #test ~dvx localhost dvx.irc.dvx dvx H :0 dvx
                    addNick(utils::instance().getParam(params, 6, FALSE), utils::instance().getParam(params, 3, FALSE), utils::instance().getParam(params, 9, TRUE), utils::instance().getParam(params, 4, FALSE), utils::instance().getParam(params, 7, FALSE)[0] == 'H' ? FALSE : TRUE);
                    ignoreEvent = TRUE;
                    break;
                }
            }
            if(!ignoreEvent) sendEvent(IRC_SERVERREPLY, utils::instance().getParam(params, 2, TRUE));
        }break;
        case 353: //RPL_NAMREPLY
        {
            FXString nicks = utils::instance().getParam(params, 4, TRUE);
            //checking ':' is for servers, which don't use RFC right :)
            sendEvent(IRC_353, utils::instance().getParam(params, 3, FALSE), nicks[0]==':' ? nicks.after(':') : nicks);
        }break;
        case 366: //RPL_ENDOFNAMES
        {
            sendEvent(IRC_366, utils::instance().getParam(params, 2, FALSE));
        }break;
        case 367: //RPL_BANLIST
        {
            FXString datestr = utils::instance().getParam(params, 5, TRUE);
            if(datestr.empty()) sendEvent(IRC_SERVERREPLY, utils::instance().getParam(params, 2, FALSE)+" "+utils::instance().getParam(params, 3, FALSE));
            else
            {
                FXlong time = FXLongVal(datestr);
                datestr = FXSystem::time("%x %X", time);
                sendEvent(IRC_SERVERREPLY, utils::instance().getParam(params, 2, FALSE)+" "+utils::instance().getParam(params, 3, FALSE)+" "+utils::instance().getParam(params, 4, FALSE)+" "+datestr);
            }
        }break;
        case 369: //RPL_ENDOFWHOWAS
        {
            sendEvent(IRC_SERVERREPLY, _("End of WHOWAS"));
        }break;
        case 372: //RPL_MOTD
        {
            sendEvent(IRC_372, utils::instance().getParam(params, 2, TRUE).after(':'));
        }break;
        case 374: //RPL_ENDOFINFO
        {
            sendEvent(IRC_SERVERREPLY, _("End of INFO list"));
        }break;
        case 376: //RPL_ENDOFMOTD
        {
            sendEvent(IRC_SERVERREPLY, _("End of /MOTD command"));
            sendEvent(IRC_ENDMOTD);
            m_endmotd = TRUE;
            if(!m_startChannels.empty())
            {
                sendJoin(m_startChannels);
                m_startChannels.clear();
            }
            if(!m_startCommands.empty())
            {
                sendCommands();
            }
            sendLine("USERHOST "+m_nickName+" \r\n");
            m_ignoreUserHost = TRUE;
        }break;
        case 381: //RPL_YOUREOPER
        {
            sendEvent(IRC_SERVERREPLY, _("You are now an IRC operator"));
        }break;
        case 401: //ERR_NOSUCHNICK
        {
            sendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :No such nick/channel"), utils::instance().getParam(params, 2, FALSE).text()));
        }break;
        case 402: //ERR_NOSUCHSERVER
        {
            sendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :No such server"), utils::instance().getParam(params, 2, FALSE).text()));
        }break;
        case 403: //ERR_NOSUCHCHANNEL
        {
            sendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :No such channel"), utils::instance().getParam(params, 2, FALSE).text()));
        }break;
        case 404: //ERR_CANNOTSENDTOCHAN
        {
            sendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :Cannot send to channel"), utils::instance().getParam(params, 2, FALSE).text()));
        }break;
        case 405: //ERR_TOOMANYCHANNELS
        {
            sendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :You have joined too many channels"), utils::instance().getParam(params, 2, FALSE).text()));
        }break;
        case 406: //ERR_WASNOSUCHNICK
        {
            sendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :There was no such nickname"), utils::instance().getParam(params, 2, FALSE).text()));
        }break;
        case 407: //ERR_TOOMANYTARGETS
        {
            sendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :Too many targets"), utils::instance().getParam(params, 2, FALSE).text()));
        }break;
        case 408: //ERR_NOSUCHSERVICE
        {
            sendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :No such service"), utils::instance().getParam(params, 2, FALSE).text()));
        }break;
        case 409: //ERR_NOORIGIN
        {
            sendEvent(IRC_SERVERERROR, _("No origin specified"));
        }break;
        case 411: //ERR_NORECIPIENT
        {
            sendEvent(IRC_SERVERERROR, _("No recipient given"));
        }break;
        case 412: //ERR_NOTEXTTOSEND
        {
            sendEvent(IRC_SERVERERROR, _("No text to send"));
        }break;
        case 413: //ERR_NOTOPLEVEL
        {
            sendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :No toplevel domain specified"), utils::instance().getParam(params, 2, FALSE).text()));
        }break;
        case 414: //ERR_WILDTOPLEVEL
        {
            sendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :Wildcard in toplevel domain"), utils::instance().getParam(params, 2, FALSE).text()));
        }break;
        case 415: //ERR_BADMASK
        {
            sendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :Bad Server/host mask"), utils::instance().getParam(params, 2, FALSE).text()));
        }break;
        case 421: //ERR_UNKNOWNCOMMAND
        {
            sendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :Unknown command"), utils::instance().getParam(params, 2, FALSE).text()));
        }break;
        case 422: //ERR_NOMOTD
        {
            sendEvent(IRC_SERVERERROR, _("MOTD File is missing"));
            sendEvent(IRC_ENDMOTD);
            m_endmotd = TRUE;
            if(!m_startChannels.empty())
            {
                sendJoin(m_startChannels);
                m_startChannels.clear();
            }
            if(!m_startCommands.empty())
            {
                sendCommands();
            }
            sendLine("USERHOST "+m_nickName+" \r\n");
            m_ignoreUserHost = TRUE;
        }break;
        case 423: //ERR_NOADMININFO
        {
            sendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :No administrative info available"), utils::instance().getParam(params, 2, FALSE).text()));
        }break;
        case 424: //ERR_FILEERROR
        {
            sendEvent(IRC_SERVERERROR, _("File error doing"));
        }break;
        case 431: //ERR_NONICKNAMEGIVEN
        {
            sendEvent(IRC_SERVERERROR, _("No nickname given"));
        }break;
        case 432: //ERR_ERRONEUSNICKNAME
        {
            sendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :Erroneous nickname"), utils::instance().getParam(params, 2, FALSE).text()));
            if(m_endmotd) sendEvent(IRC_SERVERREPLY, FXStringFormat(_("You still have nick: %s"), m_nickName.text()));
            else
            {
                m_nickName = "_xxx_";
                sendNick(m_nickName);
            }
        }break;
        case 433: //ERR_NICKNAMEINUSE
        {
            sendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :Nickname is already in use"), utils::instance().getParam(params, 2, FALSE).text()));
            if(m_endmotd) sendEvent(IRC_SERVERREPLY, FXStringFormat(_("You still have nick: %s"), m_nickName.text()));
            else
            {
                m_nickName += "_";
                sendNick(m_nickName);
            }
        }break;
        case 436: //ERR_NICKCOLLISION
        {
            sendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :Nickname collision"), utils::instance().getParam(params, 2, FALSE).text()));
        }break;
        case 437: //ERR_UNAVAILRESOURCE
        {
            sendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :Nick/channel is temporarily unavailable"), utils::instance().getParam(params, 2, FALSE).text()));
        }break;
        case 441: //ERR_USERNOTINCHANNEL
        {
            sendEvent(IRC_SERVERERROR, FXStringFormat(_("%s %s :They aren't on that channel"), utils::instance().getParam(params, 2, FALSE).text(), utils::instance().getParam(params, 3, FALSE).text()));
        }break;
        case 442: //ERR_NOTONCHANNEL
        {
            sendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :You're not on that channel"), utils::instance().getParam(params, 2, FALSE).text()));
        }break;
        case 443: //ERR_USERONCHANNEL
        {
            sendEvent(IRC_SERVERERROR, FXStringFormat(_("%s %s :is already on channel"), utils::instance().getParam(params, 2, FALSE).text(), utils::instance().getParam(params, 3, FALSE).text()));
        }break;
        case 444: //ERR_NOLOGIN
        {
            sendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :User not logged in"), utils::instance().getParam(params, 2, FALSE).text()));
        }break;
        case 445: //ERR_SUMMONDISABLED
        {
            sendEvent(IRC_SERVERERROR, _("SUMMON has been disabled"));
        }break;
        case 446: //ERR_USERSDISABLED
        {
            sendEvent(IRC_SERVERERROR, _("USERS has been disabled"));
        }break;
        case 451: //ERR_NOTREGISTERED
        {
            sendEvent(IRC_SERVERERROR, _("You have not registered"));
        }break;
        case 461: //ERR_NEEDMOREPARAMS
        {
            sendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :Not enough parameters"), utils::instance().getParam(params, 2, FALSE).text()));
        }break;
        case 462: //ERR_ALREADYREGISTRED
        {
            sendEvent(IRC_SERVERERROR, _("Unauthorized command (already registered)"));
        }break;
        case 463: //ERR_NOPERMFORHOST
        {
            sendEvent(IRC_SERVERERROR, _("Your host isn't among the privileged"));
        }break;
        case 464: //ERR_PASSWDMISMATCH
        {
            sendEvent(IRC_SERVERERROR, _("Password incorrect"));
        }break;
        case 465: //ERR_YOUREBANNEDCREEP
        {
            sendEvent(IRC_SERVERERROR, _("You are banned from this server"));
        }break;
        case 466: //ERR_YOUWILLBEBANNED
        {
            sendEvent(IRC_SERVERERROR, _("You will be banned from this server"));
        }break;
        case 467: //ERR_KEYSET
        {
            sendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :Channel key already set"), utils::instance().getParam(params, 2, FALSE).text()));
        }break;
        case 471: //ERR_CHANNELISFULL
        {
            sendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :Cannot join channel (+l)"), utils::instance().getParam(params, 2, FALSE).text()));
        }break;
        case 472: //ERR_UNKNOWNMODE
        {
            sendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :is unknown mode char to me for channel"), utils::instance().getParam(params, 2, FALSE).text()));
        }break;
        case 473: //ERR_INVITEONLYCHAN
        {
            sendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :Cannot join channel (+i)"), utils::instance().getParam(params, 2, FALSE).text()));
        }break;
        case 474: //ERR_BANNEDFROMCHAN
        {
            sendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :Cannot join channel (+b)"), utils::instance().getParam(params, 2, FALSE).text()));
        }break;
        case 475: //ERR_BADCHANNELKEY
        {
            sendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :Cannot join channel (+k)"), utils::instance().getParam(params, 2, FALSE).text()));
        }break;
        case 476: //ERR_BADCHANMASK
        {
            if(utils::instance().getParam(params, 3, TRUE).contains("Bad Channel Mask")) sendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :Bad Channel Mask"), utils::instance().getParam(params, 2, FALSE).text()));
            else sendEvent(IRC_SERVERERROR, params);
        }break;
        case 477: //ERR_NOCHANMODES
        {
            if(utils::instance().getParam(params, 3, TRUE).contains("Channel doesn't support modes")) sendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :Channel doesn't support modes"), utils::instance().getParam(params, 2, FALSE).text()));
            else sendEvent(IRC_SERVERERROR, params);
        }break;
        case 478: //ERR_BANLISTFULL
        {
            if(utils::instance().getParam(params, 3, TRUE).contains("Channel list is full")) sendEvent(IRC_SERVERERROR, FXStringFormat(_("%s %s :Channel list is full"), utils::instance().getParam(params, 2, FALSE).text(), utils::instance().getParam(params, 3, FALSE).text()));
            else sendEvent(IRC_SERVERERROR, params);
        }break;
        case 481: //ERR_NOPRIVILEGES
        {
            sendEvent(IRC_SERVERERROR, _("Permission Denied- You're not an IRC operator"));
        }break;
        case 482: //ERR_CHANOPRIVSNEEDED
        {
            sendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :You're not channel operator"), utils::instance().getParam(params, 2, FALSE).text()));
        }break;
        case 483: //ERR_CANTKILLSERVER
        {
            sendEvent(IRC_SERVERERROR, _("You can't kill a server!"));
        }break;
        case 484: //ERR_RESTRICTED
        {
            sendEvent(IRC_SERVERERROR, _("Your connection is restricted!"));
        }break;
        case 485: //ERR_UNIQOPPRIVSNEEDED
        {
            sendEvent(IRC_SERVERERROR, _("You're not the original channel operator"));
        }break;
        case 491: //ERR_NOOPERHOST
        {
            sendEvent(IRC_SERVERERROR, _("No O-lines for your host"));
        }break;
        case 501: //ERR_UMODEUNKNOWNFLAG
        {
            sendEvent(IRC_SERVERERROR, _("Unknown MODE flag"));
        }break;
        case 502: //ERR_USERSDONTMATCH
        {
            sendEvent(IRC_SERVERERROR, _("Cannot change mode for other users"));
        }break;
        default:
        {
            if(utils::instance().getParam(params, 2, TRUE)[0] == ':') sendEvent(IRC_SERVERREPLY, utils::instance().getParam(params, 2, TRUE).after(':'));
            else sendEvent(IRC_SERVERREPLY, utils::instance().getParam(params, 2, TRUE));
        }
    }
}

/*
 * Parse 005 - RPL_ISUPPORT
 * http://www.irc.org/tech_docs/005.html
 */
void IrcSocket::parseRplsupport(FXString text)
{
    // nickLen, topicLen, kickLen, awayLen
    if(text.right(1) != " ") text.append(" ");
    for(FXint i=0; i<text.contains(' '); i++)
    {
        FXString parameter = text.section(' ', i).before('=');
        FXString value = text.section(' ', i).after('=');
        if(value.empty())
            continue;
        if(!comparecase(parameter, "PREFIX"))
        {
            FXString modes = value.after('(').before(')');
            FXString chars = value.after(')');
            if(modes.length() != chars.length())
                continue;
            for(FXint j=0; j<value.length(); j++)
            {
                switch(modes[j]) {
                    case 'a':
                        m_adminPrefix = chars[j];
                        break;
                    case 'q':
                        m_ownerPrefix = chars[j];
                        break;
                    case 'o':
                        m_opPrefix = chars[j];
                        break;
                    case 'v':
                        m_voicePrefix = chars[j];
                        break;
                    case 'h':
                        m_halfopPrefix = chars[j];
                        break;
                }
            }
            continue;
        }
        if(!comparecase(parameter, "CHANTYPES"))
        {
            m_chanTypes = value;
            continue;
        }
        if(!comparecase(parameter, "NICKLEN"))
        {
            m_nickLen = FXIntVal(value);
            continue;
        }
        if(!comparecase(parameter, "TOPICLEN"))
        {
            m_topicLen = FXIntVal(value);
            continue;
        }
        if(!comparecase(parameter, "KICKLEN"))
        {
            m_kickLen = FXIntVal(value);
            continue;
        }
        if(!comparecase(parameter, "AWAYLEN"))
        {
            m_awayLen = FXIntVal(value);
            continue;
        }
    }
    utils::instance().debugLine(FXStringFormat("adminPrefix=%c;ownerPrefix=%c;opPrefix=%c;voicePrefix=%c;halfopPrefix=%c", m_adminPrefix, m_ownerPrefix, m_opPrefix, m_voicePrefix, m_halfopPrefix));
    utils::instance().debugLine(FXStringFormat("chantypes=%s", m_chanTypes.text()));
    utils::instance().debugLine(FXStringFormat("nickLen=%d", m_nickLen));
    utils::instance().debugLine(FXStringFormat("topicLen=%d", m_topicLen));
    utils::instance().debugLine(FXStringFormat("kickLen=%d", m_kickLen));
    utils::instance().debugLine(FXStringFormat("awayLen=%d", m_awayLen));
}

void IrcSocket::privmsg(const FXString &from, const FXString &params)
{
    FXString nick = from.before('!');
    FXString to = utils::instance().getParam(params, 1, FALSE);
    FXString msg = utils::instance().getParam(params, 2, TRUE).after(':');
    if(msg[0] == '\001') ctcp(from, params);
    else
    {
        if(m_nickName == to && !isUserIgnored(nick, from.after('!').before('@'), from.after('@'), to)) sendEvent(IRC_QUERY, nick);
        if(!isUserIgnored(nick, from.after('!').before('@'), from.after('@'), to)) sendEvent(IRC_PRIVMSG, nick, to, msg);
    }
}

#ifdef WIN32
// inet_aton for windows
int inet_aton(const char *address, struct in_addr *sock)
{
    int s;
    s = inet_addr(address);
    if ( s == 1 && strcmp (address, "255.255.255.225") )
    return 0;
    sock->s_addr = s;
    return 1;
}
#endif

void IrcSocket::ctcp(const FXString &from, const FXString &params)
{
    FXString nick = from.before('!');
    FXString to = utils::instance().getParam(params, 1, FALSE);
    FXString msg = utils::instance().getParam(params, 2, TRUE).after(':').after('\001').before('\001');
    FXString ctcpCommand = msg.before(' ').upper();
    FXString ctcpRest = msg.after(' ');
    if(ctcpCommand == "VERSION")
    {
        if(isUserIgnored(nick, from.after('!').before('@'), from.after('@'), to))
            return;
        sendEvent(IRC_CTCPREQUEST, nick, ctcpCommand);
        sendVersion(nick);
    }
    else if(ctcpCommand == "ACTION")
    {
        if(m_nickName == to && !isUserIgnored(nick, from.after('!').before('@'), from.after('@'), to)) sendEvent(IRC_QUERY, nick);
        if(!isUserIgnored(nick, from.after('!').before('@'), from.after('@'), to)) sendEvent(IRC_ACTION, nick, to, ctcpRest);
    }
    else if(ctcpCommand == "PING")
    {
        if(isUserIgnored(nick, from.after('!').before('@'), from.after('@'), to))
            return;
        sendEvent(IRC_CTCPREQUEST, nick, ctcpCommand);
        sendCtcpNotice(nick, ctcpCommand+" "+ctcpRest);
    }
    else if(ctcpCommand == "DCC")
    {
        FXString dccCommand = utils::instance().getParam(ctcpRest, 1, FALSE).upper();
        if(dccCommand == "CHAT")
        {
            if(isUserIgnored(nick, from.after('!').before('@'), from.after('@'), to))
                return;
            sendEvent(IRC_DCCCHAT, nick, binaryIPToString(utils::instance().getParam(ctcpRest, 3, FALSE)), utils::instance().getParam(ctcpRest, 4, FALSE));
        }
        if(dccCommand == "SEND")
        {
            if(isUserIgnored(nick, from.after('!').before('@'), from.after('@'), to))
                return;
            if(utils::instance().getParam(ctcpRest, 5, FALSE) != utils::instance().getParam(ctcpRest, 6, FALSE)) //passive send
            {
                if(FXIntVal(utils::instance().getParam(ctcpRest, 4, FALSE)))
                    sendEvent(IRC_DCCMYTOKEN, binaryIPToString(utils::instance().getParam(ctcpRest, 3, FALSE)), utils::instance().getParam(ctcpRest, 4, FALSE), utils::instance().getParam(ctcpRest, 6, FALSE));
                else
                    sendEvent(IRC_DCCTOKEN, nick, utils::instance().getParam(ctcpRest, 2, FALSE), utils::instance().getParam(ctcpRest, 5, FALSE), utils::instance().getParam(ctcpRest, 6, FALSE));
            }
            else
                sendEvent(IRC_DCCIN, nick, binaryIPToString(utils::instance().getParam(ctcpRest, 3, FALSE))+"@"+utils::instance().getParam(ctcpRest, 4, FALSE), utils::instance().getParam(ctcpRest, 2, FALSE), utils::instance().getParam(ctcpRest, 5, FALSE));
        }
        if(dccCommand == "RESUME")
        {
            // 2-filename,3-port,4-position,5-token
            if(isUserIgnored(nick, from.after('!').before('@'), from.after('@'), to))
                return;
            if(utils::instance().getParam(ctcpRest, 4, FALSE) != utils::instance().getParam(ctcpRest, 5, FALSE)) //passive send
                sendEvent(IRC_DCCPRESUME, utils::instance().getParam(ctcpRest, 5, FALSE), utils::instance().getParam(ctcpRest, 4, FALSE));
            else
                sendEvent(IRC_DCCRESUME, nick, utils::instance().getParam(ctcpRest, 2, FALSE), utils::instance().getParam(ctcpRest, 3, FALSE), utils::instance().getParam(ctcpRest, 4, FALSE));
        }
        if(dccCommand == "ACCEPT")
        {
            // 2-filename,3-port,4-position,5-token
            if(isUserIgnored(nick, from.after('!').before('@'), from.after('@'), to))
                return;
            if(utils::instance().getParam(ctcpRest, 4, FALSE) != utils::instance().getParam(ctcpRest, 5, FALSE)) //passive send
                sendEvent(IRC_DCCPACCEPT, utils::instance().getParam(ctcpRest, 5, FALSE), utils::instance().getParam(ctcpRest, 4, FALSE));
            else
                sendEvent(IRC_DCCACCEPT, nick, utils::instance().getParam(ctcpRest, 2, FALSE), utils::instance().getParam(ctcpRest, 3, FALSE), utils::instance().getParam(ctcpRest, 4, FALSE));
        }
    }
    else if(ctcpCommand == "USERINFO")
    {
        if(isUserIgnored(nick, from.after('!').before('@'), from.after('@'), to))
            return;
        sendEvent(IRC_CTCPREQUEST, nick, ctcpCommand);
        sendCtcpNotice(nick, ctcpCommand+" "+m_realName);
    }
    else if(ctcpCommand == "CLIENTINFO")
    {
        if(isUserIgnored(nick, from.after('!').before('@'), from.after('@'), to))
            return;
        sendEvent(IRC_CTCPREQUEST, nick, ctcpCommand);
        sendCtcpNotice(nick, "CLIENTINFO ACTION CLIENTINFO PING TIME USERINFO VERSION");
    }
    else if(ctcpCommand == "TIME")
    {
        if(isUserIgnored(nick, from.after('!').before('@'), from.after('@'), to))
            return;
        sendEvent(IRC_CTCPREQUEST, nick, ctcpCommand);
        sendCtcpNotice(nick, ctcpCommand+" "+FXSystem::time("%x %X", FXSystem::now()));
    }
}

void IrcSocket::dccMsg(const FXString &line)
{
    if(line.contains('\001')) sendEvent(IRC_DCCACTION, line.after('\001').before('\001').after(' '));
    else sendEvent(IRC_DCCMSG, line);
}

void IrcSocket::join(const FXString &from, const FXString &params)
{
    FXString channel = utils::instance().getParam(params, 1, FALSE);
    if (channel[0] == ':') channel = channel.after(':');
    FXString nick = from.before('!');
    if(m_nickName == nick) sendEvent(IRC_NEWCHANNEL, (channel[0] == '&' ? "&"+channel : channel));
    else sendEvent(IRC_JOIN, nick, channel);
    addNick(nick, from.after('!').before('@'), "", from.after('@'), FALSE);
}

void IrcSocket::quitirc(const FXString &from, const FXString &params)
{
    FXString reason = utils::instance().getParam(params, 1, TRUE).after(':');
    FXString nick = from.before('!');
    if (reason.empty())
    {
        sendEvent(IRC_QUIT, nick);
    }
    else
    {
        sendEvent(IRC_QUIT, nick, reason);
    }
    removeNick(nick);
}

void IrcSocket::part(const FXString &from, const FXString &params)
{
    FXString channel = utils::instance().getParam(params, 1, FALSE);
    FXString reason = utils::instance().getParam(params, 1, TRUE).after(':');
    FXString nick = from.before('!');
    if (reason.empty())
    {
        sendEvent(IRC_PART, nick, channel);
    }
    else
    {
        sendEvent(IRC_PART, nick, channel, reason);
    }
}

void IrcSocket::ping(const FXString &params)
{
    sendLine("PONG "+params+"\r\n");
}

void IrcSocket::pong(const FXString &from, const FXString &params)
{
    sendEvent(IRC_SERVERREPLY, _("PONG from ")+from+" "+params.after(':'));
}

void IrcSocket::notice(const FXString &from, const FXString &params)
{
    FXString to = utils::instance().getParam(params, 1, FALSE);
    FXString nick = from.before('!');
    FXString msg = utils::instance().getParam(params, 2, TRUE).after(':');
    if(msg[0] == '\001')
    {
        if(isUserIgnored(nick, from.after('!').before('@'), from.after('@'), to))
            return;
        sendEvent(IRC_CTCPREPLY, nick, msg.after('\001').before('\001'));
    }
    else
    {
        if(from.empty()) //public notice
        {
            sendEvent(IRC_NOTICE, to, msg);
        }
        else
        {
            if(!isUserIgnored(nick, from.after('!').before('@'), from.after('@'), to))
                sendEvent(IRC_CHNOTICE, nick, to, msg);
        }
    }
}

void IrcSocket::nick(const FXString &from, const FXString &params)
{
    FXString nick = from.before('!');
    FXString newnick = utils::instance().getParam(params, 1, FALSE);
    if(newnick[0] == ':') newnick = newnick.after(':');
    if(m_nickName == nick) m_nickName = newnick;
    sendEvent(IRC_NICK, nick, newnick);
    for(FXint i = 0; i < m_nicks.no(); i++)
    {
        if(nick == m_nicks[i].nick)
        {
            m_nicks[i].nick = newnick;
            break;
        }
    }
}

void IrcSocket::topic(const FXString &from, const FXString &params)
{
    FXString nick = from.before('!');
    FXString channel = utils::instance().getParam(params, 1, FALSE);
    FXString topic = utils::instance().getParam(params, 2, TRUE).after(':');
    sendEvent(IRC_TOPIC, nick, channel, topic);
}

void IrcSocket::invite(const FXString &from, const FXString &params)
{
    FXString nick = from.before('!');
    FXString to = utils::instance().getParam(params, 1, FALSE);
    FXString channel = utils::instance().getParam(params, 2, FALSE);
    if (channel[0] == ':') channel = channel.after(':');
    if(!isUserIgnored(nick, from.after('!').before('@'), from.after('@'), channel)) sendEvent(IRC_INVITE, nick, to, channel);
}

void IrcSocket::kick(const FXString &from, const FXString &params)
{
    FXString nick = from.before('!');
    FXString to = utils::instance().getParam(params, 2, FALSE);
    FXString channel = utils::instance().getParam(params, 1, FALSE);
    FXString reason = utils::instance().getParam(params, 3, TRUE).after(':');
    sendEvent(IRC_KICK, nick, to, channel, reason);
}

void IrcSocket::mode(const FXString &from, const FXString &params)
{
    if(params.contains(':')) sendEvent(IRC_MODE, utils::instance().getParam(params, 2, TRUE).after(':'), utils::instance().getParam(params, 1, FALSE));
    else //channel mode
    {
        FXString moderator = from.before('!');
        FXString channel = utils::instance().getParam(params, 1, FALSE);
        FXString modes = utils::instance().getParam(params, 2, FALSE);
        FXString args = utils::instance().getParam(params, 3, TRUE);
        sendEvent(IRC_UMODE, moderator, channel, modes, args);
    }
}

void IrcSocket::error(const FXString &params)
{
    sendEvent(IRC_SERVERERROR, params[0] == ':' ? params.after(':') : params);
}

void IrcSocket::unknown(const FXString &command, const FXString &params)
{
    sendEvent(IRC_UNKNOWN, command, params);
}

void IrcSocket::addIgnoreCommands(const FXString& command)
{
    m_ignoreCommands.append(command);
}

void IrcSocket::removeIgnoreCommands(const FXString& command)
{
    for(FXint i=0; i < m_ignoreCommands.no(); i++)
    {
        if(m_ignoreCommands[i] == command)
        {
            m_ignoreCommands.erase(i);
            break;
        }
    }
}

FXbool IrcSocket::sendAdmin(const FXString& params)
{
    return sendLine("ADMIN "+params+"\r\n");
}

FXbool IrcSocket::sendAway(const FXString& params)
{
    return sendLine("AWAY :"+params+"\r\n");
}

FXbool IrcSocket::sendBanlist(const FXString& channel)
{
    return sendLine("MODE "+channel+" +b\r\n");
}

FXbool IrcSocket::sendCtcp(const FXString& to, const FXString& params)
{
    return sendLine("PRIVMSG "+to+" :\001"+params+"\001\r\n");
}

FXbool IrcSocket::sendCtcpNotice(const FXString& to, const FXString& params)
{
    return sendLine("NOTICE "+to+" :\001"+params+"\001\r\n");
}

FXbool IrcSocket::sendDccChatText(const FXString& message)
{
    return sendLine(message+"\n");
}

FXbool IrcSocket::sendMode(const FXString& params)
{
    return sendLine("MODE "+params+"\r\n");
}

FXbool IrcSocket::sendInvite(const FXString& to, const FXString& params)
{
    return sendLine("INVITE "+to+" "+params+"\r\n");
}

FXbool IrcSocket::sendJoin(const FXString& chan)
{
    return sendLine("JOIN "+chan+"\r\n");
}

FXbool IrcSocket::sendKick(const FXString& chan, const FXString& nick, const FXString& reason)
{
    return sendLine("KICK "+chan+" "+nick+" :"+reason+"\r\n");
}

FXbool IrcSocket::sendKill(const FXString& nick, const FXString& reason)
{
    return sendLine("KILL "+nick+" :"+reason+"\r\n");
}

FXbool IrcSocket::sendList(const FXString& params)
{
    return sendLine("LIST "+params+"\r\n");
}

FXbool IrcSocket::sendMe(const FXString& to, const FXString& message)
{
    return sendLine("PRIVMSG "+to+" :\001ACTION "+message+"\001\r\n");
}

FXbool IrcSocket::sendMsg(const FXString& to, const FXString& message)
{
    return sendLine("PRIVMSG "+to+" :"+message+"\r\n");
}

FXbool IrcSocket::sendNames(const FXString& channel)
{
    return sendLine("NAMES "+channel+"\r\n");
}

FXbool IrcSocket::sendNick(const FXString& nick)
{
    return sendLine("NICK "+nick+"\r\n");
}

FXbool IrcSocket::sendNotice(const FXString& to, const FXString& message)
{
    return sendLine("NOTICE "+to+" :"+message+"\r\n");
}

FXbool IrcSocket::sendOper(const FXString& login, const FXString& password)
{
    return sendLine("OPER "+login+" "+password+"\r\n");
}

FXbool IrcSocket::sendPart(const FXString& chan)
{
    return sendLine("PART "+chan+"\r\n");
}

FXbool IrcSocket::sendPart(const FXString& chan, const FXString& reason)
{
    return sendLine("PART "+chan+" :"+reason+"\r\n");
}

FXbool IrcSocket::sendQuote(const FXString& text)
{
    return sendLine(text+"\r\n");
}

FXbool IrcSocket::sendStats(const FXString& type)
{
    if(type.empty())
        return sendLine("STATS\r\n");
    else
        return sendLine("STATS "+type+"\r\n");
}

FXbool IrcSocket::sendTopic(const FXString& chan, const FXString& topic)
{
    if (topic.empty())
        return sendLine("TOPIC "+chan+"\r\n");
    else
        return sendLine("TOPIC "+chan+" :"+topic+"\r\n");
}

FXbool IrcSocket::sendTopic(const FXString& chan)
{
    return sendLine("TOPIC "+chan+"\r\n");
}

FXbool IrcSocket::sendWallops(const FXString& msg)
{
    return sendLine("WALLOPS :"+msg+"\r\n");
}

FXbool IrcSocket::sendVersion(const FXString& to)
{
    return sendLine("NOTICE "+to+FXStringFormat(" :\001VERSION dxirc %s (C) 2008~ by David Vachulka\001\r\n", VERSION));
}

FXbool IrcSocket::sendWho(const FXString& mask)
{
    return sendLine("WHO "+mask+"\r\n");
}

FXbool IrcSocket::sendWhoami()
{
    return sendLine("WHOIS "+m_nickName+"\r\n");
}

FXbool IrcSocket::sendWhois(const FXString& params)
{
    return sendLine("WHOIS "+params+"\r\n");
}

FXbool IrcSocket::sendWhowas(const FXString& params)
{
    return sendLine("WHOWAS "+params+"\r\n");
}

FXbool IrcSocket::sendLine(const FXString& line)
{
    utils::instance().debugLine(FXStringFormat(">> %s", line.before('\n').text()));
    int size;
    if (m_connected)
    {
        if(m_useSsl)
        {
#ifdef HAVE_OPENSSL
            size = SSL_write(m_ssl, line.text(), line.length());
#endif
        }
        else
        {
            if ((size = send(m_dccType == DCC_CHATOUT ? m_clientid : m_serverid, line.text(), line.length(), 0)) == -1)
            {
                sendEvent(IRC_ERROR, _("Unable to send data"));
#ifdef WIN32
                WSACleanup();
#endif
                closeConnection();
                return FALSE;
            }
        }
        return TRUE;
    }
    else return FALSE;
}

void IrcSocket::sendCommands()
{
    if(!m_startCommands.contains('\n')) m_startCommands.append('\n');
    if(m_startCommands.right(1) != "\n") m_startCommands.append('\n');
    for(FXint i=0; i < m_startCommands.contains('\n'); i++)
    {
        sendCommand(m_startCommands.section('\n', i));
    }
}

FXbool IrcSocket::sendCommand(const FXString& commandtext)
{
    if(m_connected)
    {
        FXString command = commandtext.after('/').before(' ').lower();
        if(command == "admin")
        {
            return sendAdmin(commandtext.after(' '));
        }
        if(command == "away")
        {
            return sendAway(commandtext.after(' '));
        }
        if(command == "banlist")
        {
            FXString channel = commandtext.after(' ');
            return sendBanlist(channel);
        }
        if(command == "ctcp")
        {
            FXString to = commandtext.after(' ').before(' ');
            FXString msg = commandtext.after(' ', 2);
            return sendCtcp(to, msg);
        }
        if(command == "deop")
        {
            FXString params = commandtext.after(' ');
            FXString channel = params.before(' ');
            FXString nicks = params.after(' ');
            FXString modeparams = utils::instance().createModes('-', 'o', nicks);
            return sendMode(channel+" "+modeparams);
        }
        if(command == "devoice")
        {
            FXString params = commandtext.after(' ');
            FXString channel = params.before(' ');
            FXString nicks = params.after(' ');
            FXString modeparams = utils::instance().createModes('-', 'v', nicks);
            return sendMode(channel+" "+modeparams);
        }
        if(command == "invite")
        {
            FXString params = commandtext.after(' ');
            FXString nick = params.before(' ');
            FXString channel = params.after(' ');
            return sendInvite(nick, channel);
        }
        if(command == "join")
        {
            FXString channel = commandtext.after(' ');
            return sendJoin(channel);
        }
        if(command == "kick")
        {
            FXString params = commandtext.after(' ');
            FXString channel = params.before(' ');
            FXString nick = params.after(' ');
            FXString reason = params.after(' ', 2);
            return sendKick(channel, nick, reason);
        }
        if(command == "kill")
        {
            FXString params = commandtext.after(' ');
            FXString nick = params.before(' ');
            FXString reason = params.after(' ');
            return sendKill(nick, reason);
        }
        if(command == "list")
        {
            return sendList(commandtext.after(' '));
        }
        if(command == "me")
        {
            FXString params = commandtext.after(' ');
            FXString to = params.before(' ');
            FXString message = params.after(' ');
            return sendMe(to, message);
        }
        if(command == "msg")
        {
            FXString params = commandtext.after(' ');
            FXString to = params.before(' ');
            FXString message = params.after(' ');
            return sendMsg(to, message);
        }
        if(command == "names")
        {
            FXString params = commandtext.after(' ');
            return sendNames(params);
        }
        if(command == "nick")
        {
            FXString nick = commandtext.after(' ');
            return sendNick(nick);
        }
        if(command == "notice")
        {
            FXString params = commandtext.after(' ');
            FXString to = params.before(' ');
            FXString message = params.after(' ');
            return sendNotice(to, message);
        }
        if(command == "op")
        {
            FXString params = commandtext.after(' ');
            FXString channel = params.before(' ');
            FXString nicks = params.after(' ');
            FXString modeparams = utils::instance().createModes('+', 'o', nicks);
            return sendMode(channel+" "+modeparams);
        }
        if(command == "oper")
        {
            FXString params = commandtext.after(' ');
            FXString login = params.before(' ');
            FXString password = params.after(' ');
            return sendOper(login, password);
        }
        if(command == "quote")
        {
            return sendQuote(commandtext.after(' '));
        }
        if(command == "topic")
        {
            FXString params = commandtext.after(' ');
            FXString channel = params.before(' ');
            FXString topic = params.after(' ');
            return sendTopic(channel, topic);
        }
        if(command == "voice")
        {
            FXString params = commandtext.after(' ');
            FXString channel = params.before(' ');
            FXString nicks = params.after(' ');
            FXString modeparams = utils::instance().createModes('+', 'v', nicks);
            return sendMode(channel+" "+modeparams);
        }
        if(command == "wallops")
        {
            FXString params = commandtext.after(' ');
            return sendWallops(params);
        }
        if(command == "who")
        {
            FXString params = commandtext.after(' ');
            return sendWho(params);
        }
        if(command == "whois")
        {
            FXString params = commandtext.after(' ');
            return sendWhois(params);
        }
        if(command == "whowas")
        {
            FXString params = commandtext.after(' ');
            return sendWhowas(params);
        }
        sendEvent(IRC_ERROR, FXStringFormat(_("Bad command on connection:%s"), commandtext.text()));
        return FALSE;
    }
    else
        return FALSE;
}

void IrcSocket::sendEvents()
{
    if(m_thread->running())
        return;
    while(m_events.no())
    {
        for(FXint i=0; i < m_targets.no(); i++)
        {
            m_targets.at(i)->handle(this, FXSEL(SEL_COMMAND, ID_SERVER), &m_events[0]);
        }
        m_events.erase(0);
    }
}

void IrcSocket::sendEvent(IrcEventType eventType)
{
    IrcEvent ev;
    ev.eventType = eventType;
    ev.param1 = "";
    ev.param2 = "";
    ev.param3 = "";
    ev.param4 = "";
    ev.dccFile = DccFile();
    ev.time = FXSystem::now();
    if(m_thread->running())
        m_events.append(ev);
    else
    {
        sendEvents();
        for(FXint i=0; i < m_targets.no(); i++)
        {
            m_targets.at(i)->handle(this, FXSEL(SEL_COMMAND, ID_SERVER), &ev);
        }
    }
}

void IrcSocket::sendEvent(IrcEventType eventType, const FXString &param1)
{
    IrcEvent ev;
    ev.eventType = eventType;
    ev.param1 = param1;
    ev.param2 = "";
    ev.param3 = "";
    ev.param4 = "";
    ev.dccFile = DccFile();
    ev.time = FXSystem::now();
    if(m_thread->running())
        m_events.append(ev);
    else
    {
        sendEvents();
        for(FXint i=0; i < m_targets.no(); i++)
        {
            m_targets.at(i)->handle(this, FXSEL(SEL_COMMAND, ID_SERVER), &ev);
        }
    }
}

void IrcSocket::sendEvent(IrcEventType eventType, const FXString &param1, const FXString &param2)
{
    IrcEvent ev;
    ev.eventType = eventType;
    ev.param1 = param1;
    ev.param2 = param2;
    ev.param3 = "";
    ev.param4 = "";
    ev.dccFile = DccFile();
    ev.time = FXSystem::now();
    if(m_thread->running())
        m_events.append(ev);
    else
    {
        sendEvents();
        for(FXint i=0; i < m_targets.no(); i++)
        {
            m_targets.at(i)->handle(this, FXSEL(SEL_COMMAND, ID_SERVER), &ev);
        }
    }
}

void IrcSocket::sendEvent(IrcEventType eventType, const FXString &param1, const FXString &param2, const FXString &param3)
{
    IrcEvent ev;
    ev.eventType = eventType;
    ev.param1 = param1;
    ev.param2 = param2;
    ev.param3 = param3;
    ev.param4 = "";
    ev.dccFile = DccFile();
    ev.time = FXSystem::now();
    if(m_thread->running())
        m_events.append(ev);
    else
    {
        sendEvents();
        for(FXint i=0; i < m_targets.no(); i++)
        {
            m_targets.at(i)->handle(this, FXSEL(SEL_COMMAND, ID_SERVER), &ev);
        }
    }
}

void IrcSocket::sendEvent(IrcEventType eventType, const FXString &param1, const FXString &param2, const FXString &param3, const FXString &param4)
{
    IrcEvent ev;
    ev.eventType = eventType;
    ev.param1 = param1;
    ev.param2 = param2;
    ev.param3 = param3;
    ev.param4 = param4;
    ev.dccFile = DccFile();
    ev.time = FXSystem::now();
    if(m_thread->running())
        m_events.append(ev);
    else
    {
        sendEvents();
        for(FXint i=0; i < m_targets.no(); i++)
        {
            m_targets.at(i)->handle(this, FXSEL(SEL_COMMAND, ID_SERVER), &ev);
        }
    }
}

//usefull for dcc receiving/sending file
void IrcSocket::sendEvent(IrcEventType eventType, DccFile file)
{
    IrcEvent ev;
    ev.eventType = eventType;
    ev.param1 = "";
    ev.param2 = "";
    ev.param3 = "";
    ev.param4 = "";
    ev.dccFile = file;
    ev.time = FXSystem::now();
    if(m_thread->running())
        m_events.append(ev);
    else
    {
        sendEvents();
        for(FXint i=0; i < m_targets.no(); i++)
        {
            m_targets.at(i)->handle(this, FXSEL(SEL_COMMAND, ID_SERVER), &ev);
        }
    }
}

void IrcSocket::appendTarget(FXObject *tgt)
{
     m_targets.append(tgt);
}

void IrcSocket::removeTarget(FXObject *tgt)
{
    for(FXint i=0; i < m_targets.no(); i++)
    {
        if(m_targets[i] == tgt)
        {
            m_targets.erase(i);
            break;
        }
    }
}

FXbool IrcSocket::findTarget(FXObject *tgt)
{
    for(FXint i=0; i < m_targets.no(); i++)
    {
        if(m_targets[i] == tgt)
        {
            return TRUE;
        }
    }
    return FALSE;
}

void IrcSocket::clearChannelsCommands(FXbool disableReconnect)
{
    if(m_reconnect && m_attempts < m_numberAttempt && !disableReconnect)
    {
        makeStartChannels();
    }
    else
    {
        m_startChannels.clear();
        m_startCommands.clear();
    }
}

void IrcSocket::makeStartChannels()
{
    m_startChannels.clear();
    IrcTabItem *tab;
    for(FXint i=0; i < m_targets.no(); i++)
    {
        if((tab = dynamic_cast<IrcTabItem*>(m_targets[i])))
        {
            if(tab->getType() == CHANNEL)
                m_startChannels.append(tab->getText()+",");
        }
    }
}

FXbool IrcSocket::clearTarget()
{
    m_targets.clear();
    if(m_targets.no()) return FALSE;
    else return TRUE;
}

void IrcSocket::addNick(const FXString &nick, const FXString &user, const FXString &real, const FXString &host, const FXbool &away)
{
    FXbool nickExist = FALSE;
    for(FXint i=0; i < m_nicks.no(); i++)
    {
        if(m_nicks[i].nick == nick)
        {
            m_nicks[i].user = user;
            m_nicks[i].real = real;
            m_nicks[i].host = host;
            m_nicks[i].away = away;
            nickExist = TRUE;
            break;
        }
    }
    if(!nickExist)
    {
        NickInfo nickStruct;
        nickStruct.nick = nick;
        nickStruct.user = user;
        nickStruct.real = real;
        nickStruct.host = host;
        nickStruct.away = away;
        m_nicks.append(nickStruct);
    }
}

void IrcSocket::addNick(const FXString &nick, const FXString &user, const FXString &real, const FXString &host)
{
    FXbool nickExist = FALSE;
    for(FXint i=0; i < m_nicks.no(); i++)
    {
        if(m_nicks[i].nick == nick)
        {
            m_nicks[i].user = user;
            m_nicks[i].real = real;
            m_nicks[i].host = host;
            nickExist = TRUE;
            break;
        }
    }
    if(!nickExist)
    {
        NickInfo nickStruct;
        nickStruct.nick = nick;
        nickStruct.user = user;
        nickStruct.real = real;
        nickStruct.host = host;
        nickStruct.away = FALSE;
        m_nicks.append(nickStruct);
    }
}

void IrcSocket::removeNick(const FXString &nick)
{
    for(FXint i=0; i < m_nicks.no(); i++)
    {
        if(m_nicks[i].nick == nick)
        {
            m_nicks.erase(i);
            break;
        }
    }
}

NickInfo IrcSocket::getNickInfo(const FXString &nick) const
{
    NickInfo nickInfo;
    for(FXint i=0; i < m_nicks.no(); i++)
    {
        if(m_nicks[i].nick == nick)
        {
            nickInfo = m_nicks[i];
            break;
        }
    }
    return nickInfo;
}

FXString IrcSocket::getBannedNick(const FXString &banmask) const
{
    FXString nick = "";
    FXString banNick = banmask.before('!');
    FXString banUser = banmask.after('!').before('@');
    FXString banHost = banmask.after('@');
    for(FXint i=0; i<m_nicks.no(); i++)
    {
        if(FXRex(FXString("\\<"+banNick+"\\>").substitute("*","\\w*")).match(m_nicks[i].nick) && FXRex(FXString(banUser+"\\>").substitute("*","\\w*")).match(m_nicks[i].user) && FXRex(FXString("\\<"+banHost+"\\>").substitute("*","\\w*")).match(m_nicks[i].host)) nick += m_nicks[i].nick+";";
    }
    return nick;
}

FXbool IrcSocket::isAway(const FXString& nick)
{
    for(FXint i=0; i < m_nicks.no(); i++)
    {
        if(m_nicks[i].nick == nick)
        {
            return m_nicks[i].away;
        }
    }
    return FALSE;
}

//ipaddr in byte order
FXbool IrcSocket::isRoutableIP(FXuint ipaddr)
{
    if(!ipaddr) return FALSE;
    unsigned char * ip = (unsigned char *)&ipaddr;
    int a, b, c, d;
    d = (int)*ip++;
    c = (int)*ip++;
    b = (int)*ip++;
    a = (int)*ip;
    if(a == 0) return FALSE;    // old-style broadcast
    if(a == 10) return FALSE;   // Class A VPN
    if(a == 127) return FALSE;   // loopback
    if((a == 172) && (b >= 16) && (b <= 31)) return FALSE; // Class B VPN
    if((a == 192) && (b == 168)) return FALSE; // Class C VPN
    if((a == 169) && (b == 254)) return FALSE; // APIPA
    if((a == 192) && (b == 0) && (c == 2)) return FALSE; // Class B VPN
    if(a >= 224) return FALSE; // class D multicast and class E reserved
    return TRUE;
}

//address as 2130706433
FXString IrcSocket::binaryIPToString(const FXString &address)
{
    struct in_addr addr;
    inet_aton(address.text(), &addr);
    return inet_ntoa(addr);
}

//Return IP usefull for DCC (byte order)
//address as "127.0.0.1" or "locahost"
FXuint IrcSocket::stringIPToBinary(const FXString &address)
{
    if(address.contains('.') == 3)
    {
        FXRex rex("\\l");
        if(!rex.match(address))
            return ntohl(inet_addr(address.text()));
    }
    struct hostent *dns_query;
    FXuint addr = 0;
    dns_query = gethostbyname(address.text());
    if (dns_query != NULL &&
           dns_query->h_length == 4 &&
           dns_query->h_addr_list[0] != NULL)
    {
            addr = *((FXuint*) dns_query->h_addr_list[0]);
    }
    return ntohl(addr);
}

//return string in dots-and-numbers format
const char* IrcSocket::getLocalIP()
{
    struct sockaddr_in local;
    socklen_t len = sizeof(struct sockaddr_in);
    getsockname(m_serverid, reinterpret_cast<struct sockaddr *>(&local), &len);
    utils::instance().debugLine(FXStringFormat("LocalIP: %s", inet_ntoa(local.sin_addr)));
    return inet_ntoa(local.sin_addr);
}

//return string in dots-and-numbers format
const char* IrcSocket::getRemoteIP()
{
    utils::instance().debugLine(FXStringFormat("RemoteIP: %s", inet_ntoa(m_serverSock.sin_addr)));
    return inet_ntoa(m_serverSock.sin_addr);
}

//Return IP usefull for DCC (byte order)
FXuint IrcSocket::getLocalIPBinary()
{
    return stringIPToBinary(getLocalIP());
}

FXbool IrcSocket::isUserIgnored(const FXString &nick, const FXString &user, const FXString &host, const FXString &on)
{
    FXbool bnick = FALSE;
    FXbool buser = FALSE;
    FXbool bhost = FALSE;
    FXbool bchannel = FALSE;
    FXbool bserver = FALSE;
    for(FXint i=0; i<m_usersList.no(); i++)
    {
        FXString inick, iuser, ihost;
        inick = m_usersList[i].nick.before('!');
        iuser = m_usersList[i].nick.after('!').before('@');
        ihost = m_usersList[i].nick.after('@');
        if(FXRex(FXString("\\<"+inick+"\\>").substitute("*","\\w*")).match(nick)) bnick = TRUE;
        if(FXRex(FXString("\\<"+iuser+"\\>").substitute("*","\\w*")).match(user) || iuser.empty()) buser = TRUE;
        if(FXRex(FXString("\\<"+ihost+"\\>").substitute("*","\\w*")).match(host) || ihost.empty()) bhost = TRUE;
        if(m_usersList[i].channel == "all") bchannel = TRUE;
        if(m_usersList[i].channel.contains(','))
        {
            for(FXint j=1; j<m_usersList[i].channel.contains(',')+2; j++)
            {
                if(FXRex(FXString(utils::instance().getParam(m_usersList[i].channel, j, FALSE, ',')+"\\>").substitute("*","\\w*")).match(on))
                {
                    bchannel = TRUE;
                    break;
                }
            }
        }
        else
        {
            if(FXRex(FXString(m_usersList[i].channel+"\\>").substitute("*","\\w*")).match(on)) bchannel = TRUE;
        }
        if(m_usersList[i].server == "all") bserver = TRUE;
        if(FXRex(FXString("\\<"+m_usersList[i].server+"\\>").substitute("*","\\w*")).match(m_serverName)) bserver = TRUE;
    }
    return bnick && buser && bhost && bchannel && bserver;
}

FXbool IrcSocket::isUserIgnored(const FXString &nick, const FXString &on)
{
    FXbool bnick = FALSE;
    FXbool buser = FALSE;
    FXbool bhost = FALSE;
    FXbool bchannel = FALSE;
    FXbool bserver = FALSE;
    FXString user, host;
    for(FXint i = 0; i < m_nicks.no(); i++)
    {
        if(nick == m_nicks[i].nick)
        {
            user = m_nicks[i].user;
            host = m_nicks[i].host;
            break;
        }
    }
    for(FXint i=0; i<m_usersList.no(); i++)
    {
        FXString inick, iuser, ihost;
        inick = m_usersList[i].nick.before('!');
        iuser = m_usersList[i].nick.after('!').before('@');
        ihost = m_usersList[i].nick.after('@');
        if(FXRex(FXString("\\<"+inick+"\\>").substitute("*","\\w*")).match(nick)) bnick = TRUE;
        if(FXRex(FXString("\\<"+iuser+"\\>").substitute("*","\\w*")).match(user) || iuser.empty()) buser = TRUE;
        if(FXRex(FXString("\\<"+ihost+"\\>").substitute("*","\\w*")).match(host) || ihost.empty()) bhost = TRUE;
        if(m_usersList[i].channel == "all") bchannel = TRUE;
        if(m_usersList[i].channel.contains(','))
        {
            for(FXint j=1; j<m_usersList[i].channel.contains(',')+2; j++)
            {
                if(FXRex(FXString(utils::instance().getParam(m_usersList[i].channel, j, FALSE, ',')+"\\>").substitute("*","\\w*")).match(on))
                {
                    bchannel = TRUE;
                    break;
                }
            }
        }
        else
        {
            if(FXRex(FXString(m_usersList[i].channel+"\\>").substitute("*","\\w*")).match(on)) bchannel = TRUE;
        }
        if(m_usersList[i].server == "all") bserver = TRUE;
        if(FXRex(FXString("\\<"+m_usersList[i].server+"\\>").substitute("*","\\w*")).match(m_serverName)) bserver = TRUE;
    }
    return bnick && buser && bhost && bchannel && bserver;
}

// reset running reconnect timeout
void IrcSocket::resetReconnect()
{
    if(m_application->hasTimeout(this, ID_RTIME))
    {
        m_application->removeTimeout(this, ID_RTIME);
        m_application->addTimeout(this, ID_RTIME, m_delayAttempt*1000);
        return;
    }
    if(m_reconnect && m_attempts < m_numberAttempt)
    {
        m_application->addTimeout(this, ID_RTIME, m_delayAttempt*1000);
    }
}

ConnectThread::ConnectThread(IrcSocket *socket) : m_socket(socket)
{
}

ConnectThread::~ConnectThread()
{
}

FXint ConnectThread::run()
{
    if(m_socket->getDccType() == DCC_CHATOUT
            || m_socket->getDccType() == DCC_OUT
            || m_socket->getDccType() == DCC_PIN)
        m_socket->listenIRC();
    else
    {
        if(m_socket->getUseSsl()) m_socket->connectSSL();
        else m_socket->connectIRC();
    }
    return 1;
}
