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

#define LUA_HELP_PATH "http://www.dxirc.org/dxirc-lua.html"

class IrcEngine;
class DccEngine;
struct lua_State;

enum TYPE {
    SERVER,
    CHANNEL,
    QUERY,
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

enum UserMode {
    ADMIN,
    OWNER,
    OP,
    HALFOP,
    VOICE,
    NONE
};

enum dxircID {
    dxirc_QUIT = FXMainWindow::ID_LAST,
    dxirc_SERVERS,
    dxirc_CONNECT,
    dxirc_DISCONNECT,
    dxirc_CLOSETAB,
    dxirc_SELECTTAB,
    dxirc_NEXTTAB,
    dxirc_NEXTUNREAD,
    dxirc_CLEAR,
    dxirc_CLEARALL,
    dxirc_USERS,
    dxirc_STATUS,
    dxirc_OPTIONS,
    dxirc_HELP,
    dxirc_ABOUT,
    dxirc_TABS,
    dxirc_ALIAS,
    dxirc_TRAY,
    dxirc_LOG,
    dxirc_SCRIPTS,
    dxirc_STIMEOUT, //timeout for status change
    dxirc_TETRIS,
    dxirc_TRANSFERS,
    dxirc_SPELL,
    dxirc_FORCEFOCUS,
    dxirc_AWAYTIMEOUT, //timeout for checking away on channels
    AliasDialog_ADD,
    AliasDialog_MODIFY,
    AliasDialog_DELETE,
    AliasDialog_TABLE,
    AliasDialog_SAVECLOSE,
    AliasDialog_CANCEL,
    ConfigDialog_ADDCOMMAND,
    ConfigDialog_DELETECOMMAND,
    ConfigDialog_ADDUSER,
    ConfigDialog_MODIFYUSER,
    ConfigDialog_DELETEUSER,
    ConfigDialog_ADDFRIEND,
    ConfigDialog_MODIFYFRIEND,
    ConfigDialog_DELETEFRIEND,
    ConfigDialog_ADDICONS,
    ConfigDialog_DELETEICONS,
    ConfigDialog_COMMAND,
    ConfigDialog_USER,
    ConfigDialog_FRIEND,
    ConfigDialog_IRCCOLORS,
    ConfigDialog_IRCFONT,
    ConfigDialog_COLORS,
    ConfigDialog_TABCOLORS,
    ConfigDialog_THEME,
    ConfigDialog_FONT,
    ConfigDialog_ACCEPT,
    ConfigDialog_CANCEL,
    ConfigDialog_ICONS,
    ConfigDialog_LOG,
    ConfigDialog_LOGPATH,
    ConfigDialog_SERVERWINDOW,
    ConfigDialog_TRAY,
    ConfigDialog_NICK,
    ConfigDialog_RECONNECT,
    ConfigDialog_TABPOS,
    ConfigDialog_AUTOLOAD,
    ConfigDialog_AUTOLOADPATH,
    ConfigDialog_DCCPATH,
    ConfigDialog_DCCPORTD,
    ConfigDialog_DCCPORTH,
    ConfigDialog_DCCTIMEOUT,
    ConfigDialog_SOUNDS,
    ConfigDialog_SOUNDCONNECT,
    ConfigDialog_SOUNDDISCONNECT,
    ConfigDialog_SOUNDMESSAGE,
    ConfigDialog_PLAYCONNECT,
    ConfigDialog_PLAYDISCONNECT,
    ConfigDialog_PLAYMESSAGE,
    ConfigDialog_NOTIFY,
    ConfigDialog_SHOWNOTIFY,
    ConfigDialog_SELECTCONNECT,
    ConfigDialog_SELECTDISCONNECT,
    ConfigDialog_SELECTMESSAGE,
    ConfigDialog_USESMILEYS,
    ConfigDialog_ADDSMILEY,
    ConfigDialog_MODIFYSMILEY,
    ConfigDialog_DELETESMILEY,
    ConfigDialog_SMILEY,
    ConfigDialog_IMPORTSMILEY,
    ConfigDialog_EXPORTSMILEY,
    SmileyDialog_ACCEPT,
    SmileyDialog_CANCEL,
    SmileyDialog_PATH,
    DccDialog_CLOSE,
    DccDialog_CLEAR,
    DccDialog_CANCELED,
    DccDialog_UTIME,
    DccDialog_POPUPCANCEL,
    DccDialog_DCCCANCEL,
    DccDialog_TABLE,
    DccEngine_SOCKET,
    DccEngine_DCC,
    DccEngine_CTIME, //timeout for check and disconnect offered connection
    DccEngine_PTIME, //dccfile position
    DccTabItem_COMMANDLINE,
    DccTabItem_PTIME,
    DccTabItem_NEXTTAB,
    DccTabItem_TEXT,
    DccTabItem_SPELL,
    DccTabItem_MYMSG,
    DccTabItem_COMMAND,
    DccTabItem_SOCKET,
    DccTabItem_CTIME, //timeout for check and disconnect offered connection
    DccTabItem_LUA,
    DccTabItem_NEWTETRIS,
    DccTabItem_NEWMSG,
    IrcEngine_DXSOCKET = FXMainWindow::ID_LAST+100,
    IrcEngine_SERVER,
    IrcEngine_RTIME, //reconnect
    IrcEngine_TATIME, //try again timeout
    DccSendDialog_SEND,
    DccSendDialog_CANCEL,
    DccSendDialog_FILE,
    IrcTabItem_COMMANDLINE,
    IrcTabItem_CDIALOG,
    IrcTabItem_CQUIT,
    IrcTabItem_PTIME,
    IrcTabItem_ETIME,
    IrcTabItem_USERS,
    IrcTabItem_NEWQUERY,
    IrcTabItem_WHOIS,
    IrcTabItem_OP,
    IrcTabItem_DEOP,
    IrcTabItem_VOICE,
    IrcTabItem_DEVOICE,
    IrcTabItem_KICK,
    IrcTabItem_BAN,
    IrcTabItem_KICKBAN,
    IrcTabItem_IGNORE,
    IrcTabItem_TOPIC,
    IrcTabItem_CSERVER,
    IrcTabItem_NEXTTAB,
    IrcTabItem_TEXT,
    IrcTabItem_NEWMSG,
    IrcTabItem_LUA,
    IrcTabItem_COMMAND,
    IrcTabItem_MYMSG,
    IrcTabItem_NEWTETRIS,
    IrcTabItem_DCCCHAT, //dcc chat on right click
    IrcTabItem_DCCSEND, //dcc sending on right click
    IrcTabItem_ADDICOMMAND, //for handle /ignore addcmd
    IrcTabItem_RMICOMMAND, //for handle /ignore rmcmd
    IrcTabItem_ADDIUSER, //for handle /ignore addusr
    IrcTabItem_RMIUSER, //for handle /ignore rmusr
    IrcTabItem_AWAY, //set away on right click
    IrcTabItem_DEAWAY, //remove away on right click
    IrcTabItem_SPELL,
    LogViewer_ADD,
    LogViewer_CLOSE,
    LogViewer_TREE,
    LogViewer_RESET,
    LogViewer_ALL,
    LogViewer_CHANNEL,
    LogViewer_FILE,
    LogViewer_SEARCH,
    LogViewer_SEARCHNEXT,
    LogViewer_PACK,
    LogViewer_UNPACK,
    LogViewer_DELETEITEM,
    ScriptDialog_LIST,
    ScriptDialog_LOAD,
    ScriptDialog_VIEW,
    ScriptDialog_VIEWNEW,
    ScriptDialog_UNLOAD,
    ScriptDialog_CLOSE,
    ServerDialog_JOIN,
    ServerDialog_LIST,
    ServerDialog_CANCEL,
    ServerDialog_ADD,
    ServerDialog_MODIFY,
    ServerDialog_DELETE,
    ServerDialog_SAVECLOSE,
    TetrisTabItem_GAMECANVAS,
    TetrisTabItem_NEXTCANVAS,
    TetrisTabItem_TETRISTIMEOUT,
    TetrisTabItem_NEW,
    TetrisTabItem_PAUSE
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
    FXString network;
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
    FXulong speed;
    FXulong size;
    FXbool canceled;
    FXulong finishedPosition; //used in sending for handle user position
    FXint token; //token passive send
    FXString ip; //ip address
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

typedef FXArray<IrcEngine*> dxIrcEnginesArray;
typedef FXArray<DccEngine*> dxDccEnginesArray;
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
