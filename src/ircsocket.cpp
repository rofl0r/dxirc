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

#include <FX885910Codec.h>
#include <FX885911Codec.h>
#include <FX885913Codec.h>
#include <FX885914Codec.h>
#include <FX885915Codec.h>
#include <FX885916Codec.h>
#include <FX88591Codec.h>
#include <FX88592Codec.h>
#include <FX88593Codec.h>
#include <FX88594Codec.h>
#include <FX88595Codec.h>
#include <FX88596Codec.h>
#include <FX88597Codec.h>
#include <FX88598Codec.h>
#include <FX88599Codec.h>
#include <FXCP1250Codec.h>
#include <FXCP1251Codec.h>
#include <FXCP1252Codec.h>
#include <FXCP1253Codec.h>
#include <FXCP1254Codec.h>
#include <FXCP1255Codec.h>
#include <FXCP1256Codec.h>
#include <FXCP1257Codec.h>
#include <FXCP1258Codec.h>
#include <FXCP437Codec.h>
#include <FXCP850Codec.h>
#include <FXCP852Codec.h>
#include <FXCP855Codec.h>
#include <FXCP856Codec.h>
#include <FXCP857Codec.h>
#include <FXCP860Codec.h>
#include <FXCP861Codec.h>
#include <FXCP862Codec.h>
#include <FXCP863Codec.h>
#include <FXCP864Codec.h>
#include <FXCP865Codec.h>
#include <FXCP866Codec.h>
#include <FXCP869Codec.h>
#include <FXCP874Codec.h>
#include <FXKOI8RCodec.h>
#include <FXUTF8Codec.h>


FXDEFMAP(IrcSocket) IrcSocketMap[] = {
    FXMAPFUNC(SEL_IO_READ,      IrcSocket::ID_READ,     IrcSocket::OnIORead)
};

FXIMPLEMENT(IrcSocket, FXObject, IrcSocketMap, ARRAYNUMBER(IrcSocketMap))

IrcSocket::IrcSocket(FXApp *app, FXObject *tgt, FXSelector sel, FXString channels)
    : application(app), selector(sel), startChannels(channels)
{
    targets.append(tgt);
    serverName = "localhost";
    realServerName = "";
    serverPort = 6667;
    serverPassword = "";
    nickName = "xxx";
    userName = nickName;
    realName = nickName;
    receiveRest = "";
    connected = false;
    FillCommands();
}

IrcSocket::~IrcSocket()
{
    if(connected) Disconnect();
}

long IrcSocket::OnIORead(FXObject *, FXSelector, void *)
{
    if(connected) ReadData();
    return 1;
}

FXint IrcSocket::Connect()
{
#ifdef WIN32
    WORD wVersionRequested = MAKEWORD(2,0);
    WSADATA data;
#endif
    hostent *host;
    SendEvent(IRC_CONNECT, FXStringFormat(_("Connecting to %s"), serverName.text()));
#ifdef WIN32
    if (WSAStartup(wVersionRequested, &data) != 0)
    {
        SendEvent(IRC_ERROR, _("Unable initiliaze socket"));
        startChannels.clear();
        return -1;
    }
#endif
    if ((host = gethostbyname(serverName.text())) == NULL)
    {
        SendEvent(IRC_ERROR, FXStringFormat(_("Bad host: %s"), serverName.text()));
        startChannels.clear();
        return -1;
    }
    if ((socketid = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    {
        SendEvent(IRC_ERROR, _("Unable create socket"));
        startChannels.clear();
#ifdef WIN32
        WSACleanup();
#endif
        return -1;
    }
    serverSock.sin_family = AF_INET;
    serverSock.sin_port = htons(serverPort);
    memcpy(&(serverSock.sin_addr), host->h_addr, host->h_length);
    if (connect(socketid, (sockaddr *)&serverSock, sizeof(serverSock)) == -1)
    {
        SendEvent(IRC_ERROR, FXStringFormat(_("Unable connect to: %s"), serverName.text()));
        startChannels.clear();
        return -1;
    }
#ifdef WIN32
    event = WSACreateEvent();
    WSAEventSelect(socketid, event, FD_CONNECT|FD_READ|FD_CLOSE); // sets non-blocking!!
    application->addInput((FXInputHandle)event, INPUT_READ, this, ID_READ);
#else
    application->addInput((FXInputHandle)socketid, INPUT_READ, this, ID_READ);
#endif
    connected = true;
    SendEvent(IRC_CONNECT, FXStringFormat(_("Connected to %s"), serverName.text()));
    if (!serverPassword.empty()) SendLine("PASS "+serverPassword);
    SendLine("NICK "+nickName);
    SendLine("USER "+userName+" 0 * :"+realName);
    return 1;
}

void IrcSocket::Disconnect()
{
    SendLine("QUIT");
    CloseConnection();
}

void IrcSocket::Disconnect(const FXString& reason)
{
    SendLine("QUIT :"+reason);
    CloseConnection();
}

void IrcSocket::CloseConnection()
{
    connected = false;
    SendEvent(IRC_DISCONNECT, FXStringFormat(_("Server %s was disconnected"), serverName.text()));
    startChannels.clear();
#ifdef WIN32
    shutdown(socketid, SD_BOTH);
    closesocket(socketid);
    if(event)
    {
        application->removeInput((FXInputHandle)event, INPUT_READ);
        WSACloseEvent(event);
        event = NULL;
    }
#else
#ifndef SHUT_RDWR
#define SHUT_RDWR 2
#endif
    shutdown(socketid, SHUT_RDWR);
    close(socketid);
    application->removeInput(socketid, INPUT_READ);
#endif
}

long IrcSocket::ReadData()
{
    FXchar buffer[1024];
    int size;

    FXString data = receiveRest;
#ifdef WIN32
    WSANETWORKEVENTS network_events;
    WSAEnumNetworkEvents(socketid, event, &network_events);
    if (network_events.lNetworkEvents&FD_READ)
    {
        size = recv(socketid, buffer, 1023, 0);
        if (size > 0)
        {
            buffer[size] = '\0';
            if (IsUtf8(buffer, size)) data.append(buffer);
            else data.append(utils::LocaleToUtf8(buffer));
            while (data.contains('\n'))
            {
                ParseLine(data.before('\n').before('\r'));
                data = data.after('\n');
            }
            receiveRest = data;
        }
        else if (size < 0)
        {
            SendEvent(IRC_ERROR, FXStringFormat(_("Error in reading data from %s"), serverName.text()));
            CloseConnection();
        }
        else CloseConnection();
    }
    //else if (network_events.lNetworkEvents&FD_CONNECT) ;
    else if (network_events.lNetworkEvents&FD_CLOSE) CloseConnection();
#else
    size = recv(socketid, buffer, 1023, 0);
    if (size > 0)
    {
        buffer[size] = '\0';
        if (IsUtf8(buffer, size)) data.append(buffer);
        else data.append(utils::LocaleToUtf8(buffer));
        while (data.contains('\n'))
        {
            ParseLine(data.before('\n').before('\r'));
            data = data.after('\n');
        }
        receiveRest = data;
    }
    else if (size < 0)
    {
        SendEvent(IRC_ERROR, FXStringFormat(_("Error in reading data from %s"), serverName.text()));
        CloseConnection();
    }
    else CloseConnection();
#endif
    return size;
}

void IrcSocket::ParseLine(const FXString &line)
{
    FXString from, command, params;
    if (line[0] == ':')
    {
        from = GetParam(line, 1, false).after(':');
        command = GetParam(line, 2, false).upper();
        params = GetParam(line, 3, true);
    }
    else
    {
        from = "";
        command = GetParam(line, 1, false).upper();
        params = GetParam(line, 2, true);
    }

    if(command.length() == 3)
    {
        realServerName = from;
        Numeric(FXIntVal(command), params);
    }
    else if(command == "PRIVMSG") Privmsg(from, params);
    else if(command == "JOIN") Join(from, params);
    else if(command == "PART") Part(from, params);
    else if(command == "QUIT") Quitirc(from, params);
    else if(command == "PING") Ping(params);
    else if(command == "PONG") Pong(from, params);
    else if(command == "MODE") Mode(from, params);
    else if(command == "TOPIC") Topic(from, params);
    else if(command == "NOTICE") Notice(from, params);
    else if(command == "KICK") Kick(from, params);
    else if(command == "NICK") Nick(from, params);
    else if(command == "INVITE") Invite(from, params);
    else if(command == "ERROR") Error(params);
    else Unknown(command, params);
}

void IrcSocket::Numeric(const FXint &command, const FXString &params)
{
    switch(command) {
        case 219: //RPL_ENDOFSTATS
        {
            SendEvent(IRC_SERVERREPLY, _("End of STATS report"));
        }break;
        case 242: //RPL_STATSUPTIME
        {
            SendEvent(IRC_SERVERREPLY, FXStringFormat(_("Server Up %s days %s"), GetParam(params, 4, false).text(), GetParam(params, 6, true).text()));
        }break;
        case 301: //RPL_AWAY
        {
            SendEvent(IRC_301, GetParam(params, 2, false), GetParam(params, 3, true).after(':'));
        }break;
        case 305: //RPL_UNAWAY
        {
            SendEvent(IRC_SERVERREPLY, _("You are no longer marked as being away"));
        }break;
        case 306: //RPL_NOWAWAY
        {
            SendEvent(IRC_SERVERREPLY, _("You have been marked as being away"));
        }break;
        case 311: //RPL_WHOISUSER
        {
            SendEvent(IRC_SERVERREPLY, _("Start of WHOIS"));
            SendEvent(IRC_SERVERREPLY, GetParam(params, 2, false)+" ["+GetParam(params, 3, false)+"@"+GetParam(params, 4, false)+"]");
            AddNick(GetParam(params, 2, false), GetParam(params, 3, false), GetParam(params, 6, true).after(':'), GetParam(params, 4, false));
            SendEvent(IRC_SERVERREPLY, _("Realname: ")+GetParam(params, 6, true).after(':'));
        }break;
        case 312: //RPL_WHOISSERVER
        {
            SendEvent(IRC_SERVERREPLY, _("Server: ")+GetParam(params, 3, false)+" ["+GetParam(params, 4, true).after(':')+"]");
        }break;
        case 313: //RPL_WHOISOPERATOR
        {
            SendEvent(IRC_SERVERREPLY, FXStringFormat(_("%s is an IRC operator"), GetParam(params, 2, false).text()));
        }break;
        case 315: //RPL_ENDOFWHO
        {
            FXbool ignoreEvent = false;
            for(FXint i=0; i < ignoreCommands.no(); i++)
            {
                if((GetParam(ignoreCommands[i], 1, false) == "who") && (GetParam(ignoreCommands[i], 2, true).lower() == GetParam(params, 2, false).lower()))
                {
                    ignoreCommands.erase(i);
                    ignoreEvent = true;
                    SendEvent(IRC_AWAY, GetParam(params, 2, false));
                    break;
                }
            }
            if(!ignoreEvent) SendEvent(IRC_SERVERREPLY, _("End of WHO list"));
        }break;
        case 317: //RPL_WHOISIDLE
        {
            FXlong idle = FXLongVal(GetParam(params, 3, false));
            SendEvent(IRC_SERVERREPLY, _("Idle: ")+FXStringVal(idle/3600)+":"+FXStringVal((idle/60)%60)+":"+FXStringVal(idle%60));
            FXString datestr = GetParam(params, 4, false);
            FXlong time = FXLongVal(datestr);
            datestr = FXSystem::time(_("%x %X"), time);
            SendEvent(IRC_SERVERREPLY, _("Signon: ")+datestr);
        }break;
        case 318: //RPL_ENDOFWHOIS
        {
            SendEvent(IRC_SERVERREPLY, _("End of WHOIS"));
        }break;
        case 319: //RPL_WHOISCHANNELS
        {
            SendEvent(IRC_SERVERREPLY, _("Channels: ")+GetParam(params, 3, true).after(':'));
        }break;
        case 320: //RPL_IDENTIFIED
        {
            SendEvent(IRC_SERVERREPLY, FXStringFormat(_("%s : is identified to services"), GetParam(params, 2, false).text()));
        }break;
        case 324: //RPL_CHANNELMODEIS
        {
            SendEvent(IRC_CHMODE, GetParam(params, 2, false), GetParam(params, 3, false));
        }break;
        case 329: //RPL of channel mode (creation time)
        {
            //now nothing action
        }break;
        case 331: //RPL_NOTOPIC
        {
            SendEvent(IRC_331, GetParam(params, 2, false), _("No topic is set"));
        }break;
        case 332: //RPL_TOPIC
        {
            SendEvent(IRC_332, GetParam(params, 2, false), FXStringFormat(_("Topic for %s : %s"), GetParam(params, 2, false).text(), GetParam(params, 3, true).after(':').text()));
        }break;
        case 333: //RPL_TOPICSETBY
        {
            FXString datestr = GetParam(params, 4, false);
            FXlong time = FXLongVal(datestr);
            datestr = FXSystem::time(_("%x %X"), time);
            SendEvent(IRC_333, GetParam(params, 2, false), FXStringFormat(_("Set %s %s"), GetParam(params, 3, false).before('!').text(), datestr.text()));
        }break;
        case 341: //RPL_INVITING
        {
            SendEvent(IRC_SERVERREPLY, FXStringFormat(_("You invite %s to %s"), GetParam(params, 2, false).text(), GetParam(params, 3, false).text()));
        }break;
        case 342: //RPL_SUMMONING
        {
            SendEvent(IRC_SERVERREPLY, FXStringFormat(_("%s :Summoning user to IRC"), GetParam(params, 2, false).text()));
        }break;
        case 352: //RPL_WHOREPLY
        {
            FXbool ignoreEvent = false;
            for(FXint i=0; i < ignoreCommands.no(); i++)
            {
                if((GetParam(ignoreCommands[i], 1, false) == "who") && (GetParam(ignoreCommands[i], 2, true).lower() == GetParam(params, 2, false).lower()))
                {
                    //param:xxx #test ~dvx localhost dvx.irc.dvx dvx H :0 dvx
                    AddNick(GetParam(params, 6, false), GetParam(params, 3, false), GetParam(params, 9, true), GetParam(params, 4, false), GetParam(params, 7, false)[0] == 'H' ? false : true);
                    ignoreEvent = true;
                    break;
                }
            }
            if(!ignoreEvent) SendEvent(IRC_SERVERREPLY, GetParam(params, 2, true));
        }break;
        case 353: //RPL_NAMREPLY
        {
            SendEvent(IRC_353, GetParam(params, 3, false), GetParam(params, 4, true).after(':'));
        }break;
        case 366: //RPL_ENDOFNAMES
        {
            SendEvent(IRC_366, GetParam(params, 2, false));
        }break;
        case 367: //RPL_BANLIST
        {
            FXString datestr = GetParam(params, 5, true);
            if(datestr.empty()) SendEvent(IRC_SERVERREPLY, GetParam(params, 2, false)+" "+GetParam(params, 3, false));
            else
            {
                FXlong time = FXLongVal(datestr);
                datestr = FXSystem::time("%x %X", time);
                SendEvent(IRC_SERVERREPLY, GetParam(params, 2, false)+" "+GetParam(params, 3, false)+" "+GetParam(params, 4, false)+" "+datestr);
            }
        }break;
        case 369: //RPL_ENDOFWHOWAS
        {
            SendEvent(IRC_SERVERREPLY, _("End of WHOWAS"));
        }break;
        case 372: //RPL_MOTD
        {
            SendEvent(IRC_372, GetParam(params, 2, true).after(':'));
        }break;
        case 374: //RPL_ENDOFINFO
        {
            SendEvent(IRC_SERVERREPLY, _("End of INFO list"));
        }break;
        case 376: //RPL_ENDOFMOTD
        {
            SendEvent(IRC_SERVERREPLY, _("End of /MOTD command"));
            SendEvent(IRC_ENDMOTD);
            if(!startChannels.empty())
            {
                SendJoin(startChannels);
                startChannels.clear();
            }
        }break;
        case 381: //RPL_YOUREOPER
        {
            SendEvent(IRC_SERVERREPLY, _("You are now an IRC operator"));
        }break;
        case 401: //ERR_NOSUCHNICK
        {
            SendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :No such nick/channel"), GetParam(params, 2, false).text()));
        }break;
        case 402: //ERR_NOSUCHSERVER
        {
            SendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :No such server"), GetParam(params, 2, false).text()));
        }break;
        case 403: //ERR_NOSUCHCHANNEL
        {
            SendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :No such channel"), GetParam(params, 2, false).text()));
        }break;
        case 404: //ERR_CANNOTSENDTOCHAN
        {
            SendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :Cannot send to channel"), GetParam(params, 2, false).text()));
        }break;
        case 405: //ERR_TOOMANYCHANNELS
        {
            SendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :You have joined too many channels"), GetParam(params, 2, false).text()));
        }break;
        case 406: //ERR_WASNOSUCHNICK
        {
            SendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :There was no such nickname"), GetParam(params, 2, false).text()));
        }break;
        case 407: //ERR_TOOMANYTARGETS
        {
            SendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :Too many targets"), GetParam(params, 2, false).text()));
        }break;
        case 408: //ERR_NOSUCHSERVICE
        {
            SendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :No such service"), GetParam(params, 2, false).text()));
        }break;
        case 409: //ERR_NOORIGIN
        {
            SendEvent(IRC_SERVERERROR, _("No origin specified"));
        }break;
        case 411: //ERR_NORECIPIENT
        {
            SendEvent(IRC_SERVERERROR, _("No recipient given"));
        }break;
        case 412: //ERR_NOTEXTTOSEND
        {
            SendEvent(IRC_SERVERERROR, _("No text to send"));
        }break;
        case 413: //ERR_NOTOPLEVEL
        {
            SendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :No toplevel domain specified"), GetParam(params, 2, false).text()));
        }break;
        case 414: //ERR_WILDTOPLEVEL
        {
            SendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :Wildcard in toplevel domain"), GetParam(params, 2, false).text()));
        }break;
        case 415: //ERR_BADMASK
        {
            SendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :Bad Server/host mask"), GetParam(params, 2, false).text()));
        }break;
        case 421: //ERR_UNKNOWNCOMMAND
        {
            SendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :Unknown command"), GetParam(params, 2, false).text()));
        }break;
        case 422: //ERR_NOMOTD
        {
            SendEvent(IRC_SERVERERROR, _("MOTD File is missing"));
            SendEvent(IRC_ENDMOTD);
            if(!startChannels.empty())
            {
                SendJoin(startChannels);
                startChannels.clear();
            }
        }break;
        case 423: //ERR_NOADMININFO
        {
            SendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :No administrative info available"), GetParam(params, 2, false).text()));
        }break;
        case 424: //ERR_FILEERROR
        {
            SendEvent(IRC_SERVERERROR, _("File error doing"));
        }break;
        case 431: //ERR_NONICKNAMEGIVEN
        {
            SendEvent(IRC_SERVERERROR, _("No nickname given"));
        }break;
        case 432: //ERR_ERRONEUSNICKNAME
        {
            SendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :Erroneous nickname"), GetParam(params, 2, false).text()));
        }break;
        case 433: //ERR_NICKNAMEINUSE
        {
            SendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :Nickname is already in use"), GetParam(params, 2, false).text()));
            nickName += "_";
            SendNick(nickName);
        }break;
        case 436: //ERR_NICKCOLLISION
        {
            SendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :Nickname collision"), GetParam(params, 2, false).text()));
        }break;
        case 437: //ERR_UNAVAILRESOURCE
        {
            SendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :Nick/channel is temporarily unavailable"), GetParam(params, 2, false).text()));
        }break;
        case 441: //ERR_USERNOTINCHANNEL
        {
            SendEvent(IRC_SERVERERROR, FXStringFormat(_("%s %s :They aren't on that channel"), GetParam(params, 2, false).text(), GetParam(params, 3, false).text()));
        }break;
        case 442: //ERR_NOTONCHANNEL
        {
            SendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :You're not on that channel"), GetParam(params, 2, false).text()));
        }break;
        case 443: //ERR_USERONCHANNEL
        {
            SendEvent(IRC_SERVERERROR, FXStringFormat(_("%s %s :is already on channel"), GetParam(params, 2, false).text(), GetParam(params, 3, false).text()));
        }break;
        case 444: //ERR_NOLOGIN
        {
            SendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :User not logged in"), GetParam(params, 2, false).text()));
        }break;
        case 445: //ERR_SUMMONDISABLED
        {
            SendEvent(IRC_SERVERERROR, _("SUMMON has been disabled"));
        }break;
        case 446: //ERR_USERSDISABLED
        {
            SendEvent(IRC_SERVERERROR, _("USERS has been disabled"));
        }break;
        case 451: //ERR_NOTREGISTERED
        {
            SendEvent(IRC_SERVERERROR, _("You have not registered"));
        }break;
        case 461: //ERR_NEEDMOREPARAMS
        {
            SendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :Not enough parameters"), GetParam(params, 2, false).text()));
        }break;
        case 462: //ERR_ALREADYREGISTRED
        {
            SendEvent(IRC_SERVERERROR, _("Unauthorized command (already registered)"));
        }break;
        case 463: //ERR_NOPERMFORHOST
        {
            SendEvent(IRC_SERVERERROR, _("Your host isn't among the privileged"));
        }break;
        case 464: //ERR_PASSWDMISMATCH
        {
            SendEvent(IRC_SERVERERROR, _("Password incorrect"));
        }break;
        case 465: //ERR_YOUREBANNEDCREEP
        {
            SendEvent(IRC_SERVERERROR, _("You are banned from this server"));
        }break;
        case 466: //ERR_YOUWILLBEBANNED
        {
            SendEvent(IRC_SERVERERROR, _("You will be banned from this server"));
        }break;
        case 467: //ERR_KEYSET
        {
            SendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :Channel key already set"), GetParam(params, 2, false).text()));
        }break;
        case 471: //ERR_CHANNELISFULL
        {
            SendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :Cannot join channel (+l)"), GetParam(params, 2, false).text()));
        }break;
        case 472: //ERR_UNKNOWNMODE
        {
            SendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :is unknown mode char to me for channel"), GetParam(params, 2, false).text()));
        }break;
        case 473: //ERR_INVITEONLYCHAN
        {
            SendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :Cannot join channel (+i)"), GetParam(params, 2, false).text()));
        }break;
        case 474: //ERR_BANNEDFROMCHAN
        {
            SendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :Cannot join channel (+b)"), GetParam(params, 2, false).text()));
        }break;
        case 475: //ERR_BADCHANNELKEY
        {
            SendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :Cannot join channel (+k)"), GetParam(params, 2, false).text()));
        }break;
        case 476: //ERR_BADCHANMASK
        {
            if(GetParam(params, 3, true).contains("Bad Channel Mask")) SendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :Bad Channel Mask"), GetParam(params, 2, false).text()));
            else SendEvent(IRC_SERVERERROR, params);
        }break;
        case 477: //ERR_NOCHANMODES
        {
            if(GetParam(params, 3, true).contains("Channel doesn't support modes")) SendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :Channel doesn't support modes"), GetParam(params, 2, false).text()));
            else SendEvent(IRC_SERVERERROR, params);
        }break;
        case 478: //ERR_BANLISTFULL
        {
            if(GetParam(params, 3, true).contains("Channel list is full")) SendEvent(IRC_SERVERERROR, FXStringFormat(_("%s %s :Channel list is full"), GetParam(params, 2, false).text(), GetParam(params, 3, false).text()));
            else SendEvent(IRC_SERVERERROR, params);
        }break;
        case 481: //ERR_NOPRIVILEGES
        {
            SendEvent(IRC_SERVERERROR, _("Permission Denied- You're not an IRC operator"));
        }break;
        case 482: //ERR_CHANOPRIVSNEEDED
        {
            SendEvent(IRC_SERVERERROR, FXStringFormat(_("%s :You're not channel operator"), GetParam(params, 2, false).text()));
        }break;
        case 483: //ERR_CANTKILLSERVER
        {
            SendEvent(IRC_SERVERERROR, _("You can't kill a server!"));
        }break;
        case 484: //ERR_RESTRICTED
        {
            SendEvent(IRC_SERVERERROR, _("Your connection is restricted!"));
        }break;
        case 485: //ERR_UNIQOPPRIVSNEEDED
        {
            SendEvent(IRC_SERVERERROR, _("You're not the original channel operator"));
        }break;
        case 491: //ERR_NOOPERHOST
        {
            SendEvent(IRC_SERVERERROR, _("No O-lines for your host"));
        }break;
        case 501: //ERR_UMODEUNKNOWNFLAG
        {
            SendEvent(IRC_SERVERERROR, _("Unknown MODE flag"));
        }break;
        case 502: //ERR_USERSDONTMATCH
        {
            SendEvent(IRC_SERVERERROR, _("Cannot change mode for other users"));
        }break;
        default:
        {
            if(GetParam(params, 2, true)[0] == ':') SendEvent(IRC_SERVERREPLY, GetParam(params, 2, true).after(':'));
            else SendEvent(IRC_SERVERREPLY, GetParam(params, 2, true));
        }
    }
}

void IrcSocket::Privmsg(const FXString &from, const FXString &params)
{
    FXString nick = from.before('!');
    FXString to = GetParam(params, 1, false);
    FXString msg = GetParam(params, 2, true).after(':');
    if(msg[0] == '\001') Ctcp(from, params);
    else
    {
        if(nickName == to && !IsUserIgnored(nick, to)) SendEvent(IRC_QUERY, nick);
        if(!IsUserIgnored(nick, to)) SendEvent(IRC_PRIVMSG, nick, to, msg);
    }
}

void IrcSocket::Ctcp(const FXString &from, const FXString &params)
{
    FXString nick = from.before('!');
    FXString to = GetParam(params, 1, false);
    FXString msg = GetParam(params, 2, true).after(':').after('\001').before('\001');
    FXString ctcpCommand = msg.before(' ').upper();
    FXString ctcpRest = msg.after(' ');
    if(ctcpCommand == "VERSION")
    {
        SendEvent(IRC_CTCPREQUEST, nick, ctcpCommand);
        SendVersion(nick);
    }
    else if(ctcpCommand == "ACTION")
    {
        if(nickName == to && !IsUserIgnored(nick, to)) SendEvent(IRC_QUERY, nick);
        if(!IsUserIgnored(nick, to)) SendEvent(IRC_ACTION, nick, to, ctcpRest);
    }
    else if(ctcpCommand == "PING")
    {
        SendEvent(IRC_CTCPREQUEST, nick, ctcpCommand);
        SendCtcpNotice(nick, ctcpCommand+" "+ctcpRest);
    }
    else if(ctcpCommand == "DCC")
    {
        SendEvent(IRC_CTCPREQUEST, nick, _("DCC (not supported. Contact developer if you need it)"));
    }
    else if(ctcpCommand == "USERINFO")
    {
        SendEvent(IRC_CTCPREQUEST, nick, ctcpCommand);
        SendCtcpNotice(nick, ctcpCommand+" "+realName);
    }
    else if(ctcpCommand == "CLIENTINFO")
    {
        SendEvent(IRC_CTCPREQUEST, nick, ctcpCommand);
        SendCtcpNotice(nick, "CLIENTINFO ACTION CLIENTINFO PING TIME USERINFO VERSION");
    }
    else if(ctcpCommand == "TIME")
    {
        SendEvent(IRC_CTCPREQUEST, nick, ctcpCommand);
        SendCtcpNotice(nick, ctcpCommand+" "+FXSystem::time("%x %X", FXSystem::now()));
    }
}

void IrcSocket::Join(const FXString &from, const FXString &params)
{
    FXString channel = GetParam(params, 1, false);
    if (channel[0] == ':') channel = channel.after(':');
    FXString nick = from.before('!');
    if(nickName == nick) SendEvent(IRC_NEWCHANNEL, (channel[0] == '&' ? "&"+channel : channel));
    else SendEvent(IRC_JOIN, nick, channel);
    AddNick(nick, from.after('!').before('@'), "", from.after('@'), false);
}

void IrcSocket::Quitirc(const FXString &from, const FXString &params)
{
    FXString reason = GetParam(params, 1, true).after(':');
    FXString nick = from.before('!');
    if (reason.empty())
    {
        SendEvent(IRC_QUIT, nick);
    }
    else
    {
        SendEvent(IRC_QUIT, nick, reason);
    }
    RemoveNick(nick);
}

void IrcSocket::Part(const FXString &from, const FXString &params)
{
    FXString channel = GetParam(params, 1, false);
    FXString reason = GetParam(params, 1, true).after(':');
    FXString nick = from.before('!');
    if (reason.empty())
    {
        SendEvent(IRC_PART, nick, channel);
    }
    else
    {
        SendEvent(IRC_PART, nick, channel, reason);
    }
}

void IrcSocket::Ping(const FXString &params)
{
    SendLine("PONG "+params);
}

void IrcSocket::Pong(const FXString &from, const FXString &params)
{
    SendEvent(IRC_SERVERREPLY, _("PONG from ")+from+" "+params.after(':'));
}

void IrcSocket::Notice(const FXString &from, const FXString &params)
{
    FXString to = GetParam(params, 1, false);
    FXString nick = from.before('!');
    FXString msg = GetParam(params, 2, true).after(':');
    if(msg[0] == '\001')
    {
        SendEvent(IRC_CTCPREPLY, nick, msg.after('\001').before('\001'));
    }
    else
    {
        if(from.empty()) //public notice
        {
            SendEvent(IRC_NOTICE, to, msg);
        }
        else
        {
            if(!IsUserIgnored(nick, to)) SendEvent(IRC_CHNOTICE, nick, to, msg);
        }
    }
}

void IrcSocket::Nick(const FXString &from, const FXString &params)
{
    FXString nick = from.before('!');
    FXString newnick = GetParam(params, 1, false).after(':');
    if(nickName == nick) nickName = newnick;
    SendEvent(IRC_NICK, nick, newnick);
    for(FXint i = 0; i < nicks.no(); i++)
    {
        if(nick == nicks[i].nick)
        {
            nicks[i].nick = newnick;
            break;
        }
    }
}

void IrcSocket::Topic(const FXString &from, const FXString &params)
{
    FXString nick = from.before('!');
    FXString channel = GetParam(params, 1, false);
    FXString topic = GetParam(params, 2, true).after(':');
    SendEvent(IRC_TOPIC, nick, channel, topic);
}

void IrcSocket::Invite(const FXString &from, const FXString &params)
{
    FXString nick = from.before('!');
    FXString to = GetParam(params, 1, false);
    FXString channel = GetParam(params, 2, false);
    if (channel[0] == ':') channel = channel.after(':');
    SendEvent(IRC_INVITE, nick, to, channel);
}

void IrcSocket::Kick(const FXString &from, const FXString &params)
{
    FXString nick = from.before('!');
    FXString to = GetParam(params, 2, false);
    FXString channel = GetParam(params, 1, false);
    FXString reason = GetParam(params, 3, true).after(':');
    SendEvent(IRC_KICK, nick, to, channel, reason);
}

void IrcSocket::Mode(const FXString &from, const FXString &params)
{
    if(params.contains(':')) SendEvent(IRC_MODE, GetParam(params, 2, true).after(':'), GetParam(params, 1, false));
    else //channel mode
    {
        FXString moderator = from.before('!');
        FXString channel = GetParam(params, 1, false);
        FXString modes = GetParam(params, 2, false);
        FXString args = GetParam(params, 3, true);
        SendEvent(IRC_UMODE, moderator, channel, modes, args);
    }
}

void IrcSocket::Error(const FXString &params)
{
    SendEvent(IRC_SERVERERROR, params[0] == ':' ? params.after(':') : params);
}

void IrcSocket::Unknown(const FXString &command, const FXString &params)
{
    SendEvent(IRC_UNKNOWN, command, params);
}

void IrcSocket::AddIgnoreCommands(const FXString& command)
{
    ignoreCommands.append(command);
}

void IrcSocket::RemoveIgnoreCommands(const FXString& command)
{
    for(FXint i=0; i < ignoreCommands.no(); i++)
    {
        if(ignoreCommands[i] == command)
        {
            ignoreCommands.erase(i);
            break;
        }
    }
}

FXbool IrcSocket::SendAdmin(const FXString& params)
{
    return SendLine("ADMIN "+params);
}

FXbool IrcSocket::SendAway(const FXString& params)
{
    return SendLine("AWAY :"+params);
}

FXbool IrcSocket::SendBanlist(const FXString& channel)
{
    return SendLine("MODE "+channel+" +b");
}

FXbool IrcSocket::SendCtcp(const FXString& to, const FXString& params)
{
    return SendLine("PRIVMSG "+to+" :\001"+params+"\001");
}

FXbool IrcSocket::SendCtcpNotice(const FXString& to, const FXString& params)
{
    return SendLine("NOTICE "+to+" :\001"+params+"\001");
}

FXbool IrcSocket::SendMode(const FXString& params)
{
    return SendLine("MODE "+params);
}

FXbool IrcSocket::SendInvite(const FXString& to, const FXString& params)
{
    return SendLine("INVITE "+to+" "+params);
}

FXbool IrcSocket::SendJoin(const FXString& chan)
{
    return SendLine("JOIN "+chan);
}

FXbool IrcSocket::SendKick(const FXString& chan, const FXString& nick, const FXString& reason)
{
    return SendLine("KICK "+chan+" "+nick+" :"+reason);
}

FXbool IrcSocket::SendKill(const FXString& nick, const FXString& reason)
{
    return SendLine("KILL "+nick+" :"+reason);
}

FXbool IrcSocket::SendList(const FXString& params)
{
    return SendLine("LIST "+params);
}

FXbool IrcSocket::SendMe(const FXString& to, const FXString& message)
{
    return SendLine("PRIVMSG "+to+" :\001ACTION "+message+"\001");
}

FXbool IrcSocket::SendMsg(const FXString& to, const FXString& message)
{
    return SendLine("PRIVMSG "+to+" :"+message);
}

FXbool IrcSocket::SendNames(const FXString& channel)
{
    return SendLine("NAMES "+channel);
}

FXbool IrcSocket::SendNick(const FXString& nick)
{
    return SendLine("NICK "+nick);
}

FXbool IrcSocket::SendNotice(const FXString& to, const FXString& message)
{
    return SendLine("NOTICE "+to+" :"+message);
}

FXbool IrcSocket::SendOper(const FXString& login, const FXString& password)
{
    return SendLine("OPER "+login+" "+password);
}

FXbool IrcSocket::SendPart(const FXString& chan)
{
    return SendLine("PART "+chan);
}

FXbool IrcSocket::SendPart(const FXString& chan, const FXString& reason)
{
    return SendLine("PART "+chan+" :"+reason);
}

FXbool IrcSocket::SendRaw(const FXString& text)
{
    return SendLine(text);
}

FXbool IrcSocket::SendTopic(const FXString& chan, const FXString& topic)
{
    if (topic.empty())
        return SendLine("TOPIC "+chan);
    else
        return SendLine("TOPIC "+chan+" :"+topic);
}

FXbool IrcSocket::SendTopic(const FXString& chan)
{
    return SendLine("TOPIC "+chan);
}

FXbool IrcSocket::SendWallops(const FXString& msg)
{
    return SendLine("WALLOPS :"+msg);
}

FXbool IrcSocket::SendVersion(const FXString& to)
{
    return SendLine("NOTICE "+to+FXStringFormat(" :\001VERSION dxirc %s (C) 2008~ by David Vachulka\001", VERSION));
}

FXbool IrcSocket::SendWho(const FXString& mask)
{
    return SendLine("WHO "+mask);
}

FXbool IrcSocket::SendWhois(const FXString& params)
{
    return SendLine("WHOIS "+params);
}

FXbool IrcSocket::SendWhowas(const FXString& params)
{
    return SendLine("WHOWAS "+params);
}

FXbool IrcSocket::SendLine(const FXString& line)
{
    FXString toSend = line + "\r\n";
    int size;
    if (connected)
    {
        if ((size = send(socketid, toSend.text(), toSend.length(), 0)) == -1)
        {
            SendEvent(IRC_ERROR, _("Unable send data"));
#ifdef WIN32
            WSACleanup();
#endif
            CloseConnection();
            return false;
        }
        return true;
    }
    else return false;
}

void IrcSocket::SendEvent(IrcEventType eventType)
{
    IrcEvent ev;
    ev.eventType = eventType;
    ev.param1 = "";
    ev.param2 = "";
    ev.param3 = "";
    ev.param4 = "";
    ev.ircSocket = this;
    for (FXint i=0; i < targets.no(); i++)
    {
        targets.at(i)->handle(this, FXSEL(SEL_COMMAND, ID_SERVER), &ev);
    }
}

void IrcSocket::SendEvent(IrcEventType eventType, const FXString &param1)
{
    IrcEvent ev;
    ev.eventType = eventType;
    ev.param1 = param1;
    ev.param2 = "";
    ev.param3 = "";
    ev.param4 = "";
    ev.ircSocket = this;
    for (FXint i=0; i < targets.no(); i++)
    {
        targets.at(i)->handle(this, FXSEL(SEL_COMMAND, ID_SERVER), &ev);
    }
}

void IrcSocket::SendEvent(IrcEventType eventType, const FXString &param1, const FXString &param2)
{
    IrcEvent ev;
    ev.eventType = eventType;
    ev.param1 = param1;
    ev.param2 = param2;
    ev.param3 = "";
    ev.param4 = "";
    ev.ircSocket = this;
    for (FXint i=0; i < targets.no(); i++)
    {
        targets.at(i)->handle(this, FXSEL(SEL_COMMAND, ID_SERVER), &ev);
    }
}

void IrcSocket::SendEvent(IrcEventType eventType, const FXString &param1, const FXString &param2, const FXString &param3)
{
    IrcEvent ev;
    ev.eventType = eventType;
    ev.param1 = param1;
    ev.param2 = param2;
    ev.param3 = param3;
    ev.param4 = "";
    ev.ircSocket = this;
    for (FXint i=0; i < targets.no(); i++)
    {
        targets.at(i)->handle(this, FXSEL(SEL_COMMAND, ID_SERVER), &ev);
    }
}

void IrcSocket::SendEvent(IrcEventType eventType, const FXString &param1, const FXString &param2, const FXString &param3, const FXString &param4)
{
    IrcEvent ev;
    ev.eventType = eventType;
    ev.param1 = param1;
    ev.param2 = param2;
    ev.param3 = param3;
    ev.param4 = param4;
    ev.ircSocket = this;
    for (FXint i=0; i < targets.no(); i++)
    {
        targets.at(i)->handle(this, FXSEL(SEL_COMMAND, ID_SERVER), &ev);
    }
}

void IrcSocket::FillCommands()
{
    commands.append("ADMIN");
    commands.append("AWAY");
    commands.append("BANLIST");
    commands.append("COMMANDS");
    commands.append("CTCP");
    commands.append("DEOP");
    commands.append("DEVOICE");
    commands.append("INVITE");
    commands.append("JOIN");
    commands.append("KICK");
    commands.append("KILL");
    commands.append("LIST");
    commands.append("ME");
    commands.append("MSG");
    commands.append("NAMES");
    commands.append("NICK");
    commands.append("NOTICE");
    commands.append("OP");
    commands.append("OPER");
    commands.append("PART");
    commands.append("QUIT");
    commands.append("RAW");
    commands.append("TOPIC");
    commands.append("VOICE");
    commands.append("WALLOPS");
    commands.append("WHO");
    commands.append("WHOIS");
    commands.append("WHOWAS");
}

//This's from Xfe, thanks
FXbool IrcSocket::IsUtf8(const FXchar* string, FXuint length)
{
    FXchar s[4];
    const FXchar BOM[] = { 0xEF, 0xBB, 0xBF, '\0' };

    // Keep only length left bytes
    FXString str=string;
    str=str.left(length);

    // Convert forth and back to UTF8
    FXUTF8Codec utf8;
    FXString utf8str=utf8.mb2utf(utf8.utf2mb(str));

    // Strings are equal => UTF8
    if (str==utf8str)
        return TRUE;

    // Strings not equal => test if BOM is present
    else
    {
        // String too small to contain BOM
        if (length<=2)
            return FALSE;

        // Test if string contains BOM
        else
        {
            s[0]=string[0];
            s[1]=string[1];
            s[2]=string[2];
            s[3]='\0';

            // String contains BOM => UTF8
            if (Streq(s,BOM))
                return TRUE;

            // String don't contain BOM
            else
                return FALSE;
        }
    }
}

//This's from Xfe, thanks
int IrcSocket::Streq(const FXchar *a, const FXchar *b)
{
    if (a == NULL || b == NULL)
        return 0;
    return (strcmp(a, b) == 0);
}

FXString IrcSocket::GetParam(FXString toParse, FXint n, FXbool toEnd)
{
    if (toEnd)
    {
        return toParse.after(' ', n-1);
    }
    else
    {
        return toParse.before(' ', n).rafter(' ');
    }
}

FXString IrcSocket::GetParam(FXString toParse, FXint n, FXbool toEnd, const FXchar &separator)
{
    if (toEnd) {
        return toParse.after(separator, n-1);
    }
    else {
        return toParse.before(separator, n).rafter(separator);
    }
}

void IrcSocket::AppendTarget(FXObject *tgt)
{
     targets.append(tgt);
}

void IrcSocket::RemoveTarget(FXObject *tgt)
{
    for(FXint i=0; i < targets.no(); i++)
    {
        if(targets[i] == tgt)
        {
            targets.erase(i);
            break;
        }
    }
}

FXbool IrcSocket::FindTarget(FXObject *tgt)
{
    for(FXint i=0; i < targets.no(); i++)
    {
        if(targets[i] == tgt)
        {
            return true;
        }
    }
    return false;
}

FXbool IrcSocket::ClearTarget()
{
    targets.clear();
    if(targets.no()) return false;
    else return true;
}

void IrcSocket::AddNick(const FXString &nick, const FXString &user, const FXString &real, const FXString &host, const FXbool &away)
{
    FXbool nickExist = false;
    for(FXint i=0; i < nicks.no(); i++)
    {
        if(nicks[i].nick == nick)
        {
            nicks[i].user = user;
            nicks[i].real = real;
            nicks[i].host = host;
            nicks[i].away = away;
            nickExist = true;
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
        nicks.append(nickStruct);
    }
}

void IrcSocket::AddNick(const FXString &nick, const FXString &user, const FXString &real, const FXString &host)
{
    FXbool nickExist = false;
    for(FXint i=0; i < nicks.no(); i++)
    {
        if(nicks[i].nick == nick)
        {
            nicks[i].user = user;
            nicks[i].real = real;
            nicks[i].host = host;
            nickExist = true;
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
        nickStruct.away = false;
        nicks.append(nickStruct);
    }
}

void IrcSocket::RemoveNick(const FXString &nick)
{
    for(FXint i=0; i < nicks.no(); i++)
    {
        if(nicks[i].nick == nick)
        {
            nicks.erase(i);
            break;
        }
    }
}

NickInfo IrcSocket::GetNickInfo(const FXString &nick)
{
    NickInfo nickInfo;
    for(FXint i=0; i < nicks.no(); i++)
    {
        if(nicks[i].nick == nick)
        {
            nickInfo = nicks[i];
            break;
        }
    }
    return nickInfo;
}

FXString IrcSocket::GetBannedNick(const FXString &banmask)
{
    FXString nick = "";
    FXString banNick = banmask.before('!');
    FXString banUser = banmask.after('!').before('@');
    FXString banHost = banmask.after('@');
    for(FXint i=0; i<nicks.no(); i++)
    {
        if(FXRex(FXString("\\<"+banNick+"\\>").substitute("*","\\w*")).match(nicks[i].nick) && FXRex(FXString(banUser+"\\>").substitute("*","\\w*")).match(nicks[i].user) && FXRex(FXString("\\<"+banHost+"\\>").substitute("*","\\w*")).match(nicks[i].host)) nick += nicks[i].nick+";";
    }
    return nick;
}

const char* IrcSocket::GetLocalIP()
{
    struct sockaddr_in local;
    socklen_t len = sizeof(struct sockaddr_in);
    getsockname(socketid, reinterpret_cast<struct sockaddr *>(&local), &len);
    return inet_ntoa(local.sin_addr);
}

FXbool IrcSocket::IsUserIgnored(const FXString &nick, const FXString &on)
{
    FXbool user = false;
    FXbool channel = false;
    FXbool host = false;
    for(FXint i=0; i<usersList.no(); i++)
    {
        if(FXRex(FXString("\\<"+usersList[i].nick+"\\>").substitute("*","\\w*")).match(nick)) user = true;
        if(usersList[i].channel == "all") channel = true;
        if(usersList[i].channel.contains(','))
        {
            for(FXint j=1; j<usersList[i].channel.contains(',')+2; j++)
            {
                if(FXRex(FXString(GetParam(usersList[i].channel, j, false, ',')+"\\>").substitute("*","\\w*")).match(on))
                {
                    channel = true;
                    break;
                }
            }
        }
        else
        {
            if(FXRex(FXString(usersList[i].channel+"\\>").substitute("*","\\w*")).match(on)) channel = true;
        }
        if(usersList[i].server == "all") host = true;
        if(FXRex(FXString("\\<"+usersList[i].server+"\\>").substitute("*","\\w*")).match(serverName)) host = true;
    }
    return user && channel && host;
}
