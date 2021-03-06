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
 #include <sys/file.h>
 #include <errno.h>
 #include <signal.h>
 #include <X11/Xlib.h>
#endif

#include <string>
#include "dxirc.h"
#include "icons.h"
#include "i18n.h"
#include "help.h"
#include "irctabitem.h"
#include "dcctabitem.h"
#include "configdialog.h"
#include "serverdialog.h"
#include "aliasdialog.h"
#include "utils.h"
#include "tetristabitem.h"
#include "scriptdialog.h"
#include "aboutdialog.h"
#ifdef HAVE_LIBNOTIFY
#include <libnotify/notify.h>
#endif

#define DISPLAY(app) ((Display*)((app)->getDisplay()))

// For handling Single Instance of Application
FXbool IsInstanceRunningAlready()
{
#ifdef WIN32
    HANDLE hSingleInstance = NULL;
    hSingleInstance = CreateMutex( NULL, FALSE,TEXT("dxirc.exe"));
    DWORD dwLastError = GetLastError();
    if(dwLastError == ERROR_ALREADY_EXISTS)
    {
        MessageBox(NULL, TEXT("dxirc already running"), TEXT("dxirc"), MB_OK | MB_ICONINFORMATION);
        CloseHandle(hSingleInstance);
        return TRUE;
    }
#else
    int pid_file = open(FXString(FXSystem::getTempDirectory()+PATHSEPSTRING+".dxirc.pid").text(), O_CREAT | O_RDWR, 0666);
    int rc = flock(pid_file, LOCK_EX | LOCK_NB);
    if(rc)
    {
        if(EWOULDBLOCK == errno)
        {
            fxmessage(_("dxirc already running"));
            FXString exec = FXPath::search(FXSystem::getExecPath(), "xmessage");
            if(exec.empty()) return TRUE;
            exec += " -center -timeout 3 ";
            exec += _("dxirc already running");
            system(exec.text());
            return TRUE;
        }
    }
#endif
    return FALSE;
}

int CompareTabs(const void **a,const void **b)
{
    dxTabItem *fa = (dxTabItem*)*a;
    dxTabItem *fb = (dxTabItem*)*b;
    return comparecase((fa->getType() == SERVER ? fa->getRealServerName() : fa->getRealServerName()+fa->getText()), (fb->getType() == SERVER ? fb->getRealServerName() : fb->getRealServerName()+fb->getText()));
}

/*from Goggles Music Manager
thanks Sander Jansen */
#ifdef ENABLE_NLS
#include <FXTranslator.h>
#include <fxdefs.h>

class dxTranslator : public FXTranslator
{
    FXDECLARE(dxTranslator)
private:
    dxTranslator(const dxTranslator&);
    dxTranslator & operator=(const dxTranslator&);
protected:
    dxTranslator() {
    }
public:
    /// Construct translator

    dxTranslator(FXApp* a) : FXTranslator(a) {
#ifndef WIN32
        setlocale(LC_MESSAGES, "");
        setlocale(LC_NUMERIC, "C");
#endif //WIN32
        bindtextdomain(PACKAGE, LOCALEDIR);
        bind_textdomain_codeset(PACKAGE, "UTF-8");
        textdomain(PACKAGE);
        utils::instance().debugLine(FXStringFormat("localedir: %s", LOCALEDIR));
    };

    virtual const FXchar* tr(const FXchar* context, const FXchar* message, const FXchar* hint = NULL) const;

    ~dxTranslator() {}
};

FXIMPLEMENT(dxTranslator, FXTranslator, NULL, 0)

const FXchar* dxTranslator::tr(const FXchar*, const FXchar* message, const FXchar*) const {
    return gettext(message);
}
#endif //ENABLE_NLS

FXDEFMAP(dxirc) dxircMap[] = {
    FXMAPFUNC(SEL_CLOSE,        0,                          dxirc::onCmdClose),
    FXMAPFUNC(SEL_COMMAND,      dxirc_QUIT,             dxirc::onCmdQuit),
    FXMAPFUNC(SEL_COMMAND,      dxirc_ABOUT,            dxirc::onCmdAbout),
    FXMAPFUNC(SEL_COMMAND,      dxirc_CONNECT,          dxirc::onCmdConnect),
    FXMAPFUNC(SEL_COMMAND,      dxirc_DISCONNECT,       dxirc::onCmdDisconnect),
    FXMAPFUNC(SEL_COMMAND,      dxirc_SERVERS,          dxirc::onCmdServers),
    FXMAPFUNC(SEL_COMMAND,      dxirc_TABS,             dxirc::onTabBook),
    FXMAPFUNC(SEL_COMMAND,      dxirc_CLEAR,            dxirc::onCmdClear),
    FXMAPFUNC(SEL_COMMAND,      dxirc_CLEARALL,         dxirc::onCmdClearAll),
    FXMAPFUNC(SEL_COMMAND,      dxirc_CLOSETAB,         dxirc::onCmdCloseTab),
    FXMAPFUNC(SEL_COMMAND,      dxirc_USERS,            dxirc::onCmdUsers),
    FXMAPFUNC(SEL_COMMAND,      dxirc_STATUS,           dxirc::onCmdStatus),
    FXMAPFUNC(SEL_COMMAND,      dxirc_HELP,             dxirc::onCmdHelp),
    FXMAPFUNC(SEL_COMMAND,      dxirc_OPTIONS,          dxirc::onCmdOptions),
    FXMAPFUNC(SEL_COMMAND,      dxirc_SELECTTAB,        dxirc::onCmdSelectTab),
    FXMAPFUNC(SEL_COMMAND,      dxirc_NEXTTAB,          dxirc::onCmdNextTab),
    FXMAPFUNC(SEL_COMMAND,      IrcTabItem_NEXTTAB,     dxirc::onCmdNextTab),
    FXMAPFUNC(SEL_COMMAND,      DccTabItem_NEXTTAB,     dxirc::onCmdNextTab),
    FXMAPFUNC(SEL_MOUSEWHEEL,   dxirc_TABS,             dxirc::onMouseWheel),
    FXMAPFUNC(SEL_COMMAND,      dxirc_NEXTUNREAD,       dxirc::onCmdNextUnread),
    FXMAPFUNC(SEL_COMMAND,      dxirc_ALIAS,            dxirc::onCmdAlias),
    FXMAPFUNC(SEL_COMMAND,      dxirc_LOG,              dxirc::onCmdLog),
    FXMAPFUNC(SEL_COMMAND,      dxirc_TRANSFERS,        dxirc::onCmdTransfers),
    FXMAPFUNC(SEL_COMMAND,      dxirc_TRAY,             dxirc::onTrayClicked),
    FXMAPFUNC(SEL_COMMAND,      dxirc_SCRIPTS,          dxirc::onCmdScripts),
    FXMAPFUNC(SEL_TIMEOUT,      dxirc_STIMEOUT,         dxirc::onStatusTimeout),
    FXMAPFUNC(SEL_COMMAND,      dxirc_TETRIS,           dxirc::onTetrisKey),
    FXMAPFUNC(SEL_COMMAND,      IrcEngine_SERVER,       dxirc::onIrcEvent),
    FXMAPFUNC(SEL_COMMAND,      DccEngine_DCC,          dxirc::onDccEvent),
    FXMAPFUNC(SEL_COMMAND,      IrcTabItem_CDIALOG,     dxirc::onCmdConnect),
    FXMAPFUNC(SEL_COMMAND,      IrcTabItem_CSERVER,     dxirc::onTabConnect),
    FXMAPFUNC(SEL_COMMAND,      IrcTabItem_CQUIT,       dxirc::onCmdQuit),
    FXMAPFUNC(SEL_COMMAND,      IrcTabItem_NEWMSG,      dxirc::onNewMsg),
    FXMAPFUNC(SEL_COMMAND,      DccTabItem_NEWMSG,      dxirc::onNewMsg),
    FXMAPFUNC(SEL_COMMAND,      IrcTabItem_LUA,         dxirc::onLua),
    FXMAPFUNC(SEL_COMMAND,      DccTabItem_LUA,         dxirc::onLua),
    FXMAPFUNC(SEL_COMMAND,      IrcTabItem_COMMAND,     dxirc::onIrcCommand),
    FXMAPFUNC(SEL_COMMAND,      DccTabItem_COMMAND,     dxirc::onIrcCommand),
    FXMAPFUNC(SEL_COMMAND,      IrcTabItem_MYMSG,       dxirc::onIrcMyMsg),
    FXMAPFUNC(SEL_COMMAND,      DccTabItem_MYMSG,       dxirc::onIrcMyMsg),
    FXMAPFUNC(SEL_COMMAND,      IrcTabItem_NEWTETRIS,   dxirc::onNewTetris),
    FXMAPFUNC(SEL_COMMAND,      DccTabItem_NEWTETRIS,   dxirc::onNewTetris),
    FXMAPFUNC(SEL_COMMAND,      IrcTabItem_ADDICOMMAND, dxirc::onAddIgnoreCommand),
    FXMAPFUNC(SEL_COMMAND,      IrcTabItem_RMICOMMAND,  dxirc::onRemoveIgnoreCommand),
    FXMAPFUNC(SEL_COMMAND,      IrcTabItem_ADDIUSER,    dxirc::onAddIgnoreUser),
    FXMAPFUNC(SEL_COMMAND,      IrcTabItem_RMIUSER,     dxirc::onRemoveIgnoreUser),
    FXMAPFUNC(SEL_COMMAND,      DccDialog_DCCCANCEL,    dxirc::onCmdDccCancel),
    FXMAPFUNC(SEL_COMMAND,      dxirc_SPELL,            dxirc::onCmdSpell),
    FXMAPFUNC(SEL_COMMAND,      dxirc_FORCEFOCUS,       dxirc::onCmdForceFocus),
    FXMAPFUNC(SEL_TIMEOUT,      dxirc_AWAYTIMEOUT,      dxirc::onAwayTimeout),
};

FXIMPLEMENT(dxirc, FXMainWindow, dxircMap, ARRAYNUMBER(dxircMap))

dxirc *dxirc::_pThis = NULL;
#ifdef HAVE_LUA
static luaL_reg dxircFunctions[] = {
    {"AddCommand",      dxirc::onLuaAddCommand},
    {"AddEvent",        dxirc::onLuaAddEvent},
    {"AddMyMsg",        dxirc::onLuaAddMyMsg},
    {"AddNewTab",       dxirc::onLuaAddNewTab},
    {"AddDxircQuit",    dxirc::onLuaAddDxircQuit},
    {"AddAll",          dxirc::onLuaAddAll},
    {"RemoveName",      dxirc::onLuaRemoveName},
    {"Command",         dxirc::onLuaCommand},
    {"Print",           dxirc::onLuaPrint},
    {"GetServers",      dxirc::onLuaGetServers},
    {"GetTab",          dxirc::onLuaGetTab},
    {"GetCurrentTab",   dxirc::onLuaGetCurrentTab},
    {"GetVersion",      dxirc::onLuaGetVersion},
    {"GetTabInfo",      dxirc::onLuaGetTabInfo},
    {"SetTab",          dxirc::onLuaSetTab},
    {"CreateTab",       dxirc::onLuaCreateTab},
    {"GetTabCount",     dxirc::onLuaGetTabCount},
    {"Clear",           dxirc::onLuaClear},
    {NULL,              NULL}
};
#endif //HAVE_LUA

dxirc::dxirc(FXApp *app)
    : FXMainWindow(app, PACKAGE, 0, 0, DECOR_ALL, 0, 0, 800, 600), m_app(app), m_trayIcon(NULL)
{
    setIcon(ICO_BIG);
    setMiniIcon(ICO_SMALL);

    m_ircFont = NULL;
    m_viewer = NULL;
    m_transfers = NULL;
    m_traymenu = NULL;
    m_lastToken = 0;
    m_lastID = 0;

    readConfig();

    m_menubar = new FXMenuBar(this, FRAME_RAISED|LAYOUT_SIDE_TOP|LAYOUT_FILL_X);

    m_servermenu = new FXMenuPane(this);
    new FXMenuCommand(m_servermenu, _("&Server list\tF2"), ICO_SERVERLIST, this, dxirc_SERVERS);
    new FXMenuCommand(m_servermenu, _("Quick &connect\tF7"), ICO_CONNECT, this, dxirc_CONNECT);
    m_disconnect = new FXMenuCommand(m_servermenu, _("&Disconnect\tCtrl-D"), ICO_DISCONNECT, this, dxirc_DISCONNECT);
    m_disconnect->disable();
    new FXMenuSeparator(m_servermenu);
    new FXMenuCommand(m_servermenu, _("DCC &transfers\tCtrl-T"), ICO_TRANSFER, this, dxirc_TRANSFERS);
#ifdef HAVE_LUA
    new FXMenuSeparator(m_servermenu);
    new FXMenuCommand(m_servermenu, _("S&cripts\tCtrl-S"), ICO_SCRIPT, this, dxirc_SCRIPTS);
#endif //HAVE_LUA
    new FXMenuSeparator(m_servermenu);
    m_logviewer = new FXMenuCommand(m_servermenu, _("&Log viewer\tCtrl-G"), ICO_LOGS, this, dxirc_LOG);
    if(!m_logging) m_logviewer->disable();
    new FXMenuSeparator(m_servermenu);
#ifdef WIN32
    new FXMenuCommand(m_servermenu, _("&Quit\tAlt-F4"), ICO_QUIT, this, dxirc_QUIT);
#else
    new FXMenuCommand(m_servermenu, _("&Quit\tCtrl-Q"), ICO_QUIT, this, dxirc_QUIT);
#endif //WIN32
    new FXMenuTitle(m_menubar, _("&Server"), NULL, m_servermenu);

    m_editmenu = new FXMenuPane(this);
    m_closeTab = new FXMenuCommand(m_editmenu, _("Close current tab\tCtrl-W"), ICO_CLOSE, this, dxirc_CLOSETAB);
    m_closeTab->disable();
    new FXMenuSeparator(m_editmenu);
    m_clearTab = new FXMenuCommand(m_editmenu, _("Clear window\tCtrl-L"), ICO_CLEAR, this, dxirc_CLEAR);
    m_clearTabs = new FXMenuCommand(m_editmenu, _("Clear all windows\tCtrl-Shift-L"), NULL, this, dxirc_CLEARALL);
    new FXMenuSeparator(m_editmenu);
    m_usersShown = utils::instance().getBoolIniEntry("SETTINGS", "usersShown", TRUE);
    m_users = new FXMenuCheck(m_editmenu, _("Users list\tCtrl-U\tShow/Hide users list"), this, dxirc_USERS);
    m_users->setCheck(m_usersShown);
#ifdef HAVE_ENCHANT
    m_spellCombo = new FXMenuCheck(m_editmenu, _("Spellchecking language list\tCtrl-P\tShow/Hide spellchecking language list"), this, dxirc_SPELL);
    if(utils::instance().getLangsNum())
        m_showSpellCombo = utils::instance().getBoolIniEntry("SETTINGS", "showSpellCombo", TRUE);
    else
    {
        m_showSpellCombo = FALSE;
        m_spellCombo->disable();
    }
    m_spellCombo->setCheck(m_showSpellCombo);
#endif //HAVE_ENCHANT
    m_status = new FXMenuCheck(m_editmenu, _("Status bar"), this, dxirc_STATUS);
    m_status->setCheck(m_statusShown);
    new FXMenuCommand(m_editmenu, _("&Aliases"), NULL, this, dxirc_ALIAS);
    new FXMenuCommand(m_editmenu, _("&Preferences"), ICO_OPTION, this, dxirc_OPTIONS);
    new FXMenuTitle(m_menubar, _("&Edit"), NULL, m_editmenu);

    m_helpmenu = new FXMenuPane(this);
    new FXMenuCommand(m_helpmenu, _("&Help\tF1"), ICO_HELP, this, dxirc_HELP);
    new FXMenuCommand(m_helpmenu, _("&About..."), NULL, this, dxirc_ABOUT);
    new FXMenuTitle(m_menubar, _("&Help"), NULL, m_helpmenu);

    m_mainframe = new FXVerticalFrame(this, LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 1,1,1,1);

    m_tabbook = new dxTabBook(m_mainframe, this, dxirc_TABS, PACK_UNIFORM_WIDTH|PACK_UNIFORM_HEIGHT|LAYOUT_FILL_X|LAYOUT_FILL_Y);

    IrcEngine *server = new IrcEngine(app, this, "", "");
    server->setUsersList(m_usersList);
    server->setReconnect(m_reconnect);
    server->setNumberAttempt(m_numberAttempt);
    server->setDelayAttempt(m_delayAttempt);
    m_ircengines.append(server);

    m_statusbar = new FXHorizontalFrame(m_mainframe, LAYOUT_LEFT|JUSTIFY_LEFT|LAYOUT_FILL_X|FRAME_NONE, 0,0,0,0, 1,1,1,1);
    FXHorizontalFrame *hframe=new FXHorizontalFrame(m_statusbar, LAYOUT_LEFT|JUSTIFY_LEFT|LAYOUT_FILL_X|FRAME_SUNKEN, 0,0,0,0, 0,0,0,0);
    m_statuslabel = new FXLabel(hframe, "dxirc", NULL, LAYOUT_LEFT|JUSTIFY_LEFT);
    if (m_statusShown)
        m_statusbar->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_SHOW), NULL);
    else
        m_statusbar->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_HIDE), NULL);

#ifdef HAVE_TRAY
    if(m_useTray)
    {
        m_trayIcon = new FXTrayIcon(app, "dxirc", ICO_TRAY, 0, this, dxirc_TRAY, TRAY_CMD_ON_LEFT|TRAY_MENU_ON_RIGHT);
        if(m_traymenu == NULL)
            m_traymenu = new FXPopup(m_trayIcon);
        new FXMenuCommand(m_traymenu, _("&Quit"), ICO_QUIT, this, dxirc_QUIT);
        m_trayIcon->setMenu(m_traymenu);
    }
#endif //HAVE_TRAY

    new FXToolTip(app,0);
#ifndef HAVE_LIBNOTIFY
    m_wnotify = new dxEXNotify(m_app, ICO_BIG, PACKAGE);
    m_wnotify->setTextColor(m_appTheme.notifyfore);
    m_wnotify->setBackColor(m_appTheme.notifyback);
#endif

    updateTheme();
    updateFont(m_fontSpec);
    updateTabs(m_useSmileys && (FXint)m_smileysMap.size());
    updateTabPosition();
    updateTrayColor();

    getAccelTable()->addAccel(MKUINT(KEY_1, ALTMASK), this, FXSEL(SEL_COMMAND, dxirc_SELECTTAB));
    getAccelTable()->addAccel(MKUINT(KEY_2, ALTMASK), this, FXSEL(SEL_COMMAND, dxirc_SELECTTAB));
    getAccelTable()->addAccel(MKUINT(KEY_3, ALTMASK), this, FXSEL(SEL_COMMAND, dxirc_SELECTTAB));
    getAccelTable()->addAccel(MKUINT(KEY_4, ALTMASK), this, FXSEL(SEL_COMMAND, dxirc_SELECTTAB));
    getAccelTable()->addAccel(MKUINT(KEY_5, ALTMASK), this, FXSEL(SEL_COMMAND, dxirc_SELECTTAB));
    getAccelTable()->addAccel(MKUINT(KEY_6, ALTMASK), this, FXSEL(SEL_COMMAND, dxirc_SELECTTAB));
    getAccelTable()->addAccel(MKUINT(KEY_7, ALTMASK), this, FXSEL(SEL_COMMAND, dxirc_SELECTTAB));
    getAccelTable()->addAccel(MKUINT(KEY_8, ALTMASK), this, FXSEL(SEL_COMMAND, dxirc_SELECTTAB));
    getAccelTable()->addAccel(MKUINT(KEY_9, ALTMASK), this, FXSEL(SEL_COMMAND, dxirc_SELECTTAB));
    getAccelTable()->addAccel(MKUINT(KEY_Tab, CONTROLMASK), this, FXSEL(SEL_COMMAND, dxirc_NEXTTAB));
    getAccelTable()->addAccel(MKUINT(KEY_n, CONTROLMASK), this, FXSEL(SEL_COMMAND, dxirc_NEXTUNREAD));
    getAccelTable()->addAccel(MKUINT(KEY_N, CONTROLMASK), this, FXSEL(SEL_COMMAND, dxirc_NEXTUNREAD));
    getAccelTable()->addAccel(MKUINT(KEY_f, CONTROLMASK), this, FXSEL(SEL_COMMAND, dxirc_FORCEFOCUS));
    getAccelTable()->addAccel(MKUINT(KEY_F, CONTROLMASK), this, FXSEL(SEL_COMMAND, dxirc_FORCEFOCUS));
    getAccelTable()->addAccel(KEY_n, this, FXSEL(SEL_COMMAND, dxirc_TETRIS));
    getAccelTable()->addAccel(KEY_N, this, FXSEL(SEL_COMMAND, dxirc_TETRIS));
    getAccelTable()->addAccel(KEY_p, this, FXSEL(SEL_COMMAND, dxirc_TETRIS));
    getAccelTable()->addAccel(KEY_P, this, FXSEL(SEL_COMMAND, dxirc_TETRIS));
    getAccelTable()->addAccel(KEY_i, this, FXSEL(SEL_COMMAND, dxirc_TETRIS));
    getAccelTable()->addAccel(KEY_I, this, FXSEL(SEL_COMMAND, dxirc_TETRIS));
    getAccelTable()->addAccel(KEY_l, this, FXSEL(SEL_COMMAND, dxirc_TETRIS));
    getAccelTable()->addAccel(KEY_L, this, FXSEL(SEL_COMMAND, dxirc_TETRIS));
    getAccelTable()->addAccel(KEY_k, this, FXSEL(SEL_COMMAND, dxirc_TETRIS));
    getAccelTable()->addAccel(KEY_K, this, FXSEL(SEL_COMMAND, dxirc_TETRIS));
    getAccelTable()->addAccel(KEY_j, this, FXSEL(SEL_COMMAND, dxirc_TETRIS));
    getAccelTable()->addAccel(KEY_J, this, FXSEL(SEL_COMMAND, dxirc_TETRIS));
}

dxirc::~dxirc()
{
    m_app->removeTimeout(this, dxirc_STIMEOUT);
    m_app->removeTimeout(this, dxirc_AWAYTIMEOUT);
    delete ICO_BIG;
    delete ICO_SMALL;
    delete ICO_IRCADMIN;
    delete ICO_IRCAWAYADMIN;
    delete ICO_IRCAWAYHALFOP;
    delete ICO_IRCAWAYNORMAL;
    delete ICO_IRCAWAYOP;
    delete ICO_IRCAWAYOWNER;
    delete ICO_IRCAWAYVOICE;
    delete ICO_IRCHALFOP;
    delete ICO_IRCNORMAL;
    delete ICO_IRCOP;
    delete ICO_IRCOWNER;
    delete ICO_IRCVOICE;
    delete ICO_SERVERLIST;
    delete ICO_CONNECT;
    delete ICO_DISCONNECT;
    delete ICO_QUIT;
    delete ICO_CLOSE;
    delete ICO_OPTION;
    delete ICO_HELP;
    delete ICO_SERVER;
    delete ICO_CHANNEL;
    delete ICO_QUERY;
    delete ICO_CLEAR;
    delete ICO_FLAG;
    delete ICO_TRAY;
    delete ICO_NEWMSG;
    delete ICO_NEWFILE;
    delete ICO_QUERYNEWMSG;
    delete ICO_CHANNELNEWMSG;
    delete ICO_CLOSEFOLDER;
    delete ICO_OPENFOLDER;
    delete ICO_FILE;
    delete ICO_CANCEL;
    delete ICO_FINISH;
    delete ICO_DOWN;
    delete ICO_UP;
    delete ICO_DCC;
    delete ICO_DCCNEWMSG;
    delete ICO_LOGS;
    delete ICO_SCRIPT;
    delete ICO_TRANSFER;
    delete ICO_PLAY;
    while(m_smileys.no())
    {
        delete m_smileys[0].icon;
        m_smileys.erase(0);
    }
    delete m_servermenu;
    delete m_editmenu;
    delete m_helpmenu;
#ifdef HAVE_TRAY
    if(m_traymenu)
    {
        delete m_traymenu;
    }
#endif //HAVE_TRAY
    delete m_ircFont;
    _pThis = NULL;
}

dxirc* dxirc::instance()
{
    return _pThis;
}

void dxirc::create()
{
    FXMainWindow::create();
    createIrcTab("(server)", ICO_SERVER, SERVER, m_ircengines[0]);
    FXbool maximized = utils::instance().getBoolIniEntry("SETTINGS", "maximized", FALSE);
    if(maximized) maximize(TRUE);
    //Checking for screen resolution and correction size, position
    else
    {
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
    }
    show();
    readServersConfig();
    _pThis = this;
    autoloadScripts();
    m_app->addTimeout(this, dxirc_AWAYTIMEOUT, 180000);
}

// Flash the window to get user's attention
// Taken from fox development version 1.7
void dxirc::flash(FXbool yes)
{
    if(xid)
    {
#ifdef WIN32
        FlashWindow((HWND)xid, true);
#else
        XEvent se;
        se.xclient.type=ClientMessage;
        se.xclient.display=DISPLAY(m_app);
        se.xclient.message_type=XInternAtom(DISPLAY(m_app), "_NET_WM_STATE", 0);;
        se.xclient.format=32;
        se.xclient.window=xid;
        se.xclient.data.l[0]=yes;   // 0=_NET_WM_STATE_REMOVE, 1=_NET_WM_STATE_ADD, 2=_NET_WM_STATE_TOGGLE
        se.xclient.data.l[1]=XInternAtom(DISPLAY(m_app), "_NET_WM_STATE_DEMANDS_ATTENTION", 0);
        se.xclient.data.l[2]=0;
        se.xclient.data.l[3]=0;
        se.xclient.data.l[4]=0;
        XSendEvent(DISPLAY(m_app),XDefaultRootWindow(DISPLAY(m_app)),False,SubstructureRedirectMask|SubstructureNotifyMask,&se);
#endif //WIN32
    }
}

void dxirc::readConfig()
{
    FXString ircfontspec;
    FXSettings set;
    set.parseFile(utils::instance().getIniFile(), TRUE);
    FXint xx=set.readIntEntry("SETTINGS","x",50);
    FXint yy=set.readIntEntry("SETTINGS","y",50);
    FXint ww=set.readIntEntry("SETTINGS","w",400);
    FXint hh=set.readIntEntry("SETTINGS","h",300);
    m_appTheme.base = set.readColorEntry("SETTINGS", "basecolor", m_app->getBaseColor());
    m_appTheme.back = set.readColorEntry("SETTINGS", "backcolor", m_app->getBackColor());
    m_appTheme.border = set.readColorEntry("SETTINGS", "bordercolor", m_app->getBorderColor());
    m_appTheme.fore = set.readColorEntry("SETTINGS", "forecolor", m_app->getForeColor());
    m_appTheme.menuback = set.readColorEntry("SETTINGS", "selmenubackcolor", m_app->getSelMenuBackColor());
    m_appTheme.menufore = set.readColorEntry("SETTINGS", "selmenutextcolor", m_app->getSelMenuTextColor());
    m_appTheme.selback = set.readColorEntry("SETTINGS", "selbackcolor", m_app->getSelbackColor());
    m_appTheme.selfore = set.readColorEntry("SETTINGS", "selforecolor", m_app->getSelforeColor());
    m_appTheme.tipback = set.readColorEntry("SETTINGS", "tipbackcolor", m_app->getTipbackColor());
    m_appTheme.tipfore = set.readColorEntry("SETTINGS", "tipforecolor", m_app->getTipforeColor());
    m_appTheme.notifyback = set.readColorEntry("SETTINGS", "notifybackcolor", m_app->getTipbackColor());
    m_appTheme.notifyfore = set.readColorEntry("SETTINGS", "notifyforecolor", m_app->getTipforeColor());
    m_appTheme.hilite = set.readColorEntry("SETTINGS", "hilitecolor", m_app->getHiliteColor());
    m_appTheme.shadow = set.readColorEntry("SETTINGS", "shadowcolor", m_app->getShadowColor());
    m_trayColor = set.readColorEntry("SETTINGS", "traycolor", m_appTheme.base);
    m_unreadColor = set.readColorEntry("SETTINGS", "unreadcolor", FXRGB(0,0,255));
    m_highlightColor = set.readColorEntry("SETTINGS", "highlightcolor", FXRGB(255,0,0));
    m_fontSpec = set.readStringEntry("SETTINGS", "normalfont", m_app->getNormalFont()->getFont().text());
    m_statusShown = set.readBoolEntry("SETTINGS", "statusShown", TRUE);
    m_tabPosition = set.readIntEntry("SETTINGS", "tabPosition", 0);
    m_notifyPosition = set.readIntEntry("SETTINGS", "notifyPosition", 3);
    m_commandsList = set.readStringEntry("SETTINGS", "commandsList");
    m_themePath = utils::instance().checkThemePath(set.readStringEntry("SETTINGS", "themePath", DXIRC_DATADIR PATHSEPSTRING "icons" PATHSEPSTRING "default"));
    m_themesList = utils::instance().checkThemesList(set.readStringEntry("SETTINGS", "themesList", FXString(m_themePath+";").text()));
    m_colors.text = set.readColorEntry("SETTINGS", "textColor", FXRGB(255,255,255));
    m_colors.back = set.readColorEntry("SETTINGS", "textBackColor", FXRGB(0,0,0));
    m_colors.user = set.readColorEntry("SETTINGS", "userColor", FXRGB(43,57,35));
    m_colors.action = set.readColorEntry("SETTINGS", "actionsColor", FXRGB(255,165,0));
    m_colors.notice = set.readColorEntry("SETTINGS", "noticeColor", FXRGB(0,163,215));
    m_colors.error = set.readColorEntry("SETTINGS", "errorColor", FXRGB(255,0,0));
    m_colors.hilight = set.readColorEntry("SETTINGS", "hilightColor", FXRGB(11,154,11));
    m_colors.link = set.readColorEntry("SETTINGS", "linkColor", FXRGB(142,196,210));
    ircfontspec = set.readStringEntry("SETTINGS", "ircFont", "");
    m_sameCmd = set.readBoolEntry("SETTINGS", "sameCmd", FALSE);
    m_sameList = set.readBoolEntry("SETTINGS", "sameList", FALSE);
    m_coloredNick = set.readBoolEntry("SETTINGS", "coloredNick", TRUE);
    if(!ircfontspec.empty())
    {
        m_ircFont = new FXFont(m_app, ircfontspec);
        m_ircFont->create();
    }
    else
    {
        m_app->getNormalFont()->create();
        FXFontDesc fontdescription;
        m_app->getNormalFont()->getFontDesc(fontdescription);
        m_ircFont = new FXFont(m_app,fontdescription);
        m_ircFont->create();
    }
    m_maxAway = set.readIntEntry("SETTINGS", "maxAway", 200);
    m_logging = set.readBoolEntry("SETTINGS", "logging", FALSE);
    m_ownServerWindow = set.readBoolEntry("SETTINGS", "serverWindow", TRUE);
#ifdef HAVE_TRAY
    m_useTray = set.readBoolEntry("SETTINGS", "tray", TRUE);
#else
    m_useTray = FALSE;
#endif //HAVE_TRAY
    if(m_useTray)
        m_closeToTray = set.readBoolEntry("SETTINGS", "closeToTray", TRUE);
    else
        m_closeToTray = FALSE;
    m_reconnect = set.readBoolEntry("SETTINGS", "reconnect", FALSE);
    m_numberAttempt = set.readIntEntry("SETTINGS", "numberAttempt", 1);
    m_delayAttempt = set.readIntEntry("SETTINGS", "delayAttempt", 20);
    m_nickCompletionChar = FXString(set.readStringEntry("SETTINGS", "nickCompletionChar", ":")).left(1);
    m_tempServerWindow = m_ownServerWindow;
    m_logPath = set.readStringEntry("SETTINGS", "logPath");
    if(m_logging && !FXStat::exists(m_logPath)) m_logging = FALSE;
    m_dccPath = set.readStringEntry("SETTINGS", "dccPath");
    if(!FXStat::exists(m_dccPath)) m_dccPath = FXSystem::getHomeDirectory();
    m_autoDccChat = set.readBoolEntry("SETTINGS", "autoDccChat", FALSE);
    m_autoDccFile = set.readBoolEntry("SETTINGS", "autoDccFile", FALSE);
    FXint usersNum = set.readIntEntry("USERS", "number", 0);
    m_usersList.clear();
    if(usersNum)
    {
        
        for(FXint i=0; i<usersNum; i++)
        {
            IgnoreUser user;
            user.nick = set.readStringEntry(FXStringFormat("USER%d", i).text(), "nick", FXStringFormat("xxx%d", i).text());
            user.channel = set.readStringEntry(FXStringFormat("USER%d", i).text(), "channel", "all");
            user.network = set.readStringEntry(FXStringFormat("USER%d", i).text(), "network", "all");
            m_usersList.append(user);
        }
    }
    FXint friendsNum = set.readIntEntry("FRIENDS", "number", 0);
    m_friendsList.clear();
    if(friendsNum)
    {

        for(FXint i=0; i<friendsNum; i++)
        {
            IgnoreUser user;
            user.nick = set.readStringEntry(FXStringFormat("FRIEND%d", i).text(), "nick", FXStringFormat("xxx%d", i).text());
            user.channel = set.readStringEntry(FXStringFormat("FRIEND%d", i).text(), "channel", "all");
            user.network = set.readStringEntry(FXStringFormat("FRIEND%d", i).text(), "network", "all");
            m_friendsList.append(user);
        }
    }
#ifdef HAVE_LUA
    m_autoload = set.readBoolEntry("SETTINGS", "autoload", FALSE);
#else
    m_autoload = FALSE;
#endif //HAVE_LUA
    m_autoloadPath = set.readStringEntry("SETTINGS", "autoloadPath");
    if(m_autoload && !FXStat::exists(utils::instance().isUtf8(m_autoloadPath.text(), m_autoloadPath.length()) ? m_autoloadPath : utils::instance().localeToUtf8(m_autoloadPath))) m_autoload = FALSE;
    m_dccIP = set.readStringEntry("SETTINGS", "dccIP");
    FXRex rex("\\l");
    if(m_dccIP.contains('.')!=3 || rex.match(m_dccIP))
        m_dccIP = "";
    m_dccPortD = set.readIntEntry("SETTINGS", "dccPortD");
    m_dccPortH = set.readIntEntry("SETTINGS", "dccPortH");
    m_dccTimeout = set.readIntEntry("SETTINGS", "dccTimeout", 66);
    m_sounds = set.readBoolEntry("SETTINGS", "sounds", FALSE);
    m_soundConnect = set.readBoolEntry("SETTINGS", "soundConnect", FALSE);
    m_soundDisconnect = set.readBoolEntry("SETTINGS", "soundDisconnect", FALSE);
    m_soundMessage = set.readBoolEntry("SETTINGS", "soundMessage", FALSE);
    m_pathConnect = set.readStringEntry("SETTINGS", "pathConnect", DXIRC_DATADIR PATHSEPSTRING "sounds" PATHSEPSTRING "connected.wav");
    m_pathDisconnect = set.readStringEntry("SETTINGS", "pathDisconnect", DXIRC_DATADIR PATHSEPSTRING "sounds" PATHSEPSTRING "disconnected.wav");
    m_pathMessage = set.readStringEntry("SETTINGS", "pathMessage", DXIRC_DATADIR PATHSEPSTRING "sounds" PATHSEPSTRING "message.wav");
    m_notify = set.readBoolEntry("SETTINGS", "notify", FALSE);
    m_notifyConnect = set.readBoolEntry("SETTINGS", "notifyConnect", FALSE);
    m_notifyDisconnect = set.readBoolEntry("SETTINGS", "notifyDisconnect", FALSE);
    m_notifyMessage = set.readBoolEntry("SETTINGS", "notifyMessage", FALSE);
    m_stripColors = set.readBoolEntry("SETTINGS", "stripColors", TRUE);
    m_useSmileys = set.readBoolEntry("SETTINGS", "useSmileys", FALSE);
    FXint smileysNum = set.readIntEntry("SMILEYS", "number", 0);
    m_smileysMap.clear();
    if(smileysNum)
    {

        for(FXint i=0; i<smileysNum; i++)
        {
            FXString key, value;
            key = set.readStringEntry("SMILEYS", FXStringFormat("smiley%d", i).text(), FXStringFormat("%d)", i).text());
            value = set.readStringEntry("SMILEYS", FXStringFormat("path%d", i).text(), "");
            if(!key.empty())
                m_smileysMap.insert(StringPair(key, value));
        }
    }
#ifdef HAVE_ENCHANT
    if(utils::instance().getLangsNum())
        m_useSpell = set.readBoolEntry("SETTINGS", "useSpell", TRUE);
    else
        m_useSpell = FALSE;
#else
    m_useSpell = FALSE;
#endif //HAVE_ENCHANT
    m_oneInstance = set.readBoolEntry("SETTINGS", "oneinstance", FALSE);
    setX(xx);
    setY(yy);
    setWidth(ww);
    setHeight(hh);
}

void dxirc::readServersConfig()
{
    FXSettings set;
    set.parseFile(utils::instance().getIniFile(), TRUE);
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
            server.passwd = utils::instance().decrypt(set.readStringEntry(FXStringFormat("SERVER%d", i).text(), "hes", ""));
            server.channels = set.readStringEntry(FXStringFormat("SERVER%d", i).text(), "channels", "");
            server.commands = set.readStringEntry(FXStringFormat("SERVER%d", i).text(), "commands", "");
            server.autoConnect = set.readBoolEntry(FXStringFormat("SERVER%d", i).text(), "autoconnect", FALSE);
            server.useSsl = set.readBoolEntry(FXStringFormat("SERVER%d", i).text(), "ssl", FALSE);
            if(server.autoConnect)
            {
                connectServer(server.hostname, server.port, server.passwd, server.nick, server.realname, server.channels, server.commands, server.useSsl);
            }
            m_serverList.append(server);
        }
    }
}

void dxirc::saveConfig()
{
    m_app->reg().setModified(FALSE);
    FXSettings set;
    //set.clear();
    set.writeIntEntry("SERVERS", "number", m_serverList.no());
    if(m_serverList.no())
    {
        for(FXint i=0; i<m_serverList.no(); i++)
        {
            set.writeStringEntry(FXStringFormat("SERVER%d", i).text(), "hostname", m_serverList[i].hostname.text());
            set.writeIntEntry(FXStringFormat("SERVER%d", i).text(), "port", m_serverList[i].port);
            set.writeStringEntry(FXStringFormat("SERVER%d", i).text(), "nick", m_serverList[i].nick.text());
            set.writeStringEntry(FXStringFormat("SERVER%d", i).text(), "realname", m_serverList[i].realname.text());
            set.writeStringEntry(FXStringFormat("SERVER%d", i).text(), "hes", utils::instance().encrypt(m_serverList[i].passwd).text());
            set.writeStringEntry(FXStringFormat("SERVER%d", i).text(), "channels", m_serverList[i].channels.text());
            set.writeStringEntry(FXStringFormat("SERVER%d", i).text(), "commands", m_serverList[i].commands.text());
            set.writeBoolEntry(FXStringFormat("SERVER%d", i).text(), "autoconnect", m_serverList[i].autoConnect);
            set.writeBoolEntry(FXStringFormat("SERVER%d", i).text(), "ssl", m_serverList[i].useSsl);
        }
    }
    set.writeBoolEntry("SETTINGS", "usersShown", m_usersShown);
    set.writeBoolEntry("SETTINGS", "statusShown", m_statusShown);
    set.writeStringEntry("SETTINGS", "commandsList", m_commandsList.text());
    set.writeStringEntry("SETTINGS", "themePath", m_themePath.text());
    set.writeStringEntry("SETTINGS", "themesList", m_themesList.text());
    set.writeColorEntry("SETTINGS", "textColor", m_colors.text);
    set.writeColorEntry("SETTINGS", "textBackColor", m_colors.back);
    set.writeColorEntry("SETTINGS", "userColor", m_colors.user);
    set.writeColorEntry("SETTINGS", "actionsColor", m_colors.action);
    set.writeColorEntry("SETTINGS", "noticeColor", m_colors.notice);
    set.writeColorEntry("SETTINGS", "errorColor", m_colors.error);
    set.writeColorEntry("SETTINGS", "hilightColor", m_colors.hilight);
    set.writeColorEntry("SETTINGS", "linkColor", m_colors.link);
    set.writeStringEntry("SETTINGS", "ircFont", m_ircFont->getFont().text());
    set.writeIntEntry("SETTINGS", "maxAway", m_maxAway);
    set.writeBoolEntry("SETTINGS", "logging", m_logging);
    set.writeBoolEntry("SETTINGS", "sameCmd", m_sameCmd);
    set.writeBoolEntry("SETTINGS", "sameList", m_sameList);
    set.writeBoolEntry("SETTINGS", "coloredNick", m_coloredNick);
    set.writeBoolEntry("SETTINGS", "tray", m_useTray);
    set.writeBoolEntry("SETTINGS", "closeToTray", m_closeToTray);
    set.writeBoolEntry("SETTINGS", "reconnect", m_reconnect);
    set.writeIntEntry("SETTINGS", "numberAttempt", m_numberAttempt);
    set.writeIntEntry("SETTINGS", "delayAttempt", m_delayAttempt);
    if(m_ownServerWindow == m_tempServerWindow) set.writeBoolEntry("SETTINGS", "serverWindow", m_ownServerWindow);
    else set.writeBoolEntry("SETTINGS", "serverWindow", m_tempServerWindow);
    set.writeStringEntry("SETTINGS", "logPath", m_logPath.text());
    set.writeStringEntry("SETTINGS", "dccPath", m_dccPath.text());
    set.writeStringEntry("SETTINGS", "nickCompletionChar", m_nickCompletionChar.text());
    set.writeIntEntry("USERS", "number", m_usersList.no());
    if(m_usersList.no())
    {

        for(FXint i=0; i<m_usersList.no(); i++)
        {
            set.writeStringEntry(FXStringFormat("USER%d", i).text(), "nick", m_usersList[i].nick.text());
            set.writeStringEntry(FXStringFormat("USER%d", i).text(), "channel", m_usersList[i].channel.text());
            set.writeStringEntry(FXStringFormat("USER%d", i).text(), "network", m_usersList[i].network.text());
        }
    }
    set.writeIntEntry("FRIENDS", "number", m_friendsList.no());
    if(m_friendsList.no())
    {

        for(FXint i=0; i<m_friendsList.no(); i++)
        {
            set.writeStringEntry(FXStringFormat("FRIEND%d", i).text(), "nick", m_friendsList[i].nick.text());
            set.writeStringEntry(FXStringFormat("FRIEND%d", i).text(), "channel", m_friendsList[i].channel.text());
            set.writeStringEntry(FXStringFormat("FRIEND%d", i).text(), "network", m_friendsList[i].network.text());
        }
    }
    set.writeIntEntry("SETTINGS","x",getX());
    set.writeIntEntry("SETTINGS","y",getY());
    set.writeIntEntry("SETTINGS","w",getWidth());
    set.writeIntEntry("SETTINGS","h",getHeight());
    set.writeBoolEntry("SETTINGS", "maximized", isMaximized());
    set.writeIntEntry("SETTINGS", "tabPosition", m_tabPosition);
    set.writeIntEntry("SETTINGS", "notifyPosition", m_notifyPosition);
    set.writeColorEntry("SETTINGS", "basecolor", m_appTheme.base);
    set.writeColorEntry("SETTINGS", "bordercolor", m_appTheme.border);
    set.writeColorEntry("SETTINGS", "backcolor", m_appTheme.back);
    set.writeColorEntry("SETTINGS", "forecolor", m_appTheme.fore);
    set.writeColorEntry("SETTINGS", "hilitecolor", m_appTheme.hilite);
    set.writeColorEntry("SETTINGS", "shadowcolor", m_appTheme.shadow);
    set.writeColorEntry("SETTINGS", "selforecolor", m_appTheme.selfore);
    set.writeColorEntry("SETTINGS", "selbackcolor", m_appTheme.selback);
    set.writeColorEntry("SETTINGS", "tipforecolor", m_appTheme.tipfore);
    set.writeColorEntry("SETTINGS", "tipbackcolor", m_appTheme.tipback);
    set.writeColorEntry("SETTINGS", "notifyforecolor", m_appTheme.notifyfore);
    set.writeColorEntry("SETTINGS", "notifybackcolor", m_appTheme.notifyback);
    set.writeColorEntry("SETTINGS", "selmenutextcolor", m_appTheme.menufore);
    set.writeColorEntry("SETTINGS", "selmenubackcolor", m_appTheme.menuback);
    set.writeColorEntry("SETTINGS", "traycolor", m_trayColor);
    set.writeColorEntry("SETTINGS", "unreadcolor", m_unreadColor);
    set.writeColorEntry("SETTINGS", "highlightcolor", m_highlightColor);
    set.writeStringEntry("SETTINGS", "normalfont", m_app->getNormalFont()->getFont().text());
    dxStringMap aliases = utils::instance().getAliases();
    set.writeIntEntry("ALIASES", "number", (FXint)aliases.size());
    if((FXint)aliases.size())
    {
        StringIt it;
        FXint i;
        for(i=0,it=aliases.begin(); it!=aliases.end(); it++,i++)
        {
            set.writeStringEntry("ALIASES", FXStringFormat("key%d", i).text(), (*it).first.text());
            set.writeStringEntry("ALIASES", FXStringFormat("value%d", i).text(), (*it).second.text());
        }
    }
    set.writeBoolEntry("SETTINGS", "autoload", m_autoload);
    set.writeStringEntry("SETTINGS", "autoloadPath", m_autoloadPath.text());
    set.writeStringEntry("SETTINGS", "dccIP", m_dccIP.text());
    set.writeIntEntry("SETTINGS", "dccPortD", m_dccPortD);
    set.writeIntEntry("SETTINGS", "dccPortH", m_dccPortH);
    set.writeIntEntry("SETTINGS", "dccTimeout", m_dccTimeout);
    set.writeBoolEntry("SETTINGS", "autoDccChat", m_autoDccChat);
    set.writeBoolEntry("SETTINGS", "autoDccFile", m_autoDccFile);
    set.writeBoolEntry("SETTINGS", "sounds", m_sounds);
    set.writeBoolEntry("SETTINGS", "soundConnect", m_soundConnect);
    set.writeBoolEntry("SETTINGS", "soundDisconnect", m_soundDisconnect);
    set.writeBoolEntry("SETTINGS", "soundMessage", m_soundMessage);
    set.writeStringEntry("SETTINGS", "pathConnect", m_pathConnect.text());
    set.writeStringEntry("SETTINGS", "pathDisconnect", m_pathDisconnect.text());
    set.writeStringEntry("SETTINGS", "pathMessage", m_pathMessage.text());
    set.writeBoolEntry("SETTINGS", "notify", m_notify);
    set.writeBoolEntry("SETTINGS", "notifyConnect", m_notifyConnect);
    set.writeBoolEntry("SETTINGS", "notifyDisconnect", m_notifyDisconnect);
    set.writeBoolEntry("SETTINGS", "notifyMessage", m_notifyMessage);
    set.writeBoolEntry("SETTINGS", "stripColors", m_stripColors);
    set.writeBoolEntry("SETTINGS", "useSmileys", m_useSmileys);
    set.writeIntEntry("SMILEYS", "number", (FXint)m_smileysMap.size());
    if((FXint)m_smileysMap.size())
    {
        StringIt it;
        FXint i;
        for(i=0, it=m_smileysMap.begin(); it!=m_smileysMap.end(); it++,i++)
        {
            set.writeStringEntry("SMILEYS", FXStringFormat("smiley%d", i).text(), (*it).first.text());
            set.writeStringEntry("SMILEYS", FXStringFormat("path%d", i).text(), (*it).second.text());
        }
    }
    set.writeBoolEntry("SETTINGS", "useSpell", m_useSpell);
    set.writeBoolEntry("SETTINGS", "showSpellCombo", m_showSpellCombo);
    set.writeBoolEntry("SETTINGS", "oneinstance", m_oneInstance);
    set.setModified();
    set.unparseFile(utils::instance().getIniFile());
}

void dxirc::saveLangs()
{
    m_app->reg().setModified(FALSE);
    FXSettings set;
    for(FXint i = 0; i<m_tabbook->numChildren(); i+=2)
    {
        if(m_tabbook->childAtIndex(i)->getMetaClass()==&IrcTabItem::metaClass
                && static_cast<IrcTabItem*>(m_tabbook->childAtIndex(i))->getType()==CHANNEL)
        {
            set.writeStringEntry("LANGS", static_cast<IrcTabItem*>(m_tabbook->childAtIndex(i))->getText().lower().prepend('_').text(), static_cast<IrcTabItem*>(m_tabbook->childAtIndex(i))->getSpellLang().text());
        }
    }
    set.unparseFile(FXPath::directory(utils::instance().getIniFile()).append(PATHSEPSTRING "langs"));
}

long dxirc::onCmdQuit(FXObject*, FXSelector, void*)
{
    saveLangs();
    while(m_ircengines.no())
    {
        if(m_ircengines[0]->getConnected()) m_ircengines[0]->disconnect();
        m_ircengines.erase(0);
    }
#ifdef HAVE_LUA
    for(FXint i=0; i<m_scriptEvents.no(); i++)
    {
        if(comparecase("quit", m_scriptEvents[i].name) == 0)
        {
            for(FXint j=0; j<m_scripts.no(); j++)
            {
                if(comparecase(m_scriptEvents[i].script, m_scripts[j].name) == 0)
                {
                    lua_pushstring(m_scripts[j].L, m_scriptEvents[i].funcname.text());
                    lua_gettable(m_scripts[j].L, LUA_GLOBALSINDEX);
                    if (lua_type(m_scripts[j].L, -1) != LUA_TFUNCTION) lua_pop(m_scripts[j].L, 1);
                    else
                    {
                        if (lua_pcall(m_scripts[j].L, 0, 0, 0))
                        {
                            appendIrcStyledText(FXStringFormat(_("Lua plugin: error calling %s %s"), m_scriptEvents[i].funcname.text(), lua_tostring(m_scripts[j].L, -1)), 4, FALSE);
                            lua_pop(m_scripts[j].L, 1);
                        }
                    }
                }
            }
        }
    }
    while(m_scripts.no())
    {
        if(m_scripts[0].L != NULL) lua_close(m_scripts[0].L);
        m_scripts.erase(0);
    }
    while(m_scriptEvents.no())
    {
        m_scriptEvents.erase(0);
    }
#endif //HAVE_LUA
    saveConfig();
    m_app->exit(0);
    return 1;
}

long dxirc::onCmdClose(FXObject*, FXSelector, void*)
{
#ifdef HAVE_TRAY
    if(m_useTray && m_closeToTray)
        hide();
    else
        onCmdQuit(NULL, 0, NULL);
#else
    onCmdQuit(NULL, 0, NULL);
#endif //HAVE_TRAY
    return 1;
}

long dxirc::onCmdHelp(FXObject*, FXSelector, void*)
{
    FXDialogBox helpDialog(this, _("Help"), DECOR_TITLE|DECOR_BORDER, 0,0,0,0, 0,0,0,0, 0,0);
    FXVerticalFrame *contents = new FXVerticalFrame(&helpDialog, LAYOUT_SIDE_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 10,10,10,10, 0,0);

    FXHorizontalFrame *textbox=new FXHorizontalFrame(contents,LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN|FRAME_THICK,0,0,0,0, 0,0,0,0);
    FXText *text=new FXText(textbox,NULL,0,TEXT_READONLY|TEXT_WORDWRAP|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    text->setVisibleRows(35);
    text->setVisibleColumns(90);
    text->setText(HELP_TEXT);

    new dxEXButton(contents, _("C&lose"), NULL, &helpDialog, FXDialogBox::ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 10,10,2,2);

    helpDialog.execute(PLACEMENT_CURSOR);
    return 1;
}

long dxirc::onCmdUsers(FXObject*, FXSelector, void*)
{
    m_usersShown = !m_usersShown;
    for(FXint i = 0; i<m_tabbook->numChildren(); i+=2)
    {        
        if(m_tabbook->childAtIndex(i)->getMetaClass()==&IrcTabItem::metaClass)
        {
            IrcTabItem *tab = static_cast<IrcTabItem*>(m_tabbook->childAtIndex(i));
            if(m_usersShown) tab->showUsers();
            else tab->hideUsers();
        }
    }
    return 1;
}

long dxirc::onCmdSpell(FXObject*, FXSelector, void*)
{
    m_showSpellCombo = !m_showSpellCombo;
    for(FXint i = 0; i<m_tabbook->numChildren(); i+=2)
    {
        if(m_tabbook->childAtIndex(i)->getMetaClass()!=&TetrisTabItem::metaClass)
        {
            dxTabItem *tab = static_cast<dxTabItem*>(m_tabbook->childAtIndex(i));
            tab->setShowSpellCombo(m_showSpellCombo);
        }
    }
    return 1;
}

long dxirc::onCmdForceFocus(FXObject*, FXSelector, void*)
{
    if(m_tabbook->numChildren())
    {
        FXint index = m_tabbook->getCurrent()*2;
        if(m_tabbook->childAtIndex(index)->getMetaClass()!=&TetrisTabItem::metaClass)
            static_cast<dxTabItem*>(m_tabbook->childAtIndex(index))->setCommandFocus();
    }
    return 1;
}

//check away on channels
long dxirc::onAwayTimeout(FXObject*, FXSelector, void*)
{
    m_app->addTimeout(this, dxirc_AWAYTIMEOUT, 180000);
    for(FXint i = 0; i<m_tabbook->numChildren(); i+=2)
    {
        if(m_tabbook->childAtIndex(i)->getMetaClass()==&IrcTabItem::metaClass)
            static_cast<IrcTabItem*>(m_tabbook->childAtIndex(i))->checkAway();
    }
    return 1;
}

long dxirc::onCmdStatus(FXObject*, FXSelector, void*)
{
    m_statusShown = !m_statusShown;
    m_statusbar->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_TOGGLESHOWN), NULL);
    return 1;
}

long dxirc::onCmdOptions(FXObject*, FXSelector, void*)
{
    FXFontDesc olddescr, newdescr;
    m_ircFont->getFontDesc(olddescr);
    dxStringMap oldsmileysMap = m_smileysMap;
    ConfigDialog dialog(this);
    if(dialog.execute(PLACEMENT_CURSOR))
    {
        saveLangs(); //need current state of spell langs for update
        FXbool recreateSmileys = FALSE;
        readConfig();
        if(m_logging) m_logviewer->enable();
        else m_logviewer->disable();
#ifdef HAVE_ENCHANT
        if(m_useSpell) m_spellCombo->enable();
        else m_spellCombo->disable();
#endif
        m_ircFont->getFontDesc(newdescr);
        if(olddescr.encoding!=newdescr.encoding || olddescr.flags!=newdescr.flags ||
                olddescr.setwidth!=newdescr.setwidth || olddescr.size!=newdescr.size ||
                olddescr.slant!=newdescr.slant || olddescr.weight!=newdescr.weight)
            recreateSmileys = TRUE;
        if(oldsmileysMap!=m_smileysMap) recreateSmileys = TRUE;
        updateTheme();
        updateFont();
        updateTabs(recreateSmileys);
        updateTabPosition();
        updateTrayColor();
        for(FXint i = 0; i<m_ircengines.no(); i++)
        {
            m_ircengines[i]->setUsersList(m_usersList);
            m_ircengines[i]->setReconnect(m_reconnect);
            m_ircengines[i]->setNumberAttempt(m_numberAttempt);
            m_ircengines[i]->setDelayAttempt(m_delayAttempt);
        }
        recalc();
    }
    return 1;
}

long dxirc::onCmdAlias(FXObject*, FXSelector, void*)
{
    AliasDialog dialog(this);
    if(dialog.execute(PLACEMENT_CURSOR))
    {
        saveConfig();
    }
    return 1;
}

long dxirc::onCmdLog(FXObject*, FXSelector, void*)
{
    if(m_viewer == NULL)
        m_viewer = new LogViewer(m_app, m_logPath, m_ircFont);
    m_viewer->create();
    return 1;
}

long dxirc::onCmdTransfers(FXObject*, FXSelector, void*)
{
    if(m_transfers == NULL)
        m_transfers = new DccDialog(m_app);
    m_transfers->create();
    return 1;
}

void dxirc::updateTheme()
{
    register FXWindow *w = FXApp::instance()->getRootWindow();

    FX7Segment * sevensegment;
    FXTextField * textfield;
    FXIconList * iconlist;
    FXList * list;
    FXListBox * listbox;
    FXTreeList * treelist;
    FXComboBox * combobox;
    FXArrowButton * arrowbuton;
    dxEXButton * button;
    FXFrame * frame;
    FXLabel * label;
    FXPopup * popup;
    FXMenuTitle * menutitle;
    FXMenuCheck * menucheck;
    FXMenuRadio * menuradio;
    FXMenuCaption * menucaption;
    FXMenuSeparator * menuseparator;
    FXText * text;
    dxText * dtext;
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
    dxEXNotify * notify;

    FXbool update = FALSE;
    FXColor oldForeColor = m_app->getForeColor();
    if(m_app->getBaseColor() != m_appTheme.base)
    {
        update = TRUE;
        m_app->setBaseColor(m_appTheme.base);
    }
    if(m_app->getBackColor() != m_appTheme.back)
    {
        update = TRUE;
        m_app->setBackColor(m_appTheme.back);
    }
    if(m_app->getBorderColor() != m_appTheme.border)
    {
        update = TRUE;
        m_app->setBorderColor(m_appTheme.border);
    }
    if(m_app->getForeColor() != m_appTheme.fore)
    {
        update = TRUE;
        m_app->setForeColor(m_appTheme.fore);
    }
    if(m_app->getSelMenuBackColor() != m_appTheme.menuback)
    {
        update = TRUE;
        m_app->setSelMenuBackColor(m_appTheme.menuback);
    }
    if(m_app->getSelMenuTextColor() != m_appTheme.menufore)
    {
        update = TRUE;
        m_app->setSelMenuTextColor(m_appTheme.menufore);
    }
    if(m_app->getSelbackColor() != m_appTheme.selback)
    {
        update = TRUE;
        m_app->setSelbackColor(m_appTheme.selback);
    }
    if(m_app->getSelforeColor() != m_appTheme.selfore)
    {
        update = TRUE;
        m_app->setSelforeColor(m_appTheme.selfore);
    }
    if(m_app->getTipbackColor() != m_appTheme.tipback)
    {
        update = TRUE;
        m_app->setTipbackColor(m_appTheme.tipback);
    }
    if(m_app->getTipforeColor() != m_appTheme.tipfore)
    {
        update = TRUE;
        m_app->setTipforeColor(m_appTheme.tipfore);
    }
    if(m_app->getHiliteColor() != m_appTheme.hilite)
    {
        update = TRUE;
        m_app->setHiliteColor(m_appTheme.hilite);
    }
    if(m_app->getShadowColor() != m_appTheme.shadow)
    {
        update = TRUE;
        m_app->setShadowColor(m_appTheme.shadow);
    }
    if(m_wnotify->getBackColor() != m_appTheme.notifyback)
    {
        update = TRUE;
    }
    if(m_wnotify->getTextColor() != m_appTheme.notifyfore)
    {
        update = TRUE;
    }
    if(!update)
        return;

    while (w)
    {
        w->setBackColor(m_appTheme.base);
        if ((frame = dynamic_cast<FXFrame*> (w)))
        {
            frame->setBaseColor(m_appTheme.base);
            frame->setBackColor(m_appTheme.base);
            frame->setShadowColor(m_appTheme.shadow);
            frame->setHiliteColor(m_appTheme.hilite);
            frame->setBorderColor(m_appTheme.border);
            if ((label = dynamic_cast<FXLabel*> (w)))
            {
                if(label->getTextColor() == oldForeColor) label->setTextColor(m_appTheme.fore);
                if ((button = dynamic_cast<dxEXButton*> (w)))
                {
                    if (dynamic_cast<FXListBox*> (button->getParent()))
                    {
                        w->setBackColor(m_appTheme.back);
                    }
                    else
                    {
                        w->setBackColor(m_appTheme.base);
                    }
                }
                else if ((checkbutton = dynamic_cast<FXCheckButton*> (w)))
                {
                    checkbutton->setCheckColor(m_appTheme.fore);
                    checkbutton->setBoxColor(m_appTheme.back);
                }
                else if ((radiobutton = dynamic_cast<FXRadioButton*> (w)))
                {
                    radiobutton->setRadioColor(m_appTheme.fore);
                    radiobutton->setDiskColor(m_appTheme.back);
                }
            }
            else if ((arrowbuton = dynamic_cast<FXArrowButton*> (w)))
            {
                arrowbuton->setArrowColor(m_appTheme.fore);
            }
            else if ((textfield = dynamic_cast<FXTextField*> (w)))
            {
                w->setBackColor(m_appTheme.back);
                textfield->setTextColor(m_appTheme.fore);
                textfield->setSelTextColor(m_appTheme.selfore);
                textfield->setSelBackColor(m_appTheme.selback);
            }
            else if ((docktitle = dynamic_cast<FXDockTitle*> (w)))
            {
                docktitle->setCaptionColor(m_appTheme.selfore);
                docktitle->setBackColor(m_appTheme.selback);
            }
            else if ((header = dynamic_cast<FXHeader*> (w)))
            {
                header->setTextColor(m_appTheme.fore);
            }
            else if ((statusline = dynamic_cast<FXStatusLine*> (w)))
            {
                statusline->setTextColor(m_appTheme.fore);
            }
            else if ((sevensegment = dynamic_cast<FX7Segment*> (w)))
            {
                sevensegment->setTextColor(m_appTheme.fore);
            }
            else if ((slider = dynamic_cast<FXSlider*> (w)))
            {
                slider->setSlotColor(m_appTheme.back);
            }
            else if ((imageframe = dynamic_cast<FXImageFrame*> (w)))
            {
                imageframe->setBackColor(m_appTheme.back); /// fixme, only for coverframe in mainwindow
            }
        }
        else if ((packer = dynamic_cast<FXPacker*> (w)))
        {
            packer->setBaseColor(m_appTheme.base);
            packer->setBackColor(m_appTheme.base);
            packer->setShadowColor(m_appTheme.shadow);
            packer->setHiliteColor(m_appTheme.hilite);
            packer->setBorderColor(m_appTheme.border);
            if ((combobox = dynamic_cast<FXComboBox*> (w)))
            {
                w->setBackColor(m_appTheme.back);
            }
            else if ((listbox = dynamic_cast<FXListBox*> (w)))
            {
                w->setBackColor(m_appTheme.back);
            }
            else if ((groupbox = dynamic_cast<FXGroupBox*> (w)))
            {
                groupbox->setTextColor(m_appTheme.fore);
            }
        }
        else if ((popup = dynamic_cast<FXPopup*> (w)))
        {
            popup->setBaseColor(m_appTheme.base);
            popup->setShadowColor(m_appTheme.shadow);
            popup->setHiliteColor(m_appTheme.hilite);
            popup->setBorderColor(m_appTheme.border);
        }
        else if ((menucaption = dynamic_cast<FXMenuCaption*> (w)))
        {
            w->setBackColor(m_appTheme.base);
            menucaption->setTextColor(m_appTheme.fore);
            menucaption->setSelTextColor(m_appTheme.menufore);
            menucaption->setSelBackColor(m_appTheme.menuback);
            menucaption->setShadowColor(m_appTheme.shadow);
            menucaption->setHiliteColor(m_appTheme.hilite);

            if ((menucheck = dynamic_cast<FXMenuCheck*> (w)))
            {
                menucheck->setBoxColor(m_appTheme.back);
            }
            else if ((menuradio = dynamic_cast<FXMenuRadio*> (w)))
            {
                menuradio->setRadioColor(m_appTheme.back);
            }
            else if ((menutitle = dynamic_cast<FXMenuTitle*> (w)))
            {
                menutitle->setTextColor(m_appTheme.fore);
                menutitle->setSelTextColor(m_appTheme.fore);
                menutitle->setSelBackColor(m_appTheme.base);
            }
        }
        else if ((menuseparator = dynamic_cast<FXMenuSeparator*> (w)))
        {
            menuseparator->setShadowColor(m_appTheme.shadow);
            menuseparator->setHiliteColor(m_appTheme.hilite);
        }
        else if ((scrollbar = dynamic_cast<FXScrollBar*> (w)))
        {
            scrollbar->setShadowColor(m_appTheme.shadow);
            scrollbar->setHiliteColor(m_appTheme.hilite);
            scrollbar->setBorderColor(m_appTheme.border);
            scrollbar->setArrowColor(m_appTheme.fore);
        }
        else if ((dragcorner = dynamic_cast<FXDragCorner*> (w)))
        {
            dragcorner->setShadowColor(m_appTheme.shadow);
            dragcorner->setHiliteColor(m_appTheme.hilite);
        }
        else if (dynamic_cast<FXScrollArea*> (w))
        {
            if ((text = dynamic_cast<FXText*> (w)))
            {
                w->setBackColor(m_appTheme.back);
                text->setTextColor(m_appTheme.fore);
                text->setSelTextColor(m_appTheme.selfore);
                text->setSelBackColor(m_appTheme.selback);
            }
            else if ((dtext = dynamic_cast<dxText*> (w)))
            {
                w->setBackColor(m_appTheme.back);
                dtext->setTextColor(m_appTheme.fore);
                dtext->setSelTextColor(m_appTheme.selfore);
                dtext->setSelBackColor(m_appTheme.selback);
            }
            else if ((list = dynamic_cast<FXList*> (w)))
            {
                w->setBackColor(m_appTheme.back);
                list->setTextColor(m_appTheme.fore);
                list->setSelTextColor(m_appTheme.selfore);
                list->setSelBackColor(m_appTheme.selback);
            }
            else if ((treelist = dynamic_cast<FXTreeList*> (w)))
            {
                w->setBackColor(m_appTheme.back);
                treelist->setTextColor(m_appTheme.fore);
                treelist->setLineColor(m_appTheme.shadow);
                treelist->setSelTextColor(m_appTheme.selfore);
                treelist->setSelBackColor(m_appTheme.selback);
            }
            else if ((iconlist = dynamic_cast<FXIconList*> (w)))
            {
                w->setBackColor(m_appTheme.back);
                iconlist->setTextColor(m_appTheme.fore);
                iconlist->setSelTextColor(m_appTheme.selfore);
                iconlist->setSelBackColor(m_appTheme.selback);
            }
            else if ((foldinglist = dynamic_cast<FXFoldingList*> (w)))
            {
                w->setBackColor(m_appTheme.back);
                foldinglist->setTextColor(m_appTheme.fore);
                foldinglist->setSelTextColor(m_appTheme.selfore);
                foldinglist->setSelBackColor(m_appTheme.selback);
                foldinglist->setLineColor(m_appTheme.shadow);
            }
            else if ((table = dynamic_cast<FXTable*> (w)))
            {
                w->setBackColor(m_appTheme.back);
                table->setTextColor(m_appTheme.fore);
                table->setSelTextColor(m_appTheme.selfore);
                table->setSelBackColor(m_appTheme.selback);
            }
        }
        else if ((mdichild = dynamic_cast<FXMDIChild*> (w)))
        {
            mdichild->setBackColor(m_appTheme.base);
            mdichild->setBaseColor(m_appTheme.base);
            mdichild->setShadowColor(m_appTheme.shadow);
            mdichild->setHiliteColor(m_appTheme.hilite);
            mdichild->setBorderColor(m_appTheme.border);
            mdichild->setTitleColor(m_appTheme.selfore);
            mdichild->setTitleBackColor(m_appTheme.selback);
        }
        else if ((tooltip = dynamic_cast<FXToolTip*> (w)))
        {
            tooltip->setTextColor(m_appTheme.tipfore);
            tooltip->setBackColor(m_appTheme.tipback);
        }
        else if ((notify = dynamic_cast<dxEXNotify*> (w)))
        {
            notify->setTextColor(m_appTheme.notifyfore);
            notify->setBackColor(m_appTheme.notifyback);
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

//update tray backround color for X11
void dxirc::updateTrayColor()
{
#ifndef WIN32
#ifdef HAVE_TRAY
    if(m_useTray && m_trayIcon)
        m_trayIcon->setColor(m_trayColor);
#endif //HAVE_TRAY
#endif //WIN32
}

void dxirc::updateFont()
{
    updateFont(m_fontSpec);
}

void dxirc::updateFont(FXString fnt)
{
    m_app->getNormalFont()->destroy();
    m_app->getNormalFont()->setFont(fnt);
    m_app->getNormalFont()->create();
    register FXWindow *w = this;
    while(w)
    {
        w->recalc();
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

void dxirc::updateTabs(FXbool recreateSmileys)
{
    if(recreateSmileys)
    {
        for(FXint i = 0; i<m_tabbook->numChildren(); i+=2)
        {
            if(m_tabbook->childAtIndex(i)->getMetaClass()!=&TetrisTabItem::metaClass)
            {
                static_cast<dxTabItem*>(m_tabbook->childAtIndex(i))->removeSmileys();
            }
        }
        createSmileys();
    }
    for(FXint i = 0; i<m_tabbook->numChildren(); i+=2)
    {        
        if(m_tabbook->childAtIndex(i)->getMetaClass()==&IrcTabItem::metaClass)
        {
            IrcTabItem *irctab = static_cast<IrcTabItem*>(m_tabbook->childAtIndex(i));
            irctab->setColor(m_colors);
            irctab->setCommandsList(m_commandsList);
            irctab->setMaxAway(m_maxAway);
            irctab->setLogging(m_logging);
            irctab->setLogPath(m_logPath);
            irctab->setNickCompletionChar(m_nickCompletionChar);
            irctab->setSameCmd(m_sameCmd);
            irctab->setSameList(m_sameList);
            irctab->setIrcFont(m_ircFont);
            irctab->setColoredNick(m_coloredNick);
            irctab->setStripColors(m_stripColors);
            irctab->setSmileys(m_useSmileys, m_smileys);
            irctab->setUseSpell(m_useSpell);
            irctab->setUnreadTabColor(m_unreadColor);
            irctab->setHighlightTabColor(m_highlightColor);
        }
        if(m_tabbook->childAtIndex(i)->getMetaClass()==&DccTabItem::metaClass)
        {
            DccTabItem *irctab = static_cast<DccTabItem*>(m_tabbook->childAtIndex(i));
            irctab->setColor(m_colors);
            irctab->setLogging(m_logging);
            irctab->setLogPath(m_logPath);
            irctab->setSameCmd(m_sameCmd);
            irctab->setIrcFont(m_ircFont);
            irctab->setColoredNick(m_coloredNick);
            irctab->setStripColors(m_stripColors);
            irctab->setSmileys(m_useSmileys, m_smileys);
            irctab->setUseSpell(m_useSpell);
            irctab->setUnreadTabColor(m_unreadColor);
            irctab->setHighlightTabColor(m_highlightColor);
        }
        if(m_tabbook->childAtIndex(i)->getMetaClass()==&TetrisTabItem::metaClass)
        {
            TetrisTabItem *tetristab = static_cast<TetrisTabItem*>(m_tabbook->childAtIndex(i));
            tetristab->setColor(m_colors);
        }
    }
    //update font in LogViewer too. Both must be same
    if(m_viewer) m_viewer->setFont(m_ircFont);
}

void dxirc::updateTabPosition()
{
    switch(m_tabPosition) {
        case 0: //bottom
            {
                m_tabbook->setTabStyle(TABBOOK_BOTTOMTABS);
                for(FXint i = 0; i<m_tabbook->numChildren(); i+=2)
                {
                    static_cast<FXTabItem*>(m_tabbook->childAtIndex(i))->setTabOrientation(TAB_BOTTOM);
                }
                FXuint packing = m_tabbook->getPackingHints();
                packing &= ~PACK_UNIFORM_WIDTH;
                m_tabbook->setPackingHints(packing);
            }break;
        case 1: //left
            {
                m_tabbook->setTabStyle(TABBOOK_LEFTTABS);
                for(FXint i = 0; i<m_tabbook->numChildren(); i+=2)
                {
                    static_cast<FXTabItem*>(m_tabbook->childAtIndex(i))->setTabOrientation(TAB_LEFT);
                }
                FXuint packing = m_tabbook->getPackingHints();
                packing |= PACK_UNIFORM_WIDTH;
                m_tabbook->setPackingHints(packing);
            }break;
        case 2: //top
            {
                m_tabbook->setTabStyle(TABBOOK_TOPTABS);
                for(FXint i = 0; i<m_tabbook->numChildren(); i+=2)
                {
                    static_cast<FXTabItem*>(m_tabbook->childAtIndex(i))->setTabOrientation(TAB_TOP);
                }
                FXuint packing = m_tabbook->getPackingHints();
                packing &= ~PACK_UNIFORM_WIDTH;
                m_tabbook->setPackingHints(packing);
            }break;
        case 3: //right
            {
                m_tabbook->setTabStyle(TABBOOK_RIGHTTABS);
                for(FXint i = 0; i<m_tabbook->numChildren(); i+=2)
                {
                    static_cast<FXTabItem*>(m_tabbook->childAtIndex(i))->setTabOrientation(TAB_RIGHT);
                }
                FXuint packing = m_tabbook->getPackingHints();
                packing |= PACK_UNIFORM_WIDTH;
                m_tabbook->setPackingHints(packing);
            }break;
        default:
            {
                m_tabbook->setTabStyle(TABBOOK_BOTTOMTABS);
                for(FXint i = 0; i<m_tabbook->numChildren(); i+=2)
                {
                    static_cast<FXTabItem*>(m_tabbook->childAtIndex(i))->setTabOrientation(TAB_BOTTOM);
                }
                FXuint packing = m_tabbook->getPackingHints();
                packing &= ~PACK_UNIFORM_WIDTH;
                m_tabbook->setPackingHints(packing);
            }
    }
}

void dxirc::createSmileys()
{
    while(m_smileys.no())
    {
        delete m_smileys[0].icon;
        m_smileys[0].icon = NULL;
        m_smileys.erase(0);
    }
    if((FXint)m_smileysMap.size() && m_ircFont)
    {
        StringIt it;
        for(it=m_smileysMap.begin(); it!=m_smileysMap.end(); it++)
        {
            dxSmiley smiley;
            smiley.text = (*it).first;
            smiley.path = (*it).second;
            smiley.icon = makeIcon(m_app, smiley.path, m_ircFont->getFontHeight(), m_colors.back);
            m_smileys.append(smiley);
        }
    }
}

long dxirc::onCmdAbout(FXObject*, FXSelector, void*)
{
    AboutDialog about(this);
    about.execute(PLACEMENT_OWNER);
    return 1;
}

long dxirc::onCmdDccCancel(FXObject*, FXSelector, void *ptr)
{
    FXint index = (FXint)(FXival)ptr;
    if(index >= m_dccfilesList.no())
        return 1;
    for(FXint i=0; i<m_dccengines.no(); i++)
    {
        if(m_dccengines[i]->hasDccFile(m_dccfilesList[index]))
        {
            m_dccengines[i]->disconnect();
            return 1;
        }
    }
    //dccfile hasn't server
    m_dccfilesList[index].canceled = TRUE;
    m_dccfilesList[index].token = -1;
    return 1;
}

long dxirc::onCmdServers(FXObject*, FXSelector, void*)
{
    utils::instance().debugLine("OnCommandServers");
    ServerDialog *dialog = new ServerDialog(this, m_serverList);
    if (dialog->execute(PLACEMENT_OWNER))
    {
        FXint indexJoin = -1;
        m_serverList = dialog->getServers();
        indexJoin = dialog->getIndexJoin();
        if (indexJoin != -1 && !serverExist(m_serverList[indexJoin].hostname, m_serverList[indexJoin].port, m_serverList[indexJoin].nick))
        {
            connectServer(m_serverList[indexJoin].hostname, m_serverList[indexJoin].port, m_serverList[indexJoin].passwd, m_serverList[indexJoin].nick, m_serverList[indexJoin].realname, m_serverList[indexJoin].channels, m_serverList[indexJoin].commands, m_serverList[indexJoin].useSsl);
        }
        saveConfig();
    }
    return 1;
}

long dxirc::onCmdConnect(FXObject*, FXSelector, void*)
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
#endif //HAVE_OPENSSL

    new FXLabel(matrix, _("Commands on connection:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXHorizontalFrame *commandsbox=new FXHorizontalFrame(matrix, LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|FRAME_SUNKEN|FRAME_THICK,0,0,0,0, 0,0,0,0);
    FXText *command = new FXText(commandsbox, NULL, 0, TEXT_WORDWRAP|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    command->setVisibleRows(4);
    command->setVisibleColumns(25);

    FXHorizontalFrame *buttonframe = new FXHorizontalFrame(contents,LAYOUT_FILL_X|LAYOUT_FILL_Y|PACK_UNIFORM_WIDTH);
    new dxEXButton(buttonframe, _("&Cancel"), NULL, &serverEdit, FXDialogBox::ID_CANCEL, FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,2);
    new dxEXButton(buttonframe, _("&OK"), NULL, &serverEdit, FXDialogBox::ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,2);
    if (serverEdit.execute(PLACEMENT_OWNER))
    {
#ifdef HAVE_OPENSSL
        connectServer(hostname->getText(), port->getValue(), passwd->getText(), nick->getText(), realname->getText(), channel->getText(), command->getText(), ussl->getCheck());
#else
        connectServer(hostname->getText(), port->getValue(), passwd->getText(), nick->getText(), realname->getText(), channel->getText(), command->getText(), FALSE);
#endif //HAVE_OPENSSL
    }
    return 1;
}

long dxirc::onTabConnect(FXObject*, FXSelector, void *data)
{
    ServerInfo *srv = (ServerInfo*)data;
    connectServer(srv->hostname, srv->port, srv->passwd, srv->nick, srv->realname, srv->channels, "", FALSE);
    return 1;
}

void dxirc::connectServer(FXString hostname, FXint port, FXString pass, FXString nick, FXString rname, FXString channels, FXString commands, FXbool ssl)
{
    utils::instance().debugLine("ConnectServer");
    if(m_ircengines.no() == 1 && !m_ircengines[0]->getConnected() && !m_ircengines[0]->getConnecting())
    {
        m_ircengines[0]->setServerName(hostname);
        m_ircengines[0]->setServerPort(port);
        m_ircengines[0]->setServerPassword(pass);
        nick.length() ? m_ircengines[0]->setNickName(nick) : m_ircengines[0]->setNickName("_xxx_");
        nick.length() ? m_ircengines[0]->setUserName(nick) : m_ircengines[0]->setUserName("_xxx_");
        rname.length() ? m_ircengines[0]->setRealName(rname) : m_ircengines[0]->setRealName(nick.length() ? nick : "_xxx_");
        if(channels.length()>1) m_ircengines[0]->setStartChannels(channels);
        if(commands.length()) m_ircengines[0]->setStartCommands(commands);
#ifndef HAVE_OPENSSL
        ssl = FALSE;
#endif //HAVE_OPENSSL
        m_ircengines[0]->setUseSsl(ssl);
        m_ircengines[0]->setReconnect(m_reconnect);
        m_ircengines[0]->setNumberAttempt(m_numberAttempt);
        m_ircengines[0]->setDelayAttempt(m_delayAttempt);
        if(!m_tabbook->numChildren())
        {
            createIrcTab(hostname, ICO_SERVER, SERVER, m_ircengines[0]);
            m_tabbook->recalc();
        }
        else
        {
            FXbool needNewTab = TRUE;
            for(FXint i = 0; i<m_tabbook->numChildren(); i+=2)
            {
                if(m_tabbook->childAtIndex(i)->getMetaClass()==&IrcTabItem::metaClass
                        && static_cast<IrcTabItem*>(m_tabbook->childAtIndex(i))->getType()==SERVER)
                {
                    static_cast<IrcTabItem*>(m_tabbook->childAtIndex(i))->setText(hostname);
                    sortTabs();
                    needNewTab = FALSE;
                    break;
                }
            }
            if(needNewTab) createIrcTab(hostname, ICO_SERVER, SERVER, m_ircengines[0]);
        }
        m_ircengines[0]->clearAttempts();
        m_ircengines[0]->startConnection();
    }
    else if(!serverExist(hostname, port, nick))
    {
        IrcEngine *server = new IrcEngine(m_app, this, channels.length()>1 ? channels : "", commands.length() ? commands : "");
        server->setUsersList(m_usersList);
        m_ircengines.prepend(server);
        m_ircengines[0]->setServerName(hostname);
        m_ircengines[0]->setServerPort(port);
        m_ircengines[0]->setServerPassword(pass);
        nick.length() ? m_ircengines[0]->setNickName(nick) : m_ircengines[0]->setNickName("_xxx_");
        nick.length() ? m_ircengines[0]->setUserName(nick) : m_ircengines[0]->setUserName("_xxx_");
        rname.length() ? m_ircengines[0]->setRealName(rname) : m_ircengines[0]->setRealName(nick.length() ? nick : "_xxx_");
#ifndef HAVE_OPENSSL
        ssl = FALSE;
#endif //HAVE_OPENSSL
        m_ircengines[0]->setUseSsl(ssl);
        m_ircengines[0]->setReconnect(m_reconnect);
        m_ircengines[0]->setNumberAttempt(m_numberAttempt);
        m_ircengines[0]->setDelayAttempt(m_delayAttempt);
        createIrcTab(hostname, ICO_SERVER, SERVER, m_ircengines[0]);
        m_ircengines[0]->clearAttempts();
        m_ircengines[0]->startConnection();
    }
    updateMenus();
}

long dxirc::onCmdDisconnect(FXObject*, FXSelector, void*)
{
    if(m_tabbook->numChildren())
    {
        FXint index = m_tabbook->getCurrent()*2;
        if(m_tabbook->childAtIndex(index)->getMetaClass()==&TetrisTabItem::metaClass) return 0;
        if(m_tabbook->childAtIndex(index)->getMetaClass()==&IrcTabItem::metaClass)
        {
            IrcTabItem *currenttab = static_cast<IrcTabItem*>(m_tabbook->childAtIndex(index));
            IrcEngine *currentserver = NULL;
            for(FXint i=0; i < m_ircengines.no(); i++)
            {
                if(m_ircengines[i]->findTarget(currenttab))
                {
                    currentserver = m_ircengines[i];
                    break;
                }
            }
            if(currentserver == NULL) return 0;
            if(currentserver->getConnected())
            {
                FXDialogBox confirmDialog(this, _("Confirm disconnect"), DECOR_TITLE|DECOR_BORDER, 0,0,0,0, 0,0,0,0, 0,0);
                FXVerticalFrame *contents = new FXVerticalFrame(&confirmDialog, LAYOUT_SIDE_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 10,10,10,10, 0,0);
                new FXLabel(contents, FXStringFormat(_("Disconnect server: %s\nPort: %d\nNick: %s"), currentserver->getNetworkName().text(), currentserver->getServerPort(), currentserver->getNickName().text()), NULL, JUSTIFY_LEFT|ICON_BEFORE_TEXT);
                FXHorizontalFrame* buttonframe = new FXHorizontalFrame(contents,LAYOUT_FILL_X|LAYOUT_FILL_Y|PACK_UNIFORM_WIDTH);
                new dxEXButton(buttonframe, _("&No"), NULL, &confirmDialog, FXDialogBox::ID_CANCEL, FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,2);
                new dxEXButton(buttonframe, _("&Yes"), NULL, &confirmDialog, FXDialogBox::ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,2);
                if (confirmDialog.execute(PLACEMENT_OWNER))
                {
                    currentserver->disconnect();
                    for(FXint i = m_tabbook->numChildren()-2; i > -1; i-=2)
                    {
                        if(currentserver->findTarget(static_cast<IrcTabItem*>(m_tabbook->childAtIndex(i))))
                        {
                            currentserver->removeTarget(static_cast<IrcTabItem*>(m_tabbook->childAtIndex(i)));
                            delete m_tabbook->childAtIndex(i);
                            delete m_tabbook->childAtIndex(i);
                        }
                    }
                    m_tabbook->recalc();
                    if(m_tabbook->numChildren())
                    {
                        sortTabs();
                        m_tabbook->setCurrent(m_tabbook->numChildren()/2-1, TRUE);
                    }
                }
            }
            else
            {
                currentserver->disconnect();
                for(FXint i = m_tabbook->numChildren()-2; i > -1; i-=2)
                {
                    if(currentserver->findTarget(static_cast<IrcTabItem*>(m_tabbook->childAtIndex(i))))
                    {
                        currentserver->removeTarget(static_cast<IrcTabItem*>(m_tabbook->childAtIndex(i)));
                        delete m_tabbook->childAtIndex(i);
                        delete m_tabbook->childAtIndex(i);
                    }
                }
                m_tabbook->recalc();
                if(m_tabbook->numChildren())
                {
                    sortTabs();
                    m_tabbook->setCurrent(m_tabbook->numChildren()/2-1, TRUE);
                }
            }
            return 1;
        }
        if(m_tabbook->childAtIndex(index)->getMetaClass()==&DccTabItem::metaClass)
        {
            DccTabItem *currenttab = static_cast<DccTabItem*>(m_tabbook->childAtIndex(index));
            if(currenttab->getConnected())
            {
                FXDialogBox confirmDialog(this, _("Confirm disconnect"), DECOR_TITLE|DECOR_BORDER, 0,0,0,0, 0,0,0,0, 0,0);
                FXVerticalFrame *contents = new FXVerticalFrame(&confirmDialog, LAYOUT_SIDE_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 10,10,10,10, 0,0);
                new FXLabel(contents, FXStringFormat(_("Disconnect dcc connection: %s\nPort: %d\nWith: %s"), currenttab->getServerName().text(), currenttab->getServerPort(), currenttab->getText().text()), NULL, JUSTIFY_LEFT|ICON_BEFORE_TEXT);
                FXHorizontalFrame* buttonframe = new FXHorizontalFrame(contents,LAYOUT_FILL_X|LAYOUT_FILL_Y|PACK_UNIFORM_WIDTH);
                new dxEXButton(buttonframe, _("&No"), NULL, &confirmDialog, FXDialogBox::ID_CANCEL, FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,2);
                new dxEXButton(buttonframe, _("&Yes"), NULL, &confirmDialog, FXDialogBox::ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,2);
                if (confirmDialog.execute(PLACEMENT_OWNER))
                {
                    currenttab->disconnect();
                    delete m_tabbook->childAtIndex(index);
                    delete m_tabbook->childAtIndex(index);
                    m_tabbook->recalc();
                    if(m_tabbook->numChildren())
                    {
                        sortTabs();
                        m_tabbook->setCurrent(m_tabbook->numChildren()/2-1, TRUE);
                    }
                }
            }
            else
            {
                delete m_tabbook->childAtIndex(index);
                delete m_tabbook->childAtIndex(index);
                m_tabbook->recalc();
                if(m_tabbook->numChildren())
                {
                    sortTabs();
                    m_tabbook->setCurrent(m_tabbook->numChildren()/2-1, TRUE);
                }
            }
            return 1;
        }
    }
    updateMenus();
    return 1;
}

long dxirc::onIrcEvent(FXObject *obj, FXSelector, void *data)
{
    IrcEvent *ev = (IrcEvent *)data;
    IrcEngine *server = (IrcEngine *)obj;
    if(ev->eventType == IRC_NEWCHANNEL)
    {
        onIrcNewchannel(server, ev);
        return 1;
    }
    if(ev->eventType == IRC_QUERY)
    {
        onIrcQuery(server, ev);
        return 1;
    }
    if(ev->eventType == IRC_PART)
    {
        onIrcPart(server, ev);
        return 1;
    }
    if(ev->eventType == IRC_KICK)
    {
        onIrcKick(server, ev);
        return 1;
    }
    if(ev->eventType == IRC_DISCONNECT)
    {
        onIrcDisconnect(server, ev);
        return 1;
    }
    if(ev->eventType == IRC_RECONNECT || ev->eventType == IRC_CONNECT)
    {
        onIrcConnectAndReconnect(ev);
        return 1;
    }
    if(ev->eventType == IRC_ENDMOTD)
    {
        onIrcEndmotd();
        return 1;
    }
    if(ev->eventType == IRC_PRIVMSG || ev->eventType == IRC_ACTION)
    {
        onIrcPrivmsgAndAction(server, ev);
        return 1;
    }
    if(ev->eventType == IRC_JOIN)
    {
        onIrcJoin(server, ev);
        return 1;
    }
    if(ev->eventType == IRC_QUIT)
    {
        onIrcQuit(server, ev);
        return 1;
    }
    if(ev->eventType == IRC_DCCCHAT)
    {
        onIrcDccChat(server, ev);
        return 1;
    }
    if(ev->eventType == IRC_DCCSERVER)
    {
        onIrcDccServer(server, ev);
        return 1;
    }
    if(ev->eventType == IRC_DCCIN)
    {
        onIrcDccIn(server, ev);
        return 1;
    }
    if(ev->eventType == IRC_DCCOUT)
    {
        onIrcDccOut(server, ev);
        return 1;
    }
    if(ev->eventType == IRC_DCCPOUT)
    {
        onIrcDccPout(server, ev);
        return 1;
    }
    if(ev->eventType == IRC_DCCMYTOKEN)
    {
        onIrcDccMyToken(server, ev);
        return 1;
    }
    if(ev->eventType == IRC_DCCTOKEN)
    {
        onIrcDccToken(server, ev);
        return 1;
    }
    //IrcEngine couldn't send IRC_DCCPOSITION :)
    if(ev->eventType == IRC_DCCPOSITION)
    {
        return 1;
    }
    if(ev->eventType == IRC_DCCRESUME)
    {
        onIrcDccResume(server, ev);
        return 1;
    }
    if(ev->eventType == IRC_DCCPRESUME)
    {
        onIrcDccPresume(server, ev);
        return 1;
    }
    if(ev->eventType == IRC_DCCACCEPT)
    {
        onIrcDccAccept(server, ev);
        return 1;
    }
    if(ev->eventType == IRC_DCCPACCEPT)
    {
        onIrcDccPaccept(server, ev);
        return 1;
    }
    return 1;
}

long dxirc::onDccEvent(FXObject *obj, FXSelector, void *data)
{
    IrcEvent *ev = (IrcEvent *)data;
    DccEngine *dcc = (DccEngine *)obj;
    if(ev->eventType == IRC_DISCONNECT)
    {
        appendIrcStyledText(FXStringFormat("\"%s\": %s", FXPath::name(ev->dccFile.path).text(), ev->param1.text()), 4, FALSE);
        return 1;
    }
    if(ev->eventType == IRC_ERROR)
    {
        appendIrcStyledText(FXStringFormat("\"%s\": %s", FXPath::name(ev->dccFile.path).text(), ev->param1.text()), 4, FALSE);
        return 1;
    }
    if(ev->eventType == IRC_DCCPOSITION)
    {
        onIrcDccPosition(dcc, ev);
        return 1;
    }
    return 1;
}

//handle IrcEvent IRC_NEWCHANNEL
void dxirc::onIrcNewchannel(IrcEngine *server, IrcEvent *ev)
{
    FXint serverTabIndex = getServerTab(server);
    if(tabExist(server, ev->param1))
    {
        return;
    }
    if(serverTabIndex != -1 && !m_ownServerWindow)
    {
        static_cast<IrcTabItem*>(m_tabbook->childAtIndex(serverTabIndex))->setType(CHANNEL, ev->param1);
        m_tabbook->setCurrent(FXMAX(0,serverTabIndex/2-1), TRUE);
        sortTabs();
    }
    else
    {
        createIrcTab(ev->param1, ICO_CHANNEL, CHANNEL, server);
    }
    updateMenus();
}

//handle IrcEvent IRC_QUERY
void dxirc::onIrcQuery(IrcEngine *server, IrcEvent *ev)
{
    if(tabExist(server, ev->param1))
        return;
    FXint serverTabIndex = getServerTab(server);
    if(serverTabIndex != -1 && !m_ownServerWindow)
    {
        static_cast<IrcTabItem*>(m_tabbook->childAtIndex(serverTabIndex))->setType(QUERY, ev->param1);
        m_tabbook->setCurrent(FXMAX(0,serverTabIndex/2-1), TRUE);
        sortTabs();
    }
    else
    {
        createIrcTab(ev->param1, ICO_QUERY, QUERY, server);
    }
    if(ev->param2 == server->getNickName())
    {
        for(FXint i = 0; i < m_tabbook->numChildren(); i+=2)
        {
            if(server->findTarget(static_cast<IrcTabItem*>(m_tabbook->childAtIndex(i))) && comparecase(static_cast<FXTabItem*>(m_tabbook->childAtIndex(i))->getText(), ev->param1) == 0) m_tabbook->setCurrent(i/2, TRUE);
        }
    }
    updateMenus();
}

//handle IrcEvent IRC_PART
void dxirc::onIrcPart(IrcEngine *server, IrcEvent *ev)
{
    if(isFriend(ev->param1, ev->param2, server->getNetworkName()))
    {
        if(m_sounds && m_soundDisconnect)
            utils::instance().playFile(m_pathDisconnect);
        if(m_notify && m_notifyDisconnect)
            showNotify(FXStringFormat(_("%s has parted %s"), ev->param1.text(), ev->param2.text()));
    }
    if(tabExist(server, ev->param2))
    {
        if(ev->param1 == server->getNickName())
        {
            if(server->getConnected() && isLastTab(server))
            {
                for(FXint j = 0; j < m_tabbook->numChildren(); j+=2)
                {
                    if(server->findTarget(static_cast<IrcTabItem*>(m_tabbook->childAtIndex(j))))
                    {
                        static_cast<IrcTabItem*>(m_tabbook->childAtIndex(j))->setType(SERVER, server->getNetworkName());
                        m_tabbook->setCurrent(j/2-1, TRUE);
                        break;
                    }
                }
            }
            else
            {
                FXint index = -1;
                for(FXint j = 0; j < m_tabbook->numChildren(); j+=2)
                {
                    if((comparecase(static_cast<FXTabItem*>(m_tabbook->childAtIndex(j))->getText(), ev->param2) == 0) && server->findTarget(static_cast<IrcTabItem*>(m_tabbook->childAtIndex(j)))) index = j;
                }
                if(index == -1) return;
                server->removeTarget(static_cast<IrcTabItem*>(m_tabbook->childAtIndex(index)));
                delete m_tabbook->childAtIndex(index);
                delete m_tabbook->childAtIndex(index);
                m_tabbook->recalc();
                if(m_tabbook->numChildren())
                {
                    m_tabbook->setCurrent(FXMAX(0,index/2-1), TRUE);
                }
            }
            sortTabs();
            updateMenus();
        }
    }
}

//handle IrcEvent IRC_KICK
void dxirc::onIrcKick(IrcEngine *server, IrcEvent *ev)
{
    if(ev->param2 == server->getNickName())
    {
        if(server->getConnected() && isLastTab(server))
        {
            for(FXint j = 0; j < m_tabbook->numChildren(); j+=2)
            {
                if(server->findTarget(static_cast<IrcTabItem*>(m_tabbook->childAtIndex(j))))
                {
                    static_cast<IrcTabItem*>(m_tabbook->childAtIndex(j))->setType(SERVER, server->getNetworkName());
                    m_tabbook->setCurrent(j/2-1, TRUE);
                    break;
                }
            }
        }
        else
        {
            FXint index = -1;
            for(FXint j = 0; j < m_tabbook->numChildren(); j+=2)
            {
                if((comparecase(static_cast<FXTabItem*>(m_tabbook->childAtIndex(j))->getText(), ev->param3) == 0) && server->findTarget(static_cast<IrcTabItem*>(m_tabbook->childAtIndex(j)))) index = j;
            }
            if(index == -1) return;
            server->removeTarget(static_cast<IrcTabItem*>(m_tabbook->childAtIndex(index)));
            delete m_tabbook->childAtIndex(index);
            delete m_tabbook->childAtIndex(index);
            m_tabbook->recalc();
            if(m_tabbook->numChildren())
            {
                m_tabbook->setCurrent(FXMAX(0,index/2-1), TRUE);
            }
        }
        sortTabs();
        updateMenus();
    }
}

//handle IrcEvent IRC_DISCONNECT
void dxirc::onIrcDisconnect(IrcEngine *server, IrcEvent *ev)
{
    for(FXint i = m_tabbook->numChildren()-2; i > -1; i-=2)
    {
        if(server->findTarget(static_cast<IrcTabItem*>(m_tabbook->childAtIndex(i))))
        {
            server->removeTarget(static_cast<IrcTabItem*>(m_tabbook->childAtIndex(i)));
            delete m_tabbook->childAtIndex(i);
            delete m_tabbook->childAtIndex(i);
        }
    }
    m_tabbook->recalc();
    if(m_tabbook->numChildren())
    {
        sortTabs();
        m_tabbook->setCurrent(0, TRUE);
    }
    updateMenus();
    updateStatus(ev->param1);
}

//handle IrcEvent IRC_CONNECT and IRC_RECONNECT
void dxirc::onIrcConnectAndReconnect(IrcEvent *ev)
{
    updateStatus(ev->param1);
}

//handle IrcEvent IRC_ENDMOTD
void dxirc::onIrcEndmotd()
{
    updateMenus();
}

//handle IrcEvent IRC_PRIVMSG and IRC_ACTION
void dxirc::onIrcPrivmsgAndAction(IrcEngine *server, IrcEvent *ev)
{
#ifdef HAVE_LUA
    if(!m_scripts.no() || !m_scriptEvents.no()) return;
    for(FXint i=0; i<m_scriptEvents.no(); i++)
    {
        if(comparecase("privmsg", m_scriptEvents[i].name) == 0)
        {
            for(FXint j=0; j<m_scripts.no(); j++)
            {
                if(comparecase(m_scriptEvents[i].script, m_scripts[j].name) == 0)
                {
                    lua_pushstring(m_scripts[j].L, m_scriptEvents[i].funcname.text());
                    lua_gettable(m_scripts[j].L, LUA_GLOBALSINDEX);
                    if (lua_type(m_scripts[j].L, -1) != LUA_TFUNCTION) lua_pop(m_scripts[j].L, 1);
                    else
                    {
                        lua_pushstring(m_scripts[j].L, ev->param1.text());
                        lua_pushstring(m_scripts[j].L, ev->param3.text());
                        lua_pushinteger(m_scripts[j].L, getTabId(server, ev->param2 == server->getNickName() ? ev->param1 : ev->param2));
                        if (lua_pcall(m_scripts[j].L, 3, 0, 0))
                        {
                            appendIrcStyledText(FXStringFormat(_("Lua plugin: error calling %s %s"), m_scriptEvents[i].funcname.text(), lua_tostring(m_scripts[j].L, -1)), 4, FALSE);
                            lua_pop(m_scripts[j].L, 1);
                        }
                    }
                }
            }
        }
    }
#endif //HAVE_LUA
}

//handle IrcEvent IRC_JOIN
void dxirc::onIrcJoin(IrcEngine *server, IrcEvent *ev)
{
    if(isFriend(ev->param1, ev->param2, server->getNetworkName()))
    {
        if(m_sounds && m_soundConnect)
            utils::instance().playFile(m_pathConnect);
        if(m_notify && m_notifyConnect)
            showNotify(FXStringFormat(_("%s has joined to %s"), ev->param1.text(), ev->param2.text()));
    }
#ifdef HAVE_LUA
    if(server->isUserIgnored(ev->param1, ev->param2)) return;
    if(!m_scripts.no() || !m_scriptEvents.no()) return;
    for(FXint i=0; i<m_scriptEvents.no(); i++)
    {
        if(comparecase("join", m_scriptEvents[i].name) == 0)
        {
            for(FXint j=0; j<m_scripts.no(); j++)
            {
                if(comparecase(m_scriptEvents[i].script, m_scripts[j].name) == 0)
                {
                    lua_pushstring(m_scripts[j].L, m_scriptEvents[i].funcname.text());
                    lua_gettable(m_scripts[j].L, LUA_GLOBALSINDEX);
                    if (lua_type(m_scripts[j].L, -1) != LUA_TFUNCTION) lua_pop(m_scripts[j].L, 1);
                    else
                    {
                        lua_pushstring(m_scripts[j].L, ev->param1.text());
                        //lua_pushstring(scripts[j].L, ev->param2.text());
                        lua_pushinteger(m_scripts[j].L, getTabId(server, ev->param2));
                        if (lua_pcall(m_scripts[j].L, 2, 0, 0))
                        {
                            appendIrcStyledText(FXStringFormat(_("Lua plugin: error calling %s %s"), m_scriptEvents[i].funcname.text(), lua_tostring(m_scripts[j].L, -1)), 4, FALSE);
                            lua_pop(m_scripts[j].L, 1);
                        }
                    }
                }
            }
        }
    }
#endif //HAVE_LUA
}

//handle IrcEvent IRC_QUIT
void dxirc::onIrcQuit(IrcEngine *server, IrcEvent *ev)
{
    if(isFriend(ev->param1, "all", server->getNetworkName()))
    {
        if(m_sounds && m_soundDisconnect)
            utils::instance().playFile(m_pathDisconnect);
        if(m_notify && m_notifyDisconnect)
            showNotify(FXStringFormat(_("%s has quit"), ev->param1.text()));
    }
}

//handle IrcEvent IRC_DCCCHAT
void dxirc::onIrcDccChat(IrcEngine *server, IrcEvent *ev)
{
    if(m_autoDccChat)
        createDccTab(server->getNickName(), ev->param1, ev->param2, FXIntVal(ev->param3));
    if(!m_autoDccChat && dxEXMessageBox::question(this, MBOX_YES_NO, _("Question"), _("%s offers DCC Chat on %s(%s) port %s.\n Do you want connect?"), ev->param1.text(), ev->param2.text(), server->getHostname(ev->param2).text(), ev->param3.text()) == 1)
        createDccTab(server->getNickName(), ev->param1, ev->param2, FXIntVal(ev->param3));
}

//handle IrcEvent IRC_DCCSERVER
void dxirc::onIrcDccServer(IrcEngine *server, IrcEvent *ev)
{
    createDccTab(server->getNickName(), ev->param1, server->getLocalIP(), m_dccPortD, m_dccPortH, TRUE, server);
}

//handle IrcEvent IRC_DCCIN
void dxirc::onIrcDccIn(IrcEngine *server, IrcEvent *ev)
{
    if(m_autoDccFile)
    {
        DccFile dcc;
        dcc.path = getUniqueName(m_dccPath, FXPath::stripExtension(ev->param3), FXPath::extension(ev->param3));
        dcc.speed = 0;
        dcc.currentPosition = 0;
        dcc.size = FXLongVal(ev->param4);
        dcc.type = DCC_IN;
        dcc.canceled = FALSE;
        dcc.finishedPosition = 0;
        dcc.token = -1;
        dcc.ip = ev->param2.before('@');
        dcc.port = FXIntVal(ev->param2.after('@'));
        dcc.nick = ev->param1;
        for(FXint i=0; i<m_dccfilesList.no(); i++)
        {
            if(dcc.path == m_dccfilesList[i].path && (m_dccfilesList[i].type==DCC_IN || m_dccfilesList[i].type==DCC_PIN))
            {
                m_dccfilesList.erase(i);
                break;
            }
        }
        m_dccfilesList.append(dcc);
        if(isForResume(FXPath::name(dcc.path)))
        {
            server->sendCtcp(ev->param1, "DCC RESUME "+FXPath::name(dcc.path)+" "+FXStringVal(dcc.port)+" "+FXStringVal(FXStat::size(dcc.path+".part")));
        }
        else
        {
            DccEngine *engine = new DccEngine(m_app, this, dcc, server);
            m_dccengines.append(engine);
            engine->startConnection();
            appendIrcStyledText(FXStringFormat(_("Receiving \"%s\" from %s"), ev->param3.text(), ev->param1.text()),8,FALSE);
        }
    }
    else
    {
        if(isForResume(ev->param3)
            &&dxEXMessageBox::question(this, MBOX_YES_NO, _("Question"), _("%s offers file %s with size %s over DCC .\nFile is already partially downloaded.\nDo you want continue in download?"), ev->param1.text(), ev->param3.text(), utils::instance().getFileSize(ev->param4).text()) == 1)
        {
            DccFile dcc;
            dcc.path = m_dccPath+PATHSEPSTRING+ev->param3;
            dcc.speed = 0;
            dcc.currentPosition = 0;
            dcc.size = FXLongVal(ev->param4);
            dcc.type = DCC_IN;
            dcc.canceled = FALSE;
            dcc.finishedPosition = 0;
            dcc.token = -1;
            dcc.ip = ev->param2.before('@');
            dcc.port = FXIntVal(ev->param2.after('@'));
            dcc.nick = ev->param1;
            for(FXint i=0; i<m_dccfilesList.no(); i++)
            {
                if(dcc.path == m_dccfilesList[i].path && (m_dccfilesList[i].type==DCC_IN || m_dccfilesList[i].type==DCC_PIN))
                {
                    m_dccfilesList.erase(i);
                    break;
                }
            }
            m_dccfilesList.append(dcc);
            server->sendCtcp(ev->param1, "DCC RESUME "+FXPath::name(dcc.path)+" "+FXStringVal(dcc.port)+" "+FXStringVal(FXStat::size(dcc.path+".part")));
            return;
        }
        if(dxEXMessageBox::question(this, MBOX_YES_NO, _("Question"), _("%s offers file %s with size %s over DCC .\nDo you want connect?"), ev->param1.text(), ev->param3.text(), utils::instance().getFileSize(ev->param4).text()) == 1)
        {
            dxEXFileDialog dialog(this, _("Save file"));
            dialog.setFilename(m_dccPath+PATHSEPSTRING+ev->param3);
            if(dialog.execute())
            {
                DccFile dcc;
                dcc.path = getUniqueName(FXPath::directory(dialog.getFilename()), FXPath::title(dialog.getFilename()), FXPath::extension(dialog.getFilename())); // dirty (owerwrite user opinion) but absolutely needed :)
                dcc.speed = 0;
                dcc.currentPosition = 0;
                dcc.size = FXLongVal(ev->param4);
                dcc.type = DCC_IN;
                dcc.canceled = FALSE;
                dcc.finishedPosition = 0;
                dcc.token = -1;
                dcc.ip = ev->param2.before('@');
                dcc.port = FXIntVal(ev->param2.after('@'));
                dcc.nick = ev->param1;
                for(FXint i=0; i<m_dccfilesList.no(); i++)
                {
                    if(dcc.path == m_dccfilesList[i].path && (m_dccfilesList[i].type==DCC_IN || m_dccfilesList[i].type==DCC_PIN))
                    {
                        m_dccfilesList.erase(i);
                        break;
                    }
                }
                //old .part file have to be deleted
                if(FXStat::exists(dcc.path+".part"))
                    FXFile::remove(dcc.path+".part");
                m_dccfilesList.append(dcc);
                DccEngine *engine = new DccEngine(m_app, this, dcc, server);
                m_dccengines.append(engine);
                engine->startConnection();
                appendIrcStyledText(FXStringFormat(_("Receiving \"%s\" from %s"), ev->param3.text(), ev->param1.text()),8,FALSE);
                onCmdTransfers(NULL, 0, NULL);
            }
        }
    }
}

//handle IrcEvent IRC_DCCOUT
void dxirc::onIrcDccOut(IrcEngine *server, IrcEvent *ev)
{
    DccFile dcc;
    dcc.path = ev->param2;
    dcc.speed = 0;
    dcc.currentPosition = 0;
    dcc.size = FXStat::size(ev->param2);
    dcc.type = DCC_OUT;
    dcc.canceled = FALSE;
    dcc.finishedPosition = 0;
    dcc.ip = server->getLocalIP();
    dcc.port = 0;
    dcc.token = -1;
    dcc.nick = ev->param1;
    for(FXint i=0; i<m_dccfilesList.no(); i++)
    {
        if(dcc.path == m_dccfilesList[i].path && dcc.type == m_dccfilesList[i].type)
        {
            m_dccfilesList.erase(i);
            break;
        }
    }
    m_dccfilesList.append(dcc);
    DccEngine *engine = new DccEngine(m_app, this, dcc, server);
    m_dccengines.append(engine);
    engine->startConnection();
    onCmdTransfers(NULL, 0, NULL);
}

//handle IrcEvent IRC_DCCPOUT
void dxirc::onIrcDccPout(IrcEngine *server, IrcEvent *ev)
{
    DccFile dcc;
    dcc.path = ev->param2;
    dcc.speed = 0;
    dcc.currentPosition = 0;
    dcc.size = FXStat::size(ev->param2);
    dcc.type = DCC_POUT;
    dcc.canceled = FALSE;
    dcc.finishedPosition = 0;
    if(m_lastToken==32767) m_lastToken=0;
    dcc.token = ++m_lastToken;
    dcc.nick = ev->param1;
    for(FXint i=0; i<m_dccfilesList.no(); i++)
    {
        if(dcc.path == m_dccfilesList[i].path && dcc.type == m_dccfilesList[i].type)
        {
            m_dccfilesList.erase(i);
            break;
        }
    }
    m_dccfilesList.append(dcc);
    server->sendCtcp(ev->param1, "DCC SEND "+utils::instance().removeSpaces(dcc.path.rafter(PATHSEP))+" "+server->getLocalIPBinary()+" 0 "+FXStringVal(dcc.size)+" "+FXStringVal(dcc.token));
}

//handle IrcEvent IRC_DCCMYTOKEN
void dxirc::onIrcDccMyToken(IrcEngine *server, IrcEvent *ev)
{
    FXint token = FXIntVal(ev->param3);
    FXint index = -1;
    for(FXint i=0; i<m_dccfilesList.no(); i++)
    {
        if(m_dccfilesList[i].token == token && m_dccfilesList[i].type == DCC_POUT)
        {
            index = i;
            break;
        }
    }
    if(index == -1)
        return;
    m_dccfilesList[index].ip = ev->param1;
    m_dccfilesList[index].port = FXIntVal(ev->param2);
    DccEngine *engine = new DccEngine(m_app, this, m_dccfilesList[index], server);
    m_dccengines.append(engine);
    engine->startConnection();
    onCmdTransfers(NULL, 0, NULL);
}

//handle IrcEvent IRC_DCCTOKEN
void dxirc::onIrcDccToken(IrcEngine *server, IrcEvent *ev)
{
    if(m_autoDccFile)
    {
        DccFile dcc;
        dcc.path = getUniqueName(m_dccPath, FXPath::stripExtension(ev->param2), FXPath::extension(ev->param2));
        dcc.speed = 0;
        dcc.currentPosition = 0;
        dcc.size = FXLongVal(ev->param3);
        dcc.type = DCC_PIN;
        dcc.canceled = FALSE;
        dcc.finishedPosition = 0;
        dcc.token = FXIntVal(ev->param4);
        dcc.nick = ev->param1;
        dcc.ip = server->getLocalIP();
        dcc.port = 0;
        for(FXint i=0; i<m_dccfilesList.no(); i++)
        {
            if(dcc.path == m_dccfilesList[i].path && (m_dccfilesList[i].type==DCC_IN || m_dccfilesList[i].type==DCC_PIN))
            {
                m_dccfilesList.erase(i);
                break;
            }
        }
        m_dccfilesList.append(dcc);
        if(isForResume(FXPath::name(dcc.path)))
        {
            server->sendCtcp(ev->param1, "DCC RESUME "+FXPath::name(dcc.path)+" 0 "+FXStringVal(FXStat::size(dcc.path+".part"))+" "+FXStringVal(dcc.token));
        }
        else
        {
            DccEngine *engine = new DccEngine(m_app, this, dcc, server);
            m_dccengines.append(engine);
            engine->startConnection();
            appendIrcStyledText(FXStringFormat(_("Receiving \"%s\" from %s"), ev->param2.text(), ev->param1.text()),8,FALSE);
        }
    }
    else
    {
        if(isForResume(ev->param2)
                && dxEXMessageBox::question(this, MBOX_YES_NO, _("Question"), _("%s offers file %s with size %s over DCC passive.\nFile is already partially downloaded.\nDo you want continue in download?"), ev->param1.text(), ev->param2.text(), utils::instance().getFileSize(ev->param3).text()) == 1)
        {
            DccFile dcc;
            dcc.path = m_dccPath+PATHSEPSTRING+ev->param2;
            dcc.speed = 0;
            dcc.currentPosition = 0;
            dcc.size = FXLongVal(ev->param3);
            dcc.type = DCC_PIN;
            dcc.canceled = FALSE;
            dcc.finishedPosition = 0;
            dcc.token = FXIntVal(ev->param4);
            dcc.nick = ev->param1;
            for(FXint i=0; i<m_dccfilesList.no(); i++)
            {
                if(dcc.path == m_dccfilesList[i].path && (m_dccfilesList[i].type==DCC_IN || m_dccfilesList[i].type==DCC_PIN))
                {
                    m_dccfilesList.erase(i);
                    break;
                }
            }
            m_dccfilesList.append(dcc);
            server->sendCtcp(ev->param1, "DCC RESUME "+FXPath::name(dcc.path)+" 0 "+FXStringVal(FXStat::size(dcc.path+".part"))+" "+FXStringVal(dcc.token));
            return;
        }
        if(dxEXMessageBox::question(this, MBOX_YES_NO, _("Question"), _("%s offers file %s with size %s over DCC passive.\n Do you want accept?"), ev->param1.text(), ev->param2.text(), utils::instance().getFileSize(ev->param3).text()) == 1)
        {
            dxEXFileDialog dialog(this, _("Save file"));
            dialog.setFilename(m_dccPath+PATHSEPSTRING+ev->param2);
            if(dialog.execute())
            {
                DccFile dcc;
                dcc.path = getUniqueName(FXPath::directory(dialog.getFilename()), FXPath::title(dialog.getFilename()), FXPath::extension(dialog.getFilename())); // dirty (owerwrite user opinion) but absolutely needed :)
                dcc.speed = 0;
                dcc.currentPosition = 0;
                dcc.size = FXLongVal(ev->param3);
                dcc.type = DCC_PIN;
                dcc.canceled = FALSE;
                dcc.finishedPosition = 0;
                dcc.token = FXIntVal(ev->param4);
                dcc.nick = ev->param1;
                dcc.ip = server->getLocalIP();
                dcc.port = 0;
                for(FXint i=0; i<m_dccfilesList.no(); i++)
                {
                    if(dcc.path == m_dccfilesList[i].path && (m_dccfilesList[i].type==DCC_IN || m_dccfilesList[i].type==DCC_PIN))
                    {
                        m_dccfilesList.erase(i);
                        break;
                    }
                }
                //old .part file have to be deleted
                if(FXStat::exists(dcc.path+".part"))
                    FXFile::remove(dcc.path+".part");
                m_dccfilesList.append(dcc);
                DccEngine *engine = new DccEngine(m_app, this, dcc, server);
                m_dccengines.append(engine);
                engine->startConnection();
                appendIrcStyledText(FXStringFormat(_("Receiving \"%s\" from %s"), ev->param2.text(), ev->param1.text()),8,FALSE);
                onCmdTransfers(NULL, 0, NULL);
            }
        }
    }
}

//handle IrcEvent IRC_DCCPOSITION
void dxirc::onIrcDccPosition(DccEngine*, IrcEvent *ev)
{
    FXint index = -1;
    for(FXint i=0; i<m_dccfilesList.no(); i++)
    {
        if(m_dccfilesList[i].path == ev->dccFile.path)
        {
            index = i;
            break;
        }
    }
    if(index == -1) return;
    m_dccfilesList[index].speed = ev->dccFile.speed;
    m_dccfilesList[index].currentPosition = ev->dccFile.currentPosition;
    m_dccfilesList[index].finishedPosition = ev->dccFile.finishedPosition;
    m_dccfilesList[index].canceled = ev->dccFile.canceled;
    m_dccfilesList[index].ip = ev->dccFile.ip;
    m_dccfilesList[index].port = ev->dccFile.port;
    if(m_dccfilesList[index].type == DCC_IN  || m_dccfilesList[index].type == DCC_PIN)
    {
        if(m_dccfilesList[index].currentPosition >= m_dccfilesList[index].size)
        {
            if(m_dccfilesList[index].canceled)
            {
                appendIrcStyledText(FXStringFormat(_("Download of \"%s\" from %s canceled"), FXPath::name(m_dccfilesList[index].path).text(), m_dccfilesList[index].nick.text()), 4, FALSE);
            }
            else
            {
                appendIrcStyledText(FXStringFormat(_("Download of \"%s\" from %s finished"), FXPath::name(m_dccfilesList[index].path).text(), m_dccfilesList[index].nick.text()), 8, FALSE);
#ifdef HAVE_TRAY
                if(m_trayIcon && m_trayIcon->getIcon() != ICO_NEWFILE && !shown())
                    m_trayIcon->setIcon(ICO_NEWFILE);
#endif //HAVE_TRAY
            }
        }
        else
        {
            if(m_dccfilesList[index].canceled)
            {
                appendIrcStyledText(FXStringFormat(_("Download of \"%s\" from %s canceled"), FXPath::name(m_dccfilesList[index].path).text(), m_dccfilesList[index].nick.text()), 4, FALSE);
            }
        }
    }
    else
    {
        if(m_dccfilesList[index].finishedPosition >= m_dccfilesList[index].size)
        {
            if(m_dccfilesList[index].canceled)
            {
                appendIrcStyledText(FXStringFormat(_("Upload of \"%s\" to %s canceled"), FXPath::name(m_dccfilesList[index].path).text(), m_dccfilesList[index].nick.text()), 4, FALSE);
            }
            else
            {
                appendIrcStyledText(FXStringFormat(_("Upload of \"%s\" to %s finished"), FXPath::name(m_dccfilesList[index].path).text(), m_dccfilesList[index].nick.text()), 8, FALSE);
#ifdef HAVE_TRAY
                if(m_trayIcon && m_trayIcon->getIcon() != ICO_NEWFILE && !shown())
                    m_trayIcon->setIcon(ICO_NEWFILE);
#endif //HAVE_TRAY
            }
        }
        else
        {
            if(m_dccfilesList[index].canceled)
            {
                appendIrcStyledText(FXStringFormat(_("Upload of \"%s\" to %s canceled"), FXPath::name(m_dccfilesList[index].path).text(), m_dccfilesList[index].nick.text()), 4, FALSE);
            }
        }
    }
}

//handle IrcEvent IRC_DCCRESUME
void dxirc::onIrcDccResume(IrcEngine *server, IrcEvent *ev)
{
    FXint engineIndex = -1;
    for(FXint i=0; i<m_dccengines.no(); i++)
    {
        if(m_dccengines[i]->isForResume(ev->param1, ev->param2, FXIntVal(ev->param3)))
        {
            engineIndex = i;
            break;
        }
    }
    if(engineIndex == -1) return;
    FXint dccIndex = -1;
    for(FXint i=0; i<m_dccfilesList.no(); i++)
    {
        if(m_dccfilesList[i].path == m_dccengines[engineIndex]->getDccFile().path
                && m_dccfilesList[i].size == m_dccengines[engineIndex]->getDccFile().size
                && m_dccfilesList[i].nick == m_dccengines[engineIndex]->getDccFile().nick)
        {
            dccIndex = i;
            break;
        }
    }
    if(dccIndex == -1) return;
    FXulong position = FXULongVal(ev->param4);
    if(position >= m_dccfilesList[dccIndex].size) return;
    m_dccengines[engineIndex]->setDccPosition(position);
    m_dccfilesList[dccIndex].ip = m_dccengines[engineIndex]->getDccFile().ip;
    m_dccfilesList[dccIndex].port = m_dccengines[engineIndex]->getDccFile().port;
    m_dccfilesList[dccIndex].currentPosition = position;
    m_dccfilesList[dccIndex].speed = 0;
    m_dccfilesList[dccIndex].finishedPosition = position;
    m_dccfilesList[dccIndex].canceled = FALSE;
    server->sendCtcp(ev->param1, "DCC ACCEPT "+utils::instance().removeSpaces(m_dccengines[engineIndex]->getDccFile().path.rafter(PATHSEP))+" "+FXStringVal(m_dccengines[engineIndex]->getDccFile().port)+" "+ev->param4);
}

//handle IrcEvent IRC_DCCPRESUME
void dxirc::onIrcDccPresume(IrcEngine *server, IrcEvent *ev)
{
    FXint dccIndex = -1;
    FXint token = FXIntVal(ev->param1);
    for(FXint i=0; i<m_dccfilesList.no(); i++)
    {
        if(m_dccfilesList[i].token == token)
        {
            dccIndex = i;
            break;
        }
    }
    if(dccIndex == -1) return;
    FXulong position = FXULongVal(ev->param2);
    if(position >= m_dccfilesList[dccIndex].size) return;
    m_dccfilesList[dccIndex].currentPosition = position;
    m_dccfilesList[dccIndex].speed = 0;
    m_dccfilesList[dccIndex].finishedPosition = position;
    m_dccfilesList[dccIndex].canceled = FALSE;
    server->sendCtcp(m_dccfilesList[dccIndex].nick, "DCC ACCEPT "+utils::instance().removeSpaces(m_dccfilesList[dccIndex].path.rafter(PATHSEP))+" 0 "+ev->param2+" "+FXStringVal(token));
}

//handle IrcEvent IRC_DCCACCEPT
void dxirc::onIrcDccAccept(IrcEngine *server, IrcEvent *ev)
{
    FXint dccIndex = -1;
    for(FXint i=0; i<m_dccfilesList.no(); i++)
    {
        if(m_dccfilesList[i].nick == ev->param1 && FXPath::name(m_dccfilesList[i].path) == ev->param2
                && m_dccfilesList[i].port == FXIntVal(ev->param3))
        {
            dccIndex = i;
            break;
        }
    }
    if(dccIndex == -1) return;
    FXulong position = FXULongVal(ev->param4);
    if(position >= m_dccfilesList[dccIndex].size) return;
    m_dccfilesList[dccIndex].currentPosition = position;
    m_dccfilesList[dccIndex].speed = 0;
    m_dccfilesList[dccIndex].finishedPosition = position;
    m_dccfilesList[dccIndex].canceled = FALSE;
    DccEngine *engine = new DccEngine(m_app, this, m_dccfilesList[dccIndex], server);
    m_dccengines.append(engine);
    engine->startConnection();
    appendIrcStyledText(FXStringFormat(_("Receiving \"%s\" from %s"), FXPath::name(m_dccfilesList[dccIndex].path).text(), m_dccfilesList[dccIndex].nick.text()),8);
    if(!m_autoDccFile) onCmdTransfers(NULL, 0, NULL);
}

//handle IrcEvent IRC_DCCPACCEPT
void dxirc::onIrcDccPaccept(IrcEngine *server, IrcEvent *ev)
{
    FXint dccIndex = -1;
    FXint token = FXIntVal(ev->param1);
    for(FXint i=0; i<m_dccfilesList.no(); i++)
    {
        if(m_dccfilesList[i].token == token)
        {
            dccIndex = i;
            break;
        }
    }
    if(dccIndex == -1) return;
    FXulong position = FXULongVal(ev->param2);
    if(position >= m_dccfilesList[dccIndex].size) return;
    m_dccfilesList[dccIndex].currentPosition = position;
    m_dccfilesList[dccIndex].speed = 0;
    m_dccfilesList[dccIndex].finishedPosition = position;
    m_dccfilesList[dccIndex].canceled = FALSE;
    m_dccfilesList[dccIndex].ip = server->getLocalIP();
    m_dccfilesList[dccIndex].port = 0;
    DccEngine *engine = new DccEngine(m_app, this, m_dccfilesList[dccIndex], server);
    m_dccengines.append(engine);
    engine->startConnection();
    appendIrcStyledText(FXStringFormat(_("Receiving \"%s\" from %s"), FXPath::name(m_dccfilesList[dccIndex].path).text(), m_dccfilesList[dccIndex].nick.text()),8,FALSE);
    if(!m_autoDccFile) onCmdTransfers(NULL, 0, NULL);
}

long dxirc::onTabBook(FXObject *, FXSelector, void *ptr)
{
    FXint index = (FXint)(FXival)ptr*2;
    if(m_tabbook->childAtIndex(index)->getMetaClass()==&IrcTabItem::metaClass)
    {
        IrcTabItem *currenttab = static_cast<IrcTabItem*>(m_tabbook->childAtIndex(index));
        utils::instance().debugLine(FXStringFormat("OnTabBook(%d), Class: %s, id: %d", index, currenttab->getClassName(), currenttab->getID()));
        if(m_appTheme.fore != currenttab->getTextColor()) currenttab->setTextColor(m_appTheme.fore);
        if(currenttab->getType() == CHANNEL && currenttab->getIcon() == ICO_CHANNELNEWMSG)
        {
            currenttab->setIcon(ICO_CHANNEL);
#ifdef HAVE_TRAY
            if(m_trayIcon && m_trayIcon->getIcon() == ICO_NEWMSG)
                m_trayIcon->setIcon(ICO_TRAY);
#endif //HAVE_TRAY
        }
        if(currenttab->getType() == QUERY && currenttab->getIcon() == ICO_QUERYNEWMSG)
        {
            currenttab->setIcon(ICO_QUERY);
#ifdef HAVE_TRAY
            if(m_trayIcon && m_trayIcon->getIcon() == ICO_NEWMSG)
                m_trayIcon->setIcon(ICO_TRAY);
#endif //HAVE_TRAY
        }
        currenttab->setFocus();
        currenttab->setCommandFocus();
        currenttab->makeLastRowVisible();
        if(hasTetrisTab())
        {
            TetrisTabItem *tetristab = static_cast<TetrisTabItem*>(getTabItemById(getTabId("tetris")));
            if(tetristab && tetristab->isPauseEnable() && !tetristab->isPaused()) tetristab->pauseResumeGame();
        }
        if(currenttab->getType() == SERVER)
            updateStatus(currenttab->getRealServerName()+"-"+currenttab->getNickName());
        else if(currenttab->getType() == OTHER)
            updateStatus(currenttab->getText());
        else
            updateStatus((currenttab->getText()[0]=='&' ? "&"+currenttab->getText(): currenttab->getText())+"-"+currenttab->getRealServerName()+"-"+currenttab->getNickName());
    }
    if(m_tabbook->childAtIndex(index)->getMetaClass()==&DccTabItem::metaClass)
    {
        DccTabItem *currenttab = static_cast<DccTabItem*>(m_tabbook->childAtIndex(index));
        utils::instance().debugLine(FXStringFormat("OnTabBook(%d), Class: %s, id: %d", index, currenttab->getClassName(), currenttab->getID()));
        if(m_appTheme.fore != currenttab->getTextColor()) currenttab->setTextColor(m_appTheme.fore);
        if(currenttab->getIcon() == ICO_DCCNEWMSG)
            currenttab->setIcon(ICO_DCC);
#ifdef HAVE_TRAY
        if(m_trayIcon && m_trayIcon->getIcon() == ICO_NEWMSG)
            m_trayIcon->setIcon(ICO_TRAY);
#endif //HAVE_TRAY
        currenttab->setFocus();
        currenttab->setCommandFocus();
        currenttab->makeLastRowVisible();
        if(hasTetrisTab())
        {
            TetrisTabItem *tetristab = static_cast<TetrisTabItem*>(getTabItemById(getTabId("tetris")));
            if(tetristab && tetristab->isPauseEnable() && !tetristab->isPaused()) tetristab->pauseResumeGame();
        }
        updateStatus((currenttab->getText()[0]=='&' ? "&"+currenttab->getText(): currenttab->getText())+"-"+currenttab->getServerName()+"-"+currenttab->getNickName());
    }
    if(m_tabbook->childAtIndex(index)->getMetaClass()==&TetrisTabItem::metaClass)
    {
        TetrisTabItem *tetristab = static_cast<TetrisTabItem*>(m_tabbook->childAtIndex(index));
        utils::instance().debugLine(FXStringFormat("OnTabBook(%d), Class: %s, id: %d", index, tetristab->getClassName(), tetristab->getID()));
        tetristab->setFocus();
        tetristab->setGameFocus();
    }
    return 1;
}

long dxirc::onCmdNextTab(FXObject *, FXSelector, void *)
{
    FXint index = m_tabbook->getCurrent();
    if(m_tabbook->numChildren())
    {
        if((index+1)*2 < m_tabbook->numChildren()) m_tabbook->setCurrent(index+1, m_tabbook->numChildren() > index*2 ? TRUE : FALSE);
        else m_tabbook->setCurrent(0, TRUE);
    }
    return 1;
}

//Handle mousewheel for change currenttab
long dxirc::onMouseWheel(FXObject *, FXSelector, void *ptr)
{
    FXEvent *event = (FXEvent*)ptr;
    FXint index = m_tabbook->getCurrent();
    if(event->code > 0) //positive movement
    {
        if(m_tabbook->numChildren())
        {
            if((index+1)*2 < m_tabbook->numChildren()) m_tabbook->setCurrent(index+1, m_tabbook->numChildren() > index*2 ? TRUE : FALSE);
            else m_tabbook->setCurrent(0, TRUE);
        }
    }
    else
    {
        if(m_tabbook->numChildren())
        {
            if((index-1) >= 0) m_tabbook->setCurrent(index-1, TRUE);
            else m_tabbook->setCurrent(m_tabbook->numChildren()/2-1, TRUE);
        }
    }
    return 1;
}

long dxirc::onCmdNextUnread(FXObject *, FXSelector, void*)
{
    if(m_tabbook->numChildren())
    {
        for(FXint i = m_tabbook->getCurrent()*2; i<m_tabbook->numChildren(); i+=2)
        {
            if (m_appTheme.fore != static_cast<FXTabItem*>(m_tabbook->childAtIndex(i))->getTextColor())
            {
                m_tabbook->setCurrent(i/2, TRUE);
                return 1;
            }
        }
        for(FXint i = m_tabbook->getCurrent()*2; i>-1; i-=2)
        {
            if (m_appTheme.fore != static_cast<FXTabItem*>(m_tabbook->childAtIndex(i))->getTextColor())
            {
                m_tabbook->setCurrent(i/2, TRUE);
                return 1;
            }
        }
    }
    return 1;
}

long dxirc::onCmdClear(FXObject *, FXSelector, void *)
{
    if(m_tabbook->numChildren())
    {
        FXint index = m_tabbook->getCurrent()*2;
        if(m_tabbook->childAtIndex(index)->getMetaClass()!=&TetrisTabItem::metaClass)
        {
            dxTabItem *currenttab = static_cast<dxTabItem*>(m_tabbook->childAtIndex(index));
            currenttab->clearChat();
        }
    }
    return 1;
}

long dxirc::onCmdClearAll(FXObject *, FXSelector, void *)
{
    for(FXint i = 0; i<m_tabbook->numChildren(); i+=2)
    {
        if(m_tabbook->childAtIndex(i)->getMetaClass()!=&TetrisTabItem::metaClass)
        {
            static_cast<dxTabItem*>(m_tabbook->childAtIndex(i))->clearChat();
            if (m_appTheme.fore != static_cast<dxTabItem*>(m_tabbook->childAtIndex(i))->getTextColor()) static_cast<dxTabItem*>(m_tabbook->childAtIndex(i))->setTextColor(m_appTheme.fore);
            if(static_cast<IrcTabItem*>(m_tabbook->childAtIndex(i))->getType() == CHANNEL) static_cast<IrcTabItem*>(m_tabbook->childAtIndex(i))->setIcon(ICO_CHANNEL);
            if(static_cast<IrcTabItem*>(m_tabbook->childAtIndex(i))->getType() == QUERY) static_cast<IrcTabItem*>(m_tabbook->childAtIndex(i))->setIcon(ICO_QUERY);
            if(m_tabbook->childAtIndex(i)->getMetaClass()==&DccTabItem::metaClass) static_cast<DccTabItem*>(m_tabbook->childAtIndex(i))->setIcon(ICO_DCC);
        }
    }
#ifdef HAVE_TRAY
    if(m_trayIcon && m_trayIcon->getIcon() == ICO_NEWMSG)
        m_trayIcon->setIcon(ICO_TRAY);
#endif //HAVE_TRAY
    return 1;
}

long dxirc::onCmdCloseTab(FXObject *, FXSelector, void *)
{
    if(m_tabbook->numChildren())
    {
        FXint index = m_tabbook->getCurrent()*2;
        if(m_tabbook->childAtIndex(index)->getMetaClass()==&IrcTabItem::metaClass)
        {
            IrcTabItem *currenttab = static_cast<IrcTabItem*>(m_tabbook->childAtIndex(index));
            if(currenttab->getType() == OTHER)
            {
                delete m_tabbook->childAtIndex(index);
                delete m_tabbook->childAtIndex(index);
                m_tabbook->recalc();
                if(m_tabbook->numChildren())
                {
                    m_tabbook->setCurrent(FXMAX(0,index/2-1), TRUE);
                }
                sortTabs();
                updateMenus();
                return 1;
            }
            IrcEngine *currentserver = NULL;
            for(FXint i=0; i < m_ircengines.no(); i++)
            {
                if(m_ircengines[i]->findTarget(currenttab))
                {
                    currentserver = m_ircengines[i];
                    break;
                }
            }
            if(currentserver == NULL) return 0;
            if(currenttab->getType() == QUERY)
            {
                if(currentserver->getConnected() && isLastTab(currentserver))
                {
                    for(FXint j = 0; j < m_tabbook->numChildren(); j+=2)
                    {
                        if(currentserver->findTarget(static_cast<IrcTabItem*>(m_tabbook->childAtIndex(j))))
                        {
                            static_cast<IrcTabItem*>(m_tabbook->childAtIndex(j))->setType(SERVER, currentserver->getNetworkName());
                            m_tabbook->setCurrent(j/2-1, TRUE);
                            break;
                        }
                    }
                }
                else
                {
                    if(!currentserver->getConnected()) currentserver->disconnect();
                    currentserver->removeTarget(currenttab);
                    delete m_tabbook->childAtIndex(index);
                    delete m_tabbook->childAtIndex(index);
                    m_tabbook->recalc();
                    if(m_tabbook->numChildren())
                    {
                        m_tabbook->setCurrent(FXMAX(0,index/2-1), TRUE);
                    }
                }
                sortTabs();
                updateMenus();
                return 1;
            }
            if(currenttab->getType() == CHANNEL)
            {
                if(currentserver->getConnected()) currentserver->sendPart(currenttab->getText());
                if(currentserver->getConnected() && isLastTab(currentserver))
                {
                    for(FXint j = 0; j < m_tabbook->numChildren(); j+=2)
                    {
                        if(currentserver->findTarget(static_cast<IrcTabItem*>(m_tabbook->childAtIndex(j))))
                        {
                            static_cast<IrcTabItem*>(m_tabbook->childAtIndex(j))->setType(SERVER, currentserver->getNetworkName());
                            m_tabbook->setCurrent(j/2-1, TRUE);
                            break;
                        }
                    }
                }
                else
                {

                    if(!currentserver->getConnected()) currentserver->disconnect();
                    currentserver->removeTarget(currenttab);
                    delete m_tabbook->childAtIndex(index);
                    delete m_tabbook->childAtIndex(index);
                    m_tabbook->recalc();
                    if(m_tabbook->numChildren())
                    {
                        m_tabbook->setCurrent(FXMAX(0,index/2-1), TRUE);
                    }
                }
                sortTabs();
                updateMenus();
                return 1;
            }
            if(currenttab->getType() == SERVER)
            {
                currentserver->disconnect();
                for(FXint i = m_tabbook->numChildren()-2; i > -1; i-=2)
                {
                    if(currentserver->findTarget(static_cast<IrcTabItem*>(m_tabbook->childAtIndex(i))))
                    {
                        currentserver->removeTarget(static_cast<IrcTabItem*>(m_tabbook->childAtIndex(i)));
                        delete m_tabbook->childAtIndex(i);
                        delete m_tabbook->childAtIndex(i);
                    }
                }
                m_tabbook->recalc();
                if(m_tabbook->numChildren())
                {
                    m_tabbook->setCurrent(FXMAX(0,index/2-1), TRUE);
                }
                sortTabs();
                updateMenus();
                return 1;
            }
        }
        else if(m_tabbook->childAtIndex(index)->getMetaClass()==&DccTabItem::metaClass)
        {
            DccTabItem *dcctab = static_cast<DccTabItem*>(m_tabbook->childAtIndex(index));
            dcctab->disconnect();
            delete m_tabbook->childAtIndex(index);
            delete m_tabbook->childAtIndex(index);
            m_tabbook->recalc();
            if(m_tabbook->numChildren())
            {
                m_tabbook->setCurrent(FXMAX(0,index/2-1), TRUE);
            }
            sortTabs();
            updateMenus();
            return 1;
        }
        else
        {
            TetrisTabItem *tetristab = static_cast<TetrisTabItem*>(m_tabbook->childAtIndex(index));
            tetristab->stopGame();
            delete m_tabbook->childAtIndex(index);
            delete m_tabbook->childAtIndex(index);
            m_tabbook->recalc();
            if(m_tabbook->numChildren())
            {
                m_tabbook->setCurrent(FXMAX(0,index/2-1), TRUE);
            }
            sortTabs();
            updateMenus();
            return 1;
        }
    }
    return 1;
}

long dxirc::onCmdSelectTab(FXObject*, FXSelector, void *ptr)
{
    FXint index = 0;
    FXEvent* event = (FXEvent*)ptr;
    switch(event->code){
        case KEY_1:
        {
            index = 0;
            break;
        }
        case KEY_2:
        {
            index = 1;
            break;
        }
        case KEY_3:
        {
            index = 2;
            break;
        }
        case KEY_4:
        {
            index = 3;
            break;
        }
        case KEY_5:
        {
            index = 4;
            break;
        }
        case KEY_6:
        {
            index = 5;
            break;
        }
        case KEY_7:
        {
            index = 6;
            break;
        }
        case KEY_8:
        {
            index = 7;
            break;
        }
        case KEY_9:
        {
            index = 8;
            break;
        }
    }
    if((index)*2 < m_tabbook->numChildren()) m_tabbook->setCurrent(index, TRUE);
    return 1;
}

long dxirc::onTrayClicked(FXObject*, FXSelector, void*)
{
#ifdef HAVE_TRAY
    if(shown())
        hide();
    else
    {
        show();
        if(m_trayIcon->getIcon() == ICO_NEWFILE) onCmdTransfers(NULL, 0, NULL);
    }
    if(m_trayIcon && m_trayIcon->getIcon() != ICO_TRAY)
        m_trayIcon->setIcon(ICO_TRAY);
#endif //HAVE_TRAY
    return 1;
}

//handle highlighted msg from IrcTabITem::ID_NEWMSG or DccTabItem::ID_NEWMSG
long dxirc::onNewMsg(FXObject *obj, FXSelector, void*)
{
#ifdef HAVE_TRAY
    if(m_trayIcon && m_trayIcon->getIcon() == ICO_TRAY && (!shown() || static_cast<dxTabItem*>(m_tabbook->childAtIndex(m_tabbook->getCurrent()*2)) != static_cast<dxTabItem*>(obj)))
        m_trayIcon->setIcon(ICO_NEWMSG);
#endif //HAVE_TRAY
    if(m_sounds && m_soundMessage && (!shown() || isMinimized() || static_cast<dxTabItem*>(m_tabbook->childAtIndex(m_tabbook->getCurrent()*2)) != static_cast<dxTabItem*>(obj)))
        utils::instance().playFile(m_pathMessage);
    if(static_cast<IrcTabItem*>(obj)->getType() == CHANNEL)
    {
        updateStatus(FXStringFormat(_("New highlighted message on %s"), static_cast<IrcTabItem*>(obj)->getText().text()));
        if(m_notify && m_notifyMessage)
            showNotify(FXStringFormat(_("New highlighted message on %s"), static_cast<IrcTabItem*>(obj)->getText().text()));
    }
    else
    {
        updateStatus(FXStringFormat(_("New message on chat with %s"), static_cast<dxTabItem*>(obj)->getText().text()));
        if(m_notify && m_notifyMessage)
            showNotify(FXStringFormat(_("New message on chat with %s"), static_cast<dxTabItem*>(obj)->getText().text()));
    }
    flash(TRUE);
    return 1;
}

//handle for: /ignore addcmd
long dxirc::onAddIgnoreCommand(FXObject *sender, FXSelector, void *data)
{
    if(sender->getMetaClass()!=&IrcTabItem::metaClass) return 0;
    IrcTabItem *tab = static_cast<IrcTabItem*>(sender);
    FXString text = static_cast<FXString*>(data)->text();
    FXString available[10] = { "away", "ban", "ctcp", "join", "me", "nick", "notice", "mode", "part", "quit"};
    FXbool canAdd = FALSE;
    for(FXint i=0; i<10; i++)
    {
        if(comparecase(text,available[i])==0)
        {
            canAdd = TRUE;
            break;
        }
    }
    if(!canAdd)
    {
        tab->appendStyledText(FXStringFormat(_("'%s' can't be added to ignored commands"), text.text()), 4, FALSE, FALSE, FALSE);
        return 1;
    }
    else
    {
        for(FXint i=0; i<m_commandsList.contains(';'); i++)
        {
            if(comparecase(text,m_commandsList.section(';', i))==0)
            {
                tab->appendStyledText(FXStringFormat(_("'%s' is already added in ignored commands"), text.text()), 4, FALSE, FALSE, FALSE);
                return 1;
            }
        }
        tab->appendStyledText(FXStringFormat(_("'%s' was added to ignored commands"), text.text()), 3, FALSE, FALSE, FALSE);
        m_commandsList.append(text+";");
        saveConfig();
        for(FXint i = 0; i<m_tabbook->numChildren(); i+=2)
        {
            if(m_tabbook->childAtIndex(i)->getMetaClass()==&IrcTabItem::metaClass)
            {
                IrcTabItem *irctab = static_cast<IrcTabItem*>(m_tabbook->childAtIndex(i));
                irctab->setCommandsList(m_commandsList);
            }
        }
    }
    return 1;
}

//handle for: /ignore rmcmd
long dxirc::onRemoveIgnoreCommand(FXObject *sender, FXSelector, void *data)
{
    if(sender->getMetaClass()!=&IrcTabItem::metaClass) return 0;
    IrcTabItem *tab = static_cast<IrcTabItem*>(sender);
    FXString text = static_cast<FXString*>(data)->text();
    FXString tempList = "";
    FXbool inCommands = FALSE;
    for(FXint i=0; i<m_commandsList.contains(';'); i++)
    {
        if(comparecase(text,m_commandsList.section(';', i))==0)
        {
            tab->appendStyledText(FXStringFormat(_("'%s' was removed from ignored commands"), text.text()), 3, FALSE, FALSE, FALSE);
            inCommands = TRUE;
        }
        else
            tempList.append(m_commandsList.section(';', i)+";");
    }
    if(!inCommands) tab->appendStyledText(FXStringFormat(_("'%s' isn't in ignored commands"), text.text()), 4, FALSE, FALSE, FALSE);
    m_commandsList = tempList;
    saveConfig();
    for(FXint i = 0; i<m_tabbook->numChildren(); i+=2)
    {
        if(m_tabbook->childAtIndex(i)->getMetaClass()==&IrcTabItem::metaClass)
        {
            IrcTabItem *irctab = static_cast<IrcTabItem*>(m_tabbook->childAtIndex(i));
            irctab->setCommandsList(m_commandsList);
        }
    }
    return 1;
}

//handle for: /ignore addusr
long dxirc::onAddIgnoreUser(FXObject *sender, FXSelector, void *data)
{
    if(sender->getMetaClass()!=&IrcTabItem::metaClass) return 0;
    IrcTabItem *tab = static_cast<IrcTabItem*>(sender);
    FXString text = static_cast<FXString*>(data)->text();
    FXString user = text.section(' ',0);
    FXString channel = text.section(' ',1);
    FXString network = utils::instance().getParam(text, 3, TRUE);
    if(m_usersList.no())
    {
        FXbool updated = FALSE;
        for(FXint i=0; i<m_usersList.no(); i++)
        {
            if(compare(user, m_usersList[i].nick)==0)
            {
                updated = TRUE;
                channel.empty() ? m_usersList[i].channel = "all" : m_usersList[i].channel = channel;
                network.empty() ? m_usersList[i].network = "all" : m_usersList[i].network = network;
                tab->appendStyledText(FXStringFormat(_("'%s' was updated in ignored users"), user.text()), 3, FALSE, FALSE, FALSE);
                break;
            }
        }
        if(!updated)
        {
            IgnoreUser iuser;
            iuser.nick = user;
            channel.empty() ? iuser.channel = "all" : iuser.channel = channel;
            network.empty() ? iuser.network = "all" : iuser.network = network;
            m_usersList.append(iuser);
            tab->appendStyledText(FXStringFormat(_("'%s' was added to ignored users"), user.text()), 3, FALSE, FALSE, FALSE);
        }
    }
    else
    {
        IgnoreUser iuser;
        iuser.nick = user;
        channel.empty() ? iuser.channel = "all" : iuser.channel = channel;
        network.empty() ? iuser.network = "all" : iuser.network = network;
        m_usersList.append(iuser);
        tab->appendStyledText(FXStringFormat(_("'%s' was added to ignored users"), user.text()), 3, FALSE, FALSE, FALSE);
    }
    saveConfig();
    for(FXint i = 0; i<m_ircengines.no(); i++)
    {
        m_ircengines[i]->setUsersList(m_usersList);
    }
    return 1;
}

//handle for: /ignore rmusr
long dxirc::onRemoveIgnoreUser(FXObject *sender, FXSelector, void *data)
{
    if(sender->getMetaClass()!=&IrcTabItem::metaClass) return 0;
    IrcTabItem *tab = static_cast<IrcTabItem*>(sender);
    FXString text = static_cast<FXString*>(data)->text();
    FXbool updated = FALSE;
    if(m_usersList.no())
    {
        for(FXint i=m_usersList.no()-1; i>-1; i--)
        {
            if(compare(text, m_usersList[i].nick)==0)
            {
                updated = TRUE;
                m_usersList.erase(i);
                tab->appendStyledText(FXStringFormat(_("'%s' was removed from ignored users"), text.text()), 3, FALSE, FALSE, FALSE);
                break;
            }
        }
        if(!updated)
        {
            tab->appendStyledText(FXStringFormat(_("'%s' wasn't removed from ignored users"), text.text()), 3, FALSE, FALSE, FALSE);
        }
    }
    saveConfig();
    for(FXint i = 0; i<m_ircengines.no(); i++)
    {
        m_ircengines[i]->setUsersList(m_usersList);
    }
    return 1;
}

long dxirc::onNewTetris(FXObject*, FXSelector, void*)
{
    if(hasTetrisTab()) return 1;
    TetrisTabItem *tab = new TetrisTabItem(m_tabbook, "tetris", 0, TAB_TOP, m_lastID);
    sendNewTab(NULL, "tetris", m_lastID, TRUE, FALSE, OTHER);
    m_lastID++;
    tab->create();
    tab->createGeom();
    tab->setColor(m_colors);
    updateTabPosition();
    sortTabs();
    updateMenus();
    m_tabbook->setCurrent(m_tabbook->numChildren()/2-1, TRUE);
    return 1;
}

long dxirc::onTetrisKey(FXObject*, FXSelector, void *ptr)
{
    if(m_tabbook->childAtIndex(m_tabbook->getCurrent()*2)->getMetaClass()!=&TetrisTabItem::metaClass) return 1;
    FXEvent* event = (FXEvent*)ptr;
    switch(event->code){
        case KEY_N:
        case KEY_n:
        {
            static_cast<TetrisTabItem*>(m_tabbook->childAtIndex(m_tabbook->getCurrent()*2))->newGame();
            break;
        }
        case KEY_P:
        case KEY_p:
        {
            static_cast<TetrisTabItem*>(m_tabbook->childAtIndex(m_tabbook->getCurrent()*2))->pauseResumeGame();
            break;
        }
        case KEY_I:
        case KEY_i:
        {
            static_cast<TetrisTabItem*>(m_tabbook->childAtIndex(m_tabbook->getCurrent()*2))->rotate();
            break;
        }
        case KEY_L:
        case KEY_l:
        {
            static_cast<TetrisTabItem*>(m_tabbook->childAtIndex(m_tabbook->getCurrent()*2))->moveRight();
            break;
        }
        case KEY_K:
        case KEY_k:
        {
            static_cast<TetrisTabItem*>(m_tabbook->childAtIndex(m_tabbook->getCurrent()*2))->drop();
            break;
        }
        case KEY_J:
        case KEY_j:
        {
            static_cast<TetrisTabItem*>(m_tabbook->childAtIndex(m_tabbook->getCurrent()*2))->moveLeft();
            break;
        }
    }
    return 1;
}

void dxirc::autoloadScripts()
{
#ifdef HAVE_LUA
    if(m_autoload && FXStat::exists(m_autoloadPath))
    {
        FXDir dir;
        FXString name, pathname;
        FXStat info;
        FXint islink;
        // Assume not a link
        islink = FALSE;
        // Managed to open directory
        if (dir.open(m_autoloadPath))
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
                pathname = m_autoloadPath;
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
#endif //WIN32
                // If it is a directory
                if(info.isDirectory()) continue;
                // If it is not matching pattern skip it
                if (!FXPath::match("*.lua", name))continue;
                loadLuaScript(pathname, FALSE);
            }
            // Close it
            dir.close();
        }
    }
#endif //HAVE_LUA
}

long dxirc::onCmdScripts(FXObject*, FXSelector, void*)
{
#ifdef HAVE_LUA
    ScriptDialog *dialog = new ScriptDialog(this);
    dialog->execute(PLACEMENT_OWNER);
#endif //HAVE_LUA
    return 1;
}

//fired from tab by command /lua
long dxirc::onLua(FXObject *obj, FXSelector, void *data)
{
#ifdef HAVE_LUA
    LuaRequest *lua = (LuaRequest*)data;
    if(lua->type == LUA_LOAD)
    {
        return loadLuaScript(lua->text);
    }
    if(lua->type == LUA_UNLOAD)
    {
        return unloadLuaScript(lua->text);
    }
    if(lua->type == LUA_LIST)
    {
        if(!m_scripts.no())
        {
            appendIrcStyledText(_("Scripts aren't loaded"), 4, FALSE);
            return 0;
        }
        else
        {
            appendIrcStyledText(_("Loaded scrips:"), 7, FALSE);
            for(FXint i=0; i<m_scripts.no(); i++)
            {                
                appendIrcText(FXStringFormat(_("Name: %s"), m_scripts[i].name.text()), FALSE);
                appendIrcText(FXStringFormat(_("Description: %s"), m_scripts[i].description.text()), FALSE);
                appendIrcText(FXStringFormat(_("Version: %s"), m_scripts[i].version.text()), FALSE);
                appendIrcText(FXStringFormat(_("Path: %s"), m_scripts[i].path.text()), FALSE);
                appendIrcText(utils::instance().availableScriptCommands(m_scripts[i].name), FALSE);
                if(i+1<m_scripts.no()) appendIrcText("", FALSE);
            }
        }
        return 1;
    }
    if(lua->type == LUA_COMMAND)
    {
        dxTabItem *tab = static_cast<dxTabItem*>(obj);
        FXString command = lua->text.before(' ');
        FXString text = lua->text.after(' ');
        for(FXint i=0; i<m_scripts.no(); i++)
        {
            if(comparecase(utils::instance().getScriptName(command), m_scripts[i].name) == 0)
            {
                lua_pushstring(m_scripts[i].L, utils::instance().getFuncname(command).text());
                lua_gettable(m_scripts[i].L, LUA_GLOBALSINDEX);
                if(lua_isfunction(m_scripts[i].L, -1))
                {
                    lua_pushstring(m_scripts[i].L, text.text());
                    lua_pushnumber(m_scripts[i].L, tab->getID());
                    if(lua_pcall(m_scripts[i].L, 2, 0, 0)) appendIrcStyledText(FXStringFormat(_("Error: %s"), lua_tostring(m_scripts[i].L, -1)), 4, FALSE);
                }
                else lua_pop(m_scripts[i].L, 1);
                return 1;
            }
        }
    }
#else
    appendIrcStyledText(_("dxirc is compiled without support for Lua scripting"), 4, FALSE);
#endif //HAVE_LUA
    return 1;
}

//Handle for entered text in IrcTab for mymsg in lua scripting
long dxirc::onIrcMyMsg(FXObject *sender, FXSelector, void *data)
{
#ifdef HAVE_LUA
    if(sender->getMetaClass()==&TetrisTabItem::metaClass) return 0;
    dxTabItem *tab = static_cast<dxTabItem*>(sender);
    FXString *text = static_cast<FXString*>(data);
    if(!m_scripts.no() || !m_scriptEvents.no())
    {
        tab->hasMyMsg(FALSE);
        return 0;
    }
    tab->hasMyMsg(hasMyMsg());
    for(FXint i=0; i<m_scriptEvents.no(); i++)
    {
        if(comparecase("mymsg", m_scriptEvents[i].name) == 0)
        {
            for(FXint j=0; j<m_scripts.no(); j++)
            {
                if(comparecase(m_scriptEvents[i].script, m_scripts[j].name) == 0)
                {
                    lua_pushstring(m_scripts[j].L, m_scriptEvents[i].funcname.text());
                    lua_gettable(m_scripts[j].L, LUA_GLOBALSINDEX);
                    if (lua_type(m_scripts[j].L, -1) != LUA_TFUNCTION) lua_pop(m_scripts[j].L, 1);
                    else
                    {
                        lua_pushstring(m_scripts[j].L, text->text());
                        lua_pushinteger(m_scripts[j].L, tab->getID());
                        if (lua_pcall(m_scripts[j].L, 2, 0, 0))
                        {
                            appendIrcStyledText(FXStringFormat(_("Lua plugin: error calling %s %s"), m_scriptEvents[i].funcname.text(), lua_tostring(m_scripts[j].L, -1)), 4, FALSE);
                            lua_pop(m_scripts[j].L, 1);
                        }
                    }
                }
            }
        }
    }
#endif //HAVE_LUA
    return 1;
}

//send event to script for new created tab
void dxirc::sendNewTab(IrcEngine *server, const FXString &name, FXint id, FXbool isTetris, FXbool isDccTab, TYPE type)
{
#ifdef HAVE_LUA
    if(!m_scripts.no() || !m_scriptEvents.no())
        return;
    for(FXint i=0; i<m_scriptEvents.no(); i++)
    {
        if(comparecase("newtab", m_scriptEvents[i].name) == 0)
        {
            for(FXint j=0; j<m_scripts.no(); j++)
            {
                if(comparecase(m_scriptEvents[i].script, m_scripts[j].name) == 0)
                {
                    lua_pushstring(m_scripts[j].L, m_scriptEvents[i].funcname.text());
                    lua_gettable(m_scripts[j].L, LUA_GLOBALSINDEX);
                    if (lua_type(m_scripts[j].L, -1) != LUA_TFUNCTION) lua_pop(m_scripts[j].L, 1);
                    else
                    {
                        lua_newtable(m_scripts[j].L);
                        lua_pushstring(m_scripts[j].L, "id");
                        lua_pushinteger(m_scripts[j].L, id);
                        lua_settable(m_scripts[j].L, -3);
                        lua_pushstring(m_scripts[j].L, "name");
                        lua_pushstring(m_scripts[j].L, name.text());
                        lua_settable(m_scripts[j].L, -3);
                        if(isTetris)
                        {
                            lua_pushstring(m_scripts[j].L, "type");
                            lua_pushstring(m_scripts[j].L, "tetris");
                            lua_settable(m_scripts[j].L, -3);
                        }
                        else if(isDccTab)
                        {
                            lua_pushstring(m_scripts[j].L, "type");
                            lua_pushstring(m_scripts[j].L, "dccchat");
                            lua_settable(m_scripts[j].L, -3);
                        }
                        else
                        {
                            switch(type) {
                                case SERVER:
                                {
                                    lua_pushstring(m_scripts[j].L, "type");
                                    lua_pushstring(m_scripts[j].L, "server");
                                    lua_settable(m_scripts[j].L, -3);
                                }break;
                                case CHANNEL:
                                {
                                    lua_pushstring(m_scripts[j].L, "type");
                                    lua_pushstring(m_scripts[j].L, "channel");
                                    lua_settable(m_scripts[j].L, -3);
                                }break;
                                case QUERY:
                                {
                                    lua_pushstring(m_scripts[j].L, "type");
                                    lua_pushstring(m_scripts[j].L, "query");
                                    lua_settable(m_scripts[j].L, -3);
                                }break;
                                case OTHER:
                                {
                                    lua_pushstring(m_scripts[j].L, "type");
                                    lua_pushstring(m_scripts[j].L, "other");
                                    lua_settable(m_scripts[j].L, -3);
                                }break;
                            }
                        }
                        lua_pushstring(m_scripts[j].L, "servername");
                        lua_pushstring(m_scripts[j].L, server ? server->getServerName().text() : "");
                        lua_settable(m_scripts[j].L, -3);
                        lua_pushstring(m_scripts[j].L, "port");
                        lua_pushinteger(m_scripts[j].L, server ? server->getServerPort() : 0);
                        lua_settable(m_scripts[j].L, -3);
                        lua_pushstring(m_scripts[j].L, "nick");
                        lua_pushstring(m_scripts[j].L, server ? server->getNickName().text() : "");
                        lua_settable(m_scripts[j].L, -3);
                        if (lua_pcall(m_scripts[j].L, 1, 0, 0))
                        {
                            appendIrcStyledText(FXStringFormat(_("Lua plugin: error calling %s %s"), m_scriptEvents[i].funcname.text(), lua_tostring(m_scripts[j].L, -1)), 4, FALSE);
                            lua_pop(m_scripts[j].L, 1);
                        }
                    }
                }
            }
        }
    }
#endif //HAVE_LUA
}

//Handle for entered command in IrcTab for all in lua scripting
long dxirc::onIrcCommand(FXObject *sender, FXSelector, void *data)
{
#ifdef HAVE_LUA    
    if(sender->getMetaClass()==&TetrisTabItem::metaClass) return 0;
    dxTabItem *tab = static_cast<dxTabItem*>(sender);
    FXString *commandtext = static_cast<FXString*>(data);
    if(!m_scripts.no() || !m_scriptEvents.no())
    {
        tab->hasAllCommand(FALSE);
        return 0;
    }
    tab->hasAllCommand(hasAllCommand());
    for(FXint i=0; i<m_scriptEvents.no(); i++)
    {
        if(comparecase("all", m_scriptEvents[i].name) == 0)
        {            
            for(FXint j=0; j<m_scripts.no(); j++)
            {
                if(comparecase(m_scriptEvents[i].script, m_scripts[j].name) == 0)
                {
                    lua_pushstring(m_scripts[j].L, m_scriptEvents[i].funcname.text());
                    lua_gettable(m_scripts[j].L, LUA_GLOBALSINDEX);
                    if (lua_type(m_scripts[j].L, -1) != LUA_TFUNCTION) lua_pop(m_scripts[j].L, 1);
                    else
                    {
                        if(commandtext->at(0) == '/')
                        {
                            lua_pushstring(m_scripts[j].L, commandtext->before(' ').after('/').text());
                            lua_pushstring(m_scripts[j].L, commandtext->after(' ').text());
                        }
                        else
                        {
                            lua_pushnil(m_scripts[j].L);
                            lua_pushstring(m_scripts[j].L, commandtext->text());
                        }
                        lua_pushinteger(m_scripts[j].L, tab->getID());
                        if (lua_pcall(m_scripts[j].L, 3, 0, 0))
                        {
                            appendIrcStyledText(FXStringFormat(_("Lua plugin: error calling %s %s"), m_scriptEvents[i].funcname.text(), lua_tostring(m_scripts[j].L, -1)), 4, FALSE);
                            lua_pop(m_scripts[j].L, 1);
                        }
                    }
                }
            }
        }
    }
#endif //HAVE_LUA
    return 1;
}

FXbool dxirc::tabExist(IrcEngine *server, FXString name)
{
    for(FXint i = 0; i < m_tabbook->numChildren(); i+=2)
    {
        if(server->findTarget(static_cast<IrcTabItem*>(m_tabbook->childAtIndex(i))) && comparecase(static_cast<FXTabItem*>(m_tabbook->childAtIndex(i))->getText(), name) == 0) return TRUE;
    }
    return FALSE;
}

FXbool dxirc::serverExist(FXString server, FXint port, FXString nick)
{
    for(FXint i = 0; i < m_ircengines.no(); i++)
    {
        if(m_ircengines[i]->getServerName().lower() == server.lower() && m_ircengines[i]->getServerPort() == port && m_ircengines[i]->getNickName().lower() == nick.lower() && m_ircengines[i]->getConnected()) return TRUE;
    }
    return FALSE;
}

FXint dxirc::getServerTab(IrcEngine *server)
{
    for(FXint i = 0; i < m_tabbook->numChildren(); i+=2)
    {        
        if(m_tabbook->childAtIndex(i)->getMetaClass()==&IrcTabItem::metaClass)
        {
            IrcTabItem *tab = static_cast<IrcTabItem*>(m_tabbook->childAtIndex(i));
            if(server->findTarget(tab) && tab->getType() == SERVER) return i;
        }
    }
    return -1;
}

FXint dxirc::getTabId(IrcEngine *server, FXString name)
{
    for(FXint i = 0; i < m_tabbook->numChildren(); i+=2)
    {
        if(server->findTarget(static_cast<IrcTabItem*>(m_tabbook->childAtIndex(i))) && comparecase(name, static_cast<FXTabItem*>(m_tabbook->childAtIndex(i))->getText()) == 0)
            return static_cast<IrcTabItem*>(m_tabbook->childAtIndex(i))->getID();
    }
    return -1;
}

//usefull mainly for tetristab, othertab
FXint dxirc::getTabId(FXString name)
{
    for(FXint i = 0; i < m_tabbook->numChildren(); i+=2)
    {
        if(comparecase(name, static_cast<FXTabItem*>(m_tabbook->childAtIndex(i))->getText()) == 0)
        {
            if(m_tabbook->childAtIndex(i)->getMetaClass()==&TetrisTabItem::metaClass)
                return static_cast<TetrisTabItem*>(m_tabbook->childAtIndex(i))->getID();
            else
                return static_cast<dxTabItem*>(m_tabbook->childAtIndex(i))->getID();
        }
    }
    return -1;
}

FXint dxirc::getCurrentTabId()
{
    FXint index = m_tabbook->getCurrent()*2;
    if(m_tabbook->childAtIndex(index)->getMetaClass()==&TetrisTabItem::metaClass)
        return static_cast<TetrisTabItem*>(m_tabbook->childAtIndex(index))->getID();
    else
        return static_cast<dxTabItem*>(m_tabbook->childAtIndex(index))->getID();
    return -1;
}

void dxirc::setCurrentTabById(FXint id)
{
    for(FXint i = 0; i < m_tabbook->numChildren(); i+=2)
    {
        if(m_tabbook->childAtIndex(i)->getMetaClass()!=&TetrisTabItem::metaClass
                && static_cast<dxTabItem*>(m_tabbook->childAtIndex(i))->getID() == id)
        {
            m_tabbook->setCurrent(i/2, TRUE);
            return;
        }
        if(m_tabbook->childAtIndex(i)->getMetaClass()==&TetrisTabItem::metaClass
                && static_cast<TetrisTabItem*>(m_tabbook->childAtIndex(i))->getID() == id)
        {
            m_tabbook->setCurrent(i/2, TRUE);
            return;
        }
    }
}

FXWindow* dxirc::getTabItemById(FXint id)
{
    for(FXint i = 0; i < m_tabbook->numChildren(); i+=2)
    {
        if(m_tabbook->childAtIndex(i)->getMetaClass()!=&TetrisTabItem::metaClass
                && static_cast<dxTabItem*>(m_tabbook->childAtIndex(i))->getID() == id)
        {
            return m_tabbook->childAtIndex(i);
        }
        if(m_tabbook->childAtIndex(i)->getMetaClass()==&TetrisTabItem::metaClass
                && static_cast<TetrisTabItem*>(m_tabbook->childAtIndex(i))->getID() == id)
        {
            return m_tabbook->childAtIndex(i);
        }
    }
    return NULL;
}

FXbool dxirc::isValidTabId(FXint id)
{
    if(id<0) return FALSE;
    for(FXint i = 0; i < m_tabbook->numChildren(); i+=2)
    {
        if(m_tabbook->childAtIndex(i)->getMetaClass()==&TetrisTabItem::metaClass)
        {
            if(static_cast<TetrisTabItem*>(m_tabbook->childAtIndex(i))->getID()==id) return TRUE;
        }
        else
        {
            if(static_cast<dxTabItem*>(m_tabbook->childAtIndex(i))->getID()==id) return TRUE;
        }
    }
    return FALSE;
}

FXbool dxirc::isIddxTabItem(FXint id)
{
    if(id<0) return FALSE;
    for(FXint i = 0; i < m_tabbook->numChildren(); i+=2)
    {
        if(m_tabbook->childAtIndex(i)->getMetaClass()!=&TetrisTabItem::metaClass)
        {
            if(static_cast<dxTabItem*>(m_tabbook->childAtIndex(i))->getID()==id) return TRUE;
        }
    }
    return FALSE;
}

FXbool dxirc::isFriend(FXString nick, FXString on, FXString network)
{
    FXbool bnick = FALSE;
    FXbool bchannel = FALSE;
    FXbool bnetwork = FALSE;
    for(FXint i=0; i<m_friendsList.no(); i++)
    {
        FXString fnick;
        fnick = m_friendsList[i].nick;
        if(FXRex(FXString("\\<"+fnick+"\\>").lower().substitute("*","\\w*")).match(nick.lower())) bnick = TRUE;
        if(m_friendsList[i].channel == "all") bchannel = TRUE;
        if(m_friendsList[i].channel.contains(','))
        {
            for(FXint j=1; j<m_friendsList[i].channel.contains(',')+2; j++)
            {
                if(FXRex(FXString(utils::instance().getParam(m_friendsList[i].channel, j, FALSE, ',').lower()+"\\>").substitute("*","\\w*")).match(on.lower()))
                {
                    bchannel = TRUE;
                    break;
                }
            }
        }
        else
        {
            if(FXRex(FXString(m_friendsList[i].channel+"\\>").lower().substitute("*","\\w*")).match(on.lower())) bchannel = TRUE;
        }
        if(m_friendsList[i].network == "all") bnetwork = TRUE;
        if(FXRex(FXString("\\<"+m_friendsList[i].network+"\\>").lower().substitute("*","\\w*")).match(network.lower())) bnetwork = TRUE;
    }
    return bnick && bchannel && bnetwork;
}

FXint dxirc::createIrcTab(const FXString& tabtext, FXIcon* icon, TYPE typ, IrcEngine* engine)
{
    FXuint style;
    switch(m_tabPosition) {
        case 0: //bottom
            {
                style = TAB_BOTTOM;
            }break;
        case 1: //left
            {
                style = TAB_LEFT;
            }break;
        case 2: //top
            {
                style = TAB_TOP;
            }break;
        case 3: //right
            {
                style = TAB_RIGHT;
            }break;
        default:
            {
                style = TAB_BOTTOM;
            }
    }
    IrcTabItem *tabitem = new IrcTabItem(m_tabbook, tabtext, icon, style, m_lastID, typ, engine, m_ownServerWindow, m_usersShown, typ == OTHER ? FALSE : m_logging, m_commandsList, m_logPath, m_maxAway, m_colors, m_nickCompletionChar, m_ircFont, m_sameCmd, m_sameList, m_coloredNick, m_stripColors, m_useSpell, m_showSpellCombo);
    sendNewTab(engine, tabtext, m_lastID, FALSE, FALSE, typ);
    m_lastID++;
    tabitem->create();
    tabitem->createGeom();
    tabitem->setSmileys(m_useSmileys, m_smileys);
    tabitem->setUnreadTabColor(m_unreadColor);
    tabitem->setHighlightTabColor(m_highlightColor);
    if(engine) engine->appendTarget(tabitem);
    sortTabs();
    return m_lastID-1;
}

void dxirc::createDccTab(const FXString &mynick, const FXString &nick, const FXString &address, FXint portD, FXint portH, FXbool listen, IrcEngine *engine)
{
FXuint style;
    switch(m_tabPosition) {
        case 0: //bottom
            {
                style = TAB_BOTTOM;
            }break;
        case 1: //left
            {
                style = TAB_LEFT;
            }break;
        case 2: //top
            {
                style = TAB_TOP;
            }break;
        case 3: //right
            {
                style = TAB_RIGHT;
            }break;
        default:
            {
                style = TAB_BOTTOM;
            }
    }
    DccTabItem *tabitem = new DccTabItem(m_tabbook, mynick, nick, address, portD, portH, listen, ICO_DCC, style, m_lastID, m_logging, m_logPath, m_colors, m_ircFont, m_sameCmd, m_coloredNick, m_stripColors, m_useSpell, m_showSpellCombo, engine);
    sendNewTab(NULL, nick, m_lastID, FALSE, TRUE, OTHER);
    m_lastID++;
    tabitem->create();
    tabitem->createGeom();
    tabitem->setSmileys(m_useSmileys, m_smileys);
    tabitem->setUnreadTabColor(m_unreadColor);
    tabitem->setHighlightTabColor(m_highlightColor);
    if(engine)
        engine->appendTarget(tabitem);
    sortTabs();
}

FXbool dxirc::isLastTab(IrcEngine *server)
{
    FXint numTabs = 0;
    for(FXint i = 0; i < m_tabbook->numChildren(); i+=2)
    {
        if(server->findTarget(static_cast<IrcTabItem*>(m_tabbook->childAtIndex(i)))) numTabs++;
    }
    if(numTabs > 1) return FALSE;
    else return TRUE;
}

FXbool dxirc::hasTetrisTab()
{
    if(m_tabbook->numChildren())
    {
        for(FXint i = 0; i < m_tabbook->numChildren()/2; i++)
        {
            if(m_tabbook->childAtIndex(i*2)->getMetaClass()==&TetrisTabItem::metaClass) return TRUE;
        }
    }
    return FALSE;
}

void dxirc::sortTabs()
{
    if(m_tabbook->numChildren()/2 > 1)
    {
        FXint id = getCurrentTabId();
        if(hasTetrisTab())
        {
            FXint index = 0;
            TetrisTabItem *tetristab = NULL;
            dxTabItem* *tabpole = new dxTabItem*[m_tabbook->numChildren()/2-1];
            for(FXint i = 0; i < m_tabbook->numChildren()/2; i++)
            {
                if(m_tabbook->childAtIndex(i*2)->getMetaClass()!=&TetrisTabItem::metaClass)
                {
                    tabpole[index] = static_cast<dxTabItem*>(m_tabbook->childAtIndex(i*2));
                    index++;
                }
                else tetristab = static_cast<TetrisTabItem*>(m_tabbook->childAtIndex(i*2));
            }
            qsort(tabpole, m_tabbook->numChildren()/2-1, sizeof(tabpole[0]), (int(*)(const void*, const void*))&CompareTabs);
            for(FXint i = 0; i < m_tabbook->numChildren()/2-1; i++)
            {
                tabpole[i]->reparentTab();
            }
            tetristab->reparentTab();
            m_tabbook->recalc();
            delete []tabpole;
        }
        else
        {
            dxTabItem* *tabpole = new dxTabItem*[m_tabbook->numChildren()/2];
            for(FXint i = 0; i < m_tabbook->numChildren()/2; i++)
            {
                tabpole[i] = (dxTabItem*)m_tabbook->childAtIndex(i*2);
            }
            qsort(tabpole, m_tabbook->numChildren()/2, sizeof(tabpole[0]), (int(*)(const void*, const void*))&CompareTabs);
            for(int i=0; i < m_tabbook->numChildren()/2; i++)
            {
                tabpole[i]->reparentTab();
            }
            m_tabbook->recalc();
            delete []tabpole;
        }
        setCurrentTabById(id);
    }
}

void dxirc::updateMenus()
{
    if(m_tabbook->numChildren())
    {
        m_closeTab->enable();
        m_clearTab->enable();
        m_clearTabs->enable();
    }
    else
    {
        m_closeTab->disable();
        m_clearTab->disable();
        m_clearTabs->disable();
    }
    FXbool someConnected = FALSE;
    for(FXint i = 0; i < m_ircengines.no(); i++)
    {
        if(m_ircengines[i]->getConnected()) someConnected = TRUE;
    }
    if(someConnected) m_disconnect->enable();
    else m_disconnect->disable();
}

void dxirc::updateStatus(FXString text)
{
    m_statuslabel->setText(text);
#ifdef HAVE_TRAY
    if(m_useTray)
    {
        m_trayIcon->setText("dxirc\n"+text);
    }
#endif //HAVE_TRAY
    m_app->addTimeout(this, dxirc_STIMEOUT, 5000);
}

long dxirc::onStatusTimeout(FXObject*, FXSelector, void*)
{
    m_statuslabel->setText(" ");
#ifdef HAVE_TRAY
    if(m_useTray)
    {
        m_trayIcon->setText("dxirc");
    }
#endif //HAVE_TRAY
    return 1;
}

void dxirc::appendIrcText(FXString text, FXbool logLine)
{
    if(m_tabbook->numChildren())
    {
        FXint index = m_tabbook->getCurrent()*2;
        if(m_tabbook->childAtIndex(index)->getMetaClass()==&TetrisTabItem::metaClass)
        {
            for(FXint i=0; i<m_tabbook->numChildren(); i+=2)
            {
                if(m_tabbook->childAtIndex(i)->getMetaClass()!=&TetrisTabItem::metaClass)
                {
                    index = i;
                    break;
                }
            }
        }
        dxTabItem *currenttab = static_cast<dxTabItem*>(m_tabbook->childAtIndex(index));
        FXASSERT(currenttab != 0);
        currenttab->appendText(text, TRUE, logLine);
        currenttab->makeLastRowVisible();
    }
}

void dxirc::appendIrcStyledText(FXString text, FXint style, FXbool logLine)
{
    if(m_tabbook->numChildren())
    {
        FXint index = m_tabbook->getCurrent()*2;
        if(m_tabbook->childAtIndex(index)->getMetaClass()==&TetrisTabItem::metaClass)
        {
            for(FXint i=0; i<m_tabbook->numChildren(); i+=2)
            {
                if(m_tabbook->childAtIndex(i)->getMetaClass()!=&TetrisTabItem::metaClass)
                {
                    index = i;
                    break;
                }
            }
        }
        dxTabItem *currenttab = static_cast<dxTabItem*>(m_tabbook->childAtIndex(index));
        FXASSERT(currenttab != 0);
        currenttab->appendStyledText(text, style, TRUE, FALSE, logLine);
        currenttab->makeLastRowVisible();
    }
}

FXint dxirc::loadLuaScript(FXString path, FXbool showMessage)
{
#ifdef HAVE_LUA
    if(m_scripts.no())
    {
       for(FXint i=0; i<m_scripts.no(); i++)
        {
            if(comparecase(path, m_scripts[i].path)==0)
            {
                appendIrcStyledText(FXStringFormat(_("Script %s is already loaded"), path.text()), 4, FALSE);
                return 0;
            }
        }
    }
    if(hasLuaAll(path))
    {
        if(showMessage)
        {
            if(dxEXMessageBox::question(this, MBOX_YES_NO, _("Question"), _("Script %s contains dxirc.AddAll\nThis can BREAK dxirc funcionality.\nLoad it anyway?"), path.text()) == 2) return 0;
        }
        else appendIrcStyledText(FXStringFormat(_("Script %s contains dxirc.AddAll. This can BREAK dxirc funcionality."), path.text()), 4, FALSE);
    }
    lua_State *L = luaL_newstate();
    if(L == NULL)
    {
        appendIrcStyledText(_("Unable to initialize Lua."), 4, FALSE);
        return 0;
    }
    if(L)
    {
        luaL_openlibs(L);
        luaL_register(L, "dxirc", dxircFunctions);
        if(luaL_dofile(L, path.text()))
        {
            appendIrcStyledText(FXStringFormat(_("Unable to load/run the file %s"), lua_tostring(L, -1)), 4, FALSE);
            return 0;
        }
        lua_pushstring(L, "dxirc_Register");
        lua_gettable(L, LUA_GLOBALSINDEX);
        if(lua_pcall(L, 0, 3, 0))
        {
            appendIrcStyledText(FXStringFormat(_("Lua plugin: error registering script %s"), lua_tostring(L, -1)), 4, FALSE);
            return 0;
        }
        FXString name = lua_tostring(L, -3);
        FXString version = lua_tostring(L, -2);
        FXString description = lua_tostring(L, -1);
        lua_pop(L, 4);
        if(m_scripts.no())
        {
           for(FXint i=0; i<m_scripts.no(); i++)
            {
                if(comparecase(name, m_scripts[i].name)==0)
                {
                    appendIrcStyledText(FXStringFormat(_("Script with name %s is already loaded"), name.lower().text()), 4, FALSE);
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
        m_scripts.append(script);
        appendIrcStyledText(FXStringFormat(_("Script %s was loaded"), path.text()), 3, FALSE);
        appendIrcStyledText(FXStringFormat("%s: %s", script.name.text(), script.description.text()), 3, FALSE);
        lua_pushstring(L, "dxirc_Init");
        lua_gettable(L, LUA_GLOBALSINDEX);
        if (lua_type(L, -1) != LUA_TFUNCTION) lua_pop(L, 1);
        else
        {
            if (lua_pcall(L, 0, 0, 0))
            {
                appendIrcStyledText(FXStringFormat(_("Lua plugin: error calling dxirc_Init() %s"), lua_tostring(L, -1)), 4, FALSE);
                lua_pop(L, 1);
            }
        }        
        return 1;
    }
    return 1;
#else
    return 0;
#endif //HAVE_LUA
}

FXint dxirc::unloadLuaScript(FXString name)
{
#ifdef HAVE_LUA
    FXbool success = FALSE;
    if(!m_scripts.no())
    {
        appendIrcStyledText(FXStringFormat(_("Script %s isn't loaded"), name.text()), 4, FALSE);
        return 0;
    }
    else
    {
        for(FXint i=m_scripts.no()-1; i>-1; i--)
        {
            if(comparecase(name, m_scripts[i].name)==0)
            {
                utils::instance().removeScriptCommands(m_scripts[i].name);
                lua_close(m_scripts[i].L);
                m_scripts.erase(i);
                success = TRUE;
            }
        }
    }
    for(FXint i=m_scriptEvents.no()-1; i>-1; i--)
    {
        if(comparecase(name, m_scriptEvents[i].script)==0)
        {
            m_scriptEvents.erase(i);
            success = TRUE;
        }
    }
    if(success) appendIrcStyledText(FXStringFormat(_("Script %s was unloaded"), name.text()), 4, FALSE);
    else appendIrcStyledText(FXStringFormat(_("Script %s isn't loaded"), name.text()), 4, FALSE);
    return 1;
#else
    return 0;
#endif //HAVE_LUA
}

FXbool dxirc::hasLuaAll(const FXString &file)
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

//check for mymsg in loaded script
FXbool dxirc::hasMyMsg()
{
    if(!m_scripts.no() || !m_scriptEvents.no())
    {
        return FALSE;
    }
    for(FXint i=0; i<m_scriptEvents.no(); i++)
    {
        if(comparecase("mymsg", m_scriptEvents[i].name) == 0)
        {
            return TRUE;
        }
    }
    return FALSE;
}

//check for all in loaded script
FXbool dxirc::hasAllCommand()
{
    if(!m_scripts.no() || !m_scriptEvents.no())
    {
        return FALSE;
    }
    for(FXint i=0; i<m_scriptEvents.no(); i++)
    {
        if(comparecase("all", m_scriptEvents[i].name) == 0)
        {
            return TRUE;
        }
    }
    return FALSE;
}

/*return unique filename
usefull mainly for autoDccFile */
FXString dxirc::getUniqueName(const FXString &path, const FXString &name, const FXString &extension)
{
    if(extension.empty())
    {
        if(!FXStat::exists(path+PATHSEPSTRING+name)) return path+PATHSEPSTRING+name;
        else return path+PATHSEPSTRING+name+"-"+FXSystem::time("%Y%m%d%H%M%S", FXSystem::now());
    }
    else
    {
        if(!FXStat::exists(path+PATHSEPSTRING+name+"."+extension)) return path+PATHSEPSTRING+name+"."+extension;
        else return path+PATHSEPSTRING+name+"-"+FXSystem::time("%Y%m%d%H%M%S", FXSystem::now())+"."+extension;
    }
}

//check if exist .part file in dccpath
FXbool dxirc::isForResume(const FXString& name)
{
    return FXStat::exists(m_dccPath+PATHSEPSTRING+name+".part");
}

int dxirc::onLuaAddCommand(lua_State *lua)
{
#ifdef HAVE_LUA
    FXString name, funcname, helptext, script;
    if(lua_isstring(lua, 1)) name = lua_tostring(lua,1);
    if(lua_isstring(lua, 2)) funcname = lua_tostring(lua,2);
    if(lua_isstring(lua, 3)) helptext = lua_tostring(lua,3);
    if(name.empty() || funcname.empty() || helptext.empty()) return 0;
    if(utils::instance().isCommand(name))
    {
        _pThis->appendIrcStyledText(FXStringFormat(_("Command %s already exists"), name.text()), 4, FALSE);
        return 0;
    }
    if(_pThis->m_scripts.no())
    {
        for(FXint i=0; i<_pThis->m_scripts.no(); i++)
        {
            if(lua == _pThis->m_scripts[i].L) script = _pThis->m_scripts[i].name;
        }
    }
    if(script.empty()) return 0;
    LuaScriptCommand command;
    command.name = name;
    command.funcname = funcname;
    command.helptext = helptext;
    command.script = script;
    utils::instance().addScriptCommand(command);
    return  1;
#else
    return 0;
#endif //HAVE_LUA
}

int dxirc::onLuaAddEvent(lua_State *lua)
{
#ifdef HAVE_LUA
    FXString name, funcname, script;
    if(lua_isstring(lua, 1)) name = lua_tostring(lua,1);
    if(lua_isstring(lua, 2)) funcname = lua_tostring(lua,2);
    if(name.empty() || funcname.empty()) return 0;
    if(_pThis->m_scripts.no())
    {
        for(FXint i=0; i<_pThis->m_scripts.no(); i++)
        {
            if(lua == _pThis->m_scripts[i].L) script = _pThis->m_scripts[i].name;
        }
    }
    if(script.empty()) return 0;
    if(_pThis->m_scriptEvents.no())
    {
        for(FXint i=0; i<_pThis->m_scriptEvents.no(); i++)
        {
            if(comparecase(name, _pThis->m_scriptEvents[i].name)==0 && comparecase(funcname, _pThis->m_scriptEvents[i].funcname)==0 && comparecase(script, _pThis->m_scriptEvents[i].script)==0)
            {
                _pThis->appendIrcStyledText(FXStringFormat(_("Function %s for event %s already exists"), funcname.text(), name.text()), 4, FALSE);
                return 0;
            }
        }
    }
    LuaScriptEvent event;
    event.name = name;
    event.funcname = funcname;
    event.script = script;
    _pThis->m_scriptEvents.append(event);
    return  1;
#else
    return 0;
#endif //HAVE_LUA
}

int dxirc::onLuaAddMyMsg(lua_State *lua)
{
#ifdef HAVE_LUA
    FXString funcname, script;
    if(lua_isstring(lua, 1)) funcname = lua_tostring(lua,1);
    if(funcname.empty()) return 0;
    if(_pThis->m_scripts.no())
    {
        for(FXint i=0; i<_pThis->m_scripts.no(); i++)
        {
            if(lua == _pThis->m_scripts[i].L) script = _pThis->m_scripts[i].name;
        }
    }
    if(script.empty()) return 0;
    LuaScriptEvent event;
    event.name = "mymsg";
    event.funcname = funcname;
    event.script = script;
    _pThis->m_scriptEvents.append(event);
    return  1;
#else
    return 0;
#endif //HAVE_LUA
}

int dxirc::onLuaAddNewTab(lua_State *lua)
{
#ifdef HAVE_LUA
    FXString funcname, script;
    if(lua_isstring(lua, 1)) funcname = lua_tostring(lua,1);
    if(funcname.empty()) return 0;
    if(_pThis->m_scripts.no())
    {
        for(FXint i=0; i<_pThis->m_scripts.no(); i++)
        {
            if(lua == _pThis->m_scripts[i].L) script = _pThis->m_scripts[i].name;
        }
    }
    if(script.empty()) return 0;
    LuaScriptEvent event;
    event.name = "newtab";
    event.funcname = funcname;
    event.script = script;
    _pThis->m_scriptEvents.append(event);
    return  1;
#else
    return 0;
#endif //HAVE_LUA
}

int dxirc::onLuaAddDxircQuit(lua_State *lua)
{
#ifdef HAVE_LUA
    FXString funcname, script;
    if(lua_isstring(lua, 1)) funcname = lua_tostring(lua,1);
    if(funcname.empty()) return 0;
    if(_pThis->m_scripts.no())
    {
        for(FXint i=0; i<_pThis->m_scripts.no(); i++)
        {
            if(lua == _pThis->m_scripts[i].L) script = _pThis->m_scripts[i].name;
        }
    }
    if(script.empty()) return 0;
    LuaScriptEvent event;
    event.name = "quit";
    event.funcname = funcname;
    event.script = script;
    _pThis->m_scriptEvents.append(event);
    return  1;
#else
    return 0;
#endif //HAVE_LUA
}

int dxirc::onLuaAddAll(lua_State *lua)
{
#ifdef HAVE_LUA
    FXString funcname, script;
    if(lua_isstring(lua, 1)) funcname = lua_tostring(lua,1);
    if(funcname.empty()) return 0;
    if(_pThis->m_scripts.no())
    {
        for(FXint i=0; i<_pThis->m_scripts.no(); i++)
        {
            if(lua == _pThis->m_scripts[i].L) script = _pThis->m_scripts[i].name;
        }
    }
    if(script.empty()) return 0;
    LuaScriptEvent event;
    event.name = "all";
    event.funcname = funcname;
    event.script = script;
    _pThis->m_scriptEvents.append(event);
    return  1;
#else
    return 0;
#endif //HAVE_LUA
}

int dxirc::onLuaRemoveName(lua_State *lua)
{
#ifdef HAVE_LUA
    FXString command, script;
    if(lua_isstring(lua, 1)) command = lua_tostring(lua, 1);
    if(command.empty()) return 0;
    if(utils::instance().removeScriptCommand(command)) return 1;
    if(_pThis->m_scripts.no())
    {
        for(FXint i=0; i<_pThis->m_scripts.no(); i++)
        {
            if(lua == _pThis->m_scripts[i].L) script = _pThis->m_scripts[i].name;
        }
    }
    if(script.empty()) return 0;
    if(_pThis->m_scriptEvents.no())
    {
        for(FXint i=0; i<_pThis->m_scriptEvents.no(); i++)
        {
            if(comparecase(command, _pThis->m_scriptEvents[i].name) == 0 && comparecase(script, _pThis->m_scriptEvents[i].script) == 0)
            {
                _pThis->m_scriptEvents.erase(i);
                _pThis->appendIrcStyledText(FXStringFormat(_("Command/event %s in script %s was removed"), command.text(), script.text()), 3, FALSE);
            }
        }
    }
    return  1;
#else
    return 0;
#endif //HAVE_LUA
}

int dxirc::onLuaCommand(lua_State *lua)
{
#ifdef HAVE_LUA
    FXString command;
    if(lua_isstring(lua, 1)) command = lua_tostring(lua, 1);
    if(command.empty()) return 0;
    FXint id;
    if(lua_isnumber(lua, 2) && _pThis->isValidTabId(lua_tointeger(lua, 2))) id = lua_tointeger(lua, 2);
    else id = _pThis->getCurrentTabId();
    if(_pThis->m_tabbook->numChildren())
    {
        for(FXint i = 0; i<_pThis->m_tabbook->numChildren(); i+=2)
        {
            if(_pThis->m_tabbook->childAtIndex(i)->getMetaClass()!=&TetrisTabItem::metaClass)
            {
                dxTabItem *tab = static_cast<dxTabItem*>(_pThis->m_tabbook->childAtIndex(i));
                if(id==tab->getID())
                {
                    tab->processLine(command);
                    return 1;
                }
            }
        }
    }
    return  1;
#else
    return 0;
#endif //HAVE_LUA
}

int dxirc::onLuaPrint(lua_State *lua)
{
#ifdef HAVE_LUA
    FXString text;
    if(lua_isstring(lua, 1)) text = lua_tostring(lua, 1);
    if(text.empty()) return 0;
    FXint id, style;
    if(lua_isnumber(lua, 2) && _pThis->isIddxTabItem(lua_tointeger(lua, 2))) id = lua_tointeger(lua, 2);
    else id = _pThis->getCurrentTabId();
    if(lua_isnumber(lua, 3))
    {
        style = lua_tointeger(lua, 3);
        if(style<0 || style>8) style = 0;
    }
    else style = 0;
    if(_pThis->m_tabbook->numChildren())
    {
        for(FXint i = 0; i<_pThis->m_tabbook->numChildren(); i+=2)
        {
            if(_pThis->m_tabbook->childAtIndex(i)->getMetaClass()!=&TetrisTabItem::metaClass)
            {
                dxTabItem *tab = static_cast<dxTabItem*>(_pThis->m_tabbook->childAtIndex(i));
                if(id==tab->getID())
                {
                    tab->appendStyledText(text, style, TRUE, TRUE, TRUE);
                    tab->makeLastRowVisible();
                    return 1;
                }
            }
        }
    }
    return  1;
#else
    return 0;
#endif //HAVE_LUA
}

int dxirc::onLuaGetServers(lua_State *lua)
{
#ifdef HAVE_LUA
    if(_pThis->m_ircengines.no())
    {
        lua_newtable(lua);
        for(FXint i=0; i<_pThis->m_ircengines.no(); i++)
        {
            lua_pushnumber(lua, i+1);
            lua_newtable(lua);
            lua_pushstring(lua, "server");
            lua_pushstring(lua, _pThis->m_ircengines[i]->getServerName().text());
            lua_settable(lua, -3);
            lua_pushstring(lua, "port");
            lua_pushnumber(lua, _pThis->m_ircengines[i]->getServerPort());
            lua_settable(lua, -3);
            lua_pushstring(lua, "nick");
            lua_pushstring(lua, _pThis->m_ircengines[i]->getNickName().text());
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
#endif //HAVE_LUA
}

int dxirc::onLuaGetTab(lua_State *lua)
{
#ifdef HAVE_LUA
    FXString tab, server;
    if(lua_isstring(lua, 1)) tab = lua_tostring(lua, 1);
    if(lua_isstring(lua, 2)) server = lua_tostring(lua,2);
    if(_pThis->m_tabbook->numChildren())
    {
        for(FXint i = 0; i<_pThis->m_tabbook->numChildren(); i+=2)
        {
            if(_pThis->m_tabbook->childAtIndex(i)->getMetaClass()!=&TetrisTabItem::metaClass)
            {
                if(comparecase(tab, static_cast<FXTabItem*>(_pThis->m_tabbook->childAtIndex(i))->getText()) == 0 && comparecase(server, static_cast<dxTabItem*>(_pThis->m_tabbook->childAtIndex(i))->getServerName()) == 0)
                {
                    lua_pushnumber(lua, static_cast<dxTabItem*>(_pThis->m_tabbook->childAtIndex(i))->getID());
                    return 1;
                }
            }
        }
        lua_pushnumber(lua, _pThis->getCurrentTabId());
        return 1;
    }
    else lua_pushnumber(lua, -1);
    return  1;
#else
    return 0;
#endif //HAVE_LUA
}

int dxirc::onLuaGetCurrentTab(lua_State *lua)
{
#ifdef HAVE_LUA
    lua_pushnumber(lua, _pThis->getCurrentTabId());
    return 1;
#else
    return 0;
#endif //HAVE_LUA
}

int dxirc::onLuaGetVersion(lua_State *lua)
{
#ifdef HAVE_LUA
    lua_pushstring(lua, VERSION);
    return 1;
#else
    return 0;
#endif //HAVE_LUA
}

int dxirc::onLuaGetTabInfo(lua_State *lua)
{
#ifdef HAVE_LUA
    FXint id;
    if(lua_isnumber(lua, 1))  id = lua_tointeger(lua, 1);
    else id = -1;
    if(_pThis->m_tabbook->numChildren() && _pThis->isIddxTabItem(id))
    {
        dxTabItem *tab = NULL;
        for(FXint i = 0; i<_pThis->m_tabbook->numChildren(); i+=2)
        {
            if(_pThis->m_tabbook->childAtIndex(i)->getMetaClass()!=&TetrisTabItem::metaClass)
            {
                tab = static_cast<dxTabItem*>(_pThis->m_tabbook->childAtIndex(i));
                if(id==tab->getID())
                {
                    break;
                }
            }
        }
        lua_newtable(lua);
        lua_pushstring(lua, "name");
        lua_pushstring(lua, tab->getText().text());
        lua_settable(lua, -3);
        switch(tab->getType()) {
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
            case OTHER:
            {
                lua_pushstring(lua, "type");
                lua_pushstring(lua, "other");
                lua_settable(lua, -3);
            }break;
        }
        if(tab->getMetaClass()==&DccTabItem::metaClass)
        {
            lua_pushstring(lua, "type");
            lua_pushstring(lua, "dccchat");
            lua_settable(lua, -3);
        }
        lua_pushstring(lua, "servername");
        lua_pushstring(lua, tab->getServerName().text());
        lua_settable(lua, -3);
        lua_pushstring(lua, "port");
        lua_pushinteger(lua, tab->getServerPort());
        lua_settable(lua, -3);
        lua_pushstring(lua, "nick");
        lua_pushstring(lua, tab->getNickName().text());
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
#endif //HAVE_LUA
}

int dxirc::onLuaSetTab(lua_State *lua)
{
#ifdef HAVE_LUA
    _pThis->setCurrentTabById(lua_tointeger(lua, 1));
    return  1;
#else
    return 0;
#endif //HAVE_LUA
}

int dxirc::onLuaCreateTab(lua_State *lua)
{
#ifdef HAVE_LUA
    FXString name;
    if(lua_isstring(lua, 1)) name = lua_tostring(lua, 1);
    else
    {
        lua_pushnil(lua);
        return 0;
    }
    if(_pThis->m_tabbook->numChildren())
    {
        for(FXint i = 0; i < _pThis->m_tabbook->numChildren(); i+=2)
        {
            if(_pThis->m_tabbook->childAtIndex(i)->getMetaClass()==&IrcTabItem::metaClass
                    && comparecase(static_cast<FXTabItem*>(_pThis->m_tabbook->childAtIndex(i))->getText(), name) == 0
                    && static_cast<IrcTabItem*>(_pThis->m_tabbook->childAtIndex(i))->getType() == OTHER)
            {
                lua_pushnil(lua);
                return 0;
            }
        }
    }
    lua_pushnumber(lua, _pThis->createIrcTab(name, NULL, OTHER, NULL));
    return 1;
#else
    return 0;
#endif //HAVE_LUA
}

int dxirc::onLuaGetTabCount(lua_State *lua)
{
#ifdef HAVE_LUA
    lua_pushnumber(lua, _pThis->m_tabbook->numChildren()/2);
    return 1;
#else
    return 0;
#endif //HAVE_LUA
}

int dxirc::onLuaClear(lua_State *lua)
{
#ifdef HAVE_LUA
    FXint id = -1;
    if(lua_isnumber(lua, 1) && _pThis->isIddxTabItem(lua_tointeger(lua, 1)))  id = lua_tointeger(lua, 1);
    else return 0;
    utils::instance().debugLine(FXStringFormat("onLuaClear id:%d",id));
    if(_pThis->m_tabbook->numChildren())
    {
        for(FXint i = 0; i < _pThis->m_tabbook->numChildren(); i+=2)
        {
            if(_pThis->m_tabbook->childAtIndex(i)->getMetaClass()!=&TetrisTabItem::metaClass
                    && static_cast<dxTabItem*>(_pThis->m_tabbook->childAtIndex(i))->getID() == id)
            {
                static_cast<dxTabItem*>(_pThis->m_tabbook->childAtIndex(i))->clearChat();
                return 1;
            }
        }
    }
    return  1;
#else
    return 0;
#endif //HAVE_LUA
}

//show notify
void dxirc::showNotify(const FXString& notify, FXint pos)
{
#ifdef HAVE_LIBNOTIFY
    NotifyNotification *notification = NULL;
    notify_init("Basics");
    FXString datadir;
    datadir.format("%s%sicons%sbig_dxirc.png",DXIRC_DATADIR,PATHSEPSTRING,PATHSEPSTRING);
    notification = notify_notification_new(PACKAGE, notify.text(), datadir.text());
    if(!notify_notification_show(notification, NULL))
        return;
    g_object_unref(G_OBJECT(notification));
#else
    m_wnotify->setText(notify);
    m_wnotify->notify(pos);
#endif //HAVE_LIBNOTIFY
}


#define USAGE_MSG _("\
\nUsage: dxirc [options] \n\
\n\
    [options] can be any of the following:\n\
\n\
        -h, --help         Print (this) help screen and exit.\n\
        -v, --version      Print version information and exit.\n\
        -l [FILE]          Load configuration from FILE.\n\
        -i [PATH]          Use PATH for icons.\n\
\n")

int main(int argc,char *argv[])
{
#ifndef WIN32
    signal(SIGPIPE, SIG_IGN);
#endif //WIN32
    FXbool loadIcon;
    FXString datadir = DXIRC_DATADIR;

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
            utils::instance().setIniFile(argv[i+1]);
        }
        if(compare(argv[i],"-i")==0)
        {
            if(FXStat::exists(argv[i+1])) datadir = argv[i+1];
        }
    }
    utils::instance().setLangs();

    FXbool oneinstance = utils::instance().getBoolIniEntry("SETTINGS", "oneinstance", FALSE);
    if(oneinstance && IsInstanceRunningAlready())
        return 0;

#ifdef HAVE_TRAY
    FXTrayApp app(PACKAGE, FXString::null);
#else
    FXApp app(PACKAGE, FXString::null);
#endif //HAVE_TRAY
    app.reg().setAsciiMode(TRUE);
    app.init(argc,argv);
#ifdef ENABLE_NLS
    app.setTranslator(new dxTranslator(&app));
#endif //ENABLE_NLS
    loadIcon = makeAllIcons(&app, utils::instance().getIniFile(), datadir);
    new dxirc(&app);
    app.create();
    utils::instance().setAlias();
    return app.run();
}
