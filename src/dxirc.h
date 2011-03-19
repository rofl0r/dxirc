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
#include "ircengine.h"
#include "dccengine.h"

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

        void create();
        void flash(FXbool yes);

        long onCmdQuit(FXObject*, FXSelector, void*);
        long onCmdAbout(FXObject*, FXSelector, void*);
        long onCmdHelp(FXObject*, FXSelector, void*);
        long onCmdServers(FXObject*, FXSelector, void*);
        long onCmdConnect(FXObject*, FXSelector, void*);
        long onCmdDisconnect(FXObject*, FXSelector, void*);
        long onIrcEvent(FXObject*, FXSelector, void*);
        long onDccEvent(FXObject*, FXSelector, void*);
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
        long onCmdForceFocus(FXObject*, FXSelector, void*);
        long onAwayTimeout(FXObject*, FXSelector, void*);
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
        void showNotify(const FXString& notify);

    private:
        dxirc(){}
        FXApp *m_app;
        dxServerInfoArray m_serverList;
        dxIgnoreUserArray m_usersList, m_friendsList;
        FXbool m_usersShown, m_statusShown, m_logging, m_ownServerWindow, m_tempServerWindow, m_sameCmd;
        FXbool m_sameList, m_useTray, m_coloredNick, m_closeToTray, m_reconnect, m_autoload;
        FXbool m_sounds, m_soundConnect, m_soundDisconnect, m_soundMessage, m_stripColors, m_useSmileys;
        FXbool m_autoDccChat, m_autoDccFile;
        FXbool m_notify, m_notifyConnect, m_notifyDisconnect, m_notifyMessage;
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
        FXColor m_trayColor, m_unreadColor, m_highlightColor;
        FXMenuBar *m_menubar;
        FXMenuPane *m_servermenu, *m_helpmenu, *m_editmenu;
        FXMenuCommand *m_disconnect, *m_closeTab, *m_clearTab, *m_clearTabs, *m_logviewer;
        FXMenuCheck *m_users, *m_status;
#ifdef HAVE_ENCHANT
        FXMenuCheck *m_spellCombo;
#endif
        FXHorizontalFrame *m_statusbar;
        FXLabel *m_statuslabel;
        FXVerticalFrame *m_mainframe;
        dxTabBook *m_tabbook;
        LogViewer *m_viewer;
        DccDialog *m_transfers;
        dxIrcEnginesArray m_ircengines;
        dxDccEnginesArray m_dccengines;
        dxScriptEventsArray m_scriptEvents;
        FXbool m_useSpell, m_showSpellCombo;
#ifndef HAVE_LIBNOTIFY
        dxEXNotify *m_wnotify;
#endif

        void onIrcNewchannel(IrcEngine*, IrcEvent*);
        void onIrcQuery(IrcEngine*, IrcEvent*);
        void onIrcPart(IrcEngine*, IrcEvent*);
        void onIrcKick(IrcEngine*, IrcEvent*);
        void onIrcDisconnect(IrcEngine*, IrcEvent*);
        void onIrcConnectAndReconnect(IrcEvent*);
        void onIrcEndmotd();
        void onIrcPrivmsgAndAction(IrcEngine*, IrcEvent*);
        void onIrcJoin(IrcEngine*, IrcEvent*);
        void onIrcQuit(IrcEngine*, IrcEvent*);
        void onIrcDccChat(IrcEngine*, IrcEvent*);
        void onIrcDccServer(IrcEngine*, IrcEvent*);
        void onIrcDccIn(IrcEngine*, IrcEvent*);
        void onIrcDccOut(IrcEngine*, IrcEvent*);
        void onIrcDccPout(IrcEngine*, IrcEvent*);
        void onIrcDccMyToken(IrcEngine*, IrcEvent*);
        void onIrcDccToken(IrcEngine*, IrcEvent*);
        void onIrcDccPosition(DccEngine*, IrcEvent*);
        void onIrcDccResume(IrcEngine*, IrcEvent*);
        void onIrcDccPresume(IrcEngine*, IrcEvent*);
        void onIrcDccAccept(IrcEngine*, IrcEvent*);
        void onIrcDccPaccept(IrcEngine*, IrcEvent*);
        FXbool tabExist(IrcEngine*, FXString);
        FXbool serverExist(FXString, FXint, FXString);
        FXint getServerTab(IrcEngine*);
        FXint getTabId(IrcEngine*, FXString);
        FXint getTabId(FXString);
        FXint getCurrentTabId();
        FXbool isValidTabId(FXint id);
        FXbool isIddxTabItem(FXint id);
        FXbool isLastTab(IrcEngine*);
        FXbool isFriend(FXString nick, FXString on, FXString network);
        void connectServer(FXString hostname, FXint port, FXString pass, FXString nick, FXString rname, FXString channels, FXString commands, FXbool ssl);
        void readServersConfig();
        void readConfig();
        void saveConfig();
        void saveLangs();
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
        void appendIrcText(FXString text, FXbool logLine=TRUE);
        void appendIrcStyledText(FXString text, FXint style, FXbool logLine=TRUE);
        FXbool hasLuaAll(const FXString &);
        FXbool hasAllCommand();
        FXbool hasMyMsg();
        void autoloadScripts();
        void sendNewTab(IrcEngine *server, const FXString &name, FXint id, FXbool isTetris, FXbool isDccTab, TYPE type);
        void createSmileys();
        FXString getUniqueName(const FXString &path, const FXString &name, const FXString &extension);
        FXbool isForResume(const FXString &name);
        FXint createIrcTab(const FXString &tabtext, FXIcon *icon, TYPE typ, IrcEngine *engine);
        void createDccTab(const FXString &mynick, const FXString &nick, const FXString &address="0", FXint portD=0, FXint portH=0, FXbool listen=FALSE, IrcEngine *engine=NULL);

    protected:
        dxScriptsArray m_scripts;
        dxDccFilesArray m_dccfilesList;
        static dxirc *_pThis;

        FXint loadLuaScript(FXString, FXbool=TRUE);
        FXint unloadLuaScript(FXString);
};

#endif // DXIRC_H
