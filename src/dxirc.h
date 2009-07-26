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

class dxirc: public FXMainWindow
{
    FXDECLARE(dxirc)
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
            ID_OPTIONS,
            ID_HELP,
            ID_ABOUT,
            ID_TABS,
            ID_ALIAS,
            ID_TRAY,
            ID_TCANCEL,
            ID_LOG,
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
        long OnCommandOptions(FXObject*, FXSelector, void*);
        long OnCommandAlias(FXObject*, FXSelector, void*);
        long OnCommandLog(FXObject*, FXSelector, void*);
        long OnCommandClose(FXObject*, FXSelector, void*);
        long OnTabConnect(FXObject*, FXSelector, void*);
        long OnTrayClicked(FXObject*, FXSelector, void*);
        long OnNewMsg(FXObject*, FXSelector, void*);
        long OnTrayCancel(FXObject*, FXSelector, void*);

    private:
        dxirc(){}
        FXApp *app;
        dxServerInfoArray serverList;
        dxIgnoreUserArray usersList;
        FXbool usersShown, logging, ownServerWindow, tempServerWindow, sameCmd;
        FXbool sameList, useTray, coloredNick, closeToTray, reconnect;
        IrcColor colors;
        FXString commandsList, themesList, themePath, logPath;
        FXint maxAway, numberAttempt, delayAttempt;
        FXString nickCompletionChar, fontSpec;
        FXFont *ircFont;
        ColorTheme appTheme;
        FXPopup *traymenu;
        FXTrayIcon *trayIcon;
        
        FXbool TabExist(IrcSocket*, FXString);
        FXbool ServerExist(const FXString&, const FXint&, const FXString&);
        FXint GetServerTab(IrcSocket*);
        FXbool IsLastTab(IrcSocket*);
        void ConnectServer(FXString, FXint, FXString, FXString, FXString, FXString, FXString, FXbool);
        void ReadServersConfig();
        void ReadConfig();
        void SaveConfig();
        void UpdateMenus();
        void UpdateTheme();
        void UpdateTabs();
        void UpdateFont(FXString);
        void SortTabs();
        FXString Encrypt(const FXString&);
        FXString Decrypt(const FXString&);
        FXString CheckThemePath(const FXString&);
        FXString CheckThemesList(const FXString&);

    protected:
        FXMenuBar *menubar;
        FXMenuPane *servermenu, *helpmenu, *editmenu;
        FXMenuCommand *disconnect, *closeTab, *clearTab, *clearTabs, *logviewer;
        FXMenuCheck *users;
        FXStatusBar *statusbar;
        FXVerticalFrame *mainframe;
        FXTabBook *tabbook;
        LogViewer *viewer;
        dxServersArray servers;
};

#endif // DXIRC_H
