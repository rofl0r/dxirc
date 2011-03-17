/*
 *      ircengine.cpp
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

#include "ircengine.h"
#include "config.h"
#include "i18n.h"
#include "utils.h"
#include "irctabitem.h"

FXDEFMAP(IrcEngine) IrcEngineMap[] = {
    FXMAPFUNC(SOCKET_CANREAD,       IrcEngine_DXSOCKET, IrcEngine::onSocketCanRead),
    FXMAPFUNC(SOCKET_CONNECTED,     IrcEngine_DXSOCKET, IrcEngine::onSocketConnected),
    FXMAPFUNC(SOCKET_DISCONNECTED,  IrcEngine_DXSOCKET, IrcEngine::onSocketDisconnected),
    FXMAPFUNC(SOCKET_ERR,           IrcEngine_DXSOCKET, IrcEngine::onSocketError),
    FXMAPFUNC(SEL_TIMEOUT,          IrcEngine_RTIME,    IrcEngine::onReconnectTimeout),
    FXMAPFUNC(SEL_TIMEOUT,          IrcEngine_TATIME,   IrcEngine::onTryAgainTimeout)
};

FXIMPLEMENT(IrcEngine, FXObject, IrcEngineMap, ARRAYNUMBER(IrcEngineMap))

IrcEngine::IrcEngine(FXApp *app, FXObject *tgt, FXString channels, FXString commands)
    : m_application(app), m_startChannels(utils::instance().removeSpaces(channels)), m_startCommands(commands)
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
    m_chanTypes = "#&+!";
    m_myUserHost = "";
    m_ignoreWho = "";
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
    m_socket = new dxSocket(app, this, IrcEngine_DXSOCKET);
}

IrcEngine::~IrcEngine()
{
    m_socket->disconnect();
    delete m_socket;
}

void IrcEngine::setUserName(const FXString& user)
{
    //needed for freenode
    m_userName = utils::instance().removeNonalphanumeric(user);
}

long IrcEngine::onReconnectTimeout(FXObject*, FXSelector, void*)
{
    if(m_attempts < m_numberAttempt && !m_connected)
    {
        if(m_connecting)
        {
            m_application->addTimeout(this, IrcEngine_RTIME, 1000);
            return 1;
        }
        clearChannelsCommands(FALSE);
        startConnection();
        m_application->addTimeout(this, IrcEngine_RTIME, m_delayAttempt*1000);
    }
    else
        clearAttempts();
    return 1;
}

//timeout after RPL_TRYAGAIN
long IrcEngine::onTryAgainTimeout(FXObject*, FXSelector, void*)
{
    if(!m_ignoreWho.empty() && !m_chanTypes.contains(m_ignoreWho[0]))
    {
        sendWho(m_ignoreWho);
        return 1;
    }
    if(m_ignoreWhoQueue.no())
    {
        m_ignoreWho = m_ignoreWhoQueue[0];
        m_ignoreWhoQueue.erase(0);
        sendWho(m_ignoreWho);
    }
    else
        m_ignoreWho = "";
    return 1;
}

long IrcEngine::onSocketCanRead(FXObject *, FXSelector, void *)
{
    if(m_connected)
    {
        readData();
    }
    return 1;
}

long IrcEngine::onSocketConnected(FXObject*, FXSelector, void*)
{
    m_connected = TRUE;
    m_connecting = FALSE;
    clearAttempts();
    if(getRemoteIP().empty()) sendEvent(IRC_CONNECT, FXStringFormat(_("Connected to %s - port %d"), m_serverName.text(), m_serverPort));
    else sendEvent(IRC_CONNECT, FXStringFormat(_("Connected to %s (%s) - port %d"), m_serverName.text(), getRemoteIP().text(), m_serverPort));
    if(!m_serverPassword.empty())
        sendLine("PASS "+m_serverPassword+"\r\n");
    sendLine("NICK "+m_nickName+"\r\n");
    sendLine("USER "+m_userName+" 0 * :"+m_realName+"\r\n");
    return 1;
}

long IrcEngine::onSocketDisconnected(FXObject*, FXSelector, void*)
{
    m_connected = FALSE;
    m_connecting = FALSE;
    if(m_reconnect && m_attempts < m_numberAttempt)
    {
        sendEvent(IRC_RECONNECT, FXStringFormat(_("Server %s was disconnected"), m_serverName.text()));
        m_application->addTimeout(this, IrcEngine_RTIME, m_delayAttempt*1000);
    }
    else
    {
        m_application->removeTimeout(this, IrcEngine_RTIME);
        sendEvent(IRC_DISCONNECT, FXStringFormat(_("Server %s was disconnected"), m_serverName.text()));
    }
    return 1;
}

long IrcEngine::onSocketError(FXObject*, FXSelector, void *ptr)
{
    m_connected = FALSE;
    m_connecting = FALSE;
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
                sendEvent(IRC_ERROR, FXStringFormat(_("Bad host: %s"), m_serverName.text()));
                break;
            }
            case UNABLECREATE:
            {
                sendEvent(IRC_ERROR, _("Unable to create socket"));
                break;
            }
            case UNABLECONNECT:
            {
                sendEvent(IRC_ERROR, FXStringFormat(_("Unable to connect to: %s"), m_serverName.text()));
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
                resetReconnect();
                break;
            }
            case UNABLEREAD:
            {
                sendEvent(IRC_ERROR, FXStringFormat(_("Error in reading data from %s"), m_serverName.text()));
                resetReconnect();
                break;
            }
            case SSLZERO:
            {
                sendEvent(IRC_ERROR, _("SSL_read() returns zero - closing socket"));
                resetReconnect();
                break;
            }
            case SSLUNABLEREAD:
            {
                sendEvent(IRC_ERROR, _("SSL read problem"));
                resetReconnect();
                break;
            }
            case SSLABNORMAL:
            {
                sendEvent(IRC_ERROR, _("Abnormal value from SSL read"));
                resetReconnect();
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

void IrcEngine::startConnection()
{
    if(m_connected || m_connecting)
        return;
    m_connecting = TRUE;
    sendEvent(IRC_CONNECT, FXStringFormat(_("Connecting to %s"), m_serverName.text()));
    utils::instance().debugLine("StartConnection");
    utils::instance().debugLine(FXStringFormat("Attempts on %s-%d-%s: %d", m_serverName.text(), m_serverPort, m_nickName.text(), m_attempts));
    utils::instance().debugLine("Connect");
    utils::instance().debugLine(FXStringFormat("startChannels: %s", m_startChannels.text()));
    utils::instance().debugLine(FXStringFormat("startCommands: %s", m_startCommands.text()));
    m_attempts++;
    m_endmotd = FALSE;
    if(m_attempts<2)
    {
        m_socket->setIsSSL(m_useSsl);
        m_socket->setHost(m_serverName, m_serverPort);
    }
    m_socket->setTarget(this);
    m_socket->connectTo();
}

void IrcEngine::disconnect()
{
    utils::instance().debugLine(FXStringFormat("Quit: %s-%d-%s", m_serverName.text(), m_serverPort, m_nickName.text()));
    if(m_connected) sendLine("QUIT\r\n");
    closeConnection(TRUE);
}

void IrcEngine::disconnect(const FXString& reason)
{
    utils::instance().debugLine(FXStringFormat("Quit: %s-%d-%s", m_serverName.text(), m_serverPort, m_nickName.text()));
    if(m_connected) sendLine("QUIT :"+reason+"\r\n");
    closeConnection(TRUE);
}

void IrcEngine::closeConnection(FXbool disableReconnect)
{
    m_ignoreWho = "";
    m_ignoreWhoQueue.clear();
    clearChannelsCommands(disableReconnect);
    if(!m_connected && disableReconnect)
    {
        m_application->removeTimeout(this, IrcEngine_RTIME);
        return;
    }
    m_connected = FALSE;
    m_connecting = FALSE;
    if(m_reconnect && m_attempts < m_numberAttempt && !disableReconnect)
    {
        sendEvent(IRC_RECONNECT, FXStringFormat(_("Server %s was disconnected"), m_serverName.text()));
        m_application->addTimeout(this, IrcEngine_RTIME, m_delayAttempt*1000);
    }
    else
    {
        m_application->removeTimeout(this, IrcEngine_RTIME);
        sendEvent(IRC_DISCONNECT, FXStringFormat(_("Server %s was disconnected"), m_serverName.text()));
    }
    m_socket->disconnect();
}

void IrcEngine::readData()
{
    FXchar buffer[1024];
    FXint size = 0;
    FXString data = m_receiveRest;
    while((size = m_socket->read(buffer, 1023)) > 0)
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
}

void IrcEngine::parseLine(const FXString &line)
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
    if(command.length() == 3)
    {
        FXint ncommand = FXIntVal(command);
        if((ncommand==376||ncommand==422)&&m_realServerName.empty())
            m_realServerName = from;
        numeric(ncommand, params);
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

void IrcEngine::numeric(const FXint &command, const FXString &params)
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
        case 263: //RPL_TRYAGAIN
        {
            //this reply is for 99,9% thru sended WHO
            if(m_application->hasTimeout(this, IrcEngine_TATIME))
                m_application->removeTimeout(this, IrcEngine_TATIME);
            m_application->addTimeout(this, IrcEngine_TATIME, 60000);
            m_ignoreWho = "";
            utils::instance().debugLine("RPL_TRYAGAIN fired!!!");
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
                m_myUserHost = m_socket->getIP(utils::instance().getParam(params, 2, TRUE).after('@'));
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
            FXString who = utils::instance().getParam(params, 2, FALSE);
            if(comparecase(m_ignoreWho, who) == 0)
            {
                ignoreEvent = TRUE;
                sendEvent(IRC_AWAY, who);
                if(m_ignoreWhoQueue.no())
                {
                    m_ignoreWho = m_ignoreWhoQueue[0];
                    m_ignoreWhoQueue.erase(0);
                    sendWho(m_ignoreWho);
                }
                else
                    m_ignoreWho = "";
                break;
            }
            if(comparecase("*", who) == 0)
            {
                ignoreEvent = TRUE;
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
            FXString nick = utils::instance().getParam(params, 6, FALSE);
            FXString channel = utils::instance().getParam(params, 2, FALSE);
            if(comparecase(m_ignoreWho, channel)==0 || comparecase(m_ignoreWho, nick)==0)
            {
                //param:xxx #test ~dvx localhost dvx.irc.dvx dvx H :0 dvx
                addNick(nick, utils::instance().getParam(params, 3, FALSE), utils::instance().getParam(params, 9, TRUE), utils::instance().getParam(params, 4, FALSE), utils::instance().getParam(params, 7, FALSE)[0] == 'H' ? FALSE : TRUE);
                ignoreEvent = TRUE;
                break;
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
void IrcEngine::parseRplsupport(FXString text)
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
            for(FXint j=0; j<modes.length(); j++)
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
        if(!comparecase(parameter, "NETWORK"))
        {
            m_realServerName = value;
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

void IrcEngine::privmsg(const FXString &from, const FXString &params)
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

void IrcEngine::ctcp(const FXString &from, const FXString &params)
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
            sendEvent(IRC_DCCCHAT, nick, m_socket->binaryIPToString(utils::instance().getParam(ctcpRest, 3, FALSE)), utils::instance().getParam(ctcpRest, 4, FALSE));
        }
        if(dccCommand == "SEND")
        {
            if(isUserIgnored(nick, from.after('!').before('@'), from.after('@'), to))
                return;
            if(utils::instance().getParam(ctcpRest, 5, FALSE) != utils::instance().getParam(ctcpRest, 6, FALSE)) //passive send
            {
                if(FXIntVal(utils::instance().getParam(ctcpRest, 4, FALSE)))
                    sendEvent(IRC_DCCMYTOKEN, m_socket->binaryIPToString(utils::instance().getParam(ctcpRest, 3, FALSE)), utils::instance().getParam(ctcpRest, 4, FALSE), utils::instance().getParam(ctcpRest, 6, FALSE));
                else
                    sendEvent(IRC_DCCTOKEN, nick, utils::instance().getParam(ctcpRest, 2, FALSE), utils::instance().getParam(ctcpRest, 5, FALSE), utils::instance().getParam(ctcpRest, 6, FALSE));
            }
            else
                sendEvent(IRC_DCCIN, nick, m_socket->binaryIPToString(utils::instance().getParam(ctcpRest, 3, FALSE))+"@"+utils::instance().getParam(ctcpRest, 4, FALSE), utils::instance().getParam(ctcpRest, 2, FALSE), utils::instance().getParam(ctcpRest, 5, FALSE));
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
    else
    {
        if(isUserIgnored(nick, from.after('!').before('@'), from.after('@'), to))
            return;
        sendEvent(IRC_CTCPREQUEST, nick, ctcpCommand);
    }
}

void IrcEngine::join(const FXString &from, const FXString &params)
{
    FXString channel = utils::instance().getParam(params, 1, FALSE);
    if (channel[0] == ':') channel = channel.after(':');
    FXString nick = from.before('!');
    if(m_nickName == nick)
    {
        addNick(m_nickName, m_userName, m_realName, from.after('@'));
        sendEvent(IRC_NEWCHANNEL, (channel[0] == '&' ? "&"+channel : channel));
        return;
    }
    else
    {
        sendEvent(IRC_JOIN, nick, channel);
        if(m_ignoreWho==nick)
        {
            return;
        }
        for(FXint i=0; i < m_nicks.no(); i++)
        {
            if(m_nicks[i].nick==nick)
                return;
        }
        addIgnoreWho(nick); //for update NickInfo
        addNick(nick, from.after('!').before('@'), "", from.after('@'), FALSE);
    }
}

void IrcEngine::quitirc(const FXString &from, const FXString &params)
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

void IrcEngine::part(const FXString &from, const FXString &params)
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

void IrcEngine::ping(const FXString &params)
{
    sendLine("PONG "+params+"\r\n");
}

void IrcEngine::pong(const FXString &from, const FXString &params)
{
    sendEvent(IRC_SERVERREPLY, _("PONG from ")+from+" "+params.after(':'));
}

void IrcEngine::notice(const FXString &from, const FXString &params)
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

void IrcEngine::nick(const FXString &from, const FXString &params)
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

void IrcEngine::topic(const FXString &from, const FXString &params)
{
    FXString nick = from.before('!');
    FXString channel = utils::instance().getParam(params, 1, FALSE);
    FXString topic = utils::instance().getParam(params, 2, TRUE).after(':');
    sendEvent(IRC_TOPIC, nick, channel, topic);
}

void IrcEngine::invite(const FXString &from, const FXString &params)
{
    FXString nick = from.before('!');
    FXString to = utils::instance().getParam(params, 1, FALSE);
    FXString channel = utils::instance().getParam(params, 2, FALSE);
    if (channel[0] == ':') channel = channel.after(':');
    if(!isUserIgnored(nick, from.after('!').before('@'), from.after('@'), channel)) sendEvent(IRC_INVITE, nick, to, channel);
}

void IrcEngine::kick(const FXString &from, const FXString &params)
{
    FXString nick = from.before('!');
    FXString to = utils::instance().getParam(params, 2, FALSE);
    FXString channel = utils::instance().getParam(params, 1, FALSE);
    FXString reason = utils::instance().getParam(params, 3, TRUE).after(':');
    sendEvent(IRC_KICK, nick, to, channel, reason);
}

void IrcEngine::mode(const FXString &from, const FXString &params)
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

void IrcEngine::error(const FXString &params)
{
    sendEvent(IRC_SERVERERROR, params[0] == ':' ? params.after(':') : params);
}

void IrcEngine::unknown(const FXString &command, const FXString &params)
{
    sendEvent(IRC_UNKNOWN, command, params);
}

void IrcEngine::addIgnoreWho(const FXString& who)
{
    if(m_ignoreWhoQueue.no())
    {
        for(FXint i=0; i<m_ignoreWhoQueue.no(); i++)
        {
            if(who==m_ignoreWhoQueue[i])
                return;
        }
        m_ignoreWhoQueue.append(who);
    }
    else
    {
        if(m_ignoreWho.empty())
        {
            m_ignoreWho = who;
            if(!m_application->hasTimeout(this, IrcEngine_TATIME)) sendWho(who);
            else m_ignoreWhoQueue.append(who);
        }
        else
            m_ignoreWhoQueue.append(who);
    }
}

FXbool IrcEngine::sendAdmin(const FXString& params)
{
    return sendLine("ADMIN "+params+"\r\n");
}

FXbool IrcEngine::sendAway(const FXString& params)
{
    return sendLine("AWAY :"+params+"\r\n");
}

FXbool IrcEngine::sendBanlist(const FXString& channel)
{
    return sendLine("MODE "+channel+" +b\r\n");
}

FXbool IrcEngine::sendCtcp(const FXString& to, const FXString& params)
{
    return sendLine("PRIVMSG "+to+" :\001"+params+"\001\r\n");
}

FXbool IrcEngine::sendCtcpNotice(const FXString& to, const FXString& params)
{
    return sendLine("NOTICE "+to+" :\001"+params+"\001\r\n");
}

FXbool IrcEngine::sendMode(const FXString& params)
{
    return sendLine("MODE "+params+"\r\n");
}

FXbool IrcEngine::sendInvite(const FXString& to, const FXString& params)
{
    return sendLine("INVITE "+to+" "+params+"\r\n");
}

FXbool IrcEngine::sendJoin(const FXString& chan)
{
    return sendLine("JOIN "+chan+"\r\n");
}

FXbool IrcEngine::sendKick(const FXString& chan, const FXString& nick, const FXString& reason)
{
    return sendLine("KICK "+chan+" "+nick+" :"+reason+"\r\n");
}

FXbool IrcEngine::sendKill(const FXString& nick, const FXString& reason)
{
    return sendLine("KILL "+nick+" :"+reason+"\r\n");
}

FXbool IrcEngine::sendList(const FXString& params)
{
    return sendLine("LIST "+params+"\r\n");
}

FXbool IrcEngine::sendMe(const FXString& to, const FXString& message)
{
    return sendLine("PRIVMSG "+to+" :\001ACTION "+message+"\001\r\n");
}

FXbool IrcEngine::sendMsg(const FXString& to, const FXString& message)
{
    return sendLine("PRIVMSG "+to+" :"+message+"\r\n");
}

FXbool IrcEngine::sendNames(const FXString& channel)
{
    return sendLine("NAMES "+channel+"\r\n");
}

FXbool IrcEngine::sendNick(const FXString& nick)
{
    return sendLine("NICK "+nick+"\r\n");
}

FXbool IrcEngine::sendNotice(const FXString& to, const FXString& message)
{
    return sendLine("NOTICE "+to+" :"+message+"\r\n");
}

FXbool IrcEngine::sendOper(const FXString& login, const FXString& password)
{
    return sendLine("OPER "+login+" "+password+"\r\n");
}

FXbool IrcEngine::sendPart(const FXString& chan)
{
    return sendLine("PART "+chan+"\r\n");
}

FXbool IrcEngine::sendPart(const FXString& chan, const FXString& reason)
{
    return sendLine("PART "+chan+" :"+reason+"\r\n");
}

FXbool IrcEngine::sendQuote(const FXString& text)
{
    return sendLine(text+"\r\n");
}

FXbool IrcEngine::sendStats(const FXString& type)
{
    if(type.empty())
        return sendLine("STATS\r\n");
    else
        return sendLine("STATS "+type+"\r\n");
}

FXbool IrcEngine::sendTopic(const FXString& chan, const FXString& topic)
{
    if (topic.empty())
        return sendLine("TOPIC "+chan+"\r\n");
    else
        return sendLine("TOPIC "+chan+" :"+topic+"\r\n");
}

FXbool IrcEngine::sendTopic(const FXString& chan)
{
    return sendLine("TOPIC "+chan+"\r\n");
}

FXbool IrcEngine::sendWallops(const FXString& msg)
{
    return sendLine("WALLOPS :"+msg+"\r\n");
}

FXbool IrcEngine::sendVersion(const FXString& to)
{
    return sendLine("NOTICE "+to+FXStringFormat(" :\001VERSION dxirc %s (C) 2008~ by David Vachulka\001\r\n", VERSION));
}

FXbool IrcEngine::sendWho(const FXString& mask)
{
    return sendLine("WHO "+mask+"\r\n");
}

FXbool IrcEngine::sendWhoami()
{
    return sendLine("WHOIS "+m_nickName+"\r\n");
}

FXbool IrcEngine::sendWhois(const FXString& params)
{
    return sendLine("WHOIS "+params+"\r\n");
}

FXbool IrcEngine::sendWhowas(const FXString& params)
{
    return sendLine("WHOWAS "+params+"\r\n");
}

FXbool IrcEngine::sendLine(const FXString& line)
{
    utils::instance().debugLine(FXStringFormat(">> %s", line.before('\n').text()));
    FXint size=0;
    if(m_connected)
    {
        size = m_socket->write(line, TRUE);
        if(size != -1) return TRUE;
        else return FALSE;
    }
    else return FALSE;
}

void IrcEngine::sendCommands()
{
    if(!m_startCommands.contains('\n')) m_startCommands.append('\n');
    if(m_startCommands.right(1) != "\n") m_startCommands.append('\n');
    for(FXint i=0; i < m_startCommands.contains('\n'); i++)
    {
        sendCommand(m_startCommands.section('\n', i));
    }
}

FXbool IrcEngine::sendCommand(const FXString& commandtext)
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

void IrcEngine::sendEvent(IrcEventType eventType)
{
    IrcEvent ev;
    ev.eventType = eventType;
    ev.param1 = "";
    ev.param2 = "";
    ev.param3 = "";
    ev.param4 = "";
    ev.dccFile = DccFile();
    ev.time = FXSystem::now();
    for(FXint i=0; i < m_targets.no(); i++)
    {
        m_targets.at(i)->handle(this, FXSEL(SEL_COMMAND, IrcEngine_SERVER), &ev);
    }
}

void IrcEngine::sendEvent(IrcEventType eventType, const FXString &param1)
{
    IrcEvent ev;
    ev.eventType = eventType;
    ev.param1 = param1;
    ev.param2 = "";
    ev.param3 = "";
    ev.param4 = "";
    ev.dccFile = DccFile();
    ev.time = FXSystem::now();
    for(FXint i=0; i < m_targets.no(); i++)
    {
        m_targets.at(i)->handle(this, FXSEL(SEL_COMMAND, IrcEngine_SERVER), &ev);
    }
}

void IrcEngine::sendEvent(IrcEventType eventType, const FXString &param1, const FXString &param2)
{
    IrcEvent ev;
    ev.eventType = eventType;
    ev.param1 = param1;
    ev.param2 = param2;
    ev.param3 = "";
    ev.param4 = "";
    ev.dccFile = DccFile();
    ev.time = FXSystem::now();
    for(FXint i=0; i < m_targets.no(); i++)
    {
        m_targets.at(i)->handle(this, FXSEL(SEL_COMMAND, IrcEngine_SERVER), &ev);
    }
}

void IrcEngine::sendEvent(IrcEventType eventType, const FXString &param1, const FXString &param2, const FXString &param3)
{
    IrcEvent ev;
    ev.eventType = eventType;
    ev.param1 = param1;
    ev.param2 = param2;
    ev.param3 = param3;
    ev.param4 = "";
    ev.dccFile = DccFile();
    ev.time = FXSystem::now();
    for(FXint i=0; i < m_targets.no(); i++)
    {
        m_targets.at(i)->handle(this, FXSEL(SEL_COMMAND, IrcEngine_SERVER), &ev);
    }
}

void IrcEngine::sendEvent(IrcEventType eventType, const FXString &param1, const FXString &param2, const FXString &param3, const FXString &param4)
{
    IrcEvent ev;
    ev.eventType = eventType;
    ev.param1 = param1;
    ev.param2 = param2;
    ev.param3 = param3;
    ev.param4 = param4;
    ev.dccFile = DccFile();
    ev.time = FXSystem::now();
    for(FXint i=0; i < m_targets.no(); i++)
    {
        m_targets.at(i)->handle(this, FXSEL(SEL_COMMAND, IrcEngine_SERVER), &ev);
    }
}

void IrcEngine::appendTarget(FXObject *tgt)
{
     m_targets.append(tgt);
}

void IrcEngine::removeTarget(FXObject *tgt)
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

FXbool IrcEngine::findTarget(FXObject *tgt)
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

FXString IrcEngine::getLocalIPBinary()
{
    return m_socket->stringIPToBinary(getLocalIP());
}

FXString IrcEngine::getHostname(const FXString& address)
{
    return m_socket->getHostname(address);
}

void IrcEngine::clearChannelsCommands(FXbool disableReconnect)
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

void IrcEngine::makeStartChannels()
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

FXbool IrcEngine::clearTarget()
{
    m_targets.clear();
    if(m_targets.no()) return FALSE;
    else return TRUE;
}

void IrcEngine::addNick(const FXString &nick, const FXString &user, const FXString &real, const FXString &host, const FXbool &away)
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

void IrcEngine::addNick(const FXString &nick, const FXString &user, const FXString &real, const FXString &host)
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

void IrcEngine::removeNick(const FXString &nick)
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

NickInfo IrcEngine::getNickInfo(const FXString &nick) const
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

FXString IrcEngine::getBannedNick(const FXString &banmask) const
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

FXbool IrcEngine::isAway(const FXString& nick)
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

FXbool IrcEngine::isUserIgnored(const FXString &nick, const FXString &user, const FXString &host, const FXString &on)
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

FXbool IrcEngine::isUserIgnored(const FXString &nick, const FXString &on)
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
void IrcEngine::resetReconnect()
{
    if(m_application->hasTimeout(this, IrcEngine_RTIME))
    {
        m_application->removeTimeout(this, IrcEngine_RTIME);
        m_application->addTimeout(this, IrcEngine_RTIME, m_delayAttempt*1000);
        return;
    }
    if(m_reconnect && m_attempts < m_numberAttempt)
    {
        m_application->addTimeout(this, IrcEngine_RTIME, m_delayAttempt*1000);
    }
}

