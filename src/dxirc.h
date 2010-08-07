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
#include "ircsocket.h"

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
            ID_SPELL,
            ID_LAST
        };

        void create();
        void flash(FXbool yes);

        long onCmdQuit(FXObject*, FXSelector, void*);
        long onCmdAbout(FXObject*, FXSelector, void*);
        long onCmdHelp(FXObject*, FXSelector, void*);
        long onCmdServers(FXObject*, FXSelector, void*);
        long onCmdConnect(FXObject*, FXSelector, void*);
        long onCmdDisconnect(FXObject*, FXSelector, void*);
        long onIrcEvent(FXObject*, FXSelector, void*);
        long onTabBook(FXObject*, FXSelector, void*);
        long onCmdClear(FXObject*, FXSelector, void*);
        long onCmdClearAll(FXObject*, FXSelector, void*);
        long onCmdCloseTab(FXObject*, FXSelector, void*);
        long onCmdSelectTab(FXObject*, FXSelector, void*);
        long onCmdNextTab(FXObject*, FXSelector, void*);
        long onCmdNextUnread(FXObject*, FXSelector, void*);
        long onCmdUsers(FXObject*, FXSelector, void*);
        long onCmdStatus(FXObject*, FXSelector, void*);
        long onCmdOptions(FXObject*, FXSelector, void*);
        long onCmdAlias(FXObject*, FXSelector, void*);
        long onCmdLog(FXObject*, FXSelector, void*);
        long onCmdTransfers(FXObject*, FXSelector, void*);
        long onCmdClose(FXObject*, FXSelector, void*);
        long onCmdDccCancel(FXObject*, FXSelector, void*);
        long onMouseWheel(FXObject*, FXSelector, void*);
        long onTabConnect(FXObject*, FXSelector, void*);
        long onTrayClicked(FXObject*, FXSelector, void*);
        long onNewMsg(FXObject*, FXSelector, void*);
        long onNewTetris(FXObject*, FXSelector, void*);
        long onTetrisKey(FXObject*, FXSelector, void*);
        long onLua(FXObject*, FXSelector, void*);
        long onCmdScripts(FXObject*, FXSelector, void*);
        long onIrcCommand(FXObject*, FXSelector, void*);
        long onIrcMyMsg(FXObject*, FXSelector, void*);
        long onStatusTimeout(FXObject*, FXSelector, void*);
        long onAddIgnoreCommand(FXObject*, FXSelector, void*);
        long onRemoveIgnoreCommand(FXObject*, FXSelector, void*);
        long onAddIgnoreUser(FXObject*, FXSelector, void*);
        long onRemoveIgnoreUser(FXObject*, FXSelector, void*);
        long onCmdSpell(FXObject*, FXSelector, void*);
        static int onLuaAddCommand(lua_State*);
        static int onLuaAddEvent(lua_State*);
        static int onLuaAddMyMsg(lua_State*);
        static int onLuaAddNewTab(lua_State*);
        static int onLuaAddDxircQuit(lua_State*);
        static int onLuaAddAll(lua_State*);
        static int onLuaRemoveName(lua_State*);
        static int onLuaCommand(lua_State*);
        static int onLuaPrint(lua_State*);
        static int onLuaGetServers(lua_State*);
        static int onLuaGetTab(lua_State*);
        static int onLuaGetCurrentTab(lua_State*);
        static int onLuaGetVersion(lua_State*);
        static int onLuaGetTabInfo(lua_State*);
        static int onLuaSetTab(lua_State*);
        static int onLuaCreateTab(lua_State*);
        static int onLuaGetTabCount(lua_State*);
        static int onLuaClear(lua_State*);

    private:
        dxirc(){}
        FXApp *m_app;
        dxServerInfoArray m_serverList;
        dxIgnoreUserArray m_usersList, m_friendsList;
        FXbool m_usersShown, m_statusShown, m_logging, m_ownServerWindow, m_tempServerWindow, m_sameCmd;
        FXbool m_sameList, m_useTray, m_coloredNick, m_closeToTray, m_reconnect, m_autoload;
        FXbool m_sounds, m_soundConnect, m_soundDisconnect, m_soundMessage, m_stripColors, m_useSmileys;
        FXbool m_autoDccChat, m_autoDccFile;
        IrcColor m_colors;
        FXString m_commandsList, m_themesList, m_themePath, m_logPath, m_autoloadPath;
        FXint m_maxAway, m_numberAttempt, m_delayAttempt, m_tabPosition;
        FXint m_dccPortD, m_dccPortH, m_dccTimeout;
        FXString m_nickCompletionChar, m_fontSpec, m_dccPath, m_dccIP;
        FXString m_pathConnect, m_pathDisconnect, m_pathMessage;
        FXFont *m_ircFont;
        ColorTheme m_appTheme;
        FXPopup *m_traymenu;
        FXTrayIcon *m_trayIcon;
        FXint m_lastToken;
        FXuint m_lastID;
        dxStringMap m_smileysMap;
        dxSmileyArray m_smileys;
        FXColor m_trayColor;
        FXMenuBar *m_menubar;
        FXMenuPane *m_servermenu, *m_helpmenu, *m_editmenu;
        FXMenuCommand *m_disconnect, *m_closeTab, *m_clearTab, *m_clearTabs, *m_logviewer;
        FXMenuCheck *m_users, *m_status, *m_spellCombo;
        FXHorizontalFrame *m_statusbar;
        FXLabel *m_statuslabel;
        FXVerticalFrame *m_mainframe;
        dxTabBook *m_tabbook;
        LogViewer *m_viewer;
        DccDialog *m_transfers;
        dxServersArray m_servers;
        dxScriptEventsArray m_scriptEvents;
        FXbool m_useSpell, m_showSpellCombo;

        void onIrcNewchannel(IrcSocket*, IrcEvent*);
        void onIrcQuery(IrcSocket*, IrcEvent*);
        void onIrcPart(IrcSocket*, IrcEvent*);
        void onIrcKick(IrcSocket*, IrcEvent*);
        void onIrcDisconnect(IrcSocket*, IrcEvent*);
        void onIrcConnectAndReconnect(IrcEvent*);
        void onIrcEndmotd();
        void onIrcPrivmsgAndAction(IrcSocket*, IrcEvent*);
        void onIrcJoin(IrcSocket*, IrcEvent*);
        void onIrcQuit(IrcSocket*, IrcEvent*);
        void onIrcDccChat(IrcSocket*, IrcEvent*);
        void onIrcDccServer(IrcSocket*, IrcEvent*);
        void onIrcDccIn(IrcSocket*, IrcEvent*);
        void onIrcDccOut(IrcSocket*, IrcEvent*);
        void onIrcDccPout(IrcSocket*, IrcEvent*);
        void onIrcDccMyToken(IrcSocket*, IrcEvent*);
        void onIrcDccToken(IrcSocket*, IrcEvent*);
        void onIrcDccPosition(IrcSocket*, IrcEvent*);
        void onIrcDccResume(IrcSocket*, IrcEvent*);
        void onIrcDccPresume(IrcSocket*, IrcEvent*);
        void onIrcDccAccept(IrcSocket*, IrcEvent*);
        void onIrcDccPaccept(IrcSocket*, IrcEvent*);
        FXbool tabExist(IrcSocket*, FXString);
        FXbool serverExist(const FXString&, const FXint&, const FXString&);
        FXint getServerTab(IrcSocket*);
        FXint getTabId(IrcSocket*, FXString);
        FXint getTabId(FXString);
        FXint getCurrentTabId();
        FXbool isValidTabId(FXint id);
        FXbool isIdIrcTabItem(FXint id);
        FXbool isLastTab(IrcSocket*);
        FXbool isFriend(const FXString &nick, const FXString &on, const FXString &server);
        void connectServer(FXString hostname, FXint port, FXString pass, FXString nick, FXString rname, FXString channels, FXString commands, FXbool ssl, DCCTYPE dccType=DCC_NONE, FXString dccNick="", IrcSocket *dccParent=0, DccFile dccFile=DccFile());
        void readServersConfig();
        void readConfig();
        void saveConfig();
        void updateMenus();
        void updateTheme();
        void updateTrayColor();
        void updateTabs(FXbool recreateSmileys=FALSE);
        void updateTabPosition();
        void updateFont();
        void updateFont(FXString);
        void updateStatus(FXString);
        FXbool hasTetrisTab();
        void sortTabs();
        void appendIrcText(FXString);
        void appendIrcStyledText(FXString, FXint);
        FXbool hasLuaAll(const FXString &);
        FXbool hasAllCommand();
        FXbool hasMyMsg();
        void autoloadScripts();
        void sendNewTab(IrcSocket*, const FXString&, FXint, FXbool, TYPE);
        void createSmileys();
        FXString getUniqueName(const FXString &path, const FXString &name, const FXString &extension);
        FXbool isForResume(const FXString &name);
        void createIrcTab(const FXString &tabtext, FXIcon *icon, TYPE typ, IrcSocket *socket);

    protected:
        dxScriptsArray m_scripts;
        dxDccFilesArray m_dccfilesList;
        static dxirc *_pThis;

        FXint loadLuaScript(FXString, FXbool=TRUE);
        FXint unloadLuaScript(FXString);
};

#endif // DXIRC_H
