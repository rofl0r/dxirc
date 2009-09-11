/*
 *      ircsocket.h
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


#ifndef IRCSOCKET_H
#define IRCSOCKET_H

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#define socklen_t int
#else
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include "defs.h"
#include "config.h"
#include "utils.h"

#ifdef HAVE_OPENSSL
#include <openssl/ssl.h>
#endif

class ConnectThread;

class IrcSocket: public FXObject
{
    FXDECLARE(IrcSocket)
    public:
        IrcSocket(FXApp*, FXObject*, FXString, FXString);
        virtual ~IrcSocket();
        enum {
            ID_READ,
            ID_SERVER,
            ID_SSLTIME,
            ID_RTIME,
            ID_LAST
        };

        void StartConnection();
        FXint Connect();
        FXint ConnectSSL();
        void Disconnect();
        void Disconnect(const FXString &reason);
        void AppendTarget(FXObject*);
        void RemoveTarget(FXObject*);
        FXbool FindTarget(FXObject*);
        FXbool ClearTarget();
        void ClearAttempts() { attempts = 0; }
        void SetServerName(const FXString &name) { serverName = name; }
        FXString GetServerName() { return serverName; }
        FXString GetRealServerName() { return realServerName; }
        void SetServerPort(FXint port) { serverPort = port; }
        FXint GetServerPort() { return serverPort; }
        void SetServerPassword(const FXString &pass) { serverPassword = pass; }
        void SetNickName(const FXString &nick) { nickName = nick; }
        FXString GetNickName() { return nickName; }
        void SetUserName(const FXString &user) { userName = user; }
        void SetRealName(const FXString &rname) { realName = rname; }
        void SetStartChannels(const FXString &channels) { startChannels = utils::RemoveSpaces(channels); }
        void SetStartCommands(const FXString &commands) { startCommands = commands; }
        void SetUsersList(const dxIgnoreUserArray &ulst) { usersList = ulst;}
        void SetUseSsl(const FXbool &ussl) { useSsl = ussl;}
        void SetReconnect(const FXbool &rcn) { reconnect = rcn; }
        void SetNumberAttempt(const FXint &na) { numberAttempt = na; }
        void SetDelayAttempt(const FXint &da) { delayAttempt = da; }
        FXbool GetConnected() { return connected; }
        FXbool GetConnecting() { return connecting; }
        FXbool GetUseSsl() { return useSsl; }
        FXint GetNickLen() { return nickLen; }
        FXint GetTopicLen() { return topicLen; }
        FXint GetKickLen() { return kickLen; }
        FXint GetAwayLen() { return awayLen; }
        void AddIgnoreCommands(const FXString &command);
        void RemoveIgnoreCommands(const FXString &command);
        void AddNick(const FXString &nick, const FXString &user, const FXString &real, const FXString &host, const FXbool &away);
        void AddNick(const FXString &nick, const FXString &user, const FXString &real, const FXString &host);
        void RemoveNick(const FXString &nick);
        NickInfo GetNickInfo(const FXString &nick);
        FXString GetBannedNick(const FXString &banmask);
        const char* GetLocalIP();
        const char* GetRemoteIP();
        FXbool SendAdmin(const FXString &params);
        FXbool SendAway(const FXString &params);
        FXbool SendBanlist(const FXString &channel);
        FXbool SendCtcp(const FXString &to, const FXString &params);
        FXbool SendCtcpNotice(const FXString &to, const FXString &params);
        FXbool SendMode(const FXString &params);
        FXbool SendInvite(const FXString &to, const FXString &params);
        FXbool SendJoin(const FXString &chan);
        FXbool SendKick(const FXString &chan, const FXString &nick, const FXString &reason);
        FXbool SendKill(const FXString &nick, const FXString &reason);
        FXbool SendList(const FXString &params);
        FXbool SendMe(const FXString &to, const FXString &message);
        FXbool SendMsg(const FXString &to, const FXString &message);
        FXbool SendNames(const FXString &channel);
        FXbool SendNick(const FXString &nick);
        FXbool SendNotice(const FXString &to, const FXString &message);
        FXbool SendOper(const FXString &login, const FXString &password);
        FXbool SendPart(const FXString &chan);
        FXbool SendPart(const FXString &chan, const FXString &reason);
        FXbool SendQuote(const FXString &text);
        FXbool SendTopic(const FXString &chan, const FXString &topic);
        FXbool SendTopic(const FXString &chan);
        FXbool SendVersion(const FXString &to);
        FXbool SendWallops(const FXString &msg);
        FXbool SendWho(const FXString &mask);
        FXbool SendWhoami();
        FXbool SendWhois(const FXString &params);
        FXbool SendWhowas(const FXString &params);

        long OnIORead(FXObject*, FXSelector, void*);
        long OnReconnectTimeout(FXObject*, FXSelector, void*);

    private:
        IrcSocket(){}

        FXApp *application;
        FXbool connected, useSsl, connecting, reconnect, endmotd;
        FXint serverPort, numberAttempt, delayAttempt, attempts;
        FXint nickLen, topicLen, kickLen, awayLen;
        FXString serverName, realServerName, serverPassword, nickName, realName, userName, startChannels, startCommands;
        FXString receiveRest;
        dxTargetsArray targets;
        dxStringArray ignoreCommands;
        dxIgnoreUserArray usersList;
        dxNicksArray nicks;
        #ifdef WIN32            
            WSAEVENT event;
        #endif
            int socketid;        
        sockaddr_in serverSock;
        ConnectThread *thread;

        int ReadData();
        FXbool SendLine(const FXString &line);
        void SendCommands();
        FXbool SendCommand(const FXString &commandtext);
        void ParseLine(const FXString &line);
        void SendEvent(IrcEventType);
        void SendEvent(IrcEventType, const FXString&);
        void SendEvent(IrcEventType, const FXString&, const FXString&);
        void SendEvent(IrcEventType, const FXString&, const FXString&, const FXString&);
        void SendEvent(IrcEventType, const FXString&, const FXString&, const FXString&, const FXString&);        
        void Numeric(const FXint&, const FXString&);
        void Privmsg(const FXString&, const FXString&);
        void Ctcp(const FXString&, const FXString&);
        void Join(const FXString&, const FXString&);
        void Quitirc(const FXString&, const FXString&);
        void Part(const FXString&, const FXString&);
        void Ping(const FXString&);
        void Pong(const FXString&, const FXString&);
        void Notice(const FXString&, const FXString&);
        void Nick(const FXString&, const FXString&);
        void Topic(const FXString&, const FXString&);
        void Invite(const FXString&, const FXString&);
        void Kick(const FXString&, const FXString&);
        void Mode(const FXString&, const FXString&);
        void Error(const FXString&);
        void Unknown(const FXString&, const FXString&);
        void ParseRplIsupport(FXString);
        FXbool IsUserIgnored(const FXString &nick, const FXString &on);
        void CloseConnection(FXbool disableReconnect=FALSE);
        void MakeStartChannels();
        void ClearChannelsCommands();

#ifdef HAVE_OPENSSL
        SSL_CTX *ctx;
        SSL *ssl;
        FXint err;
#endif

};

class ConnectThread : public FXThread
{
    public:
        ConnectThread(IrcSocket*);
        virtual ~ConnectThread();
    protected:
        FXint run();
    private:
        IrcSocket *socket;

};

#endif // IRCSOCKET_H
