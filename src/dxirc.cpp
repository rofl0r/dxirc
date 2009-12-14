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
 #define WIN32_LEAN_AND_MEAN
 #include <windows.h>
#else
 #include <signal.h>
#endif

#include <string>
#include "dxirc.h"
#include "icons.h"
#include "i18n.h"
#include "help.h"
#include "irctabitem.h"
#include "configdialog.h"
#include "serverdialog.h"
#include "aliasdialog.h"
#include "utils.h"
#include "tetristabitem.h"
#include "scriptdialog.h"

FXDEFMAP(dxirc) dxircMap[] = {
    FXMAPFUNC(SEL_CLOSE,        0,                          dxirc::OnCommandClose),
    FXMAPFUNC(SEL_COMMAND,      dxirc::ID_QUIT,             dxirc::OnCommandQuit),
    FXMAPFUNC(SEL_COMMAND,      dxirc::ID_ABOUT,            dxirc::OnCommandAbout),
    FXMAPFUNC(SEL_COMMAND,      dxirc::ID_CONNECT,          dxirc::OnCommandConnect),
    FXMAPFUNC(SEL_COMMAND,      dxirc::ID_DISCONNECT,       dxirc::OnCommandDisconnect),
    FXMAPFUNC(SEL_COMMAND,      dxirc::ID_SERVERS,          dxirc::OnCommandServers),
    FXMAPFUNC(SEL_COMMAND,      dxirc::ID_TABS,             dxirc::OnTabBook),
    FXMAPFUNC(SEL_COMMAND,      dxirc::ID_CLEAR,            dxirc::OnCommandClear),
    FXMAPFUNC(SEL_COMMAND,      dxirc::ID_CLEARALL,         dxirc::OnCommandClearAll),
    FXMAPFUNC(SEL_COMMAND,      dxirc::ID_CLOSETAB,         dxirc::OnCommandCloseTab),
    FXMAPFUNC(SEL_COMMAND,      dxirc::ID_USERS,            dxirc::OnCommandUsers),
    FXMAPFUNC(SEL_COMMAND,      dxirc::ID_STATUS,           dxirc::OnCommandStatus),
    FXMAPFUNC(SEL_COMMAND,      dxirc::ID_HELP,             dxirc::OnCommandHelp),
    FXMAPFUNC(SEL_COMMAND,      dxirc::ID_OPTIONS,          dxirc::OnCommandOptions),
    FXMAPFUNC(SEL_COMMAND,      dxirc::ID_SELECTTAB,        dxirc::OnCommandSelectTab),
    FXMAPFUNC(SEL_COMMAND,      dxirc::ID_NEXTTAB,          dxirc::OnCommandNextTab),
    FXMAPFUNC(SEL_COMMAND,      IrcTabItem::ID_NEXTTAB,     dxirc::OnCommandNextTab),
    FXMAPFUNC(SEL_MOUSEWHEEL,   dxirc::ID_TABS,             dxirc::OnMouseWheel),
    FXMAPFUNC(SEL_COMMAND,      dxirc::ID_NEXTUNREAD,       dxirc::OnCommandNextUnread),
    FXMAPFUNC(SEL_COMMAND,      dxirc::ID_ALIAS,            dxirc::OnCommandAlias),
    FXMAPFUNC(SEL_COMMAND,      dxirc::ID_LOG,              dxirc::OnCommandLog),
    FXMAPFUNC(SEL_COMMAND,      dxirc::ID_TRANSFERS,        dxirc::OnCommandTransfers),
    FXMAPFUNC(SEL_COMMAND,      dxirc::ID_TRAY,             dxirc::OnTrayClicked),
    FXMAPFUNC(SEL_COMMAND,      dxirc::ID_SCRIPTS,          dxirc::OnCommandScripts),
    FXMAPFUNC(SEL_TIMEOUT,      dxirc::ID_STIMEOUT,         dxirc::OnStatusTimeout),
    FXMAPFUNC(SEL_COMMAND,      dxirc::ID_TETRIS,           dxirc::OnTetrisKey),
    FXMAPFUNC(SEL_COMMAND,      IrcSocket::ID_SERVER,       dxirc::OnIrcEvent),
    FXMAPFUNC(SEL_COMMAND,      IrcTabItem::ID_CDIALOG,     dxirc::OnCommandConnect),
    FXMAPFUNC(SEL_COMMAND,      IrcTabItem::ID_CSERVER,     dxirc::OnTabConnect),
    FXMAPFUNC(SEL_COMMAND,      IrcTabItem::ID_CQUIT,       dxirc::OnCommandQuit),
    FXMAPFUNC(SEL_COMMAND,      IrcTabItem::ID_NEWMSG,      dxirc::OnNewMsg),
    FXMAPFUNC(SEL_COMMAND,      IrcTabItem::ID_LUA,         dxirc::OnLua),
    FXMAPFUNC(SEL_COMMAND,      IrcTabItem::ID_COMMAND,     dxirc::OnIrcCommand),
    FXMAPFUNC(SEL_COMMAND,      IrcTabItem::ID_NEWTETRIS,   dxirc::OnNewTetris),
    FXMAPFUNC(SEL_COMMAND,      DccDialog::ID_DCCCANCEL,    dxirc::OnCommandDccCancel)
};

FXIMPLEMENT(dxirc, FXMainWindow, dxircMap, ARRAYNUMBER(dxircMap))

dxirc *dxirc::pThis = NULL;
#ifdef HAVE_LUA
static luaL_reg dxircFunctions[] = {
    {"AddCommand",  dxirc::OnLuaAddCommand},
    {"AddEvent",    dxirc::OnLuaAddEvent},
    {"AddAll",      dxirc::OnLuaAddAll},
    {"RemoveName",  dxirc::OnLuaRemoveName},
    {"Command",     dxirc::OnLuaCommand},
    {"Print",       dxirc::OnLuaPrint},
    {"GetServers",  dxirc::OnLuaGetServers},
    {"GetTab",      dxirc::OnLuaGetTab},
    {"GetTabInfo",  dxirc::OnLuaGetTabInfo},
    {"SetTab",      dxirc::OnLuaSetTab},
    {NULL,          NULL}
};
#endif

dxirc::dxirc(FXApp *app)
    : FXMainWindow(app, PACKAGE, 0, 0, DECOR_ALL, 0, 0, 800, 600), app(app), trayIcon(NULL)
{
    setIcon(bigicon);
    setMiniIcon(smallicon);

    ircFont = NULL;
    viewer = NULL;
    transfers = NULL;
    lastToken = 0;

    ReadConfig();

    menubar = new FXMenuBar(this, FRAME_RAISED|LAYOUT_SIDE_TOP|LAYOUT_FILL_X);

    servermenu = new FXMenuPane(this);
    new FXMenuCommand(servermenu, _("&Server list\tF2"), serverlisticon, this, ID_SERVERS);
    new FXMenuCommand(servermenu, _("Quick &connect\tCtrl-K"), connecticon, this, ID_CONNECT);
    disconnect = new FXMenuCommand(servermenu, _("&Disconnect\tCtrl-D"), disconnecticon, this, ID_DISCONNECT);
    disconnect->disable();
    new FXMenuSeparator(servermenu);
    new FXMenuCommand(servermenu, _("DCC &transfers\tCtrl-T"), NULL, this, ID_TRANSFERS);
#ifdef HAVE_LUA
    new FXMenuSeparator(servermenu);
    new FXMenuCommand(servermenu, _("S&cripts\tCtrl-S"), NULL, this, ID_SCRIPTS);
#endif
    new FXMenuSeparator(servermenu);
    logviewer = new FXMenuCommand(servermenu, _("&Log viewer\tCtrl-G"), NULL, this, ID_LOG);
    if(!logging) logviewer->disable();
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
    status = new FXMenuCheck(editmenu, _("Status bar"), this, ID_STATUS);
    status->setCheck(statusShown);
    new FXMenuCommand(editmenu, _("&Aliases"), NULL, this, ID_ALIAS);
    new FXMenuCommand(editmenu, _("&Preferences"), optionicon, this, ID_OPTIONS);
    new FXMenuTitle(menubar, _("&Edit"), NULL, editmenu);

    helpmenu = new FXMenuPane(this);
    new FXMenuCommand(helpmenu, _("&Help\tF1"), helpicon, this, ID_HELP);
    new FXMenuCommand(helpmenu, _("&About..."), NULL, this, ID_ABOUT);
    new FXMenuTitle(menubar, _("&Help"), NULL, helpmenu);

    mainframe = new FXVerticalFrame(this, LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 1,1,1,1);

    tabbook = new FXTabBook(mainframe, this, ID_TABS, PACK_UNIFORM_WIDTH|PACK_UNIFORM_HEIGHT|LAYOUT_FILL_X|LAYOUT_FILL_Y);    

    IrcSocket *server = new IrcSocket(app, this, "", "");
    server->SetUsersList(usersList);
    server->SetReconnect(reconnect);
    server->SetNumberAttempt(numberAttempt);
    server->SetDelayAttempt(delayAttempt);
    servers.append(server);

    IrcTabItem *tabitem = new IrcTabItem(tabbook, "(server)", servericon, TAB_BOTTOM, SERVER, server, ownServerWindow, usersShown, logging, commandsList, logPath, maxAway, colors, nickCompletionChar, ircFont, sameCmd, sameList, coloredNick);
    server->AppendTarget(tabitem);

    statusbar = new FXHorizontalFrame(mainframe, LAYOUT_LEFT|JUSTIFY_LEFT|LAYOUT_FILL_X|FRAME_NONE, 0,0,0,0, 1,1,1,1);
    FXHorizontalFrame *hframe=new FXHorizontalFrame(statusbar, LAYOUT_LEFT|JUSTIFY_LEFT|LAYOUT_FILL_X|FRAME_SUNKEN, 0,0,0,0, 0,0,0,0);
    statuslabel = new FXLabel(hframe, "dxirc", NULL, LAYOUT_LEFT|JUSTIFY_LEFT);
    if (statusShown)
        statusbar->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_SHOW), NULL);
    else
        statusbar->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_HIDE), NULL);

#ifdef HAVE_TRAY
    if(useTray)
    {
        trayIcon = new FXTrayIcon(app, "dxirc", trayicon, 0, this, ID_TRAY, TRAY_CMD_ON_LEFT|TRAY_MENU_ON_RIGHT);
        traymenu = new FXPopup(trayIcon);
        new FXMenuCommand(traymenu, _("&Quit"), quiticon, this, ID_QUIT);
        trayIcon->setMenu(traymenu);
    }
#endif

    new FXToolTip(app,0);

    UpdateTheme();
    UpdateFont(fontSpec);
    UpdateTabs();
    UpdateTabPosition();

    getAccelTable()->addAccel(MKUINT(KEY_1, ALTMASK), this, FXSEL(SEL_COMMAND, ID_SELECTTAB));
    getAccelTable()->addAccel(MKUINT(KEY_2, ALTMASK), this, FXSEL(SEL_COMMAND, ID_SELECTTAB));
    getAccelTable()->addAccel(MKUINT(KEY_3, ALTMASK), this, FXSEL(SEL_COMMAND, ID_SELECTTAB));
    getAccelTable()->addAccel(MKUINT(KEY_4, ALTMASK), this, FXSEL(SEL_COMMAND, ID_SELECTTAB));
    getAccelTable()->addAccel(MKUINT(KEY_5, ALTMASK), this, FXSEL(SEL_COMMAND, ID_SELECTTAB));
    getAccelTable()->addAccel(MKUINT(KEY_6, ALTMASK), this, FXSEL(SEL_COMMAND, ID_SELECTTAB));
    getAccelTable()->addAccel(MKUINT(KEY_7, ALTMASK), this, FXSEL(SEL_COMMAND, ID_SELECTTAB));
    getAccelTable()->addAccel(MKUINT(KEY_8, ALTMASK), this, FXSEL(SEL_COMMAND, ID_SELECTTAB));
    getAccelTable()->addAccel(MKUINT(KEY_9, ALTMASK), this, FXSEL(SEL_COMMAND, ID_SELECTTAB));
    getAccelTable()->addAccel(MKUINT(KEY_Tab, CONTROLMASK), this, FXSEL(SEL_COMMAND, ID_NEXTTAB));
    getAccelTable()->addAccel(MKUINT(KEY_n, CONTROLMASK), this, FXSEL(SEL_COMMAND, ID_NEXTUNREAD));
    getAccelTable()->addAccel(MKUINT(KEY_N, CONTROLMASK), this, FXSEL(SEL_COMMAND, ID_NEXTUNREAD));
    getAccelTable()->addAccel(KEY_n, this, FXSEL(SEL_COMMAND, ID_TETRIS));
    getAccelTable()->addAccel(KEY_N, this, FXSEL(SEL_COMMAND, ID_TETRIS));
    getAccelTable()->addAccel(KEY_p, this, FXSEL(SEL_COMMAND, ID_TETRIS));
    getAccelTable()->addAccel(KEY_P, this, FXSEL(SEL_COMMAND, ID_TETRIS));
    getAccelTable()->addAccel(KEY_KP_5, this, FXSEL(SEL_COMMAND, ID_TETRIS));
    getAccelTable()->addAccel(KEY_KP_3, this, FXSEL(SEL_COMMAND, ID_TETRIS));
    getAccelTable()->addAccel(KEY_KP_2, this, FXSEL(SEL_COMMAND, ID_TETRIS));
    getAccelTable()->addAccel(KEY_KP_1, this, FXSEL(SEL_COMMAND, ID_TETRIS));
}

dxirc::~dxirc()
{
    app->removeTimeout(this, ID_STIMEOUT);
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
    delete foldericon;
    delete ofoldericon;
    delete fileicon;
    delete cancelicon;
    delete finishicon;
    delete downicon;
    delete upicon;
    delete servermenu;
    delete editmenu;
    delete helpmenu;
    delete traymenu;
    delete ircFont;
    pThis = NULL;
}

void dxirc::create()
{
    FXMainWindow::create();
    //Checking for screen resolution and correction size, position
    FXint maxWidth = getRoot()->getWidth();
    FXint maxHeight = getRoot()->getHeight();
    if(getX()+getWidth() > maxWidth || getX()<0)
    {
        setX(0);
        setWidth(maxWidth);
    }
    if(getY()+getHeight() > maxHeight || getY()<0)
    {
        setY(0);
        setHeight(maxHeight);
    }    
    show();
    ReadServersConfig();
    pThis = this;
    AutoloadScripts();
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
    set.parseFile(utils::GetIniFile(), TRUE);
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
    usersShown = set.readBoolEntry("SETTINGS", "usersShown", TRUE);
    statusShown = set.readBoolEntry("SETTINGS", "statusShown", TRUE);
    tabPosition = set.readIntEntry("SETTINGS", "tabPosition", 0);
    commandsList = set.readStringEntry("SETTINGS", "commandsList");
    themePath = utils::CheckThemePath(set.readStringEntry("SETTINGS", "themePath", DXIRC_DATADIR PATHSEPSTRING "icons" PATHSEPSTRING "default"));
    themesList = utils::CheckThemesList(set.readStringEntry("SETTINGS", "themesList", FXString(themePath+";").text()));
    colors.text = set.readColorEntry("SETTINGS", "textColor", FXRGB(0,0,0));
    colors.back = set.readColorEntry("SETTINGS", "textBackColor", FXRGB(255,255,255));
    colors.user = set.readColorEntry("SETTINGS", "userColor", FXRGB(191,191,191));
    colors.action = set.readColorEntry("SETTINGS", "actionsColor", FXRGB(255,165,0));
    colors.notice = set.readColorEntry("SETTINGS", "noticeColor", FXRGB(0,0,255));
    colors.error = set.readColorEntry("SETTINGS", "errorColor", FXRGB(255,0,0));
    colors.hilight = set.readColorEntry("SETTINGS", "hilightColor", FXRGB(0,255,0));
    colors.link = set.readColorEntry("SETTINGS", "linkColor", FXRGB(0,0,255));
    ircfontspec = set.readStringEntry("SETTINGS", "ircFont", "");
    sameCmd = set.readBoolEntry("SETTINGS", "sameCmd", FALSE);
    sameList = set.readBoolEntry("SETTINGS", "sameList", FALSE);
    coloredNick = set.readBoolEntry("SETTINGS", "coloredNick", FALSE);
    if(!ircfontspec.empty())
    {
        ircFont = new FXFont(getApp(), ircfontspec);
        ircFont->create();
    }
    else
    {
        getApp()->getNormalFont()->create();
        FXFontDesc fontdescription;
        getApp()->getNormalFont()->getFontDesc(fontdescription);
        ircFont = new FXFont(getApp(),fontdescription);
        ircFont->create();
    }
    maxAway = set.readIntEntry("SETTINGS", "maxAway", 200);
    logging = set.readBoolEntry("SETTINGS", "logging", FALSE);
    ownServerWindow = set.readBoolEntry("SETTINGS", "serverWindow", TRUE);
#ifdef HAVE_TRAY
    useTray = set.readBoolEntry("SETTINGS", "tray", FALSE);
#else
    useTray = FALSE;
#endif
    if(useTray)
        closeToTray = set.readBoolEntry("SETTINGS", "closeToTray", FALSE);
    else
        closeToTray = FALSE;
    reconnect = set.readBoolEntry("SETTINGS", "reconnect", FALSE);
    numberAttempt = set.readIntEntry("SETTINGS", "numberAttempt", 1);
    delayAttempt = set.readIntEntry("SETTINGS", "delayAttempt", 20);
    nickCompletionChar = FXString(set.readStringEntry("SETTINGS", "nickCompletionChar", ":")).left(1);
    tempServerWindow = ownServerWindow;
    logPath = set.readStringEntry("SETTINGS", "logPath");
    if(logging && !FXStat::exists(logPath)) logging = FALSE;
    dccPath = set.readStringEntry("SETTINGS", "dccPath");
    if(!FXStat::exists(dccPath)) dccPath = FXSystem::getHomeDirectory();
    FXint usersNum = set.readIntEntry("USERS", "number", 0);
    usersList.clear();
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
#ifdef HAVE_LUA
    autoload = set.readBoolEntry("SETTINGS", "autoload", FALSE);
#else
    autoload = FALSE;
#endif
    autoloadPath = set.readStringEntry("SETTINGS", "autoloadPath");
    if(autoload && !FXStat::exists(utils::IsUtf8(autoloadPath.text(), autoloadPath.length()) ? autoloadPath : utils::LocaleToUtf8(autoloadPath))) autoload = FALSE;
    dccIP = set.readStringEntry("SETTINGS", "dccIP");
    FXRex rex("\\l");
    if(dccIP.contains('.')!=3 || rex.match(dccIP))
        dccIP = "";
    dccPortD = set.readIntEntry("SETTINGS", "dccPortD");
    dccPortH = set.readIntEntry("SETTINGS", "dccPortH");
    dccTimeout = set.readIntEntry("SETTINGS", "dccTimeout", 66);
    setX(xx);
    setY(yy);
    setWidth(ww);
    setHeight(hh);
}

void dxirc::ReadServersConfig()
{
    FXSettings set;
    set.parseFile(utils::GetIniFile(), TRUE);
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
            server.passwd = utils::Decrypt(set.readStringEntry(FXStringFormat("SERVER%d", i).text(), "hes", ""));
            server.channels = set.readStringEntry(FXStringFormat("SERVER%d", i).text(), "channels", "");
            server.commands = set.readStringEntry(FXStringFormat("SERVER%d", i).text(), "commands", "");
            server.autoConnect = set.readBoolEntry(FXStringFormat("SERVER%d", i).text(), "autoconnect", FALSE);
            server.useSsl = set.readBoolEntry(FXStringFormat("SERVER%d", i).text(), "ssl", FALSE);
            if(server.autoConnect)
            {
                ConnectServer(server.hostname, server.port, server.passwd, server.nick, server.realname, server.channels, server.commands, server.useSsl);
                fxsleep(500000);
            }
            serverList.append(server);
        }
    }
}

void dxirc::SaveConfig()
{
    getApp()->reg().setModified(FALSE);
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
            set.writeStringEntry(FXStringFormat("SERVER%d", i).text(), "hes", utils::Encrypt(serverList[i].passwd).text());
            set.writeStringEntry(FXStringFormat("SERVER%d", i).text(), "channels", serverList[i].channels.text());
            set.writeStringEntry(FXStringFormat("SERVER%d", i).text(), "commands", serverList[i].commands.text());
            set.writeBoolEntry(FXStringFormat("SERVER%d", i).text(), "autoconnect", serverList[i].autoConnect);
            set.writeBoolEntry(FXStringFormat("SERVER%d", i).text(), "ssl", serverList[i].useSsl);
        }
    }
    set.writeBoolEntry("SETTINGS", "usersShown", usersShown);
    set.writeBoolEntry("SETTINGS", "statusShown", statusShown);
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
    set.writeBoolEntry("SETTINGS", "coloredNick", coloredNick);
    set.writeBoolEntry("SETTINGS", "tray", useTray);
    set.writeBoolEntry("SETTINGS", "closeToTray", closeToTray);
    set.writeBoolEntry("SETTINGS", "reconnect", reconnect);
    set.writeIntEntry("SETTINGS", "numberAttempt", numberAttempt);
    set.writeIntEntry("SETTINGS", "delayAttempt", delayAttempt);
    if(ownServerWindow == tempServerWindow) set.writeBoolEntry("SETTINGS", "serverWindow", ownServerWindow);
    else set.writeBoolEntry("SETTINGS", "serverWindow", tempServerWindow);
    set.writeStringEntry("SETTINGS", "logPath", logPath.text());
    set.writeStringEntry("SETTINGS", "dccPath", dccPath.text());
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
    set.writeIntEntry("SETTINGS", "tabPosition", tabPosition);
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
    set.writeBoolEntry("SETTINGS", "autoload", autoload);
    set.writeStringEntry("SETTINGS", "autoloadPath", autoloadPath.text());
    set.writeStringEntry("SETTINGS", "dccIP", dccIP.text());
    set.writeIntEntry("SETTINGS", "dccPortD", dccPortD);
    set.writeIntEntry("SETTINGS", "dccPortH", dccPortH);
    set.writeIntEntry("SETTINGS", "dccTimeout", dccTimeout);
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
#ifdef HAVE_LUA
    while(scripts.no())
    {
        lua_close(scripts[0].L);
        scripts.erase(0);
    }
    while(scriptEvents.no())
    {
        scriptEvents.erase(0);
    }
#endif
    SaveConfig();
    getApp()->exit(0);
    return 1;
}

long dxirc::OnCommandClose(FXObject*, FXSelector, void*)
{
#ifdef HAVE_TRAY
    if(closeToTray)
        hide();
    else
        OnCommandQuit(NULL, 0, NULL);
#else
    OnCommandQuit(NULL, 0, NULL);
#endif
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

    new FXButton(contents, _("C&lose"), NULL, &helpDialog, FXDialogBox::ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 32,32,5,5);

    helpDialog.execute(PLACEMENT_CURSOR);
    return 1;
}

long dxirc::OnCommandUsers(FXObject*, FXSelector, void*)
{
    usersShown = !usersShown;
    for (FXint i = 0; i<tabbook->numChildren(); i=i+2)
    {        
        if(compare(tabbook->childAtIndex(i)->getClassName(), "IrcTabItem") == 0)
        {
            IrcTabItem *tab = static_cast<IrcTabItem*>(tabbook->childAtIndex(i));
            if(usersShown) tab->ShowUsers();
            else tab->HideUsers();
        }
    }
    return 1;
}

long dxirc::OnCommandStatus(FXObject*, FXSelector, void*)
{
    statusShown = !statusShown;
    statusbar->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_TOGGLESHOWN), NULL);
    return 1;
}

long dxirc::OnCommandOptions(FXObject*, FXSelector, void*)
{
    ConfigDialog dialog(this);
    if(dialog.execute(PLACEMENT_CURSOR))
    {
        ReadConfig();
        if(logging) logviewer->enable();
        else logviewer->disable();
        UpdateTheme();
        UpdateFont();
        UpdateTabs();
        UpdateTabPosition();
        for (FXint i = 0; i<servers.no(); i++)
        {
            servers[i]->SetUsersList(usersList);
            servers[i]->SetReconnect(reconnect);
            servers[i]->SetNumberAttempt(numberAttempt);
            servers[i]->SetDelayAttempt(delayAttempt);
        }
        recalc();
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

long dxirc::OnCommandLog(FXObject*, FXSelector, void*)
{
    if(viewer == NULL)
        viewer = new LogViewer(app, logPath, ircFont);
    viewer->create();
    return 1;
}

long dxirc::OnCommandTransfers(FXObject*, FXSelector, void*)
{
    if(transfers == NULL)
        transfers = new DccDialog(app, this);
    transfers->create();
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
                if(label->getTextColor() != FXRGB(255,0,0) && label->getTextColor() != FXRGB(0,0,255)) label->setTextColor(appTheme.fore);
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

void dxirc::UpdateFont()
{
    UpdateFont(fontSpec);
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
        if(compare(tabbook->childAtIndex(i)->getClassName(), "IrcTabItem") == 0)
        {
            IrcTabItem *irctab = static_cast<IrcTabItem*>(tabbook->childAtIndex(i));
            irctab->SetColor(colors);
            irctab->SetCommandsList(commandsList);
            irctab->SetMaxAway(maxAway);
            irctab->SetLogging(logging);
            irctab->SetLogPath(logPath);
            irctab->SetNickCompletionChar(nickCompletionChar);
            irctab->SetSameCmd(sameCmd);
            irctab->SetSameList(sameList);
            irctab->SetIrcFont(ircFont);
            irctab->SetColoredNick(coloredNick);
        }        
        if(compare(tabbook->childAtIndex(i)->getClassName(), "TetrisTabItem") == 0)
        {
            TetrisTabItem *tetristab = static_cast<TetrisTabItem*>(tabbook->childAtIndex(i));
            tetristab->SetColor(colors);
        }
    }
    //update font in LogViewer too. Both must be same
    if(viewer) viewer->SetFont(ircFont);
}

void dxirc::UpdateTabPosition()
{
    switch(tabPosition) {
        case 0: //bottom
            {
                tabbook->setTabStyle(TABBOOK_BOTTOMTABS);
                for(FXint i = 0; i<tabbook->numChildren(); i=i+2)
                {
                    static_cast<FXTabItem*>(tabbook->childAtIndex(i))->setTabOrientation(TAB_BOTTOM);
                }
                FXuint packing = tabbook->getPackingHints();
                packing &= ~PACK_UNIFORM_WIDTH;
                tabbook->setPackingHints(packing);
            }break;
        case 1: //left
            {
                tabbook->setTabStyle(TABBOOK_LEFTTABS);
                for(FXint i = 0; i<tabbook->numChildren(); i=i+2)
                {
                    static_cast<FXTabItem*>(tabbook->childAtIndex(i))->setTabOrientation(TAB_LEFT);
                }
                FXuint packing = tabbook->getPackingHints();
                packing |= PACK_UNIFORM_WIDTH;
                tabbook->setPackingHints(packing);
            }break;
        case 2: //top
            {
                tabbook->setTabStyle(TABBOOK_TOPTABS);
                for(FXint i = 0; i<tabbook->numChildren(); i=i+2)
                {
                    static_cast<FXTabItem*>(tabbook->childAtIndex(i))->setTabOrientation(TAB_TOP);
                }
                FXuint packing = tabbook->getPackingHints();
                packing &= ~PACK_UNIFORM_WIDTH;
                tabbook->setPackingHints(packing);
            }break;
        case 3: //right
            {
                tabbook->setTabStyle(TABBOOK_RIGHTTABS);
                for(FXint i = 0; i<tabbook->numChildren(); i=i+2)
                {
                    static_cast<FXTabItem*>(tabbook->childAtIndex(i))->setTabOrientation(TAB_RIGHT);
                }
                FXuint packing = tabbook->getPackingHints();
                packing |= PACK_UNIFORM_WIDTH;
                tabbook->setPackingHints(packing);
            }break;
        default:
            {
                tabbook->setTabStyle(TABBOOK_BOTTOMTABS);
                for(FXint i = 0; i<tabbook->numChildren(); i=i+2)
                {
                    static_cast<FXTabItem*>(tabbook->childAtIndex(i))->setTabOrientation(TAB_BOTTOM);
                }
                FXuint packing = tabbook->getPackingHints();
                packing &= ~PACK_UNIFORM_WIDTH;
                tabbook->setPackingHints(packing);
            }
    }   
}

long dxirc::OnCommandAbout(FXObject*, FXSelector, void*)
{
    FXDialogBox about(this, FXStringFormat(_("About %s"), PACKAGE), DECOR_TITLE|DECOR_BORDER, 0,0,0,0, 0,0,0,0, 0,0);
    FXVerticalFrame *content = new FXVerticalFrame(&about, LAYOUT_SIDE_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 10,10,10,10, 0,0);
    new FXLabel(content, FXStringFormat("%s %s \n", PACKAGE, VERSION), bigicon, ICON_BEFORE_TEXT|JUSTIFY_CENTER_Y|JUSTIFY_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(content, _("\nCopyright (C) 2008~ David Vachulka (david@konstrukce-cad.com)\n"), 0, JUSTIFY_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(content, FXStringFormat(_("This software was built with the FOX Toolkit Library version %d.%d.%d (http://www.fox-toolkit.org)."),FOX_MAJOR,FOX_MINOR,FOX_LEVEL), 0, JUSTIFY_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);    
#ifdef HAVE_OPENSSL
    new FXLabel(content, FXStringFormat(_("This software was built with %s"), OPENSSL_VERSION_TEXT), 0, JUSTIFY_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
#endif
#ifdef HAVE_LUA
    new FXLabel(content, FXStringFormat(_("This software was built with %s"), LUA_RELEASE), 0, JUSTIFY_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
#endif
    new FXLabel(content, _("This sofware uses http://www.famfamfam.com/lab/icons/"), 0, JUSTIFY_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    FXButton *button = new FXButton(content, _("&OK"), 0, &about, FXDialogBox::ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 32,32,5,5);
    button->setFocus();
    about.execute(PLACEMENT_OWNER);

    return 1;
}

long dxirc::OnCommandDccCancel(FXObject*, FXSelector, void *ptr)
{
    FXint index = (FXint)(FXival)ptr;
    if(index >= dccfilesList.no())
        return 1;
    for(FXint i=0; i<servers.no(); i++)
    {
        servers[i]->CloseDccfileConnection(dccfilesList[index]);
    }
    return 1;
}

long dxirc::OnCommandServers(FXObject*, FXSelector, void*)
{
#ifdef DEBUG
    fxmessage("OnCommandServers\n");
#endif
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
    new FXButton(buttonframe, _("&Cancel"), NULL, &serverEdit, FXDialogBox::ID_CANCEL, FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 32,32,5,5);
    if (serverEdit.execute(PLACEMENT_OWNER))
    {
#ifdef HAVE_OPENSSL
        ConnectServer(hostname->getText(), port->getValue(), passwd->getText(), nick->getText(), realname->getText(), channel->getText(), command->getText(), ussl->getCheck());
#else
        ConnectServer(hostname->getText(), port->getValue(), passwd->getText(), nick->getText(), realname->getText(), channel->getText(), command->getText(), FALSE);
#endif
    }
    return 1;
}

long dxirc::OnTabConnect(FXObject*, FXSelector, void *data)
{
    ServerInfo *srv = (ServerInfo*)data;
    ConnectServer(srv->hostname, srv->port, srv->passwd, srv->nick, srv->realname, srv->channels, "", FALSE);
    return 1;
}

void dxirc::ConnectServer(FXString hostname, FXint port, FXString pass, FXString nick, FXString rname, FXString channels, FXString commands, FXbool ssl, DCCTYPE dccType, FXString dccNick, IrcSocket *dccParent, DccFile dccFile)
{
#ifdef DEBUG
    fxmessage("ConnectServer\n");
#endif
    FXbool dcc = (dccType == DCC_CHATIN || dccType == DCC_CHATOUT);
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
        ssl = FALSE;
#endif
        servers[0]->SetUseSsl(ssl);
        servers[0]->SetReconnect(reconnect);
        servers[0]->SetNumberAttempt(numberAttempt);
        servers[0]->SetDelayAttempt(delayAttempt);
        servers[0]->SetDccType(dccType);
        servers[0]->SetDccFile(dccFile);
        if(dccType != DCC_IN && dccType != DCC_OUT && dccType != DCC_PIN && dccType != DCC_POUT)
        {
            if (!tabbook->numChildren())
            {
                IrcTabItem *tabitem = new IrcTabItem(tabbook, dcc ? dccNick : hostname, dcc ? queryicon : servericon, TAB_BOTTOM, dcc ? DCCCHAT : SERVER, servers[0], ownServerWindow, usersShown, logging, commandsList, logPath, maxAway, colors, nickCompletionChar, ircFont, sameCmd, sameList, coloredNick);
                tabitem->create();
                tabitem->CreateGeom();
                servers[0]->AppendTarget(tabitem);
                UpdateTabPosition();
            }
            if(compare(tabbook->childAtIndex(0)->getClassName(), "IrcTabItem") == 0)
            {
                static_cast<IrcTabItem*>(tabbook->childAtIndex(0))->SetType(SERVER, hostname);
                tabbook->setCurrent(0, TRUE);
            }
            SortTabs();
        }
        servers[0]->ClearAttempts();
        if(dccType == DCC_CHATOUT
                || dccType == DCC_OUT
                || dccType == DCC_PIN) servers[0]->StartListening(dccNick, dccParent);
        else servers[0]->StartConnection();
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
#ifndef HAVE_OPENSSL
        ssl = FALSE;
#endif
        servers[0]->SetUseSsl(ssl);
        servers[0]->SetReconnect(reconnect);
        servers[0]->SetNumberAttempt(numberAttempt);
        servers[0]->SetDelayAttempt(delayAttempt);
        servers[0]->SetDccType(dccType);
        servers[0]->SetDccFile(dccFile);
        if(dccType != DCC_IN && dccType != DCC_OUT && dccType != DCC_PIN && dccType != DCC_POUT)
        {
            IrcTabItem *tabitem = new IrcTabItem(tabbook, dcc ? dccNick : hostname, dcc ? queryicon : servericon, TAB_BOTTOM, dcc ? DCCCHAT : SERVER, servers[0], ownServerWindow, usersShown, logging, commandsList, logPath, maxAway, colors, nickCompletionChar, ircFont, sameCmd, sameList, coloredNick);
            tabitem->create();
            tabitem->CreateGeom();
            servers[0]->AppendTarget(tabitem);
            UpdateTabPosition();
            SortTabs();
        }
        servers[0]->ClearAttempts();
        if(dccType == DCC_CHATOUT
                || dccType == DCC_OUT
                || dccType == DCC_PIN) servers[0]->StartListening(dccNick, dccParent);
        else servers[0]->StartConnection();
    }
    UpdateMenus();    
}

long dxirc::OnCommandDisconnect(FXObject*, FXSelector, void*)
{
    if(tabbook->numChildren())
    {
        FXint index = tabbook->getCurrent()*2;        
        if(compare(tabbook->childAtIndex(index)->getClassName(), "IrcTabItem") != 0) return 0;
        IrcTabItem *currenttab = static_cast<IrcTabItem*>(tabbook->childAtIndex(index));
        IrcSocket *currentserver = NULL;
        for(FXint i=0; i < servers.no(); i++)
        {
            if(servers[i]->FindTarget(currenttab))
            {
                currentserver = servers[i];
                break;
            }
        }
        if(currentserver == NULL) return 0;
        if(currentserver->GetConnected())
        {
            FXDialogBox confirmDialog(this, _("Confirm disconnect"), DECOR_TITLE|DECOR_BORDER, 0,0,0,0, 0,0,0,0, 0,0);
            FXVerticalFrame *contents = new FXVerticalFrame(&confirmDialog, LAYOUT_SIDE_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 10,10,10,10, 0,0);
            new FXLabel(contents, FXStringFormat(_("Disconnect server: %s\nPort: %d\nNick: %s"), currentserver->GetServerName().text(), currentserver->GetServerPort(), currentserver->GetNickName().text()), NULL, JUSTIFY_LEFT|ICON_BEFORE_TEXT);
            FXHorizontalFrame* buttonframe = new FXHorizontalFrame(contents,LAYOUT_FILL_X|LAYOUT_FILL_Y);
            new FXButton(buttonframe, _("&Yes"), NULL, &confirmDialog, FXDialogBox::ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 32,32,5,5);
            new FXButton(buttonframe, _("&No"), NULL, &confirmDialog, FXDialogBox::ID_CANCEL, FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 32,32,5,5);
            if (confirmDialog.execute(PLACEMENT_OWNER))
            {
                currentserver->Disconnect();
                for(FXint i = tabbook->numChildren()-2; i > -1; i=i-2)
                {
                    if(currentserver->FindTarget(static_cast<IrcTabItem*>(tabbook->childAtIndex(i))))
                    {
                        currentserver->RemoveTarget(static_cast<IrcTabItem*>(tabbook->childAtIndex(i)));
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
                if(currentserver->FindTarget(static_cast<IrcTabItem*>(tabbook->childAtIndex(i))))
                {
                    currentserver->RemoveTarget(static_cast<IrcTabItem*>(tabbook->childAtIndex(i)));
                    delete tabbook->childAtIndex(i);
                    delete tabbook->childAtIndex(i);
                    tabbook->recalc();
                }
            }
        }
        if(tabbook->numChildren())
        {
            tabbook->setCurrent(tabbook->numChildren()/2-1, TRUE);
            SortTabs();
        }
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
        if(TabExist(server, ev->param1))
        {
            return 1;
        }
        if(serverTabIndex != -1 && !ownServerWindow)
        {
            static_cast<IrcTabItem*>(tabbook->childAtIndex(serverTabIndex))->SetType(CHANNEL, ev->param1);
            tabbook->setCurrent(serverTabIndex/2-1, TRUE);
        }
        else
        {
            IrcTabItem* tabitem = new IrcTabItem(tabbook, ev->param1, channelicon, TAB_BOTTOM, CHANNEL, server, ownServerWindow, usersShown, logging, commandsList, logPath, maxAway, colors, nickCompletionChar, ircFont, sameCmd, sameList, coloredNick);
            server->AppendTarget(tabitem);
            tabitem->create();
            tabitem->CreateGeom();
            UpdateTabPosition();
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
            static_cast<IrcTabItem*>(tabbook->childAtIndex(serverTabIndex))->SetType(QUERY, ev->param1);
            tabbook->setCurrent(serverTabIndex/2-1, TRUE);
        }
        else
        {
            IrcTabItem* tabitem = new IrcTabItem(tabbook, ev->param1, queryicon, TAB_BOTTOM, QUERY, server, ownServerWindow, usersShown, logging, commandsList, logPath, maxAway, colors, nickCompletionChar, ircFont, sameCmd, sameList, coloredNick);
            server->AppendTarget(tabitem);
            tabitem->create();
            tabitem->CreateGeom();
            UpdateTabPosition();
        }        
        SortTabs();
        if(ev->param2 == server->GetNickName())
        {
            for(FXint i = 0; i < tabbook->numChildren(); i=i+2)
            {
                if(server->FindTarget(static_cast<IrcTabItem*>(tabbook->childAtIndex(i))) && comparecase(static_cast<FXTabItem*>(tabbook->childAtIndex(i))->getText(), ev->param1) == 0) tabbook->setCurrent(i/2, TRUE);
            }
        }
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
                        if(server->FindTarget(static_cast<IrcTabItem*>(tabbook->childAtIndex(j))))
                        {
                            static_cast<IrcTabItem*>(tabbook->childAtIndex(j))->SetType(SERVER, server->GetServerName());
                            tabbook->setCurrent(j/2-1, TRUE);
                            break;
                        }
                    }
                }
                else
                {
                    FXint index = -1;
                    for(FXint j = 0; j < tabbook->numChildren(); j=j+2)
                    {
                        if((comparecase(static_cast<FXTabItem*>(tabbook->childAtIndex(j))->getText(), ev->param2) == 0) && server->FindTarget(static_cast<IrcTabItem*>(tabbook->childAtIndex(j)))) index = j;
                    }
                    if(index == -1) return 0;
                    server->RemoveTarget(static_cast<IrcTabItem*>(tabbook->childAtIndex(index)));
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
                    if(server->FindTarget(static_cast<IrcTabItem*>(tabbook->childAtIndex(j))))
                    {
                        static_cast<IrcTabItem*>(tabbook->childAtIndex(j))->SetType(SERVER, server->GetServerName());
                        tabbook->setCurrent(j/2-1, TRUE);
                        break;
                    }
                }
            }
            else
            {
                FXint index = -1;
                for(FXint j = 0; j < tabbook->numChildren(); j=j+2)
                {
                    if((comparecase(static_cast<FXTabItem*>(tabbook->childAtIndex(j))->getText(), ev->param3) == 0) && server->FindTarget(static_cast<IrcTabItem*>(tabbook->childAtIndex(j)))) index = j;
                }
                if(index == -1) return 0;
                tabbook->setCurrent(index/2, TRUE);
                server->RemoveTarget(static_cast<IrcTabItem*>(tabbook->childAtIndex(index)));
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
            if(server->FindTarget(static_cast<IrcTabItem*>(tabbook->childAtIndex(i))))
            {
                tabbook->setCurrent(i/2, TRUE);
                if(static_cast<IrcTabItem*>(tabbook->childAtIndex(i))->GetType() == DCCCHAT && server->GetDccType() == DCC_CHATOUT)
                    return 1;
                server->RemoveTarget(static_cast<IrcTabItem*>(tabbook->childAtIndex(i)));
                delete tabbook->childAtIndex(i);
                delete tabbook->childAtIndex(i);
                tabbook->recalc();
            }
        }
        SortTabs();
        UpdateMenus();
        UpdateStatus(ev->param1);
        return 1;
    }
    if(ev->eventType == IRC_RECONNECT || ev->eventType == IRC_CONNECT)
    {
        UpdateStatus(ev->param1);
        return 1;
    }
    if(ev->eventType == IRC_ENDMOTD)
    {
        UpdateMenus();
        return 1;
    }
    if(ev->eventType == IRC_PRIVMSG || ev->eventType == IRC_ACTION)
    {
        if(ev->param3.contains(server->GetNickName())) UpdateStatus(FXStringFormat(_("New highlighted message on %s"), ev->param2 == server->GetNickName() ? ev->param1.text() : ev->param2.text()));
#ifdef HAVE_LUA
        if(!scripts.no() || !scriptEvents.no()) return 0;
        for(FXint i=0; i<scriptEvents.no(); i++)
        {
            if(comparecase("privmsg", scriptEvents[i].name) == 0)
            {
                for(FXint j=0; j<scripts.no(); j++)
                {
                    if(comparecase(scriptEvents[i].script, scripts[j].name) == 0)
                    {
                        lua_pushstring(scripts[j].L, scriptEvents[i].funcname.text());
                        lua_gettable(scripts[j].L, LUA_GLOBALSINDEX);
                        if (lua_type(scripts[j].L, -1) != LUA_TFUNCTION) lua_pop(scripts[j].L, 1);
                        else
                        {
                            lua_pushstring(scripts[j].L, ev->param1.text());
                            lua_pushstring(scripts[j].L, ev->param3.text());
                            lua_pushinteger(scripts[j].L, GetTabId(server, ev->param2 == server->GetNickName() ? ev->param1 : ev->param2));
                            if (lua_pcall(scripts[j].L, 3, 0, 0))
                            {
                                AppendIrcStyledText(FXStringFormat(_("Lua plugin: error calling %s %s"), scriptEvents[i].funcname.text(), lua_tostring(scripts[j].L, -1)), 4);
                                lua_pop(scripts[j].L, 1);
                            }
                        }
                    }
                }
            }
        }
#endif        
        return 1;
    }
    if(ev->eventType == IRC_JOIN)
    {
#ifdef HAVE_LUA
        if(!scripts.no() || !scriptEvents.no()) return 0;
        for(FXint i=0; i<scriptEvents.no(); i++)
        {
            if(comparecase("join", scriptEvents[i].name) == 0)
            {
                for(FXint j=0; j<scripts.no(); j++)
                {
                    if(comparecase(scriptEvents[i].script, scripts[j].name) == 0)
                    {
                        lua_pushstring(scripts[j].L, scriptEvents[i].funcname.text());
                        lua_gettable(scripts[j].L, LUA_GLOBALSINDEX);
                        if (lua_type(scripts[j].L, -1) != LUA_TFUNCTION) lua_pop(scripts[j].L, 1);
                        else
                        {
                            lua_pushstring(scripts[j].L, ev->param1.text());
                            //lua_pushstring(scripts[j].L, ev->param2.text());
                            lua_pushinteger(scripts[j].L, GetTabId(server, ev->param2));
                            if (lua_pcall(scripts[j].L, 2, 0, 0))
                            {
                                AppendIrcStyledText(FXStringFormat(_("Lua plugin: error calling %s %s"), scriptEvents[i].funcname.text(), lua_tostring(scripts[j].L, -1)), 4);
                                lua_pop(scripts[j].L, 1);
                            }
                        }
                    }
                }
            }
        }
#endif
        return 1;
    }
    if(ev->eventType == IRC_DCCCHAT)
    {
        if(FXMessageBox::question(this, MBOX_YES_NO, _("Question"), _("%s offers DCC Chat on %s port %s.\n Do you want connect?"), ev->param1.text(), ev->param2.text(), ev->param3.text()) == 1)
        {
            ConnectServer(ev->param2, FXIntVal(ev->param3), "", server->GetNickName(), "", "", "", FALSE, DCC_CHATIN, ev->param1);
        }
        return 1;
    }
    if(ev->eventType == IRC_DCCSERVER)
    {
        ConnectServer(server->GetLocalIP(), 0, "", server->GetNickName(), "", "", "", FALSE, DCC_CHATOUT, ev->param1, server);
        return 1;
    }
    if(ev->eventType == IRC_DCCIN)
    {
        if(FXMessageBox::question(this, MBOX_YES_NO, _("Question"), _("%s offers file %s with size %s over DCC .\n Do you want connect?"), ev->param1.text(), ev->param3.text(), utils::GetFileSize(ev->param4).text()) == 1)
        {
            FXFileDialog dialog(this, _("Save file"));
            dialog.setFilename(dccPath+PATHSEPSTRING+ev->param3);
            if(dialog.execute())
            {
                DccFile dcc;
                dcc.path = dialog.getFilename();
                dcc.previousPostion = 0;
                dcc.currentPosition = 0;
                dcc.size = FXLongVal(ev->param4);
                dcc.type = DCC_IN;
                dcc.canceled = FALSE;
                dcc.finishedPosition = 0;
                dcc.token = -1;
                dcc.ip = ev->param2.before('@');
                dcc.port = FXIntVal(ev->param2.after('@'));
                for(FXint i=0; i<dccfilesList.no(); i++)
                {
                    if(dcc.path == dccfilesList[i].path && (dccfilesList[i].type==DCC_IN || dccfilesList[i].type==DCC_PIN))
                    {
                        dccfilesList.erase(i);
                        break;
                    }
                }
                ConnectServer(ev->param2.before('@'), FXIntVal(ev->param2.after('@')), "", server->GetNickName(), "", "", "", FALSE, DCC_IN, ev->param1, NULL, dcc);
                dccfilesList.append(dcc);
                OnCommandTransfers(NULL, 0, NULL);
            }
        }
        return 1;
    }
    if(ev->eventType == IRC_DCCOUT)
    {
        DccFile dcc;
        dcc.path = ev->param2;
        dcc.previousPostion = 0;
        dcc.currentPosition = 0;
        dcc.size = FXStat::size(ev->param2);
        dcc.type = DCC_OUT;
        dcc.canceled = FALSE;
        dcc.finishedPosition = 0;
        dcc.token = -1;
        for(FXint i=0; i<dccfilesList.no(); i++)
        {
            if(dcc.path == dccfilesList[i].path && dcc.type == dccfilesList[i].type)
            {
                dccfilesList.erase(i);
                break;
            }
        }
        ConnectServer(server->GetLocalIP(), 0, "", server->GetNickName(), "", "", "", FALSE, DCC_OUT, ev->param1, server, dcc);
        dccfilesList.append(dcc);
        OnCommandTransfers(NULL, 0, NULL);
        return 1;
    }
    if(ev->eventType == IRC_DCCPOUT)
    {
        DccFile dcc;
        dcc.path = ev->param2;
        dcc.previousPostion = 0;
        dcc.currentPosition = 0;
        dcc.size = FXStat::size(ev->param2);
        dcc.type = DCC_POUT;
        dcc.canceled = FALSE;
        dcc.finishedPosition = 0;
        if(lastToken==65000) lastToken=0;
        dcc.token = ++lastToken;
        for(FXint i=0; i<dccfilesList.no(); i++)
        {
            if(dcc.path == dccfilesList[i].path && dcc.type == dccfilesList[i].type)
            {
                dccfilesList.erase(i);
                break;
            }
        }
        dccfilesList.append(dcc);
        server->SendCtcp(ev->param1, "DCC SEND "+utils::RemoveSpaces(dcc.path.rafter(PATHSEP))+" "+server->GetLocalIP()+" 0 "+FXStringVal(dcc.size)+" "+FXStringVal(dcc.token));
        return 1;
    }
    if(ev->eventType == IRC_DCCMYTOKEN)
    {
        FXint token = FXIntVal(ev->param3);
        FXint index = -1;
        for(FXint i=0; i<dccfilesList.no(); i++)
        {
            if(dccfilesList[i].token == token && dccfilesList[i].type == DCC_POUT)
            {
                index = i;
                break;
            }
        }
        if(index == -1)
            return 1;
        dccfilesList[index].ip = ev->param1;
        dccfilesList[index].port = FXIntVal(ev->param2);
        ConnectServer(ev->param1, FXIntVal(ev->param2), "", server->GetNickName(), "", "", "", FALSE, dccfilesList[index].type, "", NULL, dccfilesList[index]);
        OnCommandTransfers(NULL, 0, NULL);
        return 1;
    }
    if(ev->eventType == IRC_DCCTOKEN)
    {
        if(FXMessageBox::question(this, MBOX_YES_NO, _("Question"), _("%s offers file %s with size %s over DCC passive.\n Do you want accept?"), ev->param1.text(), ev->param2.text(), utils::GetFileSize(ev->param3).text()) == 1)
        {
            FXFileDialog dialog(this, _("Save file"));
            dialog.setFilename(dccPath+PATHSEPSTRING+ev->param2);
            if(dialog.execute())
            {
                DccFile dcc;
                dcc.path = dialog.getFilename();
                dcc.previousPostion = 0;
                dcc.currentPosition = 0;
                dcc.size = FXLongVal(ev->param3);
                dcc.type = DCC_PIN;
                dcc.canceled = FALSE;
                dcc.finishedPosition = 0;
                dcc.token = FXIntVal(ev->param4);
                for(FXint i=0; i<dccfilesList.no(); i++)
                {
                    if(dcc.path == dccfilesList[i].path && (dccfilesList[i].type==DCC_IN || dccfilesList[i].type==DCC_PIN))
                    {
                        dccfilesList.erase(i);
                        break;
                    }
                }
                ConnectServer(server->GetLocalIP(), 0, "", server->GetNickName(), "", "", "", FALSE, DCC_PIN, ev->param1, server, dcc);
                dccfilesList.append(dcc);
                OnCommandTransfers(NULL, 0, NULL);
            }
        }
        return 1;
    }
    if(ev->eventType == IRC_DCCPOSITION)
    {
        FXint index = -1;
        for(FXint i=0; i<dccfilesList.no(); i++)
        {
            if (dccfilesList[i].path == ev->dccFile.path)
            {
                index = i;
                break;
            }
        }
        dccfilesList[index].previousPostion = dccfilesList[index].currentPosition;
        dccfilesList[index].currentPosition = ev->dccFile.currentPosition;
        dccfilesList[index].finishedPosition = ev->dccFile.finishedPosition;
        dccfilesList[index].canceled = ev->dccFile.canceled;
        dccfilesList[index].ip = ev->dccFile.ip;
        dccfilesList[index].port = ev->dccFile.port;
        return 1;
    }
    return 1;
}

long dxirc::OnTabBook(FXObject *, FXSelector, void *ptr)
{
    FXint index = (FXint)(FXival)ptr*2;
#ifdef DEBUG
    fxmessage("OnTabBook(%d), Class: %s\n", index, tabbook->childAtIndex(index)->getClassName());
#endif    
    if(compare(tabbook->childAtIndex(index)->getClassName(), "IrcTabItem") == 0)
    {
        IrcTabItem *currenttab = static_cast<IrcTabItem*>(tabbook->childAtIndex(index));
        if (appTheme.fore != currenttab->getTextColor()) currenttab->setTextColor(appTheme.fore);
        if(currenttab->GetType() == CHANNEL && currenttab->getIcon() == chnewm)
        {
            currenttab->setIcon(channelicon);
#ifdef HAVE_TRAY
            if(trayIcon && trayIcon->getIcon() == newm)
                trayIcon->setIcon(trayicon);
#endif
        }
        if((currenttab->GetType() == QUERY || currenttab->GetType() == DCCCHAT) && currenttab->getIcon() == unewm)
        {
            currenttab->setIcon(queryicon);
#ifdef HAVE_TRAY
            if(trayIcon && trayIcon->getIcon() == newm)
                trayIcon->setIcon(trayicon);
#endif
        }
        currenttab->setFocus();
        currenttab->SetCommandFocus();
        if(HasTetrisTab())
        {
            TetrisTabItem *tetristab = static_cast<TetrisTabItem*>(tabbook->childAtIndex(GetTabId("tetris")*2));
            if(tetristab->IsPauseEnable() && !tetristab->IsPaused()) tetristab->PauseResumeGame();
        }
    }    
    if(compare(tabbook->childAtIndex(index)->getClassName(), "TetrisTabItem") == 0)
    {
        TetrisTabItem *tetristab = static_cast<TetrisTabItem*>(tabbook->childAtIndex(index));
        tetristab->setFocus();
        tetristab->SetGameFocus();
    }
    return 1;
}

long dxirc::OnCommandNextTab(FXObject *, FXSelector, void *)
{
    FXint index = tabbook->getCurrent();
    if(tabbook->numChildren())
    {
        if((index+1)*2 < tabbook->numChildren()) tabbook->setCurrent(index+1, tabbook->numChildren() > index*2 ? TRUE : FALSE);
        else tabbook->setCurrent(0, TRUE);
    }
    return 1;
}

//Handle mousewheel for change currenttab
long dxirc::OnMouseWheel(FXObject *, FXSelector, void *ptr)
{
    FXEvent *event = (FXEvent*)ptr;
    FXint index = tabbook->getCurrent();
    if(event->code > 0) //positive movement
    {
        if(tabbook->numChildren())
        {
            if((index+1)*2 < tabbook->numChildren()) tabbook->setCurrent(index+1, tabbook->numChildren() > index*2 ? TRUE : FALSE);
            else tabbook->setCurrent(0, TRUE);
        }
    }
    else
    {
        if(tabbook->numChildren())
        {
            if((index-1) >= 0) tabbook->setCurrent(index-1, TRUE);
            else tabbook->setCurrent(tabbook->numChildren()/2-1, TRUE);
        }
    }
    return 1;
}

long dxirc::OnCommandNextUnread(FXObject *, FXSelector, void*)
{
    if(tabbook->numChildren())
    {
        for (FXint i = tabbook->getCurrent()*2; i<tabbook->numChildren(); i=i+2)
        {
            if (appTheme.fore != static_cast<FXTabItem*>(tabbook->childAtIndex(i))->getTextColor())
            {
                tabbook->setCurrent(i/2, TRUE);
                return 1;
            }
        }
        for (FXint i = tabbook->getCurrent()*2; i>-1; i=i-2)
        {
            if (appTheme.fore != static_cast<FXTabItem*>(tabbook->childAtIndex(i))->getTextColor())
            {
                tabbook->setCurrent(i/2, TRUE);
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
        if(compare(tabbook->childAtIndex(index)->getClassName(), "IrcTabItem") == 0)
        {
            IrcTabItem *currenttab = static_cast<IrcTabItem*>(tabbook->childAtIndex(index));
            currenttab->ClearChat();
        }
    }
    return 1;
}

long dxirc::OnCommandClearAll(FXObject *, FXSelector, void *)
{
    for (FXint i = 0; i<tabbook->numChildren(); i=i+2)
    {
        if(compare(tabbook->childAtIndex(i)->getClassName(), "IrcTabItem") == 0)
        {
        static_cast<IrcTabItem*>(tabbook->childAtIndex(i))->ClearChat();
        if (appTheme.fore != static_cast<IrcTabItem*>(tabbook->childAtIndex(i))->getTextColor()) static_cast<IrcTabItem*>(tabbook->childAtIndex(i))->setTextColor(appTheme.fore);
        if(static_cast<IrcTabItem*>(tabbook->childAtIndex(i))->GetType() == CHANNEL) static_cast<IrcTabItem*>(tabbook->childAtIndex(i))->setIcon(channelicon);
        if(static_cast<IrcTabItem*>(tabbook->childAtIndex(i))->GetType() == QUERY) static_cast<IrcTabItem*>(tabbook->childAtIndex(i))->setIcon(queryicon);
        }
    }
#ifdef HAVE_TRAY
    if(trayIcon && trayIcon->getIcon() == newm)
        trayIcon->setIcon(trayicon);
#endif
    return 1;
}

long dxirc::OnCommandCloseTab(FXObject *, FXSelector, void *)
{
    if(tabbook->numChildren())
    {
        FXint index = tabbook->getCurrent()*2;
        if(compare(tabbook->childAtIndex(index)->getClassName(), "IrcTabItem") == 0)
        {
            IrcTabItem *currenttab = static_cast<IrcTabItem*>(tabbook->childAtIndex(index));
            IrcSocket *currentserver = NULL;
            for(FXint i=0; i < servers.no(); i++)
            {
                if(servers[i]->FindTarget(currenttab))
                {
                    currentserver = servers[i];
                    break;
                }
            }
            if(currentserver == NULL) return 0;
            if(currenttab->GetType() == QUERY)
            {
                if(currentserver->GetConnected() && IsLastTab(currentserver))
                {
                    for(FXint j = 0; j < tabbook->numChildren(); j=j+2)
                    {
                        if(currentserver->FindTarget(static_cast<IrcTabItem*>(tabbook->childAtIndex(j))))
                        {
                            static_cast<IrcTabItem*>(tabbook->childAtIndex(j))->SetType(SERVER, currentserver->GetServerName());
                            tabbook->setCurrent(j/2-1, TRUE);
                            break;
                        }
                    }
                }
                else
                {
                    if(!currentserver->GetConnected()) currentserver->Disconnect();
                    currentserver->RemoveTarget(currenttab);
                    delete tabbook->childAtIndex(index);
                    delete tabbook->childAtIndex(index);
                    tabbook->recalc();
                    if(tabbook->numChildren())
                    {
                        tabbook->setCurrent(index/2-1, TRUE);
                    }
                }
                SortTabs();
                UpdateMenus();
                return 1;
            }
            if(currenttab->GetType() == CHANNEL)
            {
                if(currentserver->GetConnected()) currentserver->SendPart(currenttab->getText());
                if(currentserver->GetConnected() && IsLastTab(currentserver))
                {
                    for(FXint j = 0; j < tabbook->numChildren(); j=j+2)
                    {
                        if(currentserver->FindTarget(static_cast<IrcTabItem*>(tabbook->childAtIndex(j))))
                        {
                            static_cast<IrcTabItem*>(tabbook->childAtIndex(j))->SetType(SERVER, currentserver->GetServerName());
                            tabbook->setCurrent(j/2-1, TRUE);
                            break;
                        }
                    }
                }
                else
                {

                    if(!currentserver->GetConnected()) currentserver->Disconnect();
                    currentserver->RemoveTarget(currenttab);
                    delete tabbook->childAtIndex(index);
                    delete tabbook->childAtIndex(index);
                    tabbook->recalc();
                    if(tabbook->numChildren())
                    {
                        tabbook->setCurrent(index/2-1, TRUE);
                    }
                }
                SortTabs();
                UpdateMenus();
                return 1;
            }
            if(currenttab->GetType() == DCCCHAT)
            {
                currentserver->Disconnect();
                if(currentserver->FindTarget(currenttab))
                {
                    currentserver->RemoveTarget(currenttab);
                }
                else
                    return 1;
                delete tabbook->childAtIndex(index);
                delete tabbook->childAtIndex(index);
                tabbook->recalc();
                if(tabbook->numChildren())
                {
                    tabbook->setCurrent(index/2-1, TRUE);
                }
                SortTabs();
                UpdateMenus();
                return 1;
            }
            if(currenttab->GetType() == SERVER)
            {
                currentserver->Disconnect();
                for(FXint i = tabbook->numChildren()-2; i > -1; i=i-2)
                {
                    if(currentserver->FindTarget(static_cast<IrcTabItem*>(tabbook->childAtIndex(i))))
                    {
                        currentserver->RemoveTarget(static_cast<IrcTabItem*>(tabbook->childAtIndex(i)));
                        delete tabbook->childAtIndex(i);
                        delete tabbook->childAtIndex(i);
                        tabbook->recalc();
                        if(tabbook->numChildren())
                        {
                            tabbook->setCurrent(index/2-1, TRUE);
                        }
                    }
                }
                SortTabs();
                UpdateMenus();
                return 1;
            }
        }
        else
        {
            TetrisTabItem *tetristab = static_cast<TetrisTabItem*>(tabbook->childAtIndex(index));
            tetristab->StopGame();
            delete tabbook->childAtIndex(index);
            delete tabbook->childAtIndex(index);
            tabbook->recalc();
            if(tabbook->numChildren())
            {
                tabbook->setCurrent(index/2-1, TRUE);
            }
            SortTabs();
            UpdateMenus();
            return 1;
        }
    }
    return 1;
}

long dxirc::OnCommandSelectTab(FXObject*, FXSelector, void *ptr)
{
    FXEvent* event = (FXEvent*)ptr;
    switch(event->code){
        case KEY_1:
        {
            tabbook->setCurrent(0, tabbook->numChildren() ? TRUE : FALSE);
            break;
        }
        case KEY_2:
        {
            tabbook->setCurrent(1, tabbook->numChildren() > 2 ? TRUE : FALSE);
            break;
        }
        case KEY_3:
        {
            tabbook->setCurrent(2, tabbook->numChildren() > 4 ? TRUE : FALSE);
            break;
        }
        case KEY_4:
        {
            tabbook->setCurrent(3, tabbook->numChildren() > 6 ? TRUE : FALSE);
            break;
        }
        case KEY_5:
        {
            tabbook->setCurrent(4, tabbook->numChildren() > 8 ? TRUE : FALSE);
            break;
        }
        case KEY_6:
        {
            tabbook->setCurrent(5, tabbook->numChildren() > 10 ? TRUE : FALSE);
            break;
        }
        case KEY_7:
        {
            tabbook->setCurrent(6, tabbook->numChildren() > 12 ? TRUE : FALSE);
            break;
        }
        case KEY_8:
        {
            tabbook->setCurrent(7, tabbook->numChildren() > 14 ? TRUE : FALSE);
            break;
        }
        case KEY_9:
        {
            tabbook->setCurrent(8, tabbook->numChildren() > 16 ? TRUE : FALSE);
            break;
        }
    }
    return 1;
}

long dxirc::OnTrayClicked(FXObject*, FXSelector, void*)
{
#ifdef HAVE_TRAY
    if(shown())
        hide();
    else
        show();
    if(trayIcon && trayIcon->getIcon() == newm)
        trayIcon->setIcon(trayicon);
#endif
    return 1;
}

long dxirc::OnNewMsg(FXObject *obj, FXSelector, void*)
{
#ifdef HAVE_TRAY
    if(trayIcon && trayIcon->getIcon() == trayicon && (!shown() || static_cast<IrcTabItem*>(tabbook->childAtIndex(tabbook->getCurrent()*2)) != static_cast<IrcTabItem*>(obj)))
        trayIcon->setIcon(newm);
#endif
    return 1;
}

long dxirc::OnNewTetris(FXObject*, FXSelector, void*)
{
    if(HasTetrisTab()) return 1;
    TetrisTabItem *tab = new TetrisTabItem(tabbook, "tetris", 0, TAB_TOP);
    tab->create();
    tab->CreateGeom();
    tab->SetColor(colors);   
    UpdateTabPosition();
    SortTabs();
    UpdateMenus();
    tabbook->setCurrent(tabbook->numChildren()/2-1, TRUE);
    return 1;
}

long dxirc::OnTetrisKey(FXObject*, FXSelector, void *ptr)
{
    if(compare(tabbook->childAtIndex(tabbook->getCurrent()*2)->getClassName(), "TetrisTabItem") != 0) return 1;
    FXEvent* event = (FXEvent*)ptr;
    switch(event->code){
        case KEY_N:
        case KEY_n:
        {
            static_cast<TetrisTabItem*>(tabbook->childAtIndex(tabbook->getCurrent()*2))->NewGame();
            break;
        }
        case KEY_P:
        case KEY_p:
        {
            static_cast<TetrisTabItem*>(tabbook->childAtIndex(tabbook->getCurrent()*2))->PauseResumeGame();
            break;
        }
        case KEY_KP_5:
        {
            static_cast<TetrisTabItem*>(tabbook->childAtIndex(tabbook->getCurrent()*2))->Rotate();
            break;
        }
        case KEY_KP_3:
        {
            static_cast<TetrisTabItem*>(tabbook->childAtIndex(tabbook->getCurrent()*2))->MoveRight();
            break;
        }
        case KEY_KP_2:
        {
            static_cast<TetrisTabItem*>(tabbook->childAtIndex(tabbook->getCurrent()*2))->Drop();
            break;
        }
        case KEY_KP_1:
        {
            static_cast<TetrisTabItem*>(tabbook->childAtIndex(tabbook->getCurrent()*2))->MoveLeft();
            break;
        }
    }
    return 1;
}

void dxirc::AutoloadScripts()
{
#ifdef HAVE_LUA
    if(autoload && FXStat::exists(autoloadPath))
    {
        FXDir dir;
        FXString name, pathname;
        FXStat info;
        FXint islink;
        // Assume not a link
        islink = FALSE;
        // Managed to open directory
        if (dir.open(autoloadPath))
        {
            // Process directory entries
            while (dir.next())
            {
                // Get name of entry
                name = dir.name();
                // A dot special file?
                if (name[0] == '.' && (name[1] == 0 || (name[1] == '.' && name[2] == 0))) continue;
                // Hidden file or directory normally not shown
                if (name[0] == '.') continue;
                // Build full pathname of entry
                pathname = autoloadPath;
                if (!ISPATHSEP(pathname[pathname.length() - 1])) pathname += PATHSEPSTRING;
                pathname += name;
#ifndef WIN32
                // Get file/link info
                if (!FXStat::statLink(pathname, info)) continue;
                // If its a link, get the info on file itself
                islink = info.isLink();
                if (islink && !FXStat::statFile(pathname, info)) continue;
#else
                // Get file/link info
                if (!FXStat::statFile(pathname, info)) continue;
                // Hidden file or directory normally not shown
                if (info.isHidden()) continue;
#endif
                // If it is a directory
                if(info.isDirectory()) continue;
                // If it is not matching pattern skip it
                if (!FXPath::match("*.lua", name))continue;
                LoadLuaScript(pathname, FALSE);
            }
            // Close it
            dir.close();
        }
    }
#endif
}

long dxirc::OnCommandScripts(FXObject*, FXSelector, void*)
{
#ifdef HAVE_LUA
    ScriptDialog *dialog = new ScriptDialog(this);
    dialog->execute(PLACEMENT_OWNER);
#endif
    return 1;
}

long dxirc::OnLua(FXObject *obj, FXSelector, void *data)
{
#ifdef HAVE_LUA
    LuaRequest *lua = (LuaRequest*)data;
    if(lua->type == LUA_LOAD)
    {
        return LoadLuaScript(lua->text);
    }
    if(lua->type == LUA_UNLOAD)
    {
        return UnloadLuaScript(lua->text);
    }
    if(lua->type == LUA_LIST)
    {
        if(!scripts.no())
        {
            AppendIrcStyledText(_("Scripts aren't loaded"), 4);
            return 0;
        }
        else
        {
            AppendIrcStyledText(_("Loaded scrips:"), 7);
            for(FXint i=0; i<scripts.no(); i++)
            {                
                AppendIrcText(FXStringFormat(_("Name: %s"), scripts[i].name.text()));
                AppendIrcText(FXStringFormat(_("Description: %s"), scripts[i].description.text()));
                AppendIrcText(FXStringFormat(_("Version: %s"), scripts[i].version.text()));
                AppendIrcText(FXStringFormat(_("Path: %s"), scripts[i].path.text()));
                if(i+1<scripts.no()) AppendIrcText("");
            }
        }
        return 1;
    }
    if(lua->type == LUA_COMMAND)
    {
        IrcTabItem *tab = static_cast<IrcTabItem*>(obj);
        FXString command = lua->text.before(' ');
        FXString text = lua->text.after(' ');
        for(FXint i=0; i<scripts.no(); i++)
        {
            if(comparecase(utils::GetScriptName(command), scripts[i].name) == 0)
            {
                lua_pushstring(scripts[i].L, utils::GetFuncname(command).text());
                lua_gettable(scripts[i].L, LUA_GLOBALSINDEX);
                if(lua_isfunction(scripts[i].L, -1))
                {
                    lua_pushstring(scripts[i].L, text.text());
                    lua_pushnumber(scripts[i].L, tabbook->indexOfChild(tab)/2);
                    if(lua_pcall(scripts[i].L, 2, 0, 0)) AppendIrcStyledText(FXStringFormat(_("Error: %s"), lua_tostring(scripts[i].L, -1)), 4);
                }
                else lua_pop(scripts[i].L, 1);
                return 1;
            }
        }
    }
#else
    AppendIrcStyledText(_("dxirc is compiled without support for Lua scripting"), 4);
#endif
    return 1;
}

//Handle for entered command in IrcTab for all in lua scripting
long dxirc::OnIrcCommand(FXObject *sender, FXSelector, void *data)
{
#ifdef HAVE_LUA    
    if(compare(sender->getClassName(), "IrcTabItem") != 0) return 0;
    IrcTabItem *tab = static_cast<IrcTabItem*>(sender);
    FXString *commandtext = static_cast<FXString*>(data);
    if(!scripts.no() || !scriptEvents.no())
    {
        return 0;
    }
    tab->HasAllCommand(HasAllCommand());
    for(FXint i=0; i<scriptEvents.no(); i++)
    {
        if(comparecase("all", scriptEvents[i].name) == 0)
        {            
            for(FXint j=0; j<scripts.no(); j++)
            {
                if(comparecase(scriptEvents[i].script, scripts[j].name) == 0)
                {
                    lua_pushstring(scripts[j].L, scriptEvents[i].funcname.text());
                    lua_gettable(scripts[j].L, LUA_GLOBALSINDEX);
                    if (lua_type(scripts[j].L, -1) != LUA_TFUNCTION) lua_pop(scripts[j].L, 1);
                    else
                    {
                        if(commandtext->at(0) == '/')
                        {
                            lua_pushstring(scripts[j].L, commandtext->before(' ').after('/').text());
                            lua_pushstring(scripts[j].L, commandtext->after(' ').text());
                        }
                        else
                        {
                            lua_pushnil(scripts[j].L);
                            lua_pushstring(scripts[j].L, commandtext->text());
                        }
                        lua_pushinteger(scripts[j].L, tabbook->indexOfChild(tab)/2);
                        if (lua_pcall(scripts[j].L, 3, 0, 0))
                        {
                            AppendIrcStyledText(FXStringFormat(_("Lua plugin: error calling %s %s"), scriptEvents[i].funcname.text(), lua_tostring(scripts[j].L, -1)), 4);
                            lua_pop(scripts[j].L, 1);
                        }
                    }
                }
            }
        }
    }
#endif
    return 1;
}

FXbool dxirc::TabExist(IrcSocket *server, FXString name)
{
    for(FXint i = 0; i < tabbook->numChildren(); i=i+2)
    {
        if(server->FindTarget(static_cast<IrcTabItem*>(tabbook->childAtIndex(i))) && comparecase(static_cast<FXTabItem*>(tabbook->childAtIndex(i))->getText(), name) == 0) return TRUE;
    }
    return FALSE;
}

FXbool dxirc::ServerExist(const FXString &server, const FXint &port, const FXString &nick)
{
    for(FXint i = 0; i < servers.no(); i++)
    {
        if(servers[i]->GetServerName() == server && servers[i]->GetServerPort() == port && servers[i]->GetNickName() == nick && servers[i]->GetConnected()) return TRUE;
    }
    return FALSE;
}

FXint dxirc::GetServerTab(IrcSocket *server)
{
    for(FXint i = 0; i < tabbook->numChildren(); i=i+2)
    {        
        if(compare(tabbook->childAtIndex(i)->getClassName(), "IrcTabItem") == 0)
        {
            IrcTabItem *tab = static_cast<IrcTabItem*>(tabbook->childAtIndex(i));
            if(server->FindTarget(tab) && tab->GetType() == SERVER) return i;
        }
    }
    return -1;
}

FXint dxirc::GetTabId(IrcSocket *server, FXString name)
{
    for(FXint i = 0; i < tabbook->numChildren(); i=i+2)
    {
        if(server->FindTarget(static_cast<IrcTabItem*>(tabbook->childAtIndex(i))) && comparecase(name, static_cast<FXTabItem*>(tabbook->childAtIndex(i))->getText()) == 0) return i/2;
    }
    return -1;
}

//usefull mainly for tetristab
FXint dxirc::GetTabId(FXString name)
{
    for(FXint i = 0; i < tabbook->numChildren(); i=i+2)
    {
        if(comparecase(name, static_cast<FXTabItem*>(tabbook->childAtIndex(i))->getText()) == 0) return i/2;
    }
    return -1;
}

FXbool dxirc::IsLastTab(IrcSocket *server)
{
    FXint numTabs = 0;
    for(FXint i = 0; i < tabbook->numChildren(); i=i+2)
    {
        if(server->FindTarget(static_cast<IrcTabItem*>(tabbook->childAtIndex(i)))) numTabs++;
    }
    if(numTabs > 1) return FALSE;
    else return TRUE;
}

FXbool dxirc::HasTetrisTab()
{
    if(tabbook->numChildren())
    {
        for(FXint i = 0; i < tabbook->numChildren()/2; i++)
        {
            if(compare(tabbook->childAtIndex(i*2)->getClassName(), "TetrisTabItem") == 0) return TRUE;
        }
    }
    return FALSE;
}

void dxirc::SortTabs()
{
    if(tabbook->numChildren()/2 > 1)
    {
        if(HasTetrisTab())
        {
            FXint index = 0;
            TetrisTabItem *tetristab = NULL;
            IrcTabItem* *tabpole = new IrcTabItem*[tabbook->numChildren()/2-1];
            for(FXint i = 0; i < tabbook->numChildren()/2; i++)
            {
                if(compare(tabbook->childAtIndex(i*2)->getClassName(), "IrcTabItem") == 0)
                {
                    tabpole[index] = static_cast<IrcTabItem*>(tabbook->childAtIndex(i*2));
                    index++;
                }
                else tetristab = static_cast<TetrisTabItem*>(tabbook->childAtIndex(i*2));
            }
            qsort(tabpole, tabbook->numChildren()/2-1, sizeof(tabpole[0]), (int(*)(const void*, const void*))&CompareTabs);
            for(int i=0; i < tabbook->numChildren()/2-1; i++)
            {
                tabpole[i]->ReparentTab();
            }
            tetristab->ReparentTab();
            tabbook->recalc();
            delete []tabpole;
        }
        else
        {
            IrcTabItem* *tabpole = new IrcTabItem*[tabbook->numChildren()/2];
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
            delete []tabpole;
        }
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
    FXbool someConnected = FALSE;
    for(FXint i = 0; i < servers.no(); i++)
    {
        if(servers[i]->GetConnected()) someConnected = TRUE;
    }
    if(someConnected) disconnect->enable();
    else disconnect->disable();
}

void dxirc::UpdateStatus(FXString text)
{
    statuslabel->setText(text);
#ifdef HAVE_TRAY
    if(useTray)
    {
        trayIcon->setText("dxirc\n"+text);
    }
#endif
    app->addTimeout(this, ID_STIMEOUT, 5000);
}

long dxirc::OnStatusTimeout(FXObject*, FXSelector, void*)
{
    statuslabel->setText(" ");
#ifdef HAVE_TRAY
    if(useTray)
    {
        trayIcon->setText("dxirc");
    }
#endif
    return 1;
}

void dxirc::AppendIrcText(FXString text)
{
    if(tabbook->numChildren())
    {
        FXint index = tabbook->getCurrent()*2;        
        if(compare(tabbook->childAtIndex(index)->getClassName(), "IrcTabItem"))
        {
            for(FXint i=0; i<tabbook->numChildren(); i+=2)
            {
                if(!compare(tabbook->childAtIndex(index)->getClassName(), "IrcTabItem"))
                {
                    index = i;
                    break;
                }
            }
        }
        IrcTabItem *currenttab = static_cast<IrcTabItem*>(tabbook->childAtIndex(index));
        FXASSERT(currenttab != 0);
        currenttab->AppendIrcText(text, TRUE);
        currenttab->MakeLastRowVisible(TRUE);
    }
}

void dxirc::AppendIrcStyledText(FXString text, FXint style)
{
    if(tabbook->numChildren())
    {
        FXint index = tabbook->getCurrent()*2;
        if(compare(tabbook->childAtIndex(index)->getClassName(), "IrcTabItem"))
        {
            for(FXint i=0; i<tabbook->numChildren(); i+=2)
            {
                if(!compare(tabbook->childAtIndex(index)->getClassName(), "IrcTabItem"))
                {
                    index = i;
                    break;
                }
            }
        }
        IrcTabItem *currenttab = static_cast<IrcTabItem*>(tabbook->childAtIndex(index));
        FXASSERT(currenttab != 0);
        currenttab->AppendIrcStyledText(text, style, TRUE);
        currenttab->MakeLastRowVisible(TRUE);
    }
}

FXint dxirc::LoadLuaScript(FXString path, FXbool showMessage)
{
#ifdef HAVE_LUA
    if(scripts.no())
    {
       for(FXint i=0; i<scripts.no(); i++)
        {
            if(comparecase(path, scripts[i].path)==0)
            {
                AppendIrcStyledText(FXStringFormat(_("Script %s is already loaded"), path.text()), 4);
                return 0;
            }
        }
    }
    if(HasLuaAll(path))
    {
        if(showMessage)
        {
            if(FXMessageBox::question(this, MBOX_YES_NO, _("Question"), _("Script %s contains dxirc.AddAll\nThis can BREAK dxirc funcionality.\nLoad it anyway?"), path.text()) == 2) return 0;
        }
        else AppendIrcStyledText(FXStringFormat(_("Script %s contains dxirc.AddAll. This can BREAK dxirc funcionality."), path.text()), 4);
    }
    lua_State *L = luaL_newstate();
    if(L == NULL)
    {
        AppendIrcStyledText(_("Unable to initialize Lua."), 4);
        return 0;
    }
    if(L)
    {
        luaL_openlibs(L);
        luaL_register(L, "dxirc", dxircFunctions);
        if(luaL_dofile(L, path.text()))
        {
            AppendIrcStyledText(FXStringFormat(_("Unable to load/run the file %s"), lua_tostring(L, -1)), 4);
            return 0;
        }
        lua_pushstring(L, "dxirc_Register");
        lua_gettable(L, LUA_GLOBALSINDEX);
        if(lua_pcall(L, 0, 3, 0))
        {
            AppendIrcStyledText(FXStringFormat(_("Lua plugin: error registering script %s"), lua_tostring(L, -1)), 4);
            return 0;
        }
        FXString name = lua_tostring(L, -3);
        FXString version = lua_tostring(L, -2);
        FXString description = lua_tostring(L, -1);
        lua_pop(L, 4);
        if(scripts.no())
        {
           for(FXint i=0; i<scripts.no(); i++)
            {
                if(comparecase(name, scripts[i].name)==0)
                {
                    AppendIrcStyledText(FXStringFormat(_("Script with name %s is already loaded"), name.lower().text()), 4);
                    return 0;
                }
            }
        }
        LuaScript script;
        script.L = L;
        script.name = name;
        script.version = version;
        script.description = description;
        script.path = path;
        scripts.append(script);
        AppendIrcStyledText(FXStringFormat(_("Script %s was loaded"), path.text()), 3);
        lua_pushstring(L, "dxirc_Init");
        lua_gettable(L, LUA_GLOBALSINDEX);
        if (lua_type(L, -1) != LUA_TFUNCTION) lua_pop(L, 1);
        else
        {
            if (lua_pcall(L, 0, 0, 0))
            {
                AppendIrcStyledText(FXStringFormat("Lua plugin: error calling dxirc_Init() %s", lua_tostring(L, -1)), 3);
                lua_pop(L, 1);
            }
        }        
        return 1;
    }
    return 1;
#else
    return 0;
#endif
}

FXint dxirc::UnloadLuaScript(FXString name)
{
#ifdef HAVE_LUA
    FXbool success = FALSE;
    if(!scripts.no())
    {
        AppendIrcStyledText(FXStringFormat(_("Script %s isn't loaded"), name.text()), 4);
        return 0;
    }
    else
    {
        for(FXint i=scripts.no()-1; i>-1; i--)
        {
            if(comparecase(name, scripts[i].name)==0)
            {
                utils::RemoveScriptCommands(scripts[i].name);
                lua_close(scripts[i].L);
                scripts.erase(i);
                success = TRUE;
            }
        }
    }
    for(FXint i=scriptEvents.no()-1; i>-1; i--)
    {
        if(comparecase(name, scriptEvents[i].script)==0)
        {
            scriptEvents.erase(i);
            success = TRUE;
        }
    }
    if(success) AppendIrcStyledText(FXStringFormat(_("Script %s was unloaded"), name.text()), 4);
    else AppendIrcStyledText(FXStringFormat(_("Script %s isn't loaded"), name.text()), 4);
    return 1;
#else
    return 0;
#endif
}

FXbool dxirc::HasLuaAll(const FXString &file)
{
    std::ifstream fin(file.text());
    std::string line;
    while(fin.good())
    {
        std::getline(fin, line);
        if(line.find("dxirc.AddAll") != std::string::npos) return TRUE;
    }
    return FALSE;
}

//check for all in loaded script
FXbool dxirc::HasAllCommand()
{
    if(!scripts.no() || !scriptEvents.no())
    {
        return FALSE;
    }
    for(FXint i=0; i<scriptEvents.no(); i++)
    {
        if(comparecase("all", scriptEvents[i].name) == 0)
        {
            return TRUE;
        }
    }
    return FALSE;
}

int dxirc::OnLuaAddCommand(lua_State *lua)
{
#ifdef HAVE_LUA
    FXString name, funcname, helptext, script;
    if(lua_isstring(lua, 1)) name = lua_tostring(lua,1);
    if(lua_isstring(lua, 2)) funcname = lua_tostring(lua,2);
    if(lua_isstring(lua, 3)) helptext = lua_tostring(lua,3);
    if(name.empty() || funcname.empty() || helptext.empty()) return 0;
    if(utils::IsCommand(name))
    {
        pThis->AppendIrcStyledText(FXStringFormat(_("Command %s already exists"), name.text()), 4);
        return 0;
    }
    if(pThis->scripts.no())
    {
        for(FXint i=0; i<pThis->scripts.no(); i++)
        {
            if(lua == pThis->scripts[i].L) script = pThis->scripts[i].name;
        }
    }
    if(script.empty()) return 0;
    LuaScriptCommand command;
    command.name = name;
    command.funcname = funcname;
    command.helptext = helptext;
    command.script = script;
    utils::AddScriptCommand(command);
    return  1;
#else
    return 0;
#endif    
}

int dxirc::OnLuaAddEvent(lua_State *lua)
{
#ifdef HAVE_LUA
    FXString name, funcname, script;
    if(lua_isstring(lua, 1)) name = lua_tostring(lua,1);
    if(lua_isstring(lua, 2)) funcname = lua_tostring(lua,2);
    if(name.empty() || funcname.empty()) return 0;
    if(pThis->scripts.no())
    {
        for(FXint i=0; i<pThis->scripts.no(); i++)
        {
            if(lua == pThis->scripts[i].L) script = pThis->scripts[i].name;
        }
    }
    if(script.empty()) return 0;
    if(pThis->scriptEvents.no())
    {
        for(FXint i=0; i<pThis->scriptEvents.no(); i++)
        {
            if(comparecase(name, pThis->scriptEvents[i].name)==0 && comparecase(funcname, pThis->scriptEvents[i].funcname)==0 && comparecase(script, pThis->scriptEvents[i].script)==0)
            {
                pThis->AppendIrcStyledText(FXStringFormat(_("Function %s for event %s already exists"), funcname.text(), name.text()), 4);
                return 0;
            }
        }
    }
    LuaScriptEvent event;
    event.name = name;
    event.funcname = funcname;
    event.script = script;
    pThis->scriptEvents.append(event);
    return  1;
#else
    return 0;
#endif    
}

int dxirc::OnLuaAddAll(lua_State *lua)
{
#ifdef HAVE_LUA
    FXString funcname, script;
    if(lua_isstring(lua, 1)) funcname = lua_tostring(lua,1);
    if(funcname.empty()) return 0;
    if(pThis->scripts.no())
    {
        for(FXint i=0; i<pThis->scripts.no(); i++)
        {
            if(lua == pThis->scripts[i].L) script = pThis->scripts[i].name;
        }
    }
    if(script.empty()) return 0;
    LuaScriptEvent event;
    event.name = "all";
    event.funcname = funcname;
    event.script = script;
    pThis->scriptEvents.append(event);
    return  1;
#else
    return 0;
#endif
}

int dxirc::OnLuaRemoveName(lua_State *lua)
{
#ifdef HAVE_LUA
    FXString command, script;
    if(lua_isstring(lua, 1)) command = lua_tostring(lua, 1);
    if(command.empty()) return 0;
    if(utils::RemoveScriptCommand(command)) return 1;
    if(pThis->scripts.no())
    {
        for(FXint i=0; i<pThis->scripts.no(); i++)
        {
            if(lua == pThis->scripts[i].L) script = pThis->scripts[i].name;
        }
    }
    if(script.empty()) return 0;
    if(pThis->scriptEvents.no())
    {
        for(FXint i=0; i<pThis->scriptEvents.no(); i++)
        {
            if(comparecase(command, pThis->scriptEvents[i].name) == 0 && comparecase(script, pThis->scriptEvents[i].script) == 0)
            {
                pThis->scriptEvents.erase(i);
            }
        }
    }
    return  1;
#else
    return 0;
#endif    
}

int dxirc::OnLuaCommand(lua_State *lua)
{
#ifdef HAVE_LUA
    FXString command;
    if(lua_isstring(lua, 1)) command = lua_tostring(lua, 1);
    if(command.empty()) return 0;
    FXint id;
    if(lua_isnumber(lua, 2)) id = lua_tointeger(lua, 2);
    else id = pThis->tabbook->getCurrent();
    if(pThis->tabbook->numChildren())
    {
        for (FXint i = 0; i<pThis->tabbook->numChildren(); i=i+2)
        {
            if(id*2 == i)
            {
                if(compare(pThis->tabbook->childAtIndex(i)->getClassName(), "IrcTabItem") != 0) return 0;
                IrcTabItem *tab = static_cast<IrcTabItem*>(pThis->tabbook->childAtIndex(i));
                tab->ProcessLine(command);
                return 1;
            }
        }
    }
    return  1;
#else
    return 0;
#endif    
}

int dxirc::OnLuaPrint(lua_State *lua)
{
#ifdef HAVE_LUA
    FXString text;
    if(lua_isstring(lua, 1)) text = lua_tostring(lua, 1);
    if(text.empty()) return 0;
    FXint id, style;
    if(lua_isnumber(lua, 2)) id = lua_tointeger(lua, 2);
    else id = pThis->tabbook->getCurrent();
    if(lua_isnumber(lua, 3)) style = lua_tointeger(lua, 3);
    else style = 0;
    if(pThis->tabbook->numChildren())
    {
        for (FXint i = 0; i<pThis->tabbook->numChildren(); i=i+2)
        {
            if(id*2 == i)
            {
                if(compare(pThis->tabbook->childAtIndex(i)->getClassName(), "IrcTabItem") != 0) return 0;
                IrcTabItem *tab = static_cast<IrcTabItem*>(pThis->tabbook->childAtIndex(i));
                tab->AppendIrcStyledText(text, style, TRUE);
                tab->MakeLastRowVisible(TRUE);
                return 1;
            }
        }
    }
    return  1;
#else
    return 0;
#endif    
}

int dxirc::OnLuaGetServers(lua_State *lua)
{
#ifdef HAVE_LUA
    if(pThis->servers.no())
    {
        lua_newtable(lua);
        for(FXint i=0; i<pThis->servers.no(); i++)
        {
            lua_pushnumber(lua, i+1);
            lua_newtable(lua);
            lua_pushstring(lua, "server");
            lua_pushstring(lua, pThis->servers[i]->GetServerName().text());
            lua_settable(lua, -3);
            lua_pushstring(lua, "port");
            lua_pushnumber(lua, pThis->servers[i]->GetServerPort());
            lua_settable(lua, -3);
            lua_pushstring(lua, "nick");
            lua_pushstring(lua, pThis->servers[i]->GetNickName().text());
            lua_settable(lua, -3);
            lua_settable(lua, -3);
        }
    }
    else
    {
        lua_newtable(lua);
        lua_pushnumber(lua, 1);
        lua_newtable(lua);
        lua_pushstring(lua, "server");
        lua_pushnil(lua);
        lua_pushstring(lua, "port");
        lua_pushnil(lua);
        lua_settable(lua, -3);
        lua_pushstring(lua, "nick");
        lua_pushnil(lua);
        lua_settable(lua, -3);
        lua_settable(lua, -3);
    }
    return  1;
#else
    return 0;
#endif
}

int dxirc::OnLuaGetTab(lua_State *lua)
{
#ifdef HAVE_LUA
    FXString tab, server;
    if(lua_isstring(lua, 1)) tab = lua_tostring(lua, 1);
    if(lua_isstring(lua, 2)) server = lua_tostring(lua,2);
    if(pThis->tabbook->numChildren())
    {
        for (FXint i = 0; i<pThis->tabbook->numChildren(); i=i+2)
        {
            if(compare(pThis->tabbook->childAtIndex(i)->getClassName(), "IrcTabItem") == 0)
            {
                if(comparecase(tab, static_cast<FXTabItem*>(pThis->tabbook->childAtIndex(i))->getText()) == 0 && comparecase(server, static_cast<IrcTabItem*>(pThis->tabbook->childAtIndex(i))->GetServerName()) == 0)
                {
                    lua_pushnumber(lua, i/2);
                    return 1;
                }
            }
        }
        lua_pushnumber(lua, pThis->tabbook->getCurrent());
        return 1;
    }
    else lua_pushnumber(lua, -1);
    return  1;
#else
    return 0;
#endif    
}

int dxirc::OnLuaGetTabInfo(lua_State *lua)
{
#ifdef HAVE_LUA
    FXint id;
    if(lua_isnumber(lua, 1))  id = lua_tointeger(lua, 1);
    else id = -1;
    if(pThis->tabbook->numChildren() && id != -1 && id*2 < pThis->tabbook->numChildren() && compare(pThis->tabbook->childAtIndex(id*2)->getClassName(), "IrcTabItem") == 0)
    {
        lua_newtable(lua);
        lua_pushstring(lua, "name");
        lua_pushstring(lua, static_cast<IrcTabItem*>(pThis->tabbook->childAtIndex(id*2))->getText().text());
        lua_settable(lua, -3);
        switch(static_cast<IrcTabItem*>(pThis->tabbook->childAtIndex(id*2))->GetType()) {
            case SERVER:
            {
                lua_pushstring(lua, "type");
                lua_pushstring(lua, "server");
                lua_settable(lua, -3);
            }break;
            case CHANNEL:
            {
                lua_pushstring(lua, "type");
                lua_pushstring(lua, "channel");
                lua_settable(lua, -3);
            }break;
            case QUERY:
            {
                lua_pushstring(lua, "type");
                lua_pushstring(lua, "query");
                lua_settable(lua, -3);
            }break;
            case DCCCHAT:
            {
                lua_pushstring(lua, "type");
                lua_pushstring(lua, "dccchat");
                lua_settable(lua, -3);
            }break;
            case OTHER:
            {
                lua_pushstring(lua, "type");
                lua_pushstring(lua, "other");
                lua_settable(lua, -3);
            }break;
        }
        lua_pushstring(lua, "servername");
        lua_pushstring(lua, static_cast<IrcTabItem*>(pThis->tabbook->childAtIndex(id*2))->GetServerName().text());
        lua_settable(lua, -3);
        lua_pushstring(lua, "port");
        lua_pushinteger(lua, static_cast<IrcTabItem*>(pThis->tabbook->childAtIndex(id*2))->GetServerPort());
        lua_settable(lua, -3);
        lua_pushstring(lua, "nick");
        lua_pushstring(lua, static_cast<IrcTabItem*>(pThis->tabbook->childAtIndex(id*2))->GetNickName().text());
        lua_settable(lua, -3);
    }
    else
    {
        lua_newtable(lua);
        lua_pushstring(lua, "name");
        lua_pushnil(lua);
        lua_settable(lua, -3);
        lua_pushstring(lua, "type");
        lua_pushnil(lua);
        lua_settable(lua, -3);
        lua_pushstring(lua, "servername");
        lua_pushnil(lua);
        lua_settable(lua, -3);
        lua_pushstring(lua, "port");
        lua_pushnil(lua);
        lua_settable(lua, -3);
        lua_pushstring(lua, "nick");
        lua_pushnil(lua);
        lua_settable(lua, -3);
    }
    return  1;
#else
    return 0;
#endif
}

int dxirc::OnLuaSetTab(lua_State *lua)
{
#ifdef HAVE_LUA
    FXint number = 0;
    if(lua_isnumber(lua, 1))  number = lua_tointeger(lua, 1);
    else return 0;
    if(pThis->tabbook->numChildren() && number < pThis->tabbook->numChildren()/2)
    {
        pThis->tabbook->setCurrent(number, pThis->tabbook->numChildren() > number*2 ? TRUE : FALSE);
    }
    return  1;
#else
    return 0;
#endif    
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
#ifndef WIN32
    signal(SIGPIPE, SIG_IGN);
#endif
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

#ifdef HAVE_TRAY
    FXTrayApp app(PACKAGE, FXString::null);
#else
    FXApp app(PACKAGE, FXString::null);
#endif
    app.reg().setAsciiMode(TRUE);
    app.init(argc,argv);
    loadIcon = MakeAllIcons(&app, utils::GetIniFile());    
    new dxirc(&app);
    app.create();
    utils::SetAlias();
    return app.run();
}
