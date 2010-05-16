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
#ifndef DXTEXT_H
#include "dxtext.h"
#endif

#define FOXVERSION ((FOX_LEVEL) + (FOX_MINOR*1000) + (FOX_MAJOR*100000))
#define FXVERSION(major,minor,release) ((release)+(minor*1000)+(major*100000))

class IrcSocket;
struct lua_State;

enum TYPE {
    SERVER,
    CHANNEL,
    QUERY,
    DCCCHAT,
    OTHER
};

enum DCCTYPE {
    DCC_NONE,
    DCC_CHATIN, //connect to someone for chat
    DCC_CHATOUT, //someone connect to me for chat
    DCC_IN, //connect to someone for file receive
    DCC_OUT, //someone connect to me for file send
    DCC_PIN, //someone connect to me for file receive (PSEND)
    DCC_POUT //connect to someone for file send (PSEND)
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
    IRC_DCCCHAT,
    IRC_DCCMSG,
    IRC_DCCACTION,
    IRC_DCCSERVER,
    IRC_DCCIN,
    IRC_DCCOUT,
    IRC_DCCPIN,
    IRC_DCCPOUT,
    IRC_DCCTOKEN,
    IRC_DCCMYTOKEN,
    IRC_DCCPOSITION,
    IRC_DCCRESUME,
    IRC_DCCPRESUME,
    IRC_DCCACCEPT,
    IRC_DCCPACCEPT,
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
    IRC_331, //RPL_NOTOPIC
    IRC_332, //RPL_TOPIC
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
    LUA_LOAD,
    LUA_UNLOAD,
    LUA_COMMAND,
    LUA_LIST
};

struct NickInfo {
    FXString nick;
    FXString user;
    FXString real;
    FXString host;
    FXbool away;
};

//used for ignoreuser, tracking friends
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
    FXString version;
    FXString description;
    lua_State *L;
};

struct LuaScriptCommand {
    FXString name; //command name
    FXString funcname;
    FXString helptext;
    FXString script; //script name
};

struct LuaScriptEvent {
    FXString name; //eventname, e.g. PRIVMSG, JOIN or ALL etc.
    FXString funcname;
    FXString script; //script name
};

struct DccFile {
    FXString path;
    DCCTYPE type;
    FXulong currentPosition;
    FXulong previousPostion;
    FXulong size;
    FXbool canceled;
    FXulong finishedPosition; //used in sending for handle user position
    FXint token; //token passive send
    FXString ip; //ip adrress
    FXint port;
    FXString nick; //usefull for resume
    bool operator==(const DccFile& file) const
    {
        return path==file.path && type==file.type && ip==file.ip && port==file.port && token==file.token && nick==file.nick;
    }
};

struct IrcEvent {
    IrcEventType eventType;
    FXString param1, param2, param3, param4;
    DccFile dccFile;
    FXTime time; //for stored events
};

typedef FXArray<IrcSocket*> dxServersArray;
typedef FXArray<FXObject*> dxTargetsArray;
typedef FXArray<FXString> dxStringArray;
typedef FXArray<NickInfo> dxNicksArray;
typedef FXArray<ServerInfo> dxServerInfoArray;
typedef FXArray<IgnoreUser> dxIgnoreUserArray;
typedef FXArray<LuaScript> dxScriptsArray;
typedef FXArray<LuaScriptCommand> dxScriptCommandsArray;
typedef FXArray<LuaScriptEvent> dxScriptEventsArray;
typedef FXArray<DccFile> dxDccFilesArray;
typedef FXArray<IrcEvent> dxIrcEventArray;
typedef FXArray<dxHiliteStyle> dxHiliteArray;
typedef std::map<FXString,FXString> dxStringMap;
typedef std::pair<FXString,FXString> StringPair;
typedef std::map<FXString, FXString>::iterator StringIt;

#endif /* DEFS_H_ */
