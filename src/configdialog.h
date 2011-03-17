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
#include "fxext.h"
#include "dxtabbook.h"

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
  {"CDE"               ,FXRGB(156,153,156),FXRGB(  0,  0,  0),FXRGB(131,129,131),FXRGB(255,255,255),FXRGB( 49, 97,131),FXRGB(255,255,255),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB( 49, 97,131),FXRGB(255,255,255)},
  {"Digital CDE"       ,FXRGB( 74,121,131),FXRGB(  0,  0,  0),FXRGB( 55, 77, 78),FXRGB(255,255,255),FXRGB( 82,102,115),FXRGB(255,255,255),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB( 82,102,115),FXRGB(255,255,255)},
  {"Dark Blue"         ,FXRGB( 66,103,148),FXRGB(  0,  0,  0),FXRGB(  0, 42, 78),FXRGB(255,255,255),FXRGB( 92,179,255),FXRGB(  0,  0,  0),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB( 92,179,255),FXRGB(  0,  0,  0)},
  {"Desert FOX"        ,FXRGB(214,205,187),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(  0,  0,  0),FXRGB(128,  0,  0),FXRGB(255,255,255),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB(128,  0,  0),FXRGB(255,255,255)},
  {"EveX"              ,FXRGB(230,222,220),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(  0,  0,  0),FXRGB( 10, 95,137),FXRGB(255,255,255),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB( 10, 95,137),FXRGB(255,255,255)},
  {"Galaxy"            ,FXRGB(239,239,239),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(  0,  0,  0),FXRGB(103,141,178),FXRGB(255,255,255),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB(103,141,178),FXRGB(255,255,255)},
  {"iMac"              ,FXRGB(205,206,205),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(  0,  0,  0),FXRGB(  0,  0,128),FXRGB(255,255,255),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB(  0,  0,128),FXRGB(255,255,255)},
  {"KDE 1"             ,FXRGB(192,192,192),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(  0,  0,  0),FXRGB(  0,  0,128),FXRGB(255,255,255),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB(  0,  0,128),FXRGB(255,255,255)},
  {"KDE 2"             ,FXRGB(220,220,220),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(  0,  0,  0),FXRGB( 10, 95,137),FXRGB(255,255,255),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB( 10, 95,137),FXRGB(255,255,255)},
  {"KDE 3"             ,FXRGB(238,238,230),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(  0,  0,  0),FXRGB(255,221,118),FXRGB(255,255,255),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB(255,221,118),FXRGB(255,255,255)},
  {"Keramik"           ,FXRGB(234,233,232),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(  0,  0,  0),FXRGB(169,209,255),FXRGB(  0,  0,  0),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB(169,209,255),FXRGB(  0,  0,  0)},
  {"Keramik Emerald"   ,FXRGB(238,238,230),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(  0,  0,  0),FXRGB(134,204,134),FXRGB(  0,  0,  0),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB(134,204,134),FXRGB(  0,  0,  0)},
  {"Keramik White"     ,FXRGB(239,239,239),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(  0,  0,  0),FXRGB(103,141,178),FXRGB(255,255,255),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB(103,141,178),FXRGB(255,255,255)},
  {"Mandrake"          ,FXRGB(230,231,230),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(  0,  0,  0),FXRGB( 33, 68,156),FXRGB(255,255,255),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB( 33, 68,156),FXRGB(255,255,255)},
  {"Media Peach"       ,FXRGB(239,239,239),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(  0,  0,  0),FXRGB(103,141,178),FXRGB(255,255,255),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB(103,141,178),FXRGB(255,255,255)},
  {"Next"              ,FXRGB(168,168,168),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(  0,  0,  0),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB(  0,  0,  0),FXRGB(255,255,255)},
  {"Obsidian Coast"    ,FXRGB( 48, 47, 47),FXRGB(  0,  0,  0),FXRGB( 32, 31, 31),FXRGB(224,223,220),FXRGB( 24, 72,128),FXRGB(255,255,255),FXRGB( 16, 48, 80),FXRGB(196,209,224),FXRGB( 24, 72,128),FXRGB(255,255,255)},
  {"Pale Gray"         ,FXRGB(214,214,214),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(  0,  0,  0),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB(  0,  0,  0),FXRGB(255,255,255)},
  {"Plastik"           ,FXRGB(239,239,239),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(  0,  0,  0),FXRGB(103,141,178),FXRGB(255,255,255),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB(103,141,178),FXRGB(255,255,255)},
  {"Pumpkin"           ,FXRGB(238,216,174),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(  0,  0,  0),FXRGB(205,133, 63),FXRGB(255,255,255),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB(205,133, 63),FXRGB(255,255,255)},
  {"Redmond 95"        ,FXRGB(195,195,195),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(  0,  0,  0),FXRGB(  0,  0,128),FXRGB(255,255,255),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB(  0,  0,128),FXRGB(255,255,255)},
  {"Redmond 2000"      ,FXRGB(212,208,200),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(  0,  0,  0),FXRGB(  0, 36,104),FXRGB(255,255,255),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB(  0, 36,104),FXRGB(255,255,255)},
  {"Redmond XP"        ,FXRGB(238,238,230),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(  0,  0,  0),FXRGB( 74,121,205),FXRGB(255,255,255),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB( 74,121,205),FXRGB(255,255,255)},
  {"Solaris"           ,FXRGB(174,178,195),FXRGB(  0,  0,  0),FXRGB(147,151,165),FXRGB(  0,  0,  0),FXRGB(113,139,165),FXRGB(255,255,255),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB(113,139,165),FXRGB(255,255,255)},
  {"Storm"             ,FXRGB(192,192,192),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(  0,  0,  0),FXRGB(139,  0,139),FXRGB(255,255,255),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB(139,  0,139),FXRGB(255,255,255)},
  {"Sea Sky"           ,FXRGB(165,178,198),FXRGB(  0,  0,  0),FXRGB(255,255,255),FXRGB(  0,  0,  0),FXRGB( 49,101,156),FXRGB(255,255,255),FXRGB(255,255,225),FXRGB(  0,  0,  0),FXRGB( 49,101,156),FXRGB(255,255,255)},
  {"Wonton Soup"       ,FXRGB( 71, 76, 86),FXRGB(  0,  0,  0),FXRGB( 58, 62, 70),FXRGB(182,193,208),FXRGB(117,133,153),FXRGB(209,225,244),FXRGB(182,193,208),FXRGB( 42, 44, 48),FXRGB(117,133,153),FXRGB(209,225,244)},
};

const FXint numThemes=ARRAYNUMBER(ColorThemes);

class ConfigDialog: public FXDialogBox
{
    FXDECLARE(ConfigDialog)
    public:
        ConfigDialog(FXMainWindow *owner);
        virtual ~ConfigDialog();

        long onCommandsSelected(FXObject*,FXSelector,void*);
        long onCommandsDeselected(FXObject*,FXSelector,void*);
        long onUsersSelected(FXObject*,FXSelector,void*);
        long onUsersDeselected(FXObject*,FXSelector,void*);
        long onUsersChanged(FXObject*,FXSelector,void*);
        long onFriendsSelected(FXObject*,FXSelector,void*);
        long onFriendsDeselected(FXObject*,FXSelector,void*);
        long onFriendsChanged(FXObject*,FXSelector,void*);
        long onAddCommand(FXObject*,FXSelector,void*);
        long onDeleteCommand(FXObject*,FXSelector,void*);
        long onAddUser(FXObject*,FXSelector,void*);
        long onModifyUser(FXObject*,FXSelector,void*);
        long onDeleteUser(FXObject*,FXSelector,void*);
        long onAddFriend(FXObject*,FXSelector,void*);
        long onModifyFriend(FXObject*,FXSelector,void*);
        long onDeleteFriend(FXObject*,FXSelector,void*);
        long onColor(FXObject*,FXSelector,void*);
        long onAccept(FXObject*,FXSelector,void*);
        long onCancel(FXObject*,FXSelector,void*);
        long onKeyPress(FXObject*,FXSelector,void*);
        long onIconsChanged(FXObject*,FXSelector,void*);
        long onAddIcons(FXObject*,FXSelector,void*);
        long onDeleteIcons(FXObject*,FXSelector,void*);
        long onLogChanged(FXObject*,FXSelector,void*);
        long onServerWindow(FXObject*,FXSelector,void*);
        long onNickCharChanged(FXObject*,FXSelector,void*);
        long onPathSelect(FXObject*,FXSelector,void*);
        long onAutoloadChanged(FXObject*,FXSelector,void*);
        long onAutoloadPathSelect(FXObject*,FXSelector,void*);
        long onTheme(FXObject*,FXSelector,void*);
        long onThemeColorChanged(FXObject*,FXSelector,void*);
        long onTabColorChanged(FXObject*,FXSelector,void*);
        long onFont(FXObject*,FXSelector,void*);
        long onIrcFont(FXObject*,FXSelector,void*);
        long onTray(FXObject*,FXSelector,void*);
        long onReconnect(FXObject*,FXSelector,void*);
        long onTabPosition(FXObject*,FXSelector,void*);
        long onDccPathSelect(FXObject*,FXSelector,void*);
        long onDccPortD(FXObject*,FXSelector,void*);
        long onDccPortH(FXObject*,FXSelector,void*);
        long onSounds(FXObject*,FXSelector,void*);
        long onSoundConnect(FXObject*,FXSelector,void*);
        long onSoundDisconnect(FXObject*,FXSelector,void*);
        long onSoundMessage(FXObject*,FXSelector,void*);
        long onPlay(FXObject*,FXSelector,void*);
        long onSelectPath(FXObject*,FXSelector,void*);
        long onNotify(FXObject*,FXSelector,void*);
        long onShowNotify(FXObject*,FXSelector,void*);
        long onUseSmileys(FXObject*,FXSelector,void*);
        long onAddSmiley(FXObject*,FXSelector,void*);
        long onModifySmiley(FXObject*,FXSelector,void*);
        long onDeleteSmiley(FXObject*,FXSelector,void*);
        long onImportSmiley(FXObject*,FXSelector,void*);
        long onExportSmiley(FXObject*,FXSelector,void*);

    private:
        ConfigDialog() {}
        ConfigDialog(const ConfigDialog&);

        FXList *m_commands, *m_icons, *m_smileys;
        FXIconList *m_users, *m_friends;
        dxEXButton *m_addCommand, *m_deleteCommand, *m_addUser, *m_modifyUser, *m_deleteUser;
        dxEXButton *m_addTheme, *m_deleteTheme, *m_selectPath, *m_selectAutoloadPath;
        dxEXButton *m_addSmiley, *m_modifySmiley, *m_deleteSmiley, *m_importSmiley, *m_exportSmiley;
        dxEXButton *m_icon1, *m_icon2, *m_icon3, *m_icon4, *m_icon5, *m_icon6, *m_icon7;
        dxEXButton *m_fontButton, *m_ircfontButton, *m_addFriend, *m_modifyFriend, *m_deleteFriend;
        FXCheckButton *m_closeToTrayButton, *m_checkConnect, *m_checkDisconnect, *m_checkMessage;
        FXCheckButton *m_checkNotifyConnect, *m_checkNotifyDisconnect, *m_checkNotifyMessage;
        FXTextField *m_textTest, *m_nickCharField;
        FXString m_commandsList, m_themePath, m_themesList, m_logPath, m_autoloadPath, m_dccPath;
        FXString m_dccIP1, m_dccIP2, m_dccIP3, m_dccIP4;
        FXbool m_logging, m_serverWindow, m_sameCmd, m_sameList, m_useTray, m_coloredNick, m_closeToTray, m_reconnect;
        FXbool m_statusShown, m_autoload, m_sounds, m_soundConnect, m_soundDisconnect, m_soundMessage;
        FXbool m_stripColors, m_useSmileys, m_showImportwarning, m_showWarning;
        FXbool m_autoDccChat, m_autoDccFile;
        FXbool m_notify, m_notifyConnect, m_notifyDisconnect, m_notifyMessage;
        dxServerInfoArray m_serverList;
        dxIgnoreUserArray m_usersList, m_friendsList;
        FXToolBar *m_iconsBar;
        IrcColor m_colors;
        FXint m_maxAway, m_numberAttempt, m_delayAttempt, m_tabPosition, m_dccPortD, m_dccPortH, m_dccTimeout;
        FXSpinner *m_numberAttemptSpinner, *m_delayAttemptSpinner;
        FXLabel *m_numberAttemptLabel, *m_delayAttemptLabel;
        FXString m_nickChar;
        FXDataTarget m_textTarget, m_backTarget, m_userTarget, m_actionTarget, m_noticeTarget, m_errorTarget, m_hilightTarget, m_linkTarget;
        FXDataTarget m_trayTarget, m_logTarget, m_serverTarget, m_autoloadTarget;
        FXHiliteStyle m_textStyle[6];
        FXText *m_text;
        ColorTheme m_themeCurrent;
        FXListBox *m_themes;
        FXDataTarget m_targetBack, m_targetBase, m_targetBorder, m_targetFore, m_targetMenuback, m_targetMenufore, m_targetSelback, m_targetSelfore, m_targetTipback, m_targetTipfore;
        FXDataTarget m_targetSameCmd, m_targetSameList, m_targetColoredNick, m_targetCloseToTray;
        FXDataTarget m_targetMaxAway, m_targetReconnect, m_targetNumberAttempt, m_targetDelayAttempt;
        FXDataTarget m_targetDccPath, m_targetLogPath, m_targetAutoloadPath;
        FXDataTarget m_targetDccIP1, m_targetDccIP2, m_targetDccIP3, m_targetDccIP4, m_targetDccPortD, m_targetDccPortH, m_targetDccTimeout;
        FXDataTarget m_targetSound, m_targetSoundConnect, m_targetSoundDisconnect, m_targetSoundMessage;
        FXDataTarget m_targetNotify, m_targetNotifyConnect, m_targetNotifyDisconnect, m_targetNotifyMessage;
        FXDataTarget m_targetPathConnect, m_targetPathDisconnect, m_targetPathMessage, m_targetStripColors;
        FXDataTarget m_targetUseSmileys, m_targetAutoDccChat, m_targetAutoDccFile, m_targetTrayColor;
        FXDataTarget m_targetUnreadColor, m_targetHighlightColor;
        dxEXButton *m_selectConnect, *m_selectDisconnect, *m_selectMessage, *m_playConnect, *m_playDisconnect, *m_playMessage;
        FXString m_pathConnect, m_pathDisconnect, m_pathMessage;
        FXLabel *m_labelSelected, *m_labelNocurrent, *m_labelTip, *m_label;
        FXVerticalFrame *m_vframe2, *m_menuFrame;
        FXLabel *m_menuLabels[3];
        FXHorizontalFrame *m_textFrame1, *m_textFrame2, *m_textFrame3, *m_tabFrame;
        FXGroupBox *m_menuGroup;
        FXSeparator *m_sep1, *m_sep2;
        FXFont *m_font, *m_ircFont;
        FXListBox *m_listTabs;
        dxTabBook *m_tabs;
        dxEXTabItem *m_tab, *m_unread, *m_highlight;
        FXVerticalFrame *m_tabframe1, *m_tabframe2, *m_tabframe3;
        FXMainWindow *m_owner;
        dxStringMap m_smileysMap;
        FXColor m_trayColor, m_unreadColor, m_highlightColor;
        FXbool m_useSpell;
        FXDataTarget m_targetUseSpell;

        void fillCommnads();
        FXString fillCommandsCombo();
        void fillIcons();
        void fillUsers();
        void fillFriends();
        void fillThemes();
        void fillSmileys();
        void updateCommands();
        void updateIcons();
        void updateColors();
        void updateFont();
        void updateIrcFont();
        void showMessage();
        FXbool themeExist(const FXString &ckdTheme);
        FXbool nickExist(const FXString &ckdNick, FXbool user=TRUE);
        FXbool smileyExist(const FXString &ckdSmiley);
        void readConfig();
        void saveConfig();
        FXchar* enquote(FXchar* result, const FXchar* text);
        FXchar* dequote(FXchar* text) const;
};

class SmileyDialog: public FXDialogBox
{
    FXDECLARE(SmileyDialog)
public:
    SmileyDialog(FXWindow *owner, FXString title, FXString smiley, FXString path);
    virtual ~SmileyDialog();

    long onAccept(FXObject*,FXSelector,void*);
    long onCancel(FXObject*,FXSelector,void*);
    long onPath(FXObject*,FXSelector,void*);

    FXString getSmiley() { return m_smileyText->getText(); }
    FXString getPath() { return m_pathText->getText(); }
    FXbool iconExist();
    FXIcon* getIcon() const { return m_previewLabel->getIcon(); }
private:
    SmileyDialog() {}
    SmileyDialog(const SmileyDialog&);

    FXTextField *m_smileyText, *m_pathText;
    dxEXButton *m_pathButton;
    FXLabel *m_previewLabel;
};

#endif /* CONFIGDIALOG_H */
