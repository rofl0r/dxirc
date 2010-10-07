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
#define WIN32_LEAN_AND_MEAN
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

#include <fstream>
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
            ID_SOCKET = FXMainWindow::ID_LAST+100,
            ID_SERVER,
            ID_SSLTIME, //ssl on windows
            ID_RTIME, //reconnect
            ID_PTIME, //dccfile position
            ID_CTIME, //timeout for check and disconnect offered connection
            ID_ETIME, //event timeout
            ID_LAST
        };

        void startConnection();
        void startListening(const FXString &nick, IrcSocket *server);
        FXint connectIRC();
        FXint connectSSL();
        FXint listenIRC();
        void disconnect();
        void disconnect(const FXString &reason);
        void closeConnection(FXbool disableReconnect=FALSE);
        void closeDccfileConnection(DccFile file);
        void appendTarget(FXObject*);
        void removeTarget(FXObject*);
        FXbool findTarget(FXObject*);
        FXbool clearTarget();
        void clearAttempts() { m_attempts = 0; }
        void setServerName(const FXString &name) { m_serverName = name; }
        FXString getServerName() const { return m_serverName; }
        FXString getRealServerName() const { return m_realServerName; }
        void setServerPort(FXint port) { m_serverPort = port; }
        FXint getServerPort() const { return m_serverPort; }
        void setServerPassword(const FXString &pass) { m_serverPassword = pass; }
        void setNickName(const FXString &nick) { m_nickName = nick; }
        FXString getNickName() const { return m_nickName; }
        void setUserName(const FXString &user);
        void setRealName(const FXString &realname) { m_realName = realname; }
        void setStartChannels(const FXString &channels) { m_startChannels = utils::instance().removeSpaces(channels); }
        void setStartCommands(const FXString &commands) { m_startCommands = commands; }
        void setUsersList(const dxIgnoreUserArray &userslist) { m_usersList = userslist; }
        dxIgnoreUserArray getUsersList() { return m_usersList; }
        void setUseSsl(const FXbool &usessl) { m_useSsl = usessl;}
        void setReconnect(const FXbool &reconnect) { m_reconnect = reconnect; }
        void setNumberAttempt(const FXint &numberattempt) { m_numberAttempt = numberattempt; }
        void setDelayAttempt(const FXint &delayattemtp) { m_delayAttempt = delayattemtp; }
        void setDccType(const DCCTYPE &dcctype) { m_dccType = dcctype; }
        DCCTYPE getDccType() const { return m_dccType; }
        void setDccFile(DccFile file) { m_dccFile = file; }
        DccFile getDccFile() const { return m_dccFile; }
        FXbool hasDccFile(DccFile file) { return m_dccFile == file; }
        FXbool getConnected() const { return m_connected; }
        FXbool getConnecting() const { return m_connecting; }
        FXbool getUseSsl() const { return m_useSsl; }
        FXint getNickLen() const { return m_nickLen; }
        FXint getTopicLen() const { return m_topicLen; }
        FXint getKickLen() const { return m_kickLen; }
        FXint getAwayLen() const { return m_awayLen; }
        FXchar getAdminPrefix() const { return m_adminPrefix; }
        FXchar getOwnerPrefix() const { return m_ownerPrefix; }
        FXchar getOpPrefix() const { return m_opPrefix; }
        FXchar getVoicePrefix() const { return m_voicePrefix; }
        FXchar getHalfopPrefix() const { return m_halfopPrefix; }
        FXString getChanTypes() const { return m_chanTypes; }
        void addIgnoreCommands(const FXString &command);
        void removeIgnoreCommands(const FXString &command);
        void addNick(const FXString &nick, const FXString &user, const FXString &real, const FXString &host, const FXbool &away);
        void addNick(const FXString &nick, const FXString &user, const FXString &real, const FXString &host);
        void removeNick(const FXString &nick);
        NickInfo getNickInfo(const FXString &nick) const;
        FXString getBannedNick(const FXString &banmask) const;
        const char* getLocalIP();
        FXuint getLocalIPBinary();
        const char* getRemoteIP();
        FXString getDccIP() const  { return m_dccIP; } //IP address possible usable as routable in DCC
        FXbool sendAdmin(const FXString &params);
        FXbool sendAway(const FXString &params);
        FXbool sendBanlist(const FXString &channel);
        FXbool sendCtcp(const FXString &to, const FXString &params);
        FXbool sendCtcpNotice(const FXString &to, const FXString &params);
        FXbool sendDccChatText(const FXString &message);
        FXbool sendMode(const FXString &params);
        FXbool sendInvite(const FXString &to, const FXString &params);
        FXbool sendJoin(const FXString &chan);
        FXbool sendKick(const FXString &chan, const FXString &nick, const FXString &reason);
        FXbool sendKill(const FXString &nick, const FXString &reason);
        FXbool sendList(const FXString &params);
        FXbool sendMe(const FXString &to, const FXString &message);
        FXbool sendMsg(const FXString &to, const FXString &message);
        FXbool sendNames(const FXString &channel);
        FXbool sendNick(const FXString &nick);
        FXbool sendNotice(const FXString &to, const FXString &message);
        FXbool sendOper(const FXString &login, const FXString &password);
        FXbool sendPart(const FXString &chan);
        FXbool sendPart(const FXString &chan, const FXString &reason);
        FXbool sendQuote(const FXString &text);
        FXbool sendStats(const FXString &type);
        FXbool sendTopic(const FXString &chan, const FXString &topic);
        FXbool sendTopic(const FXString &chan);
        FXbool sendVersion(const FXString &to);
        FXbool sendWallops(const FXString &msg);
        FXbool sendWho(const FXString &mask);
        FXbool sendWhoami();
        FXbool sendWhois(const FXString &params);
        FXbool sendWhowas(const FXString &params);
        FXbool isUserIgnored(const FXString &nick, const FXString &user, const FXString &host, const FXString &on);
        FXbool isUserIgnored(const FXString &nick, const FXString &on);
        FXbool isAway(const FXString &nick);
        void setCurrentPostion(FXulong position);
        FXbool isForResume(const FXString &nick, const FXString &name, FXint port);
        FXbool isForResume(FXint token);

        long onIORead(FXObject*, FXSelector, void*);
        long onIOWrite(FXObject*, FXSelector, void*);
        long onReconnectTimeout(FXObject*, FXSelector, void*);
        long onPositionTimeout(FXObject*, FXSelector, void*);
        long onCloseTimeout(FXObject*, FXSelector, void*);
        long onEventTimeout(FXObject*, FXSelector, void*);

    private:
        IrcSocket(){}

        FXApp *m_application;
        FXbool m_connected, m_useSsl, m_connecting, m_reconnect, m_endmotd;
        FXbool m_ignoreUserHost;
        FXint m_serverPort, m_numberAttempt, m_delayAttempt, m_attempts;
        FXint m_nickLen, m_topicLen, m_kickLen, m_awayLen, m_dccPortD, m_dccPortH, m_dccTimeout;
        FXString m_serverName, m_realServerName, m_serverPassword, m_nickName, m_realName, m_userName, m_startChannels, m_startCommands;
        FXString m_receiveRest;
        FXString m_dccNick; //nick invited on dccchat
        FXString m_dccIP; //for DCC from USERHOST or INI
        FXString m_chanTypes; //channel prefixes
        DccFile m_dccFile; //file received or sended
        FXchar m_adminPrefix, m_ownerPrefix, m_opPrefix, m_voicePrefix, m_halfopPrefix; //prefix for nick modes
        dxTargetsArray m_targets;
        dxStringArray m_ignoreCommands;
        dxIgnoreUserArray m_usersList;
        dxNicksArray m_nicks;
        dxIrcEventArray m_events;
        #ifdef WIN32            
            WSAEVENT m_event;
        #endif
        int m_serverid, m_clientid;
        sockaddr_in m_serverSock, m_clientSock;
        ConnectThread *m_thread;
        DCCTYPE m_dccType;
        IrcSocket *m_dccParent; //DCC chat sending server
        std::ofstream m_receivedFile;
        std::ifstream m_sentFile;

        void readData();
        void readFileData();
        FXbool sendLine(const FXString &line);
        void sendFile();
        void sendCommands();
        FXbool sendCommand(const FXString &commandtext);
        void sendEvents();
        void parseLine(const FXString &line);
        void sendEvent(IrcEventType);
        void sendEvent(IrcEventType, const FXString&);
        void sendEvent(IrcEventType, const FXString&, const FXString&);
        void sendEvent(IrcEventType, const FXString&, const FXString&, const FXString&);
        void sendEvent(IrcEventType, const FXString&, const FXString&, const FXString&, const FXString&);
        void sendEvent(IrcEventType, DccFile);
        void numeric(const FXint&, const FXString&);
        void privmsg(const FXString&, const FXString&);
        void ctcp(const FXString&, const FXString&);
        void dccMsg(const FXString&);
        void join(const FXString&, const FXString&);
        void quitirc(const FXString&, const FXString&);
        void part(const FXString&, const FXString&);
        void ping(const FXString&);
        void pong(const FXString&, const FXString&);
        void notice(const FXString&, const FXString&);
        void nick(const FXString&, const FXString&);
        void topic(const FXString&, const FXString&);
        void invite(const FXString&, const FXString&);
        void kick(const FXString&, const FXString&);
        void mode(const FXString&, const FXString&);
        void error(const FXString&);
        void unknown(const FXString&, const FXString&);
        void parseRplsupport(FXString);
        void makeStartChannels();
        void clearChannelsCommands(FXbool);
        FXuint stringIPToBinary(const FXString &address);
        FXString binaryIPToString(const FXString &address);
        FXbool isRoutableIP(FXuint ipaddr);
        void resetReconnect();

#ifdef HAVE_OPENSSL
        SSL_CTX *m_ctx;
        SSL *m_ssl;
        FXint m_err;
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
        IrcSocket *m_socket;
};

#endif // IRCSOCKET_H
