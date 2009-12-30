/*
 *      dxirc.h
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


#ifndef DXIRC_H
#define DXIRC_H

#include "defs.h"
#include "FXTrayApp.h"
#include "FXTrayIcon.h"
#include "logviewer.h"
#include "config.h"
#include "dccdialog.h"
#include "dxtabbook.h"

#ifdef HAVE_LUA
extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}
#endif

class dxirc;
typedef void (dxirc::* func_ptr) (const FXString&);

class dxirc: public FXMainWindow
{
    FXDECLARE(dxirc)
    friend class ScriptDialog;
    friend class DccDialog;
    public:
        dxirc(FXApp *app);
        virtual ~dxirc();
        enum {
            ID_QUIT = FXMainWindow::ID_LAST,
            ID_SERVERS,
            ID_CONNECT,
            ID_DISCONNECT,
            ID_CLOSETAB,
            ID_SELECTTAB,
            ID_NEXTTAB,
            ID_NEXTUNREAD,
            ID_CLEAR,
            ID_CLEARALL,
            ID_USERS,
            ID_STATUS,
            ID_OPTIONS,
            ID_HELP,
            ID_ABOUT,
            ID_TABS,
            ID_ALIAS,
            ID_TRAY,
            ID_LOG,
            ID_SCRIPTS,
            ID_STIMEOUT,
            ID_TETRIS,
            ID_TRANSFERS,
            ID_LAST
        };

        void create();

        long OnCommandQuit(FXObject*, FXSelector, void*);
        long OnCommandAbout(FXObject*, FXSelector, void*);
        long OnCommandHelp(FXObject*, FXSelector, void*);
        long OnCommandServers(FXObject*, FXSelector, void*);
        long OnCommandConnect(FXObject*, FXSelector, void*);
        long OnCommandDisconnect(FXObject*, FXSelector, void*);
        long OnIrcEvent(FXObject*, FXSelector, void*);
        long OnTabBook(FXObject*, FXSelector, void*);
        long OnCommandClear(FXObject*, FXSelector, void*);
        long OnCommandClearAll(FXObject*, FXSelector, void*);
        long OnCommandCloseTab(FXObject*, FXSelector, void*);
        long OnCommandSelectTab(FXObject*, FXSelector, void*);
        long OnCommandNextTab(FXObject*, FXSelector, void*);
        long OnCommandNextUnread(FXObject*, FXSelector, void*);
        long OnCommandUsers(FXObject*, FXSelector, void*);
        long OnCommandStatus(FXObject*, FXSelector, void*);
        long OnCommandOptions(FXObject*, FXSelector, void*);
        long OnCommandAlias(FXObject*, FXSelector, void*);
        long OnCommandLog(FXObject*, FXSelector, void*);
        long OnCommandTransfers(FXObject*, FXSelector, void*);
        long OnCommandClose(FXObject*, FXSelector, void*);
        long OnCommandDccCancel(FXObject*, FXSelector, void*);
        long OnMouseWheel(FXObject*, FXSelector, void*);
        long OnTabConnect(FXObject*, FXSelector, void*);
        long OnTrayClicked(FXObject*, FXSelector, void*);
        long OnNewMsg(FXObject*, FXSelector, void*);
        long OnNewTetris(FXObject*, FXSelector, void*);
        long OnTetrisKey(FXObject*, FXSelector, void*);
        long OnLua(FXObject*, FXSelector, void*);
        long OnCommandScripts(FXObject*, FXSelector, void*);
        long OnIrcCommand(FXObject*, FXSelector, void*);
        long OnIrcMyMsg(FXObject*, FXSelector, void*);
        long OnStatusTimeout(FXObject*, FXSelector, void*);
        static int OnLuaAddCommand(lua_State*);
        static int OnLuaAddEvent(lua_State*);
        static int OnLuaAddMyMsg(lua_State*);
        static int OnLuaAddAll(lua_State*);
        static int OnLuaRemoveName(lua_State*);
        static int OnLuaCommand(lua_State*);
        static int OnLuaPrint(lua_State*);
        static int OnLuaGetServers(lua_State*);
        static int OnLuaGetTab(lua_State*);
        static int OnLuaGetTabInfo(lua_State*);
        static int OnLuaSetTab(lua_State*);

    private:
        dxirc(){}
        FXApp *app;
        dxServerInfoArray serverList;
        dxIgnoreUserArray usersList;
        FXbool usersShown, statusShown, logging, ownServerWindow, tempServerWindow, sameCmd;
        FXbool sameList, useTray, coloredNick, closeToTray, reconnect, autoload;
        IrcColor colors;
        FXString commandsList, themesList, themePath, logPath, autoloadPath;
        FXint maxAway, numberAttempt, delayAttempt, tabPosition, dccPortD, dccPortH, dccTimeout;
        FXString nickCompletionChar, fontSpec, dccPath, dccIP;
        FXFont *ircFont;
        ColorTheme appTheme;
        FXPopup *traymenu;
        FXTrayIcon *trayIcon;
        FXint lastToken;
        
        FXbool TabExist(IrcSocket*, FXString);
        FXbool ServerExist(const FXString&, const FXint&, const FXString&);
        FXint GetServerTab(IrcSocket*);
        FXint GetTabId(IrcSocket*, FXString);
        FXint GetTabId(FXString);
        FXbool IsLastTab(IrcSocket*);
        void ConnectServer(FXString hostname, FXint port, FXString pass, FXString nick, FXString rname, FXString channels, FXString commands, FXbool ssl, DCCTYPE dccType=DCC_NONE, FXString dccNick="", IrcSocket *dccParent=0, DccFile dccFile=DccFile());
        void ReadServersConfig();
        void ReadConfig();
        void SaveConfig();
        void UpdateMenus();
        void UpdateTheme();
        void UpdateTabs();
        void UpdateTabPosition();
        void UpdateFont();
        void UpdateFont(FXString);
        void UpdateStatus(FXString);
        FXbool HasTetrisTab();
        void SortTabs();                
        void AppendIrcText(FXString);
        void AppendIrcStyledText(FXString, FXint);        
        FXbool HasLuaAll(const FXString &);
        FXbool HasAllCommand();
        FXbool HasMyMsg();
        void AutoloadScripts();

    protected:
        FXMenuBar *menubar;
        FXMenuPane *servermenu, *helpmenu, *editmenu;
        FXMenuCommand *disconnect, *closeTab, *clearTab, *clearTabs, *logviewer;
        FXMenuCheck *users;
        FXMenuCheck *status;
        FXHorizontalFrame *statusbar;
        FXLabel *statuslabel;
        FXVerticalFrame *mainframe;
        dxTabBook *tabbook;
        LogViewer *viewer;
        DccDialog *transfers;
        dxServersArray servers;
        dxScriptsArray scripts;
        dxScriptEventsArray scriptEvents;
        dxDccFilesArray dccfilesList;
        static dxirc *pThis;

        FXint LoadLuaScript(FXString, FXbool=TRUE);
        FXint UnloadLuaScript(FXString);
};

#endif // DXIRC_H
