/*
 *      dccengine.cpp
 *
 *      Copyright 2010 David Vachulka <david@konstrukce-cad.com>
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

#include "dccengine.h"
#include "i18n.h"

FXDEFMAP(DccEngine) DccEngineMap[] = {
    FXMAPFUNC(SEL_TIMEOUT,          DccEngine_CTIME,     DccEngine::onCloseTimeout),
    FXMAPFUNC(SEL_TIMEOUT,          DccEngine_PTIME,     DccEngine::onPositionTimeout),
    FXMAPFUNC(SOCKET_CANREAD,       DccEngine_SOCKET,    DccEngine::onSocketCanRead),
    FXMAPFUNC(SOCKET_CONNECTED,     DccEngine_SOCKET,    DccEngine::onSocketConnected),
    FXMAPFUNC(SOCKET_DISCONNECTED,  DccEngine_SOCKET,    DccEngine::onSocketDisconnected),
    FXMAPFUNC(SOCKET_ERR,           DccEngine_SOCKET,    DccEngine::onSocketError),
    FXMAPFUNC(SOCKET_STARTACCEPT,   DccEngine_SOCKET,    DccEngine::onSocketStartAccept),
    FXMAPFUNC(SOCKET_LISTEN,        DccEngine_SOCKET,    DccEngine::onSocketListen),
    FXMAPFUNC(SOCKET_WRITTEN,       DccEngine_SOCKET,    DccEngine::onSocketWritten)
};

FXIMPLEMENT(DccEngine, FXObject, DccEngineMap, ARRAYNUMBER(DccEngineMap))

DccEngine::DccEngine(FXApp* app, FXObject* tgt, DccFile file, IrcEngine* engine)
        : m_application(app), m_target(tgt), m_file(file), m_engine(engine)
{
    m_connected = FALSE;
    m_socket = new dxSocket(app, this, DccEngine_SOCKET);
    m_positionChanged = FALSE;
    m_lastChange = 0;
    m_dataAmount = 0;
    m_dccTimeout = utils::instance().getIntIniEntry("SETTINGS", "dccTimeout", 66);
}

DccEngine::~DccEngine()
{
    m_connected = FALSE;
    closeFile();
    m_socket->disconnect();
    delete m_socket;
}

//for offered connectin by me
long DccEngine::onCloseTimeout(FXObject*, FXSelector, void*)
{
    m_connected = FALSE;
    m_file.canceled = TRUE;
    updateDcc();
    m_socket->disconnect();
    sendEvent(IRC_DISCONNECT, _("Connection closed. Client didn't connect in given timeout"), "", "", "");
    return 1;
}

//fired current position
long DccEngine::onPositionTimeout(FXObject*, FXSelector, void*)
{
    updateDcc();
    if(m_file.currentPosition < m_file.size && m_connected)
        m_application->addTimeout(this, DccEngine_PTIME, 1000);
    return 1;
}

long DccEngine::onSocketCanRead(FXObject*, FXSelector, void*)
{
    readData();
    return 1;
}

long DccEngine::onSocketWritten(FXObject*, FXSelector, void *ptr)
{
    if(m_file.type == DCC_OUT || m_file.type == DCC_POUT)
    {
        m_dataAmount += (FXint)(FXival)ptr;
        m_file.currentPosition += (FXint)(FXival)ptr;
        m_positionChanged = TRUE;
    }
    return 1;
}

long DccEngine::onSocketConnected(FXObject*, FXSelector, void*)
{
    if(m_file.type == DCC_IN)
    {
        if(m_file.currentPosition)
        {
            m_receivedFile.open(FXString(m_file.path+".part").text(), std::ios_base::binary|std::ios_base::ate|std::ios_base::app);
        }
        else
            m_receivedFile.open(FXString(m_file.path+".part").text(), std::ios_base::binary);
    }
    if(m_file.type == DCC_POUT)
    {
        m_sentFile.open(m_file.path.text(), std::ios_base::binary);
        if(m_file.currentPosition) m_sentFile.seekg(m_file.currentPosition, std::ios::beg);
        writeData();
    }
    m_connected = TRUE;
    m_application->addTimeout(this, DccEngine_PTIME, 1000);
    return 1;
}

long DccEngine::onSocketDisconnected(FXObject*, FXSelector, void*)
{
    m_connected = FALSE;
    m_application->removeTimeout(this, DccEngine_CTIME);
    closeFile();
    return 1;
}

long DccEngine::onSocketError(FXObject*, FXSelector, void *ptr)
{
    m_connected = FALSE;
    m_application->removeTimeout(this, DccEngine_CTIME);
    closeFile();
    SocketError *err = NULL;
    if(ptr) err = *((SocketError**)ptr);
    if(err)
    {
        if(!err->errorStr.empty())
            sendEvent(IRC_ERROR, err->errorStr);
        switch(err->errorType){
            case UNABLEINIT:
            {
                sendEvent(IRC_ERROR, _("Unable to initiliaze socket"));
                break;
            }
            case BADHOST:
            {
                sendEvent(IRC_ERROR, FXStringFormat(_("Bad host: %s"), m_file.ip.text()));
                break;
            }
            case UNABLECREATE:
            {
                sendEvent(IRC_ERROR, _("Unable to create socket"));
                break;
            }
            case UNABLECONNECT:
            {
                sendEvent(IRC_ERROR, FXStringFormat(_("Unable to connect to: %s"), m_file.ip.text()));
                break;
            }
            case SSLUNCREATE:
            {
                sendEvent(IRC_ERROR, _("SSL creation error"));
                break;
            }
            case SSLCONNECTERROR:
            {
                sendEvent(IRC_ERROR, _("SSL connect error"));
                break;
            }
            case UNABLEBIND:
            {
                sendEvent(IRC_ERROR, _("Unable to bind socket"));
                break;
            }
            case UNABLELISTEN:
            {
                sendEvent(IRC_ERROR, _("Unable to listen"));
                break;
            }
            case UNABLEACCEPT:
            {
                sendEvent(IRC_ERROR, _("Unable to accept connection"));
                break;
            }
            case UNABLESEND:
            {
                sendEvent(IRC_ERROR, _("Unable to send data"));
                break;
            }
            case UNABLEREAD:
            {
                sendEvent(IRC_ERROR, FXStringFormat(_("Error in reading data from %s"), m_file.ip.text()));
                break;
            }
            case SSLZERO:
            {
                sendEvent(IRC_ERROR, _("SSL_read() returns zero - closing socket"));
                break;
            }
            case SSLUNABLEREAD:
            {
                sendEvent(IRC_ERROR, _("SSL read problem"));
                break;
            }
            case SSLABNORMAL:
            {
                sendEvent(IRC_ERROR, _("Abnormal value from SSL read"));
                break;
            }
            case UNABLEREADBUFFER:
            {
                sendEvent(IRC_ERROR, _("Unable read data from buffer"));
                break;
            }
            default:
               break;
        }
        delete err;
    }
    return 1;
}

long DccEngine::onSocketListen(FXObject*, FXSelector, void*)
{
    m_application->removeTimeout(this, DccEngine_CTIME);
    if(m_file.type == DCC_OUT)
    {
        m_sentFile.open(m_file.path.text(), std::ios_base::binary);
        if(m_file.currentPosition) m_sentFile.seekg(m_file.currentPosition, std::ios::beg);
        writeData();
    }
    if(m_file.type == DCC_PIN)
    {
        m_receivedFile.open(FXString(m_file.path+".part").text(), std::ios_base::binary);
    }
    m_connected = TRUE;
    m_application->addTimeout(this, DccEngine_PTIME, 1000);
    return 1;
}

long DccEngine::onSocketStartAccept(FXObject*, FXSelector, void*)
{
    if(m_file.type == DCC_OUT)
    {
        m_file.port = m_socket->getPort();
        m_engine->sendCtcp(m_file.nick, "DCC SEND "+utils::instance().removeSpaces(m_file.path.rafter(PATHSEP))+" "+m_socket->stringIPToBinary(m_file.ip)+" "+FXStringVal(m_file.port)+" "+FXStringVal(m_file.size));
    }
    else if(m_file.type == DCC_PIN)
    {
        m_file.port = m_socket->getPort();
        m_engine->sendCtcp(m_file.nick, "DCC SEND "+utils::instance().removeSpaces(m_file.path.rafter(PATHSEP))+" "+m_socket->stringIPToBinary(m_file.ip)+" "+FXStringVal(m_file.port)+" "+FXStringVal(m_file.size)+" "+FXStringVal(m_file.token));
    }
    m_application->addTimeout(this, DccEngine_CTIME, m_dccTimeout*1000);
    return 1;
}

void DccEngine::sendEvent(IrcEventType eventType, const FXString &param1)
{
    IrcEvent ev;
    ev.eventType = eventType;
    ev.param1 = param1;
    ev.param2 = "";
    ev.param3 = "";
    ev.param4 = "";
    ev.dccFile = m_file;
    ev.time = FXSystem::now();
    m_target->handle(this, FXSEL(SEL_COMMAND, DccEngine_DCC), &ev);
}

void DccEngine::sendEvent(IrcEventType eventType, const FXString &param1, const FXString &param2, const FXString &param3, const FXString &param4)
{
    IrcEvent ev;
    ev.eventType = eventType;
    ev.param1 = param1;
    ev.param2 = param2;
    ev.param3 = param3;
    ev.param4 = param4;
    ev.dccFile = m_file;
    ev.time = FXSystem::now();
    m_target->handle(this, FXSEL(SEL_COMMAND, DccEngine_DCC), &ev);
}

void DccEngine::updateDcc()
{
    if(m_positionChanged)
    {
        m_file.speed = m_dataAmount - m_file.speed;
        m_lastChange = FXSystem::now();
        m_dataAmount = m_file.speed;
    }
    else
    {
        if(FXSystem::now() != m_lastChange) m_file.speed = m_dataAmount/(FXSystem::now()-m_lastChange);
        else m_file.speed = 0;
    }
    IrcEvent ev;
    ev.eventType = IRC_DCCPOSITION;
    ev.param1 = "";
    ev.param2 = "";
    ev.param3 = "";
    ev.param4 = "";
    ev.dccFile = m_file;
    ev.time = FXSystem::now();
    m_target->handle(this, FXSEL(SEL_COMMAND, DccEngine_DCC), &ev);
    m_positionChanged = FALSE;
}

void DccEngine::startConnection()
{
    if(m_connected)
        return;
    FXbool listen;
    if(m_file.type == DCC_OUT || m_file.type == DCC_PIN)
    {
        listen = TRUE;
        FXString address = utils::instance().getStringIniEntry("SETTINGS", "dccIP");
        if(address.empty()) address = m_engine->getMyUserHost();
        if(address.empty()) address = m_engine->getLocalIP();
        if(address.empty())
        {
            m_file.canceled = TRUE;
            return;
        }
        m_file.ip = address;
    }
    else
        listen = FALSE;
    m_socket->setIsSSL(FALSE);
    m_socket->setHost(m_file.ip, m_file.port);
    m_socket->setTarget(this);
    if(listen) m_socket->listenOn();
    else m_socket->connectTo();
}

void DccEngine::disconnect()
{
    m_connected = FALSE;
    closeFile();
    m_socket->disconnect();
}

void DccEngine::readData()
{
    if(m_file.type == DCC_IN || m_file.type == DCC_PIN)
    {
        FXint read = m_socket->getBytesAvailable();
        if(read<=0) read = 4096;
        FXchar *buffer = new FXchar[read];
        FXint size = 0;
        size = m_socket->read(buffer, read);
        m_file.currentPosition += size;
        if(m_receivedFile.good())
            m_receivedFile.write(buffer, size);
        FXlong pos = htonl(m_file.currentPosition);
        m_socket->write(reinterpret_cast<char *>(&pos), 4);
        if(m_file.currentPosition >= m_file.size)
            closeFile();
        m_positionChanged = TRUE;
        m_dataAmount += size;
        delete []buffer;
    }
    else
    {
        while(m_socket->getBytesAvailable())
        {
            FXulong pos;
            m_socket->read(reinterpret_cast<FXchar*>(&pos), 4);
            pos = ntohl(pos);
            m_file.finishedPosition = pos;
        }
        if(m_file.finishedPosition >= m_file.size)
        {
            closeFile();
            m_socket->disconnect();
        }
        writeData();
    }
}

void DccEngine::writeData()
{
    if(m_file.type == DCC_IN || m_file.type == DCC_PIN)
        return;
    if(m_file.currentPosition >= m_file.size)
        return;
    FXchar buf[4096];
    if(m_sentFile.good())
        m_sentFile.read(buf, 4096);
    int readedChars = (int)m_sentFile.gcount();
    m_socket->write(buf, FXMIN(readedChars, m_file.size-m_file.currentPosition));
}

void DccEngine::closeFile()
{
    if(m_file.type == DCC_IN || m_file.type == DCC_PIN)
    {
        while(m_socket->getBytesAvailable())
        {
            FXchar buffer[4096];
            FXint size = 0;
            size = m_socket->read(buffer, 4096);
            m_file.currentPosition += size;
            if(m_receivedFile.good())
                m_receivedFile.write(buffer, size);
            FXlong pos = htonl(m_file.currentPosition);
            if(m_socket->isConnected()) m_socket->write(reinterpret_cast<char *>(&pos), 4);
        }
        m_receivedFile.close();
        if(m_file.currentPosition < m_file.size)
            m_file.canceled = TRUE;
        else
            FXFile::rename(FXString(m_file.path+".part"), m_file.path);
    }
    if(m_file.type == DCC_POUT || m_file.type == DCC_OUT)
    {
        m_sentFile.close();
        while(m_socket->getBytesAvailable())
        {
            FXulong pos;
            m_socket->read(reinterpret_cast<FXchar*>(&pos), 4);
            pos = ntohl(pos);
            m_file.finishedPosition = pos;
        }
        if(m_file.finishedPosition < m_file.size)
            m_file.canceled = TRUE;
    }
}

//change position in file mainly for resumed file
FXbool DccEngine::setDccPosition(FXulong position)
{
    if(position >= m_file.size) return FALSE;
    m_file.currentPosition = position;
    m_file.speed = 0;
    m_file.finishedPosition = position;
    m_file.canceled = FALSE;
    return TRUE;
}

//check dccfile for resume
FXbool DccEngine::isForResume(const FXString& nick, const FXString& name, FXint port)
{
    return m_file.nick == nick && FXPath::name(m_file.path) == name
                && m_file.port == port;
}

//check dccfile for resume
FXbool DccEngine::isForResume(FXint token)
{
    return m_file.token == token;
}
