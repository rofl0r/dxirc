/*
 *      configdialog.h
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


#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include "defs.h"

const ColorTheme ColorThemes[]={
//|--------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------|
//|        Name        |        Base      |       Border     |       Back       |      Fore        |      Selback     |      Selfore     |      Tipback     |     Tipfore      |      Menuback    |      Menufore    |
//|--------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------|
  {"FOX"               ,FXRGB(212,208,200),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(  0,  0,  0),FXRGB( 10, 36,106),FXRGB(255,255,255),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB( 10, 36,106),FXRGB(255,255,255)},
  {"Gnome"             ,FXRGB(214,215,214),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(  0,  0,  0),FXRGB(  0,  0,128),FXRGB(255,255,255),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB(  0,  0,128),FXRGB(255,255,255)},
  {"Atlas Green"       ,FXRGB(175,180,159),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(  0,  0,  0),FXRGB(111,122, 99),FXRGB(255,255,255),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB(111,122, 99),FXRGB(255,255,255)},
  {"BeOS"              ,FXRGB(217,217,217),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(  0,  0,  0),FXRGB(168,168,168),FXRGB(  0,  0,  0),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB(168,168,168),FXRGB(  0,  0,  0)},
  {"Blue Slate"        ,FXRGB(239,239,239),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(  0,  0,  0),FXRGB(103,141,178),FXRGB(255,255,255),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB(103,141,178),FXRGB(255,255,255)},
  {"Black and White"   ,FXRGB(  8,  8,  8),FXRGB( 77, 77, 77),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(  0,  0,255),FXRGB(255,255,255),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB(  0,  0,255),FXRGB(255,255,255)},
//|--------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------|
//|        Name        |        Base      |       Border     |       Back       |      Fore        |      Selback     |      Selfore     |      Tipback     |     Tipfore      |      Menuback    |      Menufore    |
//|--------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------|
  {"CDE"               ,FXRGB(156,153,156),FXRGB(  0,  0,  0),FXRGB(131,129,131),FXRGB(255,255,255),FXRGB( 49, 97,131),FXRGB(255,255,255),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB( 49, 97,131),FXRGB(255,255,255)},
  {"Digital CDE"       ,FXRGB( 74,121,131),FXRGB(  0,  0,  0),FXRGB( 55, 77, 78),FXRGB(255,255,255),FXRGB( 82,102,115),FXRGB(255,255,255),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB( 82,102,115),FXRGB(255,255,255)},
  {"Dark Blue"         ,FXRGB( 66,103,148),FXRGB(  0,  0,  0),FXRGB(  0, 42, 78),FXRGB(255,255,255),FXRGB( 92,179,255),FXRGB(  0,  0,  0),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB( 92,179,255),FXRGB(  0,  0,  0)},
  {"Desert FOX"        ,FXRGB(214,205,187),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(  0,  0,  0),FXRGB(128,  0,  0),FXRGB(255,255,255),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB(128,  0,  0),FXRGB(255,255,255)},
  {"EveX"              ,FXRGB(230,222,220),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(  0,  0,  0),FXRGB( 10, 95,137),FXRGB(255,255,255),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB( 10, 95,137),FXRGB(255,255,255)},
//|--------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------|
//|        Name        |        Base      |       Border     |       Back       |      Fore        |      Selback     |      Selfore     |      Tipback     |     Tipfore      |      Menuback    |      Menufore    |
//|--------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------|
  {"Galaxy"            ,FXRGB(239,239,239),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(  0,  0,  0),FXRGB(103,141,178),FXRGB(255,255,255),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB(103,141,178),FXRGB(255,255,255)},
  {"iMac"              ,FXRGB(205,206,205),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(  0,  0,  0),FXRGB(  0,  0,128),FXRGB(255,255,255),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB(  0,  0,128),FXRGB(255,255,255)},
  {"KDE 1"             ,FXRGB(192,192,192),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(  0,  0,  0),FXRGB(  0,  0,128),FXRGB(255,255,255),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB(  0,  0,128),FXRGB(255,255,255)},
  {"KDE 2"             ,FXRGB(220,220,220),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(  0,  0,  0),FXRGB( 10, 95,137),FXRGB(255,255,255),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB( 10, 95,137),FXRGB(255,255,255)},
  {"KDE 3"             ,FXRGB(238,238,230),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(  0,  0,  0),FXRGB(255,221,118),FXRGB(255,255,255),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB(255,221,118),FXRGB(255,255,255)},
//|--------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------|
//|        Name        |        Base      |       Border     |       Back       |      Fore        |      Selback     |      Selfore     |      Tipback     |     Tipfore      |      Menuback    |      Menufore    |
//|--------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------|
  {"Keramik"           ,FXRGB(234,233,232),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(  0,  0,  0),FXRGB(169,209,255),FXRGB(  0,  0,  0),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB(169,209,255),FXRGB(  0,  0,  0)},
  {"Keramik Emerald"   ,FXRGB(238,238,230),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(  0,  0,  0),FXRGB(134,204,134),FXRGB(  0,  0,  0),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB(134,204,134),FXRGB(  0,  0,  0)},
  {"Keramik White"     ,FXRGB(239,239,239),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(  0,  0,  0),FXRGB(103,141,178),FXRGB(255,255,255),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB(103,141,178),FXRGB(255,255,255)},
  {"Mandrake"          ,FXRGB(230,231,230),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(  0,  0,  0),FXRGB( 33, 68,156),FXRGB(255,255,255),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB( 33, 68,156),FXRGB(255,255,255)},
  {"Media Peach"       ,FXRGB(239,239,239),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(  0,  0,  0),FXRGB(103,141,178),FXRGB(255,255,255),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB(103,141,178),FXRGB(255,255,255)},
//|--------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------|
//|        Name        |        Base      |       Border     |       Back       |      Fore        |      Selback     |      Selfore     |      Tipback     |     Tipfore      |      Menuback    |      Menufore    |
//|--------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------|
  {"Next"              ,FXRGB(168,168,168),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(  0,  0,  0),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB(  0,  0,  0),FXRGB(255,255,255)},
  {"Pale Gray"         ,FXRGB(214,214,214),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(  0,  0,  0),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB(  0,  0,  0),FXRGB(255,255,255)},
  {"Plastik"           ,FXRGB(239,239,239),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(  0,  0,  0),FXRGB(103,141,178),FXRGB(255,255,255),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB(103,141,178),FXRGB(255,255,255)},
  {"Pumpkin"           ,FXRGB(238,216,174),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(  0,  0,  0),FXRGB(205,133, 63),FXRGB(255,255,255),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB(205,133, 63),FXRGB(255,255,255)},
  {"Redmond 95"        ,FXRGB(195,195,195),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(  0,  0,  0),FXRGB(  0,  0,128),FXRGB(255,255,255),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB(  0,  0,128),FXRGB(255,255,255)},
//|--------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------|
//|        Name        |        Base      |       Border     |       Back       |      Fore        |      Selback     |      Selfore     |      Tipback     |     Tipfore      |      Menuback    |      Menufore    |
//|--------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------|
  {"Redmond 2000"      ,FXRGB(212,208,200),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(  0,  0,  0),FXRGB(  0, 36,104),FXRGB(255,255,255),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB(  0, 36,104),FXRGB(255,255,255)},
  {"Redmond XP"        ,FXRGB(238,238,230),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(  0,  0,  0),FXRGB( 74,121,205),FXRGB(255,255,255),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB( 74,121,205),FXRGB(255,255,255)},
  {"Solaris"           ,FXRGB(174,178,195),FXRGB(  0,  0,  0),FXRGB(147,151,165),FXRGB(  0,  0,  0),FXRGB(113,139,165),FXRGB(255,255,255),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB(113,139,165),FXRGB(255,255,255)},
  {"Storm"             ,FXRGB(192,192,192),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(  0,  0,  0),FXRGB(139,  0,139),FXRGB(255,255,255),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB(139,  0,139),FXRGB(255,255,255)},
  {"Sea Sky"           ,FXRGB(165,178,198),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(  0,  0,  0),FXRGB( 49,101,156),FXRGB(255,255,255),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB( 49,101,156),FXRGB(255,255,255)},
};

const FXint numThemes=ARRAYNUMBER(ColorThemes);

class ConfigDialog: public FXDialogBox
{
    FXDECLARE(ConfigDialog)
    public:
        ConfigDialog(FXMainWindow *owner);
        virtual ~ConfigDialog();
        enum {
            ID_ADDCOMMAND = FXTopWindow::ID_LAST,
            ID_DELETECOMMAND,
            ID_ADDUSER,
            ID_MODIFYUSER,
            ID_DELETEUSER,
            ID_ADDFRIEND,
            ID_MODIFYFRIEND,
            ID_DELETEFRIEND,
            ID_ADDICONS,
            ID_DELETEICONS,
            ID_COMMAND,
            ID_USER,
            ID_FRIEND,
            ID_IRCCOLORS,
            ID_IRCFONT,
            ID_COLORS,
            ID_THEME,
            ID_FONT,
            ID_ACCEPT,
            ID_CANCEL,
            ID_ICONS,
            ID_LOG,
            ID_LOGPATH,
            ID_SERVERWINDOW,
            ID_TRAY,
            ID_NICK,
            ID_RECONNECT,
            ID_TABPOS,
            ID_AUTOLOAD,
            ID_AUTOLOADPATH,
            ID_DCCPATH,
            ID_DCCPORTD,
            ID_DCCPORTH,
            ID_DCCTIMEOUT,
            ID_SOUNDS,
            ID_SOUNDCONNECT,
            ID_SOUNDDISCONNECT,
            ID_SOUNDMESSAGE,
            ID_PLAYCONNECT,
            ID_PLAYDISCONNECT,
            ID_PLAYMESSAGE,
            ID_SELECTCONNECT,
            ID_SELECTDISCONNECT,
            ID_SELECTMESSAGE,
            ID_USESMILEYS,
            ID_ADDSMILEY,
            ID_MODIFYSMILEY,
            ID_DELETESMILEY,
            ID_SMILEY,
            ID_IMPORTSMILEY,
            ID_EXPORTSMILEY,
            ID_LAST
        };

        long OnCommandsSelected(FXObject*,FXSelector,void*);
        long OnCommandsDeselected(FXObject*,FXSelector,void*);
        long OnUsersSelected(FXObject*,FXSelector,void*);
        long OnUsersDeselected(FXObject*,FXSelector,void*);
        long OnUsersChanged(FXObject*,FXSelector,void*);
        long OnFriendsSelected(FXObject*,FXSelector,void*);
        long OnFriendsDeselected(FXObject*,FXSelector,void*);
        long OnFriendsChanged(FXObject*,FXSelector,void*);
        long OnAddCommand(FXObject*,FXSelector,void*);
        long OnDeleteCommand(FXObject*,FXSelector,void*);
        long OnAddUser(FXObject*,FXSelector,void*);
        long OnModifyUser(FXObject*,FXSelector,void*);
        long OnDeleteUser(FXObject*,FXSelector,void*);
        long OnAddFriend(FXObject*,FXSelector,void*);
        long OnModifyFriend(FXObject*,FXSelector,void*);
        long OnDeleteFriend(FXObject*,FXSelector,void*);
        long OnColor(FXObject*,FXSelector,void*);
        long OnAccept(FXObject*,FXSelector,void*);
        long OnCancel(FXObject*,FXSelector,void*);
        long OnKeyPress(FXObject*,FXSelector,void*);
        long OnIconsChanged(FXObject*,FXSelector,void*);
        long OnAddIcons(FXObject*,FXSelector,void*);
        long OnDeleteIcons(FXObject*,FXSelector,void*);
        long OnLogChanged(FXObject*,FXSelector,void*);
        long OnServerWindow(FXObject*,FXSelector,void*);
        long OnNickCharChanged(FXObject*,FXSelector,void*);
        long OnPathSelect(FXObject*,FXSelector,void*);
        long OnAutoloadChanged(FXObject*,FXSelector,void*);
        long OnAutoloadPathSelect(FXObject*,FXSelector,void*);
        long OnTheme(FXObject*,FXSelector,void*);
        long OnThemeColorChanged(FXObject*,FXSelector,void*);
        long OnFont(FXObject*,FXSelector,void*);
        long OnIrcFont(FXObject*,FXSelector,void*);
        long OnTray(FXObject*,FXSelector,void*);
        long OnReconnect(FXObject*,FXSelector,void*);
        long OnTabPosition(FXObject*,FXSelector,void*);
        long OnDccPathSelect(FXObject*,FXSelector,void*);
        long OnDccPortD(FXObject*,FXSelector,void*);
        long OnDccPortH(FXObject*,FXSelector,void*);
        long OnSounds(FXObject*,FXSelector,void*);
        long OnSoundConnect(FXObject*,FXSelector,void*);
        long OnSoundDisconnect(FXObject*,FXSelector,void*);
        long OnSoundMessage(FXObject*,FXSelector,void*);
        long OnPlay(FXObject*,FXSelector,void*);
        long OnSelectPath(FXObject*,FXSelector,void*);
        long OnUseSmileys(FXObject*,FXSelector,void*);
        long OnAddSmiley(FXObject*,FXSelector,void*);
        long OnModifySmiley(FXObject*,FXSelector,void*);
        long OnDeleteSmiley(FXObject*,FXSelector,void*);
        long OnImportSmiley(FXObject*,FXSelector,void*);
        long OnExportSmiley(FXObject*,FXSelector,void*);

    private:
        ConfigDialog() {}
        ConfigDialog(const ConfigDialog&);

        FXList *commands, *icons, *smileys;
        FXIconList *users, *friends;
        FXButton *addCommand, *deleteCommand, *addUser, *modifyUser, *deleteUser;
        FXButton *addTheme, *deleteTheme, *selectPath, *selectAutoloadPath;
        FXButton *addSmiley, *modifySmiley, *deleteSmiley, *importSmiley, *exportSmiley;
        FXButton *icon1, *icon2, *icon3, *icon4, *icon5, *icon6, *icon7;
        FXButton *fontButton, *ircfontButton, *addFriend, *modifyFriend, *deleteFriend;
        FXCheckButton *closeToTrayButton, *checkConnect, *checkDisconnect, *checkMessage;
        FXTextField *textTest, *nickCharField;
        FXString commandsList, themePath, themesList, logPath, autoloadPath, dccPath;
        FXString dccIP1, dccIP2, dccIP3, dccIP4;
        FXbool logging, serverWindow, sameCmd, sameList, useTray, coloredNick, closeToTray, reconnect;
        FXbool usersShown, statusShown, autoload, sounds, soundConnect, soundDisconnect, soundMessage;
        FXbool stripColors, useSmileys, showImportwarning, showWarning;
        FXbool autoDccChat, autoDccFile;
        dxServerInfoArray serverList;
        dxIgnoreUserArray usersList, friendsList;
        FXToolBar *iconsBar;
        IrcColor colors;
        FXint maxAway, numberAttempt, delayAttempt, tabPosition, dccPortD, dccPortH, dccTimeout;
        FXSpinner *numberAttemptSpinner, *delayAttemptSpinner;
        FXLabel *numberAttemptLabel, *delayAttemptLabel;
        FXString nickChar;
        FXDataTarget textTarget, backTarget, userTarget, actionTarget, noticeTarget, errorTarget, hilightTarget, linkTarget;
        FXDataTarget trayTarget, logTarget, serverTarget, autoloadTarget;
        FXHiliteStyle textStyle[6];
        FXText *text;
        ColorTheme themeCurrent, themeUser;
        FXListBox *themes;
        FXDataTarget targetBack, targetBase, targetBorder, targetFore, targetMenuback, targetMenufore, targetSelback, targetSelfore, targetTipback, targetTipfore;
        FXDataTarget targetSameCmd, targetSameList, targetColoredNick, targetCloseToTray;
        FXDataTarget targetMaxAway, targetReconnect, targetNumberAttempt, targetDelayAttempt;
        FXDataTarget targetDccPath, targetLogPath, targetAutoloadPath;
        FXDataTarget targetDccIP1, targetDccIP2, targetDccIP3, targetDccIP4, targetDccPortD, targetDccPortH, targetDccTimeout;
        FXDataTarget targetSound, targetSoundConnect, targetSoundDisconnect, targetSoundMessage;
        FXDataTarget targetPathConnect, targetPathDisconnect, targetPathMessage, targetStripColors;
        FXDataTarget targetUseSmileys, targetAutoDccChat, targetAutoDccFile, targetTrayColor;
        FXButton *selectConnect, *selectDisconnect, *selectMessage, *playConnect, *playDisconnect, *playMessage;
        FXString pathConnect, pathDisconnect, pathMessage;
        FXLabel *labelSelected, *labelNocurrent, *labelTip, *label;
        FXVerticalFrame *vframe2, *menuFrame;
        FXLabel *menuLabels[3];
        FXHorizontalFrame *textFrame1, *textFrame2, *textFrame3;
        FXGroupBox *menuGroup;
        FXSeparator *sep1, *sep2;
        FXFont *font, *ircFont;
        FXListBox *listTabs;
        FXMainWindow *owner;
        dxStringMap smileysMap;
        FXColor trayColor;

        void FillCommnads();
        FXString FillCommandsCombo();
        void FillIcons();
        void FillUsers();
        void FillFriends();
        void FillThemes();
        void FillSmileys();
        void UpdateCommands();
        void UpdateIcons();
        void UpdateColors();
        void UpdateFont();
        void UpdateIrcFont();
        void ShowMessage();
        FXbool ThemeExist(const FXString &ckdTheme);
        FXbool NickExist(const FXString &ckdNick, FXbool user=TRUE);
        FXbool SmileyExist(const FXString &ckdSmiley);
        void ReadConfig();
        void SaveConfig();
        FXchar* Enquote(FXchar* result, const FXchar* text);
        FXchar* Dequote(FXchar* text) const;
};

class SmileyDialog: public FXDialogBox
{
    FXDECLARE(SmileyDialog)
public:
    SmileyDialog(FXWindow *owner, FXString title, FXString smiley, FXString path);
    virtual ~SmileyDialog();
    enum {
        ID_ACCEPT = ConfigDialog::ID_LAST,
        ID_CANCEL,
        ID_PATH
    };

    long OnAccept(FXObject*,FXSelector,void*);
    long OnCancel(FXObject*,FXSelector,void*);
    long OnPath(FXObject*,FXSelector,void*);

    FXString GetSmiley() { return smileyText->getText(); }
    FXString GetPath() { return pathText->getText(); }
    FXbool IconExist();
    FXIcon* GetIcon() const { return previewLabel->getIcon(); }
private:
    SmileyDialog() {}
    SmileyDialog(const SmileyDialog&);

    FXTextField *smileyText, *pathText;
    FXButton *pathButton;
    FXLabel *previewLabel;
};

#endif /* CONFIGDIALOG_H */
