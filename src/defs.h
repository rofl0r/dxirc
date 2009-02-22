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

class IrcSocket;

enum TYPE {
    SERVER,
    CHANNEL,
    QUERY,
    OTHER
};

enum IrcEventType {
    IRC_CONNECT,
    IRC_DISCONNECT,
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
    IRC_301,
    IRC_331,
    IRC_332,
    IRC_333,
    IRC_353,
    IRC_366,
    IRC_372,
    IRC_KICK,
    IRC_NOTICE,
    IRC_CHNOTICE,
    IRC_INVITE,
    IRC_AWAY,
    IRC_ENDMOTD,
};

struct IrcEvent {
    IrcEventType eventType;
    IrcSocket *ircSocket;
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
};

struct IrcColor {
    FXColor text;
    FXColor back;
    FXColor user;
    FXColor action;
    FXColor notice;
    FXColor error;
    FXColor hilight;
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

typedef FXArray<FXString> FXHistoryArray;
typedef FXArray<IrcSocket*> FXServersArray;
typedef FXArray<FXObject*> FXTargetsArray;
typedef FXArray<FXString> FXCommandsArray;
typedef FXArray<NickInfo> FXNicksArray;
typedef FXArray<ServerInfo> FXServerInfoArray;
typedef FXArray<IgnoreUser> FXIgnoreUserArray;

#endif /* DEFS_H_ */
