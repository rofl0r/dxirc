/*
 *      ircengine.h
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


#ifndef IRCENGINE_H
#define IRCENGINE_H

#include "defs.h"
#include "config.h"
#include "utils.h"
#ifndef DXSOCKET_H
#include "dxsocket.h"
#endif

class IrcEngine: public FXObject
{
    FXDECLARE(IrcEngine)
    public:
        IrcEngine(FXApp*, FXObject*, FXString, FXString);
        virtual ~IrcEngine();
        enum {
            ID_DXSOCKET = FXMainWindow::ID_LAST+100,
            ID_SERVER,
            ID_RTIME, //reconnect
            ID_TATIME, //try again timeout
            ID_LAST
        };

        void startConnection();
        void disconnect();
        void disconnect(const FXString &reason);
        void closeConnection(FXbool disableReconnect=FALSE);
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
        FXString getLocalIP() { return m_socket->getLocalIP(); }
        FXString getLocalIPBinary();
        FXString getHostname(const FXString &address);
        FXString getRemoteIP() { return m_socket->getRemoteIP(); }
        void addIgnoreWho(const FXString &who);
        void addNick(const FXString &nick, const FXString &user, const FXString &real, const FXString &host, const FXbool &away);
        void addNick(const FXString &nick, const FXString &user, const FXString &real, const FXString &host);
        void removeNick(const FXString &nick);
        NickInfo getNickInfo(const FXString &nick) const;
        FXString getBannedNick(const FXString &banmask) const;
        FXbool sendAdmin(const FXString &params);
        FXbool sendAway(const FXString &params);
        FXbool sendBanlist(const FXString &channel);
        FXbool sendCtcp(const FXString &to, const FXString &params);
        FXbool sendCtcpNotice(const FXString &to, const FXString &params);
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
        FXString getMyUserHost() const { return m_myUserHost; }

        long onSocketCanRead(FXObject*, FXSelector, void*);
        long onSocketConnected(FXObject*, FXSelector, void*);
        long onSocketDisconnected(FXObject*, FXSelector, void*);
        long onSocketError(FXObject*, FXSelector, void*);
        long onReconnectTimeout(FXObject*, FXSelector, void*);
        long onTryAgainTimeout(FXObject*, FXSelector, void*);

    private:
        IrcEngine(){}

        FXApp *m_application;
        FXbool m_connected, m_useSsl, m_connecting, m_reconnect, m_endmotd;
        FXbool m_ignoreUserHost;
        FXint m_serverPort, m_numberAttempt, m_delayAttempt, m_attempts;
        FXint m_nickLen, m_topicLen, m_kickLen, m_awayLen;
        FXString m_serverName, m_realServerName, m_serverPassword, m_nickName;
        FXString m_realName, m_userName, m_startChannels, m_startCommands;
        FXString m_receiveRest;
        FXString m_chanTypes; //channel prefixes
        FXString m_myUserHost; //usefull for dcc
        FXString m_ignoreWho;
        FXchar m_adminPrefix, m_ownerPrefix, m_opPrefix, m_voicePrefix, m_halfopPrefix; //prefix for nick modes
        dxTargetsArray m_targets;
        dxStringArray m_ignoreWhoQueue;
        dxIgnoreUserArray m_usersList;
        dxNicksArray m_nicks;
        dxIrcEventArray m_events;
        dxSocket *m_socket;

        void readData();
        FXbool sendLine(const FXString &line);
        void sendCommands();
        FXbool sendCommand(const FXString &commandtext);
        void parseLine(const FXString &line);
        void sendEvent(IrcEventType);
        void sendEvent(IrcEventType, const FXString&);
        void sendEvent(IrcEventType, const FXString&, const FXString&);
        void sendEvent(IrcEventType, const FXString&, const FXString&, const FXString&);
        void sendEvent(IrcEventType, const FXString&, const FXString&, const FXString&, const FXString&);
        void numeric(const FXint&, const FXString&);
        void privmsg(const FXString&, const FXString&);
        void ctcp(const FXString&, const FXString&);
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
        void resetReconnect();
};

#endif // IRCENGINE_H
