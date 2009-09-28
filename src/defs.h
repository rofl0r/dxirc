/*
 *      defs.h
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

#ifndef DEFS_H_
#define DEFS_H_

#include <fx.h>
#include <fxkeys.h>
#include <FXArray.h>
#include <new>
#include <iostream>
#include <map>

class IrcSocket;

enum TYPE {
    SERVER,
    CHANNEL,
    QUERY,
    OTHER
};

enum IrcEventType {
    IRC_CONNECT,
    IRC_DISCONNECT, //send at closeconnection w/o reconnection
    IRC_RECONNECT, //send at closeconnection with reconnection
    IRC_ERROR,
    IRC_NEWCHANNEL,
    IRC_QUERY,
    IRC_PRIVMSG,
    IRC_ACTION,
    IRC_CTCPREPLY,
    IRC_CTCPREQUEST,
    IRC_JOIN,
    IRC_PART,
    IRC_QUIT,
    IRC_UNKNOWN,
    IRC_TOPIC,
    IRC_NICK,
    IRC_MODE,
    IRC_UMODE,
    IRC_CHMODE,
    IRC_SERVERREPLY, //command 001,002,....
    IRC_SERVERERROR, //command 400~599
    IRC_301, //RPL_AWAY
    IRC_305, //RPL_UNAWAY
    IRC_306, //RPL_NOWAWAY
    IRC_331, //RPL_UNAWAY
    IRC_332, //RPL_UNAWAY
    IRC_333, //RPL_TOPICSETBY
    IRC_353, //RPL_NAMREPLY
    IRC_366, //RPL_ENDOFNAMES
    IRC_372, //RPL_MOTD
    IRC_KICK,
    IRC_NOTICE,
    IRC_CHNOTICE,
    IRC_INVITE,
    IRC_AWAY,
    IRC_ENDMOTD
};

enum LuaCommands {
    LUA_HELP,
    LUA_LOAD,
    LUA_UNLOAD,
    LUA_COMMAND,
    LUA_LIST
};

struct IrcEvent {
    IrcEventType eventType;
    FXString param1, param2, param3, param4;
};

struct NickInfo {
    FXString nick;
    FXString user;
    FXString real;
    FXString host;
    FXbool away;
};

struct IgnoreUser {
    FXString nick;
    FXString channel;
    FXString server;
};

struct ServerInfo {
    FXString hostname;
    FXint port;
    FXString nick;
    FXString realname;
    FXString passwd;
    FXString channels;
    FXString commands;
    FXbool autoConnect;
    FXbool useSsl;
};

struct IrcColor {
    FXColor text;
    FXColor back;
    FXColor user;
    FXColor action;
    FXColor notice;
    FXColor error;
    FXColor hilight;
    FXColor link;
};

struct ColorTheme {
    const FXchar* name;
    FXColor base;
    FXColor border;
    FXColor back;
    FXColor fore;
    FXColor selback;
    FXColor selfore;
    FXColor tipback;
    FXColor tipfore;
    FXColor menuback;
    FXColor menufore;
    FXColor shadow;
    FXColor hilite;
};

struct LuaRequest {
    LuaCommands type;
    FXString text;
};

struct LuaScript {
    FXString path;
    FXString name;
    void *L;
};

typedef FXArray<IrcSocket*> dxServersArray;
typedef FXArray<FXObject*> dxTargetsArray;
typedef FXArray<FXString> dxStringArray;
typedef FXArray<NickInfo> dxNicksArray;
typedef FXArray<ServerInfo> dxServerInfoArray;
typedef FXArray<IgnoreUser> dxIgnoreUserArray;
typedef FXArray<LuaScript> dxScriptsArray;
typedef std::map<FXString,FXString> dxStringMap;
typedef std::pair<FXString,FXString> StringPair;
typedef std::map<FXString, FXString>::iterator StringIt;

#endif /* DEFS_H_ */
