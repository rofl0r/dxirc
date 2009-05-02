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
#include "aliasdialog.h"
#include "utils.h"

FXDEFMAP(dxirc) dxircMap[] = {
    FXMAPFUNC(SEL_CLOSE,    0,                          dxirc::OnCommandQuit),
    FXMAPFUNC(SEL_COMMAND,  dxirc::ID_QUIT,             dxirc::OnCommandQuit),
    FXMAPFUNC(SEL_COMMAND,  dxirc::ID_ABOUT,            dxirc::OnCommandAbout),
    FXMAPFUNC(SEL_COMMAND,  dxirc::ID_CONNECT,          dxirc::OnCommandConnect),    
    FXMAPFUNC(SEL_COMMAND,  dxirc::ID_DISCONNECT,       dxirc::OnCommandDisconnect),
    FXMAPFUNC(SEL_COMMAND,  dxirc::ID_SERVERS,          dxirc::OnCommandServers),    
    FXMAPFUNC(SEL_COMMAND,  dxirc::ID_TABS,             dxirc::OnTabBook),
    FXMAPFUNC(SEL_COMMAND,  dxirc::ID_CLEAR,            dxirc::OnCommandClear),
    FXMAPFUNC(SEL_COMMAND,  dxirc::ID_CLEARALL,         dxirc::OnCommandClearAll),
    FXMAPFUNC(SEL_COMMAND,  dxirc::ID_CLOSETAB,         dxirc::OnCommandCloseTab),    
    FXMAPFUNC(SEL_COMMAND,  dxirc::ID_USERS,            dxirc::OnCommandUsers),
    FXMAPFUNC(SEL_COMMAND,  dxirc::ID_HELP,             dxirc::OnCommandHelp),
    FXMAPFUNC(SEL_COMMAND,  dxirc::ID_OPTIONS,          dxirc::OnCommandOptions),
    FXMAPFUNC(SEL_KEYPRESS, dxirc::ID_SELECTTAB,        dxirc::OnCommandSelectTab),
    FXMAPFUNC(SEL_KEYPRESS, dxirc::ID_NEXTTAB,          dxirc::OnCommandNextTab),
    FXMAPFUNC(SEL_KEYPRESS, dxirc::ID_NEXTUNREAD,       dxirc::OnCommandNextUnread),
    FXMAPFUNC(SEL_COMMAND,  dxirc::ID_ALIAS,            dxirc::OnCommandAlias),
    FXMAPFUNC(SEL_COMMAND,  dxirc::ID_TRAY,             dxirc::OnTrayClicked),
    FXMAPFUNC(SEL_COMMAND,  dxirc::ID_TCANCEL,          dxirc::OnTrayCancel),
    FXMAPFUNC(SEL_COMMAND,  IrcSocket::ID_SERVER,       dxirc::OnIrcEvent),
    FXMAPFUNC(SEL_COMMAND,  IrcTabItem::ID_CDIALOG,     dxirc::OnCommandConnect),
    FXMAPFUNC(SEL_COMMAND,  IrcTabItem::ID_CSERVER,     dxirc::OnTabConnect),
    FXMAPFUNC(SEL_COMMAND,  IrcTabItem::ID_TABQUIT,     dxirc::OnCommandDisconnect),
    FXMAPFUNC(SEL_COMMAND,  IrcTabItem::ID_NEWMSG,      dxirc::OnNewMsg)
};

FXIMPLEMENT(dxirc, FXMainWindow, dxircMap, ARRAYNUMBER(dxircMap))

dxirc::dxirc(FXApp *app)
    : FXMainWindow(app, PACKAGE, 0, 0, DECOR_ALL, 0, 0, 800, 600), app(app), trayIcon(NULL)
{
    setIcon(bigicon);
    setMiniIcon(smallicon);

    ircFont = NULL;

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
    users = new FXMenuCheck(editmenu, _("Users list\tCtrl-U\tShow/Hide users list"), this, ID_USERS);
    users->setCheck(usersShown);
    new FXMenuCommand(editmenu, _("&Aliases"), NULL, this, ID_ALIAS);
    new FXMenuCommand(editmenu, _("&Preferences"), optionicon, this, ID_OPTIONS);
    new FXMenuTitle(menubar, _("&Edit"), NULL, editmenu);

    helpmenu = new FXMenuPane(this);
    new FXMenuCommand(helpmenu, _("&Help\tF1"), helpicon, this, ID_HELP);
    new FXMenuCommand(helpmenu, _("&About..."), NULL, this, ID_ABOUT);
    new FXMenuTitle(menubar, _("&Help"), NULL, helpmenu);

    statusbar = new FXStatusBar(this, LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X);

    mainframe = new FXVerticalFrame(this, LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 1,1,1,1);

    tabbook = new FXTabBook(mainframe, this, ID_TABS, PACK_UNIFORM_WIDTH|PACK_UNIFORM_HEIGHT|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_RIGHT);
    tabbook->setTabStyle(TABBOOK_BOTTOMTABS);
    FXuint packing = tabbook->getPackingHints();
    packing &= ~PACK_UNIFORM_WIDTH;
    tabbook->setPackingHints(packing);

    IrcSocket *server = new IrcSocket(app, this, "", "");
    server->SetUsersList(usersList);
    servers.append(server);

    IrcTabItem *tabitem = new IrcTabItem(tabbook, "(server)", servericon, TAB_BOTTOM, SERVER, server, ownServerWindow, usersShown, logging, commandsList, logPath, maxAway, colors, nickCompletionChar, ircFont, sameCmd, sameList);
    server->AppendTarget(tabitem);

    if(useTray)
    {
        trayIcon = new FXTrayIcon(app, "dxirc", trayicon, 0, this, ID_TRAY, TRAY_CMD_ON_LEFT|TRAY_MENU_ON_RIGHT);
        traymenu = new FXPopup(trayIcon);
        new FXMenuCommand(traymenu, _("&Cancel"), NULL, this, ID_TCANCEL);
        new FXMenuSeparator(traymenu);
        new FXMenuCommand(traymenu, _("&Quit"), quiticon, this, ID_QUIT);
        trayIcon->setMenu(traymenu);
    }

    new FXToolTip(app,0);

    UpdateTheme();
    UpdateFont(fontSpec);
    UpdateTabs();

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
    delete trayicon;
    delete newm;
    delete unewm;
    delete chnewm;
    delete servermenu;
    delete editmenu;
    delete helpmenu;
    delete ircFont;
}

void dxirc::create()
{
    FXMainWindow::create();
    ReadServersConfig();
    show();
}

int CompareTabs(const void **a,const void **b)
{
    IrcTabItem *fa = (IrcTabItem*)*a;
    IrcTabItem *fb = (IrcTabItem*)*b;
    return comparecase((fa->GetType() == SERVER ? fa->GetServerName() : fa->GetServerName()+fa->getText()), (fb->GetType() == SERVER ? fb->GetServerName() : fb->GetServerName()+fb->getText()));
}

void dxirc::ReadConfig()
{
    FXString ircfontspec;
    FXSettings set;
    set.parseFile(utils::GetIniFile(), true);
    FXint xx=set.readIntEntry("SETTINGS","x",50);
    FXint yy=set.readIntEntry("SETTINGS","y",50);
    FXint ww=set.readIntEntry("SETTINGS","w",400);
    FXint hh=set.readIntEntry("SETTINGS","h",300);
    appTheme.base = set.readColorEntry("SETTINGS", "basecolor", getApp()->getBaseColor());
    appTheme.back = set.readColorEntry("SETTINGS", "backcolor", getApp()->getBackColor());
    appTheme.border = set.readColorEntry("SETTINGS", "bordercolor", getApp()->getBorderColor());
    appTheme.fore = set.readColorEntry("SETTINGS", "forecolor", getApp()->getForeColor());
    appTheme.menuback = set.readColorEntry("SETTINGS", "selmenubackcolor", getApp()->getSelMenuBackColor());
    appTheme.menufore = set.readColorEntry("SETTINGS", "selmenutextcolor", getApp()->getSelMenuTextColor());
    appTheme.selback = set.readColorEntry("SETTINGS", "selbackcolor", getApp()->getSelbackColor());
    appTheme.selfore = set.readColorEntry("SETTINGS", "selforecolor", getApp()->getSelforeColor());
    appTheme.tipback = set.readColorEntry("SETTINGS", "tipbackcolor", getApp()->getTipbackColor());
    appTheme.tipfore = set.readColorEntry("SETTINGS", "tipforecolor", getApp()->getTipforeColor());
    appTheme.hilite = set.readColorEntry("SETTINGS", "hilitecolor", getApp()->getHiliteColor());
    appTheme.shadow = set.readColorEntry("SETTINGS", "shadowcolor", getApp()->getShadowColor());
    fontSpec = set.readStringEntry("SETTINGS", "normalfont", getApp()->getNormalFont()->getFont().text());
    usersShown = set.readBoolEntry("SETTINGS", "usersShown", true);
    commandsList = set.readStringEntry("SETTINGS", "commandsList");
    themePath = CheckThemePath(set.readStringEntry("SETTINGS", "themePath", DXIRC_DATADIR PATHSEPSTRING "icons" PATHSEPSTRING "default"));
    themesList = CheckThemesList(set.readStringEntry("SETTINGS", "themesList", FXString(themePath+";").text()));
    colors.text = set.readColorEntry("SETTINGS", "textColor", FXRGB(0,0,0));
    colors.back = set.readColorEntry("SETTINGS", "textBackColor", FXRGB(255,255,255));
    colors.user = set.readColorEntry("SETTINGS", "userColor", FXRGB(191,191,191));
    colors.action = set.readColorEntry("SETTINGS", "actionsColor", FXRGB(255,165,0));
    colors.notice = set.readColorEntry("SETTINGS", "noticeColor", FXRGB(0,0,255));
    colors.error = set.readColorEntry("SETTINGS", "errorColor", FXRGB(255,0,0));
    colors.hilight = set.readColorEntry("SETTINGS", "hilightColor", FXRGB(0,255,0));
    colors.link = set.readColorEntry("SETTINGS", "linkColor", FXRGB(0,0,255));
    ircfontspec = set.readStringEntry("SETTINGS", "ircFont", "");
    sameCmd = set.readBoolEntry("SETTINGS", "sameCmd", false);
    sameList = set.readBoolEntry("SETTINGS", "sameList", false);
    if(!ircfontspec.empty()) ircFont = new FXFont(getApp(), ircfontspec);
    else
    {
        getApp()->getNormalFont()->create();
        FXFontDesc fontdescription;
        getApp()->getNormalFont()->getFontDesc(fontdescription);
        ircFont = new FXFont(getApp(),fontdescription);
        ircFont->create();
    }
    maxAway = set.readIntEntry("SETTINGS", "maxAway", 200);
    logging = set.readBoolEntry("SETTINGS", "logging", false);
    ownServerWindow = set.readBoolEntry("SETTINGS", "serverWindow", true);
    useTray = set.readBoolEntry("SETTINGS", "tray", false);
    nickCompletionChar = FXString(set.readStringEntry("SETTINGS", "nickCompletionChar", ":")).left(1);
    tempServerWindow = ownServerWindow;
    logPath = set.readStringEntry("SETTINGS", "logPath");
    if(logging && !FXStat::exists(logPath)) logging = false;
    FXint usersNum = set.readIntEntry("USERS", "number", 0);
    if(usersNum)
    {
        for(FXint i=0; i<usersNum; i++)
        {
            IgnoreUser user;
            user.nick = set.readStringEntry(FXStringFormat("USER%d", i).text(), "nick", FXStringFormat("xxx%d", i).text());
            user.channel = set.readStringEntry(FXStringFormat("USER%d", i).text(), "channel", "all");
            user.server = set.readStringEntry(FXStringFormat("USER%d", i).text(), "server", "all");
            usersList.append(user);
        }
    }
    setX(xx);
    setY(yy);
    setWidth(ww);
    setHeight(hh);
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
    FXSettings set;
    set.parseFile(utils::GetIniFile(), true);
    FXint serversNum = set.readIntEntry("SERVERS", "number", 0);
    if(serversNum)
    {
        for(FXint i=0; i<serversNum; i++)
        {
            ServerInfo server;
            server.hostname = set.readStringEntry(FXStringFormat("SERVER%d", i).text(), "hostname", FXStringFormat("localhost%d", i).text());
            server.port = set.readIntEntry(FXStringFormat("SERVER%d", i).text(), "port", 6667);
            server.nick = set.readStringEntry(FXStringFormat("SERVER%d", i).text(), "nick", "xxx");
            server.realname = set.readStringEntry(FXStringFormat("SERVER%d", i).text(), "realname", "xxx");
            server.passwd = Decrypt(set.readStringEntry(FXStringFormat("SERVER%d", i).text(), "hes", ""));
            server.channels = set.readStringEntry(FXStringFormat("SERVER%d", i).text(), "channels", "");
            server.commands = set.readStringEntry(FXStringFormat("SERVER%d", i).text(), "commands", "");
            server.autoConnect = set.readBoolEntry(FXStringFormat("SERVER%d", i).text(), "autoconnect", false);
            server.useSsl = set.readBoolEntry(FXStringFormat("SERVER%d", i).text(), "ssl", false);
            if(server.autoConnect)
            {
                ConnectServer(server.hostname, server.port, server.passwd, server.nick, server.realname, server.channels, server.commands, server.useSsl);
                fxsleep(1000);
            }
            serverList.append(server);
        }
    }
}

void dxirc::SaveConfig()
{
    getApp()->reg().setModified(false);
    FXSettings set;
    //set.clear();
    set.writeIntEntry("SERVERS", "number", serverList.no());
    if(serverList.no())
    {
        for(FXint i=0; i<serverList.no(); i++)
        {
            set.writeStringEntry(FXStringFormat("SERVER%d", i).text(), "hostname", serverList[i].hostname.text());
            set.writeIntEntry(FXStringFormat("SERVER%d", i).text(), "port", serverList[i].port);
            set.writeStringEntry(FXStringFormat("SERVER%d", i).text(), "nick", serverList[i].nick.text());
            set.writeStringEntry(FXStringFormat("SERVER%d", i).text(), "realname", serverList[i].realname.text());
            set.writeStringEntry(FXStringFormat("SERVER%d", i).text(), "hes", Encrypt(serverList[i].passwd).text());
            set.writeStringEntry(FXStringFormat("SERVER%d", i).text(), "channels", serverList[i].channels.text());
            set.writeStringEntry(FXStringFormat("SERVER%d", i).text(), "commands", serverList[i].commands.text());
            set.writeBoolEntry(FXStringFormat("SERVER%d", i).text(), "autoconnect", serverList[i].autoConnect);
            set.writeBoolEntry(FXStringFormat("SERVER%d", i).text(), "ssl", serverList[i].useSsl);
        }
    }
    set.writeBoolEntry("SETTINGS", "usersShown", usersShown);
    set.writeStringEntry("SETTINGS", "commandsList", commandsList.text());
    set.writeStringEntry("SETTINGS", "themePath", themePath.text());
    set.writeStringEntry("SETTINGS", "themesList", themesList.text());
    set.writeColorEntry("SETTINGS", "textColor", colors.text);
    set.writeColorEntry("SETTINGS", "textBackColor", colors.back);
    set.writeColorEntry("SETTINGS", "userColor", colors.user);
    set.writeColorEntry("SETTINGS", "actionsColor", colors.action);
    set.writeColorEntry("SETTINGS", "noticeColor", colors.notice);
    set.writeColorEntry("SETTINGS", "errorColor", colors.error);
    set.writeColorEntry("SETTINGS", "hilightColor", colors.hilight);
    set.writeColorEntry("SETTINGS", "linkColor", colors.link);
    set.writeStringEntry("SETTINGS", "ircFont", ircFont->getFont().text());
    set.writeIntEntry("SETTINGS", "maxAway", maxAway);
    set.writeBoolEntry("SETTINGS", "logging", logging);
    set.writeBoolEntry("SETTINGS", "sameCmd", sameCmd);
    set.writeBoolEntry("SETTINGS", "sameList", sameList);
    set.writeBoolEntry("SETTINGS", "tray", useTray);
    if(ownServerWindow == tempServerWindow) set.writeBoolEntry("SETTINGS", "serverWindow", ownServerWindow);
    else set.writeBoolEntry("SETTINGS", "serverWindow", tempServerWindow);
    set.writeStringEntry("SETTINGS", "logPath", logPath.text());
    set.writeStringEntry("SETTINGS", "nickCompletionChar", nickCompletionChar.text());
    set.writeIntEntry("USERS", "number", usersList.no());
    if(usersList.no())
    {

        for(FXint i=0; i<usersList.no(); i++)
        {
            set.writeStringEntry(FXStringFormat("USER%d", i).text(), "nick", usersList[i].nick.text());
            set.writeStringEntry(FXStringFormat("USER%d", i).text(), "channel", usersList[i].channel.text());
            set.writeStringEntry(FXStringFormat("USER%d", i).text(), "server", usersList[i].server.text());
        }
    }
    set.writeIntEntry("SETTINGS","x",getX());
    set.writeIntEntry("SETTINGS","y",getY());
    set.writeIntEntry("SETTINGS","w",getWidth());
    set.writeIntEntry("SETTINGS","h",getHeight());
    set.writeColorEntry("SETTINGS", "basecolor", appTheme.base);
    set.writeColorEntry("SETTINGS", "bordercolor", appTheme.border);
    set.writeColorEntry("SETTINGS", "backcolor", appTheme.back);
    set.writeColorEntry("SETTINGS", "forecolor", appTheme.fore);
    set.writeColorEntry("SETTINGS", "hilitecolor", appTheme.hilite);
    set.writeColorEntry("SETTINGS", "shadowcolor", appTheme.shadow);
    set.writeColorEntry("SETTINGS", "selforecolor", appTheme.selfore);
    set.writeColorEntry("SETTINGS", "selbackcolor", appTheme.selback);
    set.writeColorEntry("SETTINGS", "tipforecolor", appTheme.tipfore);
    set.writeColorEntry("SETTINGS", "tipbackcolor", appTheme.tipback);
    set.writeColorEntry("SETTINGS", "selmenutextcolor", appTheme.menufore);
    set.writeColorEntry("SETTINGS", "selmenubackcolor", appTheme.menuback);
    set.writeStringEntry("SETTINGS", "normalfont", getApp()->getNormalFont()->getFont().text());
    dxStringMap aliases = utils::GetAliases();
    set.writeIntEntry("ALIASES", "number", (FXint)aliases.size());
    if((FXint)aliases.size())
    {
        StringIt it;
        FXint i=0;
        for(it=aliases.begin(); it!=aliases.end(); it++)
        {
            set.writeStringEntry("ALIASES", FXStringFormat("key%d", i).text(), (*it).first.text());
            set.writeStringEntry("ALIASES", FXStringFormat("value%d", i).text(), (*it).second.text());
            i++;
        }
    }
    set.setModified();
    set.unparseFile(utils::GetIniFile());
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
    usersShown = !usersShown;
    for (FXint i = 0; i<tabbook->numChildren(); i=i+2)
    {
        if(usersShown) ((IrcTabItem *)tabbook->childAtIndex(i))->ShowUsers();
        else ((IrcTabItem *)tabbook->childAtIndex(i))->HideUsers();
    }
    return 1;
}

long dxirc::OnCommandOptions(FXObject*, FXSelector, void*)
{
    ConfigDialog dialog(this, colors, commandsList, usersList, themePath, themesList, maxAway, logging, logPath, tempServerWindow, nickCompletionChar, ircFont->getFont(), sameCmd, sameList, appTheme, useTray);
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
        sameCmd = dialog.GetSameCmd();
        sameList = dialog.GetSameList();
        appTheme = dialog.GetTheme();
        useTray = dialog.GetUseTray();
        UpdateTheme();
        UpdateFont(dialog.GetFont());
        ircFont = new FXFont(getApp(), dialog.GetIrcFont());
        ircFont->create();
        UpdateTabs();
        for (FXint i = 0; i<servers.no(); i++)
        {
            servers[i]->SetUsersList(usersList);
        }
        SaveConfig();
    }
    return 1;
}

long dxirc::OnCommandAlias(FXObject*, FXSelector, void*)
{
    AliasDialog dialog(this);
    if(dialog.execute(PLACEMENT_CURSOR))
    {
        SaveConfig();
    }
    return 1;
}

void dxirc::UpdateTheme()
{
    register FXWindow *w = FXApp::instance()->getRootWindow();

    FX7Segment * sevensegment;
    FXTextField * textfield;
    FXIconList * iconlist;
    FXList * list;
    FXListBox * listbox;
    FXTreeList * treelist;
    FXComboBox * combobox;
    FXButton * button;
    FXFrame * frame;
    FXLabel * label;
    FXPopup * popup;
    FXMenuTitle * menutitle;
    FXMenuCheck * menucheck;
    FXMenuRadio * menuradio;
    FXMenuCaption * menucaption;
    FXMenuSeparator * menuseparator;
    FXText * text;
    FXFoldingList * foldinglist;
    FXMDIChild * mdichild;
    FXTable * table;
    FXDockTitle * docktitle;
    FXPacker * packer;
    FXHeader * header;
    FXGroupBox * groupbox;
    FXScrollBar * scrollbar;
    FXSlider * slider;
    FXStatusLine * statusline;
    FXDragCorner * dragcorner;
    FXRadioButton * radiobutton;
    FXCheckButton * checkbutton;
    FXToolTip * tooltip;
    FXImageFrame * imageframe;

    getApp()->setBaseColor(appTheme.base);
    getApp()->setBackColor(appTheme.back);
    getApp()->setBorderColor(appTheme.border);
    getApp()->setForeColor(appTheme.fore);
    getApp()->setSelMenuBackColor(appTheme.menuback);
    getApp()->setSelMenuTextColor(appTheme.menufore);
    getApp()->setSelbackColor(appTheme.selback);
    getApp()->setSelforeColor(appTheme.selfore);
    getApp()->setTipbackColor(appTheme.tipback);
    getApp()->setTipforeColor(appTheme.tipfore);
    getApp()->setHiliteColor(appTheme.hilite);
    getApp()->setShadowColor(appTheme.shadow);

    while (w)
    {
        w->setBackColor(appTheme.base);
        if ((frame = dynamic_cast<FXFrame*> (w)))
        {
            frame->setBaseColor(appTheme.base);
            frame->setBackColor(appTheme.base);
            frame->setShadowColor(appTheme.shadow);
            frame->setHiliteColor(appTheme.hilite);
            frame->setBorderColor(appTheme.border);
            if ((label = dynamic_cast<FXLabel*> (w)))
            {
                label->setTextColor(appTheme.fore);
                if ((button = dynamic_cast<FXButton*> (w)))
                {
                    if (dynamic_cast<FXListBox*> (button->getParent()))
                    {
                        w->setBackColor(appTheme.back);
                    }
                    else
                    {
                        w->setBackColor(appTheme.base);
                    }
                }
                else if ((checkbutton = dynamic_cast<FXCheckButton*> (w)))
                {
                    checkbutton->setCheckColor(appTheme.fore);
                    checkbutton->setBoxColor(appTheme.back);
                }
                else if ((radiobutton = dynamic_cast<FXRadioButton*> (w)))
                {
                    radiobutton->setRadioColor(appTheme.fore);
                    radiobutton->setDiskColor(appTheme.back);
                }
            }
            else if ((textfield = dynamic_cast<FXTextField*> (w)))
            {
                w->setBackColor(appTheme.back);
                textfield->setTextColor(appTheme.fore);
                textfield->setSelTextColor(appTheme.selfore);
                textfield->setSelBackColor(appTheme.selback);
            }
            else if ((docktitle = dynamic_cast<FXDockTitle*> (w)))
            {
                docktitle->setCaptionColor(appTheme.selfore);
                docktitle->setBackColor(appTheme.selback);
            }
            else if ((header = dynamic_cast<FXHeader*> (w)))
            {
                header->setTextColor(appTheme.fore);
            }
            else if ((statusline = dynamic_cast<FXStatusLine*> (w)))
            {
                statusline->setTextColor(appTheme.fore);
            }
            else if ((sevensegment = dynamic_cast<FX7Segment*> (w)))
            {
                sevensegment->setTextColor(appTheme.fore);
            }
            else if ((slider = dynamic_cast<FXSlider*> (w)))
            {
                slider->setSlotColor(appTheme.back);
            }
            else if ((imageframe = dynamic_cast<FXImageFrame*> (w)))
            {
                imageframe->setBackColor(appTheme.back); /// fixme, only for coverframe in mainwindow
            }
        }
        else if ((packer = dynamic_cast<FXPacker*> (w)))
        {
            packer->setBaseColor(appTheme.base);
            packer->setBackColor(appTheme.base);
            packer->setShadowColor(appTheme.shadow);
            packer->setHiliteColor(appTheme.hilite);
            packer->setBorderColor(appTheme.border);
            if ((combobox = dynamic_cast<FXComboBox*> (w)))
            {
                w->setBackColor(appTheme.back);
            }
            else if ((listbox = dynamic_cast<FXListBox*> (w)))
            {
                w->setBackColor(appTheme.back);
            }
            else if ((groupbox = dynamic_cast<FXGroupBox*> (w)))
            {
                groupbox->setTextColor(appTheme.fore);
            }
        }
        else if ((popup = dynamic_cast<FXPopup*> (w)))
        {
            popup->setBaseColor(appTheme.base);
            popup->setShadowColor(appTheme.shadow);
            popup->setHiliteColor(appTheme.hilite);
            popup->setBorderColor(appTheme.border);
        }
        else if ((menucaption = dynamic_cast<FXMenuCaption*> (w)))
        {
            w->setBackColor(appTheme.base);
            menucaption->setTextColor(appTheme.fore);
            menucaption->setSelTextColor(appTheme.menufore);
            menucaption->setSelBackColor(appTheme.menuback);
            menucaption->setShadowColor(appTheme.shadow);
            menucaption->setHiliteColor(appTheme.hilite);

            if ((menucheck = dynamic_cast<FXMenuCheck*> (w)))
            {
                menucheck->setBoxColor(appTheme.back);
            }
            else if ((menuradio = dynamic_cast<FXMenuRadio*> (w)))
            {
                menuradio->setRadioColor(appTheme.back);
            }
            else if ((menutitle = dynamic_cast<FXMenuTitle*> (w)))
            {
                menutitle->setTextColor(appTheme.fore);
                menutitle->setSelTextColor(appTheme.fore);
                menutitle->setSelBackColor(appTheme.base);
            }
        }
        else if ((menuseparator = dynamic_cast<FXMenuSeparator*> (w)))
        {
            menuseparator->setShadowColor(appTheme.shadow);
            menuseparator->setHiliteColor(appTheme.hilite);
        }
        else if ((scrollbar = dynamic_cast<FXScrollBar*> (w)))
        {
            scrollbar->setShadowColor(appTheme.shadow);
            scrollbar->setHiliteColor(appTheme.hilite);
            scrollbar->setBorderColor(appTheme.border);
            scrollbar->setArrowColor(appTheme.fore);
        }
        else if ((dragcorner = dynamic_cast<FXDragCorner*> (w)))
        {
            dragcorner->setShadowColor(appTheme.shadow);
            dragcorner->setHiliteColor(appTheme.hilite);
        }
        else if (dynamic_cast<FXScrollArea*> (w))
        {
            if ((text = dynamic_cast<FXText*> (w)))
            {
                w->setBackColor(appTheme.back);
                text->setTextColor(appTheme.fore);
                text->setSelTextColor(appTheme.selfore);
                text->setSelBackColor(appTheme.selback);
            }
            else if ((list = dynamic_cast<FXList*> (w)))
            {
                w->setBackColor(appTheme.back);
                list->setTextColor(appTheme.fore);
                list->setSelTextColor(appTheme.selfore);
                list->setSelBackColor(appTheme.selback);
            }
            else if ((treelist = dynamic_cast<FXTreeList*> (w)))
            {
                w->setBackColor(appTheme.back);
                treelist->setTextColor(appTheme.fore);
                treelist->setLineColor(appTheme.shadow);
                treelist->setSelTextColor(appTheme.selfore);
                treelist->setSelBackColor(appTheme.selback);
            }
            else if ((iconlist = dynamic_cast<FXIconList*> (w)))
            {
                w->setBackColor(appTheme.back);
                iconlist->setTextColor(appTheme.fore);
                iconlist->setSelTextColor(appTheme.selfore);
                iconlist->setSelBackColor(appTheme.selback);
            }
            else if ((foldinglist = dynamic_cast<FXFoldingList*> (w)))
            {
                w->setBackColor(appTheme.back);
                foldinglist->setTextColor(appTheme.fore);
                foldinglist->setSelTextColor(appTheme.selfore);
                foldinglist->setSelBackColor(appTheme.selback);
                foldinglist->setLineColor(appTheme.shadow);
            }
            else if ((table = dynamic_cast<FXTable*> (w)))
            {
                w->setBackColor(appTheme.back);
                table->setTextColor(appTheme.fore);
                table->setSelTextColor(appTheme.selfore);
                table->setSelBackColor(appTheme.selback);
            }
        }
        else if ((mdichild = dynamic_cast<FXMDIChild*> (w)))
        {
            mdichild->setBackColor(appTheme.base);
            mdichild->setBaseColor(appTheme.base);
            mdichild->setShadowColor(appTheme.shadow);
            mdichild->setHiliteColor(appTheme.hilite);
            mdichild->setBorderColor(appTheme.border);
            mdichild->setTitleColor(appTheme.selfore);
            mdichild->setTitleBackColor(appTheme.selback);
        }
        else if ((tooltip = dynamic_cast<FXToolTip*> (w)))
        {
            tooltip->setTextColor(appTheme.tipfore);
            tooltip->setBackColor(appTheme.tipback);
        }

        w->update();
        if (w->getFirst())
        {
            w = w->getFirst();
            continue;
        }
        while (!w->getNext() && w->getParent())
        {
            w = w->getParent();
        }
        w = w->getNext();
    }
}

void dxirc::UpdateFont(FXString fnt)
{
    getApp()->getNormalFont()->destroy();
    getApp()->getNormalFont()->setFont(fnt);
    getApp()->getNormalFont()->create();
    register FXWindow *w = this;
    while(w)
    {
        w->recalc();
        w->update();
        if(w->getFirst())
        {
            w = w->getFirst();
            continue;
        }
        while(!w->getNext() && w->getParent())
        {
            w = w->getParent();
        }
        w = w->getNext();
    }
}

void dxirc::UpdateTabs()
{
    for (FXint i = 0; i<tabbook->numChildren(); i=i+2)
    {
        ((IrcTabItem *)tabbook->childAtIndex(i))->SetColor(colors);
        ((IrcTabItem *)tabbook->childAtIndex(i))->SetCommandsList(commandsList);
        ((IrcTabItem *)tabbook->childAtIndex(i))->SetMaxAway(maxAway);
        ((IrcTabItem *)tabbook->childAtIndex(i))->SetLogging(logging);
        ((IrcTabItem *)tabbook->childAtIndex(i))->SetLogPath(logPath);
        ((IrcTabItem *)tabbook->childAtIndex(i))->SetNickCompletionChar(nickCompletionChar);
        ((IrcTabItem *)tabbook->childAtIndex(i))->SetSameCmd(sameCmd);
        ((IrcTabItem *)tabbook->childAtIndex(i))->SetSameList(sameList);
        ((IrcTabItem *)tabbook->childAtIndex(i))->SetIrcFont(ircFont);
    }
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
        if (indexJoin != -1 && !ServerExist(serverList[indexJoin].hostname, serverList[indexJoin].port, serverList[indexJoin].nick))
        {
            ConnectServer(serverList[indexJoin].hostname, serverList[indexJoin].port, serverList[indexJoin].passwd, serverList[indexJoin].nick, serverList[indexJoin].realname, serverList[indexJoin].channels, serverList[indexJoin].commands, serverList[indexJoin].useSsl);
        }
        SaveConfig();
    }
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

    new FXLabel(matrix, _("Realname:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField* realname = new FXTextField(matrix, 25, NULL, 0, TEXTFIELD_ENTER_ONLY|FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);

    new FXLabel(matrix, _("Channel(s):\tChannels need to be comma separated"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *channel = new FXTextField(matrix, 25, NULL, 0, TEXTFIELD_ENTER_ONLY|FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    channel->setText("#");

#ifdef HAVE_OPENSSL
    new FXLabel(matrix, _("Use SSL:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXCheckButton *ussl = new FXCheckButton(matrix, "", NULL, 0);
#endif

    new FXLabel(matrix, _("Commands on connection:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXHorizontalFrame *commandsbox=new FXHorizontalFrame(matrix, LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|FRAME_SUNKEN|FRAME_THICK,0,0,0,0, 0,0,0,0);
    FXText *command = new FXText(commandsbox, NULL, 0, TEXT_WORDWRAP|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    command->setVisibleRows(4);
    command->setVisibleColumns(25);

    FXHorizontalFrame *buttonframe = new FXHorizontalFrame(contents,LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXButton(buttonframe, _("&OK"), NULL, &serverEdit, FXDialogBox::ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 32,32,5,5);
    new FXButton(buttonframe, _("Cancel"), NULL, &serverEdit, FXDialogBox::ID_CANCEL, FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 32,32,5,5);
    if (serverEdit.execute(PLACEMENT_OWNER))
    {
#ifdef HAVE_OPENSSL
        ConnectServer(hostname->getText(), port->getValue(), passwd->getText(), nick->getText(), realname->getText(), channel->getText(), command->getText(), ussl->getCheck());
#else
        ConnectServer(hostname->getText(), port->getValue(), passwd->getText(), nick->getText(), realname->getText(), channel->getText(), command->getText(), false);
#endif
    }
    return 1;
}

long dxirc::OnTabConnect(FXObject*, FXSelector, void *data)
{
    ServerInfo *srv = (ServerInfo*)data;
    ConnectServer(srv->hostname, srv->port, srv->passwd, srv->nick, srv->realname, srv->channels, "", false);
    return 1;
}

void dxirc::ConnectServer(FXString hostname, FXint port, FXString pass, FXString nick, FXString rname, FXString channels, FXString commands, FXbool ssl)
{
    if(servers.no() == 1 && !servers[0]->GetConnected() && !servers[0]->GetConnecting())
    {
        servers[0]->SetServerName(hostname);
        servers[0]->SetServerPort(port);
        servers[0]->SetServerPassword(pass);
        nick.length() ? servers[0]->SetNickName(nick) : servers[0]->SetNickName("_xxx_");
        nick.length() ? servers[0]->SetUserName(nick) : servers[0]->SetUserName("_xxx_");
        rname.length() ? servers[0]->SetRealName(rname) : servers[0]->SetRealName(nick.length() ? nick : "_xxx_");
        if(channels.length()>1) servers[0]->SetStartChannels(channels);
        if(commands.length()) servers[0]->SetStartCommands(commands);
#ifndef HAVE_OPENSSL
        ssl = false;
#endif
        servers[0]->SetUseSsl(ssl);
        if (!tabbook->numChildren())
        {
            IrcTabItem *tabitem = new IrcTabItem(tabbook, hostname, servericon, TAB_BOTTOM, SERVER, servers[0], ownServerWindow, usersShown, logging, commandsList, logPath, maxAway, colors, nickCompletionChar, ircFont, sameCmd, sameList);
            servers[0]->AppendTarget(tabitem);
            tabitem->create();
            tabitem->CreateGeom();
            //tabbook->setCurrent(tabbook->numChildren()/2);
        }
        ((IrcTabItem *)tabbook->childAtIndex(0))->SetType(SERVER, hostname);
        SortTabs();
        servers[0]->StartConnection();
    }
    else if(!ServerExist(hostname, port, nick))
    {
        IrcSocket *server = new IrcSocket(app, this, channels.length()>1 ? channels : "", commands.length() ? commands : "");
        server->SetUsersList(usersList);
        servers.prepend(server);
        servers[0]->SetServerName(hostname);
        servers[0]->SetServerPort(port);
        servers[0]->SetServerPassword(pass);
        nick.length() ? servers[0]->SetNickName(nick) : servers[0]->SetNickName("_xxx_");
        nick.length() ? servers[0]->SetUserName(nick) : servers[0]->SetUserName("_xxx_");
        rname.length() ? servers[0]->SetRealName(rname) : servers[0]->SetRealName(nick.length() ? nick : "_xxx_");
        IrcTabItem *tabitem = new IrcTabItem(tabbook, hostname, servericon, TAB_BOTTOM, SERVER, servers[0], ownServerWindow, usersShown, logging, commandsList, logPath, maxAway, colors, nickCompletionChar, ircFont, sameCmd, sameList);
        servers[0]->AppendTarget(tabitem);
#ifndef HAVE_OPENSSL
        ssl = false;
#endif
        servers[0]->SetUseSsl(ssl);
        tabitem->create();
        tabitem->CreateGeom();
        //tabbook->setCurrent(tabbook->numChildren()/2);
        SortTabs();
        servers[0]->StartConnection();
    }
    UpdateMenus();
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
            new FXLabel(contents, FXStringFormat(_("Disconnect server: %s\nPort: %d\nNick: %s?"), currentserver->GetServerName().text(), currentserver->GetServerPort(), currentserver->GetNickName().text()), NULL, JUSTIFY_LEFT|ICON_BEFORE_TEXT);
            FXHorizontalFrame* buttonframe = new FXHorizontalFrame(contents,LAYOUT_FILL_X|LAYOUT_FILL_Y);
            new FXButton(buttonframe, _("OK"), NULL, &confirmDialog, FXDialogBox::ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 32,32,5,5);
            new FXButton(buttonframe, _("Cancel"), NULL, &confirmDialog, FXDialogBox::ID_CANCEL, FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 32,32,5,5);
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

long dxirc::OnIrcEvent(FXObject *obj, FXSelector, void *data)
{
    IrcEvent *ev = (IrcEvent *)data;
    IrcSocket *server = (IrcSocket *)obj;
    if(ev->eventType == IRC_NEWCHANNEL)
    {
        FXint serverTabIndex = GetServerTab(server);
        if(serverTabIndex != -1 && !ownServerWindow)
        {
            ((IrcTabItem *)tabbook->childAtIndex(serverTabIndex))->SetType(CHANNEL, ev->param1);
        }
        else
        {
            IrcTabItem* tabitem = new IrcTabItem(tabbook, ev->param1, channelicon, TAB_BOTTOM, CHANNEL, server, ownServerWindow, usersShown, logging, commandsList, logPath, maxAway, colors, nickCompletionChar, ircFont, sameCmd, sameList);
            server->AppendTarget(tabitem);
            tabitem->create();
            tabitem->CreateGeom();
            //tabbook->setCurrent(tabbook->numChildren()/2);
        }
        SortTabs();
        UpdateMenus();
        return 1;
    }
    if(ev->eventType == IRC_QUERY && !TabExist(server, ev->param1))
    {
        FXint serverTabIndex = GetServerTab(server);
        if(serverTabIndex != -1 && !ownServerWindow)
        {
            ((IrcTabItem *)tabbook->childAtIndex(serverTabIndex))->SetType(QUERY, ev->param1);
        }
        else
        {
            IrcTabItem* tabitem = new IrcTabItem(tabbook, ev->param1, queryicon, TAB_BOTTOM, QUERY, server, ownServerWindow, usersShown, logging, commandsList, logPath, maxAway, colors, nickCompletionChar, ircFont, sameCmd, sameList);
            server->AppendTarget(tabitem);
            tabitem->create();
            tabitem->CreateGeom();
            //tabbook->setCurrent(tabbook->numChildren()/2);
        }
        SortTabs();
        UpdateMenus();
        return 1;
    }
    if(ev->eventType == IRC_PART)
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
                        if((comparecase(((IrcTabItem *)tabbook->childAtIndex(j))->getText(), ev->param2) == 0) && server->FindTarget((IrcTabItem *)tabbook->childAtIndex(j))) index = j;
                    }
                    server->RemoveTarget((IrcTabItem *)tabbook->childAtIndex(index));
                    delete tabbook->childAtIndex(index);
                    delete tabbook->childAtIndex(index);
                    tabbook->recalc();
                }
                SortTabs();
                UpdateMenus();
            }
        }
        return 1;
    }
    if(ev->eventType == IRC_KICK)
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
                    if((comparecase(((IrcTabItem *)tabbook->childAtIndex(j))->getText(), ev->param3) == 0) && server->FindTarget((IrcTabItem *)tabbook->childAtIndex(j))) index = j;
                }
                server->RemoveTarget((IrcTabItem *)tabbook->childAtIndex(index));
                delete tabbook->childAtIndex(index);
                delete tabbook->childAtIndex(index);
                tabbook->recalc();
            }
            SortTabs();
            UpdateMenus();
        }
        return 1;
    }
    if(ev->eventType == IRC_DISCONNECT)
    {
        for(FXint i = tabbook->numChildren()-2; i > -1; i=i-2)
        {
            if(server->FindTarget((IrcTabItem *)tabbook->childAtIndex(i)))
            {
                if(IsLastTab(server)) ((IrcTabItem *)tabbook->childAtIndex(i))->SetType(SERVER, server->GetServerName());
                else
                {
                    server->RemoveTarget((IrcTabItem *)tabbook->childAtIndex(i));
                    delete tabbook->childAtIndex(i);
                    delete tabbook->childAtIndex(i);
                    tabbook->recalc();
                }
            }
        }
        SortTabs();
        UpdateMenus();
        return 1;
    }
    if(ev->eventType == IRC_ENDMOTD)
    {
        UpdateMenus();
        return 1;
    }
    return 1;
}

long dxirc::OnTabBook(FXObject *, FXSelector, void *ptr)
{
    FXint index = (FXint)(FXival)ptr*2;
    IrcTabItem *currenttab = (IrcTabItem *)tabbook->childAtIndex(index);
    if (appTheme.fore != currenttab->getTextColor())
    {
        currenttab->setTextColor(appTheme.fore);
        if(currenttab->GetType() == CHANNEL) currenttab->setIcon(channelicon);
        if(currenttab->GetType() == QUERY) currenttab->setIcon(queryicon);
    }
    currenttab->setFocus();
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
            if (appTheme.fore != ((IrcTabItem *)tabbook->childAtIndex(i))->getTextColor())
            {
                tabbook->setCurrent(i/2, true);
                return 1;
            }
        }
        for (FXint i = tabbook->getCurrent()*2; i>-1; i=i-2)
        {
            if (appTheme.fore != ((IrcTabItem *)tabbook->childAtIndex(i))->getTextColor())
            {
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

long dxirc::OnCommandClearAll(FXObject *, FXSelector sel, void *)
{
    for (FXint i = 0; i<tabbook->numChildren(); i=i+2)
    {
        ((IrcTabItem *)tabbook->childAtIndex(i))->ClearChat();
        if (appTheme.fore != ((IrcTabItem *)tabbook->childAtIndex(i))->getTextColor())
        {
            ((IrcTabItem *)tabbook->childAtIndex(i))->setTextColor(appTheme.fore);
            if(((IrcTabItem *)tabbook->childAtIndex(i))->GetType() == CHANNEL) ((IrcTabItem *)tabbook->childAtIndex(i))->setIcon(channelicon);
            if(((IrcTabItem *)tabbook->childAtIndex(i))->GetType() == QUERY) ((IrcTabItem *)tabbook->childAtIndex(i))->setIcon(queryicon);
        }
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
            if(servers[i]->FindTarget(currenttab))
            {
                currentserver = servers[i];
                break;
            }
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

long dxirc::OnTrayClicked(FXObject*, FXSelector, void*)
{
    if(shown())
        hide();
    else
        show();
    if(trayIcon && trayIcon->getIcon() == newm)
        trayIcon->setIcon(trayicon);
    return 1;
}

long dxirc::OnTrayCancel(FXObject*, FXSelector, void*)
{
    traymenu->popdown();
    return 1;
}

long dxirc::OnNewMsg(FXObject*, FXSelector, void*)
{
    if(trayIcon && trayIcon->getIcon() == trayicon && !shown())
        trayIcon->setIcon(newm);
    return 1;
}

FXbool dxirc::TabExist(IrcSocket *server, FXString name)
{
    for(FXint i = 0; i < tabbook->numChildren(); i=i+2)
    {
        if(server->FindTarget((IrcTabItem *)tabbook->childAtIndex(i)) && comparecase(((IrcTabItem *)tabbook->childAtIndex(i))->getText(), name) == 0) return true;
    }
    return false;
}

FXbool dxirc::ServerExist(const FXString &server, const FXint &port, const FXString &nick)
{
    for(FXint i = 0; i < servers.no(); i++)
    {
        if(servers[i]->GetServerName() == server && servers[i]->GetServerPort() == port && servers[i]->GetNickName() == nick && servers[i]->GetConnected()) return true;
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
    FXString result = "";
    for(FXint i=0; i<text.count(); i++)
    {
        result += text[i];
        result += FXchar((rand()%126)+33);
    }
    return result;
}

FXString dxirc::Decrypt(const FXString &text)
{
    FXString result = "";
    for(FXint i=0; i<text.count(); i++)
    {
        if((i+1)%2) result += text[i];
    }
    return result;
}

#define USAGE_MSG _("\
\nUsage: dxirc [options] \n\
\n\
    [options] can be any of the following:\n\
\n\
        -h, --help         Print (this) help screen and exit.\n\
        -v, --version      Print version information and exit.\n\
        -l [FILE]          Load configuration from FILE.\n\
\n")

int main(int argc,char *argv[])
{
    FXbool loadIcon;    

#if ENABLE_NLS
    bindtextdomain(PACKAGE, LOCALEDIR);
    bind_textdomain_codeset(PACKAGE, "utf-8");
    textdomain(PACKAGE);
#endif

    for(FXint i=0; i<argc; ++i)
    {
        if(compare(argv[i],"-v")==0 || compare(argv[i],"--version")==0)
        {
            fprintf(stdout, "%s %s\n", PACKAGE, VERSION);
            exit(0);
        }
        if(compare(argv[i],"-h")==0 || compare(argv[i],"--help")==0)
        {
            fprintf(stdout, USAGE_MSG);
            exit(0);
        }
        if(compare(argv[i],"-l")==0)
        {
            utils::SetIniFile(argv[i+1]);
        }
    }

    FXTrayApp app(PACKAGE, FXString::null);
    app.reg().setAsciiMode(true);
    app.init(argc,argv);
    loadIcon = MakeAllIcons(&app, utils::GetIniFile());    
    new dxirc(&app);
    app.create();
    utils::SetAlias();
    return app.run();
}
