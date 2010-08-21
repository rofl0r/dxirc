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
    FXMAPFUNC(SEL_COMMAND, SmileyDialog::ID_ACCEPT, SmileyDialog::onAccept),
    FXMAPFUNC(SEL_COMMAND, SmileyDialog::ID_CANCEL, SmileyDialog::onCancel),
    FXMAPFUNC(SEL_COMMAND, SmileyDialog::ID_PATH, SmileyDialog::onPath)
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
    m_smileyText = new FXTextField(matrix, 25, NULL, 0, FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    if(!smiley.empty()) m_smileyText->setText(smiley);
    new FXLabel(matrix, _("Path:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXHorizontalFrame *pathframe = new FXHorizontalFrame(matrix, LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW, 0,0,0,0, 0,0,0,0);
    m_pathText = new FXTextField(pathframe, 25, NULL, 0, TEXTFIELD_READONLY|FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_X);
    m_pathText->setBackColor(getApp()->getBaseColor());
    if(!path.empty()) m_pathText->setText(path);
    m_pathButton = new FXButton(pathframe, "...", NULL, this, ID_PATH, FRAME_RAISED|FRAME_THICK);
    new FXLabel(matrix, _("Preview:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    m_previewLabel = new FXLabel(matrix, "", path.empty() ? NULL : createIconFromName(getApp(), path), JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
}

SmileyDialog::~SmileyDialog()
{

}

long SmileyDialog::onAccept(FXObject*, FXSelector, void*)
{
    getApp()->stopModal(this,TRUE);
    hide();
    return 1;
}

long SmileyDialog::onCancel(FXObject*, FXSelector, void*)
{
    getApp()->stopModal(this,FALSE);
    hide();
    return 1;
}

long SmileyDialog::onPath(FXObject*, FXSelector, void*)
{
    FXFileDialog dialog(this, _("Select file"));
    if(!m_pathText->getText().empty()) dialog.setFilename(m_pathText->getText());
    else dialog.setFilename((FXString)DXIRC_DATADIR+PATHSEPSTRING+"icons"+PATHSEPSTRING+"smileys"+PATHSEPSTRING);
    if(dialog.execute())
    {
        m_pathText->setText(dialog.getFilename());
        updateLabelIcon(getApp(), m_previewLabel, dialog.getFilename());
    }
    return 1;
}

FXbool SmileyDialog::iconExist()
{
    if(m_previewLabel->getIcon()) return TRUE;
    return FALSE;
}

FXDEFMAP(ConfigDialog) ConfigDialogMap[] = {
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_ACCEPT, ConfigDialog::onAccept),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_CANCEL, ConfigDialog::onCancel),
    FXMAPFUNC(SEL_SELECTED, ConfigDialog::ID_COMMAND, ConfigDialog::onCommandsSelected),
    FXMAPFUNC(SEL_DESELECTED, ConfigDialog::ID_COMMAND, ConfigDialog::onCommandsDeselected),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_ADDCOMMAND, ConfigDialog::onAddCommand),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_DELETECOMMAND, ConfigDialog::onDeleteCommand),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_ADDUSER, ConfigDialog::onAddUser),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_MODIFYUSER, ConfigDialog::onModifyUser),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_DELETEUSER, ConfigDialog::onDeleteUser),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_ADDFRIEND, ConfigDialog::onAddFriend),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_MODIFYFRIEND, ConfigDialog::onModifyFriend),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_DELETEFRIEND, ConfigDialog::onDeleteFriend),
    FXMAPFUNC(SEL_CHANGED, ConfigDialog::ID_ICONS, ConfigDialog::onIconsChanged),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_ADDICONS, ConfigDialog::onAddIcons),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_DELETEICONS, ConfigDialog::onDeleteIcons),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_TRAY, ConfigDialog::onTray),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_LOG, ConfigDialog::onLogChanged),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_LOGPATH, ConfigDialog::onPathSelect),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_AUTOLOAD, ConfigDialog::onAutoloadChanged),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_AUTOLOADPATH, ConfigDialog::onAutoloadPathSelect),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_DCCPATH, ConfigDialog::onDccPathSelect),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_SERVERWINDOW, ConfigDialog::onServerWindow),
    FXMAPFUNC(SEL_CHANGED, ConfigDialog::ID_NICK, ConfigDialog::onNickCharChanged),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_RECONNECT, ConfigDialog::onReconnect),
    FXMAPFUNC(SEL_KEYPRESS, 0, ConfigDialog::onKeyPress),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_IRCCOLORS, ConfigDialog::onColor),
    FXMAPFUNC(SEL_CHANGED, ConfigDialog::ID_IRCCOLORS, ConfigDialog::onColor),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_COLORS, ConfigDialog::onThemeColorChanged),
    FXMAPFUNC(SEL_CHANGED, ConfigDialog::ID_COLORS, ConfigDialog::onThemeColorChanged),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_THEME, ConfigDialog::onTheme),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_FONT, ConfigDialog::onFont),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_IRCFONT, ConfigDialog::onIrcFont),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_TABPOS, ConfigDialog::onTabPosition),
    FXMAPFUNC(SEL_SELECTED, ConfigDialog::ID_USER, ConfigDialog::onUsersSelected),
    FXMAPFUNC(SEL_DESELECTED, ConfigDialog::ID_USER, ConfigDialog::onUsersDeselected),
    FXMAPFUNC(SEL_CHANGED, ConfigDialog::ID_USER, ConfigDialog::onUsersChanged),
    FXMAPFUNC(SEL_SELECTED, ConfigDialog::ID_FRIEND, ConfigDialog::onFriendsSelected),
    FXMAPFUNC(SEL_DESELECTED, ConfigDialog::ID_FRIEND, ConfigDialog::onFriendsDeselected),
    FXMAPFUNC(SEL_CHANGED, ConfigDialog::ID_FRIEND, ConfigDialog::onFriendsChanged),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_DCCPORTD, ConfigDialog::onDccPortD),
    FXMAPFUNC(SEL_CHANGED, ConfigDialog::ID_DCCPORTD, ConfigDialog::onDccPortD),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_DCCPORTH, ConfigDialog::onDccPortH),
    FXMAPFUNC(SEL_CHANGED, ConfigDialog::ID_DCCPORTH, ConfigDialog::onDccPortH),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_SOUNDS, ConfigDialog::onSounds),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_SOUNDCONNECT, ConfigDialog::onSoundConnect),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_SOUNDDISCONNECT, ConfigDialog::onSoundDisconnect),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_SOUNDMESSAGE, ConfigDialog::onSoundMessage),
    FXMAPFUNCS(SEL_COMMAND, ConfigDialog::ID_PLAYCONNECT, ConfigDialog::ID_PLAYMESSAGE, ConfigDialog::onPlay),
    FXMAPFUNCS(SEL_COMMAND, ConfigDialog::ID_SELECTCONNECT, ConfigDialog::ID_SELECTMESSAGE, ConfigDialog::onSelectPath),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_USESMILEYS, ConfigDialog::onUseSmileys),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_ADDSMILEY, ConfigDialog::onAddSmiley),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_MODIFYSMILEY, ConfigDialog::onModifySmiley),
    FXMAPFUNC(SEL_DOUBLECLICKED, ConfigDialog::ID_SMILEY, ConfigDialog::onModifySmiley),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_DELETESMILEY, ConfigDialog::onDeleteSmiley),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_IMPORTSMILEY, ConfigDialog::onImportSmiley),
    FXMAPFUNC(SEL_COMMAND, ConfigDialog::ID_EXPORTSMILEY, ConfigDialog::onExportSmiley)
};

FXIMPLEMENT(ConfigDialog, FXDialogBox, ConfigDialogMap, ARRAYNUMBER(ConfigDialogMap))

ConfigDialog::ConfigDialog(FXMainWindow *owner)
    : FXDialogBox(owner, _("Preferences"), DECOR_RESIZE|DECOR_TITLE|DECOR_BORDER, 0,0,0,0, 0,0,0,0, 0,0), m_owner(owner)
{
    readConfig();
    m_showImportwarning = TRUE;
    m_showWarning = TRUE;

    m_textTarget.connect(m_colors.text);
    m_textTarget.setTarget(this);
    m_textTarget.setSelector(ID_IRCCOLORS);
    m_backTarget.connect(m_colors.back);
    m_backTarget.setTarget(this);
    m_backTarget.setSelector(ID_IRCCOLORS);
    m_userTarget.connect(m_colors.user);
    m_userTarget.setTarget(this);
    m_userTarget.setSelector(ID_IRCCOLORS);
    m_actionTarget.connect(m_colors.action);
    m_actionTarget.setTarget(this);
    m_actionTarget.setSelector(ID_IRCCOLORS);
    m_noticeTarget.connect(m_colors.notice);
    m_noticeTarget.setTarget(this);
    m_noticeTarget.setSelector(ID_IRCCOLORS);
    m_errorTarget.connect(m_colors.error);
    m_errorTarget.setTarget(this);
    m_errorTarget.setSelector(ID_IRCCOLORS);
    m_hilightTarget.connect(m_colors.hilight);
    m_hilightTarget.setTarget(this);
    m_hilightTarget.setSelector(ID_IRCCOLORS);
    m_linkTarget.connect(m_colors.link);
    m_linkTarget.setTarget(this);
    m_linkTarget.setSelector(ID_IRCCOLORS);

    m_targetSameCmd.connect(m_sameCmd);
    m_targetSameList.connect(m_sameList);
    m_targetColoredNick.connect(m_coloredNick);
    m_targetCloseToTray.connect(m_closeToTray);

    m_targetBack.connect(m_themeCurrent.back);
    m_targetBack.setTarget(this);
    m_targetBack.setSelector(ID_COLORS);
    m_targetBase.connect(m_themeCurrent.base);
    m_targetBase.setTarget(this);
    m_targetBase.setSelector(ID_COLORS);
    m_targetBorder.connect(m_themeCurrent.border);
    m_targetBorder.setTarget(this);
    m_targetBorder.setSelector(ID_COLORS);
    m_targetFore.connect(m_themeCurrent.fore);
    m_targetFore.setTarget(this);
    m_targetFore.setSelector(ID_COLORS);
    m_targetMenuback.connect(m_themeCurrent.menuback);
    m_targetMenuback.setTarget(this);
    m_targetMenuback.setSelector(ID_COLORS);
    m_targetMenufore.connect(m_themeCurrent.menufore);
    m_targetMenufore.setTarget(this);
    m_targetMenufore.setSelector(ID_COLORS);
    m_targetSelback.connect(m_themeCurrent.selback);
    m_targetSelback.setTarget(this);
    m_targetSelback.setSelector(ID_COLORS);
    m_targetSelfore.connect(m_themeCurrent.selfore);
    m_targetSelfore.setTarget(this);
    m_targetSelfore.setSelector(ID_COLORS);
    m_targetTipback.connect(m_themeCurrent.tipback);
    m_targetTipback.setTarget(this);
    m_targetTipback.setSelector(ID_COLORS);
    m_targetTipfore.connect(m_themeCurrent.tipfore);
    m_targetTipfore.setTarget(this);
    m_targetTipfore.setSelector(ID_COLORS);

    m_trayTarget.connect(m_useTray);
    m_trayTarget.setTarget(this);
    m_trayTarget.setSelector(ID_TRAY);
    m_serverTarget.connect(m_serverWindow);
    m_serverTarget.setTarget(this);
    m_serverTarget.setSelector(ID_SERVERWINDOW);
    m_logTarget.connect(m_logging);
    m_logTarget.setTarget(this);
    m_logTarget.setSelector(ID_LOG);
    m_autoloadTarget.connect(m_autoload);
    m_autoloadTarget.setTarget(this);
    m_autoloadTarget.setSelector(ID_AUTOLOAD);

    m_targetReconnect.connect(m_reconnect);
    m_targetReconnect.setTarget(this);
    m_targetReconnect.setSelector(ID_RECONNECT);
    m_targetNumberAttempt.connect(m_numberAttempt);
    m_targetDelayAttempt.connect(m_delayAttempt);
    m_targetMaxAway.connect(m_maxAway);

    m_targetLogPath.connect(m_logPath);
    m_targetDccPath.connect(m_dccPath);
    m_targetAutoloadPath.connect(m_autoloadPath);

    m_targetDccIP1.connect(m_dccIP1);
    m_targetDccIP2.connect(m_dccIP2);
    m_targetDccIP3.connect(m_dccIP3);
    m_targetDccIP4.connect(m_dccIP4);

    m_targetDccPortD.connect(m_dccPortD);
    m_targetDccPortD.setTarget(this);
    m_targetDccPortD.setSelector(ID_DCCPORTD);
    m_targetDccPortH.connect(m_dccPortH);
    m_targetDccPortH.setTarget(this);
    m_targetDccPortH.setSelector(ID_DCCPORTH);

    m_targetDccTimeout.connect(m_dccTimeout);

    m_targetAutoDccChat.connect(m_autoDccChat);
    m_targetAutoDccFile.connect(m_autoDccFile);

    m_targetSound.connect(m_sounds);
    m_targetSound.setTarget(this);
    m_targetSound.setSelector(ID_SOUNDS);
    m_targetSoundConnect.connect(m_soundConnect);
    m_targetSoundConnect.setTarget(this);
    m_targetSoundConnect.setSelector(ID_SOUNDCONNECT);
    m_targetSoundDisconnect.connect(m_soundDisconnect);
    m_targetSoundDisconnect.setTarget(this);
    m_targetSoundDisconnect.setSelector(ID_SOUNDDISCONNECT);
    m_targetSoundMessage.connect(m_soundMessage);
    m_targetSoundMessage.setTarget(this);
    m_targetSoundMessage.setSelector(ID_SOUNDMESSAGE);
    m_targetPathConnect.connect(m_pathConnect);
    m_targetPathDisconnect.connect(m_pathDisconnect);
    m_targetPathMessage.connect(m_pathMessage);

    m_targetStripColors.connect(m_stripColors);

    m_targetUseSmileys.connect(m_useSmileys);
    m_targetUseSmileys.setTarget(this);
    m_targetUseSmileys.setSelector(ID_USESMILEYS);

    m_targetTrayColor.connect(m_trayColor);

    m_targetUseSpell.connect(m_useSpell);

    getApp()->getNormalFont()->create();
    FXFontDesc fontdescription;
    getApp()->getNormalFont()->getFontDesc(fontdescription);
    m_font = new FXFont(getApp(),fontdescription);
    m_font->create();

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
    new FXColorWell(colormatrix, FXRGB(0,0,255), &m_backTarget, FXDataTarget::ID_VALUE, COLORWELL_OPAQUEONLY|FRAME_SUNKEN|FRAME_THICK|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW, 0,0,40,24);
    new FXLabel(colormatrix, _("Text backround color"), NULL, JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
    new FXColorWell(colormatrix, FXRGB(0,0,255), &m_textTarget, FXDataTarget::ID_VALUE, COLORWELL_OPAQUEONLY|FRAME_SUNKEN|FRAME_THICK|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW, 0,0,40,24);
    new FXLabel(colormatrix, _("Text color"), NULL, JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
    new FXColorWell(colormatrix, FXRGB(0,0,255), &m_userTarget, FXDataTarget::ID_VALUE, COLORWELL_OPAQUEONLY|FRAME_SUNKEN|FRAME_THICK|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW, 0,0,40,24);
    new FXLabel(colormatrix, _("User events text color"), NULL, JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
    new FXColorWell(colormatrix, FXRGB(0,0,255), &m_actionTarget, FXDataTarget::ID_VALUE, COLORWELL_OPAQUEONLY|FRAME_SUNKEN|FRAME_THICK|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW, 0,0,40,24);
    new FXLabel(colormatrix, _("Actions message text color"), NULL, JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
    new FXColorWell(colormatrix, FXRGB(0,0,255), &m_noticeTarget, FXDataTarget::ID_VALUE, COLORWELL_OPAQUEONLY|FRAME_SUNKEN|FRAME_THICK|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW, 0,0,40,24);
    new FXLabel(colormatrix, _("Notice text color"), NULL, JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
    new FXColorWell(colormatrix, FXRGB(0,0,255), &m_errorTarget, FXDataTarget::ID_VALUE, COLORWELL_OPAQUEONLY|FRAME_SUNKEN|FRAME_THICK|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW, 0,0,40,24);
    new FXLabel(colormatrix, _("Error text color"), NULL, JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
    new FXColorWell(colormatrix, FXRGB(0,0,255), &m_hilightTarget, FXDataTarget::ID_VALUE, COLORWELL_OPAQUEONLY|FRAME_SUNKEN|FRAME_THICK|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW, 0,0,40,24);
    new FXLabel(colormatrix, _("Highlight message text color"), NULL, JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
    new FXColorWell(colormatrix, FXRGB(0,0,255), &m_linkTarget, FXDataTarget::ID_VALUE, COLORWELL_OPAQUEONLY|FRAME_SUNKEN|FRAME_THICK|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW, 0,0,40,24);
    new FXLabel(colormatrix, _("Link color"), NULL, JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
    new FXLabel(colormatrix, _("Font"));
    m_ircfontButton = new FXButton(colormatrix, " ", NULL, this, ID_IRCFONT, LAYOUT_CENTER_Y|FRAME_RAISED|JUSTIFY_CENTER_X|JUSTIFY_CENTER_Y|LAYOUT_FILL_X, 0,0,0,0, 10,10,2,5);
    new FXCheckButton(cframe, _("Use same font for commandline"), &m_targetSameCmd, FXDataTarget::ID_VALUE, CHECKBUTTON_NORMAL|LAYOUT_FILL_X|LAYOUT_SIDE_LEFT|JUSTIFY_LEFT);
    new FXCheckButton(cframe, _("Use same font for user list"), &m_targetSameList, FXDataTarget::ID_VALUE, CHECKBUTTON_NORMAL|LAYOUT_FILL_X|LAYOUT_SIDE_LEFT|JUSTIFY_LEFT);
    new FXCheckButton(cframe, _("Use colored nick"), &m_targetColoredNick, FXDataTarget::ID_VALUE, CHECKBUTTON_NORMAL|LAYOUT_FILL_X|LAYOUT_SIDE_LEFT|JUSTIFY_LEFT);
    new FXCheckButton(cframe, _("Strip colors in text"), &m_targetStripColors, FXDataTarget::ID_VALUE, CHECKBUTTON_NORMAL|LAYOUT_FILL_X|LAYOUT_SIDE_LEFT|JUSTIFY_LEFT);
    FXVerticalFrame *tframe = new FXVerticalFrame(hframe, FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    m_text = new FXText(tframe, NULL, 0, LAYOUT_FILL_X|LAYOUT_FILL_Y|TEXT_READONLY);
    m_text->setScrollStyle(HSCROLLING_OFF);

    FXVerticalFrame *ignorepane = new FXVerticalFrame(switcher, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(ignorepane, _("Ignore commands and users"), NULL, LAYOUT_LEFT);
    new FXHorizontalSeparator(ignorepane, SEPARATOR_LINE|LAYOUT_FILL_X);
    FXGroupBox *commandsgroup = new FXGroupBox(ignorepane, _("Commands"), FRAME_GROOVE|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_SIDE_TOP);
    FXVerticalFrame *commandsbuttons = new FXVerticalFrame(commandsgroup, LAYOUT_SIDE_RIGHT|LAYOUT_FILL_Y|PACK_UNIFORM_WIDTH);
    m_addCommand = new FXButton(commandsbuttons, _("Add"), NULL, this, ID_ADDCOMMAND, FRAME_RAISED|FRAME_THICK);
    m_deleteCommand = new FXButton(commandsbuttons, _("Delete"), NULL, this, ID_DELETECOMMAND, FRAME_RAISED|FRAME_THICK);
    FXVerticalFrame *commandslist = new FXVerticalFrame(commandsgroup, LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN|FRAME_THICK);
    m_commands = new FXList(commandslist, this, ID_COMMAND, LIST_BROWSESELECT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    FXGroupBox *usersgroup = new FXGroupBox(ignorepane, _("Users"), FRAME_GROOVE|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    FXVerticalFrame *usersbuttons = new FXVerticalFrame(usersgroup, LAYOUT_SIDE_RIGHT|LAYOUT_FILL_Y|PACK_UNIFORM_WIDTH);
    m_addUser = new FXButton(usersbuttons, _("Add"), NULL, this, ID_ADDUSER, FRAME_RAISED|FRAME_THICK);
    m_modifyUser = new FXButton(usersbuttons, _("Modify"), NULL, this, ID_MODIFYUSER, FRAME_RAISED|FRAME_THICK);
    m_deleteUser = new FXButton(usersbuttons, _("Delete"), NULL, this, ID_DELETEUSER, FRAME_RAISED|FRAME_THICK);
    FXVerticalFrame *userspane = new FXVerticalFrame(usersgroup, LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN|FRAME_THICK);
    FXHorizontalFrame *usersframe = new FXHorizontalFrame(userspane, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    m_users = new FXIconList(usersframe, this, ID_USER, ICONLIST_AUTOSIZE|ICONLIST_DETAILED|ICONLIST_BROWSESELECT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    m_users->appendHeader(_("User"), NULL, 200);
    m_users->appendHeader(_("Channel(s)"), NULL, 150);
    m_users->appendHeader(_("Server(s)"), NULL, 150);
    fillCommnads();
    fillUsers();
    m_commands->getNumItems() ? m_deleteCommand->enable() : m_deleteCommand->disable();
    if(m_usersList.no())
    {
        m_deleteUser->enable();
        m_modifyUser->enable();
    }
    else
    {
        m_deleteUser->disable();
        m_modifyUser->disable();
    }

    FXVerticalFrame *otherpane = new FXVerticalFrame(switcher, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(otherpane, _("General settings"), NULL, LAYOUT_LEFT);
    new FXHorizontalSeparator(otherpane, SEPARATOR_LINE|LAYOUT_FILL_X);
    FXGroupBox *themesgroup = new FXGroupBox(otherpane, _("Nick icons themes"), FRAME_GROOVE|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_SIDE_TOP);
    FXVerticalFrame *themesbuttons = new FXVerticalFrame(themesgroup, LAYOUT_SIDE_RIGHT|LAYOUT_FILL_Y|PACK_UNIFORM_WIDTH);
    m_addTheme = new FXButton(themesbuttons, _("Add"), NULL, this, ID_ADDICONS, FRAME_RAISED|FRAME_THICK);
    m_deleteTheme = new FXButton(themesbuttons, _("Delete"), NULL, this, ID_DELETEICONS, FRAME_RAISED|FRAME_THICK);
    FXVerticalFrame *themeslist = new FXVerticalFrame(themesgroup, LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN|FRAME_THICK);
    m_icons = new FXList(themeslist, this, ID_ICONS, LIST_BROWSESELECT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    fillIcons();
    for(FXint i=0; i<m_icons->getNumItems(); i++)
    {
        if(m_icons->getItemText(i) == m_themePath)
        {
            m_icons->setCurrentItem(i);
            break;
        }
    }
    m_icons->getNumItems()>1 ? m_deleteTheme->enable() : m_deleteTheme->disable();
    m_iconsBar = new FXToolBar(themeslist, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    m_icon1 = new FXButton(m_iconsBar, _("\tAdmin"), makeIcon(getApp(), m_themePath, "irc_admin.png", TRUE), NULL, 0, BUTTON_TOOLBAR);
    m_icon2 = new FXButton(m_iconsBar, _("\tOwner"), makeIcon(getApp(), m_themePath, "irc_owner.png", TRUE), NULL, 0, BUTTON_TOOLBAR);
    m_icon3 = new FXButton(m_iconsBar, _("\tOp"), makeIcon(getApp(), m_themePath, "irc_op.png", TRUE), NULL, 0, BUTTON_TOOLBAR);
    m_icon4 = new FXButton(m_iconsBar, _("\tHalfop"), makeIcon(getApp(), m_themePath, "irc_halfop.png", TRUE), NULL, 0, BUTTON_TOOLBAR);
    m_icon5 = new FXButton(m_iconsBar, _("\tVoice"), makeIcon(getApp(), m_themePath, "irc_voice.png", TRUE), NULL, 0, BUTTON_TOOLBAR);
    m_icon6 = new FXButton(m_iconsBar, _("\tNormal"), makeIcon(getApp(), m_themePath, "irc_normal.png", TRUE), NULL, 0, BUTTON_TOOLBAR);
    m_icon7 = new FXButton(m_iconsBar, _("\tAway"), makeAwayIcon(getApp(), m_themePath, "irc_normal.png"), NULL, 0, BUTTON_TOOLBAR);
    new FXCheckButton(otherpane, _("Reconnect after disconnection"), &m_targetReconnect, FXDataTarget::ID_VALUE, CHECKBUTTON_NORMAL|LAYOUT_FILL_X|LAYOUT_SIDE_LEFT|JUSTIFY_LEFT);
    FXHorizontalFrame *napane = new FXHorizontalFrame(otherpane, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    m_numberAttemptLabel = new FXLabel(napane, _("Number of attempts"), NULL, LAYOUT_LEFT);
    if(!m_reconnect) m_numberAttemptLabel->disable();
    m_numberAttemptSpinner = new FXSpinner(napane, 4, &m_targetNumberAttempt, FXDataTarget::ID_VALUE, SPIN_CYCLIC|FRAME_GROOVE);
    m_numberAttemptSpinner->setRange(1,20);
    if(!m_reconnect) m_numberAttemptSpinner->disable();
    FXHorizontalFrame *dapane = new FXHorizontalFrame(otherpane, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    m_delayAttemptLabel = new FXLabel(dapane, _("Delay between two attempts in seconds"), NULL, LAYOUT_LEFT);
    if(!m_reconnect) m_delayAttemptLabel->disable();
    m_delayAttemptSpinner = new FXSpinner(dapane, 4, &m_targetDelayAttempt, FXDataTarget::ID_VALUE, SPIN_CYCLIC|FRAME_GROOVE);
    m_delayAttemptSpinner->setRange(10,120);
    if(!m_reconnect) m_delayAttemptSpinner->disable();
    FXHorizontalFrame *maxpane = new FXHorizontalFrame(otherpane, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(maxpane, _("Max. users number for checking away\tToo high number can be reason for ban"), NULL, LAYOUT_LEFT);
    new FXSpinner(maxpane, 4, &m_targetMaxAway, FXDataTarget::ID_VALUE, SPIN_NOMAX|FRAME_GROOVE);
    FXHorizontalFrame *nickpane = new FXHorizontalFrame(otherpane, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(nickpane, _("Nick completion char"), NULL, LAYOUT_LEFT);
    m_nickCharField = new FXTextField(nickpane, 1, this, ID_NICK, TEXTFIELD_LIMITED|FRAME_THICK|FRAME_SUNKEN/*|LAYOUT_FILL_X*/);
    m_nickCharField->setText(m_nickChar);
#ifdef HAVE_TRAY
    new FXCheckButton(otherpane, _("Use trayicon"), &m_trayTarget, FXDataTarget::ID_VALUE, CHECKBUTTON_NORMAL|LAYOUT_FILL_X|LAYOUT_SIDE_LEFT|JUSTIFY_LEFT);
    m_closeToTrayButton = new FXCheckButton(otherpane, _("Close button hides application"), &m_targetCloseToTray, FXDataTarget::ID_VALUE, CHECKBUTTON_NORMAL|LAYOUT_FILL_X|LAYOUT_SIDE_LEFT|JUSTIFY_LEFT);
#endif
#ifdef HAVE_ENCHANT
    (new FXCheckButton(otherpane, _("Use spellchecking"), &m_targetUseSpell, FXDataTarget::ID_VALUE, CHECKBUTTON_NORMAL|LAYOUT_FILL_X|LAYOUT_SIDE_LEFT|JUSTIFY_LEFT))->setCheck(m_useSpell);
#endif
    new FXCheckButton(otherpane, _("Special tab for server messages"), &m_serverTarget, FXDataTarget::ID_VALUE, CHECKBUTTON_NORMAL|LAYOUT_FILL_X|LAYOUT_SIDE_LEFT|JUSTIFY_LEFT);
    new FXCheckButton(otherpane, _("Logging chats"), &m_logTarget, FXDataTarget::ID_VALUE, CHECKBUTTON_NORMAL|LAYOUT_FILL_X|LAYOUT_SIDE_LEFT|JUSTIFY_LEFT);
    FXHorizontalFrame *logpane = new FXHorizontalFrame(otherpane, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(logpane, _("Log path"), NULL, LAYOUT_LEFT);
    (new FXTextField(logpane, 25, &m_targetLogPath, FXDataTarget::ID_VALUE, TEXTFIELD_READONLY|FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_X))->disable();
    if(!FXStat::exists(m_logPath)) m_logPath = FXSystem::getHomeDirectory();
    m_selectPath = new FXButton(logpane, "...", NULL, this, ID_LOGPATH, FRAME_RAISED|FRAME_THICK);
    if(m_logging) m_selectPath->enable();
    else m_selectPath->disable();
#ifdef HAVE_LUA
    new FXCheckButton(otherpane, _("Automatically load scripts"), &m_autoloadTarget, FXDataTarget::ID_VALUE, CHECKBUTTON_NORMAL|LAYOUT_FILL_X|LAYOUT_SIDE_LEFT|JUSTIFY_LEFT);
    FXHorizontalFrame *aloadpane = new FXHorizontalFrame(otherpane, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(aloadpane, _("Path"), NULL, LAYOUT_LEFT);
    (new FXTextField(aloadpane, 25, &m_targetAutoloadPath, FXDataTarget::ID_VALUE, TEXTFIELD_READONLY|FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_X))->disable();
    if(!FXStat::exists(m_autoloadPath))
    {
#ifdef WIN32
        m_autoloadPath = utils::instance().localeToUtf8(FXSystem::getHomeDirectory()+PATHSEPSTRING+"scripts");
#else
        m_autoloadPath = FXSystem::getHomeDirectory()+PATHSEPSTRING+".dxirc"+PATHSEPSTRING+"scripts";
#endif
    }
    m_selectAutoloadPath = new FXButton(aloadpane, "...", NULL, this, ID_AUTOLOADPATH, FRAME_RAISED|FRAME_THICK);
    if(m_autoload) m_selectAutoloadPath->enable();
    else m_selectAutoloadPath->disable();
#endif
    FXHorizontalFrame *tabpane = new FXHorizontalFrame(otherpane, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(tabpane, _("Tab position"), NULL, LAYOUT_LEFT);
    m_listTabs = new FXListBox(tabpane, this, ID_TABPOS);
    m_listTabs->appendItem(_("Bottom"));
    m_listTabs->appendItem(_("Left"));
    m_listTabs->appendItem(_("Top"));
    m_listTabs->appendItem(_("Right"));
    m_listTabs->setNumVisible(4);
    if(m_tabPosition>=0 && m_tabPosition<4) m_listTabs->setCurrentItem(m_tabPosition);


    FXVerticalFrame *lookpane = new FXVerticalFrame(switcher, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(lookpane, _("FOX toolkit look for dxirc"), NULL, LAYOUT_LEFT);
    new FXHorizontalSeparator(lookpane, SEPARATOR_LINE|LAYOUT_FILL_X);
    FXHorizontalFrame *hframe1 = new FXHorizontalFrame(lookpane, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    FXVerticalFrame *vframe1 = new FXVerticalFrame(hframe1, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    m_vframe2 = new FXVerticalFrame(hframe1, LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_THICK|FRAME_RAISED);
    new FXLabel(vframe1, _("Theme:"), NULL, LAYOUT_CENTER_Y);
    m_themes = new FXListBox(vframe1,this,ID_THEME,LAYOUT_FILL_X|FRAME_SUNKEN|FRAME_THICK);
    m_themes->setNumVisible(9);
    fillThemes();
    new FXSeparator(vframe1, SEPARATOR_GROOVE|LAYOUT_FILL_X);
    FXMatrix *themeMatrix = new FXMatrix(vframe1, 2, LAYOUT_FILL_Y|MATRIX_BY_COLUMNS, 0,0,0,0, DEFAULT_SPACING,DEFAULT_SPACING,DEFAULT_SPACING,DEFAULT_SPACING, 1,1);
    new FXColorWell(themeMatrix, FXRGB(0,0,255), &m_targetBase, FXDataTarget::ID_VALUE);
    new FXLabel(themeMatrix, _("Base Color"));
    new FXColorWell(themeMatrix, FXRGB(0,0,255), &m_targetBorder, FXDataTarget::ID_VALUE);
    new FXLabel(themeMatrix, _("Border Color"));
    new FXColorWell(themeMatrix, FXRGB(0,0,255), &m_targetFore,FXDataTarget::ID_VALUE);
    new FXLabel(themeMatrix, _("Text Color"));
    new FXColorWell(themeMatrix, FXRGB(0,0,255), &m_targetBack, FXDataTarget::ID_VALUE);
    new FXLabel(themeMatrix, _("Background Color"));
    new FXColorWell(themeMatrix, FXRGB(0,0,255), &m_targetSelfore, FXDataTarget::ID_VALUE);
    new FXLabel(themeMatrix, _("Selected Text Color"));
    new FXColorWell(themeMatrix, FXRGB(0,0,255), &m_targetSelback, FXDataTarget::ID_VALUE);
    new FXLabel(themeMatrix, _("Selected Background Color"));
    new FXColorWell(themeMatrix, FXRGB(0,0,255), &m_targetMenufore, FXDataTarget::ID_VALUE);
    new FXLabel(themeMatrix, _("Selected Menu Text Color"));
    new FXColorWell(themeMatrix, FXRGB(0,0,255), &m_targetMenuback, FXDataTarget::ID_VALUE);
    new FXLabel(themeMatrix, _("Selected Menu Background Color"));
    new FXColorWell(themeMatrix, FXRGB(0,0,255), &m_targetTipfore, FXDataTarget::ID_VALUE);
    new FXLabel(themeMatrix, _("Tip Text Color"));
    new FXColorWell(themeMatrix, FXRGB(0,0,255), &m_targetTipback, FXDataTarget::ID_VALUE);
    new FXLabel(themeMatrix, _("Tip Background Color"));
#ifndef WIN32
    new FXColorWell(themeMatrix, FXRGB(0,0,255), &m_targetTrayColor, FXDataTarget::ID_VALUE);
    new FXLabel(themeMatrix, _("Tray Color"));
#endif
    m_label = new FXLabel(m_vframe2, "Label");
    m_textFrame1 = new FXHorizontalFrame(m_vframe2, LAYOUT_FILL_X);
    m_textTest = new FXTextField(m_textFrame1, 30, NULL, 0, LAYOUT_FILL_X|FRAME_THICK|FRAME_SUNKEN);
    m_textTest->setText(_("Select this text, to see the selected colors"));
    m_textFrame2 = new FXHorizontalFrame(m_vframe2, LAYOUT_FILL_X|FRAME_THICK|FRAME_SUNKEN, 0,0,0,0,2,2,2,2,0,0);
    m_labelSelected = new FXLabel(m_textFrame2, _("Selected Text (with focus)"), NULL, LAYOUT_FILL_X, 0,0,0,0,1,1,1,1);
    m_textFrame3 = new FXHorizontalFrame(m_vframe2, LAYOUT_FILL_X|FRAME_THICK|FRAME_SUNKEN, 0,0,0,0,2,2,2,2,0,0);
    m_labelNocurrent = new FXLabel(m_textFrame3, _("Selected Text (no focus)"), NULL, LAYOUT_FILL_X, 0,0,0,0,1,1,1,1);
    m_sep1 = new FXSeparator(m_vframe2, LAYOUT_FILL_X|SEPARATOR_LINE);
    m_labelTip = new FXLabel(m_vframe2, _("Tooltip example"), NULL, FRAME_LINE|LAYOUT_CENTER_X);
    m_menuGroup = new FXGroupBox(m_vframe2, _("Menu example"), FRAME_GROOVE|LAYOUT_FILL_Y|LAYOUT_FILL_X);
    m_menuFrame = new FXVerticalFrame(m_menuGroup, FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X|LAYOUT_CENTER_Y, 0,0,0,0,0,0,0,0,0,0);
    m_menuLabels[0]=new FXLabel(m_menuFrame, _("&Server list"), NULL, LABEL_NORMAL, 0,0,0,0,16,4);
    m_menuLabels[1]=new FXLabel(m_menuFrame, _("Selected Menu Entry"), NULL, LABEL_NORMAL, 0,0,0,0,16,4);
    m_sep2 = new FXSeparator(m_menuFrame, LAYOUT_FILL_X|SEPARATOR_LINE);
    m_menuLabels[2]=new FXLabel(m_menuFrame, _("&Quit"), NULL, LABEL_NORMAL, 0,0,0,0,16,4);
    FXHorizontalFrame *fontframe = new FXHorizontalFrame(lookpane, LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, DEFAULT_SPACING,DEFAULT_SPACING,DEFAULT_SPACING,DEFAULT_SPACING);
    new FXLabel(fontframe, _("Font"));
    m_fontButton = new FXButton(fontframe, " ", NULL, this, ID_FONT, LAYOUT_CENTER_Y|FRAME_RAISED|JUSTIFY_CENTER_X|JUSTIFY_CENTER_Y|LAYOUT_FILL_X);

    FXVerticalFrame *dccpane = new FXVerticalFrame(switcher, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(dccpane, _("DCC settings"), NULL, LAYOUT_LEFT);
    new FXHorizontalSeparator(dccpane, SEPARATOR_LINE|LAYOUT_FILL_X);
    new FXLabel(dccpane, _("Directory for saving files:"), NULL, LAYOUT_LEFT);
    FXHorizontalFrame *pathframe = new FXHorizontalFrame(dccpane, LAYOUT_FILL_X);
    (new FXTextField(pathframe, 30, &m_targetDccPath, FXDataTarget::ID_VALUE, TEXTFIELD_READONLY|FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X))->disable();
    new FXButton(pathframe, "...", NULL, this, ID_DCCPATH, FRAME_RAISED|FRAME_THICK);
    FXHorizontalFrame *ipframe = new FXHorizontalFrame(dccpane, LAYOUT_FILL_X);
    new FXLabel(ipframe, _("DCC IP address:"), NULL, LAYOUT_LEFT);
    new FXTextField(ipframe, 3, &m_targetDccIP1, FXDataTarget::ID_VALUE, TEXTFIELD_INTEGER|TEXTFIELD_LIMITED|FRAME_SUNKEN|FRAME_THICK);
    new FXLabel(ipframe, ".", NULL, LAYOUT_LEFT);
    new FXTextField(ipframe, 3, &m_targetDccIP2, FXDataTarget::ID_VALUE, TEXTFIELD_INTEGER|TEXTFIELD_LIMITED|FRAME_SUNKEN|FRAME_THICK);
    new FXLabel(ipframe, ".", NULL, LAYOUT_LEFT);
    new FXTextField(ipframe, 3, &m_targetDccIP3, FXDataTarget::ID_VALUE, TEXTFIELD_INTEGER|TEXTFIELD_LIMITED|FRAME_SUNKEN|FRAME_THICK);
    new FXLabel(ipframe, ".", NULL, LAYOUT_LEFT);
    new FXTextField(ipframe, 3, &m_targetDccIP4, FXDataTarget::ID_VALUE, TEXTFIELD_INTEGER|TEXTFIELD_LIMITED|FRAME_SUNKEN|FRAME_THICK);
    new FXLabel(dccpane, _("(Leave blank for use IP address from server)"), NULL, LAYOUT_LEFT);
    new FXSeparator(dccpane, LAYOUT_FILL_X);
    FXHorizontalFrame *portframe = new FXHorizontalFrame(dccpane, LAYOUT_FILL_X);
    new FXLabel(portframe, _("DCC ports:"), NULL, LAYOUT_LEFT);
    FXSpinner *dspinner = new FXSpinner(portframe, 6, &m_targetDccPortD, FXDataTarget::ID_VALUE, FRAME_SUNKEN|FRAME_THICK);
    dspinner->setRange(0, 65536);
    new FXLabel(portframe, "-");
    FXSpinner *hspinner = new FXSpinner(portframe, 6, &m_targetDccPortH, FXDataTarget::ID_VALUE, FRAME_SUNKEN|FRAME_THICK);
    hspinner->setRange(0, 65536);
    new FXLabel(dccpane, _("(Set 0 for use ports from OS)"), NULL, LAYOUT_LEFT);
    FXHorizontalFrame *timeframe = new FXHorizontalFrame(dccpane, LAYOUT_FILL_X);
    new FXLabel(timeframe, _("Time for waiting for connection in seconds"), NULL, LAYOUT_LEFT);
    new FXSpinner(timeframe, 4, &m_targetDccTimeout, FXDataTarget::ID_VALUE, SPIN_NOMAX|SPIN_CYCLIC|FRAME_SUNKEN|FRAME_THICK);
    new FXCheckButton(dccpane, _("Automatically connect offered chat"), &m_targetAutoDccChat, FXDataTarget::ID_VALUE, CHECKBUTTON_NORMAL|LAYOUT_FILL_X|LAYOUT_SIDE_LEFT|JUSTIFY_LEFT);
    new FXCheckButton(dccpane, _("Automatically receive offered file"), &m_targetAutoDccFile, FXDataTarget::ID_VALUE, CHECKBUTTON_NORMAL|LAYOUT_FILL_X|LAYOUT_SIDE_LEFT|JUSTIFY_LEFT);

    FXVerticalFrame *soundpane = new FXVerticalFrame(switcher, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(soundpane, _("Sound settings"), NULL, LAYOUT_LEFT);
    new FXHorizontalSeparator(soundpane, SEPARATOR_LINE|LAYOUT_FILL_X);
    new FXCheckButton(soundpane, _("Use sounds"), &m_targetSound, FXDataTarget::ID_VALUE, CHECKBUTTON_NORMAL|LAYOUT_FILL_X|LAYOUT_SIDE_LEFT|JUSTIFY_LEFT);
    FXGroupBox *eventsgroup = new FXGroupBox(soundpane, _("Events"), FRAME_GROOVE|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_SIDE_TOP);
    FXVerticalFrame *eventsframe = new FXVerticalFrame(eventsgroup, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    m_checkConnect = new FXCheckButton(eventsframe, _("Friend connected"), &m_targetSoundConnect, FXDataTarget::ID_VALUE, CHECKBUTTON_NORMAL|LAYOUT_FILL_X|LAYOUT_SIDE_LEFT|JUSTIFY_LEFT);
    if(!m_sounds)
        m_checkConnect->disable();
    FXHorizontalFrame *connectframe = new FXHorizontalFrame(eventsframe, LAYOUT_FILL_X|PACK_UNIFORM_HEIGHT);
    (new FXTextField(connectframe, 30, &m_targetPathConnect, FXDataTarget::ID_VALUE, TEXTFIELD_READONLY|FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X))->disable();
    m_selectConnect = new FXButton(connectframe, "...", NULL, this, ID_SELECTCONNECT, FRAME_RAISED|FRAME_THICK);
    m_playConnect = new FXButton(connectframe, _("\tPlay"), ICO_PLAY, this, ID_PLAYCONNECT, FRAME_RAISED|FRAME_THICK);
    if(!m_soundConnect || !m_sounds)
    {
        m_selectConnect->disable();
        m_playConnect->disable();
    }
    if(!FXStat::exists(m_pathConnect))
        m_playConnect->disable();
    m_checkDisconnect = new FXCheckButton(eventsframe, _("Friend disconnected"), &m_targetSoundDisconnect, FXDataTarget::ID_VALUE, CHECKBUTTON_NORMAL|LAYOUT_FILL_X|LAYOUT_SIDE_LEFT|JUSTIFY_LEFT);
    if(!m_sounds)
        m_checkDisconnect->disable();
    FXHorizontalFrame *disconnectframe = new FXHorizontalFrame(eventsframe, LAYOUT_FILL_X|PACK_UNIFORM_HEIGHT);
    (new FXTextField(disconnectframe, 30, &m_targetPathDisconnect, FXDataTarget::ID_VALUE, TEXTFIELD_READONLY|FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X))->disable();
    m_selectDisconnect = new FXButton(disconnectframe, "...", NULL, this, ID_SELECTDISCONNECT, FRAME_RAISED|FRAME_THICK);
    m_playDisconnect = new FXButton(disconnectframe, _("\tPlay"), ICO_PLAY, this, ID_PLAYDISCONNECT, FRAME_RAISED|FRAME_THICK);
    if(!m_soundDisconnect || !m_sounds)
    {
        m_selectDisconnect->disable();
        m_playDisconnect->disable();
    }
    if(!FXStat::exists(m_pathDisconnect))
        m_playDisconnect->disable();
    FXGroupBox *friendsgroup = new FXGroupBox(eventsframe, _("Friends"), FRAME_GROOVE|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    FXVerticalFrame *friendsbuttons = new FXVerticalFrame(friendsgroup, LAYOUT_SIDE_RIGHT|LAYOUT_FILL_Y|PACK_UNIFORM_WIDTH);
    m_addFriend = new FXButton(friendsbuttons, _("Add"), NULL, this, ID_ADDFRIEND, FRAME_RAISED|FRAME_THICK);
    m_modifyFriend = new FXButton(friendsbuttons, _("Modify"), NULL, this, ID_MODIFYFRIEND, FRAME_RAISED|FRAME_THICK);
    m_deleteFriend = new FXButton(friendsbuttons, _("Delete"), NULL, this, ID_DELETEFRIEND, FRAME_RAISED|FRAME_THICK);
    FXVerticalFrame *friendspane = new FXVerticalFrame(friendsgroup, LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN|FRAME_THICK);
    FXHorizontalFrame *friendsframe = new FXHorizontalFrame(friendspane, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    m_friends = new FXIconList(friendsframe, this, ID_FRIEND, ICONLIST_AUTOSIZE|ICONLIST_DETAILED|ICONLIST_BROWSESELECT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    m_friends->appendHeader(_("Friend"), NULL, 150);
    m_friends->appendHeader(_("Channel(s)"), NULL, 150);
    m_friends->appendHeader(_("Server(s)"), NULL, 150);
    fillFriends();
    if(m_friendsList.no())
    {
        m_deleteFriend->enable();
        m_modifyFriend->enable();
    }
    else
    {
        m_deleteFriend->disable();
        m_modifyFriend->disable();
    }
    if(!m_sounds)
    {
        m_addFriend->disable();
        m_deleteFriend->disable();
        m_modifyFriend->disable();
    }
    m_checkMessage = new FXCheckButton(eventsframe, _("Highlighted message or query message"), &m_targetSoundMessage, FXDataTarget::ID_VALUE, CHECKBUTTON_NORMAL|LAYOUT_FILL_X|LAYOUT_SIDE_LEFT|JUSTIFY_LEFT);
    if(!m_sounds)
        m_checkMessage->disable();
    FXHorizontalFrame *messageframe = new FXHorizontalFrame(eventsframe, LAYOUT_FILL_X|PACK_UNIFORM_HEIGHT);
    (new FXTextField(messageframe, 30, &m_targetPathMessage, FXDataTarget::ID_VALUE, TEXTFIELD_READONLY|FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X))->disable();
    m_selectMessage = new FXButton(messageframe, "...", NULL, this, ID_SELECTMESSAGE, FRAME_RAISED|FRAME_THICK);
    m_playMessage = new FXButton(messageframe, _("\tPlay"), ICO_PLAY, this, ID_PLAYMESSAGE, FRAME_RAISED|FRAME_THICK);
    if(!m_soundMessage || !m_sounds)
    {
        m_selectMessage->disable();
        m_playMessage->disable();
    }
    if(!FXStat::exists(m_pathMessage))
        m_playMessage->disable();

    FXVerticalFrame *smileypane = new FXVerticalFrame(switcher, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(smileypane, _("Smileys settings"), NULL, LAYOUT_LEFT);
    new FXHorizontalSeparator(smileypane, SEPARATOR_LINE|LAYOUT_FILL_X);
    new FXCheckButton(smileypane, _("Use smileys"), &m_targetUseSmileys, FXDataTarget::ID_VALUE, CHECKBUTTON_NORMAL|LAYOUT_FILL_X|LAYOUT_SIDE_LEFT|JUSTIFY_LEFT);
    FXGroupBox *smileysgroup = new FXGroupBox(smileypane, _("Smileys"), FRAME_GROOVE|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    FXVerticalFrame *smileysbuttons = new FXVerticalFrame(smileysgroup, LAYOUT_SIDE_RIGHT|LAYOUT_FILL_Y|PACK_UNIFORM_WIDTH);
    m_addSmiley = new FXButton(smileysbuttons, _("Add"), NULL, this, ID_ADDSMILEY, FRAME_RAISED|FRAME_THICK);
    m_modifySmiley = new FXButton(smileysbuttons, _("Modify"), NULL, this, ID_MODIFYSMILEY, FRAME_RAISED|FRAME_THICK);
    m_deleteSmiley = new FXButton(smileysbuttons, _("Delete"), NULL, this, ID_DELETESMILEY, FRAME_RAISED|FRAME_THICK);
    m_importSmiley = new FXButton(smileysbuttons, _("Import"), NULL, this, ID_IMPORTSMILEY, FRAME_RAISED|FRAME_THICK);
    m_exportSmiley = new FXButton(smileysbuttons, _("Export"), NULL, this, ID_EXPORTSMILEY, FRAME_RAISED|FRAME_THICK);
    if(m_useSmileys)
    {
        m_addSmiley->enable();
        m_importSmiley->enable();
        if((FXint)m_smileysMap.size())
        {
            m_modifySmiley->enable();
            m_deleteSmiley->enable();
            m_exportSmiley->enable();
        }
        else
        {
            m_modifySmiley->disable();
            m_deleteSmiley->disable();
            m_exportSmiley->disable();
        }
    }
    else
    {
        m_addSmiley->disable();
        m_modifySmiley->disable();
        m_deleteSmiley->disable();
        m_importSmiley->disable();
        m_exportSmiley->disable();
    }
    FXVerticalFrame *smileyspane = new FXVerticalFrame(smileysgroup, LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN|FRAME_THICK);
    FXHorizontalFrame *smileysframe = new FXHorizontalFrame(smileyspane, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    m_smileys = new FXList(smileysframe, this, ID_SMILEY, LIST_BROWSESELECT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    fillSmileys();
    
    new FXButton(buttonframe, _("&General"), NULL, switcher, FXSwitcher::ID_OPEN_THIRD, FRAME_RAISED);
    new FXButton(buttonframe, _("&Look"), NULL, switcher, FXSwitcher::ID_OPEN_FOURTH, FRAME_RAISED);
    new FXButton(buttonframe, _("&Irc Text"), NULL, switcher, FXSwitcher::ID_OPEN_FIRST, FRAME_RAISED);
    new FXButton(buttonframe, _("I&gnore"), NULL, switcher, FXSwitcher::ID_OPEN_SECOND, FRAME_RAISED);
    new FXButton(buttonframe, _("&DCC"), NULL, switcher, FXSwitcher::ID_OPEN_FIFTH, FRAME_RAISED);
    new FXButton(buttonframe, _("S&ounds"), NULL, switcher, FXSwitcher::ID_OPEN_SIXTH, FRAME_RAISED);
    new FXButton(buttonframe, _("S&mileys"), NULL, switcher, FXSwitcher::ID_OPEN_SEVENTH, FRAME_RAISED);
    switcher->setCurrent(2);

    for(int i=0; i<6; i++)
    {
        m_textStyle[i].normalForeColor = m_colors.text;
        m_textStyle[i].normalBackColor = m_colors.back;
        m_textStyle[i].selectForeColor = getApp()->getSelforeColor();
        m_textStyle[i].selectBackColor = getApp()->getSelbackColor();
        m_textStyle[i].hiliteForeColor = getApp()->getHiliteColor();
        m_textStyle[i].hiliteBackColor = FXRGB(255, 128, 128); // from FXText.cpp
        m_textStyle[i].activeBackColor = m_colors.back;
        m_textStyle[i].style = 0;
    }
    //user commands
    m_textStyle[0].normalForeColor = m_colors.user;
    //Actions
    m_textStyle[1].normalForeColor = m_colors.action;
    //Notice
    m_textStyle[2].normalForeColor = m_colors.notice;
    //Errors
    m_textStyle[3].normalForeColor = m_colors.error;
    //Highlight
    m_textStyle[4].normalForeColor = m_colors.hilight;
    //link
    m_textStyle[5].normalForeColor = m_colors.link;
    m_textStyle[5].style = FXText::STYLE_UNDERLINE;

    m_text->setStyled(TRUE);
    m_text->setHiliteStyles(m_textStyle);
    m_text->setTextColor(m_colors.text);
    m_text->setBackColor(m_colors.back);
    m_text->appendText("[00:00:00] ");
    m_text->appendStyledText(FXString("dvx has joined to #dxirc\n"), 1);
    m_text->appendText("[00:00:00] <dvx> Welcome in dxirc\n");
    m_text->appendText("[00:00:00] ");
    m_text->appendStyledText(FXString("<bm> dvx, dxirc is nice\n"), 5);
    m_text->appendText("[00:00:00] ");
    m_text->appendStyledText(FXString("dvx is online\n"), 2);
    m_text->appendText("[00:00:00] ");
    m_text->appendStyledText(FXString("server's NOTICE: nice notice\n"), 3);
    m_text->appendText("[00:00:00] ");
    m_text->appendStyledText(FXString("No error\n"), 4);
    m_text->appendText("[00:00:00] <dvx> ");
    m_text->appendStyledText(FXString("http://dxirc.org"), 6);
    m_text->appendText(" \n");

    updateColors();
    updateFont();
    updateIrcFont();
}


ConfigDialog::~ConfigDialog()
{
    delete m_font;
    delete m_ircFont;
}

long ConfigDialog::onCommandsSelected(FXObject*, FXSelector, void*)
{
    m_deleteCommand->enable();
    return 1;
}

long ConfigDialog::onCommandsDeselected(FXObject*, FXSelector, void*)
{
    m_deleteCommand->disable();
    return 1;
}

long ConfigDialog::onUsersSelected(FXObject*, FXSelector, void *ptr)
{
    FXint i = (FXint)(FXival)ptr;
    m_users->selectItem(i);
    m_deleteUser->enable();
    return 1;
}

long ConfigDialog::onUsersDeselected(FXObject*, FXSelector, void*)
{
    m_deleteUser->disable();
    return 1;
}

long ConfigDialog::onUsersChanged(FXObject*, FXSelector, void *ptr)
{
    FXint i = (FXint)(FXival)ptr;
    m_users->selectItem(i);
    return 1;
}

long ConfigDialog::onFriendsSelected(FXObject*, FXSelector, void *ptr)
{
    FXint i = (FXint)(FXival)ptr;
    m_friends->selectItem(i);
    m_deleteFriend->enable();
    return 1;
}

long ConfigDialog::onFriendsDeselected(FXObject*, FXSelector, void*)
{
    m_deleteFriend->disable();
    return 1;
}

long ConfigDialog::onFriendsChanged(FXObject*, FXSelector, void *ptr)
{
    FXint i = (FXint)(FXival)ptr;
    m_friends->selectItem(i);
    return 1;
}

long ConfigDialog::onColor(FXObject*, FXSelector, void*)
{
    m_text->setTextColor(m_colors.text);
    m_text->setBackColor(m_colors.back);
    for(int i=0; i<6; i++)
    {
        m_textStyle[i].normalBackColor = m_colors.back;
    }
    m_textStyle[0].normalForeColor = m_colors.user;
    m_textStyle[1].normalForeColor = m_colors.action;
    m_textStyle[2].normalForeColor = m_colors.notice;
    m_textStyle[3].normalForeColor = m_colors.error;
    m_textStyle[4].normalForeColor = m_colors.hilight;
    m_textStyle[5].normalForeColor = m_colors.link;
    m_text->update();
    return 1;
}

long ConfigDialog::onTabPosition(FX::FXObject *, FX::FXSelector, void *ptr)
{
    m_tabPosition = (FXint)(FXival)ptr;
    return 1;
}

long ConfigDialog::onAccept(FXObject*, FXSelector, void*)
{
    getApp()->stopModal(this,TRUE);
    saveConfig();
    hide();
    return 1;
}

long ConfigDialog::onCancel(FXObject*,FXSelector,void*)
{
    getApp()->stopModal(this,FALSE);
    hide();
    return 1;
}

long ConfigDialog::onKeyPress(FXObject *sender,FXSelector sel,void *ptr)
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

long ConfigDialog::onAddCommand(FXObject*, FXSelector, void*)
{
    FXDialogBox dialog(this, _("Select ignore command"), DECOR_TITLE|DECOR_BORDER, 0,0,0,0, 0,0,0,0, 0,0);
    FXVerticalFrame *contents = new FXVerticalFrame(&dialog, LAYOUT_SIDE_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 10,10,10,10, 0,0);
    FXMatrix *matrix = new FXMatrix(contents,2,MATRIX_BY_COLUMNS|LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y);

    new FXLabel(matrix, _("Command:"),NULL,JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXComboBox *command = new FXComboBox(matrix, 1, NULL, 0, COMBOBOX_STATIC|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|FRAME_GROOVE);
    command->fillItems(fillCommandsCombo());

    FXHorizontalFrame *buttonframe = new FXHorizontalFrame(contents,LAYOUT_FILL_X|LAYOUT_FILL_Y|PACK_UNIFORM_WIDTH);
    new FXButton(buttonframe, _("&Cancel"), NULL, &dialog, FXDialogBox::ID_CANCEL, FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,5);
    new FXButton(buttonframe, _("&OK"), NULL, &dialog, FXDialogBox::ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,5);

    if(dialog.execute(PLACEMENT_CURSOR))
    {
        m_commands->appendItem(command->getItemText(command->getCurrentItem()));
    }
    updateCommands();
    return 1;
}

long ConfigDialog::onDeleteCommand(FXObject*, FXSelector, void*)
{
    FXint i = m_commands->getCurrentItem();
    m_commands->removeItem(i);
    m_commands->getNumItems() ? m_deleteCommand->enable() : m_deleteCommand->disable();
    updateCommands();
    return 1;
}

long ConfigDialog::onAddUser(FXObject*, FXSelector, void*)
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
        if(!nick->getText().empty() && !nickExist(nick->getText()))
        {
            IgnoreUser user;
            user.nick = nick->getText();
            channel->getText().empty() ? user.channel = "all" : user.channel = channel->getText();
            server->getText().empty() ? user.server = "all" : user.server = server->getText();
            m_usersList.append(user);
            m_users->appendItem(user.nick+"\t"+user.channel+"\t"+user.server);
            if(!m_deleteUser->isEnabled()) m_deleteUser->enable();
            if(!m_modifyUser->isEnabled()) m_modifyUser->enable();
        }
    }
    return 1;
}

long ConfigDialog::onModifyUser(FXObject*, FXSelector, void*)
{
    FXint i = m_users->getCurrentItem();
    FXString oldnick = m_usersList[i].nick;

    FXDialogBox dialog(this, _("Modify ignore user"), DECOR_TITLE|DECOR_BORDER, 0,0,0,0, 0,0,0,0, 0,0);
    FXVerticalFrame *contents = new FXVerticalFrame(&dialog, LAYOUT_SIDE_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 10,10,10,10, 0,0);
    FXMatrix *matrix = new FXMatrix(contents,2,MATRIX_BY_COLUMNS|LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y);

    new FXLabel(matrix, _("Nick:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *nick = new FXTextField(matrix, 25, NULL, 0, FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    nick->setText(oldnick);
    new FXLabel(matrix, _("Channel(s):\tChannels need to be comma separated"), NULL,JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *channel = new FXTextField(matrix, 25, NULL, 0, FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    channel->setText(m_usersList[i].channel);
    new FXLabel(matrix, _("Server:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *server = new FXTextField(matrix, 25, NULL, 0, FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    server->setText(m_usersList[i].server);

    FXHorizontalFrame *buttonframe = new FXHorizontalFrame(contents,LAYOUT_FILL_X|LAYOUT_FILL_Y|PACK_UNIFORM_WIDTH);
    new FXButton(buttonframe, _("&Cancel"), NULL, &dialog, FXDialogBox::ID_CANCEL, FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,5);
    new FXButton(buttonframe, _("&OK"), NULL, &dialog, FXDialogBox::ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,5);

    if(dialog.execute(PLACEMENT_CURSOR))
    {
        if(!nick->getText().empty() && (!nickExist(nick->getText()) || oldnick == nick->getText()))
        {
            m_usersList[i].nick = nick->getText();
            channel->getText().empty() ? m_usersList[i].channel = "all" : m_usersList[i].channel = channel->getText();
            server->getText().empty() ? m_usersList[i].server = "all" : m_usersList[i].server = server->getText();
            m_users->setItemText(i, m_usersList[i].nick+"\t"+m_usersList[i].channel+"\t"+m_usersList[i].server);
        }
    }
    return 1;
}

long ConfigDialog::onDeleteUser(FXObject*, FXSelector, void*)
{
    FXint i = m_users->getCurrentItem();
    m_users->removeItem(i);
    m_usersList.erase(i);
    if(m_usersList.no())
    {
        m_deleteUser->enable();
        m_modifyUser->enable();
    }
    else
    {
        m_deleteUser->disable();
        m_modifyUser->disable();
    }
    return 1;
}

long ConfigDialog::onImportSmiley(FXObject*, FXSelector, void*)
{
    if(m_showImportwarning && m_smileysMap.size())
        if(FXMessageBox::warning(this, MBOX_OK_CANCEL, _("Warning"), _("File import overwrites current settings"))==4) {m_showImportwarning=FALSE; return 1;}
    FXFileDialog dialog(this, _("Select file"));
    if(m_showImportwarning) dialog.setFilename((FXString)DXIRC_DATADIR+PATHSEPSTRING+"icons"+PATHSEPSTRING+"smileys"+PATHSEPSTRING+"dxirc.smiley");
    if(dialog.execute())
    {
        m_smileys->clearItems();
        m_smileysMap.clear();
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
                pathstr = dequote(line+path);
                if(pathstr.empty()) goto next;
                smileystr = dequote(line+smiley);
                if(smileystr.empty()) goto next;
                pathstr = FXPath::absolute(FXPath::directory(dialog.getFilename()), pathstr);
                m_smileysMap.insert(StringPair(smileystr,pathstr));

next:           bol=eol;
            }
        }
        fillSmileys();
        if(m_smileysMap.size())
        {
            m_modifySmiley->enable();
            m_deleteSmiley->enable();
            m_exportSmiley->enable();
        }
    }
    m_showImportwarning=FALSE;
    return 1;
}

long ConfigDialog::onExportSmiley(FXObject*, FXSelector, void*)
{
    FXFileDialog dialog(this, _("Save smiley settings as"));
    if(dialog.execute())
    {
        FXFile file(dialog.getFilename(), FXIO::Writing);
        FXchar line[MAXVALUE];
        if(file.isOpen())
        {
            writeString(file, HEADER);
            if((FXint)m_smileysMap.size())
            {
                StringIt it;
                FXint i;
                for(i=0, it=m_smileysMap.begin(); it!=m_smileysMap.end(); it++,i++)
                {
                    writeString(file, enquote(line, FXPath::relative(FXPath::directory(dialog.getFilename()),(*it).second).text()));
                    writeString(file, "=");
                    writeString(file, enquote(line, (*it).first.text()));
                    writeString(file, ENDLINE);
                }
            }
        }
    }
    return 1;
}

// Enquote a value
FXchar* ConfigDialog::enquote(FXchar* result, const FXchar* text)
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
FXchar* ConfigDialog::dequote(FXchar* text) const
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

long ConfigDialog::onAddFriend(FXObject*, FXSelector, void*)
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
        if(!nick->getText().empty() && !nickExist(nick->getText(), FALSE))
        {
            IgnoreUser user;
            user.nick = nick->getText();
            channel->getText().empty() ? user.channel = "all" : user.channel = channel->getText();
            server->getText().empty() ? user.server = "all" : user.server = server->getText();
            m_friendsList.append(user);
            m_friends->appendItem(user.nick+"\t"+user.channel+"\t"+user.server);
            if(!m_deleteFriend->isEnabled()) m_deleteFriend->enable();
            if(!m_modifyFriend->isEnabled()) m_modifyFriend->enable();
        }
    }
    return 1;
}

long ConfigDialog::onModifyFriend(FXObject*, FXSelector, void*)
{
    FXint i = m_friends->getCurrentItem();
    FXString oldnick = m_friendsList[i].nick;

    FXDialogBox dialog(this, _("Modify friend"), DECOR_TITLE|DECOR_BORDER, 0,0,0,0, 0,0,0,0, 0,0);
    FXVerticalFrame *contents = new FXVerticalFrame(&dialog, LAYOUT_SIDE_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 10,10,10,10, 0,0);
    FXMatrix *matrix = new FXMatrix(contents,2,MATRIX_BY_COLUMNS|LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y);

    new FXLabel(matrix, _("Nick:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *nick = new FXTextField(matrix, 25, NULL, 0, FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    nick->setText(oldnick);
    new FXLabel(matrix, _("Channel(s):\tChannels need to be comma separated"), NULL,JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *channel = new FXTextField(matrix, 25, NULL, 0, FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    channel->setText(m_friendsList[i].channel);
    new FXLabel(matrix, _("Server:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *server = new FXTextField(matrix, 25, NULL, 0, FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    server->setText(m_friendsList[i].server);

    FXHorizontalFrame *buttonframe = new FXHorizontalFrame(contents,LAYOUT_FILL_X|LAYOUT_FILL_Y|PACK_UNIFORM_WIDTH);
    new FXButton(buttonframe, _("&Cancel"), NULL, &dialog, FXDialogBox::ID_CANCEL, FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,5);
    new FXButton(buttonframe, _("&OK"), NULL, &dialog, FXDialogBox::ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,5);

    if(dialog.execute(PLACEMENT_CURSOR))
    {
        if(!nick->getText().empty() && (!nickExist(nick->getText(), FALSE) || oldnick == nick->getText()))
        {
            m_friendsList[i].nick = nick->getText();
            channel->getText().empty() ? m_friendsList[i].channel = "all" : m_friendsList[i].channel = channel->getText();
            server->getText().empty() ? m_friendsList[i].server = "all" : m_friendsList[i].server = server->getText();
            m_friends->setItemText(i, m_friendsList[i].nick+"\t"+m_friendsList[i].channel+"\t"+m_friendsList[i].server);
        }
    }
    return 1;
}

long ConfigDialog::onDeleteFriend(FXObject*, FXSelector, void*)
{
    FXint i = m_friends->getCurrentItem();
    m_friends->removeItem(i);
    m_friendsList.erase(i);
    if(m_friendsList.no())
    {
        m_deleteFriend->enable();
        m_modifyFriend->enable();
    }
    else
    {
        m_deleteFriend->disable();
        m_modifyFriend->disable();
    }
    return 1;
}

long ConfigDialog::onAddIcons(FXObject*, FXSelector, void*)
{
    FXDirDialog dirdialog(this, _("Select theme directory"));
    if(dirdialog.execute(PLACEMENT_CURSOR))
    {
        if(!FXPath::search(dirdialog.getDirectory(), "irc_normal.png").empty() && !themeExist(dirdialog.getDirectory())) m_icons->appendItem(dirdialog.getDirectory());
    }
    m_icons->getNumItems()>1 ? m_deleteTheme->enable() : m_deleteTheme->disable();
    updateIcons();
    return 1;
}

long ConfigDialog::onDeleteIcons(FXObject*, FXSelector, void*)
{
    FXint i = m_icons->getCurrentItem();
    m_icons->removeItem(i);
    i = m_icons->getCurrentItem();
    m_themePath = m_icons->getItemText(i);
    m_icon1->setIcon(makeIcon(getApp(), m_themePath, "irc_admin.png", TRUE));
    m_icon2->setIcon(makeIcon(getApp(), m_themePath, "irc_owner.png", TRUE));
    m_icon3->setIcon(makeIcon(getApp(), m_themePath, "irc_op.png", TRUE));
    m_icon4->setIcon(makeIcon(getApp(), m_themePath, "irc_halfop.png", TRUE));
    m_icon5->setIcon(makeIcon(getApp(), m_themePath, "irc_voice.png", TRUE));
    m_icon6->setIcon(makeIcon(getApp(), m_themePath, "irc_normal.png", TRUE));
    m_icon7->setIcon(makeAwayIcon(getApp(), m_themePath, "irc_normal.png"));
    m_icons->getNumItems()>1 ? m_deleteTheme->enable() : m_deleteTheme->disable();
    updateIcons();
    return 1;
}

long ConfigDialog::onIconsChanged(FXObject*, FXSelector, void *ptr)
{
    FXint i = (FXint)(FXival)ptr;
    m_themePath = m_icons->getItemText(i);
    m_icon1->setIcon(makeIcon(getApp(), m_themePath, "irc_admin.png", TRUE));
    m_icon2->setIcon(makeIcon(getApp(), m_themePath, "irc_owner.png", TRUE));
    m_icon3->setIcon(makeIcon(getApp(), m_themePath, "irc_op.png", TRUE));
    m_icon4->setIcon(makeIcon(getApp(), m_themePath, "irc_halfop.png", TRUE));
    m_icon5->setIcon(makeIcon(getApp(), m_themePath, "irc_voice.png", TRUE));
    m_icon6->setIcon(makeIcon(getApp(), m_themePath, "irc_normal.png", TRUE));
    m_icon7->setIcon(makeAwayIcon(getApp(), m_themePath, "irc_normal.png"));
    showMessage();
    return 1;
}

long ConfigDialog::onTheme(FXObject*, FXSelector, void *ptr)
{
    FXint no = (FXint)(FXival)ptr;
    ColorTheme *themeSelected = reinterpret_cast<ColorTheme*>(m_themes->getItemData(no));
    if(themeSelected)
    {
        m_themeCurrent.back = themeSelected->back;
        m_themeCurrent.base = themeSelected->base;
        m_themeCurrent.border = themeSelected->border;
        m_themeCurrent.fore = themeSelected->fore;
        m_themeCurrent.menuback = themeSelected->menuback;
        m_themeCurrent.menufore = themeSelected->menufore;
        m_themeCurrent.selback = themeSelected->selback;
        m_themeCurrent.selfore = themeSelected->selfore;
        m_themeCurrent.tipback = themeSelected->tipback;
        m_themeCurrent.tipfore = themeSelected->tipfore;
        updateColors();
    }
    return 1;
}

long ConfigDialog::onFont(FXObject*, FXSelector, void*)
{
    FXFontDialog dialog(this, _("Select font"));
    FXFontDesc fontdescription;
    m_font->getFontDesc(fontdescription);
    strncpy(fontdescription.face,m_font->getActualName().text(),sizeof(fontdescription.face));
    dialog.setFontSelection(fontdescription);
    if(dialog.execute(PLACEMENT_SCREEN))
    {
        FXFont *oldfont = m_font;
        dialog.getFontSelection(fontdescription);
        m_font = new FXFont(getApp(),fontdescription);
        m_font->create();
        delete oldfont;
        updateFont();
    }
    return 1;
}

long ConfigDialog::onIrcFont(FXObject*, FXSelector, void*)
{
    FXFontDialog dialog(this, _("Select font"));
    FXFontDesc fontdescription;
    m_ircFont->getFontDesc(fontdescription);
    strncpy(fontdescription.face,m_ircFont->getActualName().text(),sizeof(fontdescription.face));
    dialog.setFontSelection(fontdescription);
    if(dialog.execute(PLACEMENT_SCREEN))
    {
        FXFont *oldfont = m_ircFont;
        dialog.getFontSelection(fontdescription);
        m_ircFont = new FXFont(getApp(),fontdescription);
        m_ircFont->create();
        delete oldfont;
        updateIrcFont();
    }
    return 1;
}

long ConfigDialog::onThemeColorChanged(FXObject*, FXSelector, void*)
{
    m_themes->setCurrentItem(m_themes->getNumItems()-1);
    updateColors();
    return 1;
}

void ConfigDialog::updateColors()
{
    m_themeCurrent.shadow = makeShadowColor(m_themeCurrent.base);
    m_themeCurrent.hilite = makeHiliteColor(m_themeCurrent.base);

    m_vframe2->setBorderColor(m_themeCurrent.border);
    m_vframe2->setBaseColor(m_themeCurrent.base);
    m_vframe2->setBackColor(m_themeCurrent.base);
    m_vframe2->setShadowColor(m_themeCurrent.shadow);
    m_vframe2->setHiliteColor(m_themeCurrent.hilite);

    m_label->setBorderColor(m_themeCurrent.border);
    m_label->setBaseColor(m_themeCurrent.base);
    m_label->setBackColor(m_themeCurrent.base);
    m_label->setTextColor(m_themeCurrent.fore);
    m_label->setShadowColor(m_themeCurrent.shadow);
    m_label->setHiliteColor(m_themeCurrent.hilite);

    m_textFrame1->setBorderColor(m_themeCurrent.border);
    m_textFrame1->setBaseColor(m_themeCurrent.base);
    m_textFrame1->setBackColor(m_themeCurrent.base);
    m_textFrame1->setShadowColor(m_themeCurrent.shadow);
    m_textFrame1->setHiliteColor(m_themeCurrent.hilite);
    m_textTest->setBorderColor(m_themeCurrent.border);
    m_textTest->setBackColor(m_themeCurrent.back);
    m_textTest->setBaseColor(m_themeCurrent.base);
    m_textTest->setTextColor(m_themeCurrent.fore);
    m_textTest->setSelTextColor(m_themeCurrent.selfore);
    m_textTest->setSelBackColor(m_themeCurrent.selback);
    m_textTest->setCursorColor(m_themeCurrent.fore);
    m_textTest->setShadowColor(m_themeCurrent.shadow);
    m_textTest->setHiliteColor(m_themeCurrent.hilite);

    m_textFrame2->setBorderColor(m_themeCurrent.border);
    m_textFrame2->setBaseColor(m_themeCurrent.base);
    m_textFrame2->setBackColor(m_themeCurrent.back);
    m_textFrame2->setShadowColor(m_themeCurrent.shadow);
    m_textFrame2->setHiliteColor(m_themeCurrent.hilite);
    m_labelSelected->setBorderColor(m_themeCurrent.border);
    m_labelSelected->setBaseColor(m_themeCurrent.base);
    m_labelSelected->setBackColor(m_themeCurrent.selback);
    m_labelSelected->setTextColor(m_themeCurrent.selfore);
    m_labelSelected->setShadowColor(m_themeCurrent.shadow);
    m_labelSelected->setHiliteColor(m_themeCurrent.hilite);

    m_textFrame3->setBorderColor(m_themeCurrent.border);
    m_textFrame3->setBaseColor(m_themeCurrent.base);
    m_textFrame3->setBackColor(m_themeCurrent.back);
    m_textFrame3->setShadowColor(m_themeCurrent.shadow);
    m_textFrame3->setHiliteColor(m_themeCurrent.hilite);
    m_labelNocurrent->setBorderColor(m_themeCurrent.border);
    m_labelNocurrent->setBaseColor(m_themeCurrent.base);
    m_labelNocurrent->setBackColor(m_themeCurrent.base);
    m_labelNocurrent->setTextColor(m_themeCurrent.fore);
    m_labelNocurrent->setShadowColor(m_themeCurrent.shadow);
    m_labelNocurrent->setHiliteColor(m_themeCurrent.hilite);

    m_sep1->setBorderColor(m_themeCurrent.border);
    m_sep1->setBaseColor(m_themeCurrent.base);
    m_sep1->setBackColor(m_themeCurrent.base);
    m_sep1->setShadowColor(m_themeCurrent.shadow);
    m_sep1->setHiliteColor(m_themeCurrent.hilite);

    m_labelTip->setBorderColor(m_themeCurrent.tipfore);
    m_labelTip->setBaseColor(m_themeCurrent.tipback);
    m_labelTip->setBackColor(m_themeCurrent.tipback);
    m_labelTip->setTextColor(m_themeCurrent.tipfore);
    m_labelTip->setShadowColor(m_themeCurrent.shadow);
    m_labelTip->setHiliteColor(m_themeCurrent.hilite);
    
    m_menuGroup->setBorderColor(m_themeCurrent.border);
    m_menuGroup->setBaseColor(m_themeCurrent.base);
    m_menuGroup->setBackColor(m_themeCurrent.base);
    m_menuGroup->setShadowColor(m_themeCurrent.shadow);
    m_menuGroup->setHiliteColor(m_themeCurrent.hilite);
    m_menuGroup->setTextColor(m_themeCurrent.fore);
    m_menuFrame->setBorderColor(m_themeCurrent.border);
    m_menuFrame->setBaseColor(m_themeCurrent.base);
    m_menuFrame->setBackColor(m_themeCurrent.base);
    m_menuFrame->setShadowColor(m_themeCurrent.shadow);
    m_menuFrame->setHiliteColor(m_themeCurrent.hilite);
    m_sep2->setBorderColor(m_themeCurrent.border);
    m_sep2->setBaseColor(m_themeCurrent.base);
    m_sep2->setBackColor(m_themeCurrent.base);
    m_sep2->setShadowColor(m_themeCurrent.shadow);
    m_sep2->setHiliteColor(m_themeCurrent.hilite);
    for(int i=0; i<3; i++)
    {
        m_menuLabels[i]->setBorderColor(m_themeCurrent.border);
        m_menuLabels[i]->setBaseColor(m_themeCurrent.base);
        m_menuLabels[i]->setBackColor(m_themeCurrent.base);
        m_menuLabels[i]->setTextColor(m_themeCurrent.fore);
        m_menuLabels[i]->setShadowColor(m_themeCurrent.shadow);
        m_menuLabels[i]->setHiliteColor(m_themeCurrent.hilite);
    }
    m_menuLabels[1]->setBorderColor(m_themeCurrent.border);
    m_menuLabels[1]->setBaseColor(m_themeCurrent.menuback);
    m_menuLabels[1]->setBackColor(m_themeCurrent.menuback);
    m_menuLabels[1]->setTextColor(m_themeCurrent.menufore);
    m_menuLabels[1]->setShadowColor(m_themeCurrent.shadow);
    m_menuLabels[1]->setHiliteColor(m_themeCurrent.hilite);
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

void ConfigDialog::updateFont()
{
    FXString fontname = m_font->getActualName() +", " + FXStringVal(m_font->getSize()/10);
    if(m_font->getWeight()!=0 && m_font->getWeight()!=FXFont::Normal)
    {
        fontname += ", " + weightToString(m_font->getWeight());
    }
    if (m_font->getSlant()!=0 && m_font->getSlant()!=FXFont::Straight)
    {
        fontname += ", " + slantToString(m_font->getSlant());
    }
    m_fontButton->setText(fontname);
    
    m_labelSelected->setFont(m_font);
    m_labelNocurrent->setFont(m_font);
    m_labelTip->setFont(m_font);
    m_label->setFont(m_font);
    m_textTest->setFont(m_font);
    m_menuGroup->setFont(m_font);
    m_menuLabels[0]->setFont(m_font);
    m_menuLabels[1]->setFont(m_font);
    m_menuLabels[2]->setFont(m_font);
}

void ConfigDialog::updateIrcFont()
{
    FXString fontname = m_ircFont->getActualName() +", " + FXStringVal(m_ircFont->getSize()/10);
    if(m_ircFont->getWeight()!=0 && m_ircFont->getWeight()!=FXFont::Normal)
    {
        fontname += ", " + weightToString(m_ircFont->getWeight());
    }
    if (m_ircFont->getSlant()!=0 && m_ircFont->getSlant()!=FXFont::Straight)
    {
        fontname += ", " + slantToString(m_ircFont->getSlant());
    }
    m_ircfontButton->setText(fontname);

    m_text->setFont(m_ircFont);
}

long ConfigDialog::onLogChanged(FXObject*, FXSelector, void*)
{
    if(m_logging) m_selectPath->enable();
    else m_selectPath->disable();
    return 1;
}

long ConfigDialog::onAutoloadChanged(FXObject*, FXSelector, void*)
{
    if(m_autoload) m_selectAutoloadPath->enable();
    else m_selectAutoloadPath->disable();
    return 1;
}

long ConfigDialog::onNickCharChanged(FXObject*, FXSelector, void*)
{
    m_nickChar = m_nickCharField->getText().left(1);
    m_nickCharField->setText(m_nickChar);
    return 1;
}

long ConfigDialog::onServerWindow(FXObject*, FXSelector, void*)
{
    showMessage();
    return 1;
}

long ConfigDialog::onTray(FXObject*, FXSelector, void*)
{
    if(m_useTray)
        m_closeToTrayButton->enable();
    else
    {
        m_closeToTray = FALSE;
        m_closeToTrayButton->disable();
    }
    showMessage();
    return 1;
}

long ConfigDialog::onReconnect(FXObject*, FXSelector, void*)
{
    if(m_reconnect)
    {
        m_numberAttemptLabel->enable();
        m_numberAttemptSpinner->enable();
        m_delayAttemptLabel->enable();
        m_delayAttemptSpinner->enable();
    }
    else
    {
        m_numberAttemptLabel->disable();
        m_numberAttemptSpinner->disable();
        m_delayAttemptLabel->disable();
        m_delayAttemptSpinner->disable();
    }
    return 1;
}

long ConfigDialog::onPathSelect(FXObject*, FXSelector, void*)
{
    FXDirDialog dirdialog(this,_("Select log directory"));
    dirdialog.setDirectory(m_logPath);
    if(dirdialog.execute(PLACEMENT_CURSOR))
    {
        m_logPath = dirdialog.getDirectory();
    }
    return 1;
}

long ConfigDialog::onAutoloadPathSelect(FXObject*, FXSelector, void*)
{
    FXDirDialog dirdialog(this,_("Select autoload directory"));
    dirdialog.setDirectory(m_autoloadPath);
    if(dirdialog.execute(PLACEMENT_CURSOR))
    {
        m_autoloadPath = dirdialog.getDirectory();
    }
    return 1;
}

long ConfigDialog::onDccPathSelect(FXObject*, FXSelector, void*)
{
    FXDirDialog dirdialog(this,_("Select directory"));
    dirdialog.setDirectory(m_dccPath);
    if(dirdialog.execute(PLACEMENT_CURSOR))
    {
        m_dccPath = dirdialog.getDirectory();
    }
    return 1;
}

long ConfigDialog::onDccPortD(FXObject*, FXSelector, void*)
{
    if(m_dccPortD<0 || m_dccPortD>65536) m_dccPortD = 0;
    if(m_dccPortH<m_dccPortD) m_dccPortH = m_dccPortD;
    return 1;
}

long ConfigDialog::onDccPortH(FXObject*, FXSelector, void*)
{
    if(m_dccPortH<0 || m_dccPortH>65536) m_dccPortH = 0;
    if(m_dccPortD>m_dccPortH) m_dccPortD = m_dccPortH;
    return 1;
}

long ConfigDialog::onSounds(FXObject*, FXSelector, void*)
{
    if(m_sounds)
    {
        m_checkConnect->enable();
        m_selectConnect->enable();
        if(FXStat::exists(m_pathConnect)) m_playConnect->enable();
        m_checkDisconnect->enable();
        m_selectDisconnect->enable();
        if(FXStat::exists(m_pathDisconnect)) m_playDisconnect->enable();
        m_checkMessage->enable();
        m_selectMessage->enable();
        if(FXStat::exists(m_pathMessage)) m_playMessage->enable();
        m_addFriend->enable();
        if(m_friendsList.no())
        {
            m_deleteFriend->enable();
            m_modifyFriend->enable();
        }
    }
    else
    {
        m_checkConnect->disable();
        m_selectConnect->disable();
        m_playConnect->disable();
        m_checkDisconnect->disable();
        m_selectDisconnect->disable();
        m_playDisconnect->disable();
        m_checkMessage->disable();
        m_selectMessage->disable();
        m_playMessage->disable();
        m_addFriend->disable();
        m_modifyFriend->disable();
        m_deleteFriend->disable();
    }
    return 1;
}

long ConfigDialog::onSoundConnect(FXObject*, FXSelector, void*)
{
    if(m_soundConnect)
    {
        m_selectConnect->enable();
        if(FXStat::exists(m_pathConnect)) m_playConnect->enable();
    }
    else
    {
        m_selectConnect->disable();
        m_playConnect->enable();
    }
    return 1;
}

long ConfigDialog::onSoundDisconnect(FXObject*, FXSelector, void*)
{
    if(m_soundDisconnect)
    {
        m_selectDisconnect->enable();
        if(FXStat::exists(m_pathDisconnect)) m_playDisconnect->enable();
    }
    else
    {
        m_selectDisconnect->disable();
        m_playDisconnect->enable();
    }
    return 1;
}

long ConfigDialog::onSoundMessage(FXObject*, FXSelector, void*)
{
    if(m_soundMessage)
    {
        m_selectMessage->enable();
        if(FXStat::exists(m_pathMessage)) m_playMessage->enable();
    }
    else
    {
        m_selectMessage->disable();
        m_playMessage->enable();
    }
    return 1;
}

long ConfigDialog::onPlay(FXObject*, FXSelector sel, void*)
{
    switch(FXSELID(sel)) {
        case ID_PLAYCONNECT:
            utils::instance().instance().playFile(m_pathConnect);
            return 1;
        case ID_PLAYDISCONNECT:
            utils::instance().playFile(m_pathDisconnect);
            return 1;
        case ID_PLAYMESSAGE:
            utils::instance().playFile(m_pathMessage);
            return 1;
    }
    return 1;
}

long ConfigDialog::onSelectPath(FXObject*, FXSelector sel, void*)
{
    FXFileDialog file(this, _("Select file"));
    file.setPatternList(_("Sound file (*.wav)"));
    switch(FXSELID(sel)) {
        case ID_SELECTCONNECT:
            if(file.execute(PLACEMENT_CURSOR))
            {
                m_pathConnect = file.getFilename();
                m_playConnect->enable();
            }
            return 1;
        case ID_SELECTDISCONNECT:
            if(file.execute(PLACEMENT_CURSOR))
            {
                m_pathDisconnect = file.getFilename();
                m_playDisconnect->enable();
            }
            return 1;
        case ID_SELECTMESSAGE:
            if(file.execute(PLACEMENT_CURSOR))
            {
                m_pathMessage = file.getFilename();
                m_playMessage->enable();
            }
            return 1;
    }
    return 1;
}

long ConfigDialog::onUseSmileys(FXObject*, FXSelector, void*)
{
    if(m_useSmileys)
    {
        m_addSmiley->enable();
        m_importSmiley->enable();
        if((FXint)m_smileysMap.size())
        {
            m_modifySmiley->enable();
            m_deleteSmiley->enable();
            m_exportSmiley->enable();
        }
        else
        {
            m_modifySmiley->disable();
            m_deleteSmiley->disable();
            m_exportSmiley->disable();
        }
    }
    else
    {
        m_addSmiley->disable();
        m_modifySmiley->disable();
        m_deleteSmiley->disable();
        m_importSmiley->disable();
        m_exportSmiley->disable();
    }
    return 1;
}

long ConfigDialog::onAddSmiley(FXObject*, FXSelector, void*)
{
    if((FXint)m_smileysMap.size()>=256)
    {
        FXMessageBox::information(this, MBOX_OK, _("Information"), _("Maximum number of 256 smileys is reached"));
        return 1;
    }
    SmileyDialog dialog(this, _("Add smiley"), "", "");
    if(dialog.execute())
    {
        if(!dialog.iconExist())
        {
            FXMessageBox::information(this, MBOX_OK, _("Information"), _("Icon file '%s' doesn't exist or isn't image file"), dialog.getPath().text());
            return 1;
        }
        if(dialog.getSmiley().empty())
        {
            FXMessageBox::information(this, MBOX_OK, _("Information"), _("Smiley text is empty"));
            return 1;
        }
        if(smileyExist(dialog.getSmiley()))
        {
            FXMessageBox::information(this, MBOX_OK, _("Information"), _("Smiley '%s' already exist"), dialog.getSmiley().text());
            return 1;
        }
        m_smileysMap.insert(StringPair(dialog.getSmiley(), dialog.getPath()));
        m_smileys->appendItem(new FXListItem(dialog.getSmiley(), dialog.getIcon()));
        m_modifySmiley->enable();
        m_deleteSmiley->enable();
        m_exportSmiley->enable();
    }
    return 1;
}

long ConfigDialog::onModifySmiley(FXObject*, FXSelector, void*)
{
    FXint index = m_smileys->getCurrentItem();
    FXString oldkey = m_smileys->getItemText(index);
    SmileyDialog dialog(this, _("Modify smiley"), oldkey, m_smileysMap[oldkey]);
    if(dialog.execute())
    {
        if(!dialog.iconExist())
        {
            FXMessageBox::information(this, MBOX_OK, _("Information"), _("Icon file '%s' doesn't exist or isn't image file"), dialog.getPath().text());
            return 1;
        }
        if(dialog.getSmiley().empty())
        {
            FXMessageBox::information(this, MBOX_OK, _("Information"), _("Smiley text is empty"));
            return 1;
        }
        if(smileyExist(dialog.getSmiley()) && dialog.getSmiley()!=oldkey)
        {
            FXMessageBox::information(this, MBOX_OK, _("Information"), _("Smiley '%s' already exist"), dialog.getSmiley().text());
            return 1;
        }
        m_smileysMap.erase(oldkey);
        m_smileys->removeItem(index, TRUE);
        m_smileysMap.insert(StringPair(dialog.getSmiley(), dialog.getPath()));
        m_smileys->insertItem(index, new FXListItem(dialog.getSmiley(), dialog.getIcon()), TRUE);
        m_smileys->setCurrentItem(index, TRUE);
    }
    return 1;
}

long ConfigDialog::onDeleteSmiley(FXObject*, FXSelector, void*)
{
    m_smileysMap.erase(m_smileys->getItemText(m_smileys->getCurrentItem()));
    m_smileys->removeItem(m_smileys->getCurrentItem(), TRUE);
    if(!(FXint)m_smileysMap.size())
    {
        m_modifySmiley->disable();
        m_deleteSmiley->disable();
        m_exportSmiley->disable();
    }
    return 1;
}

void ConfigDialog::fillCommnads()
{
    if(!m_commandsList.empty())
    {
        for(FXint i=0; i<m_commandsList.contains(';'); i++)
        {
            m_commands->appendItem(m_commandsList.before(';', i+1).rafter(';'));
        }
    }
}

void ConfigDialog::fillIcons()
{
    if(!m_themesList.empty())
    {
        for(FXint i=0; i<m_themesList.contains(';'); i++)
        {
            m_icons->appendItem(m_themesList.before(';', i+1).rafter(';'));
        }
    }
}

void ConfigDialog::fillThemes()
{
    FXint i, scheme=-1;

    for(i=0; i<numThemes; i++)
    {
        if((m_themeCurrent.back == ColorThemes[i].back) &&
                (m_themeCurrent.base == ColorThemes[i].base) &&
                (m_themeCurrent.border == ColorThemes[i].border) &&
                (m_themeCurrent.fore == ColorThemes[i].fore) &&
                (m_themeCurrent.menuback == ColorThemes[i].menuback) &&
                (m_themeCurrent.menufore == ColorThemes[i].menufore) &&
                (m_themeCurrent.selback == ColorThemes[i].selback) &&
                (m_themeCurrent.selfore == ColorThemes[i].selfore) &&
                (m_themeCurrent.tipback == ColorThemes[i].tipback) &&
                (m_themeCurrent.tipfore == ColorThemes[i].tipfore))
        {
            scheme = i;
            break;
        }
    }

    if(scheme == -1)
    {
        m_themeUser.back = m_themeCurrent.back;
        m_themeUser.base = m_themeCurrent.base;
        m_themeUser.border = m_themeCurrent.border;
        m_themeUser.fore = m_themeCurrent.fore;
        m_themeUser.menuback = m_themeCurrent.menuback;
        m_themeUser.menufore = m_themeCurrent.menufore;
        m_themeUser.selback = m_themeCurrent.selback;
        m_themeUser.selfore = m_themeCurrent.selfore;
        m_themeUser.tipback = m_themeCurrent.tipback;
        m_themeUser.tipfore = m_themeCurrent.tipfore;
        m_themes->appendItem(_("Current"), NULL, &m_themeUser);
    }

    for(i=0; i<numThemes; i++)
    {
        m_themes->appendItem(ColorThemes[i].name,NULL,(void*)&ColorThemes[i]);
    }
    m_themes->appendItem(_("User Defined"));
    m_themes->setCurrentItem(scheme);
}

FXString ConfigDialog::fillCommandsCombo()
{
    FXString combo;
    FXString available[10] = { "away", "ban", "ctcp", "join", "me", "nick", "notice", "mode", "part", "quit"};
    for(FXint i=0; i<10; i++)
    {
        FXbool exist = FALSE;
        for(FXint j=0; j<m_commands->getNumItems(); j++)
        {
            if(m_commands->getItemText(j) == available[i])
            {
                exist = TRUE;
                break;
            }
        }
        if(!exist) combo.append(available[i]+'\n');
    }
    return combo;
}

void ConfigDialog::fillUsers()
{
    if(m_usersList.no())
    {
        for(FXint i=0; i<m_usersList.no(); i++)
        {
            m_users->appendItem(m_usersList[i].nick+"\t"+m_usersList[i].channel+"\t"+m_usersList[i].server);
        }
    }
}

void ConfigDialog::fillFriends()
{
    if(m_friendsList.no())
    {
        for(FXint i=0; i<m_friendsList.no(); i++)
        {
            m_friends->appendItem(m_friendsList[i].nick+"\t"+m_friendsList[i].channel+"\t"+m_friendsList[i].server);
        }
    }
}

void ConfigDialog::fillSmileys()
{
    if((FXint)m_smileysMap.size())
    {
        StringIt it;
        for(it=m_smileysMap.begin(); it!=m_smileysMap.end(); it++)
        {
            m_smileys->appendItem(new FXListItem((*it).first, createIconFromName(getApp(), (*it).second)));
        }
    }
}

void ConfigDialog::updateCommands()
{
    m_commandsList.clear();
    for(FXint i=0; i<m_commands->getNumItems(); i++)
    {
        m_commandsList.append(m_commands->getItemText(i)+';');
    }
}

void ConfigDialog::updateIcons()
{
    m_themesList.clear();
    for(FXint i=0; i<m_icons->getNumItems(); i++)
    {
        m_themesList.append(m_icons->getItemText(i)+';');
    }
}

void ConfigDialog::showMessage()
{
    if(m_showWarning)
    {
        FXMessageBox::information(this, FX::MBOX_OK, _("Information"), _("Some changes need restart application"));
        m_showWarning = FALSE;
    }
}

FXbool ConfigDialog::themeExist(const FXString &ckdTheme)
{
    for(FXint i=0; i<m_icons->getNumItems(); i++)
    {
        if(m_icons->getItemText(i) == ckdTheme)
        {
            return TRUE;
        }
    }
    return FALSE;
}

FXbool ConfigDialog::nickExist(const FXString &ckdNick, FXbool user)
{
    for(FXint i=0; i<(user ? m_users->getNumItems() : m_friends->getNumItems()); i++)
    {
        if((user ? m_users->getItemText(i) : m_friends->getItemText(i)) == ckdNick) return TRUE;
    }
    return FALSE;
}

FXbool ConfigDialog::smileyExist(const FXString& ckdSmiley)
{
    StringIt it;
    for(it=m_smileysMap.begin(); it!=m_smileysMap.end(); it++)
    {
        if(!compare((*it).first, ckdSmiley)) return TRUE;
    }
    return FALSE;
}

void ConfigDialog::readConfig()
{
    FXString ircfontspec;
    FXSettings set;
    set.parseFile(utils::instance().getIniFile(), TRUE);
    m_themeCurrent.base = set.readColorEntry("SETTINGS", "basecolor", getApp()->getBaseColor());
    m_themeCurrent.back = set.readColorEntry("SETTINGS", "backcolor", getApp()->getBackColor());
    m_themeCurrent.border = set.readColorEntry("SETTINGS", "bordercolor", getApp()->getBorderColor());
    m_themeCurrent.fore = set.readColorEntry("SETTINGS", "forecolor", getApp()->getForeColor());
    m_themeCurrent.menuback = set.readColorEntry("SETTINGS", "selmenubackcolor", getApp()->getSelMenuBackColor());
    m_themeCurrent.menufore = set.readColorEntry("SETTINGS", "selmenutextcolor", getApp()->getSelMenuTextColor());
    m_themeCurrent.selback = set.readColorEntry("SETTINGS", "selbackcolor", getApp()->getSelbackColor());
    m_themeCurrent.selfore = set.readColorEntry("SETTINGS", "selforecolor", getApp()->getSelforeColor());
    m_themeCurrent.tipback = set.readColorEntry("SETTINGS", "tipbackcolor", getApp()->getTipbackColor());
    m_themeCurrent.tipfore = set.readColorEntry("SETTINGS", "tipforecolor", getApp()->getTipforeColor());
    m_themeCurrent.hilite = set.readColorEntry("SETTINGS", "hilitecolor", getApp()->getHiliteColor());
    m_themeCurrent.shadow = set.readColorEntry("SETTINGS", "shadowcolor", getApp()->getShadowColor());
    m_trayColor = set.readColorEntry("SETTINGS", "traycolor", m_themeCurrent.base);
    m_statusShown = set.readBoolEntry("SETTINGS", "statusShown", TRUE);
    m_tabPosition = set.readIntEntry("SETTINGS", "tabPosition", 0);
    m_commandsList = set.readStringEntry("SETTINGS", "commandsList");
    m_themePath = utils::instance().checkThemePath(set.readStringEntry("SETTINGS", "themePath", DXIRC_DATADIR PATHSEPSTRING "icons" PATHSEPSTRING "default"));
    m_themesList = utils::instance().checkThemesList(set.readStringEntry("SETTINGS", "themesList", FXString(m_themePath+";").text()));
    m_colors.text = set.readColorEntry("SETTINGS", "textColor", FXRGB(255,255,255));
    m_colors.back = set.readColorEntry("SETTINGS", "textBackColor", FXRGB(0,0,0));
    m_colors.user = set.readColorEntry("SETTINGS", "userColor", FXRGB(191,191,191));
    m_colors.action = set.readColorEntry("SETTINGS", "actionsColor", FXRGB(255,165,0));
    m_colors.notice = set.readColorEntry("SETTINGS", "noticeColor", FXRGB(0,0,255));
    m_colors.error = set.readColorEntry("SETTINGS", "errorColor", FXRGB(255,0,0));
    m_colors.hilight = set.readColorEntry("SETTINGS", "hilightColor", FXRGB(0,255,0));
    m_colors.link = set.readColorEntry("SETTINGS", "linkColor", FXRGB(0,0,255));
    ircfontspec = set.readStringEntry("SETTINGS", "ircFont", "");
    m_sameCmd = set.readBoolEntry("SETTINGS", "sameCmd", FALSE);
    m_sameList = set.readBoolEntry("SETTINGS", "sameList", FALSE);
    m_coloredNick = set.readBoolEntry("SETTINGS", "coloredNick", FALSE);
    if(!ircfontspec.empty())
    {
        m_ircFont = new FXFont(getApp(), ircfontspec);
        m_ircFont->create();
    }
    else
    {
        getApp()->getNormalFont()->create();
        FXFontDesc fontdescription;
        getApp()->getNormalFont()->getFontDesc(fontdescription);
        m_ircFont = new FXFont(getApp(),fontdescription);
        m_ircFont->create();
    }
    m_maxAway = set.readIntEntry("SETTINGS", "maxAway", 200);
    m_logging = set.readBoolEntry("SETTINGS", "logging", FALSE);
    m_serverWindow = set.readBoolEntry("SETTINGS", "serverWindow", TRUE);
#ifdef HAVE_TRAY
    m_useTray = set.readBoolEntry("SETTINGS", "tray", FALSE);
#else
    m_useTray = FALSE;
#endif
    if(m_useTray)
        m_closeToTray = set.readBoolEntry("SETTINGS", "closeToTray", FALSE);
    else
        m_closeToTray = FALSE;
    m_reconnect = set.readBoolEntry("SETTINGS", "reconnect", FALSE);
    m_numberAttempt = set.readIntEntry("SETTINGS", "numberAttempt", 1);
    m_delayAttempt = set.readIntEntry("SETTINGS", "delayAttempt", 20);
    m_nickChar = FXString(set.readStringEntry("SETTINGS", "nickCompletionChar", ":")).left(1);
    m_logPath = set.readStringEntry("SETTINGS", "logPath");
    if(m_logging && !FXStat::exists(m_logPath)) m_logging = FALSE;
    m_dccPath = set.readStringEntry("SETTINGS", "dccPath");
    if(!FXStat::exists(m_dccPath)) m_dccPath = FXSystem::getHomeDirectory();
    m_autoDccChat = set.readBoolEntry("SETTINGS", "autoDccChat", FALSE);
    m_autoDccFile = set.readBoolEntry("SETTINGS", "autoDccFile", FALSE);
    FXint usersNum = set.readIntEntry("USERS", "number", 0);
    if(usersNum)
    {

        for(FXint i=0; i<usersNum; i++)
        {
            IgnoreUser user;
            user.nick = set.readStringEntry(FXStringFormat("USER%d", i).text(), "nick", FXStringFormat("xxx%d", i).text());
            user.channel = set.readStringEntry(FXStringFormat("USER%d", i).text(), "channel", "all");
            user.server = set.readStringEntry(FXStringFormat("USER%d", i).text(), "server", "all");
            m_usersList.append(user);
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
            m_friendsList.append(user);
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
            server.passwd = utils::instance().decrypt(set.readStringEntry(FXStringFormat("SERVER%d", i).text(), "hes", ""));
            server.channels = set.readStringEntry(FXStringFormat("SERVER%d", i).text(), "channels", "");
            server.commands = set.readStringEntry(FXStringFormat("SERVER%d", i).text(), "commands", "");
            server.autoConnect = set.readBoolEntry(FXStringFormat("SERVER%d", i).text(), "autoconnect", FALSE);
            server.useSsl = set.readBoolEntry(FXStringFormat("SERVER%d", i).text(), "ssl", FALSE);
            m_serverList.append(server);
        }
    }
#ifdef HAVE_LUA
    m_autoload = set.readBoolEntry("SETTINGS", "autoload", FALSE);
#else
    m_autoload = FALSE;
#endif
    m_autoloadPath = set.readStringEntry("SETTINGS", "autoloadPath");
    if(m_autoload && !FXStat::exists(utils::instance().isUtf8(m_autoloadPath.text(), m_autoloadPath.length()) ? m_autoloadPath : utils::instance().localeToUtf8(m_autoloadPath))) m_autoload = FALSE;
    FXString dccIP = set.readStringEntry("SETTINGS", "dccIP");
    FXRex rex("\\l");
    if(dccIP.empty() || dccIP.contains('.')!=3 || rex.match(dccIP))
    {
        m_dccIP1 = "";
        m_dccIP2 = "";
        m_dccIP3 = "";
        m_dccIP4 = "";
    }
    else
    {
        m_dccIP1 = dccIP.section('.',0);
        m_dccIP2 = dccIP.section('.',1);
        m_dccIP3 = dccIP.section('.',2);
        m_dccIP4 = dccIP.section('.',3);
    }
    m_dccPortD = set.readIntEntry("SETTINGS", "dccPortD");
    if(m_dccPortD<0 || m_dccPortD>65536) m_dccPortD = 0;
    m_dccPortH = set.readIntEntry("SETTINGS", "dccPortH");
    if(m_dccPortH<0 || m_dccPortH>65536) m_dccPortH = 0;
    if(m_dccPortH<m_dccPortD) m_dccPortH = m_dccPortD;
    m_dccTimeout = set.readIntEntry("SETTINGS", "dccTimeout", 66);
    m_sounds = set.readBoolEntry("SETTINGS", "sounds", FALSE);
    m_soundConnect = set.readBoolEntry("SETTINGS", "soundConnect", FALSE);
    m_soundDisconnect = set.readBoolEntry("SETTINGS", "soundDisconnect", FALSE);
    m_soundMessage = set.readBoolEntry("SETTINGS", "soundMessage", FALSE);
    m_pathConnect = set.readStringEntry("SETTINGS", "pathConnect", DXIRC_DATADIR PATHSEPSTRING "sounds" PATHSEPSTRING "connected.wav");
    m_pathDisconnect = set.readStringEntry("SETTINGS", "pathDisconnect", DXIRC_DATADIR PATHSEPSTRING "sounds" PATHSEPSTRING "disconnected.wav");
    m_pathMessage = set.readStringEntry("SETTINGS", "pathMessage", DXIRC_DATADIR PATHSEPSTRING "sounds" PATHSEPSTRING "message.wav");
    m_stripColors = set.readBoolEntry("SETTINGS", "stripColors", TRUE);
    m_useSmileys = set.readBoolEntry("SETTINGS", "useSmileys", FALSE);
    FXint smileysNum = set.readIntEntry("SMILEYS", "number", 0);
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
#endif
}

void ConfigDialog::saveConfig()
{
    getApp()->reg().setModified(FALSE);
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
    set.writeBoolEntry("SETTINGS", "serverWindow", m_serverWindow);
    set.writeStringEntry("SETTINGS", "logPath", m_logPath.text());
    set.writeStringEntry("SETTINGS", "dccPath", m_dccPath.text());
    set.writeStringEntry("SETTINGS", "nickCompletionChar", m_nickChar.text());
    set.writeIntEntry("USERS", "number", m_usersList.no());
    if(m_usersList.no())
    {

        for(FXint i=0; i<m_usersList.no(); i++)
        {
            set.writeStringEntry(FXStringFormat("USER%d", i).text(), "nick", m_usersList[i].nick.text());
            set.writeStringEntry(FXStringFormat("USER%d", i).text(), "channel", m_usersList[i].channel.text());
            set.writeStringEntry(FXStringFormat("USER%d", i).text(), "server", m_usersList[i].server.text());
        }
    }
    set.writeIntEntry("FRIENDS", "number", m_friendsList.no());
    if(m_friendsList.no())
    {

        for(FXint i=0; i<m_friendsList.no(); i++)
        {
            set.writeStringEntry(FXStringFormat("FRIEND%d", i).text(), "nick", m_friendsList[i].nick.text());
            set.writeStringEntry(FXStringFormat("FRIEND%d", i).text(), "channel", m_friendsList[i].channel.text());
            set.writeStringEntry(FXStringFormat("FRIEND%d", i).text(), "server", m_friendsList[i].server.text());
        }
    }
    set.writeIntEntry("SETTINGS","x", m_owner->getX());
    set.writeIntEntry("SETTINGS","y", m_owner->getY());
    set.writeIntEntry("SETTINGS","w", m_owner->getWidth());
    set.writeIntEntry("SETTINGS","h", m_owner->getHeight());
    set.writeIntEntry("SETTINGS", "tabPosition", m_tabPosition);
    set.writeColorEntry("SETTINGS", "basecolor", m_themeCurrent.base);
    set.writeColorEntry("SETTINGS", "bordercolor", m_themeCurrent.border);
    set.writeColorEntry("SETTINGS", "backcolor", m_themeCurrent.back);
    set.writeColorEntry("SETTINGS", "forecolor", m_themeCurrent.fore);
    set.writeColorEntry("SETTINGS", "hilitecolor", m_themeCurrent.hilite);
    set.writeColorEntry("SETTINGS", "shadowcolor", m_themeCurrent.shadow);
    set.writeColorEntry("SETTINGS", "selforecolor", m_themeCurrent.selfore);
    set.writeColorEntry("SETTINGS", "selbackcolor", m_themeCurrent.selback);
    set.writeColorEntry("SETTINGS", "tipforecolor", m_themeCurrent.tipfore);
    set.writeColorEntry("SETTINGS", "tipbackcolor", m_themeCurrent.tipback);
    set.writeColorEntry("SETTINGS", "selmenutextcolor", m_themeCurrent.menufore);
    set.writeColorEntry("SETTINGS", "selmenubackcolor", m_themeCurrent.menuback);
    set.writeColorEntry("SETTINGS", "traycolor", m_trayColor);
    set.writeStringEntry("SETTINGS", "normalfont", m_font->getFont().text());
    dxStringMap aliases = utils::instance().getAliases();
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
    set.writeBoolEntry("SETTINGS", "autoload", m_autoload);
    set.writeStringEntry("SETTINGS", "autoloadPath", m_autoloadPath.text());
    if(m_dccIP1.empty() || m_dccIP2.empty() || m_dccIP3.empty() || m_dccIP4.empty()) set.writeStringEntry("SETTINGS", "dccIP", "");
    else set.writeStringEntry("SETTINGS", "dccIP", FXString(m_dccIP1+"."+m_dccIP2+"."+m_dccIP3+"."+m_dccIP4).text());
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
    set.setModified();
    set.unparseFile(utils::instance().instance().getIniFile());
}
