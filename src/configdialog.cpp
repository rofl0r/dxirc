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
#include "utils.h"

#define MAXBUFFER 2000
#define MAXVALUE  2000
#define HEADER "# dxirc smiley settings\n# path is relative to file\n# Example: /home/xxx/smile.png=:)\n"

static FXIcon *createIconFromName(FXApp *app, const FXString& path)
{
    FXIconSource iconsource(app);
    FXIcon *icon = NULL;
    if(!FXStat::exists(path))
        return NULL;
    icon = iconsource.loadScaledIconFile(path,16);
    if(icon)
    {
        icon->blend(app->getBaseColor());
        icon->create();
        return icon;
    }
    return NULL;
}

static void updateLabelIcon(FXApp* app, FXLabel* label, const FXString& path)
{
    FXIconSource iconsource(app);
    FXIcon *icon = NULL;
    if(label->getIcon())
    {
        icon = label->getIcon();
        label->setIcon(NULL);
        delete icon;
        icon = NULL;
    }
    icon = iconsource.loadScaledIconFile(path,16);
    if(icon)
    {
        icon->blend(app->getBaseColor());
        icon->create();
        label->setIcon(icon);
    }
}

// Check if quotes are needed
static bool needquotes(const FXchar* text)
{
    register const FXchar *ptr=text;
    register FXuchar c;
    while((c=*ptr++)!='\0')
    {
        if(0x7f<=c || c<0x20 || c=='"' || c=='\'' || c=='\\' || (c==' ' && (ptr==(text+1) || *ptr=='\0'))) return true;
    }
    return false;
}

// Read string
static bool readString(FXFile& file, FXchar *buffer, FXint& bol, FXint& eol, FXint& end)
{
    register FXint n;
    do
    {
        if(eol >= end)
        {
            if(bol < end)
            {
                memmove(buffer, buffer + bol, end - bol);
            }
            end = end - bol;
            bol = 0;
            eol = end;
            n = file.readBlock(buffer + end, MAXBUFFER - end);
            if(n < 0) return false;
            end += n;
        }
    } while (eol < end && buffer[eol++] != '\n');
    return bol<eol;
}

// Write string
static bool writeString(FXFile& file, const FXchar* string)
{
    register FXint len = strlen(string);
    return file.writeBlock(string, len) == len;
}

FXDEFMAP(SmileyDialog) SmileyDialogMap[] = {
    FXMAPFUNC(SEL_COMMAND, SmileyDialog::ID_ACCEPT, SmileyDialog::OnAccept),
    FXMAPFUNC(SEL_COMMAND, SmileyDialog::ID_CANCEL, SmileyDialog::OnCancel),
    FXMAPFUNC(SEL_COMMAND, SmileyDialog::ID_PATH, SmileyDialog::OnPath)
};

FXIMPLEMENT(SmileyDialog, FXDialogBox, SmileyDialogMap, ARRAYNUMBER(SmileyDialogMap))

SmileyDialog::SmileyDialog(FXWindow* owner, FXString title, FXString smiley, FXString path)
        : FXDialogBox(owner, title, DECOR_RESIZE|DECOR_TITLE|DECOR_BORDER, 0,0,0,0, 0,0,0,0, 0,0)
{
    FXHorizontalFrame *closeframe = new FXHorizontalFrame(this, LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X|PACK_UNIFORM_WIDTH);
    new FXButton(closeframe, _("&Cancel"), NULL, this, ID_CANCEL, LAYOUT_RIGHT|FRAME_RAISED|FRAME_THICK, 0,0,0,0, 10,10,2,5);
    FXButton *ok = new FXButton(closeframe, _("&OK"), NULL, this, ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|LAYOUT_RIGHT|FRAME_RAISED|FRAME_THICK, 0,0,0,0, 10,10,2,5);
    ok->addHotKey(KEY_Return);

    FXMatrix *matrix = new FXMatrix(this,2,MATRIX_BY_COLUMNS|LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(matrix, _("Smiley:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    smileyText = new FXTextField(matrix, 25, NULL, 0, FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    if(!smiley.empty()) smileyText->setText(smiley);
    new FXLabel(matrix, _("Path:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXHorizontalFrame *pathframe = new FXHorizontalFrame(matrix, LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW, 0,0,0,0, 0,0,0,0);
    pathText = new FXTextField(pathframe, 25, NULL, 0, TEXTFIELD_READONLY|FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_X);
    pathText->setBackColor(getApp()->getBaseColor());
    if(!path.empty()) pathText->setText(path);
    pathButton = new FXButton(pathframe, "...", NULL, this, ID_PATH, FRAME_RAISED|FRAME_THICK);
    new FXLabel(matrix, _("Preview:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    previewLabel = new FXLabel(matrix, "", path.empty() ? NULL : createIconFromName(getApp(), path), JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
}

SmileyDialog::~SmileyDialog()
{

}

long SmileyDialog::OnAccept(FXObject*, FXSelector, void*)
{
    getApp()->stopModal(this,TRUE);
    hide();
    return 1;
}

long SmileyDialog::OnCancel(FXObject*, FXSelector, void*)
{
    getApp()->stopModal(this,FALSE);
    hide();
    return 1;
}

long SmileyDialog::OnPath(FXObject*, FXSelector, void*)
{
    FXFileDialog dialog(this, _("Select file"));
    if(!pathText->getText().empty()) dialog.setFilename(pathText->getText());
    else dialog.setFilename((FXString)DXIRC_DATADIR+PATHSEPSTRING+"icons"+PATHSEPSTRING+"smileys"+PATHSEPSTRING);
    if(dialog.execute())
    {
        pathText->setText(dialog.getFilename());
        updateLabelIcon(getApp(), previewLabel, dialog.getFilename());
    }
    return 1;
}

FXbool SmileyDialog::IconExist()
{
    if(previewLabel->getIcon()) return TRUE;
    return FALSE;
}

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
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_ADDFRIEND, ConfigDialog::OnAddFriend),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_MODIFYFRIEND, ConfigDialog::OnModifyFriend),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_DELETEFRIEND, ConfigDialog::OnDeleteFriend),
    FXMAPFUNC(SEL_CHANGED, ConfigDialog::ID_ICONS, ConfigDialog::OnIconsChanged),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_ADDICONS, ConfigDialog::OnAddIcons),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_DELETEICONS, ConfigDialog::OnDeleteIcons),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_TRAY, ConfigDialog::OnTray),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_LOG, ConfigDialog::OnLogChanged),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_LOGPATH, ConfigDialog::OnPathSelect),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_AUTOLOAD, ConfigDialog::OnAutoloadChanged),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_AUTOLOADPATH, ConfigDialog::OnAutoloadPathSelect),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_DCCPATH, ConfigDialog::OnDccPathSelect),
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
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_TABPOS, ConfigDialog::OnTabPosition),
    FXMAPFUNC(SEL_SELECTED, ConfigDialog::ID_USER, ConfigDialog::OnUsersSelected),
    FXMAPFUNC(SEL_DESELECTED, ConfigDialog::ID_USER, ConfigDialog::OnUsersDeselected),
    FXMAPFUNC(SEL_CHANGED, ConfigDialog::ID_USER, ConfigDialog::OnUsersChanged),
    FXMAPFUNC(SEL_SELECTED, ConfigDialog::ID_FRIEND, ConfigDialog::OnFriendsSelected),
    FXMAPFUNC(SEL_DESELECTED, ConfigDialog::ID_FRIEND, ConfigDialog::OnFriendsDeselected),
    FXMAPFUNC(SEL_CHANGED, ConfigDialog::ID_FRIEND, ConfigDialog::OnFriendsChanged),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_DCCPORTD, ConfigDialog::OnDccPortD),
    FXMAPFUNC(SEL_CHANGED, ConfigDialog::ID_DCCPORTD, ConfigDialog::OnDccPortD),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_DCCPORTH, ConfigDialog::OnDccPortH),
    FXMAPFUNC(SEL_CHANGED, ConfigDialog::ID_DCCPORTH, ConfigDialog::OnDccPortH),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_SOUNDS, ConfigDialog::OnSounds),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_SOUNDCONNECT, ConfigDialog::OnSoundConnect),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_SOUNDDISCONNECT, ConfigDialog::OnSoundDisconnect),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_SOUNDMESSAGE, ConfigDialog::OnSoundMessage),
    FXMAPFUNCS(SEL_COMMAND, ConfigDialog::ID_PLAYCONNECT, ConfigDialog::ID_PLAYMESSAGE, ConfigDialog::OnPlay),
    FXMAPFUNCS(SEL_COMMAND, ConfigDialog::ID_SELECTCONNECT, ConfigDialog::ID_SELECTMESSAGE, ConfigDialog::OnSelectPath),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_USESMILEYS, ConfigDialog::OnUseSmileys),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_ADDSMILEY, ConfigDialog::OnAddSmiley),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_MODIFYSMILEY, ConfigDialog::OnModifySmiley),
    FXMAPFUNC(SEL_DOUBLECLICKED, ConfigDialog::ID_SMILEY, ConfigDialog::OnModifySmiley),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_DELETESMILEY, ConfigDialog::OnDeleteSmiley),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_IMPORTSMILEY, ConfigDialog::OnImportSmiley),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_EXPORTSMILEY, ConfigDialog::OnExportSmiley)
};

FXIMPLEMENT(ConfigDialog, FXDialogBox, ConfigDialogMap, ARRAYNUMBER(ConfigDialogMap))

ConfigDialog::ConfigDialog(FXMainWindow *owner)
    : FXDialogBox(owner, _("Preferences"), DECOR_RESIZE|DECOR_TITLE|DECOR_BORDER, 0,0,0,0, 0,0,0,0, 0,0), owner(owner)
{
    ReadConfig();
    showImportwarning = TRUE;
    showWarning = TRUE;

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
    autoloadTarget.connect(autoload);
    autoloadTarget.setTarget(this);
    autoloadTarget.setSelector(ID_AUTOLOAD);

    targetReconnect.connect(reconnect);
    targetReconnect.setTarget(this);
    targetReconnect.setSelector(ID_RECONNECT);
    targetNumberAttempt.connect(numberAttempt);
    targetDelayAttempt.connect(delayAttempt);
    targetMaxAway.connect(maxAway);

    targetLogPath.connect(logPath);
    targetDccPath.connect(dccPath);
    targetAutoloadPath.connect(autoloadPath);

    targetDccIP1.connect(dccIP1);
    targetDccIP2.connect(dccIP2);
    targetDccIP3.connect(dccIP3);
    targetDccIP4.connect(dccIP4);

    targetDccPortD.connect(dccPortD);
    targetDccPortD.setTarget(this);
    targetDccPortD.setSelector(ID_DCCPORTD);
    targetDccPortH.connect(dccPortH);
    targetDccPortH.setTarget(this);
    targetDccPortH.setSelector(ID_DCCPORTH);

    targetDccTimeout.connect(dccTimeout);

    targetAutoDccChat.connect(autoDccChat);
    targetAutoDccFile.connect(autoDccFile);

    targetSound.connect(sounds);
    targetSound.setTarget(this);
    targetSound.setSelector(ID_SOUNDS);
    targetSoundConnect.connect(soundConnect);
    targetSoundConnect.setTarget(this);
    targetSoundConnect.setSelector(ID_SOUNDCONNECT);
    targetSoundDisconnect.connect(soundDisconnect);
    targetSoundDisconnect.setTarget(this);
    targetSoundDisconnect.setSelector(ID_SOUNDDISCONNECT);
    targetSoundMessage.connect(soundMessage);
    targetSoundMessage.setTarget(this);
    targetSoundMessage.setSelector(ID_SOUNDMESSAGE);
    targetPathConnect.connect(pathConnect);
    targetPathDisconnect.connect(pathDisconnect);
    targetPathMessage.connect(pathMessage);

    targetStripColors.connect(stripColors);

    targetUseSmileys.connect(useSmileys);
    targetUseSmileys.setTarget(this);
    targetUseSmileys.setSelector(ID_USESMILEYS);

    getApp()->getNormalFont()->create();
    FXFontDesc fontdescription;
    getApp()->getNormalFont()->getFontDesc(fontdescription);
    font = new FXFont(getApp(),fontdescription);
    font->create();

    FXHorizontalFrame *closeframe = new FXHorizontalFrame(this, LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X|PACK_UNIFORM_WIDTH);
    FXButton *ok = new FXButton(closeframe, _("&Save&&Close"), NULL, this, ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|LAYOUT_RIGHT|FRAME_RAISED|FRAME_THICK, 0,0,0,0, 10,10,2,5);
    ok->addHotKey(KEY_Return);
    new FXButton(closeframe, _("&Cancel"), NULL, this, ID_CANCEL, LAYOUT_RIGHT|FRAME_RAISED|FRAME_THICK, 0,0,0,0, 10,10,2,5);

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
    ircfontButton = new FXButton(colormatrix, " ", NULL, this, ID_IRCFONT, LAYOUT_CENTER_Y|FRAME_RAISED|JUSTIFY_CENTER_X|JUSTIFY_CENTER_Y|LAYOUT_FILL_X, 0,0,0,0, 10,10,2,5);
    new FXCheckButton(cframe, _("Use same font for commandline"), &targetSameCmd, FXDataTarget::ID_VALUE, CHECKBUTTON_NORMAL|LAYOUT_FILL_X|LAYOUT_SIDE_LEFT|JUSTIFY_LEFT);
    new FXCheckButton(cframe, _("Use same font for user list"), &targetSameList, FXDataTarget::ID_VALUE, CHECKBUTTON_NORMAL|LAYOUT_FILL_X|LAYOUT_SIDE_LEFT|JUSTIFY_LEFT);
    new FXCheckButton(cframe, _("Use colored nick"), &targetColoredNick, FXDataTarget::ID_VALUE, CHECKBUTTON_NORMAL|LAYOUT_FILL_X|LAYOUT_SIDE_LEFT|JUSTIFY_LEFT);
    new FXCheckButton(cframe, _("Strip colors in text"), &targetStripColors, FXDataTarget::ID_VALUE, CHECKBUTTON_NORMAL|LAYOUT_FILL_X|LAYOUT_SIDE_LEFT|JUSTIFY_LEFT);
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
    commands = new FXList(commandslist, this, ID_COMMAND, LIST_BROWSESELECT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    FXGroupBox *usersgroup = new FXGroupBox(ignorepane, _("Users"), FRAME_GROOVE|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    FXVerticalFrame *usersbuttons = new FXVerticalFrame(usersgroup, LAYOUT_SIDE_RIGHT|LAYOUT_FILL_Y|PACK_UNIFORM_WIDTH);
    addUser = new FXButton(usersbuttons, _("Add"), NULL, this, ID_ADDUSER, FRAME_RAISED|FRAME_THICK);
    modifyUser = new FXButton(usersbuttons, _("Modify"), NULL, this, ID_MODIFYUSER, FRAME_RAISED|FRAME_THICK);
    deleteUser = new FXButton(usersbuttons, _("Delete"), NULL, this, ID_DELETEUSER, FRAME_RAISED|FRAME_THICK);
    FXVerticalFrame *userspane = new FXVerticalFrame(usersgroup, LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN|FRAME_THICK);
    FXHorizontalFrame *usersframe = new FXHorizontalFrame(userspane, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    users = new FXIconList(usersframe, this, ID_USER, ICONLIST_AUTOSIZE|ICONLIST_DETAILED|ICONLIST_BROWSESELECT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    users->appendHeader(_("User"), NULL, 200);
    users->appendHeader(_("Channel(s)"), NULL, 150);
    users->appendHeader(_("Server(s)"), NULL, 150);
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
    icons = new FXList(themeslist, this, ID_ICONS, LIST_BROWSESELECT|LAYOUT_FILL_X|LAYOUT_FILL_Y);    
    FillIcons();
    for(FXint i=0; i<icons->getNumItems(); i++)
    {
        if(icons->getItemText(i) == themePath)
        {
            icons->setCurrentItem(i);
            break;
        }
    }
    icons->getNumItems()>1 ? deleteTheme->enable() : deleteTheme->disable();
    iconsBar = new FXToolBar(themeslist, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    icon1 = new FXButton(iconsBar, _("\tAdmin"), MakeIcon(getApp(), themePath, "irc_admin.png", TRUE), NULL, 0, BUTTON_TOOLBAR);
    icon2 = new FXButton(iconsBar, _("\tOwner"), MakeIcon(getApp(), themePath, "irc_owner.png", TRUE), NULL, 0, BUTTON_TOOLBAR);
    icon3 = new FXButton(iconsBar, _("\tOp"), MakeIcon(getApp(), themePath, "irc_op.png", TRUE), NULL, 0, BUTTON_TOOLBAR);
    icon4 = new FXButton(iconsBar, _("\tHalfop"), MakeIcon(getApp(), themePath, "irc_halfop.png", TRUE), NULL, 0, BUTTON_TOOLBAR);
    icon5 = new FXButton(iconsBar, _("\tVoice"), MakeIcon(getApp(), themePath, "irc_voice.png", TRUE), NULL, 0, BUTTON_TOOLBAR);
    icon6 = new FXButton(iconsBar, _("\tNormal"), MakeIcon(getApp(), themePath, "irc_normal.png", TRUE), NULL, 0, BUTTON_TOOLBAR);
    icon7 = new FXButton(iconsBar, _("\tAway"), MakeAwayIcon(getApp(), themePath, "irc_normal.png"), NULL, 0, BUTTON_TOOLBAR);
    new FXCheckButton(otherpane, _("Reconnect after disconnection"), &targetReconnect, FXDataTarget::ID_VALUE, CHECKBUTTON_NORMAL|LAYOUT_FILL_X|LAYOUT_SIDE_LEFT|JUSTIFY_LEFT);
    FXHorizontalFrame *napane = new FXHorizontalFrame(otherpane, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    numberAttemptLabel = new FXLabel(napane, _("Number of attempts"), NULL, LAYOUT_LEFT);
    if(!reconnect) numberAttemptLabel->disable();
    numberAttemptSpinner = new FXSpinner(napane, 4, &targetNumberAttempt, FXDataTarget::ID_VALUE, SPIN_CYCLIC|FRAME_GROOVE);
    numberAttemptSpinner->setRange(1,20);
    if(!reconnect) numberAttemptSpinner->disable();
    FXHorizontalFrame *dapane = new FXHorizontalFrame(otherpane, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    delayAttemptLabel = new FXLabel(dapane, _("Delay between two attempts in seconds"), NULL, LAYOUT_LEFT);
    if(!reconnect) delayAttemptLabel->disable();
    delayAttemptSpinner = new FXSpinner(dapane, 4, &targetDelayAttempt, FXDataTarget::ID_VALUE, SPIN_CYCLIC|FRAME_GROOVE);
    delayAttemptSpinner->setRange(10,120);
    if(!reconnect) delayAttemptSpinner->disable();
    FXHorizontalFrame *maxpane = new FXHorizontalFrame(otherpane, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(maxpane, _("Max. users number for checking away\tToo high number can be reason for ban"), NULL, LAYOUT_LEFT);
    new FXSpinner(maxpane, 4, &targetMaxAway, FXDataTarget::ID_VALUE, SPIN_NOMAX|FRAME_GROOVE);
    FXHorizontalFrame *nickpane = new FXHorizontalFrame(otherpane, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(nickpane, _("Nick completion char"), NULL, LAYOUT_LEFT);
    nickCharField = new FXTextField(nickpane, 1, this, ID_NICK, TEXTFIELD_LIMITED|FRAME_THICK|FRAME_SUNKEN/*|LAYOUT_FILL_X*/);
    nickCharField->setText(nickChar);
#ifdef HAVE_TRAY
    new FXCheckButton(otherpane, _("Use trayicon"), &trayTarget, FXDataTarget::ID_VALUE, CHECKBUTTON_NORMAL|LAYOUT_FILL_X|LAYOUT_SIDE_LEFT|JUSTIFY_LEFT);
    closeToTrayButton = new FXCheckButton(otherpane, _("Close button hides application"), &targetCloseToTray, FXDataTarget::ID_VALUE, CHECKBUTTON_NORMAL|LAYOUT_FILL_X|LAYOUT_SIDE_LEFT|JUSTIFY_LEFT);
#endif
    new FXCheckButton(otherpane, _("Special tab for server messages"), &serverTarget, FXDataTarget::ID_VALUE, CHECKBUTTON_NORMAL|LAYOUT_FILL_X|LAYOUT_SIDE_LEFT|JUSTIFY_LEFT);
    new FXCheckButton(otherpane, _("Logging chats"), &logTarget, FXDataTarget::ID_VALUE, CHECKBUTTON_NORMAL|LAYOUT_FILL_X|LAYOUT_SIDE_LEFT|JUSTIFY_LEFT);
    FXHorizontalFrame *logpane = new FXHorizontalFrame(otherpane, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(logpane, _("Log path"), NULL, LAYOUT_LEFT);
    (new FXTextField(logpane, 25, &targetLogPath, FXDataTarget::ID_VALUE, TEXTFIELD_READONLY|FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_X))->disable();
    if(!FXStat::exists(logPath)) logPath = FXSystem::getHomeDirectory();
    selectPath = new FXButton(logpane, "...", NULL, this, ID_LOGPATH, FRAME_RAISED|FRAME_THICK);
    if(logging) selectPath->enable();
    else selectPath->disable();
#ifdef HAVE_LUA
    new FXCheckButton(otherpane, _("Automatically load scripts"), &autoloadTarget, FXDataTarget::ID_VALUE, CHECKBUTTON_NORMAL|LAYOUT_FILL_X|LAYOUT_SIDE_LEFT|JUSTIFY_LEFT);
    FXHorizontalFrame *aloadpane = new FXHorizontalFrame(otherpane, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(aloadpane, _("Path"), NULL, LAYOUT_LEFT);
    (new FXTextField(aloadpane, 25, &targetAutoloadPath, FXDataTarget::ID_VALUE, TEXTFIELD_READONLY|FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_X))->disable();
    if(!FXStat::exists(autoloadPath))
    {
#ifdef WIN32
        autoloadPath = utils::LocaleToUtf8(FXSystem::getHomeDirectory()+PATHSEPSTRING+"scripts");
#else
        autoloadPath = FXSystem::getHomeDirectory()+PATHSEPSTRING+".dxirc"+PATHSEPSTRING+"scripts";
#endif
    }
    selectAutoloadPath = new FXButton(aloadpane, "...", NULL, this, ID_AUTOLOADPATH, FRAME_RAISED|FRAME_THICK);
    if(autoload) selectAutoloadPath->enable();
    else selectAutoloadPath->disable();
#endif
    FXHorizontalFrame *tabpane = new FXHorizontalFrame(otherpane, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(tabpane, _("Tab position"), NULL, LAYOUT_LEFT);
    listTabs = new FXListBox(tabpane, this, ID_TABPOS);
    listTabs->appendItem(_("Bottom"));
    listTabs->appendItem(_("Left"));
    listTabs->appendItem(_("Top"));
    listTabs->appendItem(_("Right"));
    listTabs->setNumVisible(4);
    if(tabPosition>=0 && tabPosition<4) listTabs->setCurrentItem(tabPosition);


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

    FXVerticalFrame *dccpane = new FXVerticalFrame(switcher, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(dccpane, _("DCC settings"), NULL, LAYOUT_LEFT);
    new FXHorizontalSeparator(dccpane, SEPARATOR_LINE|LAYOUT_FILL_X);
    new FXLabel(dccpane, _("Directory for saving files:"), NULL, LAYOUT_LEFT);
    FXHorizontalFrame *pathframe = new FXHorizontalFrame(dccpane, LAYOUT_FILL_X);
    (new FXTextField(pathframe, 30, &targetDccPath, FXDataTarget::ID_VALUE, TEXTFIELD_READONLY|FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X))->disable();
    new FXButton(pathframe, "...", NULL, this, ID_DCCPATH, FRAME_RAISED|FRAME_THICK);
    FXHorizontalFrame *ipframe = new FXHorizontalFrame(dccpane, LAYOUT_FILL_X);
    new FXLabel(ipframe, _("DCC IP address:"), NULL, LAYOUT_LEFT);
    new FXTextField(ipframe, 3, &targetDccIP1, FXDataTarget::ID_VALUE, TEXTFIELD_INTEGER|TEXTFIELD_LIMITED|FRAME_SUNKEN|FRAME_THICK);
    new FXLabel(ipframe, ".", NULL, LAYOUT_LEFT);
    new FXTextField(ipframe, 3, &targetDccIP2, FXDataTarget::ID_VALUE, TEXTFIELD_INTEGER|TEXTFIELD_LIMITED|FRAME_SUNKEN|FRAME_THICK);
    new FXLabel(ipframe, ".", NULL, LAYOUT_LEFT);
    new FXTextField(ipframe, 3, &targetDccIP3, FXDataTarget::ID_VALUE, TEXTFIELD_INTEGER|TEXTFIELD_LIMITED|FRAME_SUNKEN|FRAME_THICK);
    new FXLabel(ipframe, ".", NULL, LAYOUT_LEFT);
    new FXTextField(ipframe, 3, &targetDccIP4, FXDataTarget::ID_VALUE, TEXTFIELD_INTEGER|TEXTFIELD_LIMITED|FRAME_SUNKEN|FRAME_THICK);
    new FXLabel(dccpane, _("(Leave blank for use IP address from server)"), NULL, LAYOUT_LEFT);
    new FXSeparator(dccpane, LAYOUT_FILL_X);
    FXHorizontalFrame *portframe = new FXHorizontalFrame(dccpane, LAYOUT_FILL_X);
    new FXLabel(portframe, _("DCC ports:"), NULL, LAYOUT_LEFT);
    FXSpinner *dspinner = new FXSpinner(portframe, 6, &targetDccPortD, FXDataTarget::ID_VALUE, FRAME_SUNKEN|FRAME_THICK);
    dspinner->setRange(0, 65536);
    new FXLabel(portframe, "-");
    FXSpinner *hspinner = new FXSpinner(portframe, 6, &targetDccPortH, FXDataTarget::ID_VALUE, FRAME_SUNKEN|FRAME_THICK);
    hspinner->setRange(0, 65536);
    new FXLabel(dccpane, _("(Set 0 for use ports from OS)"), NULL, LAYOUT_LEFT);
    FXHorizontalFrame *timeframe = new FXHorizontalFrame(dccpane, LAYOUT_FILL_X);
    new FXLabel(timeframe, _("Time for waiting for connection in seconds"), NULL, LAYOUT_LEFT);
    new FXSpinner(timeframe, 4, &targetDccTimeout, FXDataTarget::ID_VALUE, SPIN_NOMAX|SPIN_CYCLIC|FRAME_SUNKEN|FRAME_THICK);
    new FXCheckButton(dccpane, _("Automatically connect offered chat"), &targetAutoDccChat, FXDataTarget::ID_VALUE, CHECKBUTTON_NORMAL|LAYOUT_FILL_X|LAYOUT_SIDE_LEFT|JUSTIFY_LEFT);
    new FXCheckButton(dccpane, _("Automatically receive offered file"), &targetAutoDccFile, FXDataTarget::ID_VALUE, CHECKBUTTON_NORMAL|LAYOUT_FILL_X|LAYOUT_SIDE_LEFT|JUSTIFY_LEFT);

    FXVerticalFrame *soundpane = new FXVerticalFrame(switcher, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(soundpane, _("Sound settings"), NULL, LAYOUT_LEFT);
    new FXHorizontalSeparator(soundpane, SEPARATOR_LINE|LAYOUT_FILL_X);
    new FXCheckButton(soundpane, _("Use sounds"), &targetSound, FXDataTarget::ID_VALUE, CHECKBUTTON_NORMAL|LAYOUT_FILL_X|LAYOUT_SIDE_LEFT|JUSTIFY_LEFT);
    FXGroupBox *eventsgroup = new FXGroupBox(soundpane, _("Events"), FRAME_GROOVE|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_SIDE_TOP);
    FXVerticalFrame *eventsframe = new FXVerticalFrame(eventsgroup, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    checkConnect = new FXCheckButton(eventsframe, _("Friend connected"), &targetSoundConnect, FXDataTarget::ID_VALUE, CHECKBUTTON_NORMAL|LAYOUT_FILL_X|LAYOUT_SIDE_LEFT|JUSTIFY_LEFT);
    if(!sounds)
        checkConnect->disable();
    FXHorizontalFrame *connectframe = new FXHorizontalFrame(eventsframe, LAYOUT_FILL_X|PACK_UNIFORM_HEIGHT);
    (new FXTextField(connectframe, 30, &targetPathConnect, FXDataTarget::ID_VALUE, TEXTFIELD_READONLY|FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X))->disable();
    selectConnect = new FXButton(connectframe, "...", NULL, this, ID_SELECTCONNECT, FRAME_RAISED|FRAME_THICK);
    playConnect = new FXButton(connectframe, _("\tPlay"), playicon, this, ID_PLAYCONNECT, FRAME_RAISED|FRAME_THICK);
    if(!soundConnect || !sounds)
    {
        selectConnect->disable();
        playConnect->disable();
    }
    if(!FXStat::exists(pathConnect))
        playConnect->disable();
    checkDisconnect = new FXCheckButton(eventsframe, _("Friend disconnected"), &targetSoundDisconnect, FXDataTarget::ID_VALUE, CHECKBUTTON_NORMAL|LAYOUT_FILL_X|LAYOUT_SIDE_LEFT|JUSTIFY_LEFT);
    if(!sounds)
        checkDisconnect->disable();
    FXHorizontalFrame *disconnectframe = new FXHorizontalFrame(eventsframe, LAYOUT_FILL_X|PACK_UNIFORM_HEIGHT);
    (new FXTextField(disconnectframe, 30, &targetPathDisconnect, FXDataTarget::ID_VALUE, TEXTFIELD_READONLY|FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X))->disable();
    selectDisconnect = new FXButton(disconnectframe, "...", NULL, this, ID_SELECTDISCONNECT, FRAME_RAISED|FRAME_THICK);
    playDisconnect = new FXButton(disconnectframe, _("\tPlay"), playicon, this, ID_PLAYDISCONNECT, FRAME_RAISED|FRAME_THICK);
    if(!soundDisconnect || !sounds)
    {
        selectDisconnect->disable();
        playDisconnect->disable();
    }
    if(!FXStat::exists(pathDisconnect))
        playDisconnect->disable();
    FXGroupBox *friendsgroup = new FXGroupBox(eventsframe, _("Friends"), FRAME_GROOVE|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    FXVerticalFrame *friendsbuttons = new FXVerticalFrame(friendsgroup, LAYOUT_SIDE_RIGHT|LAYOUT_FILL_Y|PACK_UNIFORM_WIDTH);
    addFriend = new FXButton(friendsbuttons, _("Add"), NULL, this, ID_ADDFRIEND, FRAME_RAISED|FRAME_THICK);
    modifyFriend = new FXButton(friendsbuttons, _("Modify"), NULL, this, ID_MODIFYFRIEND, FRAME_RAISED|FRAME_THICK);
    deleteFriend = new FXButton(friendsbuttons, _("Delete"), NULL, this, ID_DELETEFRIEND, FRAME_RAISED|FRAME_THICK);
    FXVerticalFrame *friendspane = new FXVerticalFrame(friendsgroup, LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN|FRAME_THICK);
    FXHorizontalFrame *friendsframe = new FXHorizontalFrame(friendspane, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    friends = new FXIconList(friendsframe, this, ID_FRIEND, ICONLIST_AUTOSIZE|ICONLIST_DETAILED|ICONLIST_BROWSESELECT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    friends->appendHeader(_("Friend"), NULL, 150);
    friends->appendHeader(_("Channel(s)"), NULL, 150);
    friends->appendHeader(_("Server(s)"), NULL, 150);
    FillFriends();
    if(friendsList.no())
    {
        deleteFriend->enable();
        modifyFriend->enable();
    }
    else
    {
        deleteFriend->disable();
        modifyFriend->disable();
    }
    if(!sounds)
    {
        addFriend->disable();
        deleteFriend->disable();
        modifyFriend->disable();
    }
    checkMessage = new FXCheckButton(eventsframe, _("Highlighted message or query message"), &targetSoundMessage, FXDataTarget::ID_VALUE, CHECKBUTTON_NORMAL|LAYOUT_FILL_X|LAYOUT_SIDE_LEFT|JUSTIFY_LEFT);
    if(!sounds)
        checkMessage->disable();
    FXHorizontalFrame *messageframe = new FXHorizontalFrame(eventsframe, LAYOUT_FILL_X|PACK_UNIFORM_HEIGHT);
    (new FXTextField(messageframe, 30, &targetPathMessage, FXDataTarget::ID_VALUE, TEXTFIELD_READONLY|FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X))->disable();
    selectMessage = new FXButton(messageframe, "...", NULL, this, ID_SELECTMESSAGE, FRAME_RAISED|FRAME_THICK);
    playMessage = new FXButton(messageframe, _("\tPlay"), playicon, this, ID_PLAYMESSAGE, FRAME_RAISED|FRAME_THICK);
    if(!soundMessage || !sounds)
    {
        selectMessage->disable();
        playMessage->disable();
    }
    if(!FXStat::exists(pathMessage))
        playMessage->disable();

    FXVerticalFrame *smileypane = new FXVerticalFrame(switcher, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(smileypane, _("Smileys settings"), NULL, LAYOUT_LEFT);
    new FXHorizontalSeparator(smileypane, SEPARATOR_LINE|LAYOUT_FILL_X);
    new FXCheckButton(smileypane, _("Use smileys"), &targetUseSmileys, FXDataTarget::ID_VALUE, CHECKBUTTON_NORMAL|LAYOUT_FILL_X|LAYOUT_SIDE_LEFT|JUSTIFY_LEFT);
    FXGroupBox *smileysgroup = new FXGroupBox(smileypane, _("Smileys"), FRAME_GROOVE|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    FXVerticalFrame *smileysbuttons = new FXVerticalFrame(smileysgroup, LAYOUT_SIDE_RIGHT|LAYOUT_FILL_Y|PACK_UNIFORM_WIDTH);
    addSmiley = new FXButton(smileysbuttons, _("Add"), NULL, this, ID_ADDSMILEY, FRAME_RAISED|FRAME_THICK);
    modifySmiley = new FXButton(smileysbuttons, _("Modify"), NULL, this, ID_MODIFYSMILEY, FRAME_RAISED|FRAME_THICK);
    deleteSmiley = new FXButton(smileysbuttons, _("Delete"), NULL, this, ID_DELETESMILEY, FRAME_RAISED|FRAME_THICK);
    importSmiley = new FXButton(smileysbuttons, _("Import"), NULL, this, ID_IMPORTSMILEY, FRAME_RAISED|FRAME_THICK);
    exportSmiley = new FXButton(smileysbuttons, _("Export"), NULL, this, ID_EXPORTSMILEY, FRAME_RAISED|FRAME_THICK);
    if(useSmileys)
    {
        addSmiley->enable();
        importSmiley->enable();
        if((FXint)smileysMap.size())
        {
            modifySmiley->enable();
            deleteSmiley->enable();
            exportSmiley->enable();
        }
        else
        {
            modifySmiley->disable();
            deleteSmiley->disable();
            exportSmiley->disable();
        }
    }
    else
    {
        addSmiley->disable();
        modifySmiley->disable();
        deleteSmiley->disable();
        importSmiley->disable();
        exportSmiley->disable();
    }
    FXVerticalFrame *smileyspane = new FXVerticalFrame(smileysgroup, LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN|FRAME_THICK);
    FXHorizontalFrame *smileysframe = new FXHorizontalFrame(smileyspane, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    smileys = new FXList(smileysframe, this, ID_SMILEY, LIST_BROWSESELECT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    FillSmileys();
    
    new FXButton(buttonframe, _("&General"), NULL, switcher, FXSwitcher::ID_OPEN_THIRD, FRAME_RAISED);
    new FXButton(buttonframe, _("&Look"), NULL, switcher, FXSwitcher::ID_OPEN_FOURTH, FRAME_RAISED);
    new FXButton(buttonframe, _("&Irc Text"), NULL, switcher, FXSwitcher::ID_OPEN_FIRST, FRAME_RAISED);
    new FXButton(buttonframe, _("I&gnore"), NULL, switcher, FXSwitcher::ID_OPEN_SECOND, FRAME_RAISED);
    new FXButton(buttonframe, _("&DCC"), NULL, switcher, FXSwitcher::ID_OPEN_FIFTH, FRAME_RAISED);
    new FXButton(buttonframe, _("&Sounds"), NULL, switcher, FXSwitcher::ID_OPEN_SIXTH, FRAME_RAISED);
    new FXButton(buttonframe, _("S&mileys"), NULL, switcher, FXSwitcher::ID_OPEN_SEVENTH, FRAME_RAISED);
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
    return 1;
}

long ConfigDialog::OnFriendsSelected(FXObject*, FXSelector, void *ptr)
{
    FXint i = (FXint)(FXival)ptr;
    friends->selectItem(i);
    deleteFriend->enable();
    return 1;
}

long ConfigDialog::OnFriendsDeselected(FXObject*, FXSelector, void*)
{
    deleteFriend->disable();
    return 1;
}

long ConfigDialog::OnFriendsChanged(FXObject*, FXSelector, void *ptr)
{
    FXint i = (FXint)(FXival)ptr;
    friends->selectItem(i);
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

long ConfigDialog::OnTabPosition(FX::FXObject *, FX::FXSelector, void *ptr)
{
    tabPosition = (FXint)(FXival)ptr;
    return 1;
}

long ConfigDialog::OnAccept(FXObject*, FXSelector, void*)
{
    getApp()->stopModal(this,TRUE);
    SaveConfig();
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

    FXHorizontalFrame *buttonframe = new FXHorizontalFrame(contents,LAYOUT_FILL_X|LAYOUT_FILL_Y|PACK_UNIFORM_WIDTH);
    new FXButton(buttonframe, _("&Cancel"), NULL, &dialog, FXDialogBox::ID_CANCEL, FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,5);
    new FXButton(buttonframe, _("&OK"), NULL, &dialog, FXDialogBox::ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,5);

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

    FXHorizontalFrame *buttonframe = new FXHorizontalFrame(contents,LAYOUT_FILL_X|LAYOUT_FILL_Y|PACK_UNIFORM_WIDTH);
    new FXButton(buttonframe, _("&Cancel"), NULL, &dialog, FXDialogBox::ID_CANCEL, FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,5);
    new FXButton(buttonframe, _("&OK"), NULL, &dialog, FXDialogBox::ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,5);

    if(dialog.execute(PLACEMENT_CURSOR))
    {
        if(!nick->getText().empty() && !NickExist(nick->getText()))
        {
            IgnoreUser user;
            user.nick = nick->getText();
            channel->getText().empty() ? user.channel = "all" : user.channel = channel->getText();
            server->getText().empty() ? user.server = "all" : user.server = server->getText();
            usersList.append(user);
            users->appendItem(user.nick+"\t"+user.channel+"\t"+user.server);
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

    FXHorizontalFrame *buttonframe = new FXHorizontalFrame(contents,LAYOUT_FILL_X|LAYOUT_FILL_Y|PACK_UNIFORM_WIDTH);
    new FXButton(buttonframe, _("&Cancel"), NULL, &dialog, FXDialogBox::ID_CANCEL, FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,5);
    new FXButton(buttonframe, _("&OK"), NULL, &dialog, FXDialogBox::ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,5);

    if(dialog.execute(PLACEMENT_CURSOR))
    {
        if(!nick->getText().empty() && (!NickExist(nick->getText()) || oldnick == nick->getText()))
        {
            usersList[i].nick = nick->getText();
            channel->getText().empty() ? usersList[i].channel = "all" : usersList[i].channel = channel->getText();
            server->getText().empty() ? usersList[i].server = "all" : usersList[i].server = server->getText();
            users->setItemText(i, usersList[i].nick+"\t"+usersList[i].channel+"\t"+usersList[i].server);
        }
    }
    return 1;
}

long ConfigDialog::OnDeleteUser(FXObject*, FXSelector, void*)
{
    FXint i = users->getCurrentItem();
    users->removeItem(i);
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

long ConfigDialog::OnImportSmiley(FXObject*, FXSelector, void*)
{
    if(showImportwarning && smileysMap.size())
        if(FXMessageBox::warning(this, MBOX_OK_CANCEL, _("Warning"), _("File import overwrites current settings"))==4) {showImportwarning=FALSE; return 1;}
    FXFileDialog dialog(this, _("Select file"));
    if(showImportwarning) dialog.setFilename((FXString)DXIRC_DATADIR+PATHSEPSTRING+"icons"+PATHSEPSTRING+"smileys"+PATHSEPSTRING+"dxirc.smiley");
    if(dialog.execute())
    {
        smileys->clearItems();
        smileysMap.clear();
        FXFile file(dialog.getFilename(), FXIO::Reading);
        if(file.isOpen())
        {
            FXchar line[MAXBUFFER];
            FXint bol,eol,end,path,smiley,p,lineno;
            FXString pathstr, smileystr;
            lineno=bol=eol=end=0;
            // Read lines
            while(readString(file,line,bol,eol,end))
            {
                lineno++;
                // Skip leading spaces
                while(bol<eol && Ascii::isBlank(line[bol])) bol++;
                // Skip comment lines and empty lines
                if(bol>=eol || line[bol]=='#' || line[bol]==';' || line[bol]=='\n' || line[bol]=='\r') goto next;
                // Scan key path
                for(path=bol; bol<eol && line[bol]!='=' && !Ascii::isControl(line[bol]); bol++);
                // Check errors
                if(bol>=eol || line[bol]!='='){ fxwarning("%s:%d: expected '=' to follow key.\n",dialog.getFilename().text(),lineno); goto next; }
                // Remove trailing spaces after path
                for(p=bol; path<p && Ascii::isBlank(line[p-1]); p--);
                // Terminate path
                line[p]='\0';
                // Skip leading spaces
                for(bol++; bol<eol && Ascii::isBlank(line[bol]); bol++);
                // Scan smiley
                for(smiley=bol; bol<eol && !Ascii::isControl(line[bol]); bol++);
                // Remove trailing spaces after smiley
                for(p=bol; smiley<p && Ascii::isBlank(line[p-1]); p--);
                // Terminate smiley
                line[p]='\0';
                //Fill smiley map
                pathstr = Dequote(line+path);
                if(pathstr.empty()) goto next;
                smileystr = Dequote(line+smiley);
                if(smileystr.empty()) goto next;
                pathstr = FXPath::absolute(FXPath::directory(dialog.getFilename()), pathstr);
                smileysMap.insert(StringPair(smileystr,pathstr));

next:           bol=eol;
            }
        }
        FillSmileys();
        if(smileysMap.size())
        {
            modifySmiley->enable();
            deleteSmiley->enable();
            exportSmiley->enable();
        }
    }
    showImportwarning=FALSE;    
    return 1;
}

long ConfigDialog::OnExportSmiley(FXObject*, FXSelector, void*)
{
    FXFileDialog dialog(this, _("Save smiley settings as"));
    if(dialog.execute())
    {
        FXFile file(dialog.getFilename(), FXIO::Writing);
        FXchar line[MAXVALUE];
        if(file.isOpen())
        {
            writeString(file, HEADER);
            if((FXint)smileysMap.size())
            {
                StringIt it;
                FXint i;
                for(i=0, it=smileysMap.begin(); it!=smileysMap.end(); it++,i++)
                {
                    writeString(file, Enquote(line, FXPath::relative(FXPath::directory(dialog.getFilename()),(*it).second).text()));
                    writeString(file, "=");
                    writeString(file, Enquote(line, (*it).first.text()));
                    writeString(file, ENDLINE);
                }
            }
        }
    }
    return 1;
}

// Enquote a value
FXchar* ConfigDialog::Enquote(FXchar* result, const FXchar* text)
{
    register FXchar *end = result + MAXVALUE - 6;
    register FXchar *ptr = result;
    register FXuchar c;
    if (needquotes(text))
    {
        *ptr++ = '"';
        while((c = *text++) != '\0' && ptr < end)
        {
            switch (c) {
                case '\n':
                    *ptr++ = '\\';
                    *ptr++ = 'n';
                    break;
                case '\r':
                    *ptr++ = '\\';
                            *ptr++ = 'r';
                    break;
                case '\b':
                    *ptr++ = '\\';
                            *ptr++ = 'b';
                    break;
                case '\v':
                    *ptr++ = '\\';
                    *ptr++ = 'v';
                    break;
                case '\a':
                    *ptr++ = '\\';
                    *ptr++ = 'a';
                    break;
                case '\f':
                    *ptr++ = '\\';
                    *ptr++ = 'f';
                    break;
                case '\t':
                    *ptr++ = '\\';
                    *ptr++ = 't';
                    break;
                case '\\':
                    *ptr++ = '\\';
                    *ptr++ = '\\';
                    break;
                case '"':
                    *ptr++ = '\\';
                    *ptr++ = '"';
                    break;
                case '\'':
                    *ptr++ = '\\';
                    *ptr++ = '\'';
                    break;
                default:
                    if(c < 0x20 || 0x7f < c)
                    {
                        *ptr++ = '\\';
                        *ptr++ = 'x';
                        *ptr++ = FXString::HEX[c >> 4];
                        *ptr++ = FXString::HEX[c & 15];
                    }
                    else
                    {
                        *ptr++ = c;
                    }
                    break;
            }
        }
        *ptr++ = '"';
    }
    else
    {
        while ((c = *text++) != '\0' && ptr < end)
        {
            *ptr++ = c;
        }
    }
    *ptr = '\0';
    return result;
}

// Dequote a value, in situ
FXchar* ConfigDialog::Dequote(FXchar* text) const
{
    register FXchar *result = text;
    register FXchar *ptr = text;
    register FXuint v;
    if(*text == '"')
    {
        text++;
        while((v = *text++) != '\0' && v != '\n' && v != '"')
        {
            if(v == '\\')
            {
                v = *text++;
                switch (v) {
                    case 'n':
                        v = '\n';
                        break;
                    case 'r':
                        v = '\r';
                        break;
                    case 'b':
                        v = '\b';
                        break;
                    case 'v':
                        v = '\v';
                        break;
                    case 'a':
                        v = '\a';
                        break;
                    case 'f':
                        v = '\f';
                        break;
                    case 't':
                        v = '\t';
                        break;
                    case '\\':
                        v = '\\';
                        break;
                    case '"':
                        v = '"';
                        break;
                    case '\'':
                        v = '\'';
                        break;
                    case 'x':
                        v = 'x';
                        if(Ascii::isHexDigit(*text))
                        {
                            v = Ascii::digitValue(*text++);
                            if(Ascii::isHexDigit(*text))
                            {
                                v = (v << 4) + Ascii::digitValue(*text++);
                            }
                        }
                        break;
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                        v = v - '0';
                        if('0' <= *text && *text <= '7')
                        {
                            v = (v << 3)+*text++-'0';
                            if('0' <= *text && *text <= '7')
                            {
                                v = (v << 3)+*text++-'0';
                            }
                        }
                        break;
                }
            }
            *ptr++ = v;
        }
        *ptr = '\0';
    }
    return result;
}

long ConfigDialog::OnAddFriend(FXObject*, FXSelector, void*)
{
    FXDialogBox dialog(this, _("Add friend"), DECOR_TITLE|DECOR_BORDER, 0,0,0,0, 0,0,0,0, 0,0);
    FXVerticalFrame *contents = new FXVerticalFrame(&dialog, LAYOUT_SIDE_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 10,10,10,10, 0,0);
    FXMatrix *matrix = new FXMatrix(contents,2,MATRIX_BY_COLUMNS|LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y);

    new FXLabel(matrix, _("Nick:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *nick = new FXTextField(matrix, 25, NULL, 0, FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    nick->setText("example");
    new FXLabel(matrix, _("Channel(s):\tChannels need to be comma separated"), NULL,JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *channel = new FXTextField(matrix, 25, NULL, 0, FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    channel->setText("all");
    channel->setTipText(_("Channels need to be comma separated"));
    new FXLabel(matrix, _("Server:"), NULL,JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *server = new FXTextField(matrix, 25, NULL, 0, FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    server->setText("all");

    FXHorizontalFrame *buttonframe = new FXHorizontalFrame(contents,LAYOUT_FILL_X|LAYOUT_FILL_Y|PACK_UNIFORM_WIDTH);
    new FXButton(buttonframe, _("&Cancel"), NULL, &dialog, FXDialogBox::ID_CANCEL, FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,5);
    new FXButton(buttonframe, _("&OK"), NULL, &dialog, FXDialogBox::ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,5);

    if(dialog.execute(PLACEMENT_CURSOR))
    {
        if(!nick->getText().empty() && !NickExist(nick->getText(), FALSE))
        {
            IgnoreUser user;
            user.nick = nick->getText();
            channel->getText().empty() ? user.channel = "all" : user.channel = channel->getText();
            server->getText().empty() ? user.server = "all" : user.server = server->getText();
            friendsList.append(user);
            friends->appendItem(user.nick+"\t"+user.channel+"\t"+user.server);
            if(!deleteFriend->isEnabled()) deleteFriend->enable();
            if(!modifyFriend->isEnabled()) modifyFriend->enable();
        }
    }
    return 1;
}

long ConfigDialog::OnModifyFriend(FXObject*, FXSelector, void*)
{
    FXint i = friends->getCurrentItem();
    FXString oldnick = friendsList[i].nick;

    FXDialogBox dialog(this, _("Modify friend"), DECOR_TITLE|DECOR_BORDER, 0,0,0,0, 0,0,0,0, 0,0);
    FXVerticalFrame *contents = new FXVerticalFrame(&dialog, LAYOUT_SIDE_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 10,10,10,10, 0,0);
    FXMatrix *matrix = new FXMatrix(contents,2,MATRIX_BY_COLUMNS|LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y);

    new FXLabel(matrix, _("Nick:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *nick = new FXTextField(matrix, 25, NULL, 0, FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    nick->setText(oldnick);
    new FXLabel(matrix, _("Channel(s):\tChannels need to be comma separated"), NULL,JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *channel = new FXTextField(matrix, 25, NULL, 0, FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    channel->setText(friendsList[i].channel);
    new FXLabel(matrix, _("Server:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *server = new FXTextField(matrix, 25, NULL, 0, FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    server->setText(friendsList[i].server);

    FXHorizontalFrame *buttonframe = new FXHorizontalFrame(contents,LAYOUT_FILL_X|LAYOUT_FILL_Y|PACK_UNIFORM_WIDTH);
    new FXButton(buttonframe, _("&Cancel"), NULL, &dialog, FXDialogBox::ID_CANCEL, FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,5);
    new FXButton(buttonframe, _("&OK"), NULL, &dialog, FXDialogBox::ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,5);

    if(dialog.execute(PLACEMENT_CURSOR))
    {
        if(!nick->getText().empty() && (!NickExist(nick->getText(), FALSE) || oldnick == nick->getText()))
        {
            friendsList[i].nick = nick->getText();
            channel->getText().empty() ? friendsList[i].channel = "all" : friendsList[i].channel = channel->getText();
            server->getText().empty() ? friendsList[i].server = "all" : friendsList[i].server = server->getText();
            friends->setItemText(i, friendsList[i].nick+"\t"+friendsList[i].channel+"\t"+friendsList[i].server);
        }
    }
    return 1;
}

long ConfigDialog::OnDeleteFriend(FXObject*, FXSelector, void*)
{
    FXint i = friends->getCurrentItem();
    friends->removeItem(i);
    friendsList.erase(i);
    if(friendsList.no())
    {
        deleteFriend->enable();
        modifyFriend->enable();
    }
    else
    {
        deleteFriend->disable();
        modifyFriend->disable();
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
    i = icons->getCurrentItem();
    themePath = icons->getItemText(i);
    icon1->setIcon(MakeIcon(getApp(), themePath, "irc_admin.png", TRUE));
    icon2->setIcon(MakeIcon(getApp(), themePath, "irc_owner.png", TRUE));
    icon3->setIcon(MakeIcon(getApp(), themePath, "irc_op.png", TRUE));
    icon4->setIcon(MakeIcon(getApp(), themePath, "irc_halfop.png", TRUE));
    icon5->setIcon(MakeIcon(getApp(), themePath, "irc_voice.png", TRUE));
    icon6->setIcon(MakeIcon(getApp(), themePath, "irc_normal.png", TRUE));
    icon7->setIcon(MakeAwayIcon(getApp(), themePath, "irc_normal.png"));
    icons->getNumItems()>1 ? deleteTheme->enable() : deleteTheme->disable();
    UpdateIcons();
    return 1;
}

long ConfigDialog::OnIconsChanged(FXObject*, FXSelector, void *ptr)
{
    FXint i = (FXint)(FXival)ptr;
    themePath = icons->getItemText(i);
    icon1->setIcon(MakeIcon(getApp(), themePath, "irc_admin.png", TRUE));
    icon2->setIcon(MakeIcon(getApp(), themePath, "irc_owner.png", TRUE));
    icon3->setIcon(MakeIcon(getApp(), themePath, "irc_op.png", TRUE));
    icon4->setIcon(MakeIcon(getApp(), themePath, "irc_halfop.png", TRUE));
    icon5->setIcon(MakeIcon(getApp(), themePath, "irc_voice.png", TRUE));
    icon6->setIcon(MakeIcon(getApp(), themePath, "irc_normal.png", TRUE));
    icon7->setIcon(MakeAwayIcon(getApp(), themePath, "irc_normal.png"));
    ShowMessage();
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
    for(int i=0; i<3; i++)
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

long ConfigDialog::OnAutoloadChanged(FXObject*, FXSelector, void*)
{
    if(autoload) selectAutoloadPath->enable();
    else selectAutoloadPath->disable();
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
    dirdialog.setDirectory(logPath);
    if(dirdialog.execute(PLACEMENT_CURSOR))
    {
        logPath = dirdialog.getDirectory();
    }
    return 1;
}

long ConfigDialog::OnAutoloadPathSelect(FXObject*, FXSelector, void*)
{
    FXDirDialog dirdialog(this,_("Select autoload directory"));
    dirdialog.setDirectory(autoloadPath);
    if(dirdialog.execute(PLACEMENT_CURSOR))
    {
        autoloadPath = dirdialog.getDirectory();
    }
    return 1;
}

long ConfigDialog::OnDccPathSelect(FXObject*, FXSelector, void*)
{
    FXDirDialog dirdialog(this,_("Select directory"));
    dirdialog.setDirectory(dccPath);
    if(dirdialog.execute(PLACEMENT_CURSOR))
    {
        dccPath = dirdialog.getDirectory();
    }
    return 1;
}

long ConfigDialog::OnDccPortD(FXObject*, FXSelector, void*)
{
    if(dccPortD<0 || dccPortD>65536) dccPortD = 0;
    if(dccPortH<dccPortD) dccPortH = dccPortD;
    return 1;
}

long ConfigDialog::OnDccPortH(FXObject*, FXSelector, void*)
{
    if(dccPortH<0 || dccPortH>65536) dccPortH = 0;
    if(dccPortD>dccPortH) dccPortD = dccPortH;
    return 1;
}

long ConfigDialog::OnSounds(FXObject*, FXSelector, void*)
{
    if(sounds)
    {
        checkConnect->enable();
        selectConnect->enable();
        if(FXStat::exists(pathConnect)) playConnect->enable();
        checkDisconnect->enable();
        selectDisconnect->enable();
        if(FXStat::exists(pathDisconnect)) playDisconnect->enable();
        checkMessage->enable();
        selectMessage->enable();
        if(FXStat::exists(pathMessage)) playMessage->enable();
        addFriend->enable();
        if(friendsList.no())
        {
            deleteFriend->enable();
            modifyFriend->enable();
        }
    }
    else
    {
        checkConnect->disable();
        selectConnect->disable();
        playConnect->disable();
        checkDisconnect->disable();
        selectDisconnect->disable();
        playDisconnect->disable();
        checkMessage->disable();
        selectMessage->disable();
        playMessage->disable();
        addFriend->disable();
        modifyFriend->disable();
        deleteFriend->disable();
    }
    return 1;
}

long ConfigDialog::OnSoundConnect(FXObject*, FXSelector, void*)
{
    if(soundConnect)
    {
        selectConnect->enable();
        if(FXStat::exists(pathConnect)) playConnect->enable();
    }
    else
    {
        selectConnect->disable();
        playConnect->enable();
    }
    return 1;
}

long ConfigDialog::OnSoundDisconnect(FXObject*, FXSelector, void*)
{
    if(soundDisconnect)
    {
        selectDisconnect->enable();
        if(FXStat::exists(pathDisconnect)) playDisconnect->enable();
    }
    else
    {
        selectDisconnect->disable();
        playDisconnect->enable();
    }
    return 1;
}

long ConfigDialog::OnSoundMessage(FXObject*, FXSelector, void*)
{
    if(soundMessage)
    {
        selectMessage->enable();
        if(FXStat::exists(pathMessage)) playMessage->enable();
    }
    else
    {
        selectMessage->disable();
        playMessage->enable();
    }
    return 1;
}

long ConfigDialog::OnPlay(FXObject*, FXSelector sel, void*)
{
    switch(FXSELID(sel)) {
        case ID_PLAYCONNECT:
            utils::PlayFile(pathConnect);
            return 1;
        case ID_PLAYDISCONNECT:
            utils::PlayFile(pathDisconnect);
            return 1;
        case ID_PLAYMESSAGE:
            utils::PlayFile(pathMessage);
            return 1;
    }
    return 1;
}

long ConfigDialog::OnSelectPath(FXObject*, FXSelector sel, void*)
{
    FXFileDialog file(this, _("Select file"));
    file.setPatternList(_("Sound file (*.wav)"));
    switch(FXSELID(sel)) {
        case ID_SELECTCONNECT:
            if(file.execute(PLACEMENT_CURSOR))
            {
                pathConnect = file.getFilename();
                playConnect->enable();
            }
            return 1;
        case ID_SELECTDISCONNECT:
            if(file.execute(PLACEMENT_CURSOR))
            {
                pathDisconnect = file.getFilename();
                playDisconnect->enable();
            }
            return 1;
        case ID_SELECTMESSAGE:
            if(file.execute(PLACEMENT_CURSOR))
            {
                pathMessage = file.getFilename();
                playMessage->enable();
            }
            return 1;
    }
    return 1;
}

long ConfigDialog::OnUseSmileys(FXObject*, FXSelector, void*)
{
    if(useSmileys)
    {
        addSmiley->enable();
        importSmiley->enable();
        if((FXint)smileysMap.size())
        {
            modifySmiley->enable();
            deleteSmiley->enable();
            exportSmiley->enable();
        }
        else
        {
            modifySmiley->disable();
            deleteSmiley->disable();
            exportSmiley->disable();
        }
    }
    else
    {
        addSmiley->disable();
        modifySmiley->disable();
        deleteSmiley->disable();
        importSmiley->disable();
        exportSmiley->disable();
    }
    return 1;
}

long ConfigDialog::OnAddSmiley(FXObject*, FXSelector, void*)
{
    if((FXint)smileysMap.size()>=256)
    {
        FXMessageBox::information(this, MBOX_OK, _("Information"), _("Maximum number of 256 smileys is reached"));
        return 1;
    }
    SmileyDialog dialog(this, _("Add smiley"), "", "");
    if(dialog.execute())
    {
        if(!dialog.IconExist())
        {
            FXMessageBox::information(this, MBOX_OK, _("Information"), _("Icon file '%s' doesn't exist or isn't image file"), dialog.GetPath().text());
            return 1;
        }
        if(dialog.GetSmiley().empty())
        {
            FXMessageBox::information(this, MBOX_OK, _("Information"), _("Smiley text is empty"));
            return 1;
        }
        if(SmileyExist(dialog.GetSmiley()))
        {
            FXMessageBox::information(this, MBOX_OK, _("Information"), _("Smiley '%s' already exist"), dialog.GetSmiley().text());
            return 1;
        }
        smileysMap.insert(StringPair(dialog.GetSmiley(), dialog.GetPath()));
        smileys->appendItem(new FXListItem(dialog.GetSmiley(), dialog.GetIcon()));
        modifySmiley->enable();
        deleteSmiley->enable();
        exportSmiley->enable();
    }
    return 1;
}

long ConfigDialog::OnModifySmiley(FXObject*, FXSelector, void*)
{
    FXint index = smileys->getCurrentItem();
    FXString oldkey = smileys->getItemText(index);
    SmileyDialog dialog(this, _("Modify smiley"), oldkey, smileysMap[oldkey]);
    if(dialog.execute())
    {
        if(!dialog.IconExist())
        {
            FXMessageBox::information(this, MBOX_OK, _("Information"), _("Icon file '%s' doesn't exist or isn't image file"), dialog.GetPath().text());
            return 1;
        }
        if(dialog.GetSmiley().empty())
        {
            FXMessageBox::information(this, MBOX_OK, _("Information"), _("Smiley text is empty"));
            return 1;
        }
        if(SmileyExist(dialog.GetSmiley()) && dialog.GetSmiley()!=oldkey)
        {
            FXMessageBox::information(this, MBOX_OK, _("Information"), _("Smiley '%s' already exist"), dialog.GetSmiley().text());
            return 1;
        }
        smileysMap.erase(oldkey);
        smileys->removeItem(index, TRUE);
        smileysMap.insert(StringPair(dialog.GetSmiley(), dialog.GetPath()));
        smileys->insertItem(index, new FXListItem(dialog.GetSmiley(), dialog.GetIcon()), TRUE);
        smileys->setCurrentItem(index, TRUE);
    }
    return 1;
}

long ConfigDialog::OnDeleteSmiley(FXObject*, FXSelector, void*)
{
    smileysMap.erase(smileys->getItemText(smileys->getCurrentItem()));
    smileys->removeItem(smileys->getCurrentItem(), TRUE);
    if(!(FXint)smileysMap.size())
    {
        modifySmiley->disable();
        deleteSmiley->disable();
        exportSmiley->disable();
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
        FXbool exist = FALSE;
        for(FXint j=0; j<commands->getNumItems(); j++)
        {
            if(commands->getItemText(j) == available[i])
            {
                exist = TRUE;
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
            users->appendItem(usersList[i].nick+"\t"+usersList[i].channel+"\t"+usersList[i].server);
        }
    }
}

void ConfigDialog::FillFriends()
{
    if(friendsList.no())
    {
        for(FXint i=0; i<friendsList.no(); i++)
        {
            friends->appendItem(friendsList[i].nick+"\t"+friendsList[i].channel+"\t"+friendsList[i].server);
        }
    }
}

void ConfigDialog::FillSmileys()
{
    if((FXint)smileysMap.size())
    {
        StringIt it;
        for(it=smileysMap.begin(); it!=smileysMap.end(); it++)
        {
            smileys->appendItem(new FXListItem((*it).first, createIconFromName(getApp(), (*it).second)));
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
    if(showWarning)
    {
        FXMessageBox::information(this, FX::MBOX_OK, _("Information"), _("Some changes need restart application"));
        showWarning = FALSE;
    }
}

FXbool ConfigDialog::ThemeExist(const FXString &ckdTheme)
{
    for(FXint i=0; i<icons->getNumItems(); i++)
    {
        if(icons->getItemText(i) == ckdTheme)
        {
            return TRUE;
        }
    }
    return FALSE;
}

FXbool ConfigDialog::NickExist(const FXString &ckdNick, FXbool user)
{
    for(FXint i=0; i<(user ? users->getNumItems() : friends->getNumItems()); i++)
    {
        if((user ? users->getItemText(i) : friends->getItemText(i)) == ckdNick) return TRUE;
    }
    return FALSE;
}

FXbool ConfigDialog::SmileyExist(const FXString& ckdSmiley)
{
    StringIt it;
    for(it=smileysMap.begin(); it!=smileysMap.end(); it++)
    {
        if(!compare((*it).first, ckdSmiley)) return TRUE;
    }
    return FALSE;
}

void ConfigDialog::ReadConfig()
{
    FXString ircfontspec;
    FXSettings set;
    set.parseFile(utils::GetIniFile(), TRUE);
    themeCurrent.base = set.readColorEntry("SETTINGS", "basecolor", getApp()->getBaseColor());
    themeCurrent.back = set.readColorEntry("SETTINGS", "backcolor", getApp()->getBackColor());
    themeCurrent.border = set.readColorEntry("SETTINGS", "bordercolor", getApp()->getBorderColor());
    themeCurrent.fore = set.readColorEntry("SETTINGS", "forecolor", getApp()->getForeColor());
    themeCurrent.menuback = set.readColorEntry("SETTINGS", "selmenubackcolor", getApp()->getSelMenuBackColor());
    themeCurrent.menufore = set.readColorEntry("SETTINGS", "selmenutextcolor", getApp()->getSelMenuTextColor());
    themeCurrent.selback = set.readColorEntry("SETTINGS", "selbackcolor", getApp()->getSelbackColor());
    themeCurrent.selfore = set.readColorEntry("SETTINGS", "selforecolor", getApp()->getSelforeColor());
    themeCurrent.tipback = set.readColorEntry("SETTINGS", "tipbackcolor", getApp()->getTipbackColor());
    themeCurrent.tipfore = set.readColorEntry("SETTINGS", "tipforecolor", getApp()->getTipforeColor());
    themeCurrent.hilite = set.readColorEntry("SETTINGS", "hilitecolor", getApp()->getHiliteColor());
    themeCurrent.shadow = set.readColorEntry("SETTINGS", "shadowcolor", getApp()->getShadowColor());
    usersShown = set.readBoolEntry("SETTINGS", "usersShown", TRUE);
    statusShown = set.readBoolEntry("SETTINGS", "statusShown", TRUE);
    tabPosition = set.readIntEntry("SETTINGS", "tabPosition", 0);
    commandsList = set.readStringEntry("SETTINGS", "commandsList");
    themePath = utils::CheckThemePath(set.readStringEntry("SETTINGS", "themePath", DXIRC_DATADIR PATHSEPSTRING "icons" PATHSEPSTRING "default"));
    themesList = utils::CheckThemesList(set.readStringEntry("SETTINGS", "themesList", FXString(themePath+";").text()));
    colors.text = set.readColorEntry("SETTINGS", "textColor", FXRGB(255,255,255));
    colors.back = set.readColorEntry("SETTINGS", "textBackColor", FXRGB(0,0,0));
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
    serverWindow = set.readBoolEntry("SETTINGS", "serverWindow", TRUE);
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
    nickChar = FXString(set.readStringEntry("SETTINGS", "nickCompletionChar", ":")).left(1);
    logPath = set.readStringEntry("SETTINGS", "logPath");
    if(logging && !FXStat::exists(logPath)) logging = FALSE;
    dccPath = set.readStringEntry("SETTINGS", "dccPath");
    if(!FXStat::exists(dccPath)) dccPath = FXSystem::getHomeDirectory();
    autoDccChat = set.readBoolEntry("SETTINGS", "autoDccChat", FALSE);
    autoDccFile = set.readBoolEntry("SETTINGS", "autoDccFile", FALSE);
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
    FXint friendsNum = set.readIntEntry("FRIENDS", "number", 0);
    if(friendsNum)
    {

        for(FXint i=0; i<friendsNum; i++)
        {
            IgnoreUser user;
            user.nick = set.readStringEntry(FXStringFormat("FRIEND%d", i).text(), "nick", FXStringFormat("xxx%d", i).text());
            user.channel = set.readStringEntry(FXStringFormat("FRIEND%d", i).text(), "channel", "");
            user.server = set.readStringEntry(FXStringFormat("FRIEND%d", i).text(), "server", "");
            friendsList.append(user);
        }
    }
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
            serverList.append(server);
        }
    }
#ifdef HAVE_LUA
    autoload = set.readBoolEntry("SETTINGS", "autoload", FALSE);
#else
    autoload = FALSE;
#endif
    autoloadPath = set.readStringEntry("SETTINGS", "autoloadPath");
    if(autoload && !FXStat::exists(utils::IsUtf8(autoloadPath.text(), autoloadPath.length()) ? autoloadPath : utils::LocaleToUtf8(autoloadPath))) autoload = FALSE;
    FXString dccIP = set.readStringEntry("SETTINGS", "dccIP");
    FXRex rex("\\l");
    if(dccIP.empty() || dccIP.contains('.')!=3 || rex.match(dccIP))
    {
        dccIP1 = "";
        dccIP2 = "";
        dccIP3 = "";
        dccIP4 = "";
    }
    else
    {
        dccIP1 = dccIP.section('.',0);
        dccIP2 = dccIP.section('.',1);
        dccIP3 = dccIP.section('.',2);
        dccIP4 = dccIP.section('.',3);
    }
    dccPortD = set.readIntEntry("SETTINGS", "dccPortD");
    if(dccPortD<0 || dccPortD>65536) dccPortD = 0;
    dccPortH = set.readIntEntry("SETTINGS", "dccPortH");
    if(dccPortH<0 || dccPortH>65536) dccPortH = 0;
    if(dccPortH<dccPortD) dccPortH = dccPortD;
    dccTimeout = set.readIntEntry("SETTINGS", "dccTimeout", 66);
    sounds = set.readBoolEntry("SETTINGS", "sounds", FALSE);
    soundConnect = set.readBoolEntry("SETTINGS", "soundConnect", FALSE);
    soundDisconnect = set.readBoolEntry("SETTINGS", "soundDisconnect", FALSE);
    soundMessage = set.readBoolEntry("SETTINGS", "soundMessage", FALSE);
    pathConnect = set.readStringEntry("SETTINGS", "pathConnect", DXIRC_DATADIR PATHSEPSTRING "sounds" PATHSEPSTRING "connected.wav");
    pathDisconnect = set.readStringEntry("SETTINGS", "pathDisconnect", DXIRC_DATADIR PATHSEPSTRING "sounds" PATHSEPSTRING "disconnected.wav");
    pathMessage = set.readStringEntry("SETTINGS", "pathMessage", DXIRC_DATADIR PATHSEPSTRING "sounds" PATHSEPSTRING "message.wav");
    stripColors = set.readBoolEntry("SETTINGS", "stripColors", TRUE);
    useSmileys = set.readBoolEntry("SETTINGS", "useSmileys", FALSE);
    FXint smileysNum = set.readIntEntry("SMILEYS", "number", 0);
    if(smileysNum)
    {

        for(FXint i=0; i<smileysNum; i++)
        {
            FXString key, value;
            key = set.readStringEntry("SMILEYS", FXStringFormat("smiley%d", i).text(), FXStringFormat("%d)", i).text());
            value = set.readStringEntry("SMILEYS", FXStringFormat("path%d", i).text(), "");
            if(!key.empty())
                smileysMap.insert(StringPair(key, value));
        }
    }
}

void ConfigDialog::SaveConfig()
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
    set.writeBoolEntry("SETTINGS", "serverWindow", serverWindow);
    set.writeStringEntry("SETTINGS", "logPath", logPath.text());
    set.writeStringEntry("SETTINGS", "dccPath", dccPath.text());
    set.writeStringEntry("SETTINGS", "nickCompletionChar", nickChar.text());
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
    set.writeIntEntry("FRIENDS", "number", friendsList.no());
    if(friendsList.no())
    {

        for(FXint i=0; i<friendsList.no(); i++)
        {
            set.writeStringEntry(FXStringFormat("FRIEND%d", i).text(), "nick", friendsList[i].nick.text());
            set.writeStringEntry(FXStringFormat("FRIEND%d", i).text(), "channel", friendsList[i].channel.text());
            set.writeStringEntry(FXStringFormat("FRIEND%d", i).text(), "server", friendsList[i].server.text());
        }
    }
    set.writeIntEntry("SETTINGS","x", owner->getX());
    set.writeIntEntry("SETTINGS","y", owner->getY());
    set.writeIntEntry("SETTINGS","w", owner->getWidth());
    set.writeIntEntry("SETTINGS","h", owner->getHeight());
    set.writeIntEntry("SETTINGS", "tabPosition", tabPosition);
    set.writeColorEntry("SETTINGS", "basecolor", themeCurrent.base);
    set.writeColorEntry("SETTINGS", "bordercolor", themeCurrent.border);
    set.writeColorEntry("SETTINGS", "backcolor", themeCurrent.back);
    set.writeColorEntry("SETTINGS", "forecolor", themeCurrent.fore);
    set.writeColorEntry("SETTINGS", "hilitecolor", themeCurrent.hilite);
    set.writeColorEntry("SETTINGS", "shadowcolor", themeCurrent.shadow);
    set.writeColorEntry("SETTINGS", "selforecolor", themeCurrent.selfore);
    set.writeColorEntry("SETTINGS", "selbackcolor", themeCurrent.selback);
    set.writeColorEntry("SETTINGS", "tipforecolor", themeCurrent.tipfore);
    set.writeColorEntry("SETTINGS", "tipbackcolor", themeCurrent.tipback);
    set.writeColorEntry("SETTINGS", "selmenutextcolor", themeCurrent.menufore);
    set.writeColorEntry("SETTINGS", "selmenubackcolor", themeCurrent.menuback);
    set.writeStringEntry("SETTINGS", "normalfont", font->getFont().text());
    dxStringMap aliases = utils::GetAliases();
    set.writeIntEntry("ALIASES", "number", (FXint)aliases.size());
    if((FXint)aliases.size())
    {
        StringIt it;
        FXint i;
        for(i=0, it=aliases.begin(); it!=aliases.end(); it++,i++)
        {
            set.writeStringEntry("ALIASES", FXStringFormat("key%d", i).text(), (*it).first.text());
            set.writeStringEntry("ALIASES", FXStringFormat("value%d", i).text(), (*it).second.text());
        }
    }
    set.writeBoolEntry("SETTINGS", "autoload", autoload);
    set.writeStringEntry("SETTINGS", "autoloadPath", autoloadPath.text());
    if(dccIP1.empty() || dccIP2.empty() || dccIP3.empty() || dccIP4.empty()) set.writeStringEntry("SETTINGS", "dccIP", "");
    else set.writeStringEntry("SETTINGS", "dccIP", FXString(dccIP1+"."+dccIP2+"."+dccIP3+"."+dccIP4).text());
    set.writeIntEntry("SETTINGS", "dccPortD", dccPortD);
    set.writeIntEntry("SETTINGS", "dccPortH", dccPortH);
    set.writeIntEntry("SETTINGS", "dccTimeout", dccTimeout);
    set.writeBoolEntry("SETTINGS", "autoDccChat", autoDccChat);
    set.writeBoolEntry("SETTINGS", "autoDccFile", autoDccFile);
    set.writeBoolEntry("SETTINGS", "sounds", sounds);
    set.writeBoolEntry("SETTINGS", "soundConnect", soundConnect);
    set.writeBoolEntry("SETTINGS", "soundDisconnect", soundDisconnect);
    set.writeBoolEntry("SETTINGS", "soundMessage", soundMessage);
    set.writeStringEntry("SETTINGS", "pathConnect", pathConnect.text());
    set.writeStringEntry("SETTINGS", "pathDisconnect", pathDisconnect.text());
    set.writeStringEntry("SETTINGS", "pathMessage", pathMessage.text());
    set.writeBoolEntry("SETTINGS", "stripColors", stripColors);
    set.writeBoolEntry("SETTINGS", "useSmileys", useSmileys);
    set.writeIntEntry("SMILEYS", "number", (FXint)smileysMap.size());
    if((FXint)smileysMap.size())
    {
        StringIt it;
        FXint i;
        for(i=0, it=smileysMap.begin(); it!=smileysMap.end(); it++,i++)
        {
            set.writeStringEntry("SMILEYS", FXStringFormat("smiley%d", i).text(), (*it).first.text());
            set.writeStringEntry("SMILEYS", FXStringFormat("path%d", i).text(), (*it).second.text());
        }
    }
    set.setModified();
    set.unparseFile(utils::GetIniFile());
}
