/*
 *      dxirc.cpp
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

#ifdef WIN32
 #include <windows.h>
#endif

#include "dxirc.h"
#include "icons.h"
#include "config.h"
#include "i18n.h"
#include "help.h"
#include "irctabitem.h"
#include "configdialog.h"
#include "serverdialog.h"

FXDEFMAP(dxirc) dxircMap[] = {
    FXMAPFUNC(SEL_CLOSE,    0,                          dxirc::OnCommandQuit),
    FXMAPFUNC(SEL_COMMAND,  dxirc::ID_QUIT,             dxirc::OnCommandQuit),
    FXMAPFUNC(SEL_COMMAND,  dxirc::ID_ABOUT,            dxirc::OnCommandAbout),
    FXMAPFUNC(SEL_COMMAND,  dxirc::ID_CONNECT,          dxirc::OnCommandConnect),
    FXMAPFUNC(SEL_COMMAND,  IrcTabItem::ID_CONNECT,     dxirc::OnCommandConnect),
    FXMAPFUNC(SEL_COMMAND,  dxirc::ID_DISCONNECT,       dxirc::OnCommandDisconnect),
    FXMAPFUNC(SEL_COMMAND,  IrcTabItem::ID_TABQUIT,     dxirc::OnCommandDisconnect),
    FXMAPFUNC(SEL_COMMAND,  dxirc::ID_SERVERS,          dxirc::OnCommandServers),
    FXMAPFUNC(SEL_COMMAND,  IrcSocket::ID_SERVER,       dxirc::OnIrcEvent),
    FXMAPFUNC(SEL_COMMAND,  dxirc::ID_TABS,             dxirc::OnTabBook),
    FXMAPFUNC(SEL_COMMAND,  dxirc::ID_CLEAR,            dxirc::OnCommandClear),
    FXMAPFUNC(SEL_COMMAND,  dxirc::ID_CLEARALL,         dxirc::OnCommandClearAll),
    FXMAPFUNC(SEL_COMMAND,  dxirc::ID_CLOSETAB,         dxirc::OnCommandCloseTab),
    FXMAPFUNC(SEL_KEYPRESS, dxirc::ID_SELECTTAB,        dxirc::OnCommandSelectTab),
    FXMAPFUNC(SEL_COMMAND,  dxirc::ID_USERS,            dxirc::OnCommandUsers),
    FXMAPFUNC(SEL_COMMAND,  dxirc::ID_HELP,             dxirc::OnCommandHelp),
    FXMAPFUNC(SEL_COMMAND,  dxirc::ID_OPTIONS,          dxirc::OnCommandOptions),
    FXMAPFUNC(SEL_KEYPRESS, dxirc::ID_NEXTTAB,          dxirc::OnCommandNextTab),
    FXMAPFUNC(SEL_KEYPRESS, dxirc::ID_NEXTUNREAD,       dxirc::OnCommandNextUnread),
};

FXIMPLEMENT(dxirc, FXMainWindow, dxircMap, ARRAYNUMBER(dxircMap))

dxirc::dxirc(FXApp *app)
    : FXMainWindow(app, PACKAGE, 0, 0, DECOR_ALL, 0, 0, 800, 600), app(app)
{
    setIcon(bigicon);
    setMiniIcon(smallicon);

    SetLocaleEncoding();

    ReadConfig();

    menubar = new FXMenuBar(this, FRAME_RAISED|LAYOUT_SIDE_TOP|LAYOUT_FILL_X);

    servermenu = new FXMenuPane(this);
    new FXMenuCommand(servermenu, _("&Server list\tF2"), serverlisticon, this, ID_SERVERS);
    new FXMenuCommand(servermenu, _("Quick &connect\tCtrl-K"), connecticon, this, ID_CONNECT);
    disconnect = new FXMenuCommand(servermenu, _("&Disconnect\tCtrl-D"), disconnecticon, this, ID_DISCONNECT);
    disconnect->disable();
    new FXMenuSeparator(servermenu);
    new FXMenuCommand(servermenu, _("&Quit\tAlt-F4"), quiticon, this, ID_QUIT);
    new FXMenuTitle(menubar, _("&Server"), NULL, servermenu);

    editmenu = new FXMenuPane(this);
    closeTab = new FXMenuCommand(editmenu, _("Close current tab\tCtrl-W"), closeicon, this, ID_CLOSETAB);
    closeTab->disable();
    new FXMenuSeparator(editmenu);
    clearTab = new FXMenuCommand(editmenu, _("Clear window\tCtrl-L"), clearicon, this, ID_CLEAR);
    clearTabs = new FXMenuCommand(editmenu, _("Clear all windows\tCtrl-Shift-L"), NULL, this, ID_CLEARALL);
    new FXMenuSeparator(editmenu);
    new FXMenuCommand(editmenu, _("Users list\tCtrl-U\tShow/Hide users list"), NULL, this, ID_USERS);
    new FXMenuCommand(editmenu, _("&Preferences"), optionicon, this, ID_OPTIONS);
    new FXMenuTitle(menubar, _("&Edit"), NULL, editmenu);

    helpmenu = new FXMenuPane(this);
    new FXMenuCommand(helpmenu, _("&Help\tF1"), helpicon, this, ID_HELP);
    new FXMenuCommand(helpmenu, _("&About.."), NULL, this, ID_ABOUT);
    new FXMenuTitle(menubar, _("&Help"), NULL, helpmenu);

    statusbar = new FXStatusBar(this, LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X);

    mainframe = new FXVerticalFrame(this, LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 1,1,1,1);

    tabbook = new FXTabBook(mainframe, this, ID_TABS, PACK_UNIFORM_WIDTH|PACK_UNIFORM_HEIGHT|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_RIGHT);
    tabbook->setTabStyle(TABBOOK_BOTTOMTABS);
    FXuint packing = tabbook->getPackingHints();
    packing &= ~PACK_UNIFORM_WIDTH;
    tabbook->setPackingHints(packing);

    IrcSocket *server = new IrcSocket(app, this, 0, localeEncoding, "");
    server->SetUsersList(usersList);
    servers.append(server);

    IrcTabItem *tabitem = new IrcTabItem(tabbook, "(server)", servericon, TAB_BOTTOM, SERVER, server, ownServerWindow, usersHidden, logging, commandsList, logPath, maxAway, colors.text, colors.back, colors.user, colors.action, colors.notice, colors.error, nickCompletionChar);
    server->AppendTarget(tabitem);

    new FXToolTip(app,0);

    getAccelTable()->addAccel(MKUINT(KEY_1, ALTMASK), this, FXSEL(SEL_KEYPRESS, ID_SELECTTAB));
    getAccelTable()->addAccel(MKUINT(KEY_2, ALTMASK), this, FXSEL(SEL_KEYPRESS, ID_SELECTTAB));
    getAccelTable()->addAccel(MKUINT(KEY_3, ALTMASK), this, FXSEL(SEL_KEYPRESS, ID_SELECTTAB));
    getAccelTable()->addAccel(MKUINT(KEY_4, ALTMASK), this, FXSEL(SEL_KEYPRESS, ID_SELECTTAB));
    getAccelTable()->addAccel(MKUINT(KEY_5, ALTMASK), this, FXSEL(SEL_KEYPRESS, ID_SELECTTAB));
    getAccelTable()->addAccel(MKUINT(KEY_6, ALTMASK), this, FXSEL(SEL_KEYPRESS, ID_SELECTTAB));
    getAccelTable()->addAccel(MKUINT(KEY_7, ALTMASK), this, FXSEL(SEL_KEYPRESS, ID_SELECTTAB));
    getAccelTable()->addAccel(MKUINT(KEY_8, ALTMASK), this, FXSEL(SEL_KEYPRESS, ID_SELECTTAB));
    getAccelTable()->addAccel(MKUINT(KEY_9, ALTMASK), this, FXSEL(SEL_KEYPRESS, ID_SELECTTAB));
    getAccelTable()->addAccel(MKUINT(KEY_Tab, CONTROLMASK), this, FXSEL(SEL_KEYPRESS, ID_NEXTTAB));
    getAccelTable()->addAccel(MKUINT(KEY_n, CONTROLMASK), this, FXSEL(SEL_KEYPRESS, ID_NEXTUNREAD));
    getAccelTable()->addAccel(MKUINT(KEY_N, CONTROLMASK), this, FXSEL(SEL_KEYPRESS, ID_NEXTUNREAD));
}

dxirc::~dxirc()
{
    delete bigicon;
    delete smallicon;
    delete irc_admin_icon;
    delete irc_away_admin_icon;
    delete irc_away_halfop_icon;
    delete irc_away_normal_icon;
    delete irc_away_op_icon;
    delete irc_away_owner_icon;
    delete irc_away_voice_icon;
    delete irc_halfop_icon;
    delete irc_normal_icon;
    delete irc_op_icon;
    delete irc_owner_icon;
    delete irc_voice_icon;
    delete serverlisticon;
    delete connecticon;
    delete disconnecticon;
    delete quiticon;
    delete closeicon;
    delete optionicon;
    delete helpicon;
    delete servericon;
    delete channelicon;
    delete queryicon;
    delete clearicon;
    delete flagicon;
    delete servermenu;
    delete editmenu;
    delete helpmenu;
}

void dxirc::create()
{
    FXMainWindow::create();
    ReadServersConfig();
    show(PLACEMENT_SCREEN);
}

int CompareTabs(const void **a,const void **b)
{
    IrcTabItem *fa = (IrcTabItem*)*a;
    IrcTabItem *fb = (IrcTabItem*)*b;
    return comparecase((fa->GetType() == SERVER ? fa->GetServerName() : fa->GetServerName()+fa->getText()), (fb->GetType() == SERVER ? fb->GetServerName() : fb->GetServerName()+fb->getText()));
}

void dxirc::ReadConfig()
{
    usersHidden = getApp()->reg().readBoolEntry("SETTINGS", "usersHidden", false);
    commandsList = getApp()->reg().readStringEntry("SETTINGS", "commandsList");
    themePath = CheckThemePath(getApp()->reg().readStringEntry("SETTINGS", "themePath", DXIRC_DATADIR PATHSEPSTRING "icons" PATHSEPSTRING "default"));
    themesList = CheckThemesList(getApp()->reg().readStringEntry("SETTINGS", "themesList", FXString(themePath+";").text()));
    colors.text = getApp()->reg().readColorEntry("SETTINGS", "textColor", FXRGB(0,0,0));
    colors.back = getApp()->reg().readColorEntry("SETTINGS", "textBackColor", FXRGB(255,255,255));
    colors.user = getApp()->reg().readColorEntry("SETTINGS", "userColor", FXRGB(191,191,191));
    colors.action = getApp()->reg().readColorEntry("SETTINGS", "actionsColor", FXRGB(255,165,0));
    colors.notice = getApp()->reg().readColorEntry("SETTINGS", "noticeColor", FXRGB(0,0,255));
    colors.error = getApp()->reg().readColorEntry("SETTINGS", "errorColor", FXRGB(255,0,0));
    maxAway = getApp()->reg().readIntEntry("SETTINGS", "maxAway", 200);
    logging = getApp()->reg().readBoolEntry("SETTINGS", "logging", false);
    ownServerWindow = getApp()->reg().readBoolEntry("SETTINGS", "serverWindow", true);
    nickCompletionChar = FXString(getApp()->reg().readStringEntry("SETTINGS", "nickCompletionChar", ":")).left(1);
    tempServerWindow = ownServerWindow;
    logPath = getApp()->reg().readStringEntry("SETTINGS", "logPath");
    if(logging && !FXStat::exists(logPath)) logging = false;
    FXint usersNum = getApp()->reg().readIntEntry("USERS", "number", 0);
    if(usersNum)
    {
        for(FXint i=0; i<usersNum; i++)
        {
            IgnoreUser user;
            user.nick = getApp()->reg().readStringEntry(FXStringFormat("USER%d", i).text(), "nick", FXStringFormat("xxx%d", i).text());
            user.channel = getApp()->reg().readStringEntry(FXStringFormat("USER%d", i).text(), "channel", "all");
            user.server = getApp()->reg().readStringEntry(FXStringFormat("USER%d", i).text(), "server", "all");
            usersList.append(user);
        }
    }
}

FXString dxirc::CheckThemePath(const FXString &path)
{
    if(path == "internal") return path;
    else
    {
        const char *themeDefaultPath = DXIRC_DATADIR PATHSEPSTRING "icons" PATHSEPSTRING "default";
        if(FXStat::exists(path)) return path;
        return FXString(themeDefaultPath);
    }
}

FXString dxirc::CheckThemesList(const FXString &list)
{
    const char *themeDefaultPath = DXIRC_DATADIR PATHSEPSTRING "icons" PATHSEPSTRING "default;";
    FXString themes;
    for(FXint i=0; i<list.contains(';'); i++)
    {
        if(list.before(';', i+1).rafter(';') == "internal") themes.append("internal;");
        if(FXStat::exists(list.before(';', i+1).rafter(';'))) themes.append(list.before(';', i+1).rafter(';')+";");
    }
    if(!themes.empty()) return themes;
    return FXString("internal;")+FXString(themeDefaultPath);
}

void dxirc::ReadServersConfig()
{
    FXint serversNum = getApp()->reg().readIntEntry("SERVERS", "number", 0);
    FXint autoConnect = 0;
    if(serversNum)
    {
        for(FXint i=0; i<serversNum; i++)
        {
            ServerInfo server;
            server.hostname = getApp()->reg().readStringEntry(FXStringFormat("SERVER%d", i).text(), "hostname", FXStringFormat("localhost%d", i).text());
            server.port = getApp()->reg().readIntEntry(FXStringFormat("SERVER%d", i).text(), "port", 6667);
            server.nick = getApp()->reg().readStringEntry(FXStringFormat("SERVER%d", i).text(), "nick", "xxx");
            server.realname = getApp()->reg().readStringEntry(FXStringFormat("SERVER%d", i).text(), "realname", "xxx");
            server.passwd = Decrypt(getApp()->reg().readStringEntry(FXStringFormat("SERVER%d", i).text(), "hes", ""));
            server.channels = getApp()->reg().readStringEntry(FXStringFormat("SERVER%d", i).text(), "channels", "");
            server.autoConnect = getApp()->reg().readBoolEntry(FXStringFormat("SERVER%d", i).text(), "autoconnect", false);
            if(server.autoConnect)
            {
                autoConnect++;
                if(servers.no() == 1 && !servers[0]->GetConnected() && autoConnect < 2)
                {
                    servers[0]->SetServerName(server.hostname);
                    servers[0]->SetServerPort(server.port);
                    servers[0]->SetServerPassword(server.passwd);
                    server.nick.length() ? servers[0]->SetNickName(server.nick) : servers[0]->SetNickName("_xxx_");
                    server.nick.length() ? servers[0]->SetUserName(server.nick) : servers[0]->SetUserName("_xxx_");
                    server.realname.length() ? servers[0]->SetRealName(server.realname) : servers[0]->SetRealName(server.nick.length() ? server.nick : "_xxx_");
                    if (server.channels.length()>1) servers[0]->SetStartChannels(server.channels);
                    if (!tabbook->numChildren())
                    {
                        IrcTabItem *tabitem = new IrcTabItem(tabbook, server.hostname, servericon, TAB_BOTTOM, SERVER, servers[0], ownServerWindow, usersHidden, logging, commandsList, logPath, maxAway, colors.text, colors.back, colors.user, colors.action, colors.notice, colors.error, nickCompletionChar);
                        servers[0]->AppendTarget(tabitem);
                        tabitem->create();
                        tabitem->CreateGeom();
                        //tabbook->setCurrent(tabbook->numChildren()/2);
                    }
                    ((IrcTabItem *)tabbook->childAtIndex(0))->SetType(SERVER, server.hostname);
                    SortTabs();
                    servers[0]->Connect();
                }
                else if(!ServerExist(server.hostname, server.port))
                {
                    IrcSocket *newserver = new IrcSocket(app, this, 0, localeEncoding, "");
                    newserver->SetUsersList(usersList);
                    servers.prepend(newserver);
                    servers[0]->SetServerName(server.hostname);
                    servers[0]->SetServerPort(server.port);
                    servers[0]->SetServerPassword(server.passwd);
                    server.nick.length() ? servers[0]->SetNickName(server.nick) : servers[0]->SetNickName("_xxx_");
                    server.nick.length() ? servers[0]->SetUserName(server.nick) : servers[0]->SetUserName("_xxx_");
                    server.realname.length() ? servers[0]->SetRealName(server.realname) : servers[0]->SetRealName(server.nick.length() ? server.nick : "_xxx_");
                    if (server.channels.length()>1) servers[0]->SetStartChannels(server.channels);
                    IrcTabItem *tabitem = new IrcTabItem(tabbook, server.hostname, servericon, TAB_BOTTOM, SERVER, servers[0], ownServerWindow, usersHidden, logging, commandsList, logPath, maxAway, colors.text, colors.back, colors.user, colors.action, colors.notice, colors.error, nickCompletionChar);
                    servers[0]->AppendTarget(tabitem);
                    tabitem->create();
                    tabitem->CreateGeom();
                    SortTabs();
                    //tabbook->setCurrent(tabbook->numChildren()/2);
                    servers[0]->Connect();
                }
            }
            serverList.append(server);
        }
    }
}

void dxirc::SaveConfig()
{
    //getApp()->reg().clear();
    getApp()->reg().writeIntEntry("SERVERS", "number", serverList.no());
    if(serverList.no())
    {
        for(FXint i=0; i<serverList.no(); i++)
        {
            getApp()->reg().writeStringEntry(FXStringFormat("SERVER%d", i).text(), "hostname", serverList[i].hostname.text());
            getApp()->reg().writeIntEntry(FXStringFormat("SERVER%d", i).text(), "port", serverList[i].port);
            getApp()->reg().writeStringEntry(FXStringFormat("SERVER%d", i).text(), "nick", serverList[i].nick.text());
            getApp()->reg().writeStringEntry(FXStringFormat("SERVER%d", i).text(), "realname", serverList[i].realname.text());
            getApp()->reg().writeStringEntry(FXStringFormat("SERVER%d", i).text(), "hes", Encrypt(serverList[i].passwd).text());
            getApp()->reg().writeStringEntry(FXStringFormat("SERVER%d", i).text(), "channels", serverList[i].channels.text());
            getApp()->reg().writeBoolEntry(FXStringFormat("SERVER%d", i).text(), "autoconnect", serverList[i].autoConnect);
        }
    }
    getApp()->reg().writeBoolEntry("SETTINGS", "usersHidden", usersHidden);
    getApp()->reg().writeStringEntry("SETTINGS", "commandsList", commandsList.text());
    getApp()->reg().writeStringEntry("SETTINGS", "themePath", themePath.text());
    getApp()->reg().writeStringEntry("SETTINGS", "themesList", themesList.text());
    getApp()->reg().writeColorEntry("SETTINGS", "textColor", colors.text);
    getApp()->reg().writeColorEntry("SETTINGS", "textBackColor", colors.back);
    getApp()->reg().writeColorEntry("SETTINGS", "userColor", colors.user);
    getApp()->reg().writeColorEntry("SETTINGS", "actionsColor", colors.action);
    getApp()->reg().writeColorEntry("SETTINGS", "noticeColor", colors.notice);
    getApp()->reg().writeColorEntry("SETTINGS", "errorColor", colors.error);
    getApp()->reg().writeIntEntry("SETTINGS", "maxAway", maxAway);
    getApp()->reg().writeBoolEntry("SETTINGS", "logging", logging);
    if(ownServerWindow == tempServerWindow) getApp()->reg().writeBoolEntry("SETTINGS", "serverWindow", ownServerWindow);
    else getApp()->reg().writeBoolEntry("SETTINGS", "serverWindow", tempServerWindow);
    getApp()->reg().writeStringEntry("SETTINGS", "logPath", logPath.text());
    getApp()->reg().writeStringEntry("SETTINGS", "nickCompletionChar", nickCompletionChar.text());
    getApp()->reg().writeIntEntry("USERS", "number", usersList.no());
    if(usersList.no())
    {

        for(FXint i=0; i<usersList.no(); i++)
        {
            getApp()->reg().writeStringEntry(FXStringFormat("USER%d", i).text(), "nick", usersList[i].nick.text());
            getApp()->reg().writeStringEntry(FXStringFormat("USER%d", i).text(), "channel", usersList[i].channel.text());
            getApp()->reg().writeStringEntry(FXStringFormat("USER%d", i).text(), "server", usersList[i].server.text());
        }
    }
}

long dxirc::OnCommandQuit(FXObject*, FXSelector, void*)
{
    while(servers.no())
    {
        if(servers[0]->GetConnected()) servers[0]->Disconnect();
        servers.erase(0);
    }
    SaveConfig();
    getApp()->exit(0);
    return 1;
}

long dxirc::OnCommandHelp(FXObject*, FXSelector, void*)
{
    FXDialogBox helpDialog(this, _("Help"), DECOR_TITLE|DECOR_BORDER, 0,0,0,0, 0,0,0,0, 0,0);
    FXVerticalFrame *contents = new FXVerticalFrame(&helpDialog, LAYOUT_SIDE_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 10,10,10,10, 0,0);

    FXHorizontalFrame *textbox=new FXHorizontalFrame(contents,LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN|FRAME_THICK,0,0,0,0, 0,0,0,0);
    FXText *text=new FXText(textbox,NULL,0,TEXT_READONLY|TEXT_WORDWRAP|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    text->setVisibleRows(35);
    text->setVisibleColumns(90);
    text->setText(HELP_TEXT);

    new FXButton(contents, _("Close"), NULL, &helpDialog, FXDialogBox::ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 32,32,5,5);

    helpDialog.execute(PLACEMENT_CURSOR);
    return 1;
}

long dxirc::OnCommandUsers(FXObject*, FXSelector, void*)
{
    usersHidden = !usersHidden;
    for (FXint i = 0; i<tabbook->numChildren(); i=i+2)
    {
        if(usersHidden) ((IrcTabItem *)tabbook->childAtIndex(i))->HideUsers();
        else ((IrcTabItem *)tabbook->childAtIndex(i))->ShowUsers();
    }
    return 1;
}

long dxirc::OnCommandOptions(FXObject*, FXSelector, void*)
{
    ConfigDialog dialog(this, colors, commandsList, usersList, themePath, themesList, maxAway, logging, logPath, tempServerWindow, nickCompletionChar);
    if(dialog.execute(PLACEMENT_CURSOR))
    {
        commandsList = dialog.GetCommandsList();
        usersList = dialog.GetUsersList();
        colors = dialog.GetColors();
        themePath = dialog.GetThemePath();
        themesList = dialog.GetThemesList();
        maxAway = dialog.GetMaxAway();
        logging = dialog.GetLogging();
        tempServerWindow = dialog.GetServerWindow();
        logPath = dialog.GetLogPath();
        nickCompletionChar = dialog.GetNickCompletionChar();
        for (FXint i = 0; i<tabbook->numChildren(); i=i+2)
        {
            ((IrcTabItem *)tabbook->childAtIndex(i))->SetColor(colors.text, colors.back, colors.user, colors.action, colors.notice, colors.error);
            ((IrcTabItem *)tabbook->childAtIndex(i))->SetCommandsList(commandsList);
            ((IrcTabItem *)tabbook->childAtIndex(i))->SetMaxAway(maxAway);
            ((IrcTabItem *)tabbook->childAtIndex(i))->SetLogging(logging);
            ((IrcTabItem *)tabbook->childAtIndex(i))->SetLogPath(logPath);
            ((IrcTabItem *)tabbook->childAtIndex(i))->SetNickCompletionChar(nickCompletionChar);
        }
        for (FXint i = 0; i<servers.no(); i++)
        {
            servers[i]->SetUsersList(usersList);
        }
    }
    return 1;
}

long dxirc::OnCommandAbout(FXObject*, FXSelector, void*)
{
    FXDialogBox about(this, FXStringFormat(_("About %s"), PACKAGE), DECOR_TITLE|DECOR_BORDER, 0,0,0,0, 0,0,0,0, 0,0);
    FXVerticalFrame *content = new FXVerticalFrame(&about, LAYOUT_SIDE_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 10,10,10,10, 0,0);
    new FXLabel(content, FXStringFormat("%s %s\n", PACKAGE, VERSION), 0, JUSTIFY_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(content, _("Copyright (C) 2008~ David Vachulka (david@konstrukce-cad.com)\n"), 0, JUSTIFY_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(content, FXStringFormat(_("This software uses the FOX Toolkit Library version %d.%d.%d (http://www.fox-toolkit.org).\n"),FOX_MAJOR,FOX_MINOR,FOX_LEVEL), 0, JUSTIFY_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    FXButton *button = new FXButton(content, _("OK"), 0, &about, FXDialogBox::ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 32,32,5,5);
    button->setFocus();
    about.execute(PLACEMENT_OWNER);

    return 1;
}

long dxirc::OnCommandServers(FXObject*, FXSelector, void*)
{
    ServerDialog *dialog = new ServerDialog(this, serverList);
    FXint indexJoin = -1;
    if (dialog->execute(PLACEMENT_OWNER))
    {
        serverList = dialog->GetServers();
        indexJoin = dialog->GetIndexJoin();
        if (indexJoin != -1 && !ServerExist(serverList[indexJoin].hostname, serverList[indexJoin].port))
        {
            if(servers.no() == 1 && !servers[0]->GetConnected())
            {
                servers[0]->SetServerName(serverList[indexJoin].hostname);
                servers[0]->SetServerPort(serverList[indexJoin].port);
                servers[0]->SetServerPassword(serverList[indexJoin].passwd);
                servers[0]->SetNickName(serverList[indexJoin].nick);
                servers[0]->SetUserName(serverList[indexJoin].nick);
                serverList[indexJoin].realname.length() ? servers[0]->SetRealName(serverList[indexJoin].realname) : servers[0]->SetRealName(serverList[indexJoin].nick);
                if (serverList[indexJoin].channels.length()>1) servers[0]->SetStartChannels(serverList[indexJoin].channels);
                if (!tabbook->numChildren())
                {
                    IrcTabItem *tabitem = new IrcTabItem(tabbook, serverList[indexJoin].hostname, servericon, TAB_BOTTOM, SERVER, servers[0], ownServerWindow, usersHidden, logging, commandsList, logPath, maxAway, colors.text, colors.back, colors.user, colors.action, colors.notice, colors.error, nickCompletionChar);
                    servers[0]->AppendTarget(tabitem);
                    tabitem->create();
                    tabitem->CreateGeom();
                    //tabbook->setCurrent(tabbook->numChildren()/2);
                }
                ((IrcTabItem *)tabbook->childAtIndex(0))->SetType(SERVER, serverList[indexJoin].hostname);
                SortTabs();
                servers[0]->Connect();
            }
            else if(!ServerExist(serverList[indexJoin].hostname, serverList[indexJoin].port))
            {
                IrcSocket *server = new IrcSocket(app, this, 0, localeEncoding, "");
                server->SetUsersList(usersList);
                servers.prepend(server);
                servers[0]->SetServerName(serverList[indexJoin].hostname);
                servers[0]->SetServerPort(serverList[indexJoin].port);
                servers[0]->SetServerPassword(serverList[indexJoin].passwd);
                servers[0]->SetNickName(serverList[indexJoin].nick);
                servers[0]->SetUserName(serverList[indexJoin].nick);
                serverList[indexJoin].realname.length() ? servers[0]->SetRealName(serverList[indexJoin].realname) : servers[0]->SetRealName(serverList[indexJoin].nick);
                if (serverList[indexJoin].channels.length()>1) servers[0]->SetStartChannels(serverList[indexJoin].channels);
                IrcTabItem *tabitem = new IrcTabItem(tabbook, serverList[indexJoin].hostname, servericon, TAB_BOTTOM, SERVER, servers[0], ownServerWindow, usersHidden, logging, commandsList, logPath, maxAway, colors.text, colors.back, colors.user, colors.action, colors.notice, colors.error, nickCompletionChar);
                servers[0]->AppendTarget(tabitem);
                tabitem->create();
                tabitem->CreateGeom();
                //tabbook->setCurrent(tabbook->numChildren()/2);
                SortTabs();
                servers[0]->Connect();
            }
        }
    }
    UpdateMenus();
    return 1;
}

long dxirc::OnCommandConnect(FXObject*, FXSelector, void*)
{
    FXDialogBox serverEdit(this, _("Quick connect"), DECOR_TITLE|DECOR_BORDER, 0,0,0,0, 0,0,0,0, 0,0);
    serverEdit.getAccelTable()->addAccel(KEY_Return, &serverEdit, FXDialogBox::ID_ACCEPT);
    FXVerticalFrame *contents = new FXVerticalFrame(&serverEdit, LAYOUT_SIDE_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 10,10,10,10, 0,0);
    FXMatrix *matrix = new FXMatrix(contents,2,MATRIX_BY_COLUMNS|LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y);

    new FXLabel(matrix, _("Hostname:"),NULL,JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *hostname = new FXTextField(matrix, 25,NULL, 0, TEXTFIELD_ENTER_ONLY|FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    hostname->setText("localhost");

    new FXLabel(matrix, _("Port:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXSpinner *port = new FXSpinner(matrix, 23,NULL, 0, FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    port->setRange(0, 65536);
    port->setValue(6667);

    new FXLabel(matrix, _("Password:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *passwd = new FXTextField(matrix, 25, NULL, 0, TEXTFIELD_PASSWD|TEXTFIELD_ENTER_ONLY|FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);

    new FXLabel(matrix, _("Nickname:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *nick = new FXTextField(matrix, 25, NULL, 0, TEXTFIELD_ENTER_ONLY|FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    nick->setText(FXSystem::currentUserName());

    new FXLabel(matrix, _("Realname:"),NULL,JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField* realname = new FXTextField(matrix, 25, NULL, 0, TEXTFIELD_ENTER_ONLY|FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);

    new FXLabel(matrix, _("Channel(s):\tChannels need to be comma separated"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *channel = new FXTextField(matrix, 25, NULL, 0, TEXTFIELD_ENTER_ONLY|FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    channel->setText("#");

    FXHorizontalFrame *buttonframe = new FXHorizontalFrame(contents,LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXButton(buttonframe, _("&OK"), NULL, &serverEdit, FXDialogBox::ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 32,32,5,5);
    new FXButton(buttonframe, _("Cancel"), NULL, &serverEdit, FXDialogBox::ID_CANCEL, BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 32,32,5,5);
    if (serverEdit.execute(PLACEMENT_OWNER))
    {
        if(servers.no() == 1 && !servers[0]->GetConnected())
        {
            servers[0]->SetServerName(hostname->getText());
            servers[0]->SetServerPort(port->getValue());
            servers[0]->SetServerPassword(passwd->getText());
            nick->getText().length() ? servers[0]->SetNickName(nick->getText()) : servers[0]->SetNickName("_xxx_");
            nick->getText().length() ? servers[0]->SetUserName(nick->getText()) : servers[0]->SetUserName("_xxx_");
            realname->getText().length() ? servers[0]->SetRealName(realname->getText()) : servers[0]->SetRealName(nick->getText().length() ? nick->getText() : "_xxx_");
            if (channel->getText().length()>1) servers[0]->SetStartChannels(channel->getText());
            if (!tabbook->numChildren())
            {
                IrcTabItem *tabitem = new IrcTabItem(tabbook, hostname->getText(), servericon, TAB_BOTTOM, SERVER, servers[0], ownServerWindow, usersHidden, logging, commandsList, logPath, maxAway, colors.text, colors.back, colors.user, colors.action, colors.notice, colors.error, nickCompletionChar);
                servers[0]->AppendTarget(tabitem);
                tabitem->create();
                tabitem->CreateGeom();
                //tabbook->setCurrent(tabbook->numChildren()/2);
            }
            ((IrcTabItem *)tabbook->childAtIndex(0))->SetType(SERVER, hostname->getText());
            SortTabs();
            servers[0]->Connect();
        }
        else if(!ServerExist(hostname->getText(), port->getValue()))
        {
            IrcSocket *server = new IrcSocket(app, this, 0, localeEncoding, "");
            server->SetUsersList(usersList);
            servers.prepend(server);
            servers[0]->SetServerName(hostname->getText());
            servers[0]->SetServerPort(port->getValue());
            servers[0]->SetServerPassword(passwd->getText());
            nick->getText().length() ? servers[0]->SetNickName(nick->getText()) : servers[0]->SetNickName("_xxx_");
            nick->getText().length() ? servers[0]->SetUserName(nick->getText()) : servers[0]->SetUserName("_xxx_");
            realname->getText().length() ? servers[0]->SetRealName(realname->getText()) : servers[0]->SetRealName(nick->getText().length() ? nick->getText() : "_xxx_");
            if (channel->getText().length()>1) servers[0]->SetStartChannels(channel->getText());
            IrcTabItem *tabitem = new IrcTabItem(tabbook, hostname->getText(), servericon, TAB_BOTTOM, SERVER, servers[0], ownServerWindow, usersHidden, logging, commandsList, logPath, maxAway, colors.text, colors.back, colors.user, colors.action, colors.notice, colors.error, nickCompletionChar);
            servers[0]->AppendTarget(tabitem);
            tabitem->create();
            tabitem->CreateGeom();
            //tabbook->setCurrent(tabbook->numChildren()/2);
            SortTabs();
            servers[0]->Connect();
        }
    }
    UpdateMenus();
    return 1;
}

long dxirc::OnCommandDisconnect(FXObject*, FXSelector, void*)
{
    if(tabbook->numChildren())
    {
        FXint index = tabbook->getCurrent()*2;
        IrcTabItem *currenttab = (IrcTabItem *)tabbook->childAtIndex(index);
        IrcSocket *currentserver;
        for(FXint i=0; i < servers.no(); i++)
        {
            if(servers[i]->FindTarget(currenttab))
            {
                currentserver = servers[i];
                break;
            }
        }
        if(currentserver->GetConnected())
        {
            FXDialogBox confirmDialog(this, _("Confirm disconnect"), DECOR_TITLE|DECOR_BORDER, 0,0,0,0, 0,0,0,0, 0,0);
            FXVerticalFrame *contents = new FXVerticalFrame(&confirmDialog, LAYOUT_SIDE_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 10,10,10,10, 0,0);
            new FXLabel(contents, FXStringFormat(_("Disconnect server: %s:%d?"), currentserver->GetServerName().text(), currentserver->GetServerPort()),NULL);
            FXHorizontalFrame* buttonframe = new FXHorizontalFrame(contents,LAYOUT_FILL_X|LAYOUT_FILL_Y);
            new FXButton(buttonframe, _("OK"), NULL, &confirmDialog, FXDialogBox::ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 32,32,5,5);
            new FXButton(buttonframe, _("Cancel"), NULL, &confirmDialog, FXDialogBox::ID_CANCEL, BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 32,32,5,5);
            if (confirmDialog.execute(PLACEMENT_OWNER))
            {
                currentserver->Disconnect();
                for(FXint i = tabbook->numChildren()-2; i > -1; i=i-2)
                {
                    if(currentserver->FindTarget((IrcTabItem *)tabbook->childAtIndex(i)))
                    {
                        currentserver->RemoveTarget((IrcTabItem *)tabbook->childAtIndex(i));
                        delete tabbook->childAtIndex(i);
                        delete tabbook->childAtIndex(i);
                        tabbook->recalc();
                    }
                }
            }
        }
        else
        {
            for(FXint i = tabbook->numChildren()-2; i > -1; i=i-2)
            {
                if(currentserver->FindTarget((IrcTabItem *)tabbook->childAtIndex(i)))
                {
                    currentserver->RemoveTarget((IrcTabItem *)tabbook->childAtIndex(i));
                    delete tabbook->childAtIndex(i);
                    delete tabbook->childAtIndex(i);
                    tabbook->recalc();
                }
            }
        }
        SortTabs();
    }
    UpdateMenus();
    return 1;
}

long dxirc::OnIrcEvent(FXObject *, FXSelector, void *data)
{
    IrcEvent *ev = (IrcEvent *) data;
    IrcSocket *server = ev->ircSocket;
    if(ev->eventType == IRC_NEWCHANNEL)
    {
        FXint serverTabIndex = GetServerTab(server);
        if(serverTabIndex != -1 && !ownServerWindow)
        {
            ((IrcTabItem *)tabbook->childAtIndex(serverTabIndex))->SetType(CHANNEL, ev->param1);
        }
        else
        {
            IrcTabItem* tabitem = new IrcTabItem(tabbook, ev->param1, channelicon, TAB_BOTTOM, CHANNEL, server, ownServerWindow, usersHidden, logging, commandsList, logPath, maxAway, colors.text, colors.back, colors.user, colors.action, colors.notice, colors.error, nickCompletionChar);
            server->AppendTarget(tabitem);
            tabitem->create();
            tabitem->CreateGeom();
            //tabbook->setCurrent(tabbook->numChildren()/2);
        }
        SortTabs();
    }
    else if(ev->eventType == IRC_QUERY && !TabExist(server, ev->param1))
    {
        FXint serverTabIndex = GetServerTab(server);
        if(serverTabIndex != -1 && !ownServerWindow)
        {
            ((IrcTabItem *)tabbook->childAtIndex(serverTabIndex))->SetType(QUERY, ev->param1);
        }
        else
        {
            IrcTabItem* tabitem = new IrcTabItem(tabbook, ev->param1, queryicon, TAB_BOTTOM, QUERY, server, ownServerWindow, usersHidden, logging, commandsList, logPath, maxAway, colors.text, colors.back, colors.user, colors.action, colors.notice, colors.error, nickCompletionChar);
            server->AppendTarget(tabitem);
            tabitem->create();
            tabitem->CreateGeom();
            //tabbook->setCurrent(tabbook->numChildren()/2);
        }
        SortTabs();
    }
    else if(ev->eventType == IRC_PART)
    {
        if(TabExist(server, ev->param2))
        {
            if(ev->param1 == server->GetNickName())
            {
                if(server->GetConnected() && IsLastTab(server))
                {
                    for(FXint j = 0; j < tabbook->numChildren(); j=j+2)
                    {
                        if(server->FindTarget((IrcTabItem *)tabbook->childAtIndex(j)))
                        {
                            ((IrcTabItem *)tabbook->childAtIndex(j))->SetType(SERVER, server->GetServerName());
                            break;
                        }
                    }
                }
                else
                {
                    FXint index;
                    for(FXint j = 0; j < tabbook->numChildren(); j=j+2)
                    {
                        if((((IrcTabItem *)tabbook->childAtIndex(j))->getText().lower() == ev->param2.lower()) && (((IrcTabItem *)tabbook->childAtIndex(j))->GetServerName() == server->GetServerName())) index = j;
                    }
                    server->RemoveTarget((IrcTabItem *)tabbook->childAtIndex(index));
                    delete tabbook->childAtIndex(index);
                    delete tabbook->childAtIndex(index);
                    tabbook->recalc();
                }
                SortTabs();
            }
        }
    }
    else if(ev->eventType == IRC_KICK)
    {
        if(ev->param2 == server->GetNickName())
        {
            if(server->GetConnected() && IsLastTab(server))
            {
                for(FXint j = 0; j < tabbook->numChildren(); j=j+2)
                {
                    if(server->FindTarget((IrcTabItem *)tabbook->childAtIndex(j)))
                    {
                        ((IrcTabItem *)tabbook->childAtIndex(j))->SetType(SERVER, server->GetServerName());
                        break;
                    }
                }
            }
            else
            {
                FXint index;
                for(FXint j = 0; j < tabbook->numChildren(); j=j+2)
                {
                    if((((IrcTabItem *)tabbook->childAtIndex(j))->getText().lower() == ev->param3.lower()) && server->FindTarget((IrcTabItem *)tabbook->childAtIndex(j))) index = j;
                }
                server->RemoveTarget((IrcTabItem *)tabbook->childAtIndex(index));
                delete tabbook->childAtIndex(index);
                delete tabbook->childAtIndex(index);
                tabbook->recalc();
            }
            SortTabs();
        }
    }
    UpdateMenus();
    return 1;
}

long dxirc::OnTabBook(FXObject *, FXSelector, void *)
{
    FXint index = tabbook->getCurrent()*2;
    IrcTabItem *currenttab = (IrcTabItem *)tabbook->childAtIndex(index);
    if (FXRGB(0,0,0) != currenttab->getTextColor()) currenttab->setTextColor(FXRGB(0,0,0));
    return 1;
}

long dxirc::OnCommandNextTab(FXObject *, FXSelector, void *)
{
    FXint index = tabbook->getCurrent();
    if(tabbook->numChildren())
    {
        if((index+1)*2 < tabbook->numChildren()) tabbook->setCurrent(index+1, tabbook->numChildren() > index*2 ? true : false);
        else tabbook->setCurrent(0, true);
    }
    return 1;
}

long dxirc::OnCommandNextUnread(FXObject *, FXSelector, void*)
{
    if(tabbook->numChildren())
    {
        for (FXint i = tabbook->getCurrent()*2; i<tabbook->numChildren(); i=i+2)
        {
            if (FXRGB(0,0,0) != ((IrcTabItem *)tabbook->childAtIndex(i))->getTextColor())
            {
                ((IrcTabItem *)tabbook->childAtIndex(i))->setTextColor(FXRGB(0,0,0));
                tabbook->setCurrent(i/2, true);
                return 1;
            }
        }
        for (FXint i = tabbook->getCurrent()*2; i>-1; i=i-2)
        {
            if (FXRGB(0,0,0) != ((IrcTabItem *)tabbook->childAtIndex(i))->getTextColor())
            {
                ((IrcTabItem *)tabbook->childAtIndex(i))->setTextColor(FXRGB(0,0,0));
                tabbook->setCurrent(i/2, true);
                return 1;
            }
        }
    }
    return 1;
}

long dxirc::OnCommandClear(FXObject *, FXSelector, void *)
{
    if(tabbook->numChildren())
    {
        FXint index = tabbook->getCurrent()*2;
        IrcTabItem *currenttab = (IrcTabItem *)tabbook->childAtIndex(index);
        currenttab->ClearChat();
    }
    return 1;
}

long dxirc::OnCommandClearAll(FXObject *, FXSelector, void *)
{
    for (FXint i = 0; i<tabbook->numChildren(); i=i+2)
    {
        ((IrcTabItem *)tabbook->childAtIndex(i))->ClearChat();
        if (FXRGB(0,0,0) != ((IrcTabItem *)tabbook->childAtIndex(i))->getTextColor()) ((IrcTabItem *)tabbook->childAtIndex(i))->setTextColor(FXRGB(0,0,0));
    }
    return 1;
}

long dxirc::OnCommandCloseTab(FXObject *, FXSelector, void *)
{
    if(tabbook->numChildren())
    {
        FXint index = tabbook->getCurrent()*2;
        IrcTabItem *currenttab = (IrcTabItem *)tabbook->childAtIndex(index);
        IrcSocket *currentserver;
        for(FXint i=0; i < servers.no(); i++)
        {
            if(servers[i]->FindTarget(currenttab)) currentserver = servers[i];
        }
        if(currenttab->GetType() == QUERY)
        {
            if(currentserver->GetConnected() && IsLastTab(currentserver))
            {
                for(FXint j = 0; j < tabbook->numChildren(); j=j+2)
                {
                    if(currentserver->FindTarget((IrcTabItem *)tabbook->childAtIndex(j)))
                    {
                        ((IrcTabItem *)tabbook->childAtIndex(j))->SetType(SERVER, currentserver->GetServerName());
                        break;
                    }
                }
            }
            else
            {
                currentserver->RemoveTarget(currenttab);
                delete tabbook->childAtIndex(index);
                delete tabbook->childAtIndex(index);
                tabbook->recalc();
            }
        }
        else if(currenttab->GetType() == CHANNEL)
        {
            if(currentserver->GetConnected()) currentserver->SendPart(currenttab->getText());
            if(currentserver->GetConnected() && IsLastTab(currentserver))
            {
                for(FXint j = 0; j < tabbook->numChildren(); j=j+2)
                {
                    if(currentserver->FindTarget((IrcTabItem *)tabbook->childAtIndex(j)))
                    {
                        ((IrcTabItem *)tabbook->childAtIndex(j))->SetType(SERVER, currentserver->GetServerName());
                        break;
                    }
                }
            }
            else
            {
                currentserver->RemoveTarget(currenttab);
                delete tabbook->childAtIndex(index);
                delete tabbook->childAtIndex(index);
                tabbook->recalc();
            }
        }
        else if(currenttab->GetType() == SERVER)
        {
            if(currentserver->GetConnected()) currentserver->Disconnect();
            for(FXint i = tabbook->numChildren()-2; i > -1; i=i-2)
            {
                if(currentserver->FindTarget((IrcTabItem *)tabbook->childAtIndex(i)))
                {
                    currentserver->RemoveTarget((IrcTabItem *)tabbook->childAtIndex(i));
                    delete tabbook->childAtIndex(i);
                    delete tabbook->childAtIndex(i);
                    tabbook->recalc();
                }
            }
        }
        SortTabs();
    }
    UpdateMenus();
    return 1;
}

long dxirc::OnCommandSelectTab(FXObject*, FXSelector, void *ptr)
{
    FXEvent* event = (FXEvent*)ptr;
    switch(event->code){
        case KEY_1:
        {
            tabbook->setCurrent(0, tabbook->numChildren() ? true : false);
            break;
        }
        case KEY_2:
        {
            tabbook->setCurrent(1, tabbook->numChildren() > 2 ? true : false);
            break;
        }
        case KEY_3:
        {
            tabbook->setCurrent(2, tabbook->numChildren() > 4 ? true : false);
            break;
        }
        case KEY_4:
        {
            tabbook->setCurrent(3, tabbook->numChildren() > 6 ? true : false);
            break;
        }
        case KEY_5:
        {
            tabbook->setCurrent(4, tabbook->numChildren() > 8 ? true : false);
            break;
        }
        case KEY_6:
        {
            tabbook->setCurrent(5, tabbook->numChildren() > 10 ? true : false);
            break;
        }
        case KEY_7:
        {
            tabbook->setCurrent(6, tabbook->numChildren() > 12 ? true : false);
            break;
        }
        case KEY_8:
        {
            tabbook->setCurrent(7, tabbook->numChildren() > 14 ? true : false);
            break;
        }
        case KEY_9:
        {
            tabbook->setCurrent(8, tabbook->numChildren() > 16 ? true : false);
            break;
        }
    }
    return 1;
}

FXbool dxirc::TabExist(IrcSocket *server, FXString name)
{
    for(FXint i = 0; i < tabbook->numChildren(); i=i+2)
    {
        if(server->FindTarget((IrcTabItem *)tabbook->childAtIndex(i)) && ((IrcTabItem *)tabbook->childAtIndex(i))->getText().lower() == name.lower()) return true;
    }
    return false;
}

FXbool dxirc::ServerExist(const FXString &server, const FXint &port)
{
    for(FXint i = 0; i < servers.no(); i++)
    {
        if(servers[i]->GetServerName() == server && servers[i]->GetServerPort() == port && servers[i]->GetConnected()) return true;
    }
    return false;
}

FXint dxirc::GetServerTab(IrcSocket *server)
{
    for(FXint i = 0; i < tabbook->numChildren(); i=i+2)
    {
        if(server->FindTarget((IrcTabItem *)tabbook->childAtIndex(i)) && ((IrcTabItem *)tabbook->childAtIndex(i))->GetType() == SERVER) return i;
    }
    return -1;
}

FXbool dxirc::IsLastTab(IrcSocket *server)
{
    FXint numTabs = 0;
    for(FXint i = 0; i < tabbook->numChildren(); i=i+2)
    {
        if(server->FindTarget((IrcTabItem *)tabbook->childAtIndex(i))) numTabs++;
    }
    if(numTabs > 1) return false;
    else return true;
}

void dxirc::SortTabs()
{
    if(tabbook->numChildren()/2 > 1)
    {
        IrcTabItem *tabpole[tabbook->numChildren()/2];
        for(FXint i = 0; i < tabbook->numChildren()/2; i++)
        {
            tabpole[i] = (IrcTabItem*)tabbook->childAtIndex(i*2);
        }
        qsort(tabpole, tabbook->numChildren()/2, sizeof(tabpole[0]), (int(*)(const void*, const void*))&CompareTabs);
        for(int i=0; i < tabbook->numChildren()/2; i++)
        {
            tabpole[i]->ReparentTab();
        }
        tabbook->recalc();
    }
}

void dxirc::UpdateMenus()
{
    if(tabbook->numChildren())
    {
        closeTab->enable();
        clearTab->enable();
        clearTabs->enable();
    }
    else
    {
        closeTab->disable();
        clearTab->disable();
        clearTabs->disable();
    }
    FXbool someConnected = false;
    for(FXint i = 0; i < servers.no(); i++)
    {
        if(servers[i]->GetConnected()) someConnected = true;
    }
    if(someConnected) disconnect->enable();
    else disconnect->disable();
}

FXString dxirc::Encrypt(const FXString &text)
{
    FXString result = text;
    int key[8] = {13,15,0,8,9,4,39,26};
    for(FXint i=0; i<text.count(); i++)
    {
        result[i] = char(int(text[i])+key[i%8]);
    }
    return result;
}

FXString dxirc::Decrypt(const FXString &text)
{
    FXString result = text;
    int key[8] = {13,15,0,8,9,4,39,26};
    for(FXint i=0; i<text.count(); i++)
    {
        result[i] = char(int(text[i])-key[i%8]);
    }
    return result;
}

void dxirc::SetLocaleEncoding()
{
    FXString locale, language, territory, codeset, modifier;
#ifdef WIN32
    UINT codepage = ::GetACP();
    switch(codepage)
    {
        case 1250:
        case 1251:
        case 1252:
        case 1253:
        case 1254:
        case 1255:
        case 1256:
        case 1257:
        case 1258:
        case 437:
        case 850:
        case 852:
        case 855:
        case 856:
        case 857:
        case 860:
        case 861:
        case 862:
        case 863:
        case 864:
        case 865:
        case 866:
        case 869:
        case 874: localeEncoding = FXStringFormat("CP%d", codepage); break;
        case 20866: localeEncoding = "KOI8R"; break;
        default: localeEncoding = "CP1252";
    }
#else
    if(!(FXSystem::getEnvironment("LANG")).empty()) locale = FXSystem::getEnvironment("LANG");
    else if(!(FXSystem::getEnvironment("LC_ALL")).empty()) locale = FXSystem::getEnvironment("LC_ALL");
    else if(!(FXSystem::getEnvironment("LC_MESSAGES")).empty()) locale = FXSystem::getEnvironment("LC_MESSAGES");
    else locale = "en_US";
    if(locale == "C" || locale == "POSIX")
    {
        locale = "en_US";
    }
    locale = locale.lower();
    if(locale.contains("utf")) locale = locale.before('.');
    language = locale.before('_');
    territory = locale.after('_').before('.').before('@');
    codeset = locale.after('.');
    modifier = locale.after('@');

    if(language == "af") localeEncoding = "88591";
    else if(language == "ar") localeEncoding = "88596";
    else if(language == "az") localeEncoding = "88599";
    else if(language == "be") localeEncoding = "CP1251";
    else if(language == "bg")
    {
        if(codeset=="iso88595") localeEncoding = "88595";
        else if(codeset=="koi8r") localeEncoding = "KOI8R";
        else localeEncoding = "CP1251";
    }
    else if(language == "br")
    {
        if(modifier=="euro" || codeset=="iso885915") localeEncoding = "885915";
        else if(codeset=="iso885914") localeEncoding = "885914";
        else localeEncoding = "88591";
    }
    else if(language == "ca")
    {
        if(modifier=="euro" || codeset=="iso885915") localeEncoding = "885915";
        else localeEncoding = "88591";
    }
    else if(language == "cs") localeEncoding = "88592";
    else if(language == "cy")
    {
        if(modifier=="euro" || codeset=="iso885915") localeEncoding = "885915";
        else if(codeset=="iso885914") localeEncoding = "885914";
        else localeEncoding = "88591";
    }
    else if(language == "da")
    {
        if(modifier=="euro" || codeset=="iso885915") localeEncoding = "885915";
        else localeEncoding = "88591";
    }
    else if(language == "de")
    {
        if(modifier=="euro" || codeset=="iso885915") localeEncoding = "885915";
        else localeEncoding = "88591";
    }
    else if(language == "ee") localeEncoding = "88594";
    else if(language == "el")
    {
        if(modifier=="euro") localeEncoding = "885915";
        else localeEncoding = "88597";
    }
    else if(language == "en")
    {
        if(modifier=="euro" || codeset=="iso885915") localeEncoding = "885915";
        else localeEncoding = "88591";
    }
    else if(language == "eo") localeEncoding = "88593";
    else if(language == "es")
    {
        if(modifier=="euro" || codeset=="iso885915") localeEncoding = "885915";
        else localeEncoding = "88591";
    }
    else if(language == "et")
    {
        if(codeset=="iso88591") localeEncoding = "88591";
        else if(codeset=="iso885913") localeEncoding = "885913";
        else if(codeset=="iso88594") localeEncoding = "88594";
        else localeEncoding = "885915";
    }
    else if(language == "eu")
    {
        if(modifier=="euro" || codeset=="iso885915") localeEncoding = "885915";
        else localeEncoding = "88591";
    }
    else if(language == "fi")
    {
        if(codeset=="88591" || codeset=="iso88591") localeEncoding = "88591";
        else localeEncoding = "885915";
    }
    else if(language == "fo")
    {
        if(modifier=="euro" || codeset=="iso885915") localeEncoding = "885915";
        else localeEncoding = "88591";
    }
    else if(language == "fr")
    {
        if(modifier=="euro" || codeset=="iso885915") localeEncoding = "885915";
        else localeEncoding = "88591";
    }
    else if(language == "fre") localeEncoding = "88591";
    else if(language == "ga")
    {
        if(modifier=="euro" || codeset=="iso885915") localeEncoding = "885915";
        else if(codeset=="iso885914") localeEncoding = "885914";
        else localeEncoding = "88591";
    }
    else if(language == "gd")
    {
        if(modifier=="euro" || codeset=="iso885915") localeEncoding = "885915";
        else if(codeset=="iso885914") localeEncoding = "885914";
        else localeEncoding = "88591";
    }
    else if(language == "ger") localeEncoding = "88591";
    else if(language == "gl")
    {
        if(modifier=="euro" || codeset=="iso885915") localeEncoding = "885915";
        else localeEncoding = "88591";
    }
    else if(language == "gv")
    {
        if(modifier=="euro" || codeset=="iso885915") localeEncoding = "885915";
        else if(codeset=="iso885914") localeEncoding = "885914";
        else localeEncoding = "88591";
    }
    else if(language == "he")
    {
        if(codeset=="cp1255" || codeset=="microsoftcp1255") localeEncoding = "CP1255";
        else localeEncoding = "88598";
    }
    else if(language == "hr") localeEncoding = "88592";
    else if(language == "hu") localeEncoding = "88592";
    else if(language == "id") localeEncoding = "88591";
    else if(language == "in") localeEncoding = "88591";
    else if(language == "is")
    {
        if(modifier=="euro" || codeset=="iso885915") localeEncoding = "885915";
        else localeEncoding = "88591";
    }
    else if(language == "it")
    {
        if(modifier=="euro" || codeset=="iso885915") localeEncoding = "885915";
        else localeEncoding = "88591";
    }
    else if(language == "iw") localeEncoding = "88598";
    else if(language == "kl")
    {
        if(modifier=="euro" || codeset=="iso885915") localeEncoding = "885915";
        else localeEncoding = "88591";
    }
    else if(language == "kw")
    {
        if(modifier=="euro" || codeset=="iso885915") localeEncoding = "885915";
        else if(codeset=="iso885914") localeEncoding = "885914";
        else localeEncoding = "88591";
    }
    else if(language == "lt")
    {
        if(codeset=="iso88594") localeEncoding = "88594";
        else localeEncoding = "885913";
    }
    else if(language == "lv")
    {
        if(codeset=="iso88594") localeEncoding = "88594";
        else localeEncoding = "885913";
    }
    else if(language == "mi") localeEncoding = "88591";
    else if(language == "mk")
    {
        if(codeset=="cp1251" || codeset=="microsoftc1251") localeEncoding = "CP1251";
        else localeEncoding = "88595";
    }
    else if(language == "ms") localeEncoding = "88591";
    else if(language == "mt") localeEncoding = "88593";
    else if(language == "nb")
    {
        if(modifier=="euro" || codeset=="iso885915") localeEncoding = "885915";
        else localeEncoding = "88591";
    }
    else if(language == "nl")
    {
        if(modifier=="euro" || codeset=="iso885915") localeEncoding = "885915";
        else localeEncoding = "88591";
    }
    else if(language == "nn")
    {
        if(modifier=="euro" || codeset=="iso885915") localeEncoding = "885915";
        else localeEncoding = "88591";
    }
    else if(language == "no")
    {
        if(modifier=="euro" || codeset=="iso885915") localeEncoding = "885915";
        else localeEncoding = "88591";
    }
    else if(language == "ny")
    {
        if(modifier=="euro" || codeset=="iso885915") localeEncoding = "885915";
        else localeEncoding = "88591";
    }
    else if(language == "oc")
    {
        if(modifier=="euro" || codeset=="iso885915") localeEncoding = "885915";
        else localeEncoding = "88591";
    }
    else if(language == "pd")
    {
        if(modifier=="euro" || codeset=="iso885915") localeEncoding = "885915";
        else localeEncoding = "88591";
    }
    else if(language == "ph") localeEncoding = "88591";
    else if(language == "pl") localeEncoding = "88592";
    else if(language == "pp") localeEncoding = "88591";
    else if(language == "pt")
    {
        if(modifier=="euro" || codeset=="iso885915") localeEncoding = "885915";
        else localeEncoding = "88591";
    }
    else if(language == "ro") localeEncoding = "88592";
    else if(language == "ru")
    {
        if(codeset=="koi8r" || territory=="ua") localeEncoding = "KOI8R";
        else if(codeset=="cp1251" || codeset=="microsoftcp1251") localeEncoding = "CP1251";
        else localeEncoding = "88595";
    }
    else if(language == "sh") localeEncoding = "88592";
    else if(language == "sk") localeEncoding = "88592";
    else if(language == "sl") localeEncoding = "88592";
    else if(language == "sp") localeEncoding = "88595";
    else if(language == "sq") localeEncoding = "88592";
    else if(language == "sr")
    {
        if(codeset=="iso88592" || territory=="sp") localeEncoding = "88592";
        else if(codeset=="cp1251" || codeset=="microsoftcp1251") localeEncoding = "CP1251";
        else localeEncoding = "88595";
    }
    else if(language == "sv")
    {
        if(modifier=="euro" || codeset=="iso885915") localeEncoding = "885915";
        else localeEncoding = "88591";
    }
    else if(language == "th") localeEncoding = "885911";
    else if(language == "tl") localeEncoding = "88591";
    else if(language == "tr") localeEncoding = "88599";
    else if(language == "tt") localeEncoding = "KOI8R";
    else if(language == "uk")
    {
        if(codeset=="cp1251" || codeset=="microsoftcp1251") localeEncoding = "CP1251";
        else if(codeset=="koi8u") localeEncoding = "KOI8-U";
        else localeEncoding = "KOI8R";
    }
    else if(language == "ur") localeEncoding = "CP1256";
    else if(language == "wa")
    {
        if(modifier=="euro" || codeset=="iso885915") localeEncoding = "885915";
        else localeEncoding = "88591";
    }
    else if(language == "yi") localeEncoding = "CP1255";
    else localeEncoding = "88591";

#endif
}

int main(int argc,char *argv[])
{
    FXbool loadIcon;

#if ENABLE_NLS
    bindtextdomain(PACKAGE, LOCALEDIR);
    bind_textdomain_codeset(PACKAGE, "utf-8");
    textdomain(PACKAGE);
#endif

    FXApp app(PACKAGE, FXString::null);
    app.reg().setAsciiMode(true);
    app.init(argc,argv);
    loadIcon = MakeAllIcons(&app);
    new dxirc(&app);
    app.create();
    return app.run();
}
