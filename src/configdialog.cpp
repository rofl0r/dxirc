/*
 *      configdialog.cpp
 *
 *      Copyright 2008 David Vachulka <david@konstrukce-cad.com>
 *      Copyright (C) 2004-2005 Sander Jansen (some code)
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

#include "configdialog.h"
#include "config.h"
#include "i18n.h"
#include "icons.h"

FXDEFMAP(ConfigDialog) ConfigDialogMap[] = {
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_ACCEPT, ConfigDialog::OnAccept),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_CANCEL, ConfigDialog::OnCancel),
    FXMAPFUNC(SEL_SELECTED, ConfigDialog::ID_COMMAND, ConfigDialog::OnCommandsSelected),
    FXMAPFUNC(SEL_DESELECTED, ConfigDialog::ID_COMMAND, ConfigDialog::OnCommandsDeselected),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_ADDCOMMAND, ConfigDialog::OnAddCommand),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_DELETECOMMAND, ConfigDialog::OnDeleteCommand),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_ADDUSER, ConfigDialog::OnAddUser),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_MODIFYUSER, ConfigDialog::OnModifyUser),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_DELETEUSER, ConfigDialog::OnDeleteUser),
    FXMAPFUNC(SEL_CHANGED, ConfigDialog::ID_ICONS, ConfigDialog::OnIconsChanged),
    FXMAPFUNC(SEL_SELECTED, ConfigDialog::ID_ICONS, ConfigDialog::OnIconsChanged),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_ADDICONS, ConfigDialog::OnAddIcons),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_DELETEICONS, ConfigDialog::OnDeleteIcons),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_TRAY, ConfigDialog::OnTray),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_LOG, ConfigDialog::OnLogChanged),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_LOGPATH, ConfigDialog::OnPathSelect),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_SERVERWINDOW, ConfigDialog::OnServerWindow),
    FXMAPFUNC(SEL_CHANGED, ConfigDialog::ID_NICK, ConfigDialog::OnNickCharChanged),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_RECONNECT, ConfigDialog::OnReconnect),
    FXMAPFUNC(SEL_KEYPRESS, 0, ConfigDialog::OnKeyPress),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_IRCCOLORS, ConfigDialog::OnColor),
    FXMAPFUNC(SEL_CHANGED, ConfigDialog::ID_IRCCOLORS, ConfigDialog::OnColor),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_COLORS, ConfigDialog::OnThemeColorChanged),
    FXMAPFUNC(SEL_CHANGED, ConfigDialog::ID_COLORS, ConfigDialog::OnThemeColorChanged),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_THEME, ConfigDialog::OnTheme),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_FONT, ConfigDialog::OnFont),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_IRCFONT, ConfigDialog::OnIrcFont),
    FXMAPFUNCS(SEL_SELECTED, ConfigDialog::ID_USER, ConfigDialog::ID_SERVER, ConfigDialog::OnUsersSelected),
    FXMAPFUNCS(SEL_DESELECTED, ConfigDialog::ID_USER, ConfigDialog::ID_SERVER, ConfigDialog::OnUsersDeselected),
    FXMAPFUNCS(SEL_CHANGED, ConfigDialog::ID_USER, ConfigDialog::ID_SERVER, ConfigDialog::OnUsersChanged),
};

FXIMPLEMENT(ConfigDialog, FXDialogBox, ConfigDialogMap, ARRAYNUMBER(ConfigDialogMap))

ConfigDialog::ConfigDialog(FXMainWindow *owner, IrcColor clrs, FXString clist, dxIgnoreUserArray ulist, FXString tpth, FXString thm, FXint maxa, FXbool log, FXString lpth, FXbool srvw, FXString nichar, FXString fnt, FXbool scmd, FXbool slst, ColorTheme atheme, FXbool utray, FXbool cnick, FXbool ctt, FXbool rcn, FXint na, FXint da)
    : FXDialogBox(owner, _("Preferences"), DECOR_RESIZE|DECOR_TITLE|DECOR_BORDER, 0,0,0,0, 0,0,0,0, 0,0),
        commandsList(clist), themePath(tpth), themesList(thm), logPath(lpth), logging(log), serverWindow(srvw), sameCmd(scmd), sameList(slst), useTray(utray), coloredNick(cnick), closeToTray(ctt), reconnect(rcn), usersList(ulist), colors(clrs), maxAway(maxa), numberAttempt(na), delayAttempt(da), nickChar(nichar), themeCurrent(atheme)
{
    textTarget.connect(colors.text);
    textTarget.setTarget(this);
    textTarget.setSelector(ID_IRCCOLORS);
    backTarget.connect(colors.back);
    backTarget.setTarget(this);
    backTarget.setSelector(ID_IRCCOLORS);
    userTarget.connect(colors.user);
    userTarget.setTarget(this);
    userTarget.setSelector(ID_IRCCOLORS);
    actionTarget.connect(colors.action);
    actionTarget.setTarget(this);
    actionTarget.setSelector(ID_IRCCOLORS);
    noticeTarget.connect(colors.notice);
    noticeTarget.setTarget(this);
    noticeTarget.setSelector(ID_IRCCOLORS);
    errorTarget.connect(colors.error);
    errorTarget.setTarget(this);
    errorTarget.setSelector(ID_IRCCOLORS);
    hilightTarget.connect(colors.hilight);
    hilightTarget.setTarget(this);
    hilightTarget.setSelector(ID_IRCCOLORS);
    linkTarget.connect(colors.link);
    linkTarget.setTarget(this);
    linkTarget.setSelector(ID_IRCCOLORS);

    targetSameCmd.connect(sameCmd);
    targetSameList.connect(sameList);
    targetColoredNick.connect(coloredNick);
    targetCloseToTray.connect(closeToTray);

    targetBack.connect(themeCurrent.back);
    targetBack.setTarget(this);
    targetBack.setSelector(ID_COLORS);
    targetBase.connect(themeCurrent.base);
    targetBase.setTarget(this);
    targetBase.setSelector(ID_COLORS);
    targetBorder.connect(themeCurrent.border);
    targetBorder.setTarget(this);
    targetBorder.setSelector(ID_COLORS);
    targetFore.connect(themeCurrent.fore);
    targetFore.setTarget(this);
    targetFore.setSelector(ID_COLORS);
    targetMenuback.connect(themeCurrent.menuback);
    targetMenuback.setTarget(this);
    targetMenuback.setSelector(ID_COLORS);
    targetMenufore.connect(themeCurrent.menufore);
    targetMenufore.setTarget(this);
    targetMenufore.setSelector(ID_COLORS);
    targetSelback.connect(themeCurrent.selback);
    targetSelback.setTarget(this);
    targetSelback.setSelector(ID_COLORS);
    targetSelfore.connect(themeCurrent.selfore);
    targetSelfore.setTarget(this);
    targetSelfore.setSelector(ID_COLORS);
    targetTipback.connect(themeCurrent.tipback);
    targetTipback.setTarget(this);
    targetTipback.setSelector(ID_COLORS);
    targetTipfore.connect(themeCurrent.tipfore);
    targetTipfore.setTarget(this);
    targetTipfore.setSelector(ID_COLORS);

    trayTarget.connect(useTray);
    trayTarget.setTarget(this);
    trayTarget.setSelector(ID_TRAY);
    serverTarget.connect(serverWindow);
    serverTarget.setTarget(this);
    serverTarget.setSelector(ID_SERVERWINDOW);
    logTarget.connect(logging);
    logTarget.setTarget(this);
    logTarget.setSelector(ID_LOG);

    targetReconnect.connect(reconnect);
    targetReconnect.setTarget(this);
    targetReconnect.setSelector(ID_RECONNECT);
    targetNumberAttempt.connect(numberAttempt);
    targetDelayAttempt.connect(delayAttempt);
    targetMaxAway.connect(maxAway);

    getApp()->getNormalFont()->create();
    FXFontDesc fontdescription;
    getApp()->getNormalFont()->getFontDesc(fontdescription);
    font = new FXFont(getApp(),fontdescription);
    font->create();

    ircFont = new FXFont(getApp(), fnt);
    ircFont->create();

    FXHorizontalFrame *closeframe = new FXHorizontalFrame(this, LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X);    
    FXButton *ok = new FXButton(closeframe, _("&Close"), NULL, this, ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|LAYOUT_RIGHT|FRAME_RAISED|FRAME_THICK, 0,0,0,0, 20,20);
    ok->addHotKey(KEY_Return);
    new FXButton(closeframe, _("Cancel"), NULL, this, ID_CANCEL, LAYOUT_RIGHT|FRAME_RAISED|FRAME_THICK, 0,0,0,0, 20,20);

    FXHorizontalFrame *contents = new FXHorizontalFrame(this, LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    FXVerticalFrame *buttonframe = new FXVerticalFrame(contents, LAYOUT_FILL_Y|LAYOUT_LEFT|PACK_UNIFORM_WIDTH);
    FXSwitcher *switcher = new FXSwitcher(contents, LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_RIGHT);

    FXVerticalFrame *colorpane = new FXVerticalFrame(switcher, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(colorpane, _("Irc text settings"), NULL, LAYOUT_LEFT);
    new FXHorizontalSeparator(colorpane, SEPARATOR_LINE|LAYOUT_FILL_X);
    FXHorizontalFrame *hframe = new FXHorizontalFrame(colorpane, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    FXVerticalFrame *cframe = new FXVerticalFrame(hframe, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    FXMatrix *colormatrix = new FXMatrix(cframe, 2, MATRIX_BY_COLUMNS, 0,0,0,0, DEFAULT_SPACING,DEFAULT_SPACING,DEFAULT_SPACING,DEFAULT_SPACING, 1,1);
    new FXColorWell(colormatrix, FXRGB(0,0,255), &backTarget, FXDataTarget::ID_VALUE, COLORWELL_OPAQUEONLY|FRAME_SUNKEN|FRAME_THICK|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW, 0,0,40,24);
    new FXLabel(colormatrix, _("Text backround color"), NULL, JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
    new FXColorWell(colormatrix, FXRGB(0,0,255), &textTarget, FXDataTarget::ID_VALUE, COLORWELL_OPAQUEONLY|FRAME_SUNKEN|FRAME_THICK|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW, 0,0,40,24);
    new FXLabel(colormatrix, _("Text color"), NULL, JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
    new FXColorWell(colormatrix, FXRGB(0,0,255), &userTarget, FXDataTarget::ID_VALUE, COLORWELL_OPAQUEONLY|FRAME_SUNKEN|FRAME_THICK|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW, 0,0,40,24);
    new FXLabel(colormatrix, _("User events text color"), NULL, JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
    new FXColorWell(colormatrix, FXRGB(0,0,255), &actionTarget, FXDataTarget::ID_VALUE, COLORWELL_OPAQUEONLY|FRAME_SUNKEN|FRAME_THICK|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW, 0,0,40,24);
    new FXLabel(colormatrix, _("Actions message text color"), NULL, JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
    new FXColorWell(colormatrix, FXRGB(0,0,255), &noticeTarget, FXDataTarget::ID_VALUE, COLORWELL_OPAQUEONLY|FRAME_SUNKEN|FRAME_THICK|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW, 0,0,40,24);
    new FXLabel(colormatrix, _("Notice text color"), NULL, JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
    new FXColorWell(colormatrix, FXRGB(0,0,255), &errorTarget, FXDataTarget::ID_VALUE, COLORWELL_OPAQUEONLY|FRAME_SUNKEN|FRAME_THICK|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW, 0,0,40,24);
    new FXLabel(colormatrix, _("Error text color"), NULL, JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
    new FXColorWell(colormatrix, FXRGB(0,0,255), &hilightTarget, FXDataTarget::ID_VALUE, COLORWELL_OPAQUEONLY|FRAME_SUNKEN|FRAME_THICK|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW, 0,0,40,24);
    new FXLabel(colormatrix, _("Highlight message text color"), NULL, JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
    new FXColorWell(colormatrix, FXRGB(0,0,255), &linkTarget, FXDataTarget::ID_VALUE, COLORWELL_OPAQUEONLY|FRAME_SUNKEN|FRAME_THICK|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW, 0,0,40,24);
    new FXLabel(colormatrix, _("Link color"), NULL, JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
    new FXLabel(colormatrix, _("Font"));
    ircfontButton = new FXButton(colormatrix, " ", NULL, this, ID_IRCFONT, LAYOUT_CENTER_Y|FRAME_RAISED|JUSTIFY_CENTER_X|JUSTIFY_CENTER_Y|LAYOUT_FILL_X);
    new FXCheckButton(cframe, _("Use same font for commandline"), &targetSameCmd, FXDataTarget::ID_VALUE, CHECKBUTTON_NORMAL|LAYOUT_FILL_X|LAYOUT_SIDE_LEFT|JUSTIFY_LEFT);
    new FXCheckButton(cframe, _("Use same font for user list"), &targetSameList, FXDataTarget::ID_VALUE, CHECKBUTTON_NORMAL|LAYOUT_FILL_X|LAYOUT_SIDE_LEFT|JUSTIFY_LEFT);
    new FXCheckButton(cframe, _("Use colored nick"), &targetColoredNick, FXDataTarget::ID_VALUE, CHECKBUTTON_NORMAL|LAYOUT_FILL_X|LAYOUT_SIDE_LEFT|JUSTIFY_LEFT);
    FXVerticalFrame *tframe = new FXVerticalFrame(hframe, FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    text = new FXText(tframe, NULL, 0, LAYOUT_FILL_X|LAYOUT_FILL_Y|TEXT_READONLY);
    text->setScrollStyle(HSCROLLING_OFF);

    FXVerticalFrame *ignorepane = new FXVerticalFrame(switcher, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(ignorepane, _("Ignore commands and users"), NULL, LAYOUT_LEFT);
    new FXHorizontalSeparator(ignorepane, SEPARATOR_LINE|LAYOUT_FILL_X);
    FXGroupBox *commandsgroup = new FXGroupBox(ignorepane, _("Commands"), FRAME_GROOVE|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_SIDE_TOP);
    FXVerticalFrame *commandsbuttons = new FXVerticalFrame(commandsgroup, LAYOUT_SIDE_RIGHT|LAYOUT_FILL_Y|PACK_UNIFORM_WIDTH);
    addCommand = new FXButton(commandsbuttons, _("Add"), NULL, this, ID_ADDCOMMAND, FRAME_RAISED|FRAME_THICK);
    deleteCommand = new FXButton(commandsbuttons, _("Delete"), NULL, this, ID_DELETECOMMAND, FRAME_RAISED|FRAME_THICK);
    FXVerticalFrame *commandslist = new FXVerticalFrame(commandsgroup, LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN|FRAME_THICK);
    commands = new FXList(commandslist, this, ID_COMMAND, LIST_SINGLESELECT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    FXGroupBox *usersgroup = new FXGroupBox(ignorepane, _("Users"), FRAME_GROOVE|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    FXVerticalFrame *usersbuttons = new FXVerticalFrame(usersgroup, LAYOUT_SIDE_RIGHT|LAYOUT_FILL_Y|PACK_UNIFORM_WIDTH);
    addUser = new FXButton(usersbuttons, _("Add"), NULL, this, ID_ADDUSER, FRAME_RAISED|FRAME_THICK);
    modifyUser = new FXButton(usersbuttons, _("Modify"), NULL, this, ID_MODIFYUSER, FRAME_RAISED|FRAME_THICK);
    deleteUser = new FXButton(usersbuttons, _("Delete"), NULL, this, ID_DELETEUSER, FRAME_RAISED|FRAME_THICK);
    FXVerticalFrame *userspane = new FXVerticalFrame(usersgroup, LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN|FRAME_THICK);
    FXHorizontalFrame *usersframe = new FXHorizontalFrame(userspane, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    users = new FXList(usersframe, this, ID_USER, LIST_SINGLESELECT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    users->setScrollStyle(HSCROLLING_OFF);
    channels = new FXList(usersframe, this, ID_CHANNEL, LIST_SINGLESELECT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    channels->setScrollStyle(HSCROLLING_OFF);
    servers = new FXList(usersframe, this, ID_SERVER, LIST_SINGLESELECT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    servers->setScrollStyle(HSCROLLING_OFF);
    FillCommnads();
    FillUsers();
    commands->getNumItems() ? deleteCommand->enable() : deleteCommand->disable();
    if(usersList.no())
    {
        deleteUser->enable();
        modifyUser->enable();
    }
    else
    {
        deleteUser->disable();
        modifyUser->disable();
    }

    FXVerticalFrame *otherpane = new FXVerticalFrame(switcher, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(otherpane, _("General settings"), NULL, LAYOUT_LEFT);
    new FXHorizontalSeparator(otherpane, SEPARATOR_LINE|LAYOUT_FILL_X);
    FXGroupBox *themesgroup = new FXGroupBox(otherpane, _("Nick icons themes"), FRAME_GROOVE|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_SIDE_TOP);
    FXVerticalFrame *themesbuttons = new FXVerticalFrame(themesgroup, LAYOUT_SIDE_RIGHT|LAYOUT_FILL_Y|PACK_UNIFORM_WIDTH);
    addTheme = new FXButton(themesbuttons, _("Add"), NULL, this, ID_ADDICONS, FRAME_RAISED|FRAME_THICK);
    deleteTheme = new FXButton(themesbuttons, _("Delete"), NULL, this, ID_DELETEICONS, FRAME_RAISED|FRAME_THICK);
    FXVerticalFrame *themeslist = new FXVerticalFrame(themesgroup, LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN|FRAME_THICK);
    icons = new FXList(themeslist, this, ID_ICONS, LIST_SINGLESELECT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    iconsBar = new FXToolBar(themeslist, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    icon1 = new FXButton(iconsBar, _("\tAdmin"), NULL, NULL, 0, BUTTON_TOOLBAR);
    icon2 = new FXButton(iconsBar, _("\tOwner"), NULL, NULL, 0, BUTTON_TOOLBAR);
    icon3 = new FXButton(iconsBar, _("\tOp"), NULL, NULL, 0, BUTTON_TOOLBAR);
    icon4 = new FXButton(iconsBar, _("\tHalfop"), NULL, NULL, 0, BUTTON_TOOLBAR);
    icon5 = new FXButton(iconsBar, _("\tVoice"), NULL, NULL, 0, BUTTON_TOOLBAR);
    icon6 = new FXButton(iconsBar, _("\tNormal"), NULL, NULL, 0, BUTTON_TOOLBAR);
    icon7 = new FXButton(iconsBar, _("\tAway"), NULL, NULL, 0, BUTTON_TOOLBAR);
    FillIcons();
    for(FXint i=0; i<icons->getNumItems(); i++)
    {
        if(icons->getItemText(i) == themePath)
        {
            icons->selectItem(i, true);
            icons->setCurrentItem(i);
            break;
        }
    }
    icons->getNumItems()>1 ? deleteTheme->enable() : deleteTheme->disable();
    new FXCheckButton(otherpane, _("Reconnect after disconnection"), &targetReconnect, FXDataTarget::ID_VALUE, CHECKBUTTON_NORMAL|LAYOUT_FILL_X|LAYOUT_SIDE_LEFT|JUSTIFY_LEFT);
    FXHorizontalFrame *napane = new FXHorizontalFrame(otherpane, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    numberAttemptLabel = new FXLabel(napane, _("Number of attempt"), NULL, LAYOUT_LEFT);
    if(!reconnect) numberAttemptLabel->disable();
    numberAttemptSpinner = new FXSpinner(napane, 4, &targetNumberAttempt, FXDataTarget::ID_VALUE, SPIN_CYCLIC|FRAME_GROOVE);
    numberAttemptSpinner->setRange(1,20);
    if(!reconnect) numberAttemptSpinner->disable();
    FXHorizontalFrame *dapane = new FXHorizontalFrame(otherpane, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    delayAttemptLabel = new FXLabel(dapane, _("Delay between attempts [seconds]"), NULL, LAYOUT_LEFT);
    if(!reconnect) delayAttemptLabel->disable();
    delayAttemptSpinner = new FXSpinner(dapane, 4, &targetDelayAttempt, FXDataTarget::ID_VALUE, SPIN_CYCLIC|FRAME_GROOVE);
    delayAttemptSpinner->setRange(20,120);
    if(!reconnect) delayAttemptSpinner->disable();
    FXHorizontalFrame *maxpane = new FXHorizontalFrame(otherpane, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(maxpane, _("Max. users number for checking away\tToo high number can be reason for ban"), NULL, LAYOUT_LEFT);
    new FXSpinner(maxpane, 4, &targetMaxAway, FXDataTarget::ID_VALUE, SPIN_NOMAX|FRAME_GROOVE);
    FXHorizontalFrame *nickpane = new FXHorizontalFrame(otherpane, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(nickpane, _("Nick completion char"), NULL, LAYOUT_LEFT);
    nickCharField = new FXTextField(nickpane, 1, this, ID_NICK, FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_X);
    nickCharField->setText(nickChar);
#ifdef HAVE_TRAY
    new FXCheckButton(otherpane, _("Use trayicon"), &trayTarget, FXDataTarget::ID_VALUE, CHECKBUTTON_NORMAL|LAYOUT_FILL_X|LAYOUT_SIDE_LEFT|JUSTIFY_LEFT);
    closeToTrayButton = new FXCheckButton(otherpane, _("Close button hide application"), &targetCloseToTray, FXDataTarget::ID_VALUE, CHECKBUTTON_NORMAL|LAYOUT_FILL_X|LAYOUT_SIDE_LEFT|JUSTIFY_LEFT);
#endif
    new FXCheckButton(otherpane, _("Special tab for server messages"), &serverTarget, FXDataTarget::ID_VALUE, CHECKBUTTON_NORMAL|LAYOUT_FILL_X|LAYOUT_SIDE_LEFT|JUSTIFY_LEFT);
    new FXCheckButton(otherpane, _("Logging chats"), &logTarget, FXDataTarget::ID_VALUE, CHECKBUTTON_NORMAL|LAYOUT_FILL_X|LAYOUT_SIDE_LEFT|JUSTIFY_LEFT);
    FXHorizontalFrame *logpane = new FXHorizontalFrame(otherpane, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(logpane, _("Log path"), NULL, LAYOUT_LEFT);
    folder = new FXTextField(logpane, 25, NULL, 0, FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_X);
    if(FXStat::exists(logPath)) folder->setText(logPath);
    else folder->setText(FXSystem::getHomeDirectory());
    folder->disable();
    selectPath = new FXButton(logpane, "...", NULL, this, ID_LOGPATH, FRAME_RAISED|FRAME_THICK);
    if(logging) selectPath->enable();
    else selectPath->disable();

    FXVerticalFrame *lookpane = new FXVerticalFrame(switcher, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(lookpane, _("FOX toolkit look for dxirc"), NULL, LAYOUT_LEFT);
    new FXHorizontalSeparator(lookpane, SEPARATOR_LINE|LAYOUT_FILL_X);
    FXHorizontalFrame *hframe1 = new FXHorizontalFrame(lookpane, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    FXVerticalFrame *vframe1 = new FXVerticalFrame(hframe1, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    vframe2 = new FXVerticalFrame(hframe1, LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_THICK|FRAME_RAISED);
    new FXLabel(vframe1, _("Theme:"), NULL, LAYOUT_CENTER_Y);
    themes = new FXListBox(vframe1,this,ID_THEME,LAYOUT_FILL_X|FRAME_SUNKEN|FRAME_THICK);
    themes->setNumVisible(9);
    FillThemes();
    new FXSeparator(vframe1, SEPARATOR_GROOVE|LAYOUT_FILL_X);
    FXMatrix *themeMatrix = new FXMatrix(vframe1, 2, LAYOUT_FILL_Y|MATRIX_BY_COLUMNS, 0,0,0,0, DEFAULT_SPACING,DEFAULT_SPACING,DEFAULT_SPACING,DEFAULT_SPACING, 1,1);
    new FXColorWell(themeMatrix, FXRGB(0,0,255), &targetBase, FXDataTarget::ID_VALUE);
    new FXLabel(themeMatrix, _("Base Color"));
    new FXColorWell(themeMatrix, FXRGB(0,0,255), &targetBorder, FXDataTarget::ID_VALUE);
    new FXLabel(themeMatrix, _("Border Color"));
    new FXColorWell(themeMatrix, FXRGB(0,0,255), &targetFore,FXDataTarget::ID_VALUE);
    new FXLabel(themeMatrix, _("Text Color"));
    new FXColorWell(themeMatrix, FXRGB(0,0,255), &targetBack, FXDataTarget::ID_VALUE);
    new FXLabel(themeMatrix, _("Background Color"));
    new FXColorWell(themeMatrix, FXRGB(0,0,255), &targetSelfore, FXDataTarget::ID_VALUE);
    new FXLabel(themeMatrix, _("Selected Text Color"));
    new FXColorWell(themeMatrix, FXRGB(0,0,255), &targetSelback, FXDataTarget::ID_VALUE);
    new FXLabel(themeMatrix, _("Selected Background Color"));
    new FXColorWell(themeMatrix, FXRGB(0,0,255), &targetMenufore, FXDataTarget::ID_VALUE);
    new FXLabel(themeMatrix, _("Selected Menu Text Color"));
    new FXColorWell(themeMatrix, FXRGB(0,0,255), &targetMenuback, FXDataTarget::ID_VALUE);
    new FXLabel(themeMatrix, _("Selected Menu Background Color"));
    new FXColorWell(themeMatrix, FXRGB(0,0,255), &targetTipfore, FXDataTarget::ID_VALUE);
    new FXLabel(themeMatrix, _("Tip Text Color"));
    new FXColorWell(themeMatrix, FXRGB(0,0,255), &targetTipback, FXDataTarget::ID_VALUE);
    new FXLabel(themeMatrix, _("Tip Background Color"));
    label = new FXLabel(vframe2, "Label");
    textFrame1 = new FXHorizontalFrame(vframe2, LAYOUT_FILL_X);
    textTest = new FXTextField(textFrame1, 30, NULL, 0, LAYOUT_FILL_X|FRAME_THICK|FRAME_SUNKEN);
    textTest->setText(_("Select this text, to see the selected colors"));
    textFrame2 = new FXHorizontalFrame(vframe2, LAYOUT_FILL_X|FRAME_THICK|FRAME_SUNKEN, 0,0,0,0,2,2,2,2,0,0);
    labelSelected = new FXLabel(textFrame2, _("Selected Text (with focus)"), NULL, LAYOUT_FILL_X, 0,0,0,0,1,1,1,1);
    textFrame3 = new FXHorizontalFrame(vframe2, LAYOUT_FILL_X|FRAME_THICK|FRAME_SUNKEN, 0,0,0,0,2,2,2,2,0,0);
    labelNocurrent = new FXLabel(textFrame3, _("Selected Text (no focus)"), NULL, LAYOUT_FILL_X, 0,0,0,0,1,1,1,1);
    sep1 = new FXSeparator(vframe2, LAYOUT_FILL_X|SEPARATOR_LINE);
    labelTip = new FXLabel(vframe2, _("Tooltip example"), NULL, FRAME_LINE|LAYOUT_CENTER_X);
    menuGroup = new FXGroupBox(vframe2, _("Menu example"), FRAME_GROOVE|LAYOUT_FILL_Y|LAYOUT_FILL_X);
    menuFrame = new FXVerticalFrame(menuGroup, FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X|LAYOUT_CENTER_Y, 0,0,0,0,0,0,0,0,0,0);
    menuLabels[0]=new FXLabel(menuFrame, _("&Server list"), NULL, LABEL_NORMAL, 0,0,0,0,16,4);
    menuLabels[1]=new FXLabel(menuFrame, _("Selected Menu Entry"), NULL, LABEL_NORMAL, 0,0,0,0,16,4);
    sep2 = new FXSeparator(menuFrame, LAYOUT_FILL_X|SEPARATOR_LINE);
    menuLabels[2]=new FXLabel(menuFrame, _("&Quit"), NULL, LABEL_NORMAL, 0,0,0,0,16,4);
    FXHorizontalFrame *fontframe = new FXHorizontalFrame(lookpane, LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, DEFAULT_SPACING,DEFAULT_SPACING,DEFAULT_SPACING,DEFAULT_SPACING);
    new FXLabel(fontframe, _("Font"));
    fontButton = new FXButton(fontframe, " ", NULL, this, ID_FONT, LAYOUT_CENTER_Y|FRAME_RAISED|JUSTIFY_CENTER_X|JUSTIFY_CENTER_Y|LAYOUT_FILL_X);
    
    new FXButton(buttonframe, _("&General"), NULL, switcher, FXSwitcher::ID_OPEN_THIRD, FRAME_RAISED);
    new FXButton(buttonframe, _("&Look"), NULL, switcher, FXSwitcher::ID_OPEN_FOURTH, FRAME_RAISED);
    new FXButton(buttonframe, _("&Irc Text"), NULL, switcher, FXSwitcher::ID_OPEN_FIRST, FRAME_RAISED);
    new FXButton(buttonframe, _("I&gnore"), NULL, switcher, FXSwitcher::ID_OPEN_SECOND, FRAME_RAISED);
    switcher->setCurrent(2);

    for(int i=0; i<6; i++)
    {
        textStyle[i].normalForeColor = colors.text;
        textStyle[i].normalBackColor = colors.back;
        textStyle[i].selectForeColor = getApp()->getSelforeColor();
        textStyle[i].selectBackColor = getApp()->getSelbackColor();
        textStyle[i].hiliteForeColor = getApp()->getHiliteColor();
        textStyle[i].hiliteBackColor = FXRGB(255, 128, 128); // from FXText.cpp
        textStyle[i].activeBackColor = colors.back;
        textStyle[i].style = 0;
    }
    //user commands
    textStyle[0].normalForeColor = colors.user;
    //Actions
    textStyle[1].normalForeColor = colors.action;
    //Notice
    textStyle[2].normalForeColor = colors.notice;
    //Errors
    textStyle[3].normalForeColor = colors.error;
    //Highlight
    textStyle[4].normalForeColor = colors.hilight;
    //link
    textStyle[5].normalForeColor = colors.link;
    textStyle[5].style = FXText::STYLE_UNDERLINE;

    text->setStyled(TRUE);
    text->setHiliteStyles(textStyle);
    text->setTextColor(colors.text);
    text->setBackColor(colors.back);
    text->appendText("[00:00:00] ");
    text->appendStyledText(FXString("dvx has joined to #dxirc\n"), 1);
    text->appendText("[00:00:00] <dvx> Welcome in dxirc\n");
    text->appendText("[00:00:00] ");
    text->appendStyledText(FXString("<bm> dvx, dxirc is nice\n"), 5);
    text->appendText("[00:00:00] ");
    text->appendStyledText(FXString("dvx is online\n"), 2);
    text->appendText("[00:00:00] ");
    text->appendStyledText(FXString("server's NOTICE: nice notice\n"), 3);
    text->appendText("[00:00:00] ");
    text->appendStyledText(FXString("No error\n"), 4);
    text->appendText("[00:00:00] <dvx> ");
    text->appendStyledText(FXString("http://dxirc.org"), 6);
    text->appendText(" \n");

    UpdateColors();
    UpdateFont();
    UpdateIrcFont();
}


ConfigDialog::~ConfigDialog()
{
    delete font;
    delete ircFont;
}

long ConfigDialog::OnCommandsSelected(FXObject*, FXSelector, void*)
{
    deleteCommand->enable();
    return 1;
}

long ConfigDialog::OnCommandsDeselected(FXObject*, FXSelector, void*)
{
    deleteCommand->disable();
    return 1;
}

long ConfigDialog::OnUsersSelected(FXObject*, FXSelector, void *ptr)
{
    FXint i = (FXint)(FXival)ptr;
    users->selectItem(i);
    channels->selectItem(i);
    servers->selectItem(i);
    deleteUser->enable();
    return 1;
}

long ConfigDialog::OnUsersDeselected(FXObject*, FXSelector, void*)
{
    deleteUser->disable();
    return 1;
}

long ConfigDialog::OnUsersChanged(FXObject*, FXSelector, void *ptr)
{
    FXint i = (FXint)(FXival)ptr;
    users->selectItem(i);
    channels->selectItem(i);
    servers->selectItem(i);
    return 1;
}

long ConfigDialog::OnIconsChanged(FXObject*, FXSelector, void *ptr)
{
    FXint i = (FXint)(FXival)ptr;
    icons->selectItem(i);
    themePath = icons->getItemText(i);
    icon1->setIcon(MakeIcon(getApp(), themePath, "irc_admin.png", true));
    icon2->setIcon(MakeIcon(getApp(), themePath, "irc_owner.png", true));
    icon3->setIcon(MakeIcon(getApp(), themePath, "irc_op.png", true));
    icon4->setIcon(MakeIcon(getApp(), themePath, "irc_halfop.png", true));
    icon5->setIcon(MakeIcon(getApp(), themePath, "irc_voice.png", true));
    icon6->setIcon(MakeIcon(getApp(), themePath, "irc_normal.png", true));
    icon7->setIcon(MakeAwayIcon(getApp(), themePath, "irc_normal.png"));
    return 1;
}

long ConfigDialog::OnColor(FXObject*, FXSelector, void*)
{
    text->setTextColor(colors.text);
    text->setBackColor(colors.back);
    for(int i=0; i<6; i++)
    {
        textStyle[i].normalBackColor = colors.back;
    }
    textStyle[0].normalForeColor = colors.user;
    textStyle[1].normalForeColor = colors.action;
    textStyle[2].normalForeColor = colors.notice;
    textStyle[3].normalForeColor = colors.error;
    textStyle[4].normalForeColor = colors.hilight;
    textStyle[5].normalForeColor = colors.link;
    text->update();
    return 1;
}

long ConfigDialog::OnAccept(FXObject*, FXSelector, void*)
{
    getApp()->stopModal(this,TRUE);
    hide();
    return 1;
}

long ConfigDialog::OnCancel(FXObject*,FXSelector,void*)
{
    getApp()->stopModal(this,FALSE);
    hide();
    return 1;
}

long ConfigDialog::OnKeyPress(FXObject *sender,FXSelector sel,void *ptr)
{
    if(FXTopWindow::onKeyPress(sender,sel,ptr)) return 1;
    if(((FXEvent*)ptr)->code == KEY_Escape)
    {
        getApp()->stopModal(this,FALSE);
        hide();
        return 1;
    }
    return 0;
}

long ConfigDialog::OnAddCommand(FXObject*, FXSelector, void*)
{
    FXDialogBox dialog(this, _("Select ignore command"), DECOR_TITLE|DECOR_BORDER, 0,0,0,0, 0,0,0,0, 0,0);
    FXVerticalFrame *contents = new FXVerticalFrame(&dialog, LAYOUT_SIDE_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 10,10,10,10, 0,0);
    FXMatrix *matrix = new FXMatrix(contents,2,MATRIX_BY_COLUMNS|LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y);

    new FXLabel(matrix, _("Command:"),NULL,JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXComboBox *command = new FXComboBox(matrix, 1, NULL, 0, COMBOBOX_STATIC|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|FRAME_GROOVE);
    command->fillItems(FillCommandsCombo());

    FXHorizontalFrame *buttonframe = new FXHorizontalFrame(contents,LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXButton(buttonframe, _("OK"), NULL, &dialog, FXDialogBox::ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 32,32,5,5);
    new FXButton(buttonframe, _("Cancel"), NULL, &dialog, FXDialogBox::ID_CANCEL, FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 32,32,5,5);

    if(dialog.execute(PLACEMENT_CURSOR))
    {
        commands->appendItem(command->getItemText(command->getCurrentItem()));
    }
    UpdateCommands();
    return 1;
}

long ConfigDialog::OnDeleteCommand(FXObject*, FXSelector, void*)
{
    FXint i = commands->getCurrentItem();
    commands->removeItem(i);
    commands->getNumItems() ? deleteCommand->enable() : deleteCommand->disable();
    UpdateCommands();
    return 1;
}

long ConfigDialog::OnAddUser(FXObject*, FXSelector, void*)
{
    FXDialogBox dialog(this, _("Add ignore user"), DECOR_TITLE|DECOR_BORDER, 0,0,0,0, 0,0,0,0, 0,0);
    FXVerticalFrame *contents = new FXVerticalFrame(&dialog, LAYOUT_SIDE_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 10,10,10,10, 0,0);
    FXMatrix *matrix = new FXMatrix(contents,2,MATRIX_BY_COLUMNS|LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y);

    new FXLabel(matrix, _("Nick:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *nick = new FXTextField(matrix, 25, NULL, 0, FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    nick->setText("_example!example@example.com");
    new FXLabel(matrix, _("Channel(s):\tChannels need to be comma separated"), NULL,JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *channel = new FXTextField(matrix, 25, NULL, 0, FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    channel->setText("all");
    channel->setTipText(_("Channels need to be comma separated"));
    new FXLabel(matrix, _("Server:"), NULL,JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *server = new FXTextField(matrix, 25, NULL, 0, FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    server->setText("all");

    FXHorizontalFrame *buttonframe = new FXHorizontalFrame(contents,LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXButton(buttonframe, _("OK"), NULL, &dialog, FXDialogBox::ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 32,32,5,5);
    new FXButton(buttonframe, _("Cancel"), NULL, &dialog, FXDialogBox::ID_CANCEL, FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 32,32,5,5);

    if(dialog.execute(PLACEMENT_CURSOR))
    {
        if(!nick->getText().empty() && !NickExist(nick->getText()))
        {
            IgnoreUser user;
            user.nick = nick->getText();
            channel->getText().empty() ? user.channel = "all" : user.channel = channel->getText();
            server->getText().empty() ? user.server = "all" : user.server = server->getText();
            usersList.append(user);
            users->appendItem(user.nick);
            channels->appendItem(user.channel);
            servers->appendItem(user.server);
            if(!deleteUser->isEnabled()) deleteUser->enable();
            if(!modifyUser->isEnabled()) modifyUser->enable();
        }
    }
    return 1;
}

long ConfigDialog::OnModifyUser(FXObject*, FXSelector, void*)
{
    FXint i = users->getCurrentItem();
    FXString oldnick = usersList[i].nick;

    FXDialogBox dialog(this, _("Modify ignore user"), DECOR_TITLE|DECOR_BORDER, 0,0,0,0, 0,0,0,0, 0,0);
    FXVerticalFrame *contents = new FXVerticalFrame(&dialog, LAYOUT_SIDE_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 10,10,10,10, 0,0);
    FXMatrix *matrix = new FXMatrix(contents,2,MATRIX_BY_COLUMNS|LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y);

    new FXLabel(matrix, _("Nick:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *nick = new FXTextField(matrix, 25, NULL, 0, FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    nick->setText(oldnick);
    new FXLabel(matrix, _("Channel(s):\tChannels need to be comma separated"), NULL,JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *channel = new FXTextField(matrix, 25, NULL, 0, FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    channel->setText(usersList[i].channel);
    new FXLabel(matrix, _("Server:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *server = new FXTextField(matrix, 25, NULL, 0, FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    server->setText(usersList[i].server);

    FXHorizontalFrame *buttonframe = new FXHorizontalFrame(contents,LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXButton(buttonframe, _("OK"), NULL, &dialog, FXDialogBox::ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 32,32,5,5);
    new FXButton(buttonframe, _("Cancel"), NULL, &dialog, FXDialogBox::ID_CANCEL, FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 32,32,5,5);

    if(dialog.execute(PLACEMENT_CURSOR))
    {
        if(!nick->getText().empty() && (!NickExist(nick->getText()) || oldnick == nick->getText()))
        {
            usersList[i].nick = nick->getText();
            channel->getText().empty() ? usersList[i].channel = "all" : usersList[i].channel = channel->getText();
            server->getText().empty() ? usersList[i].server = "all" : usersList[i].server = server->getText();
            users->setItemText(i, usersList[i].nick);
            channels->setItemText(i, usersList[i].channel);
            servers->setItemText(i, usersList[i].server);
        }
    }
    return 1;
}

long ConfigDialog::OnDeleteUser(FXObject*, FXSelector, void*)
{
    FXint i = users->getCurrentItem();
    users->removeItem(i);
    channels->removeItem(i);
    servers->removeItem(i);
    usersList.erase(i);
    if(usersList.no())
    {
        deleteUser->enable();
        modifyUser->enable();
    }
    else
    {
        deleteUser->disable();
        modifyUser->disable();
    }
    return 1;
}

long ConfigDialog::OnAddIcons(FXObject*, FXSelector, void*)
{
    FXDirDialog dirdialog(this, _("Select theme directory"));
    if(dirdialog.execute(PLACEMENT_CURSOR))
    {
        if(!FXPath::search(dirdialog.getDirectory(), "irc_normal.png").empty() && !ThemeExist(dirdialog.getDirectory())) icons->appendItem(dirdialog.getDirectory());
    }
    icons->getNumItems()>1 ? deleteTheme->enable() : deleteTheme->disable();
    UpdateIcons();
    return 1;
}

long ConfigDialog::OnDeleteIcons(FXObject*, FXSelector, void*)
{
    FXint i = icons->getCurrentItem();
    icons->removeItem(i);
    icons->selectItem(icons->getNumItems()-1, true);
    icons->getNumItems()>1 ? deleteTheme->enable() : deleteTheme->disable();
    UpdateIcons();
    return 1;
}

long ConfigDialog::OnTheme(FXObject*, FXSelector, void *ptr)
{
    FXint no = (FXint)(FXival)ptr;
    ColorTheme *themeSelected = reinterpret_cast<ColorTheme*>(themes->getItemData(no));
    if(themeSelected)
    {
        themeCurrent.back = themeSelected->back;
        themeCurrent.base = themeSelected->base;
        themeCurrent.border = themeSelected->border;
        themeCurrent.fore = themeSelected->fore;
        themeCurrent.menuback = themeSelected->menuback;
        themeCurrent.menufore = themeSelected->menufore;
        themeCurrent.selback = themeSelected->selback;
        themeCurrent.selfore = themeSelected->selfore;
        themeCurrent.tipback = themeSelected->tipback;
        themeCurrent.tipfore = themeSelected->tipfore;
        UpdateColors();
    }
    return 1;
}

long ConfigDialog::OnFont(FXObject*, FXSelector, void*)
{
    FXFontDialog dialog(this, _("Select font"));
    FXFontDesc fontdescription;
    font->getFontDesc(fontdescription);
    strncpy(fontdescription.face,font->getActualName().text(),sizeof(fontdescription.face));
    dialog.setFontSelection(fontdescription);
    if(dialog.execute(PLACEMENT_SCREEN))
    {
        FXFont *oldfont = font;
        dialog.getFontSelection(fontdescription);
        font = new FXFont(getApp(),fontdescription);
        font->create();
        delete oldfont;
        UpdateFont();
    }
    return 1;
}

long ConfigDialog::OnIrcFont(FXObject*, FXSelector, void*)
{
    FXFontDialog dialog(this, _("Select font"));
    FXFontDesc fontdescription;
    ircFont->getFontDesc(fontdescription);
    strncpy(fontdescription.face,ircFont->getActualName().text(),sizeof(fontdescription.face));
    dialog.setFontSelection(fontdescription);
    if(dialog.execute(PLACEMENT_SCREEN))
    {
        FXFont *oldfont = ircFont;
        dialog.getFontSelection(fontdescription);
        ircFont = new FXFont(getApp(),fontdescription);
        ircFont->create();
        delete oldfont;
        UpdateIrcFont();
    }
    return 1;
}

long ConfigDialog::OnThemeColorChanged(FXObject*, FXSelector, void*)
{
    themes->setCurrentItem(themes->getNumItems()-1);
    UpdateColors();
    return 1;
}

void ConfigDialog::UpdateColors()
{
    themeCurrent.shadow = makeShadowColor(themeCurrent.base);
    themeCurrent.hilite = makeHiliteColor(themeCurrent.base);

    vframe2->setBorderColor(themeCurrent.border);
    vframe2->setBaseColor(themeCurrent.base);
    vframe2->setBackColor(themeCurrent.base);
    vframe2->setShadowColor(themeCurrent.shadow);
    vframe2->setHiliteColor(themeCurrent.hilite);

    label->setBorderColor(themeCurrent.border);
    label->setBaseColor(themeCurrent.base);
    label->setBackColor(themeCurrent.base);
    label->setTextColor(themeCurrent.fore);
    label->setShadowColor(themeCurrent.shadow);
    label->setHiliteColor(themeCurrent.hilite);

    textFrame1->setBorderColor(themeCurrent.border);
    textFrame1->setBaseColor(themeCurrent.base);
    textFrame1->setBackColor(themeCurrent.base);
    textFrame1->setShadowColor(themeCurrent.shadow);
    textFrame1->setHiliteColor(themeCurrent.hilite);
    textTest->setBorderColor(themeCurrent.border);
    textTest->setBackColor(themeCurrent.back);
    textTest->setBaseColor(themeCurrent.base);
    textTest->setTextColor(themeCurrent.fore);
    textTest->setSelTextColor(themeCurrent.selfore);
    textTest->setSelBackColor(themeCurrent.selback);
    textTest->setCursorColor(themeCurrent.fore);
    textTest->setShadowColor(themeCurrent.shadow);
    textTest->setHiliteColor(themeCurrent.hilite);

    textFrame2->setBorderColor(themeCurrent.border);
    textFrame2->setBaseColor(themeCurrent.base);
    textFrame2->setBackColor(themeCurrent.back);
    textFrame2->setShadowColor(themeCurrent.shadow);
    textFrame2->setHiliteColor(themeCurrent.hilite);
    labelSelected->setBorderColor(themeCurrent.border);
    labelSelected->setBaseColor(themeCurrent.base);
    labelSelected->setBackColor(themeCurrent.selback);
    labelSelected->setTextColor(themeCurrent.selfore);
    labelSelected->setShadowColor(themeCurrent.shadow);
    labelSelected->setHiliteColor(themeCurrent.hilite);

    textFrame3->setBorderColor(themeCurrent.border);
    textFrame3->setBaseColor(themeCurrent.base);
    textFrame3->setBackColor(themeCurrent.back);
    textFrame3->setShadowColor(themeCurrent.shadow);
    textFrame3->setHiliteColor(themeCurrent.hilite);
    labelNocurrent->setBorderColor(themeCurrent.border);
    labelNocurrent->setBaseColor(themeCurrent.base);
    labelNocurrent->setBackColor(themeCurrent.base);
    labelNocurrent->setTextColor(themeCurrent.fore);
    labelNocurrent->setShadowColor(themeCurrent.shadow);
    labelNocurrent->setHiliteColor(themeCurrent.hilite);

    sep1->setBorderColor(themeCurrent.border);
    sep1->setBaseColor(themeCurrent.base);
    sep1->setBackColor(themeCurrent.base);
    sep1->setShadowColor(themeCurrent.shadow);
    sep1->setHiliteColor(themeCurrent.hilite);

    labelTip->setBorderColor(themeCurrent.tipfore);
    labelTip->setBaseColor(themeCurrent.tipback);
    labelTip->setBackColor(themeCurrent.tipback);
    labelTip->setTextColor(themeCurrent.tipfore);
    labelTip->setShadowColor(themeCurrent.shadow);
    labelTip->setHiliteColor(themeCurrent.hilite);
    
    menuGroup->setBorderColor(themeCurrent.border);
    menuGroup->setBaseColor(themeCurrent.base);
    menuGroup->setBackColor(themeCurrent.base);
    menuGroup->setShadowColor(themeCurrent.shadow);
    menuGroup->setHiliteColor(themeCurrent.hilite);
    menuGroup->setTextColor(themeCurrent.fore);
    menuFrame->setBorderColor(themeCurrent.border);
    menuFrame->setBaseColor(themeCurrent.base);
    menuFrame->setBackColor(themeCurrent.base);
    menuFrame->setShadowColor(themeCurrent.shadow);
    menuFrame->setHiliteColor(themeCurrent.hilite);
    sep2->setBorderColor(themeCurrent.border);
    sep2->setBaseColor(themeCurrent.base);
    sep2->setBackColor(themeCurrent.base);
    sep2->setShadowColor(themeCurrent.shadow);
    sep2->setHiliteColor(themeCurrent.hilite);
    for (int i=0; i<3; i++)
    {
        menuLabels[i]->setBorderColor(themeCurrent.border);
        menuLabels[i]->setBaseColor(themeCurrent.base);
        menuLabels[i]->setBackColor(themeCurrent.base);
        menuLabels[i]->setTextColor(themeCurrent.fore);
        menuLabels[i]->setShadowColor(themeCurrent.shadow);
        menuLabels[i]->setHiliteColor(themeCurrent.hilite);
    }
    menuLabels[1]->setBorderColor(themeCurrent.border);
    menuLabels[1]->setBaseColor(themeCurrent.menuback);
    menuLabels[1]->setBackColor(themeCurrent.menuback);
    menuLabels[1]->setTextColor(themeCurrent.menufore);
    menuLabels[1]->setShadowColor(themeCurrent.shadow);
    menuLabels[1]->setHiliteColor(themeCurrent.hilite);
}

static FXString weightToString(FXuint weight){
    switch(weight)
    {
        case FXFont::Thin      : return "thin"; break;
        case FXFont::ExtraLight: return "extralight"; break;
        case FXFont::Light     : return "light"; break;
        case FXFont::Normal    : return "normal"; break;
        case FXFont::Medium    : return "medium"; break;
        case FXFont::DemiBold  : return "demibold"; break;
        case FXFont::Bold      : return "bold"; break;
        case FXFont::ExtraBold : return "extrabold"; break;
        case FXFont::Black     : return "heavy"; break;
        default: return ""; break;
    }
    return "";
}

static FXString slantToString(FXuint slant)
{
    switch(slant)
    {
        case FXFont::Straight       : return "regular"; break;
        case FXFont::Italic         : return "italic"; break;
        case FXFont::Oblique        : return "oblique"; break;
        case FXFont::ReverseItalic  : return "reverse italic"; break;
        case FXFont::ReverseOblique : return "reverse oblique"; break;
        default : return ""; break;
    }
    return "";
}

void ConfigDialog::UpdateFont()
{
    FXString fontname = font->getActualName() +", " + FXStringVal(font->getSize()/10);
    if(font->getWeight()!=0 && font->getWeight()!=FXFont::Normal)
    {
        fontname += ", " + weightToString(font->getWeight());
    }
    if (font->getSlant()!=0 && font->getSlant()!=FXFont::Straight)
    {
        fontname += ", " + slantToString(font->getSlant());
    }
    fontButton->setText(fontname);
    
    labelSelected->setFont(font);
    labelNocurrent->setFont(font);
    labelTip->setFont(font);
    label->setFont(font);
    textTest->setFont(font);
    menuGroup->setFont(font);
    menuLabels[0]->setFont(font);
    menuLabels[1]->setFont(font);
    menuLabels[2]->setFont(font);
}

void ConfigDialog::UpdateIrcFont()
{
    FXString fontname = ircFont->getActualName() +", " + FXStringVal(ircFont->getSize()/10);
    if(ircFont->getWeight()!=0 && ircFont->getWeight()!=FXFont::Normal)
    {
        fontname += ", " + weightToString(ircFont->getWeight());
    }
    if (ircFont->getSlant()!=0 && ircFont->getSlant()!=FXFont::Straight)
    {
        fontname += ", " + slantToString(ircFont->getSlant());
    }
    ircfontButton->setText(fontname);

    text->setFont(ircFont);
}

long ConfigDialog::OnLogChanged(FXObject*, FXSelector, void*)
{
    if(logging) selectPath->enable();
    else selectPath->disable();
    return 1;
}

long ConfigDialog::OnNickCharChanged(FXObject*, FXSelector, void*)
{
    nickChar = nickCharField->getText().left(1);
    nickCharField->setText(nickChar);
    return 1;
}

long ConfigDialog::OnServerWindow(FXObject*, FXSelector, void*)
{
    ShowMessage();
    return 1;
}

long ConfigDialog::OnTray(FXObject*, FXSelector, void*)
{
    if(useTray)
        closeToTrayButton->enable();
    else
    {
        closeToTray = FALSE;
        closeToTrayButton->disable();
    }
    ShowMessage();
    return 1;
}

long ConfigDialog::OnReconnect(FXObject*, FXSelector, void*)
{
    if(reconnect)
    {
        numberAttemptLabel->enable();
        numberAttemptSpinner->enable();
        delayAttemptLabel->enable();
        delayAttemptSpinner->enable();
    }
    else
    {
        numberAttemptLabel->disable();
        numberAttemptSpinner->disable();
        delayAttemptLabel->disable();
        delayAttemptSpinner->disable();
    }
    return 1;
}

long ConfigDialog::OnPathSelect(FXObject*, FXSelector, void*)
{
    FXDirDialog dirdialog(this,_("Select log directory"));
    dirdialog.setDirectory(folder->getText());
    if(dirdialog.execute(PLACEMENT_CURSOR))
    {
        logPath = dirdialog.getDirectory();
        folder->setText(logPath);
    }
    return 1;
}

void ConfigDialog::FillCommnads()
{
    if(!commandsList.empty())
    {
        for(FXint i=0; i<commandsList.contains(';'); i++)
        {
            commands->appendItem(commandsList.before(';', i+1).rafter(';'));
        }
    }
}

void ConfigDialog::FillIcons()
{
    if(!themesList.empty())
    {
        for(FXint i=0; i<themesList.contains(';'); i++)
        {
            icons->appendItem(themesList.before(';', i+1).rafter(';'));
        }
    }
}

void ConfigDialog::FillThemes()
{
    FXint i, scheme=-1;

    for(i=0; i<numThemes; i++)
    {
        if((themeCurrent.back == ColorThemes[i].back) &&
                (themeCurrent.base == ColorThemes[i].base) &&
                (themeCurrent.border == ColorThemes[i].border) &&
                (themeCurrent.fore == ColorThemes[i].fore) &&
                (themeCurrent.menuback == ColorThemes[i].menuback) &&
                (themeCurrent.menufore == ColorThemes[i].menufore) &&
                (themeCurrent.selback == ColorThemes[i].selback) &&
                (themeCurrent.selfore == ColorThemes[i].selfore) &&
                (themeCurrent.tipback == ColorThemes[i].tipback) &&
                (themeCurrent.tipfore == ColorThemes[i].tipfore))
        {
            scheme = i;
            break;
        }
    }

    if(scheme == -1)
    {
        themeUser.back = themeCurrent.back;
        themeUser.base = themeCurrent.base;
        themeUser.border = themeCurrent.border;
        themeUser.fore = themeCurrent.fore;
        themeUser.menuback = themeCurrent.menuback;
        themeUser.menufore = themeCurrent.menufore;
        themeUser.selback = themeCurrent.selback;
        themeUser.selfore = themeCurrent.selfore;
        themeUser.tipback = themeCurrent.tipback;
        themeUser.tipfore = themeCurrent.tipfore;
        themes->appendItem(_("Current"), NULL, &themeUser);
    }

    for(i=0; i<numThemes; i++)
    {
        themes->appendItem(ColorThemes[i].name,NULL,(void*)&ColorThemes[i]);
    }
    themes->appendItem(_("User Defined"));
    themes->setCurrentItem(scheme);
}

FXString ConfigDialog::FillCommandsCombo()
{
    FXString combo;
    FXString available[10] = { "away", "ban", "ctcp", "join", "me", "nick", "notice", "mode", "part", "quit"};
    for(FXint i=0; i<10; i++)
    {
        FXbool exist = false;
        for(FXint j=0; j<commands->getNumItems(); j++)
        {
            if(commands->getItemText(j) == available[i])
            {
                exist = true;
                break;
            }
        }
        if(!exist) combo.append(available[i]+'\n');
    }
    return combo;
}

void ConfigDialog::FillUsers()
{
    if(usersList.no())
    {
        for(FXint i=0; i<usersList.no(); i++)
        {
            users->appendItem(usersList[i].nick);
            channels->appendItem(usersList[i].channel);
            servers->appendItem(usersList[i].server);
        }
    }
}

void ConfigDialog::UpdateCommands()
{
    commandsList.clear();
    for(FXint i=0; i<commands->getNumItems(); i++)
    {
        commandsList.append(commands->getItemText(i)+';');
    }
}

void ConfigDialog::UpdateIcons()
{
    themesList.clear();
    for(FXint i=0; i<icons->getNumItems(); i++)
    {
        themesList.append(icons->getItemText(i)+';');
    }
}

void ConfigDialog::ShowMessage()
{
    FXMessageBox::information(this, FX::MBOX_OK, _("Information"), _("Some changes need restart application"));
}

FXbool ConfigDialog::ThemeExist(const FXString &ckdTheme)
{
    for(FXint i=0; i<icons->getNumItems(); i++)
    {
        if(icons->getItemText(i) == ckdTheme)
        {
            return true;
        }
    }
    return false;
}

FXbool ConfigDialog::NickExist(const FXString &ckdNick)
{
    for(FXint i=0; i<users->getNumItems(); i++)
    {
        if(users->getItemText(i) == ckdNick) return true;
    }
    return false;
}
