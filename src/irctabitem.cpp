/*
 *      irctabitem.cpp
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
#include <shellapi.h>
#endif
#include "irctabitem.h"
#include "tetristabitem.h"
#include "icons.h"
#include "config.h"
#include "i18n.h"

#define LUA_HELP_PATH "http://www.dxirc.org/dxirc-lua.html"

FXDEFMAP(DccSendDialog) DccSendDialogMap[] = {
    FXMAPFUNC(SEL_COMMAND,  DccSendDialog::ID_FILE,     DccSendDialog::onFile),
    FXMAPFUNC(SEL_COMMAND,  DccSendDialog::ID_SEND,     DccSendDialog::onSend),
    FXMAPFUNC(SEL_COMMAND,  DccSendDialog::ID_CANCEL,   DccSendDialog::onCancel),
    FXMAPFUNC(SEL_CLOSE,    0,                          DccSendDialog::onCancel),
    FXMAPFUNC(SEL_KEYPRESS, 0,                          DccSendDialog::onKeyPress)
};

FXIMPLEMENT(DccSendDialog, FXDialogBox, DccSendDialogMap, ARRAYNUMBER(DccSendDialogMap))

DccSendDialog::DccSendDialog(FXMainWindow* owner, FXString nick)
        : FXDialogBox(owner, FXStringFormat(_("Send file to %s"), nick.text()), DECOR_RESIZE|DECOR_TITLE|DECOR_BORDER, 0,0,0,0, 0,0,0,0, 0,0)
{
    m_mainFrame = new FXVerticalFrame(this, LAYOUT_FILL_X|LAYOUT_FILL_Y);

    m_fileFrame = new FXHorizontalFrame(m_mainFrame, LAYOUT_FILL_X);
    new FXLabel(m_fileFrame, _("File:"));
    m_fileText = new FXTextField(m_fileFrame, 25, NULL, 0, TEXTFIELD_READONLY|FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_X);
    m_buttonFile = new FXButton(m_fileFrame, "...", NULL, this, ID_FILE, FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 10,10,2,5);

    m_passiveFrame = new FXHorizontalFrame(m_mainFrame, LAYOUT_FILL_X);
    m_checkPassive = new FXCheckButton(m_passiveFrame, _("Send passive"), NULL, 0);

    m_buttonFrame = new FXHorizontalFrame(m_mainFrame, LAYOUT_FILL_X|PACK_UNIFORM_WIDTH);
    m_buttonCancel = new FXButton(m_buttonFrame, _("&Cancel"), NULL, this, ID_CANCEL, FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,5);
    m_buttonSend = new FXButton(m_buttonFrame, _("&Send file"), NULL, this, ID_SEND, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,5);
}

DccSendDialog::~DccSendDialog()
{
}

FXuint DccSendDialog::execute(FXuint placement)
{
    create();
    show(placement);
    getApp()->refresh();
    FXFileDialog dialog(this, _("Select file"));
    if(dialog.execute())
    {
        m_fileText->setText(dialog.getFilename());
    }
    return getApp()->runModalFor(this);
}

long DccSendDialog::onFile(FXObject*, FXSelector, void*)
{
    FXFileDialog dialog(this, _("Select file"));
    if(dialog.execute())
    {
        m_fileText->setText(dialog.getFilename());
    }
    return 1;
}

long DccSendDialog::onSend(FXObject*, FXSelector, void*)
{
    getApp()->stopModal(this,TRUE);
    hide();
    return 1;
}

long DccSendDialog::onCancel(FXObject*, FXSelector, void*)
{
    getApp()->stopModal(this,FALSE);
    hide();
    return 1;
}

long DccSendDialog::onKeyPress(FXObject *sender, FXSelector sel, void *ptr)
{
    if(FXTopWindow::onKeyPress(sender,sel,ptr)) return 1;
    if(((FXEvent*)ptr)->code == KEY_Escape)
    {
        handle(this,FXSEL(SEL_COMMAND,ID_CANCEL),NULL);
        return 1;
    }
    return 0;
}

FXDEFMAP(IrcTabItem) IrcTabItemMap[] = {
    FXMAPFUNC(SEL_COMMAND,              IrcTabItem::ID_COMMANDLINE,     IrcTabItem::onCommandline),
    FXMAPFUNC(SEL_KEYPRESS,             IrcTabItem::ID_COMMANDLINE,     IrcTabItem::onKeyPress),
    FXMAPFUNC(SEL_COMMAND,              IrcSocket::ID_SERVER,           IrcTabItem::onIrcEvent),
    FXMAPFUNC(SEL_TIMEOUT,              IrcTabItem::ID_TIME,            IrcTabItem::onTimeout),
    FXMAPFUNC(SEL_TIMEOUT,              IrcTabItem::ID_PTIME,           IrcTabItem::onPipeTimeout),
    FXMAPFUNC(SEL_TIMEOUT,              IrcTabItem::ID_ETIME,           IrcTabItem::onEggTimeout),
    FXMAPFUNC(SEL_TEXTLINK,             IrcTabItem::ID_TEXT,            IrcTabItem::onTextLink),
    FXMAPFUNC(SEL_RIGHTBUTTONRELEASE,   IrcTabItem::ID_USERS,           IrcTabItem::onRightMouse),
    FXMAPFUNC(SEL_DOUBLECLICKED,        IrcTabItem::ID_USERS,           IrcTabItem::onDoubleclick),
    FXMAPFUNC(SEL_COMMAND,              IrcTabItem::ID_NEWQUERY,        IrcTabItem::onNewQuery),
    FXMAPFUNC(SEL_COMMAND,              IrcTabItem::ID_WHOIS,           IrcTabItem::onWhois),
    FXMAPFUNC(SEL_COMMAND,              IrcTabItem::ID_DCCCHAT,         IrcTabItem::onDccChat),
    FXMAPFUNC(SEL_COMMAND,              IrcTabItem::ID_DCCSEND,         IrcTabItem::onDccSend),
    FXMAPFUNC(SEL_COMMAND,              IrcTabItem::ID_OP,              IrcTabItem::onOp),
    FXMAPFUNC(SEL_COMMAND,              IrcTabItem::ID_DEOP,            IrcTabItem::onDeop),
    FXMAPFUNC(SEL_COMMAND,              IrcTabItem::ID_VOICE,           IrcTabItem::onVoice),
    FXMAPFUNC(SEL_COMMAND,              IrcTabItem::ID_DEVOICE,         IrcTabItem::onDevoice),
    FXMAPFUNC(SEL_COMMAND,              IrcTabItem::ID_KICK,            IrcTabItem::onKick),
    FXMAPFUNC(SEL_COMMAND,              IrcTabItem::ID_BAN,             IrcTabItem::onBan),
    FXMAPFUNC(SEL_COMMAND,              IrcTabItem::ID_KICKBAN,         IrcTabItem::onKickban),
    FXMAPFUNC(SEL_COMMAND,              IrcTabItem::ID_IGNORE,          IrcTabItem::onIgnore),
    FXMAPFUNC(SEL_COMMAND,              IrcTabItem::ID_TOPIC,           IrcTabItem::onTopic),
    FXMAPFUNC(SEL_LINK,                 IrcTabItem::ID_TOPIC,           IrcTabItem::onTopicLink),
    FXMAPFUNC(SEL_COMMAND,              dxPipe::ID_PIPE,                IrcTabItem::onPipe),
    FXMAPFUNC(SEL_COMMAND,              IrcTabItem::ID_AWAY,            IrcTabItem::onSetAway),
    FXMAPFUNC(SEL_COMMAND,              IrcTabItem::ID_DEAWAY,          IrcTabItem::onRemoveAway),
    FXMAPFUNC(SEL_COMMAND,              IrcTabItem::ID_SPELL,           IrcTabItem::onSpellLang)
};

FXIMPLEMENT(IrcTabItem, FXTabItem, IrcTabItemMap, ARRAYNUMBER(IrcTabItemMap))

IrcTabItem::IrcTabItem(dxTabBook *tab, const FXString &tabtext, FXIcon *icon, FXuint opts,
        FXint id, TYPE type, IrcSocket *socket, FXbool ownServerWindow, FXbool usersShown,
        FXbool logging, FXString commandsList, FXString logPath, FXint maxAway, IrcColor colors,
        FXString nickChar, FXFont *font, FXbool sameCommand, FXbool sameList, FXbool coloredNick,
        FXbool stripColors, FXbool useSpell, FXbool showSpellCombo)
    : FXTabItem(tab, tabtext, icon, opts), m_parent(tab), m_server(socket), m_type(type),
        m_id(id),m_usersShown(usersShown), m_logging(logging),
        m_ownServerWindow(ownServerWindow), m_sameCmd(sameCommand), m_sameList(sameList),
        m_coloredNick(coloredNick), m_stripColors(stripColors), m_useSpell(useSpell),
        m_showSpellCombo(showSpellCombo), m_colors(colors), m_commandsList(commandsList), m_logPath(logPath),
        m_maxAway(maxAway), m_nickCompletionChar(nickChar), m_logstream(NULL)
{
    m_currentPosition = 0;
    m_historyMax = 25;
    m_numberUsers = 0;
    m_maxLen = 460;
    m_checkAway = FALSE;
    m_iamOp = FALSE;
    m_topic = _("No topic is set");
    m_editableTopic = TRUE;
    m_pipe = NULL;
    m_sendPipe = FALSE;
    m_scriptHasAll = FALSE;
    m_scriptHasMyMsg = FALSE;
    m_unreadColor = FXRGB(0,0,255);
    m_highlightColor = FXRGB(255,0,0);

    if(m_type == CHANNEL && m_server->getConnected())
    {
        m_server->sendMode(getText());
    }

    m_mainframe = new FXVerticalFrame(m_parent, FRAME_RAISED|LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y);

    m_splitter = new FXSplitter(m_mainframe, LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y|SPLITTER_REVERSED|SPLITTER_TRACKING);

    m_textframe = new FXVerticalFrame(m_splitter, FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    m_topicline = new dxTextField(m_textframe, 50, this, ID_TOPIC, FRAME_SUNKEN|TEXTFIELD_ENTER_ONLY|JUSTIFY_LEFT|LAYOUT_FILL_X);
    m_topicline->setFont(font);
    m_topicline->setLinkColor(m_colors.link);
    m_topicline->setText(m_topic);
    m_topicline->setUseLink(TRUE);
    m_topicline->setTopicline(TRUE);
    if(m_type != CHANNEL)
    {
        m_topicline->hide();
    }
    m_text = new dxText(m_textframe, this, ID_TEXT, FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y|TEXT_READONLY|TEXT_WORDWRAP|TEXT_SHOWACTIVE|TEXT_AUTOSCROLL);
    m_text->setFont(font);
    m_text->setSelTextColor(getApp()->getSelforeColor());
    m_text->setSelBackColor(getApp()->getSelbackColor());

    m_usersframe = new FXVerticalFrame(m_splitter, FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_FIX_WIDTH);
    m_users = new FXList(m_usersframe, this, ID_USERS, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    m_users->setSortFunc(FXList::ascendingCase);
    m_users->setScrollStyle(HSCROLLING_OFF);
    if(m_sameList) m_users->setFont(font);
    if(m_type != CHANNEL || !m_usersShown)
    {
        m_usersframe->hide();
        m_users->hide();
    }

    m_commandframe = new FXHorizontalFrame(m_mainframe, LAYOUT_FILL_X, 0,0,0,0, 0,0,0,0);
    m_commandline = new dxTextField(m_commandframe, 25, this, ID_COMMANDLINE, TEXTFIELD_ENTER_ONLY|FRAME_SUNKEN|JUSTIFY_LEFT|LAYOUT_FILL_X|LAYOUT_BOTTOM, 0, 0, 0, 0, 1, 1, 1, 1);
    if(m_sameCmd) m_commandline->setFont(font);
    m_spellLangs = new FXComboBox(m_commandframe, 6, this, ID_SPELL, COMBOBOX_STATIC);
    m_spellLangs->setTipText(_("Spellchecking language list"));
    m_spellLangs->hide();
    if(m_sameCmd) m_spellLangs->setFont(font);
    if(m_useSpell && (m_type==CHANNEL || m_type==QUERY || m_type==DCCCHAT) && utils::instance().getLangsNum())
    {
        dxStringArray langs = utils::instance().getLangs();
        FXString lang = utils::instance().getChannelLang(getText());
        for(FXint i=0; i<langs.no(); i++)
        {
            m_spellLangs->appendItem(langs[i]);
            if(langs[i]==lang) m_spellLangs->setCurrentItem(i);;
        }
        if(m_showSpellCombo) m_spellLangs->show();
        m_commandline->setUseSpell(TRUE);
        m_commandline->setLanguage(lang);
        m_commandline->setTipText(FXStringFormat(_("Current spellchecking language: %s"),lang.text()));
    }
    
    dxHiliteStyle style = {m_colors.text,m_colors.back,getApp()->getSelforeColor(),getApp()->getSelbackColor(),0,FALSE};
    for(int i=0; i<17; i++)
    {
        m_textStyleList.append(style);
    }
    //gray text - user commands
    m_textStyleList[0].normalForeColor = m_colors.user;
    //orange text - Actions
    m_textStyleList[1].normalForeColor = m_colors.action;
    //blue text - Notice
    m_textStyleList[2].normalForeColor = m_colors.notice;
    //red text - Errors
    m_textStyleList[3].normalForeColor = m_colors.error;
    //bold style
    m_textStyleList[4].style = FXText::STYLE_BOLD;
    //underline style
    m_textStyleList[5].style = FXText::STYLE_UNDERLINE;
    //bold & underline
    m_textStyleList[6].style = FXText::STYLE_UNDERLINE;
    m_textStyleList[6].style ^=FXText::STYLE_BOLD;
    //highlight text
    m_textStyleList[7].normalForeColor = m_colors.hilight;
    //link style
    m_textStyleList[8].normalForeColor = m_colors.link;
    m_textStyleList[8].link = TRUE;
    //next styles for colored nicks
    m_textStyleList[9].normalForeColor = FXRGB(196, 160, 0);
    m_textStyleList[10].normalForeColor = FXRGB(206, 92, 0);
    m_textStyleList[11].normalForeColor = FXRGB(143, 89, 2);
    m_textStyleList[12].normalForeColor = FXRGB(78, 154, 6);
    m_textStyleList[13].normalForeColor = FXRGB(32, 74, 135);
    m_textStyleList[14].normalForeColor = FXRGB(117, 80, 123);
    m_textStyleList[15].normalForeColor = FXRGB(164, 0, 0);
    m_textStyleList[16].normalForeColor = FXRGB(85, 87, 83);

    //text->setStyled(TRUE);
    m_text->setHiliteStyles(m_textStyleList.data());

    m_text->setBackColor(m_colors.back);
    m_commandline->setBackColor(m_colors.back);
    m_topicline->setBackColor(m_colors.back);
    m_users->setBackColor(m_colors.back);
    m_text->setTextColor(m_colors.text);
    m_commandline->setTextColor(m_colors.text);
    m_commandline->setCursorColor(m_colors.text);
    m_topicline->setTextColor(m_colors.text);
    m_topicline->setCursorColor(m_colors.text);
    m_users->setTextColor(m_colors.text);
    m_spellLangs->setBackColor(m_colors.back);
    m_spellLangs->setTextColor(m_colors.text);

    this->setIconPosition(ICON_BEFORE_TEXT);
}

IrcTabItem::~IrcTabItem()
{
    this->stopLogging();
    if(m_pipe) m_pipe->stopCmd();
    m_pipeStrings.clear();
    getApp()->removeTimeout(this, ID_TIME);
    getApp()->removeTimeout(this, ID_PTIME);
}

void IrcTabItem::createGeom()
{
    m_mainframe->create();
    m_commandline->setFocus();
}

void IrcTabItem::clearChat()
{
    m_textStyleList.no(17);
    m_text->setHiliteStyles(m_textStyleList.data());
    m_text->clearText();
}

//usefull for set tab current
void IrcTabItem::makeLastRowVisible()
{
    m_text->makeLastRowVisible(TRUE);
}

FXString IrcTabItem::getSpellLang()
{
#ifdef HAVE_ENCHANT
    if(m_spellLangs->getNumItems()) return m_spellLangs->getItemText(m_spellLangs->getCurrentItem());
#endif
    return "";
}

void IrcTabItem::reparentTab()
{
    reparent(m_parent);
    m_mainframe->reparent(m_parent);
}

void IrcTabItem::hideUsers()
{
    m_usersShown = !m_usersShown;
    if(m_type == CHANNEL)
    {
        m_usersframe->hide();
        m_users->hide();
        m_splitter->setSplit(1, 0);
    }
}

void IrcTabItem::showUsers()
{
    m_usersShown = !m_usersShown;
    if(m_type == CHANNEL)
    {
        m_usersframe->show();
        m_users->show();
        m_splitter->recalc();
    }
}

void IrcTabItem::setType(const TYPE &typ, const FXString &tabtext)
{
    if(typ == CHANNEL)
    {
        if(m_usersShown) m_usersframe->show();
        if(m_usersShown) m_users->show();
        m_topicline->show();
        m_topicline->setText(m_topic);
        m_splitter->recalc();
        setText(tabtext);
        if(m_server->getConnected()) m_server->sendMode(getText());
        m_type = typ;
    }
    else if(typ == SERVER || typ == QUERY)
    {
        m_usersframe->hide();
        m_users->hide();
        m_topicline->setText("");
        m_topicline->hide();
        m_topic = _("No topic is set");
        setText(tabtext);
        m_splitter->setSplit(1, 0);
        if(m_type == CHANNEL)
        {
            m_users->clearItems();
            m_numberUsers = 0;
        }
        m_type = typ;
    }
    this->stopLogging();
    if(m_type == SERVER) this->setIcon(ICO_SERVER);
    else if(m_type == CHANNEL) this->setIcon(ICO_CHANNEL);
    else this->setIcon(ICO_QUERY);
    if(m_useSpell && (m_type==CHANNEL || m_type==QUERY) && utils::instance().getLangsNum())
    {
        dxStringArray langs = utils::instance().getLangs();
        FXString lang = utils::instance().getChannelLang(getText());
        for(FXint i=0; i<langs.no(); i++)
        {
            m_spellLangs->appendItem(langs[i]);
            if(langs[i]==lang) m_spellLangs->setCurrentItem(i);;
        }
        if(m_showSpellCombo) m_spellLangs->show();
        m_commandline->setUseSpell(TRUE);
        m_commandline->setLanguage(lang);
        m_commandline->setTipText(FXStringFormat(_("Current spellchecking language: %s"),lang.text()));
    }
    else
    {
        m_commandline->setUseSpell(FALSE);
        m_commandline->setTipText("");
        m_spellLangs->hide();
        m_commandframe->recalc();
    }
}

void IrcTabItem::setColor(IrcColor clrs)
{
    m_colors = clrs;
    setTextForeColor(clrs.text);
    setTextBackColor(clrs.back);
    setUserColor(clrs.user);
    setActionsColor(clrs.action);
    setNoticeColor(clrs.notice);
    setErrorColor(clrs.error);
    setHilightColor(clrs.hilight);
    setLinkColor(m_colors.link);
}

void IrcTabItem::setTextBackColor(FXColor clr)
{
    for(FXint i=0; i<m_textStyleList.no(); i++)
    {
        m_textStyleList[i].normalBackColor = clr;
    }
    m_text->setBackColor(clr);
    m_commandline->setBackColor(clr);
    m_topicline->setBackColor(clr);
    m_users->setBackColor(clr);
    m_spellLangs->setBackColor(clr);
}

void IrcTabItem::setTextForeColor(FXColor clr)
{
    m_textStyleList[4].normalForeColor = clr;
    m_textStyleList[5].normalForeColor = clr;
    m_textStyleList[6].normalForeColor = clr;
    m_text->setTextColor(clr);
    m_commandline->setTextColor(clr);
    m_commandline->setCursorColor(clr);
    m_topicline->setTextColor(clr);
    m_topicline->setCursorColor(clr);
    m_users->setTextColor(clr);
    m_spellLangs->setTextColor(clr);
}

void IrcTabItem::setUserColor(FXColor clr)
{
    m_textStyleList[0].normalForeColor = clr;
}

void IrcTabItem::setActionsColor(FXColor clr)
{
    m_textStyleList[1].normalForeColor = clr;
}

void IrcTabItem::setNoticeColor(FXColor clr)
{
    m_textStyleList[2].normalForeColor = clr;
}

void IrcTabItem::setErrorColor(FXColor clr)
{
    m_textStyleList[3].normalForeColor = clr;
}

void IrcTabItem::setHilightColor(FXColor clr)
{
    m_textStyleList[7].normalForeColor = clr;
}

void IrcTabItem::setLinkColor(FXColor clr)
{
    m_textStyleList[8].normalForeColor = clr;
    m_topicline->setLinkColor(clr);
}

void IrcTabItem::setUnreadTabColor(FXColor clr)
{
    if(m_unreadColor!=clr)
    {
        FXbool update = this->getTextColor()==m_unreadColor;
        m_unreadColor = clr;
        if(update) this->setTextColor(m_unreadColor);
    }
}

void IrcTabItem::setHighlightTabColor(FXColor clr)
{
    if(m_highlightColor!=clr)
    {
        FXbool update = this->getTextColor()==m_highlightColor;
        m_highlightColor = clr;
        if(update) this->setTextColor(m_highlightColor);
    }
}

void IrcTabItem::setCommandsList(FXString clst)
{
    m_commandsList = clst;
}

void IrcTabItem::setMaxAway(FXint maxa)
{
    m_maxAway = maxa;
}

void IrcTabItem::setLogging(FXbool log)
{
    m_logging = log;
}

void IrcTabItem::setLogPath(FXString pth)
{
    m_logPath = pth;
    this->stopLogging();
}

void IrcTabItem::setNickCompletionChar(FXString nichr)
{
    m_nickCompletionChar = nichr;
}

void IrcTabItem::setIrcFont(FXFont *fnt)
{
    if(m_text->getFont() != fnt)
    {
        m_text->setFont(fnt);
        m_text->recalc();
    }
    if(m_topicline->getFont() != fnt)
    {
        m_topicline->setFont(fnt);
        m_topicline->recalc();
    }
    if(m_sameCmd && m_commandline->getFont() != fnt)
    {
        m_commandline->setFont(fnt);
        m_commandline->recalc();
        m_spellLangs->setFont(fnt);
        m_spellLangs->recalc();
    }
    else
    {
        m_commandline->setFont(getApp()->getNormalFont());
        m_commandline->recalc();
        m_spellLangs->setFont(getApp()->getNormalFont());
        m_spellLangs->recalc();
    }
    if(m_sameList && m_users->getFont() != fnt)
    {
        m_users->setFont(fnt);
        m_users->recalc();
    }
    else
    {
        m_users->setFont(getApp()->getNormalFont());
        m_users->recalc();
    }
}

void IrcTabItem::setSameCmd(FXbool scmd)
{
    m_sameCmd = scmd;
}

void IrcTabItem::setSameList(FXbool slst)
{
    m_sameList = slst;
}

void IrcTabItem::setColoredNick(FXbool cnick)
{
    m_coloredNick = cnick;
}

void IrcTabItem::setStripColors(FXbool sclr)
{
    m_stripColors = sclr;
}

void IrcTabItem::setSmileys(FXbool smiley, dxSmileyArray nsmileys)
{
    m_text->setSmileys(smiley, nsmileys);
}

void IrcTabItem::setUseSpell(FXbool useSpell)
{
    m_useSpell = useSpell;
    if(m_useSpell && (m_type==CHANNEL || m_type==QUERY || m_type==DCCCHAT) && utils::instance().getLangsNum())
    {
        dxStringArray langs = utils::instance().getLangs();
        FXString lang = utils::instance().getChannelLang(getText());
        for(FXint i=0; i<langs.no(); i++)
        {
            m_spellLangs->appendItem(langs[i]);
            if(langs[i]==lang) m_spellLangs->setCurrentItem(i);;
        }
        if(m_showSpellCombo) m_spellLangs->show();
        m_commandline->setUseSpell(TRUE);
        m_commandline->setLanguage(lang);
        m_commandline->setTipText(FXStringFormat(_("Current spellchecking language: %s"),lang.text()));
    }
    else
    {
        m_commandline->setUseSpell(FALSE);
        m_commandline->setTipText("");
        m_spellLangs->hide();
    }
    m_commandframe->recalc();
}

void IrcTabItem::setShowSpellCombo(FXbool showSpellCombo)
{
    if(m_showSpellCombo!=showSpellCombo)
    {
        m_showSpellCombo = showSpellCombo;
        if(m_showSpellCombo) m_spellLangs->show();
        else m_spellLangs->hide();
        m_commandframe->recalc();
    }
}

void IrcTabItem::removeSmileys()
{
    m_text->removeSmileys();
}

//if highlight==TRUE, highlight tab
void IrcTabItem::appendText(FXString msg, FXbool highlight, FXbool logLine)
{
    appendIrcText(msg, 0, FALSE, logLine);
    if(highlight && m_highlightColor != this->getTextColor() && m_parent->getCurrent()*2 != m_parent->indexOfChild(this))
    {
        if(msg.contains(getNickName()))
        {
            this->setTextColor(m_highlightColor);
            if(m_type == CHANNEL) this->setIcon(ICO_CHANNELNEWMSG);
        }
        else this->setTextColor(m_unreadColor);
        if(m_type == QUERY) this->setIcon(ICO_QUERYNEWMSG);
    }
}

void IrcTabItem::appendIrcText(FXString msg, FXTime time, FXbool disableStrip, FXbool logLine)
{
    if(!time) time = FXSystem::now();
    if(m_type != OTHER) m_text->appendText("["+FXSystem::time("%H:%M:%S", time) +"] ");
    appendLinkText(m_stripColors && !disableStrip ? stripColors(msg, FALSE) : msg, 0);
    if(logLine) this->logLine(stripColors(msg, TRUE), time);
}

void IrcTabItem::appendIrcNickText(FXString nick, FXString msg, FXint style, FXTime time, FXbool logLine)
{
    if(!time) time = FXSystem::now();
    if(m_type != OTHER) m_text->appendText("["+FXSystem::time("%H:%M:%S", time) +"] ");
    m_text->appendStyledText(nick+": ", style);
    appendLinkText(m_stripColors ? stripColors(msg, FALSE) : msg, 0);
    if(logLine) this->logLine(stripColors("<"+nick+"> "+msg, TRUE), time);
}

/* if highlight==TRUE, highlight tab
 * disableStrip is for dxirc.Print
*/
void IrcTabItem::appendStyledText(FXString text, FXint style, FXbool highlight, FXbool disableStrip, FXbool logLine)
{
    if(style) appendIrcStyledText(text, style, 0, disableStrip, logLine);
    else appendIrcText(text, 0, disableStrip, logLine);
    if(highlight && m_highlightColor != this->getTextColor() && m_parent->getCurrent()*2 != m_parent->indexOfChild(this))
    {
        if(m_type != OTHER && text.contains(getNickName()))
        {
            this->setTextColor(m_highlightColor);
            if(m_type == CHANNEL) this->setIcon(ICO_CHANNELNEWMSG);
        }
        else this->setTextColor(m_unreadColor);
        if(m_type == QUERY) this->setIcon(ICO_QUERYNEWMSG);
    }
}

void IrcTabItem::appendIrcStyledText(FXString styled, FXint stylenum, FXTime time, FXbool disableStrip, FXbool logLine)
{
    if(!time) time = FXSystem::now();
    if(m_type != OTHER) m_text->appendText("["+FXSystem::time("%H:%M:%S", time) +"] ");
    appendLinkText(m_stripColors && !disableStrip ? stripColors(styled, TRUE) : styled, stylenum);
    if(logLine) this->logLine(stripColors(styled, TRUE), time);
}

static FXbool isBadchar(FXchar c)
{
    switch(c) {
        case ' ':
        case ',':
        case '\0':
        case '\02':
        case '\03':
        case '\017':
        case '\021':
        case '\026':
        case '\035':
        case '\037':
        case '\n':
        case '\r':
        case '<':
        case '>':
        case '"':
        case '\'':
            return TRUE;
        default:
            return FALSE;
    }
}

// checks is char nick/word delimiter
static FXbool isDelimiter(FXchar c)
{
    switch(c) {
        case ' ':
        case '.':
        case ',':
        case '/':
        case '\\':
        case '`':
        case '\'':
        case '!':
        case '(':
        case ')':
        case '{':
        case '}':
        case '|':
        case '[':
        case ']':
        case '\"':
        case ':':
        case ';':
        case '<':
        case '>':
        case '?':
            return TRUE;
        default:
            return FALSE;
    }
}

void IrcTabItem::appendLinkText(const FXString &txt, FXint stylenum)
{
    FXint i = 0;
    FXint linkLength = 0;
    FXbool bold = FALSE;
    FXbool under = FALSE;
    FXint lastStyle = stylenum;
    FXColor foreColor = stylenum && stylenum<=m_textStyleList.no() ? m_textStyleList[stylenum-1].normalForeColor : m_colors.text;
    FXColor backColor = stylenum && stylenum<=m_textStyleList.no() ? m_textStyleList[stylenum-1].normalBackColor : m_colors.back;
    FXString normalText = "";
    FXint length = txt.length();
    while(i<length)
    {
        if(txt[i]=='h' && !comparecase(txt.mid(i,7),"http://"))
        {
            if(!normalText.empty())
            {
                m_text->appendStyledText(normalText, lastStyle);
                normalText.clear();
            }
            for(FXint j=i; j<length; j++)
            {
                if(isBadchar(txt[j]))
                {
                    break;
                }
                linkLength++;
            }
            m_text->appendStyledText(txt.mid(i, linkLength), linkLength>7 ? 9 : stylenum);
            i+=linkLength-1;
            linkLength=0;
        }
        else if(txt[i]=='h' && !comparecase(txt.mid(i,8),"https://"))
        {
            if(!normalText.empty())
            {
                m_text->appendStyledText(normalText, lastStyle);
                normalText.clear();
            }
            for(FXint j=i; j<length; j++)
            {
                if(isBadchar(txt[j]))
                {
                    break;
                }
                linkLength++;
            }
            m_text->appendStyledText(txt.mid(i, linkLength), linkLength>8 ? 9 : stylenum);
            i+=linkLength-1;
            linkLength=0;
        }
        else if(txt[i]=='f' && !comparecase(txt.mid(i,6),"ftp://"))
        {
            if(!normalText.empty())
            {
                m_text->appendStyledText(normalText, lastStyle);
                normalText.clear();
            }
            for(FXint j=i; j<length; j++)
            {
                if(isBadchar(txt[j]))
                {
                    break;
                }
                linkLength++;
            }
            m_text->appendStyledText(txt.mid(i, linkLength), linkLength>6 ? 9 : stylenum);
            i+=linkLength-1;
            linkLength=0;
        }
        else if(txt[i]=='w' && !comparecase(txt.mid(i,4),"www."))
        {
            if(!normalText.empty())
            {
                m_text->appendStyledText(normalText, lastStyle);
                normalText.clear();
            }
            for(FXint j=i; j<length; j++)
            {
                if(isBadchar(txt[j]))
                {
                    break;
                }
                linkLength++;
            }
            m_text->appendStyledText(txt.mid(i, linkLength), linkLength>4 ? 9 : stylenum);
            i+=linkLength-1;
            linkLength=0;
        }
        else
        {
            if(txt[i] == '\002') //bold
            {
                if(!normalText.empty())
                {
                    m_text->appendStyledText(normalText, lastStyle);
                    normalText.clear();
                }
                bold = !bold;
                FXuint style = 0;
                if(bold && under) style = FXText::STYLE_BOLD|FXText::STYLE_UNDERLINE;
                else if(bold && !under) style = FXText::STYLE_BOLD;
                else if(!bold && under) style = FXText::STYLE_UNDERLINE;
                lastStyle = hiliteStyleExist(foreColor, backColor, style);
                if(lastStyle == -1)
                {
                    //dxText has available max. 255 styles
                    if(m_textStyleList.no()<256)
                    {
                        createHiliteStyle(foreColor, backColor, style);
                        lastStyle = m_textStyleList.no();
                    }
                    else lastStyle = 0;
                }
            }
            else if(txt[i] == '\026') //reverse
            {
                if(!normalText.empty())
                {
                    m_text->appendStyledText(normalText, lastStyle);
                    normalText.clear();
                }
                FXuint style = 0;
                if(bold && under) style = FXText::STYLE_BOLD|FXText::STYLE_UNDERLINE;
                else if(bold && !under) style = FXText::STYLE_BOLD;
                else if(!bold && under) style = FXText::STYLE_UNDERLINE;
                FXColor tempColor = foreColor;
                foreColor = backColor;
                backColor = tempColor;
                lastStyle = hiliteStyleExist(foreColor, backColor, style);
                if(lastStyle == -1)
                {
                    //dxText has available max. 255 styles
                    if(m_textStyleList.no()<256)
                    {
                        createHiliteStyle(foreColor, backColor, style);
                        lastStyle = m_textStyleList.no();
                    }
                    else lastStyle = 0;
                }
            }
            else if(txt[i] == '\037') //underline
            {
                if(!normalText.empty())
                {
                    m_text->appendStyledText(normalText, lastStyle);
                    normalText.clear();
                }
                under = !under;
                FXuint style = 0;
                if(bold && under) style = FXText::STYLE_BOLD|FXText::STYLE_UNDERLINE;
                else if(bold && !under) style = FXText::STYLE_BOLD;
                else if(!bold && under) style = FXText::STYLE_UNDERLINE;
                lastStyle = hiliteStyleExist(foreColor, backColor, style);
                if(lastStyle == -1)
                {
                    //dxText has available max. 255 styles
                    if(m_textStyleList.no()<256)
                    {
                        createHiliteStyle(foreColor, backColor, style);
                        lastStyle = m_textStyleList.no();
                    }
                    else lastStyle = 0;
                }
            }
            else if(txt[i] == '\021') //fixed
            {
                utils::instance().debugLine("Poslan fixed styl");
            }
            else if(txt[i] == '\035') //italic
            {
                utils::instance().debugLine("Poslan italic styl");
            }
            else if(txt[i] == '\003') //color
            {
                if(!normalText.empty())
                {
                    m_text->appendStyledText(normalText, lastStyle);
                    normalText.clear();
                }
                FXuint style=0;
                if(bold && under) style = FXText::STYLE_BOLD|FXText::STYLE_UNDERLINE;
                else if(bold && !under) style = FXText::STYLE_BOLD;
                else if(!bold && under) style = FXText::STYLE_UNDERLINE;
                FXbool isHexColor = FALSE;
                FXint colorLength = 0;
                foreColor = m_colors.text;
                backColor = m_colors.back;
                if(i+1<length)
                {
                    if(txt[i+1] == '#') isHexColor = TRUE;
                }
                if(isHexColor)
                {
                    if(FXRex("^\\h\\h\\h\\h\\h\\h+$").match(txt.mid(i+2,6)))
                    {
                        foreColor = FXRGB(FXIntVal(txt.mid(i+2,2),16),FXIntVal(txt.mid(i+4,2),16),FXIntVal(txt.mid(i+6,2),16));
                        colorLength +=7;
                    }
                    if(i+8 < length && txt[i+8] == ',' && FXRex("^\\h\\h\\h\\h\\h\\h+$").match(txt.mid(i+10,6)))
                    {
                        backColor = FXRGB(FXIntVal(txt.mid(i+10,2),16),FXIntVal(txt.mid(i+12,2),16),FXIntVal(txt.mid(i+14,2),16));
                        colorLength +=8;
                    }
                }
                else
                {
                    if(i+2<length)
                    {
                        FXint code = -1;
                        if(isdigit(txt[i+1]))
                        {
                            if(isdigit(txt[i+2]))
                            {
                                code = (txt[i+1]-48)*10+txt[i+2]-48;
                                colorLength +=2;
                            }
                            else
                            {
                                code = txt[i+1]-48;
                                colorLength ++;
                            }
                        }
                        if(code!=-1)
                            foreColor = getIrcColor(code%16);
                    }
                    if(i+colorLength+1 < length && txt[i+colorLength+1] == ',')
                    {
                        FXint code = -1;
                        if(isdigit(txt[i+colorLength+2]))
                        {
                            if(isdigit(txt[i+colorLength+3]))
                            {
                                code = (txt[i+colorLength+2]-48)*10+txt[i+colorLength+3]-48;
                                colorLength +=3;
                            }
                            else
                            {
                                code = txt[i+colorLength+2]-48;
                                colorLength +=2;
                            }
                        }
                        if(code!=-1)
                            backColor = getIrcColor(code%16);
                    }
                }
                lastStyle = hiliteStyleExist(foreColor, backColor, style);
                if(lastStyle == -1)
                {
                    //dxText has available max. 255 styles
                    if(m_textStyleList.no()<256)
                    {
                        createHiliteStyle(foreColor, backColor, style);
                        lastStyle = m_textStyleList.no();
                    }
                    else lastStyle = 0;
                };
                i +=colorLength;
            }
            else if(txt[i] == '\017') //reset
            {
                if(!normalText.empty())
                {
                    m_text->appendStyledText(normalText, lastStyle);
                    normalText.clear();
                }
                bold = FALSE;
                under = FALSE;
                foreColor = m_colors.text;
                backColor = m_colors.back;
                lastStyle = stylenum;
            }
            else
            {
                normalText.append(txt[i]);
            }
        }
        i++;
    }
    if(!normalText.empty())
    {
        m_text->appendStyledText(normalText, lastStyle);
    }
    m_text->appendText("\n");
}

void IrcTabItem::startLogging()
{
    if(m_logstream && FXStat::exists(m_logPath+PATHSEPSTRING+m_server->getServerName()+PATHSEPSTRING+getText()+PATHSEPSTRING+FXSystem::time("%Y-%m-%d", FXSystem::now()))) return;

    if(m_logging && m_type != SERVER)
    {
        if(!FXStat::exists(m_logPath+PATHSEPSTRING+m_server->getServerName())) FXDir::create(m_logPath+PATHSEPSTRING+m_server->getServerName());
        if(!FXStat::exists(m_logPath+PATHSEPSTRING+m_server->getServerName()+PATHSEPSTRING+getText())) FXDir::create(m_logPath+PATHSEPSTRING+m_server->getServerName()+PATHSEPSTRING+getText());
        m_logstream = new std::ofstream(FXString(m_logPath+PATHSEPSTRING+m_server->getServerName()+PATHSEPSTRING+getText()+PATHSEPSTRING+FXSystem::time("%Y-%m-%d", FXSystem::now())).text(), std::ios::out|std::ios::app);
    }
}

void IrcTabItem::stopLogging()
{
    if(m_logstream)
    {
        m_logstream->close();
        delete m_logstream;
        m_logstream = NULL;
    }
}

void IrcTabItem::logLine(const FXString &line, const FXTime &time)
{
    if(m_logging && m_type != SERVER)
    {
        this->startLogging();
        *m_logstream << "[" << FXSystem::time("%H:%M:%S", time).text() << "] " << line.text() << std::endl;
    }
}

FXbool IrcTabItem::isChannel(const FXString &text)
{
    if(text.length()) return m_server->getChanTypes().contains(text[0]);
    return FALSE;
}

long IrcTabItem::onCommandline(FXObject *, FXSelector, void *)
{
    FXString commandtext = m_commandline->getText();
    if(commandtext.empty())
        return 1;
    m_commandsHistory.append(commandtext);
    if (m_commandsHistory.no() > m_historyMax)
        m_commandsHistory.erase(0);
    m_currentPosition = m_commandsHistory.no()-1;
    m_commandline->setText("");
    if(comparecase(commandtext.left(4),"/say") != 0)
        commandtext.substitute("^B", "\002").substitute("^C", "\003").substitute("^O", "\017").substitute("^V", "\026").substitute("^_", "\037");
    for(FXint i=0; i<=commandtext.contains('\n'); i++)
    {
        FXString text = commandtext.section('\n', i).before('\r');
        if(comparecase(text.after('/').before(' '), "quit") == 0 ||
                comparecase(text.after('/').before(' '), "lua") == 0)
        {
            processLine(text);
            return 1;
        }
#ifdef HAVE_LUA
        if(text[0] != '/') m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_MYMSG), &text);
        m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_COMMAND), &text);
        if(text[0] == '/' && !m_scriptHasAll) processLine(text);
        else if(!m_scriptHasMyMsg && !m_scriptHasAll) processLine(text);
#else
        processLine(text);
#endif
    }
    return 1;
}

FXbool IrcTabItem::processLine(const FXString& commandtext)
{
    FXString command = (commandtext[0] == '/' ? commandtext.before(' ') : "");
    if(!utils::instance().getAlias(command).empty())
    {
        FXString acommand = utils::instance().getAlias(command);
        if(acommand.contains("%s"))
            acommand.substitute("%s", commandtext.after(' '));
        else
            acommand += commandtext.after(' ');
        FXint num = acommand.contains("&&");
        if(num)
        {
            FXbool result = FALSE;
            for(FXint i=0; i<=acommand.contains('&'); i++)
            {
                if(!acommand.section('&',i).trim().empty()) result = processCommand(acommand.section('&',i).trim());
            }
            return result;
        }
        else
        {
            return processCommand(acommand);
        }
    }
    return processCommand(commandtext);
}

FXbool IrcTabItem::processCommand(const FXString& commandtext)
{
    FXString command = (commandtext[0] == '/' ? commandtext.after('/').before(' ').lower() : "");
    if(m_type == DCCCHAT)
    {
        if(m_server->getConnected())
        {
            if(command == "me")
            {
                appendIrcStyledText(getNickName()+" "+commandtext.after(' '), 2, FXSystem::now());
                return m_server->sendDccChatText("\001ACTION "+commandtext.after(' ')+"\001");
            }
            if(command == "say")
            {
                if(m_coloredNick) appendIrcNickText(getNickName(), commandtext.after(' '), getNickColor(getNickName()), FXSystem::now());
                else appendIrcNickText(getNickName(), commandtext.after(' '), 5, FXSystem::now());
                return m_server->sendDccChatText(commandtext.after(' '));
            }
            if(commandtext[0] != '/')
            {
                if(m_coloredNick) appendIrcNickText(getNickName(), commandtext, getNickColor(getNickName()), FXSystem::now());
                else appendIrcNickText(getNickName(), commandtext, 5, FXSystem::now());
                return m_server->sendDccChatText(commandtext);
            }
        }
        else
            appendIrcStyledText(_("You aren't connected"), 4, FXSystem::now(), FALSE, FALSE);
        return TRUE;
    }
    if(m_type == OTHER)
    {
        if(utils::instance().isScriptCommand(command))
        {
            LuaRequest lua;
            lua.type = LUA_COMMAND;
            lua.text = commandtext.after('/');
            m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_LUA), &lua);
            return TRUE;
        }
        if(command == "commands")
        {
            appendIrcStyledText(utils::instance().availableScriptsCommands(), 3, FXSystem::now(), FALSE, FALSE);
            return TRUE;
        }
        if(command == "egg")
        {
            m_text->clearText();
            m_text->appendStyledText(FXString("ahoj sem pan Vajíčko.\n"), 3);
            getApp()->addTimeout(this, ID_ETIME, 1000);
            m_pics = 0;
            return TRUE;
        }
        if(command == "help")
        {
            return showHelp(commandtext.after(' ').lower().trim());
        }
        if(command == "tetris")
        {
            m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_NEWTETRIS), NULL);
            return TRUE;
        }
        return TRUE;
    }
    if(commandtext[0] == '/')
    {
        if(utils::instance().isScriptCommand(command))
        {
            LuaRequest lua;
            lua.type = LUA_COMMAND;
            lua.text = commandtext.after('/');
            m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_LUA), &lua);
            return TRUE;
        }
        if(command == "admin")
        {
            if(m_server->getConnected()) return m_server->sendAdmin(commandtext.after(' '));
            else
            {
                appendIrcStyledText(_("You aren't connected"), 4, FXSystem::now(), FALSE, FALSE);
                m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "away")
        {
            if(m_server->getConnected())
            {
                if(commandtext.after(' ').length() > m_server->getAwayLen())
                {
                    appendIrcStyledText(FXStringFormat(_("Warning: Away message is too long. Max. away message length is %d."), m_server->getAwayLen()), 4, FXSystem::now(), FALSE, FALSE);
                    return m_server->sendAway(commandtext.after(' '));
                }
                else
                    return m_server->sendAway(commandtext.after(' '));
            }
            else
            {
                appendIrcStyledText(_("You aren't connected"), 4, FXSystem::now(), FALSE, FALSE);
                m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "banlist")
        {
            if(m_server->getConnected())
            {
                FXString channel = commandtext.after(' ');
                if(channel.empty() && m_type == CHANNEL) return m_server->sendBanlist(getText());
                else if(!isChannel(channel) && m_type != CHANNEL)
                {
                    appendIrcStyledText(_("/banlist <channel>, shows banlist for channel."), 4, FXSystem::now(), FALSE, FALSE);
                    return FALSE;
                }
                else return m_server->sendBanlist(channel);
            }
            else
            {
                appendIrcStyledText(_("You aren't connected"), 4, FXSystem::now(), FALSE, FALSE);
                m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "connect")
        {
            if(commandtext.after(' ').empty())
            {
                appendIrcStyledText(_("/connect <server> [port] [nick] [password] [realname] [channels], connects for given server."), 4, FXSystem::now(), FALSE, FALSE);
                return FALSE;
            }
            else
            {
                ServerInfo srv;
                srv.hostname = commandtext.after(' ').section(' ', 0);
                srv.port = commandtext.after(' ').section(' ', 1).empty() ? 6667 : FXIntVal(commandtext.after(' ').section(' ', 1));
                srv.nick = commandtext.after(' ').section(' ', 2).empty() ? FXSystem::currentUserName() : commandtext.after(' ').section(' ', 2);
                srv.passwd = commandtext.after(' ').section(' ', 3).empty() ? "" : commandtext.after(' ').section(' ', 3);
                srv.realname = commandtext.after(' ').section(' ', 4).empty() ? FXSystem::currentUserName() : commandtext.after(' ').section(' ', 4);
                srv.channels = commandtext.after(' ').section(' ', 5).empty() ? "" : commandtext.after(' ').section(' ', 5);
                m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CSERVER), &srv);
                return TRUE;
            }
        }
        if(command == "commands")
        {
            appendIrcStyledText(utils::instance().availableCommands(), 3, FXSystem::now(), FALSE, FALSE);
            return TRUE;
        }
        if(command == "ctcp")
        {
            if(m_server->getConnected())
            {
                FXString to = commandtext.after(' ').before(' ');
                FXString msg = commandtext.after(' ', 2);
                if(to.empty() || msg.empty())
                {
                    appendIrcStyledText(_("/ctcp <nick> <message>, sends a CTCP message to a user."), 4, FXSystem::now(), FALSE, FALSE);
                    return FALSE;
                }
                else if(msg.length() > m_maxLen-12-to.length())
                {
                    appendIrcStyledText(FXStringFormat(_("Warning: ctcp message is too long. Max. ctcp message length is %d."), m_maxLen-12-to.length()), 4, FXSystem::now(), FALSE, FALSE);
                    return m_server->sendCtcp(to, msg);
                }
                else return m_server->sendCtcp(to, msg);
            }
            else
            {
                appendIrcStyledText(_("You aren't connected"), 4, FXSystem::now(), FALSE, FALSE);
                m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "cycle")
        {
            if(m_server->getConnected())
            {
                if(m_type == CHANNEL)
                {
                    if(isChannel(commandtext.after(' ')))
                    {
                        FXString channel = commandtext.after(' ').before(' ');
                        FXString reason = commandtext.after(' ', 2);
                        reason.empty() ? m_server->sendPart(channel) : m_server->sendPart(channel, reason);
                        return m_server->sendJoin(channel);
                    }
                    else
                    {
                        commandtext.after(' ').empty() ? m_server->sendPart(getText()) : m_server->sendPart(getText(), commandtext.after(' '));
                        return m_server->sendJoin(getText());
                    }
                }
                else
                {
                    if(isChannel(commandtext.after(' ')))
                    {
                        FXString channel = commandtext.after(' ').before(' ');
                        FXString reason = commandtext.after(' ', 2);
                        reason.empty() ? m_server->sendPart(channel) : m_server->sendPart(channel, reason);
                        return m_server->sendJoin(channel);
                    }
                    else
                    {
                        appendIrcStyledText(_("/cycle <channel> [message], leaves and join channel."), 4, FXSystem::now(), FALSE, FALSE);
                        return FALSE;
                    }
                }
            }
            else
            {
                appendIrcStyledText(_("You aren't connected"), 4, FXSystem::now(), FALSE, FALSE);
                m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "dcc")
        {
            if(m_server->getConnected())
            {
                FXString dccCommand = utils::instance().getParam(commandtext, 2, FALSE).lower();
                if(dccCommand == "chat")
                {
                    FXString nick = utils::instance().getParam(commandtext, 3, FALSE);
                    if(!comparecase(nick, "chat"))
                    {
                        appendIrcStyledText(_("Nick for chat wasn't entered."), 4, FXSystem::now(), FALSE, FALSE);
                        return FALSE;
                    }
                    if(!comparecase(nick, getNickName()))
                    {
                        appendIrcStyledText(_("Chat with yourself isn't good idea."), 4, FXSystem::now(), FALSE, FALSE);
                        return FALSE;
                    }
                    IrcEvent ev;
                    ev.eventType = IRC_DCCSERVER;
                    ev.param1 = nick;
                    m_parent->getParent()->getParent()->handle(m_server, FXSEL(SEL_COMMAND, IrcSocket::ID_SERVER), &ev);
                    return TRUE;
                }
                else if(dccCommand == "send")
                {
                    FXString nick = utils::instance().getParam(commandtext, 3, FALSE);
                    FXString file = utils::instance().getParam(commandtext, 4, TRUE);
                    if(!comparecase(nick, "send"))
                    {
                        appendIrcStyledText(_("Nick for sending file wasn't entered."), 4, FXSystem::now(), FALSE, FALSE);
                        return FALSE;
                    }
                    if(!comparecase(nick, getNickName()))
                    {
                        appendIrcStyledText(_("Sending to yourself isn't good idea."), 4, FXSystem::now(), FALSE, FALSE);
                        return FALSE;
                    }
                    if(!comparecase(nick, file))
                    {
                        appendIrcStyledText(_("Filename wasn't entered"), 4, FXSystem::now(), FALSE, FALSE);
                        return FALSE;
                    }
                    if(!FXStat::exists(file))
                    {
                        appendIrcStyledText(FXStringFormat(_("File '%s' doesn't exist"), file.text()), 4, FXSystem::now(), FALSE, FALSE);
                        return FALSE;
                    }
                    IrcEvent ev;
                    ev.eventType = IRC_DCCOUT;
                    ev.param1 = nick;
                    ev.param2 = file;
                    m_parent->getParent()->getParent()->handle(m_server, FXSEL(SEL_COMMAND, IrcSocket::ID_SERVER), &ev);
                    return TRUE;
                }
                else if(dccCommand == "psend")
                {
                    FXString nick = utils::instance().getParam(commandtext, 3, FALSE);
                    FXString file = utils::instance().getParam(commandtext, 4, TRUE);
                    if(!comparecase(nick, "psend"))
                    {
                        appendIrcStyledText(_("Nick for sending file wasn't entered."), 4, FXSystem::now(), FALSE, FALSE);
                        return FALSE;
                    }
                    if(!comparecase(nick, getNickName()))
                    {
                        appendIrcStyledText(_("Sending to yourself isn't good idea."), 4, FXSystem::now(), FALSE, FALSE);
                        return FALSE;
                    }
                    if(!comparecase(nick, file))
                    {
                        appendIrcStyledText(_("Filename wasn't entered"), 4, FXSystem::now(), FALSE, FALSE);
                        return FALSE;
                    }
                    if(!FXStat::exists(file))
                    {
                        appendIrcStyledText(FXStringFormat(_("File '%s' doesn't exist"), file.text()), 4, FXSystem::now(), FALSE, FALSE);
                        return FALSE;
                    }
                    IrcEvent ev;
                    ev.eventType = IRC_DCCPOUT;
                    ev.param1 = nick;
                    ev.param2 = file;
                    m_parent->getParent()->getParent()->handle(m_server, FXSEL(SEL_COMMAND, IrcSocket::ID_SERVER), &ev);
                    return TRUE;
                }
                else
                {
                    appendIrcStyledText(FXStringFormat(_("'%s' isn't dcc command <chat|send|psend>"), dccCommand.text()), 4, FXSystem::now(), FALSE, FALSE);
                    return FALSE;
                }
            }
            else
            {
                appendIrcStyledText(_("You aren't connected"), 4, FXSystem::now(), FALSE, FALSE);
                m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "deop")
        {
            if(m_server->getConnected())
            {
                FXString params = commandtext.after(' ');
                if(m_type == CHANNEL)
                {
                    if(params.empty())
                    {
                        appendIrcStyledText(_("/deop <nicks>, removes operator status from one or more nicks."), 4, FXSystem::now(), FALSE, FALSE);
                        return FALSE;
                    }
                    else if(isChannel(params) && params.after(' ').empty())
                    {
                        appendIrcStyledText(_("/deop <channel> <nicks>, removes operator status from one or more nicks."), 4, FXSystem::now(), FALSE, FALSE);
                        return FALSE;
                    }
                    else if(isChannel(params) && !params.after(' ').empty())
                    {
                        FXString channel = params.before(' ');
                        FXString nicks = params.after(' ');
                        FXString modeparams = utils::instance().createModes('-', 'o', nicks);
                        return m_server->sendMode(channel+" "+modeparams);
                    }
                    else
                    {
                        FXString channel = getText();
                        FXString nicks = params;
                        FXString modeparams = utils::instance().createModes('-', 'o', nicks);
                        return m_server->sendMode(channel+" "+modeparams);
                    }
                }
                else
                {
                    if(isChannel(params) && !params.after(' ').empty())
                    {
                        FXString channel = params.before(' ');
                        FXString nicks = params.after(' ');
                        FXString modeparams = utils::instance().createModes('-', 'o', nicks);
                        return m_server->sendMode(channel+" "+modeparams);
                    }
                    else
                    {
                        appendIrcStyledText(_("/deop <channel> <nicks>, removes operator status from one or more nicks."), 4, FXSystem::now(), FALSE, FALSE);
                        return FALSE;
                    }
                }
            }
            else
            {
                appendIrcStyledText(_("You aren't connected"), 4, FXSystem::now(), FALSE, FALSE);
                m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "devoice")
        {
            if(m_server->getConnected())
            {
                FXString params = commandtext.after(' ');
                if(m_type == CHANNEL)
                {
                    if(params.empty())
                    {
                        appendIrcStyledText(_("/devoice <nicks>, removes voice from one or more nicks."), 4, FXSystem::now(), FALSE, FALSE);
                        return FALSE;
                    }
                    else if(isChannel(params) && params.after(' ').empty())
                    {
                        appendIrcStyledText(_("/devoice <channel> <nicks>, removes voice from one or more nicks."), 4, FXSystem::now(), FALSE, FALSE);
                        return FALSE;
                    }
                    else if(isChannel(params) && !params.after(' ').empty())
                    {
                        FXString channel = params.before(' ');
                        FXString nicks = params.after(' ');
                        FXString modeparams = utils::instance().createModes('-', 'v', nicks);
                        return m_server->sendMode(channel+" "+modeparams);
                    }
                    else
                    {
                        FXString channel = getText();
                        FXString nicks = params;
                        FXString modeparams = utils::instance().createModes('-', 'v', nicks);
                        return m_server->sendMode(channel+" "+modeparams);
                    }
                }
                else
                {
                    if(isChannel(params) && !params.after(' ').empty())
                    {
                        FXString channel = params.before(' ');
                        FXString nicks = params.after(' ');
                        FXString modeparams = utils::instance().createModes('-', 'v', nicks);
                        return m_server->sendMode(channel+" "+modeparams);
                    }
                    else
                    {
                        appendIrcStyledText(_("/devoice <channel> <nicks>, removes voice from one or more nicks."), 4, FXSystem::now(), FALSE, FALSE);
                        return FALSE;
                    }
                }
            }
            else
            {
                appendIrcStyledText(_("You aren't connected"), 4, FXSystem::now(), FALSE, FALSE);
                m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "disconnect")
        {
            if(m_server->getConnected())
            {
                if(commandtext.after(' ').empty()) m_server->disconnect();
                else m_server->disconnect(commandtext.after(' '));
                return TRUE;
            }
            else
            {
                m_server->closeConnection(TRUE);
                return TRUE;
            }
        }
        if(command == "dxirc")
        {
            for(FXint i=0; i<5+rand()%5; i++)
            {
                appendIrcText("", FXSystem::now(), FALSE, FALSE);
            }
            appendIrcText("     __  __         _", FXSystem::now(), FALSE, FALSE);
            appendIrcText("  _/__//__/|_      | |       _", FXSystem::now(), FALSE, FALSE);
            appendIrcText(" /_| |_| |/_/|   __| |__  __|_| _ _  ___", FXSystem::now(), FALSE, FALSE);
            appendIrcText(" |_   _   _|/   / _  |\\ \\/ /| || '_)/ __)", FXSystem::now(), FALSE, FALSE);
            appendIrcText(" /_| |_| |/_/| | (_| | |  | | || | | (__", FXSystem::now(), FALSE, FALSE);
            appendIrcText(" |_   _   _|/   \\____|/_/\\_\\|_||_|  \\___)", FXSystem::now(), FALSE, FALSE);
            appendIrcText("   |_|/|_|/     (c) 2008~ David Vachulka", FXSystem::now(), FALSE, FALSE);
            appendIrcText("   http://dxirc.org", FXSystem::now(), FALSE, FALSE);
            for(FXint i=0; i<5+rand()%5; i++)
            {
                appendIrcText("", FXSystem::now(), FALSE, FALSE);
            }
            return TRUE;
        }
        if(command == "egg")
        {
            m_text->clearText();
            m_text->appendStyledText(FXString("ahoj sem pan Vajíčko.\n"), 3);
            getApp()->addTimeout(this, ID_ETIME, 1000);
            m_pics = 0;
            return TRUE;
        }
        if(command == "exec")
        {
            FXString params = commandtext.after(' ');
            if(params.empty())
            {
                appendIrcStyledText(_("/exec [-o|-c] <command>, executes command, -o sends output to channel/query, -c closes running command."), 4, FXSystem::now(), FALSE, FALSE);
                return FALSE;
            }
            else
            {
                if(!m_pipe) m_pipe = new dxPipe(getApp(), this);
                m_pipeStrings.clear();
                if(params.before(' ').contains("-o"))
                {
                    m_sendPipe = TRUE;
                    m_pipe->execCmd(params.after(' '));
                }
                else if(params.before(' ').contains("-c"))
                {
                    m_sendPipe = FALSE;
                    m_pipeStrings.clear();
                    m_pipe->stopCmd();
                }
                else
                {
                    m_sendPipe = FALSE;
                    m_pipe->execCmd(params);
                }
                return TRUE;
            }
        }
        if(command == "help")
        {
            return showHelp(commandtext.after(' ').lower().trim());
        }
        if(command == "ignore")
        {
            FXString ignorecommand = commandtext.after(' ').before(' ');
            FXString ignoretext = commandtext.after(' ').after(' ');
            if(ignorecommand.empty())
            {
                appendIrcStyledText(_("/ignore <list|addcmd|rmcmd|addusr|rmusr> [command|user] [channel] [server]"), 4, FXSystem::now(), FALSE, FALSE);
                return FALSE;
            }
            if(comparecase(ignorecommand, "list")==0)
            {
                appendIrcStyledText(_("Ignored commands:"), 7, FXSystem::now(), FALSE, FALSE);
                if(m_commandsList.empty()) appendIrcText(_("No ignored commands"), FXSystem::now(), FALSE, FALSE);
                else appendIrcText(m_commandsList.rbefore(';'), FXSystem::now(), FALSE, FALSE);
                appendIrcStyledText(_("Ignored users:"), 7, FXSystem::now(), FALSE, FALSE);
                dxIgnoreUserArray users = m_server->getUsersList();
                if(!users.no()) appendIrcText(_("No ignored users"), FXSystem::now(), FALSE, FALSE);
                else
                {
                    for(FXint i=0; i<users.no(); i++)
                    {
                        appendIrcText(FXStringFormat(_("%s on channel(s): %s and server(s): %s"), users[i].nick.text(), users[i].channel.text(), users[i].server.text()), FXSystem::now(), FALSE, FALSE);
                    }
                }
                return TRUE;
            }
            else if(comparecase(ignorecommand, "addcmd")==0)
            {
                if(ignoretext.empty())
                {
                    appendIrcStyledText(_("/ignore addcmd <command>, adds command to ignored commands."), 4, FXSystem::now(), FALSE, FALSE);
                    return FALSE;
                }
                m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_ADDICOMMAND), &ignoretext);
                return TRUE;
            }
            else if(comparecase(ignorecommand, "rmcmd")==0)
            {
                if(ignoretext.empty())
                {
                    appendIrcStyledText(_("/ignore rmcmd <command>, removes command from ignored commands."), 4, FXSystem::now(), FALSE, FALSE);
                    return FALSE;
                }
                m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_RMICOMMAND), &ignoretext);
                return TRUE;
            }
            else if(comparecase(ignorecommand, "addusr")==0)
            {
                if(ignoretext.empty())
                {
                    appendIrcStyledText(_("/ignore addusr <user> [channel] [server], adds user to ignored users."), 4, FXSystem::now(), FALSE, FALSE);
                    return FALSE;
                }
                m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_ADDIUSER), &ignoretext);
                return TRUE;
            }
            else if(comparecase(ignorecommand, "rmusr")==0)
            {
                if(ignoretext.empty())
                {
                    appendIrcStyledText(_("/ignore rmusr <user>, removes user from ignored users."), 4, FXSystem::now(), FALSE, FALSE);
                    return FALSE;
                }
                m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_RMIUSER), &ignoretext);
                return TRUE;
            }
            else
            {
                appendIrcStyledText(FXStringFormat(_("'%s' isn't <list|addcmd|rmcmd|addusr|rmusr>"), ignorecommand.text()), 4, FXSystem::now(), FALSE, FALSE);
                return FALSE;
            }
        }
        if(command == "invite")
        {
            if(m_server->getConnected())
            {
                FXString params = commandtext.after(' ');
                if(params.empty())
                {
                    appendIrcStyledText(_("/invite <nick> <channel>, invites someone to a channel."), 4, FXSystem::now(), FALSE, FALSE);
                    return FALSE;
                }
                else if(isChannel(params) && params.after(' ').empty())
                {
                    appendIrcStyledText(_("/invite <nick> <channel>, invites someone to a channel."), 4, FXSystem::now(), FALSE, FALSE);
                    return FALSE;
                }
                else
                {
                    FXString nick = params.before(' ');
                    FXString channel = params.after(' ');
                    return m_server->sendInvite(nick, channel);
                }
            }
            else
            {
                appendIrcStyledText(_("You aren't connected"), 4, FXSystem::now(), FALSE, FALSE);
                m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "join")
        {
            if(m_server->getConnected())
            {
                FXString channel = commandtext.after(' ');
                if(!isChannel(channel))
                {
                    appendIrcStyledText(_("/join <channel>, joins a channel."), 4, FXSystem::now(), FALSE, FALSE);
                    appendIrcStyledText(FXStringFormat(_("'%c' isn't valid char for channel."), channel[0]), 4, FXSystem::now(), FALSE, FALSE);
                    return FALSE;
                }
                else return m_server->sendJoin(channel);
            }
            else
            {
                appendIrcStyledText(_("You aren't connected"), 4, FXSystem::now(), FALSE, FALSE);
                m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "kick")
        {
            if(m_server->getConnected())
            {
                FXString params = commandtext.after(' ');
                if(m_type == CHANNEL)
                {
                    if(params.empty())
                    {
                        appendIrcStyledText(_("/kick <nick>, kicks a user from a channel."), 4, FXSystem::now(), FALSE, FALSE);
                        return FALSE;
                    }
                    else if(isChannel(params) && params.after(' ').empty())
                    {
                        appendIrcStyledText(_("/kick <channel> <nick>, kicks a user from a channel."), 4, FXSystem::now(), FALSE, FALSE);
                        return FALSE;
                    }
                    else if(isChannel(params) && !params.after(' ').empty())
                    {
                        FXString channel = params.before(' ');
                        FXString nick = params.after(' ');
                        FXString reason = params.after(' ', 2);
                        if(reason.length() > m_server->getKickLen())
                        {
                            appendIrcStyledText(FXStringFormat(_("Warning: reason of kick is too long. Max. reason length is %d."), m_server->getKickLen()), 4, FXSystem::now(), FALSE, FALSE);
                            return m_server->sendKick(channel, nick, reason);
                        }
                        else return m_server->sendKick(channel, nick, reason);
                    }
                    else
                    {
                        FXString channel = getText();
                        FXString nick = params.before(' ');
                        FXString reason = params.after(' ');
                        if(reason.length() > m_server->getKickLen())
                        {
                            appendIrcStyledText(FXStringFormat(_("Warning: reason of kick is too long. Max. reason length is %d."), m_server->getKickLen()), 4, FXSystem::now(), FALSE, FALSE);
                            return m_server->sendKick(channel, nick, reason);
                        }
                        else return m_server->sendKick(channel, nick, reason);
                    }
                }
                else
                {
                    if(isChannel(params) && !params.after(' ').empty())
                    {
                        FXString channel = params.before(' ');
                        FXString nick = params.after(' ');
                        FXString reason = params.after(' ', 2);
                        if(reason.length() > m_server->getKickLen())
                        {
                            appendIrcStyledText(FXStringFormat(_("Warning: reason of kick is too long. Max. reason length is %d."), m_server->getKickLen()), 4, FXSystem::now(), FALSE, FALSE);
                            return m_server->sendKick(channel, nick, reason);
                        }
                        else return m_server->sendKick(channel, nick, reason);
                    }
                    else
                    {
                        appendIrcStyledText(_("/kick <channel> <nick>, kicks a user from a channel."), 4, FXSystem::now(), FALSE, FALSE);
                        return FALSE;
                    }
                }
            }
            else
            {
                appendIrcStyledText(_("You aren't connected"), 4, FXSystem::now(), FALSE, FALSE);
                m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "kill")
        {
            if(m_server->getConnected())
            {
                FXString params = commandtext.after(' ');
                FXString nick = params.before(' ');
                FXString reason = params.after(' ');
                if(params.empty())
                {
                    appendIrcStyledText(_("/kill <user> [reason], kills a user from the network."), 4, FXSystem::now(), FALSE, FALSE);
                    return FALSE;
                }
                if(reason.length() > m_maxLen-7-nick.length())
                {
                    appendIrcStyledText(FXStringFormat(_("Warning: reason of kill is too long. Max. reason length is %d."), m_maxLen-7-nick.length()), 4, FXSystem::now(), FALSE, FALSE);
                    return m_server->sendKill(nick, reason);
                }
                else return m_server->sendKill(nick, reason);
            }
            else
            {
                appendIrcStyledText(_("You aren't connected"), 4, FXSystem::now(), FALSE, FALSE);
                m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "list")
        {
            if(m_server->getConnected()) return m_server->sendList(commandtext.after(' '));
            else
            {
                appendIrcStyledText(_("You aren't connected"), 4, FXSystem::now(), FALSE, FALSE);
                m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "lua")
        {
#ifdef HAVE_LUA
            FXString luacommand = commandtext.after(' ').before(' ');
            FXString luatext = commandtext.after(' ').after(' ');
            LuaRequest lua;
            if(luacommand.empty())
            {
                appendIrcStyledText(_("/lua <help|load|unload|list> [scriptpath|scriptname]"), 4, FXSystem::now(), FALSE, FALSE);
                return FALSE;
            }
            if(comparecase(luacommand, "help")==0)
            {
                appendIrcStyledText(FXStringFormat(_("For help about Lua scripting visit: %s"), LUA_HELP_PATH), 3, FXSystem::now(), FALSE, FALSE);
                return TRUE;
            }
            else if(comparecase(luacommand, "load")==0) lua.type = LUA_LOAD;
            else if(comparecase(luacommand, "unload")==0) lua.type = LUA_UNLOAD;
            else if(comparecase(luacommand, "list")==0) lua.type = LUA_LIST;            
            else
            {
                appendIrcStyledText(FXStringFormat(_("'%s' isn't <help|load|unload|list>"), luacommand.text()), 4, FXSystem::now(), FALSE, FALSE);
                return FALSE;
            }
            lua.text = luatext;
            m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_LUA), &lua);
            return TRUE;
#else
            appendIrcStyledText(_("dxirc is compiled without support for Lua scripting"), 4, FXSystem::now(), FALSE, FALSE);
            return FALSE;
#endif
        }
        if(command == "me")
        {
            if(m_server->getConnected())
            {
                FXString params = commandtext.after(' ');
                if(m_type == CHANNEL)
                {
                    if(params.empty())
                    {
                        appendIrcStyledText(_("/me <message>, sends the action to the current channel."), 4, FXSystem::now(), FALSE, FALSE);
                        return FALSE;
                    }
                    else if(isChannel(params) && !params.after(' ').empty())
                    {
                        FXString channel = params.before(' ');
                        FXString message = params.after(' ');
                        if(channel == getText())
                        {
                            appendIrcStyledText(getNickName()+" "+message, 2, FXSystem::now());
                            IrcEvent ev;
                            ev.eventType = IRC_ACTION;
                            ev.param1 = getNickName();
                            ev.param2 = channel;
                            ev.param3 = message;
                            m_parent->getParent()->getParent()->handle(m_server, FXSEL(SEL_COMMAND, IrcSocket::ID_SERVER), &ev);
                        }
                        if(message.length() > m_maxLen-19-channel.length())
                        {
                            dxStringArray messages = cutText(message, m_maxLen-19-channel.length());
                            FXbool result = TRUE;
                            for(FXint i=0; i<messages.no(); i++)
                            {
                                result = m_server->sendMe(channel, messages[i]) &result;
                            }
                            return result;
                        }
                        else return m_server->sendMe(channel, message);
                    }
                    else
                    {
                        appendIrcStyledText(getNickName()+" "+params, 2, FXSystem::now());
                        IrcEvent ev;
                        ev.eventType = IRC_ACTION;
                        ev.param1 = getNickName();
                        ev.param2 = getText();
                        ev.param3 = params;
                        m_parent->getParent()->getParent()->handle(m_server, FXSEL(SEL_COMMAND, IrcSocket::ID_SERVER), &ev);
                        if(params.length() > m_maxLen-19-getText().length())
                        {
                            dxStringArray messages = cutText(params, m_maxLen-19-getText().length());
                            FXbool result = TRUE;
                            for(FXint i=0; i<messages.no(); i++)
                            {
                                result = m_server->sendMe(getText(), messages[i]) &result;
                            }
                            return result;
                        }
                        else return m_server->sendMe(getText(), params);
                    }
                }
                else if(m_type == QUERY)
                {
                    if(params.empty())
                    {
                        appendIrcStyledText(_("/me <message>, sends the action to the current query."), 4, FXSystem::now(), FALSE, FALSE);
                        return FALSE;
                    }
                    else
                    {
                        appendIrcStyledText(getNickName()+" "+params, 2, FXSystem::now());
                        IrcEvent ev;
                        ev.eventType = IRC_ACTION;
                        ev.param1 = getNickName();
                        ev.param2 = getText();
                        ev.param3 = params;
                        m_parent->getParent()->getParent()->handle(m_server, FXSEL(SEL_COMMAND, IrcSocket::ID_SERVER), &ev);
                        if(params.length() > m_maxLen-19-getText().length())
                        {
                            dxStringArray messages = cutText(params, m_maxLen-19-getText().length());
                            FXbool result = TRUE;
                            for(FXint i=0; i<messages.no(); i++)
                            {
                                result = m_server->sendMe(getText(), messages[i]) &result;
                            }
                            return result;
                        }
                        else return m_server->sendMe(getText(), params);
                    }
                }
                else
                {
                    if(!params.after(' ').empty())
                    {
                        FXString to = params.before(' ');
                        FXString message = params.after(' ');
                        if(message.length() > m_maxLen-19-to.length())
                        {
                            dxStringArray messages = cutText(message, m_maxLen-19-to.length());
                            FXbool result = TRUE;
                            for(FXint i=0; i<messages.no(); i++)
                            {
                                result = m_server->sendMe(to, messages[i]) &result;
                            }
                            return result;
                        }
                        else return m_server->sendMe(to, message);
                    }
                    else
                    {
                        appendIrcStyledText(_("/me <to> <message>, sends the action."), 4, FXSystem::now(), FALSE, FALSE);
                        return FALSE;
                    }
                }
            }
            else
            {
                appendIrcStyledText(_("You aren't connected"), 4, FXSystem::now(), FALSE, FALSE);
                m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "mode")
        {
            if(m_server->getConnected())
            {
                FXString params = commandtext.after(' ');
                if(params.empty())
                {
                    appendIrcStyledText(_("/mode <channel> <modes>, sets modes for a channel."), 4, FXSystem::now(), FALSE, FALSE);
                    return FALSE;
                }
                else
                    return m_server->sendMode(params);
            }
            else
            {
                appendIrcStyledText(_("You aren't connected"), 4, FXSystem::now(), FALSE, FALSE);
                m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "msg")
        {
            if(m_server->getConnected())
            {
                FXString params = commandtext.after(' ');
                FXString to = params.before(' ');
                FXString message = params.after(' ');
                if(!to.empty() && !message.empty())
                {
                    if(to == getText())
                    {
                        if(m_coloredNick) appendIrcNickText(getNickName(), message, getNickColor(getNickName()), FXSystem::now());
                        else appendIrcNickText(getNickName(), message, 5, FXSystem::now());
                        IrcEvent ev;
                        ev.eventType = IRC_PRIVMSG;
                        ev.param1 = getNickName();
                        ev.param2 = to;
                        ev.param3 = message;
                        m_parent->getParent()->getParent()->handle(m_server, FXSEL(SEL_COMMAND, IrcSocket::ID_SERVER), &ev);
                    }
                    if(message.length() > m_maxLen-10-to.length())
                    {
                        dxStringArray messages = cutText(message, m_maxLen-10-to.length());
                        FXbool result = TRUE;
                        for(FXint i=0; i<messages.no(); i++)
                        {
                            result = m_server->sendMsg(to, messages[i]) &result;
                        }
                        return result;
                    }
                    else return m_server->sendMsg(to, message);
                }
                else
                {
                    appendIrcStyledText(_("/msg <nick/channel> <message>, sends a normal message."), 4, FXSystem::now(), FALSE, FALSE);
                    return FALSE;
                }
            }
            else
            {
                appendIrcStyledText(_("You aren't connected"), 4, FXSystem::now(), FALSE, FALSE);
                m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "names")
        {
            if(m_server->getConnected())
            {
                FXString params = commandtext.after(' ');
                if(m_type == CHANNEL)
                {
                    if(params.empty()) return m_server->sendNames(getText());
                    else return m_server->sendNames(params);
                }
                else
                {
                    if(params.empty())
                    {
                        appendIrcStyledText(_("/names <channel>, for nicks on a channel."), 4, FXSystem::now(), FALSE, FALSE);
                        return FALSE;
                    }
                    else return m_server->sendNames(params);
                }
            }
            else
            {
                appendIrcStyledText(_("You aren't connected"), 4, FXSystem::now(), FALSE, FALSE);
                m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "nick")
        {
            if(m_server->getConnected())
            {
                FXString nick = commandtext.after(' ');
                if(nick.empty())
                {
                    appendIrcStyledText(_("/nick <nick>, changes nick."), 4, FXSystem::now(), FALSE, FALSE);
                    return FALSE;
                }
                else if(nick.length() > m_server->getNickLen())
                {
                    appendIrcStyledText(FXStringFormat(_("Warning: nick is too long. Max. nick length is %d."), m_server->getNickLen()), 4, FXSystem::now(), FALSE, FALSE);
                    return m_server->sendNick(nick);
                }
                else
                {
                    return m_server->sendNick(nick);
                }
            }
            else
            {
                appendIrcStyledText(_("You aren't connected"), 4, FXSystem::now(), FALSE, FALSE);
                m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "notice")
        {
            if(m_server->getConnected())
            {
                FXString params = commandtext.after(' ');
                FXString to = params.before(' ');
                FXString message = params.after(' ');
                if(!to.empty() && !message.empty())
                {
                    appendIrcStyledText(FXStringFormat(_("NOTICE to %s: %s"), to.text(), message.text()), 2, FXSystem::now());
                    if(message.length() > m_maxLen-9-to.length())
                    {
                        dxStringArray messages = cutText(message, m_maxLen-9-to.length());
                        FXbool result = TRUE;
                        for(FXint i=0; i<messages.no(); i++)
                        {
                            result = m_server->sendNotice(to, messages[i]) &result;
                        }
                        return result;
                    }
                    return m_server->sendNotice(to, message);
                }
                else
                {
                    appendIrcStyledText(_("/notice <nick/channel> <message>, sends a notice."), 4, FXSystem::now(), FALSE, FALSE);
                    return FALSE;
                }
            }
            else
            {
                appendIrcStyledText(_("You aren't connected"), 4, FXSystem::now(), FALSE, FALSE);
                m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "op")
        {
            if(m_server->getConnected())
            {
                FXString params = commandtext.after(' ');
                if(m_type == CHANNEL)
                {
                    if(params.empty())
                    {
                        appendIrcStyledText(_("/op <nicks>, gives operator status for one or more nicks."), 4, FXSystem::now(), FALSE, FALSE);
                        return FALSE;
                    }
                    else if(isChannel(params) && params.after(' ').empty())
                    {
                        appendIrcStyledText(_("/op <channel> <nicks>, gives operator status for one or more nicks."), 4, FXSystem::now(), FALSE, FALSE);
                        return FALSE;
                    }
                    else if(isChannel(params) && !params.after(' ').empty())
                    {
                        FXString channel = params.before(' ');
                        FXString nicks = params.after(' ');
                        FXString modeparams = utils::instance().createModes('+', 'o', nicks);
                        return m_server->sendMode(channel+" "+modeparams);
                    }
                    else
                    {
                        FXString channel = getText();
                        FXString nicks = params;
                        FXString modeparams = utils::instance().createModes('+', 'o', nicks);
                        return m_server->sendMode(channel+" "+modeparams);
                    }
                }
                else
                {
                    if(isChannel(params) && !params.after(' ').empty())
                    {
                        FXString channel = params.before(' ');
                        FXString nicks = params.after(' ');
                        FXString modeparams = utils::instance().createModes('+', 'o', nicks);
                        return m_server->sendMode(channel+" "+modeparams);
                    }
                    else
                    {
                        appendIrcStyledText(_("/op <channel> <nicks>, gives operator status for one or more nicks."), 4, FXSystem::now(), FALSE, FALSE);
                        return FALSE;
                    }
                }
            }
            else
            {
                appendIrcStyledText(_("You aren't connected"), 4, FXSystem::now(), FALSE, FALSE);
                m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "oper")
        {
            if(m_server->getConnected())
            {
                FXString params = commandtext.after(' ');
                FXString login = params.before(' ');
                FXString password = params.after(' ');
                if(!login.empty() && !password.empty()) return m_server->sendOper(login, password);
                else
                {
                    appendIrcStyledText(_("/oper <login> <password>, oper up."), 4, FXSystem::now(), FALSE, FALSE);
                    return FALSE;
                }
            }
            else
            {
                appendIrcStyledText(_("You aren't connected"), 4, FXSystem::now(), FALSE, FALSE);
                m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "part")
        {
            if(m_server->getConnected())
            {
                if(m_type == CHANNEL)
                {
                    if(commandtext.after(' ').empty()) return m_server->sendPart(getText());
                    else return m_server->sendPart(getText(), commandtext.after(' '));
                }
                else
                {
                    if(isChannel(commandtext.after(' ')))
                    {
                        FXString channel = commandtext.after(' ').before(' ');
                        FXString reason = commandtext.after(' ', 2);
                        if(reason.empty()) return m_server->sendPart(channel);
                        else return m_server->sendPart(channel, reason);
                    }
                    else
                    {
                        appendIrcStyledText(_("/part <channel> [reason], leaves channel."), 4, FXSystem::now(), FALSE, FALSE);
                        return FALSE;
                    }
                }
            }
            else
            {
                appendIrcStyledText(_("You aren't connected"), 4, FXSystem::now(), FALSE, FALSE);
                m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "query")
        {
            if(m_server->getConnected())
            {
                if(commandtext.after(' ').empty())
                {
                    appendIrcStyledText(_("/query <nick>, opens query with nick."), 4, FXSystem::now(), FALSE, FALSE);
                    return FALSE;
                }
                else
                {
                    IrcEvent ev;
                    ev.eventType = IRC_QUERY;
                    ev.param1 = commandtext.after(' ').before(' ');
                    ev.param2 = getNickName();
                    m_parent->getParent()->getParent()->handle(m_server, FXSEL(SEL_COMMAND, IrcSocket::ID_SERVER), &ev);
                    return TRUE;
                }
            }
            else
            {
                appendIrcStyledText(_("You aren't connected"), 4, FXSystem::now(), FALSE, FALSE);
                m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "quit")
        {
            m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CQUIT), NULL);
            return TRUE;
        }
        if(command == "quote")
        {
            if(m_server->getConnected()) return m_server->sendQuote(commandtext.after(' '));
            else
            {
                appendIrcStyledText(_("You aren't connected"), 4, FXSystem::now(), FALSE, FALSE);
                m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "say")
        {
            if(m_server->getConnected())
            {
                if (m_type != SERVER && !commandtext.after(' ').empty())
                {
                    if(m_coloredNick) appendIrcNickText(getNickName(), commandtext.after(' '), getNickColor(getNickName()), FXSystem::now());
                    else appendIrcNickText(getNickName(), commandtext.after(' '), 5, FXSystem::now());
                    IrcEvent ev;
                    ev.eventType = IRC_PRIVMSG;
                    ev.param1 = getNickName();
                    ev.param2 = getText();
                    ev.param3 = commandtext.after(' ');
                    m_parent->getParent()->getParent()->handle(m_server, FXSEL(SEL_COMMAND, IrcSocket::ID_SERVER), &ev);
                    if(commandtext.after(' ').length() > m_maxLen-10-getText().length())
                    {
                        dxStringArray messages = cutText(commandtext.after(' '), m_maxLen-10-getText().length());
                        FXbool result = TRUE;
                        for(FXint i=0; i<messages.no(); i++)
                        {
                            result = m_server->sendMsg(getText(), messages[i]) &result;
                        }
                        return result;
                    }
                    else return m_server->sendMsg(getText(), commandtext.after(' '));
                }
                return FALSE;
            }
            else
            {
                appendIrcStyledText(_("You aren't connected"), 4, FXSystem::now(), FALSE, FALSE);
                m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "stats")
        {
            if(m_server->getConnected())
                return m_server->sendStats(commandtext.after(' '));
            else
            {
                appendIrcStyledText(_("You aren't connected"), 4, FXSystem::now(), FALSE, FALSE);
                m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "tetris")
        {
            m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_NEWTETRIS), NULL);
            return TRUE;
        }
        if(command == "time")
        {
            if(m_server->getConnected()) return m_server->sendQuote("TIME");
            else
            {
                appendIrcStyledText(_("You aren't connected"), 4, FXSystem::now(), FALSE, FALSE);
                m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "topic")
        {
            if(m_server->getConnected())
            {
                FXString params = commandtext.after(' ');
                if(m_type == CHANNEL)
                {
                    if(params.empty()) return m_server->sendTopic(getText());
                    else if(isChannel(params) && !params.after(' ').empty())
                    {
                        FXString channel = params.before(' ');
                        FXString topic = params.after(' ');
                        if(topic.length() > m_server->getTopicLen())
                        {
                            appendIrcStyledText(FXStringFormat(_("Warning: topic is too long. Max. topic length is %d."), m_server->getTopicLen()), 4, FXSystem::now(), FALSE, FALSE);
                            return m_server->sendTopic(channel, topic);
                        }
                        else return m_server->sendTopic(channel, topic);
                    }
                    else
                    {
                        if(params.length() > m_server->getTopicLen())
                        {
                            appendIrcStyledText(FXStringFormat(_("Warning: topic is too long. Max. topic length is %d."), m_server->getTopicLen()), 4, FXSystem::now(), FALSE, FALSE);
                            return m_server->sendTopic(getText(), params);
                        }
                        else return m_server->sendTopic(getText(), params);
                    }
                }
                else
                {
                    if(isChannel(params))
                    {
                        FXString channel = params.before(' ');
                        FXString topic = params.after(' ');
                        if(topic.length() > m_server->getTopicLen())
                        {
                            appendIrcStyledText(FXStringFormat(_("Warning: topic is too long. Max. topic length is %d."), m_server->getTopicLen()), 4, FXSystem::now(), FALSE, FALSE);
                            return m_server->sendTopic(channel, params);
                        }
                        else return m_server->sendTopic(channel, topic);
                    }
                    else
                    {
                        appendIrcStyledText(_("/topic <channel> [topic], views or changes channel topic."), 4, FXSystem::now(), FALSE, FALSE);
                        return FALSE;
                    }
                }
            }
            else
            {
                appendIrcStyledText(_("You aren't connected"), 4, FXSystem::now(), FALSE, FALSE);
                m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "voice")
        {
            if(m_server->getConnected())
            {
                FXString params = commandtext.after(' ');
                if(m_type == CHANNEL)
                {
                    if(params.empty())
                    {
                        appendIrcStyledText(_("/voice <nicks>, gives voice for one or more nicks."), 4, FXSystem::now(), FALSE, FALSE);
                        return FALSE;
                    }
                    else if(isChannel(params) && params.after(' ').empty())
                    {
                        appendIrcStyledText(_("/voice <channel> <nicks>, gives voice for one or more nicks."), 4, FXSystem::now(), FALSE, FALSE);
                        return FALSE;
                    }
                    else if(isChannel(params) && !params.after(' ').empty())
                    {
                        FXString channel = params.before(' ');
                        FXString nicks = params.after(' ');
                        FXString modeparams = utils::instance().createModes('+', 'v', nicks);
                        return m_server->sendMode(channel+" "+modeparams);
                    }
                    else
                    {
                        FXString channel = getText();
                        FXString nicks = params;
                        FXString modeparams = utils::instance().createModes('+', 'v', nicks);
                        return m_server->sendMode(channel+" "+modeparams);
                    }
                }
                else
                {
                    if(isChannel(params) && !params.after(' ').empty())
                    {
                        FXString channel = params.before(' ');
                        FXString nicks = params.after(' ');
                        FXString modeparams = utils::instance().createModes('+', 'v', nicks);
                        return m_server->sendMode(channel+" "+modeparams);
                    }
                    else
                    {
                        appendIrcStyledText(_("/voice <channel> <nicks>, gives voice for one or more nicks."), 4, FXSystem::now(), FALSE, FALSE);
                        return FALSE;
                    }
                }
            }
            else
            {
                appendIrcStyledText(_("You aren't connected"), 4, FXSystem::now(), FALSE, FALSE);
                m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "wallops")
        {
            if(m_server->getConnected())
            {
                FXString params = commandtext.after(' ');
                if(params.empty())
                {
                    appendIrcStyledText(_("/wallops <message>, sends wallop message."), 4, FXSystem::now(), FALSE, FALSE);
                    return FALSE;
                }
                else
                {
                    if(params.length() > m_maxLen-9)
                    {
                        dxStringArray messages = cutText(params, m_maxLen-9);
                        FXbool result = TRUE;
                        for(FXint i=0; i<messages.no(); i++)
                        {
                            result = m_server->sendWallops(messages[i]) &result;
                        }
                        return result;
                    }
                    else return m_server->sendWallops(params);
                }
            }
            else
            {
                appendIrcStyledText(_("You aren't connected"), 4, FXSystem::now(), FALSE, FALSE);
                m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "who")
        {
            if(m_server->getConnected())
            {
                FXString params = commandtext.after(' ');
                if(params.empty())
                {
                    appendIrcStyledText(_("/who <mask> [o], searchs for mask on network, if o is supplied, only search for opers."), 4, FXSystem::now(), FALSE, FALSE);
                    return FALSE;
                }
                else return m_server->sendWho(params);
            }
            else
            {
                appendIrcStyledText(_("You aren't connected"), 4, FXSystem::now(), FALSE, FALSE);
                m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "whoami")
        {
            if(m_server->getConnected()) return m_server->sendWhoami();
            else
            {
                appendIrcStyledText(_("You aren't connected"), 4, FXSystem::now(), FALSE, FALSE);
                m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "whois")
        {
            if(m_server->getConnected())
            {
                FXString params = commandtext.after(' ');
                if(params.empty())
                {
                    appendIrcStyledText(_("/whois <nick>, whois nick."), 4, FXSystem::now(), FALSE, FALSE);
                    return FALSE;
                }
                else return m_server->sendWhois(params);
            }
            else
            {
                appendIrcStyledText(_("You aren't connected"), 4, FXSystem::now(), FALSE, FALSE);
                m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "whowas")
        {
            if(m_server->getConnected())
            {
                FXString params = commandtext.after(' ');
                if(params.empty())
                {
                    appendIrcStyledText(_("/whowas <nick>, whowas nick."), 4, FXSystem::now(), FALSE, FALSE);
                    return FALSE;
                }
                else return m_server->sendWhowas(params);
            }
            else
            {
                appendIrcStyledText(_("You aren't connected"), 4, FXSystem::now(), FALSE, FALSE);
                m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        appendIrcStyledText(FXStringFormat(_("Unknown command '%s', type /commands for available commands"), command.text()), 4, FXSystem::now(), FALSE, FALSE);
        return FALSE;
    }
    else
    {
        if (command.empty() && m_type != SERVER && !commandtext.empty() && m_server->getConnected())
        {
            if(m_coloredNick) appendIrcNickText(getNickName(), commandtext, getNickColor(getNickName()), FXSystem::now());
            else appendIrcNickText(getNickName(), commandtext, 5, FXSystem::now());
            IrcEvent ev;
            ev.eventType = IRC_PRIVMSG;
            ev.param1 = getNickName();
            ev.param2 = getText();
            ev.param3 = commandtext;
            m_parent->getParent()->getParent()->handle(m_server, FXSEL(SEL_COMMAND, IrcSocket::ID_SERVER), &ev);
            if(commandtext.length() > m_maxLen-10-getText().length())
            {
                dxStringArray messages = cutText(commandtext, m_maxLen-10-getText().length());
                FXbool result = TRUE;
                for(FXint i=0; i<messages.no(); i++)
                {
                    result = m_server->sendMsg(getText(), messages[i]) &result;
                }
                return result;
            }
            else return m_server->sendMsg(getText(), commandtext);
        }
        if(!m_server->getConnected())
        {
            appendIrcStyledText(_("You aren't connected"), 4, FXSystem::now(), FALSE, FALSE);
            m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
            return TRUE;
        }
        return FALSE;
    }
    return FALSE;
}

FXbool IrcTabItem::showHelp(FXString command)
{
    if(utils::instance().isScriptCommand(command))
    {
        appendIrcStyledText(utils::instance().getHelpText(command), 3, FXSystem::now(), FALSE, FALSE);
        return TRUE;
    }
    if(command == "admin")
    {
        appendIrcStyledText(_("ADMIN [server], finds information about administrator for current server or [server]."), 3, FXSystem::now(), FALSE, FALSE);
        return TRUE;
    }
    if(command == "away")
    {
        appendIrcStyledText(_("AWAY [message], sets away status."), 3, FXSystem::now(), FALSE, FALSE);
        return TRUE;
    }
    if(command == "banlist")
    {
        appendIrcStyledText(_("BANLIST <channel>, shows banlist for channel."), 3, FXSystem::now(), FALSE, FALSE);
        return TRUE;
    }
    if(command == "commands")
    {
        appendIrcStyledText(_("COMMANDS, shows available commands"), 3, FXSystem::now(), FALSE, FALSE);
        return TRUE;
    }
    if(command == "connect")
    {
        appendIrcStyledText(_("CONNECT <server> [port] [nick] [password] [realname] [channels], connects for given server."), 3, FXSystem::now(), FALSE, FALSE);
        return TRUE;
    }
    if(command == "ctcp")
    {
        appendIrcStyledText(_("CTCP <nick> <message>, sends a CTCP message to a user."), 3, FXSystem::now(), FALSE, FALSE);
        return TRUE;
    }
    if(command == "cycle")
    {
        appendIrcStyledText(_("CYCLE <channel> [message], leaves and join channel."), 3, FXSystem::now(), FALSE, FALSE);
        return TRUE;
    }
    if(command == "dcc")
    {
        appendIrcStyledText(_("DCC chat <nick>, starts DCC chat."), 3, FXSystem::now(), FALSE, FALSE);
        appendIrcStyledText(_("DCC send <nick> <filename>, sends file over DCC."), 3, FXSystem::now(), FALSE, FALSE);
        appendIrcStyledText(_("DCC psend <nick> <filename>, sends file passive over DCC."), 3, FXSystem::now(), FALSE, FALSE);
        appendIrcStyledText(_("More information about passive DCC on http://en.wikipedia.org/wiki/Direct_Client-to-Client#Passive_DCC"), 3, FXSystem::now(), FALSE, FALSE);
        return TRUE;
    }
    if(command == "deop")
    {
        appendIrcStyledText(_("DEOP <channel> <nicks>, removes operator status from one or more nicks."), 3, FXSystem::now(), FALSE, FALSE);
        return TRUE;
    }
    if(command == "devoice")
    {
        appendIrcStyledText(_("DEVOICE <channel> <nicks>, removes voice from one or more nicks."), 3, FXSystem::now(), FALSE, FALSE);
        return TRUE;
    }
    if(command == "disconnect")
    {
        appendIrcStyledText(_("DISCONNECT [reason], leaves server."), 3, FXSystem::now(), FALSE, FALSE);
        return TRUE;
    }
#ifndef WIN32
    if(command == "exec")
    {
        appendIrcStyledText(_("EXEC [-o|-c] <command>, executes command, -o sends output to channel/query, -c closes running command."), 3, FXSystem::now(), FALSE, FALSE);
        return TRUE;
    }
#endif
    if(command == "help")
    {
        appendIrcStyledText(_("HELP <command>, shows help for command."), 3, FXSystem::now(), FALSE, FALSE);
        return TRUE;
    }
    if(command == "ignore")
    {
        appendIrcStyledText(_("IGNORE list, shows list ignored commands and users."), 3, FXSystem::now(), FALSE, FALSE);
        appendIrcStyledText(_("IGNORE addcmd <command>, adds command to ignored commands."), 3, FXSystem::now(), FALSE, FALSE);
        appendIrcStyledText(_("IGNORE rmcmd <command>, removes command from ignored commands."), 3, FXSystem::now(), FALSE, FALSE);
        appendIrcStyledText(_("IGNORE addusr <user> [channel] [server], adds user to ignored users."), 3, FXSystem::now(), FALSE, FALSE);
        appendIrcStyledText(_("IGNORE rmusr <user>, removes user from ignored users."), 3, FXSystem::now(), FALSE, FALSE);
        return TRUE;
    }
    if(command == "invite")
    {
        appendIrcStyledText(_("INVITE <nick> <channel>, invites someone to a channel."), 3, FXSystem::now(), FALSE, FALSE);
        return TRUE;
    }
    if(command == "join")
    {
        appendIrcStyledText(_("JOIN <channel>, joins a channel."), 3, FXSystem::now(), FALSE, FALSE);
        return TRUE;
    }
    if(command == "kick")
    {
        appendIrcStyledText(_("KICK <channel> <nick>, kicks a user from a channel."), 3, FXSystem::now(), FALSE, FALSE);
        return TRUE;
    }
    if(command == "kill")
    {
        appendIrcStyledText(_("KILL <user> [reason], kills a user from the network."), 3, FXSystem::now(), FALSE, FALSE);
        return TRUE;
    }
    if(command == "list")
    {
        appendIrcStyledText(_("LIST [channel], lists channels and their topics."), 3, FXSystem::now(), FALSE, FALSE);
        return TRUE;
    }
#ifdef HAVE_LUA
    if(command == "lua")
    {
        appendIrcStyledText(_("LUA help, shows help for lua scripting."), 3, FXSystem::now(), FALSE, FALSE);
        appendIrcStyledText(_("LUA load <path>, loads script."), 3, FXSystem::now(), FALSE, FALSE);
        appendIrcStyledText(_("Example: /lua load /home/dvx/test.lua"), 3, FXSystem::now(), FALSE, FALSE);
        appendIrcStyledText(_("LUA unload <name>, unloads script."), 3, FXSystem::now(), FALSE, FALSE);
        appendIrcStyledText(_("Example: /lua unload test"), 3, FXSystem::now(), FALSE, FALSE);
        appendIrcStyledText(_("LUA list, shows list of loaded scripts"), 3, FXSystem::now(), FALSE, FALSE);
        return TRUE;
    }
#else
    if(command == "lua")
    {
        appendIrcStyledText(_("dxirc is compiled without support for Lua scripting"), 4, FXSystem::now(), FALSE, FALSE);
        return TRUE;
    }
#endif
    if(command == "me")
    {
        appendIrcStyledText(_("ME <to> <message>, sends the action."), 3, FXSystem::now(), FALSE, FALSE);
        return TRUE;
    }
    if(command == "mode")
    {
        appendIrcStyledText(_("MODE <channel> <modes>, sets modes for a channel."), 3, FXSystem::now(), FALSE, FALSE);
        return TRUE;
    }
    if(command == "msg")
    {
        appendIrcStyledText(_("MSG <nick/channel> <message>, sends a normal message."), 3, FXSystem::now(), FALSE, FALSE);
        return TRUE;
    }
    if(command == "names")
    {
        appendIrcStyledText(_("NAMES <channel>, for nicks on a channel."), 3, FXSystem::now(), FALSE, FALSE);
        return TRUE;
    }
    if(command == "nick")
    {
        appendIrcStyledText(_("NICK <nick>, changes nick."), 3, FXSystem::now(), FALSE, FALSE);
        return TRUE;
    }
    if(command == "notice")
    {
        appendIrcStyledText(_("NOTICE <nick/channel> <message>, sends a notice."), 3, FXSystem::now(), FALSE, FALSE);
        return TRUE;
    }
    if(command == "op")
    {
        appendIrcStyledText(_("OP <channel> <nicks>, gives operator status for one or more nicks."), 3, FXSystem::now(), FALSE, FALSE);
        return TRUE;
    }
    if(command == "oper")
    {
        appendIrcStyledText(_("OPER <login> <password>, oper up."), 3, FXSystem::now(), FALSE, FALSE);
        return TRUE;
    }
    if(command == "part")
    {
        appendIrcStyledText(_("PART <channel> [reason], leaves channel."), 3, FXSystem::now(), FALSE, FALSE);
        return TRUE;
    }
    if(command == "query")
    {
        appendIrcStyledText(_("QUERY <nick>, opens query with nick."), 3, FXSystem::now(), FALSE, FALSE);
        return TRUE;
    }
    if(command == "quit")
    {
        appendIrcStyledText(_("QUIT, closes application."), 3, FXSystem::now(), FALSE, FALSE);
        return TRUE;
    }
    if(command == "quote")
    {
        appendIrcStyledText(_("QUOTE [text], sends text to server."), 3, FXSystem::now(), FALSE, FALSE);
        return TRUE;
    }
    if(command == "say")
    {
        appendIrcStyledText(_("SAY [text], sends text to current tab."), 3, FXSystem::now(), FALSE, FALSE);
        return TRUE;
    }
    if(command == "stats")
    {
        appendIrcStyledText(_("STATS <type>, shows some irc server usage statistics. Available types vary slightly per server; some common ones are:"), 3, FXSystem::now(), FALSE, FALSE);
        appendIrcStyledText(_("c - shows C and N lines for a given server.  These are the names of the servers that are allowed to connect."), 3, FXSystem::now(), FALSE, FALSE);
        appendIrcStyledText(_("h - shows H and L lines for a given server (Hubs and Leaves)."), 3, FXSystem::now(), FALSE, FALSE);
        appendIrcStyledText(_("k - show K lines for a server.  This shows who is not allowed to connect and possibly at what time they are not allowed to connect."), 3, FXSystem::now(), FALSE, FALSE);
        appendIrcStyledText(_("i - shows I lines. This is who CAN connect to a server."), 3, FXSystem::now(), FALSE, FALSE);
        appendIrcStyledText(_("l - shows information about amount of information passed to servers and users."), 3, FXSystem::now(), FALSE, FALSE);
        appendIrcStyledText(_("m - shows a count for the number of times the various commands have been used since the server was booted."), 3, FXSystem::now(), FALSE, FALSE);
        appendIrcStyledText(_("o - shows the list of authorized operators on the server."), 3, FXSystem::now(), FALSE, FALSE);
        appendIrcStyledText(_("p - shows online operators and their idle times."), 3, FXSystem::now(), FALSE, FALSE);
        appendIrcStyledText(_("u - shows the uptime for a server."), 3, FXSystem::now(), FALSE, FALSE);
        appendIrcStyledText(_("y - shows Y lines, which lists the various connection classes for a given server."), 3, FXSystem::now(), FALSE, FALSE);

        return TRUE;
    }
    if(command == "tetris")
    {
        appendIrcStyledText(_("TETRIS, start small easteregg."), 3, FXSystem::now(), FALSE, FALSE);
        appendIrcStyledText(_("Keys for playing:"), 3, FXSystem::now(), FALSE, FALSE);
        appendIrcStyledText(_("n .. new game"), 3, FXSystem::now(), FALSE, FALSE);
        appendIrcStyledText(_("p .. pause game"), 3, FXSystem::now(), FALSE, FALSE);
        appendIrcStyledText(_("i .. rotate piece"), 3, FXSystem::now(), FALSE, FALSE);
        appendIrcStyledText(_("l .. move piece right"), 3, FXSystem::now(), FALSE, FALSE);
        appendIrcStyledText(_("k .. drop piece"), 3, FXSystem::now(), FALSE, FALSE);
        appendIrcStyledText(_("j .. move piece left"), 3, FXSystem::now(), FALSE, FALSE);
        return TRUE;
    }
    if(command == "time")
    {
        appendIrcStyledText(_("TIME, displays the time of day, local to server."), 3, FXSystem::now(), FALSE, FALSE);
        return TRUE;
    }
    if(command == "topic")
    {
        appendIrcStyledText(_("TOPIC [topic], sets or shows topic."), 3, FXSystem::now(), FALSE, FALSE);
        return TRUE;
    }
    if(command == "voice")
    {
        appendIrcStyledText(_("VOICE <channel> <nicks>, gives voice for one or more nicks."), 3, FXSystem::now(), FALSE, FALSE);
        return TRUE;
    }
    if(command == "wallops")
    {
        appendIrcStyledText(_("WALLOPS <message>, sends wallop message."), 3, FXSystem::now(), FALSE, FALSE);
        return TRUE;
    }
    if(command == "who")
    {
        appendIrcStyledText(_("WHO <mask> [o], searchs for mask on network, if o is supplied, only search for opers."), 3, FXSystem::now(), FALSE, FALSE);
        return TRUE;
    }
    if(command == "whoami")
    {
        appendIrcStyledText(_("WHOAMI, whois about you."), 3, FXSystem::now(), FALSE, FALSE);
        return TRUE;
    }
    if(command == "whois")
    {
        appendIrcStyledText(_("WHOIS <nick>, whois nick."), 3, FXSystem::now(), FALSE, FALSE);
        return TRUE;
    }
    if(command == "whowas")
    {
        appendIrcStyledText(_("WHOWAS <nick>, whowas nick."), 3, FXSystem::now(), FALSE, FALSE);
        return TRUE;
    }
    if(!utils::instance().getAlias(command[0] == '/' ? command:"/"+command).empty())
    {
        appendIrcStyledText(FXStringFormat("%s: %s", command.upper().text(), utils::instance().getAlias(command[0] == '/' ? command:"/"+command).text()), 3, FXSystem::now(), FALSE, FALSE);
        return TRUE;
    }
    if(command.empty()) appendIrcStyledText(_("Command is empty, type /commands for available commands"), 4, FXSystem::now(), FALSE, FALSE);
    else appendIrcStyledText(FXStringFormat(_("Unknown command '%s', type /commands for available commands"), command.text()), 4, FXSystem::now(), FALSE, FALSE);
    return FALSE;
}

long IrcTabItem::onKeyPress(FXObject *, FXSelector, void *ptr)
{
    if (m_commandline->hasFocus())
    {
        FXEvent* event = (FXEvent*)ptr;
        FXString line = m_commandline->getText();
        switch(event->code){
            case KEY_Tab:
                if(event->state&CONTROLMASK)
                {
                    m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_NEXTTAB), NULL);
                    return 1;
                }
                if(line[0] == '/' && line.after(' ').empty())
                {
                    for(FXint i = 0; i < utils::instance().commandsNo(); i++)
                    {
                        if(comparecase(line.after('/').before(' '), utils::instance().commandsAt(i)) == 0)
                        {
                            if((i+1) < utils::instance().commandsNo()) m_commandline->setText("/"+utils::instance().commandsAt(++i)+" ");
                            else m_commandline->setText("/"+utils::instance().commandsAt(0)+" ");
                            break;
                        }
                        else if(comparecase(line.after('/'), utils::instance().commandsAt(i).left(line.after('/').length())) == 0)
                        {
                            m_commandline->setText("/"+utils::instance().commandsAt(i)+" ");
                            break;
                        }
                    }
                    return 1;
                }
                if(line[0] != '/' && line.after(' ').empty())
                {
                    if(line.empty())
                    {
                        m_commandline->setText(getNick(0)+m_nickCompletionChar+" ");
                        return 1;
                    }
                    for(FXint j = 0; j < m_users->getNumItems() ; j++)
                    {
                        if(comparecase(line, getNick(j).left(line.length())) == 0)
                        {
                            m_commandline->setText(getNick(j)+m_nickCompletionChar+" ");
                        }
                        else if(comparecase(line.section(m_nickCompletionChar, 0, 1), getNick(j)) == 0)
                        {
                            if((j+1) < m_users->getNumItems()) m_commandline->setText(getNick(++j)+m_nickCompletionChar+" ");
                            else m_commandline->setText(getNick(0)+m_nickCompletionChar+" ");
                        }
                    }
                    return 1;
                }
                if(line.find(' ') != -1)
                {
                    FXint curpos;
                    line[m_commandline->getCursorPos()] == ' ' ? curpos = m_commandline->getCursorPos()-1 : curpos = m_commandline->getCursorPos();
                    FXint pos = line.rfind(' ', curpos)+1;
                    FXint n = line.find(' ', curpos)>0 ? line.find(' ', curpos)-pos : line.length()-pos;
                    FXString toCompletion = line.mid(pos, n);
                    for(FXint j = 0; j < m_users->getNumItems(); j++)
                    {
                        if(comparecase(toCompletion, getNick(j)) == 0)
                        {
                            if((j+1) < m_users->getNumItems())
                            {
                                m_commandline->setText(line.replace(pos, n, getNick(j+1)));
                                m_commandline->setCursorPos(pos+getNick(j+1).length());
                            }
                            else
                            {
                                m_commandline->setText(line.replace(pos, n, getNick(0)));
                                m_commandline->setCursorPos(pos+getNick(0).length());
                            }
                            break;
                        }
                        else if(comparecase(toCompletion, getNick(j).left(toCompletion.length())) == 0)
                        {
                            m_commandline->setText(line.replace(pos, n, getNick(j)));
                            m_commandline->setCursorPos(pos+getNick(j).length());
                            break;
                        }
                    }
                    return 1;
                }
                return 1;
            case KEY_Up:
                if(m_currentPosition!=-1 && m_currentPosition<m_commandsHistory.no())
                {
                    if(!line.empty() && line!=m_commandsHistory[m_currentPosition])
                    {
                        m_commandsHistory.append(line);
                        if(m_commandsHistory.no() > m_historyMax)
                            m_commandsHistory.erase(0);
                        m_currentPosition = m_commandsHistory.no()-1;
                    }
                    if(m_currentPosition > 0 && !line.empty())
                        --m_currentPosition;
                    m_commandline->setText(m_commandsHistory[m_currentPosition]);
                }
                return 1;
            case KEY_Down:
                if(m_currentPosition!=-1 && m_currentPosition<m_commandsHistory.no())
                {
                    if(!line.empty() && line!=m_commandsHistory[m_currentPosition])
                    {
                        m_commandsHistory.append(line);
                        if(m_commandsHistory.no() > m_historyMax)
                            m_commandsHistory.erase(0);
                        m_currentPosition = m_commandsHistory.no()-1;
                    }
                    if(m_currentPosition < m_commandsHistory.no()-1)
                    {
                        ++m_currentPosition;
                        m_commandline->setText(m_commandsHistory[m_currentPosition]);
                    }
                    else
                        m_commandline->setText("");
                }
                return 1;
            case KEY_k:
            case KEY_K:
                if(event->state&CONTROLMASK)
                {
                    FXint pos = m_commandline->getCursorPos();
                    m_commandline->setText(line.insert(pos, "^C")); //color
                    m_commandline->setCursorPos(pos+2);
                    return 1;
                }
            case KEY_b:
            case KEY_B:
                if(event->state&CONTROLMASK)
                {
                    FXint pos = m_commandline->getCursorPos();
                    m_commandline->setText(line.insert(pos, "^B")); //bold
                    m_commandline->setCursorPos(pos+2);
                    return 1;
                }
            case KEY_i:
            case KEY_I:
                if(event->state&CONTROLMASK)
                {
                    FXint pos = m_commandline->getCursorPos();
                    m_commandline->setText(line.insert(pos, "^_")); //underline
                    m_commandline->setCursorPos(pos+2);
                    return 1;
                }
            case KEY_r:
            case KEY_R:
                if(event->state&CONTROLMASK)
                {
                    FXint pos = m_commandline->getCursorPos();
                    m_commandline->setText(line.insert(pos, "^V")); //reverse
                    m_commandline->setCursorPos(pos+2);
                    return 1;
                }
            case KEY_o:
            case KEY_O:
                if(event->state&CONTROLMASK)
                {
                    FXint pos = m_commandline->getCursorPos();
                    m_commandline->setText(line.insert(m_commandline->getCursorPos(), "^O")); //reset
                    m_commandline->setCursorPos(pos+2);
                    return 1;
                }
        }
    }
    return 0;
}

//Check is this tab current or first for server
FXbool IrcTabItem::isFirst()
{
    FXint indexOfCurrent = m_parent->getCurrent()*2;
    FXbool hasCurrent = m_server->findTarget(static_cast<IrcTabItem*>(m_parent->childAtIndex(indexOfCurrent)));
    FXint indexOfThis = m_parent->indexOfChild(this);
    if(hasCurrent)
    {
        if(indexOfCurrent == indexOfThis) return TRUE;
        else return FALSE;
    }
    else
    {
        for(FXint i = 0; i<m_parent->numChildren(); i+=2)
        {
            if(m_server->findTarget(static_cast<IrcTabItem*>(m_parent->childAtIndex(i))))
            {
                if(i == indexOfThis) return TRUE;
                else return FALSE;
            }
        }
    }
    return FALSE;
}

FXbool IrcTabItem::isCommandIgnored(const FXString &command)
{
    if(m_commandsList.contains(command)) return TRUE;
    return FALSE;
}

void IrcTabItem::addUser(const FXString& user)
{
    if(user[0] == m_server->getAdminPrefix()
            && m_users->findItem(user.after(m_server->getAdminPrefix())) == -1)
    {
        m_users->appendItem(user.after(m_server->getAdminPrefix()), ICO_IRCADMIN);
        m_numberUsers++;
        m_users->sortItems();
        return;
    }
    if(user[0] == m_server->getOwnerPrefix()
            && m_users->findItem(user.after(m_server->getOwnerPrefix())) == -1)
    {
        m_users->appendItem(user.after(m_server->getOwnerPrefix()), ICO_IRCOWNER);
        m_numberUsers++;
        m_users->sortItems();
        return;
    }
    if(user[0] == m_server->getOpPrefix()
            && m_users->findItem(user.after(m_server->getOpPrefix())) == -1)
    {
        m_users->appendItem(user.after(m_server->getOpPrefix()), ICO_IRCOP);
        m_numberUsers++;
        m_users->sortItems();
        return;
    }
    if(user[0] == m_server->getVoicePrefix()
            && m_users->findItem(user.after(m_server->getVoicePrefix())) == -1)
    {
        m_users->appendItem(user.after(m_server->getVoicePrefix()), ICO_IRCVOICE);
        m_numberUsers++;
        m_users->sortItems();
        return;
    }
    if(user[0] == m_server->getHalfopPrefix()
            && m_users->findItem(user.after(m_server->getHalfopPrefix())) == -1)
    {
        m_users->appendItem(user.after(m_server->getHalfopPrefix()), ICO_IRCHALFOP);
        m_numberUsers++;
        m_users->sortItems();
        return;
    }
    if(user[0] == m_server->getAdminPrefix()
            || user[0] == m_server->getOwnerPrefix()
            || user[0] == m_server->getOpPrefix()
            || user[0] == m_server->getVoicePrefix()
            || user[0] == m_server->getHalfopPrefix())
        return;
    if(m_users->findItem(user) == -1)
    {
        m_users->appendItem(user, ICO_IRCNORMAL);
        m_numberUsers++;
        m_users->sortItems();
        return;
    }
}

void IrcTabItem::removeUser(const FXString& user)
{
    if(m_users->findItem(user) != -1)
    {
        m_users->removeItem(m_users->findItem(user));
    }
    m_numberUsers--;
    m_users->sortItems();
}

void IrcTabItem::changeNickUser(const FXString& nick, const FXString& newnick)
{
    FXint i = m_users->findItem(nick);
    if(i != -1)
    {
        if(m_users->getItemIcon(i) == ICO_IRCOWNER || m_users->getItemIcon(i) == ICO_IRCAWAYOWNER)
        {
            removeUser(nick);
            addUser(m_server->getOwnerPrefix()+newnick);
        }
        else if(m_users->getItemIcon(i) == ICO_IRCADMIN || m_users->getItemIcon(i) == ICO_IRCAWAYADMIN)
        {
            removeUser(nick);
            addUser(m_server->getAdminPrefix()+newnick);
        }
        else if(m_users->getItemIcon(i) == ICO_IRCOP || m_users->getItemIcon(i) == ICO_IRCAWAYOP)
        {
            removeUser(nick);
            addUser(m_server->getOpPrefix()+newnick);
        }
        else if(m_users->getItemIcon(i) == ICO_IRCVOICE || m_users->getItemIcon(i) == ICO_IRCAWAYVOICE)
        {
            removeUser(nick);
            addUser(m_server->getVoicePrefix()+newnick);
        }
        else if(m_users->getItemIcon(i) == ICO_IRCHALFOP || m_users->getItemIcon(i) == ICO_IRCAWAYHALFOP)
        {
            removeUser(nick);
            addUser(m_server->getHalfopPrefix()+newnick);
        }
        else
        {
            removeUser(nick);
            addUser(newnick);
        }
    }
}

long IrcTabItem::onIrcEvent(FXObject *, FXSelector, void *data)
{
    IrcEvent *ev = (IrcEvent *) data;
    if(ev->eventType == IRC_PRIVMSG)
    {
        onIrcPrivmsg(ev);
        return 1;
    }
    if(ev->eventType == IRC_ACTION)
    {
        onIrcAction(ev);
        return 1;
    }
    if(ev->eventType == IRC_CTCPREPLY)
    {
        onIrcCtpcReply(ev);
        return 1;
    }
    if(ev->eventType == IRC_CTCPREQUEST)
    {
        onIrcCtcpRequest(ev);
        return 1;
    }
    if(ev->eventType == IRC_DCCMSG && m_type == DCCCHAT)
    {
        onIrcDccMsg(ev);
        return 1;
    }
    if(ev->eventType == IRC_DCCACTION && m_type == DCCCHAT)
    {
        onIrcDccAction(ev);
        return 1;
    }
    if(ev->eventType == IRC_JOIN)
    {
        onIrcJoin(ev);
        return 1;
    }
    if(ev->eventType == IRC_QUIT)
    {
        onIrcQuit(ev);
        return 1;
    }
    if(ev->eventType == IRC_PART)
    {
        onIrcPart(ev);
        return 1;
    }
    if(ev->eventType == IRC_CHNOTICE)
    {
        onIrcChnotice(ev);
        return 1;
    }
    if(ev->eventType == IRC_NOTICE)
    {
        onIrcNotice(ev);
        return 1;
    }
    if(ev->eventType == IRC_NICK)
    {
        onIrcNick(ev);
        return 1;
    }
    if(ev->eventType == IRC_TOPIC)
    {
        onIrcTopic(ev);
        return 1;
    }
    if(ev->eventType == IRC_INVITE)
    {
        onIrcInvite(ev);
        return 1;
    }
    if(ev->eventType == IRC_KICK)
    {
        onIrcKick(ev);
        return 1;
    }
    if(ev->eventType == IRC_MODE)
    {
        onIrcMode(ev);
        return 1;
    }
    if(ev->eventType == IRC_UMODE)
    {
        onIrcUmode(ev);
        return 1;
    }
    if(ev->eventType == IRC_CHMODE)
    {
        onIrcChmode(ev);
        return 1;
    }
    if(ev->eventType == IRC_SERVERREPLY)
    {
        onIrcServerReply(ev);
        return 1;
    }
    if(ev->eventType == IRC_CONNECT)
    {
        onIrcConnect(ev);
        return 1;
    }
    if(ev->eventType == IRC_ERROR)
    {
        onIrcError(ev);
        return 1;
    }
    if(ev->eventType == IRC_SERVERERROR)
    {
        onIrcServerError(ev);
        return 1;
    }
    if(ev->eventType == IRC_DISCONNECT)
    {
        onIrcDisconnect(ev);
        return 1;
    }
    if(ev->eventType == IRC_RECONNECT)
    {
        onIrcReconnect(ev);
        return 1;
    }
    if(ev->eventType == IRC_UNKNOWN)
    {
        onIrcUnknown(ev);
        return 1;
    }
    if(ev->eventType == IRC_301)
    {
        onIrc301(ev);
        return 1;
    }
    if(ev->eventType == IRC_305)
    {
        onIrc305(ev);
        return 1;
    }
    if(ev->eventType == IRC_306)
    {
        onIrc306(ev);
        return 1;
    }
    if(ev->eventType == IRC_331 || ev->eventType == IRC_332 || ev->eventType == IRC_333)
    {
        onIrc331332333(ev);
        return 1;
    }
    if(ev->eventType == IRC_353)
    {
        onIrc353(ev);
        return 1;
    }
    if(ev->eventType == IRC_366)
    {
        onIrc366(ev);
        return 1;
    }
    if(ev->eventType == IRC_372)
    {
        onIrc372(ev);
        return 1;
    }
    if(ev->eventType == IRC_AWAY)
    {
        onIrcAway(ev);
        return 1;
    }
    if(ev->eventType == IRC_ENDMOTD)
    {
        onIrcEndMotd();
        return 1;
    }
    return 1;
}

//handle IrcEvent IRC_PRIVMSG
void IrcTabItem::onIrcPrivmsg(IrcEvent* ev)
{
    if((comparecase(ev->param2, getText()) == 0 && m_type == CHANNEL) || (ev->param1 == getText() && m_type == QUERY && ev->param2 == getNickName()))
    {
        FXbool needHighlight = FALSE;
        if(ev->param3.contains(getNickName()))
            needHighlight = highlightNeeded(ev->param3);
        if(m_coloredNick)
        {
            if(needHighlight) appendIrcStyledText(ev->param1+": "+ev->param3, 8, ev->time);
            else appendIrcNickText(ev->param1, ev->param3, getNickColor(ev->param1), ev->time);
        }
        else
        {
            if(needHighlight) appendIrcStyledText(ev->param1+": "+ev->param3, 8, ev->time);
            else appendIrcNickText(ev->param1, ev->param3, 5, ev->time);
        }
        if(m_highlightColor != this->getTextColor() && m_parent->getCurrent()*2 != m_parent->indexOfChild(this))
        {
            if(needHighlight)
            {
                this->setTextColor(m_highlightColor);
                if(m_type == CHANNEL) this->setIcon(ICO_CHANNELNEWMSG);
            }
            else this->setTextColor(m_unreadColor);
            if(m_type == QUERY) this->setIcon(ICO_QUERYNEWMSG);
        }
        if((m_type == CHANNEL && needHighlight) || m_type == QUERY)
            m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_NEWMSG), NULL);
    }
}

//handle IrcEvent IRC_ACTION
void IrcTabItem::onIrcAction(IrcEvent* ev)
{
    if((comparecase(ev->param2, getText()) == 0 && m_type == CHANNEL) || (ev->param1 == getText() && m_type == QUERY && ev->param2 == getNickName()))
    {
        if(!isCommandIgnored("me"))
        {
            FXbool needHighlight = FALSE;
            if(ev->param3.contains(getNickName()))
                needHighlight = highlightNeeded(ev->param3);
            appendIrcStyledText(ev->param1+" "+ev->param3, 2, ev->time);
            if(m_highlightColor != this->getTextColor() && m_parent->getCurrent()*2 != m_parent->indexOfChild(this))
            {
                if(needHighlight)
                {
                    this->setTextColor(m_highlightColor);
                    if(m_type == CHANNEL) this->setIcon(ICO_CHANNELNEWMSG);
                }
                else this->setTextColor(m_unreadColor);
                if(m_type == QUERY) this->setIcon(ICO_QUERYNEWMSG);
            }
            if((m_type == CHANNEL && needHighlight) || m_type == QUERY)
                m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_NEWMSG), NULL);
        }
    }
}

//handle IrcEvent IRC_CTCPREPLY
void IrcTabItem::onIrcCtpcReply(IrcEvent* ev)
{
    if(m_type == SERVER || isFirst())
    {
        if(!isCommandIgnored("ctcp"))
        {
            appendIrcStyledText(FXStringFormat(_("CTCP %s reply from %s: %s"), utils::instance().getParam(ev->param2, 1, FALSE).text(), ev->param1.text(), utils::instance().getParam(ev->param2, 2, TRUE).text()), 2, ev->time);
            if(m_highlightColor != this->getTextColor() && m_parent->getCurrent()*2 != m_parent->indexOfChild(this)) this->setTextColor(m_unreadColor);
        }
    }
}

//handle IrcEvent IRC_CTCPREQUEST
void IrcTabItem::onIrcCtcpRequest(IrcEvent* ev)
{
    if(m_type == SERVER || isFirst())
    {
        if(!isCommandIgnored("ctcp"))
        {
            appendIrcStyledText(FXStringFormat(_("CTCP %s request from %s"), ev->param2.text(), ev->param1.text()), 2, ev->time);
            if(m_highlightColor != this->getTextColor() && m_parent->getCurrent()*2 != m_parent->indexOfChild(this)) this->setTextColor(m_unreadColor);
        }
    }
}

//handle IrcEvent IRC_DCCMSG
void IrcTabItem::onIrcDccMsg(IrcEvent* ev)
{
    FXbool needHighlight = FALSE;
    if(ev->param1.contains(getNickName()))
        needHighlight = highlightNeeded(ev->param1);
    if(m_coloredNick)
    {
        if(needHighlight) appendIrcStyledText(getText()+": "+ev->param1, 8, ev->time);
        else appendIrcNickText(getText(), ev->param1, getNickColor(getText()), ev->time);
    }
    else
    {
        if(needHighlight) appendIrcStyledText(getText()+": "+ev->param1, 8, ev->time);
        else appendIrcNickText(getText(), ev->param1, 5, ev->time);
    }
    if(m_highlightColor != this->getTextColor() && m_parent->getCurrent()*2 != m_parent->indexOfChild(this))
    {
        if(needHighlight)
        {
            this->setTextColor(m_highlightColor);
        }
        else this->setTextColor(m_unreadColor);
        this->setIcon(ICO_DCCNEWMSG);
        m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_NEWMSG), NULL);
    }
}

//handle IrcEvent IRC_DCCACTION
void IrcTabItem::onIrcDccAction(IrcEvent* ev)
{
    appendIrcStyledText(getText()+" "+ev->param1, 2, ev->time);
    if(m_highlightColor != this->getTextColor() && m_parent->getCurrent()*2 != m_parent->indexOfChild(this))
    {
        if(ev->param1.contains(getNickName()) && highlightNeeded(ev->param1))
        {
            this->setTextColor(m_highlightColor);
        }
        else this->setTextColor(m_unreadColor);
        this->setIcon(ICO_DCCNEWMSG);
        m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_NEWMSG), NULL);
    }
}

//handle IrcEvent IRC_JOIN
void IrcTabItem::onIrcJoin(IrcEvent* ev)
{
    if(comparecase(ev->param2, getText()) == 0 && ev->param1 != getNickName())
    {
        if(!isCommandIgnored("join") && !m_server->isUserIgnored(ev->param1, getText())) appendIrcStyledText(FXStringFormat(_("%s has joined to %s"), ev->param1.text(), ev->param2.text()), 1, ev->time);
        addUser(ev->param1);
    }
}

//handle IrcEvent IRC_QUIT
void IrcTabItem::onIrcQuit(IrcEvent* ev)
{
    if(m_type == CHANNEL && m_users->findItem(ev->param1) != -1)
    {
        removeUser(ev->param1);
        if(ev->param2.empty())
        {
            if(!isCommandIgnored("quit") && !m_server->isUserIgnored(ev->param1, getText())) appendIrcStyledText(FXStringFormat(_("%s has quit"), ev->param1.text()), 1, ev->time);
        }
        else
        {
            if(!isCommandIgnored("quit") && !m_server->isUserIgnored(ev->param1, getText()))appendIrcStyledText(FXStringFormat(_("%s has quit (%s)"), ev->param1.text(), +ev->param2.text()), 1, ev->time);
        }
    }
    else if(m_type == QUERY && getText() == ev->param1)
    {
        appendIrcStyledText(FXStringFormat(_("%s has quit"), ev->param1.text()), 1, ev->time, FALSE, FALSE);
    }
}

//handle IrcEvent IRC_PART
void IrcTabItem::onIrcPart(IrcEvent* ev)
{
    if(comparecase(ev->param2, getText()) == 0)
    {
        if(ev->param3.empty() && !isCommandIgnored("part") && !m_server->isUserIgnored(ev->param1, getText())) appendIrcStyledText(FXStringFormat(_("%s has parted %s"), ev->param1.text(), ev->param2.text()), 1, ev->time);
        else if(!isCommandIgnored("part") && !m_server->isUserIgnored(ev->param1, getText())) appendIrcStyledText(FXStringFormat(_("%s has parted %s (%s)"), ev->param1.text(), ev->param2.text(), ev->param3.text()), 1, ev->time);
        removeUser(ev->param1);
    }
}

//handle IrcEvent IRC_CHNOTICE
void IrcTabItem::onIrcChnotice(IrcEvent* ev)
{
    if(!isCommandIgnored("notice"))
    {
        FXbool tabExist = FALSE;
        for(FXint i = 0; i<m_parent->numChildren(); i+=2)
        {
            if(m_parent->childAtIndex(i)->getMetaClass()==&IrcTabItem::metaClass && m_server->findTarget(static_cast<IrcTabItem*>(m_parent->childAtIndex(i))))
            {
                if((comparecase(ev->param2, static_cast<IrcTabItem*>(m_parent->childAtIndex(i))->getText()) == 0 && static_cast<IrcTabItem*>(m_parent->childAtIndex(i))->getType() == CHANNEL) 
                    || (ev->param1 == static_cast<IrcTabItem*>(m_parent->childAtIndex(i))->getText() && static_cast<IrcTabItem*>(m_parent->childAtIndex(i))->getType() == QUERY && ev->param2 == static_cast<IrcTabItem*>(m_parent->childAtIndex(i))->getNickName()))
                {
                    tabExist = TRUE;
                    break;
                }
            }
        }
        if(tabExist)
        {
            if((comparecase(ev->param2, getText()) == 0 && m_type == CHANNEL) || (ev->param1 == getText() && m_type == QUERY && ev->param2 == getNickName()))
            {
                FXbool needHighlight = FALSE;
                if(ev->param3.contains(getNickName()))
                    needHighlight = highlightNeeded(ev->param3);
                appendIrcStyledText(FXStringFormat(_("%s's NOTICE: %s"), ev->param1.text(), ev->param3.text()), 2, ev->time);
                if(m_highlightColor != this->getTextColor() && m_parent->getCurrent()*2 != m_parent->indexOfChild(this))
                {
                    if(needHighlight)
                    {
                        this->setTextColor(m_highlightColor);
                        if(m_type == CHANNEL) this->setIcon(ICO_CHANNELNEWMSG);
                    }
                    else this->setTextColor(m_unreadColor);
                    if(m_type == QUERY) this->setIcon(ICO_QUERYNEWMSG);
                }
                if((m_type == CHANNEL && needHighlight) || m_type == QUERY)
                    m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_NEWMSG), NULL);
            }
        }
        else
        {
            if(m_type == SERVER || isFirst())
            {
                appendIrcStyledText(FXStringFormat(_("%s's NOTICE: %s"), ev->param1.text(), ev->param3.text()), 3, ev->time);
            }
        }
    }
}

//handle IrcEvent IRC_NOTICE
void IrcTabItem::onIrcNotice(IrcEvent* ev)
{
    if(m_type == SERVER || isFirst())
    {
        if(ev->param1 == getNickName() && !isCommandIgnored("notice"))
        {
            appendIrcStyledText(FXStringFormat(_("NOTICE for you: %s"), ev->param2.text()), 3, ev->time);
            if(m_highlightColor != this->getTextColor() && m_parent->getCurrent()*2 != m_parent->indexOfChild(this)) this->setTextColor(m_unreadColor);
        }
        else if(!isCommandIgnored("notice"))
        {
            appendIrcStyledText(FXStringFormat(_("%s's NOTICE: %s"), ev->param1.text(), ev->param2.text()), 3, ev->time);
            if(m_highlightColor != this->getTextColor() && m_parent->getCurrent()*2 != m_parent->indexOfChild(this)) this->setTextColor(m_unreadColor);
        }
    }
}

//handle IrcEvent IRC_NICK
void IrcTabItem::onIrcNick(IrcEvent* ev)
{
    if(m_users->findItem(ev->param1) != -1)
    {
        if(ev->param2 == getNickName() && !isCommandIgnored("nick")) appendIrcStyledText(FXStringFormat(_("You're now known as %s"), ev->param2.text()), 1, ev->time);
        else if(!isCommandIgnored("nick") && !m_server->isUserIgnored(ev->param1, getText())) appendIrcStyledText(FXStringFormat(_("%s is now known as %s"), ev->param1.text(), ev->param2.text()), 1, ev->time);
        changeNickUser(ev->param1, ev->param2);
    }
    if(m_type == QUERY && ev->param1 == getText())
    {
        this->setText(ev->param2);
    }
}

//handle IrcEvent IRC_TOPIC
void IrcTabItem::onIrcTopic(IrcEvent* ev)
{
    if(comparecase(ev->param2, getText()) == 0)
    {
        appendIrcText(FXStringFormat(_("%s set new topic for %s: %s"), ev->param1.text(), ev->param2.text(), ev->param3.text()), ev->time);
        m_topic = stripColors(ev->param3, TRUE);
        m_topicline->setText(m_topic);
        m_topicline->setCursorPos(0);
        m_topicline->makePositionVisible(0);
    }
}

//handle IrcEvent IRC_INVITE
void IrcTabItem::onIrcInvite(IrcEvent* ev)
{
    if(m_type == SERVER || isFirst())
    {
        appendIrcStyledText(FXStringFormat(_("%s invites you to: %s"), ev->param1.text(), ev->param3.text()), 3, ev->time, FALSE, FALSE);
        if(m_highlightColor != this->getTextColor() && m_parent->getCurrent()*2 != m_parent->indexOfChild(this)) this->setTextColor(m_unreadColor);
    }
}

//handle IrcEvent IRC_KICK
void IrcTabItem::onIrcKick(IrcEvent* ev)
{
    if(comparecase(ev->param3, getText()) == 0)
    {
        if(ev->param2 != getNickName())
        {
            if(ev->param4.empty()) appendIrcStyledText(FXStringFormat(_("%s was kicked from %s by %s"), ev->param2.text(), ev->param3.text(), ev->param1.text()), 1, ev->time, FALSE, FALSE);
            else appendIrcStyledText(FXStringFormat(_("%s was kicked from %s by %s (%s)"), ev->param2.text(), ev->param3.text(), ev->param1.text(), ev->param4.text()), 1, ev->time, FALSE, FALSE);
            removeUser(ev->param2);
        }
    }
    if(ev->param2 == getNickName() && (m_type == SERVER || isFirst()))
    {
        if(ev->param4.empty()) appendIrcStyledText(FXStringFormat(_("You were kicked from %s by %s"), ev->param3.text(), ev->param1.text()), 1, ev->time, FALSE, FALSE);
        else appendIrcStyledText(FXStringFormat(_("You were kicked from %s by %s (%s)"), ev->param3.text(), ev->param1.text(), ev->param4.text()), 1, ev->time, FALSE, FALSE);
        if(m_highlightColor != this->getTextColor() && m_parent->getCurrent()*2 != m_parent->indexOfChild(this)) this->setTextColor(m_unreadColor);
    }
}

//handle IrcEvent IRC_MODE
void IrcTabItem::onIrcMode(IrcEvent* ev)
{
    if(m_type == SERVER || isFirst())
    {
        appendIrcStyledText(FXStringFormat(_("Mode change [%s] for %s"), ev->param1.text(), ev->param2.text()), 1, ev->time, FALSE, FALSE);
        if(m_highlightColor != this->getTextColor() && m_parent->getCurrent()*2 != m_parent->indexOfChild(this)) this->setTextColor(m_unreadColor);
    }
}

//handle IrcEvent IRC_UMODE
void IrcTabItem::onIrcUmode(IrcEvent* ev)
{
    FXString moderator = ev->param1;
    FXString channel = ev->param2;
    FXString modes = ev->param3;
    FXString args = ev->param4;
    if(comparecase(channel, getText()) == 0)
    {
        FXbool sign = FALSE;
        int argsiter = 1;
        for(int i =0; i < modes.count(); i++) {
            switch(modes[i]) {
                case '+':
                    sign = TRUE;
                    break;
                case '-':
                    sign = FALSE;
                    break;
                case 'a': //admin
                {
                    FXString nick = utils::instance().getParam(args, argsiter, FALSE);
                    removeUser(nick);
                    if(sign)
                    {
                        addUser(m_server->getAdminPrefix()+nick);
                        if(!isCommandIgnored("mode") && !m_server->isUserIgnored(nick, getText()))
                        {
                            if(nick == getNickName()) appendIrcStyledText(FXStringFormat(_("%s gave you admin"), moderator.text()), 1, ev->time, FALSE, FALSE);
                            else appendIrcStyledText(FXStringFormat(_("%s gave %s admin"), moderator.text(), nick.text()), 1, ev->time, FALSE, FALSE);
                        }
                    }
                    else
                    {
                        addUser(nick);
                        if(!isCommandIgnored("mode") && !m_server->isUserIgnored(nick, getText()))
                        {
                            if(nick == getNickName()) appendIrcStyledText(FXStringFormat(_("%s removed you admin"), moderator.text()), 1, ev->time, FALSE, FALSE);
                            else appendIrcStyledText(FXStringFormat(_("%s removed %s admin"), moderator.text(), nick.text()), 1, ev->time, FALSE, FALSE);
                        }
                    }
                    argsiter++;
                }break;
                case 'o': //op
                {
                    FXString nick = utils::instance().getParam(args, argsiter, FALSE);
                    removeUser(nick);
                    if(sign)
                    {
                        addUser(m_server->getOpPrefix()+nick);
                        if(!isCommandIgnored("mode") && !m_server->isUserIgnored(nick, getText()))
                        {
                            if(nick == getNickName()) appendIrcStyledText(FXStringFormat(_("%s gave you op"), moderator.text()), 1, ev->time, FALSE, FALSE);
                            else appendIrcStyledText(FXStringFormat(_("%s gave %s op"), moderator.text(), nick.text()), 1, ev->time, FALSE, FALSE);
                        }
                    }
                    else
                    {
                        addUser(nick);
                        if(!isCommandIgnored("mode") && !m_server->isUserIgnored(nick, getText()))
                        {
                            if(nick == getNickName()) appendIrcStyledText(FXStringFormat(_("%s removed you op"), moderator.text()), 1, ev->time, FALSE, FALSE);
                            else appendIrcStyledText(FXStringFormat(_("%s removed %s op"), moderator.text(), nick.text()), 1, ev->time, FALSE, FALSE);
                        }
                    }
                    if(getNickName() == nick) sign ? m_iamOp = TRUE : m_iamOp = FALSE;
                    argsiter++;
                }break;
                case 'v': //voice
                {
                    FXString nick = utils::instance().getParam(args, argsiter, FALSE);
                    removeUser(nick);
                    if(sign)
                    {
                        addUser(m_server->getVoicePrefix()+nick);
                        if(!isCommandIgnored("mode") && !m_server->isUserIgnored(nick, getText()))
                        {
                            if(nick == getNickName()) appendIrcStyledText(FXStringFormat(_("%s gave you voice"), moderator.text()), 1, ev->time, FALSE, FALSE);
                            else appendIrcStyledText(FXStringFormat(_("%s gave %s voice"), moderator.text(), nick.text()), 1, ev->time, FALSE, FALSE);
                        }
                    }
                    else
                    {
                        addUser(nick);
                        if(!isCommandIgnored("mode") && !m_server->isUserIgnored(nick, getText()))
                        {
                            if(nick == getNickName()) appendIrcStyledText(FXStringFormat(_("%s removed you voice"), moderator.text()), 1, ev->time, FALSE, FALSE);
                            else appendIrcStyledText(FXStringFormat(_("%s removed %s voice"), moderator.text(), nick.text()), 1, ev->time, FALSE, FALSE);
                        }
                    }
                    argsiter++;
                }break;
                case 'h': //halfop
                {
                    FXString nick = utils::instance().getParam(args, argsiter, FALSE);
                    removeUser(nick);
                    if(sign)
                    {
                        addUser(m_server->getHalfopPrefix()+nick);
                        if(!isCommandIgnored("mode") && !m_server->isUserIgnored(nick, getText()))
                        {
                            if(nick == getNickName()) appendIrcStyledText(FXStringFormat(_("%s gave you halfop"), moderator.text()), 1, ev->time, FALSE, FALSE);
                            else appendIrcStyledText(FXStringFormat(_("%s gave %s halfop"), moderator.text(), nick.text()), 1, ev->time, FALSE, FALSE);
                        }
                    }
                    else
                    {
                        addUser(nick);
                        if(!isCommandIgnored("mode") && !m_server->isUserIgnored(nick, getText()))
                        {
                            if(nick == getNickName()) appendIrcStyledText(FXStringFormat(_("%s removed you halfop"), moderator.text()), 1, ev->time, FALSE, FALSE);
                            else appendIrcStyledText(FXStringFormat(_("%s removed %s halfop"), moderator.text(), nick.text()), 1, ev->time, FALSE, FALSE);
                        }
                    }
                    argsiter++;
                }break;
                case 'b': //ban
                {
                    FXString banmask = utils::instance().getParam(args, argsiter, FALSE);
                    onBan(banmask, sign, moderator, ev->time);
                    argsiter++;
                }break;
                case 't': //topic settable by channel operator
                {
                    sign ? m_editableTopic = FALSE : m_editableTopic = TRUE;
                }
                default:
                {
                    appendIrcStyledText(FXStringFormat(_("%s set Mode: %s"), moderator.text(), FXString(modes+" "+args).text()), 1, ev->time, FALSE, FALSE);
                }
            }
        }
    }
}

//handle IrcEvent IRC_CHMODE
void IrcTabItem::onIrcChmode(IrcEvent* ev)
{
    FXString channel = ev->param1;
    FXString modes = ev->param2;
    if(comparecase(channel, getText()) == 0)
    {
        if(modes.contains('t')) m_editableTopic = FALSE;
        appendIrcStyledText(FXStringFormat(_("Mode for %s: %s"), channel.text(), modes.text()), 1, ev->time, FALSE, FALSE);
    }
}

//handle IrcEvent IRC_SERVERREPLY
void IrcTabItem::onIrcServerReply(IrcEvent* ev)
{
    if(m_ownServerWindow)
    {
        if(m_type == SERVER)
        {
            //this->setText(server->GetRealServerName());
            appendIrcText(ev->param1, ev->time, FALSE, FALSE);
            if(getApp()->getForeColor() == this->getTextColor() && m_parent->getCurrent()*2 != m_parent->indexOfChild(this)) this->setTextColor(m_unreadColor);
        }
    }
    else
    {
        if(isFirst())
        {
            appendIrcText(ev->param1, ev->time, FALSE, FALSE);
            if(m_highlightColor != this->getTextColor() && m_parent->getCurrent()*2 != m_parent->indexOfChild(this)) this->setTextColor(m_unreadColor);
        }
    }
}

//handle IrcEvent IRC_CONNECT
void IrcTabItem::onIrcConnect(IrcEvent* ev)
{
    appendIrcStyledText(ev->param1, 3, ev->time, FALSE, FALSE);
}

//handle IrcEvent IRC_ERROR
void IrcTabItem::onIrcError(IrcEvent* ev)
{
    appendIrcStyledText(ev->param1, 4, ev->time, FALSE, FALSE);
}

//handle IrcEvent IRC_SERVERERROR
void IrcTabItem::onIrcServerError(IrcEvent* ev)
{
    if(m_ownServerWindow)
    {
        if(m_type == SERVER)
        {
            appendIrcStyledText(ev->param1, 4, ev->time, FALSE, FALSE);
            if(getApp()->getForeColor() == this->getTextColor() && m_parent->getCurrent()*2 != m_parent->indexOfChild(this)) this->setTextColor(m_unreadColor);
        }
    }
    else
    {
        if(isFirst())
        {
            appendIrcStyledText(ev->param1, 4, ev->time, FALSE, FALSE);
            if(m_highlightColor != this->getTextColor() && m_parent->getCurrent()*2 != m_parent->indexOfChild(this)) this->setTextColor(m_unreadColor);
        }
    }
}

//handle IrcEvent IRC_DISCONNECT
void IrcTabItem::onIrcDisconnect(IrcEvent* ev)
{
    appendIrcStyledText(ev->param1, 4, ev->time, FALSE, FALSE);
    if(m_ownServerWindow)
    {
        if(m_type == SERVER)
        {
            if(m_parent->getCurrent()*2 != m_parent->indexOfChild(this)) this->setTextColor(m_unreadColor);
        }
    }
    else
    {
        if(isFirst())
        {
            if(m_parent->getCurrent()*2 != m_parent->indexOfChild(this)) this->setTextColor(m_highlightColor);
        }
    }
    if(m_type == CHANNEL)
        m_users->clearItems();
}

//handle IrcEvent IRC_RECONNECT
void IrcTabItem::onIrcReconnect(IrcEvent* ev)
{
    appendIrcStyledText(ev->param1, 4, ev->time, FALSE, FALSE);
    if(m_ownServerWindow)
    {
        if(m_type == SERVER)
        {
            if(m_parent->getCurrent()*2 != m_parent->indexOfChild(this)) this->setTextColor(m_highlightColor);
        }
    }
    else
    {
        if(isFirst())
        {
            if(m_parent->getCurrent()*2 != m_parent->indexOfChild(this)) this->setTextColor(m_highlightColor);
        }
    }
    if(m_type == CHANNEL)
    {
        m_users->clearItems();
        m_iamOp = FALSE;
    }
}

//handle IrcEvent IRC_UNKNOWN
void IrcTabItem::onIrcUnknown(IrcEvent* ev)
{
    if(m_ownServerWindow)
    {
        if(m_type == SERVER)
        {
            appendIrcStyledText(FXStringFormat(_("Unhandled command '%s' params: %s"), ev->param1.text(), ev->param2.text()), 4, ev->time, FALSE, FALSE);
            if(getApp()->getForeColor() == this->getTextColor() && m_parent->getCurrent()*2 != m_parent->indexOfChild(this)) this->setTextColor(m_unreadColor);
        }
    }
    else
    {
        if(isFirst())
        {
            appendIrcStyledText(FXStringFormat(_("Unhandled command '%s' params: %s"), ev->param1.text(), ev->param2.text()), 4, ev->time, FALSE, FALSE);
            if(m_highlightColor != this->getTextColor() && m_parent->getCurrent()*2 != m_parent->indexOfChild(this)) this->setTextColor(m_unreadColor);
        }
    }
}

//handle IrcEvent IRC_301
void IrcTabItem::onIrc301(IrcEvent* ev)
{
    if(m_parent->getCurrent()*2 == m_parent->indexOfChild(this) || getText() == ev->param1)
    {
        if(!isCommandIgnored("away") && !m_server->isUserIgnored(ev->param1, getText())) appendIrcStyledText(FXStringFormat(_("%s is away: %s"),ev->param1.text(), ev->param2.text()), 1, ev->time);
    }
}

//handle IrcEvent IRC_305
void IrcTabItem::onIrc305(IrcEvent* ev)
{
    FXint i = m_users->findItem(getNickName());
    if(i != -1)
    {
        appendIrcStyledText(ev->param1, 1, ev->time, FALSE, FALSE);
        if(m_users->getItemIcon(i) == ICO_IRCAWAYOWNER)
        {
            m_users->setItemIcon(i, ICO_IRCOWNER);
            return;
        }
        if(m_users->getItemIcon(i) == ICO_IRCAWAYADMIN)
        {
            m_users->setItemIcon(i, ICO_IRCADMIN);
            return;
        }
        if(m_users->getItemIcon(i) == ICO_IRCAWAYOP)
        {
            m_users->setItemIcon(i, ICO_IRCOP);
            return;
        }
        if(m_users->getItemIcon(i) == ICO_IRCAWAYVOICE)
        {
            m_users->setItemIcon(i, ICO_IRCVOICE);
            return;
        }
        if(m_users->getItemIcon(i) == ICO_IRCAWAYHALFOP)
        {
            m_users->setItemIcon(i, ICO_IRCHALFOP);
            return;
        }
        if(m_users->getItemIcon(i) == ICO_IRCAWAYNORMAL)
        {
            m_users->setItemIcon(i, ICO_IRCNORMAL);
            return;
        }
    }
}

//handle IrcEvent IRC_306
void IrcTabItem::onIrc306(IrcEvent* ev)
{
    FXint i = m_users->findItem(getNickName());
    if(i != -1)
    {
        appendIrcStyledText(ev->param1, 1, ev->time, FALSE, FALSE);
        if(m_users->getItemIcon(i) == ICO_IRCOWNER)
        {
            m_users->setItemIcon(i, ICO_IRCAWAYOWNER);
            return;
        }
        if(m_users->getItemIcon(i) == ICO_IRCADMIN)
        {
            m_users->setItemIcon(i, ICO_IRCAWAYADMIN);
            return;
        }
        if(m_users->getItemIcon(i) == ICO_IRCOP)
        {
            m_users->setItemIcon(i, ICO_IRCAWAYOP);
            return;
        }
        if(m_users->getItemIcon(i) == ICO_IRCVOICE)
        {
            m_users->setItemIcon(i, ICO_IRCAWAYVOICE);
            return;
        }
        if(m_users->getItemIcon(i) == ICO_IRCHALFOP)
        {
            m_users->setItemIcon(i, ICO_IRCAWAYHALFOP);
            return;
        }
        if(m_users->getItemIcon(i) == ICO_IRCNORMAL)
        {
            m_users->setItemIcon(i, ICO_IRCAWAYNORMAL);
            return;
        }
    }
}

//handle IrcEvent IRC_331, IRC_332 and IRC_333
void IrcTabItem::onIrc331332333(IrcEvent* ev)
{
    if(comparecase(ev->param1, getText()) == 0)
    {
        appendIrcText(ev->param2, ev->time);
        if(ev->eventType == IRC_331)
        {
            m_topic = stripColors(ev->param2, TRUE);
            m_topicline->setText(m_topic);
            m_topicline->setCursorPos(0);
            m_topicline->makePositionVisible(0);
        }
        if(ev->eventType == IRC_332)
        {
            m_topic = stripColors(utils::instance().getParam(ev->param2, 2, TRUE, ':').after(' '), TRUE);
            m_topicline->setText(m_topic);
            m_topicline->setCursorPos(0);
            m_topicline->makePositionVisible(0);
        }
    }
}

//handle IrcEvent IRC_353
void IrcTabItem::onIrc353(IrcEvent* ev)
{
    FXString channel = ev->param1;
    FXString usersStr = ev->param2;
    FXString myNick = getNickName();
    if(usersStr.right(1) != " ") usersStr.append(" ");
    if(comparecase(channel, getText()) == 0)
    {
        while (usersStr.contains(' '))
        {
            addUser(usersStr.before(' '));
            if(usersStr[0] == m_server->getOpPrefix() && usersStr.before(' ').after(m_server->getOpPrefix()) == myNick) m_iamOp = TRUE;
            usersStr = usersStr.after(' ');
        }
    }
    else
    {
        FXbool channelOn = FALSE;
        for(FXint i = 0; i<m_parent->numChildren(); i+=2)
        {
            if(m_server->findTarget(static_cast<IrcTabItem*>(m_parent->childAtIndex(i))) && comparecase(static_cast<FXTabItem*>(m_parent->childAtIndex(i))->getText(), channel) == 0)
            {
                channelOn = TRUE;
                break;
            }
        }
        if(!channelOn && !isCommandIgnored("numeric")) appendIrcText(FXStringFormat(_("Users on %s: %s"), channel.text(), usersStr.text()), ev->time, FALSE, FALSE);
    }
}

//handle IrcEvent IRC_366
void IrcTabItem::onIrc366(IrcEvent* ev)
{
    if(comparecase(ev->param1, getText()) == 0)
    {
        m_server->sendWho(getText());
        m_server->addIgnoreCommands("who "+getText());
        if(m_numberUsers < m_maxAway)
        {
            m_checkAway = TRUE;
            getApp()->addTimeout(this, ID_TIME, 180000);
        }
    }
}

//handle IrcEvent IRC_372
void IrcTabItem::onIrc372(IrcEvent* ev)
{
    if(m_ownServerWindow)
    {
        if(m_type == SERVER)
        {
            appendIrcText(ev->param1, ev->time);
            if(getApp()->getForeColor() == this->getTextColor() && m_parent->getCurrent()*2 != m_parent->indexOfChild(this)) this->setTextColor(m_unreadColor);
        }
    }
    else
    {
        if(isFirst())
        {
            appendIrcText(ev->param1, ev->time, FALSE, FALSE);
            if(m_highlightColor != this->getTextColor() && m_parent->getCurrent()*2 != m_parent->indexOfChild(this)) this->setTextColor(m_unreadColor);
        }
    }
}

//handle IrcEvent IRC_AWAY
void IrcTabItem::onIrcAway(IrcEvent* ev)
{
    if(comparecase(ev->param1, getText()) == 0)
    {
        onAway();
    }
}

//handle IrcEvent IRC_ENDMOTD
void IrcTabItem::onIrcEndMotd()
{
    m_text->makeLastRowVisible(TRUE);
}

long IrcTabItem::onPipe(FXObject*, FXSelector, void *ptr)
{
    FXString text = *(FXString*)ptr;    
    if(m_sendPipe && (m_type == CHANNEL || m_type == QUERY))
    {
        if(!getApp()->hasTimeout(this, ID_PTIME)) getApp()->addTimeout(this, ID_PTIME);
        m_pipeStrings.append(text);
    }
    else appendIrcText(text, FXSystem::now());
    return 1;
}

//timeout for checking away in channel
long IrcTabItem::onTimeout(FXObject *, FXSelector, void*)
{
    getApp()->addTimeout(this, ID_TIME, 180000);
    if(m_checkAway)
    {
        if(m_type == CHANNEL && m_server->getConnected())
        {
            m_server->sendWho(getText());
            m_server->addIgnoreCommands("who "+getText());
        }
    }
    else
    {
        if(m_numberUsers < m_maxAway)
        {
            m_checkAway = TRUE;
            if(m_type == CHANNEL && m_server->getConnected())
            {
                m_server->sendWho(getText());
                m_server->addIgnoreCommands("who "+getText());
            }
        }
    }
    return 1;
}

long IrcTabItem::onPipeTimeout(FXObject*, FXSelector, void*)
{
    if(m_type == CHANNEL || m_type == QUERY)
    {
        if(m_pipeStrings.no() > 3)
        {
            if(m_pipeStrings[0].length() > m_maxLen-10-getText().length())
            {
                dxStringArray messages = cutText(m_pipeStrings[0], m_maxLen-10-getText().length());
                for(FXint i=0; i<messages.no(); i++)
                {
                    if(m_coloredNick) appendIrcNickText(getNickName(), messages[i], getNickColor(getNickName()), FXSystem::now());
                    else appendIrcNickText(getNickName(), messages[i], 5, FXSystem::now());
                    m_server->sendMsg(getText(), messages[i]);
                }
            }
            else
            {
                if(m_coloredNick) appendIrcNickText(getNickName(), m_pipeStrings[0], getNickColor(getNickName()), FXSystem::now());
                else appendIrcNickText(getNickName(), m_pipeStrings[0], 5, FXSystem::now());
                m_server->sendMsg(getText(), m_pipeStrings[0]);
            }
            m_pipeStrings.erase(0);
            getApp()->addTimeout(this, ID_PTIME, 3000);
        }
        else
        {
            while(m_pipeStrings.no())
            {
                if(m_pipeStrings[0].length() > m_maxLen-10-getText().length())
                {
                    dxStringArray messages = cutText(m_pipeStrings[0], m_maxLen-10-getText().length());
                    for(FXint i=0; i<messages.no(); i++)
                    {
                        if(m_coloredNick) appendIrcNickText(getNickName(), messages[i], getNickColor(getNickName()), FXSystem::now());
                        else appendIrcNickText(getNickName(), messages[i], 5, FXSystem::now());
                        m_server->sendMsg(getText(), messages[i]);
                    }
                }
                else
                {
                    if(m_coloredNick) appendIrcNickText(getNickName(), m_pipeStrings[0], getNickColor(getNickName()), FXSystem::now());
                    else appendIrcNickText(getNickName(), m_pipeStrings[0], 5, FXSystem::now());
                    m_server->sendMsg(getText(), m_pipeStrings[0]);
                }
                m_pipeStrings.erase(0);
            }
        }
    }
    return 1;
}

long IrcTabItem::onEggTimeout(FXObject*, FXSelector, void*)
{
    FXString pic1 = "     __  __\n  _/__//__/|_\n /_| |_| |/_/|\n |_   _   _|/\n /_| |_| |/_/|\n |_   _   _|/\n   |_|/|_|/\n";
    FXString pic2 = "\n\n       __  __\n    _/__//__/|_\n   /_| |_| |/_/|\n   |_   _   _|/\n   /_| |_| |/_/|\n   |_   _   _|/\n     |_|/|_|/\n";
    FXString pic3 = "\n\n\n\n         __  __\n      _/__//__/|_\n     /_| |_| |/_/|\n     |_   _   _|/\n     /_| |_| |/_/|\n     |_   _   _|/\n       |_|/|_|/\n";
    if(m_pics<24)
    {
        
        getApp()->addTimeout(this, ID_ETIME, 222);
        m_text->clearText();
        if((m_pics)%3==0)
        {
            m_text->appendStyledText(pic1, 4);
            m_pics++;
            return 1;
        }
        if((m_pics)%3==1)
        {
            m_text->appendStyledText(pic2, 4);
            m_pics++;
            return 1;
        }
        m_text->appendStyledText(pic3, 4);
        m_pics++;
    }
    return 1;
}

void IrcTabItem::onAway()
{
    if(m_checkAway)
    {
        for(FXint i = 0; i < m_users->getNumItems(); i++)
        {
            NickInfo nick = m_server->getNickInfo(m_users->getItemText(i));
            if(nick.away)
            {
                if(m_users->getItemIcon(i) == ICO_IRCOWNER) m_users->setItemIcon(i, ICO_IRCAWAYOWNER);
                else if(m_users->getItemIcon(i) == ICO_IRCADMIN) m_users->setItemIcon(i, ICO_IRCAWAYADMIN);
                else if(m_users->getItemIcon(i) == ICO_IRCOP) m_users->setItemIcon(i, ICO_IRCAWAYOP);
                else if(m_users->getItemIcon(i) == ICO_IRCVOICE) m_users->setItemIcon(i, ICO_IRCAWAYVOICE);
                else if(m_users->getItemIcon(i) == ICO_IRCHALFOP) m_users->setItemIcon(i, ICO_IRCAWAYHALFOP);
                else if(m_users->getItemIcon(i) == ICO_IRCNORMAL) m_users->setItemIcon(i, ICO_IRCAWAYNORMAL);
            }
            else
            {
                if(m_users->getItemIcon(i) == ICO_IRCAWAYOWNER) m_users->setItemIcon(i, ICO_IRCOWNER);
                else if(m_users->getItemIcon(i) == ICO_IRCAWAYADMIN) m_users->setItemIcon(i, ICO_IRCADMIN);
                else if(m_users->getItemIcon(i) == ICO_IRCAWAYOP) m_users->setItemIcon(i, ICO_IRCOP);
                else if(m_users->getItemIcon(i) == ICO_IRCAWAYVOICE) m_users->setItemIcon(i, ICO_IRCVOICE);
                else if(m_users->getItemIcon(i) == ICO_IRCAWAYHALFOP) m_users->setItemIcon(i, ICO_IRCHALFOP);
                else if(m_users->getItemIcon(i) == ICO_IRCAWAYNORMAL) m_users->setItemIcon(i, ICO_IRCNORMAL);
            }
        }
    }
    else
    {
        for(FXint i = 0; i < m_users->getNumItems(); i++)
        {
            if(m_users->getItemIcon(i) == ICO_IRCAWAYOWNER) m_users->setItemIcon(i, ICO_IRCOWNER);
            else if(m_users->getItemIcon(i) == ICO_IRCAWAYADMIN) m_users->setItemIcon(i, ICO_IRCADMIN);
            else if(m_users->getItemIcon(i) == ICO_IRCAWAYOP) m_users->setItemIcon(i, ICO_IRCOP);
            else if(m_users->getItemIcon(i) == ICO_IRCAWAYVOICE) m_users->setItemIcon(i, ICO_IRCVOICE);
            else if(m_users->getItemIcon(i) == ICO_IRCAWAYHALFOP) m_users->setItemIcon(i, ICO_IRCHALFOP);
            else if(m_users->getItemIcon(i) == ICO_IRCAWAYNORMAL) m_users->setItemIcon(i, ICO_IRCNORMAL);
        }
    }
}

long IrcTabItem::onTextLink(FXObject *, FXSelector, void *data)
{
    launchLink(static_cast<FXchar*>(data));
    return 1;
}

long IrcTabItem::onRightMouse(FXObject *, FXSelector, void *ptr)
{
    //focus();
    FXEvent* event = (FXEvent*)ptr;
    if(event->moved) return 1;
    FXint index = m_users->getItemAt(event->win_x,event->win_y);
    if(index >= 0)
    {
        NickInfo nick = m_server->getNickInfo(m_users->getItemText(index));
        m_nickOnRight = nick;
        FXString flagpath = DXIRC_DATADIR PATHSEPSTRING "icons" PATHSEPSTRING "flags";
        delete ICO_FLAG;
        ICO_FLAG = NULL;
        if(FXStat::exists(flagpath+PATHSEPSTRING+nick.host.rafter('.')+".png")) ICO_FLAG = makeIcon(getApp(), flagpath, nick.host.rafter('.')+".png", TRUE);
        else ICO_FLAG = makeIcon(getApp(), flagpath, "unknown.png", TRUE);
        FXMenuPane opmenu(this);
        new FXMenuCommand(&opmenu, _("Give op"), NULL, this, ID_OP);
        new FXMenuCommand(&opmenu, _("Remove op"), NULL, this, ID_DEOP);
        new FXMenuSeparator(&opmenu);
        new FXMenuCommand(&opmenu, _("Give voice"), NULL, this, ID_VOICE);
        new FXMenuCommand(&opmenu, _("Remove voice"), NULL, this, ID_DEVOICE);
        new FXMenuSeparator(&opmenu);
        new FXMenuCommand(&opmenu, _("Kick"), NULL, this, ID_KICK);
        new FXMenuSeparator(&opmenu);
        new FXMenuCommand(&opmenu, _("Ban"), NULL, this, ID_BAN);
        new FXMenuCommand(&opmenu, _("KickBan"), NULL, this, ID_KICKBAN);
        FXMenuPane popup(this);
        new FXMenuCommand(&popup, FXStringFormat(_("User: %s@%s"), nick.user.text(), nick.host.text()), ICO_FLAG);
        new FXMenuCommand(&popup, FXStringFormat(_("Realname: %s"), nick.real.text()));
        if(nick.nick != getNickName())
        {
            new FXMenuSeparator(&popup);
            new FXMenuCommand(&popup, _("Query"), NULL, this, ID_NEWQUERY);
            new FXMenuCommand(&popup, _("User information (WHOIS)"), NULL, this, ID_WHOIS);
            new FXMenuCommand(&popup, _("DCC chat"), NULL, this, ID_DCCCHAT);
            new FXMenuCommand(&popup, _("Send file"), NULL, this, ID_DCCSEND);
            new FXMenuCommand(&popup, _("Ignore"), NULL, this, ID_IGNORE);
            if(m_iamOp) new FXMenuCascade(&popup, _("Operator actions"), NULL, &opmenu);
        }
        else
        {
            new FXMenuSeparator(&popup);
            if(m_server->isAway(getNickName())) new FXMenuCommand(&popup, _("Remove Away"), NULL, this, ID_DEAWAY);
            else new FXMenuCommand(&popup, _("Set Away"), NULL, this, ID_AWAY);
        }
        popup.create();
        popup.popup(NULL,event->root_x,event->root_y);
        getApp()->runModalWhileShown(&popup);
    }
    return 1;
}

long IrcTabItem::onDoubleclick(FXObject*, FXSelector, void*)
{
    FXint index = m_users->getCursorItem();
    if(index >= 0)
    {
        if(m_users->getItemText(index) == getNickName()) return 1;
        IrcEvent ev;
        ev.eventType = IRC_QUERY;
        ev.param1 = m_users->getItemText(index);
        ev.param2 = getNickName();
        m_parent->getParent()->getParent()->handle(m_server, FXSEL(SEL_COMMAND, IrcSocket::ID_SERVER), &ev);
    }
    return 1;
}

long IrcTabItem::onNewQuery(FXObject *, FXSelector, void *)
{
    IrcEvent ev;
    ev.eventType = IRC_QUERY;
    ev.param1 = m_nickOnRight.nick;
    ev.param2 = getNickName();
    m_parent->getParent()->getParent()->handle(m_server, FXSEL(SEL_COMMAND, IrcSocket::ID_SERVER), &ev);
    return 1;
}

long IrcTabItem::onWhois(FXObject *, FXSelector, void *)
{
    m_server->sendWhois(m_nickOnRight.nick);
    return 1;
}

long IrcTabItem::onDccChat(FXObject*, FXSelector, void*)
{
    IrcEvent ev;
    ev.eventType = IRC_DCCSERVER;
    ev.param1 = m_nickOnRight.nick;
    m_parent->getParent()->getParent()->handle(m_server, FXSEL(SEL_COMMAND, IrcSocket::ID_SERVER), &ev);
    return 1;
}

long IrcTabItem::onDccSend(FXObject*, FXSelector, void*)
{
    DccSendDialog dialog((FXMainWindow*)m_parent->getParent()->getParent(), m_nickOnRight.nick);
    if(dialog.execute())
    {
        IrcEvent ev;
        ev.eventType = dialog.getPassive() ? IRC_DCCPOUT: IRC_DCCOUT;
        ev.param1 = m_nickOnRight.nick;
        ev.param2 = dialog.getFilename();
        m_parent->getParent()->getParent()->handle(m_server, FXSEL(SEL_COMMAND, IrcSocket::ID_SERVER), &ev);
    }
    return 1;
}

long IrcTabItem::onOp(FXObject *, FXSelector, void *)
{
    m_server->sendMode(getText()+" +o "+m_nickOnRight.nick);
    return 1;
}

long IrcTabItem::onDeop(FXObject *, FXSelector, void *)
{
    m_server->sendMode(getText()+" -o "+m_nickOnRight.nick);
    return 1;
}

long IrcTabItem::onVoice(FXObject *, FXSelector, void *)
{
    m_server->sendMode(getText()+" +v "+m_nickOnRight.nick);
    return 1;
}

long IrcTabItem::onDevoice(FXObject *, FXSelector, void *)
{
    m_server->sendMode(getText()+" -v "+m_nickOnRight.nick);
    return 1;
}

long IrcTabItem::onKick(FXObject *, FXSelector, void *)
{
    FXDialogBox kickDialog(this, _("Kick dialog"), DECOR_TITLE|DECOR_BORDER, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    FXVerticalFrame *contents = new FXVerticalFrame(&kickDialog, LAYOUT_SIDE_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0, 0, 0, 0, 10, 10, 10, 10, 0, 0);

    FXHorizontalFrame *kickframe = new FXHorizontalFrame(contents, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(kickframe, _("Kick reason:"), 0, JUSTIFY_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    FXTextField *reasonEdit = new FXTextField(kickframe, 25, NULL, 0, FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);

    FXHorizontalFrame *buttonframe = new FXHorizontalFrame(contents, LAYOUT_FILL_X|LAYOUT_FILL_Y|PACK_UNIFORM_WIDTH);
    new FXButton(buttonframe, _("&Cancel"), NULL, &kickDialog, FXDialogBox::ID_CANCEL, FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0, 0, 0, 0, 10, 10, 2, 5);
    new FXButton(buttonframe, _("&OK"), NULL, &kickDialog, FXDialogBox::ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0, 0, 0, 0, 10, 10, 2, 5);

    if(kickDialog.execute(PLACEMENT_CURSOR))
    {
        m_server->sendKick(getText(), m_nickOnRight.nick, reasonEdit->getText());
    }
    return 1;
}

long IrcTabItem::onBan(FXObject *, FXSelector, void *)
{
    FXDialogBox banDialog(this, _("Ban dialog"), DECOR_TITLE|DECOR_BORDER, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    FXVerticalFrame *contents = new FXVerticalFrame(&banDialog, LAYOUT_SIDE_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0, 0, 0, 0, 10, 10, 10, 10, 0, 0);

    FXHorizontalFrame *banframe = new FXHorizontalFrame(contents, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(banframe, _("Banmask:"), 0, JUSTIFY_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    FXTextField *banEdit = new FXTextField(banframe, 25, NULL, 0, FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    banEdit->setText(m_nickOnRight.nick+"!"+m_nickOnRight.user+"@"+m_nickOnRight.host);

    FXHorizontalFrame *buttonframe = new FXHorizontalFrame(contents, LAYOUT_FILL_X|LAYOUT_FILL_Y|PACK_UNIFORM_WIDTH);
    new FXButton(buttonframe, _("&Cancel"), NULL, &banDialog, FXDialogBox::ID_CANCEL, FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0, 0, 0, 0, 10, 10, 2, 5);
    new FXButton(buttonframe, _("&OK"), NULL, &banDialog, FXDialogBox::ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0, 0, 0, 0, 10, 10, 2, 5);

    if(banDialog.execute(PLACEMENT_CURSOR))
    {
        m_server->sendMode(getText()+" +b "+banEdit->getText());
    }
    return 1;
}

long IrcTabItem::onKickban(FXObject *, FXSelector, void *)
{
    FXDialogBox banDialog(this, _("Kick/Ban dialog"), DECOR_TITLE|DECOR_BORDER, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    FXVerticalFrame *contents = new FXVerticalFrame(&banDialog, LAYOUT_SIDE_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0, 0, 0, 0, 10, 10, 10, 10, 0, 0);

    FXHorizontalFrame *kickframe = new FXHorizontalFrame(contents, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(kickframe, _("Kick reason:"), 0, JUSTIFY_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    FXTextField *reasonEdit = new FXTextField(kickframe, 25, NULL, 0, FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);

    FXHorizontalFrame *banframe = new FXHorizontalFrame(contents, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(banframe, _("Banmask:"), 0, JUSTIFY_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    FXTextField *banEdit = new FXTextField(banframe, 25, NULL, 0, FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    banEdit->setText(m_nickOnRight.nick+"!"+m_nickOnRight.user+"@"+m_nickOnRight.host);

    FXHorizontalFrame *buttonframe = new FXHorizontalFrame(contents, LAYOUT_FILL_X|LAYOUT_FILL_Y|PACK_UNIFORM_WIDTH);
    new FXButton(buttonframe, _("&Cancel"), NULL, &banDialog, FXDialogBox::ID_CANCEL, FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0, 0, 0, 0, 10, 10, 2, 5);
    new FXButton(buttonframe, _("&OK"), NULL, &banDialog, FXDialogBox::ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0, 0, 0, 0, 10, 10, 2, 5);

    if(banDialog.execute(PLACEMENT_CURSOR))
    {
        m_server->sendKick(getText(), m_nickOnRight.nick, reasonEdit->getText());
        m_server->sendMode(getText()+" +b "+banEdit->getText());
    }
    return 1;
}

//handle popup Ignore
long IrcTabItem::onIgnore(FXObject*, FXSelector, void*)
{
    FXDialogBox dialog(this, _("Add ignore user"), DECOR_TITLE|DECOR_BORDER, 0,0,0,0, 0,0,0,0, 0,0);
    FXVerticalFrame *contents = new FXVerticalFrame(&dialog, LAYOUT_SIDE_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 10,10,10,10, 0,0);
    FXMatrix *matrix = new FXMatrix(contents,2,MATRIX_BY_COLUMNS|LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y);

    new FXLabel(matrix, _("Nick:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *nick = new FXTextField(matrix, 25, NULL, 0, FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    nick->setText(m_nickOnRight.nick+"!"+m_nickOnRight.user+"@"+m_nickOnRight.host);
    new FXLabel(matrix, _("Channel(s):\tChannels need to be comma separated"), NULL,JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *channel = new FXTextField(matrix, 25, NULL, 0, FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    channel->setText(getText());
    channel->setTipText(_("Channels need to be comma separated"));
    new FXLabel(matrix, _("Server:"), NULL,JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *server = new FXTextField(matrix, 25, NULL, 0, FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    server->setText(getServerName());

    FXHorizontalFrame *buttonframe = new FXHorizontalFrame(contents,LAYOUT_FILL_X|LAYOUT_FILL_Y|PACK_UNIFORM_WIDTH);
    new FXButton(buttonframe, _("&Cancel"), NULL, &dialog, FXDialogBox::ID_CANCEL, FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,5);
    new FXButton(buttonframe, _("&OK"), NULL, &dialog, FXDialogBox::ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,5);

    if(dialog.execute(PLACEMENT_CURSOR))
    {
        FXString ignoretext = nick->getText()+" "+channel->getText()+" "+server->getText();
        m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_ADDIUSER), &ignoretext);
    }
    return 1;
}

//handle ID_AWAY
long IrcTabItem::onSetAway(FXObject*, FXSelector, void*)
{
    FXDialogBox awayDialog(this, _("Away dialog"), DECOR_TITLE|DECOR_BORDER, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    FXVerticalFrame *contents = new FXVerticalFrame(&awayDialog, LAYOUT_SIDE_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0, 0, 0, 0, 10, 10, 10, 10, 0, 0);

    FXHorizontalFrame *msgframe = new FXHorizontalFrame(contents, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(msgframe, _("Message:"), 0, JUSTIFY_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    FXTextField *msgEdit = new FXTextField(msgframe, 25, NULL, 0, FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    msgEdit->setText(_("away"));

    FXHorizontalFrame *buttonframe = new FXHorizontalFrame(contents, LAYOUT_FILL_X|LAYOUT_FILL_Y|PACK_UNIFORM_WIDTH);
    new FXButton(buttonframe, _("&Cancel"), NULL, &awayDialog, FXDialogBox::ID_CANCEL, FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0, 0, 0, 0, 10, 10, 2, 5);
    new FXButton(buttonframe, _("&OK"), NULL, &awayDialog, FXDialogBox::ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0, 0, 0, 0, 10, 10, 2, 5);

    if(awayDialog.execute(PLACEMENT_CURSOR))
    {
        m_server->sendAway(msgEdit->getText().empty() ? _("away"): msgEdit->getText());
    }
    return 1;
}

//handle ID_DEAWAY
long IrcTabItem::onRemoveAway(FXObject*, FXSelector, void*)
{
    m_server->sendAway("");
    return 1;
}

//handle change in spellLang combobox
long IrcTabItem::onSpellLang(FXObject*, FXSelector, void*)
{
    m_commandline->setLanguage(m_spellLangs->getItemText(m_spellLangs->getCurrentItem()));
    m_commandline->setTipText(FXStringFormat(_("Current spellchecking language: %s"),m_spellLangs->getItemText(m_spellLangs->getCurrentItem()).text()));
    return 1;
}

long IrcTabItem::onTopic(FXObject*, FXSelector, void*)
{
    if(m_editableTopic || m_iamOp)
    {
        if(m_topicline->getText().length() > m_server->getTopicLen())
        {
            appendIrcStyledText(FXStringFormat(_("Warning: topic is too long. Max. topic length is %d."), m_server->getTopicLen()), 4, FXSystem::now(), FALSE, FALSE);
            m_server->sendTopic(getText(), m_topicline->getText());
            return 1;
        }
        m_server->sendTopic(getText(), m_topicline->getText());
    }
    else
    {
        m_topicline->setText(m_topic);
        m_topicline->setCursorPos(0);
        m_topicline->makePositionVisible(0);
    }
    return 1;
}

long IrcTabItem::onTopicLink(FXObject*, FXSelector, void *data)
{
    launchLink(static_cast<FXchar*>(data));
    return 1;
}

void IrcTabItem::onBan(const FXString &banmask, const FXbool &sign, const FXString &sender, const FXTime &time)
{
    if(sign)
    {
        FXString nicks = m_server->getBannedNick(banmask);
        FXString myNick = getNickName();
        while(nicks.contains(';'))
        {
            for(FXint i=m_users->getNumItems()-1; i>-1; i--)
            {
                if(nicks.before(';') == m_users->getItemText(i))
                {
                    if(m_users->getItemText(i) == myNick) appendIrcStyledText(FXStringFormat(_("You was banned by %s"), sender.text()), 1, time, FALSE, FALSE);
                    else
                    {
                        if(!isCommandIgnored("ban") && !m_server->isUserIgnored(m_users->getItemText(i), getText())) appendIrcStyledText(FXStringFormat(_("%s was banned by %s"), m_users->getItemText(i).text(), sender.text()), 1, time, FALSE, FALSE);
                        //RemoveUser(users->getItemText(i));
                    }
                }
            }
            nicks = nicks.after(';');
        }
    }
}

FXString IrcTabItem::stripColors(const FXString &text, const FXbool stripOther)
{
    FXString newstr;
    FXbool color = FALSE;
    FXint numbers = 0;
    FXint i = 0;
    while(text[i] != '\0') {
        if(text[i] == '\017') //reset
        {
            color = FALSE;
        }
        else if(stripOther && text[i] == '\002')
        {
            //remove bold mark
        }
        else if(stripOther && text[i] == '\037')
        {
            //remove underline mark
        }
        else if(text[i] == '\035')
        {
            //remove italic mark
        }
        else if(text[i] == '\021')
        {
            //remove fixed mark
        }
        else if(text[i] == '\026')
        {
            //remove reverse mark
        }
        else if(text[i] == '\003') //color
        {
            color = TRUE;
        }
        else if(color && isdigit(text[i]) && numbers < 2)
        {
            numbers++;
        }
        else if(color && text[i] == ',' && numbers < 3)
        {
            numbers = 0;
        }
        else
        {
            numbers = 0;
            color = FALSE;
            newstr += text[i];
        }
        i++;
    }
    return newstr;
}

FXString IrcTabItem::getNick(FXint i)
{
    return m_users->getItemText(i);
}

FXint IrcTabItem::launchLink(const FXString &link)
{
#ifdef WIN32
    return ((FXint)ShellExecuteA(NULL,"open",FXPath::enquote(link).text(),NULL,NULL,SW_SHOWNORMAL)) > 32;
#else
    static const char * browsers[]={"xdg-open","firefox","iceweasel","konqueror","opera","netscape","dillo","open",NULL};
    FXString path = FXSystem::getExecPath();
    FXString exec;
    for(int i=0; browsers[i]!=NULL; i++)
    {
        exec = FXPath::search(path, browsers[i]);
        if(!exec.empty()) break;
    }
    if(exec.empty()) return 0;
    exec += " "+FXPath::enquote(link)+" &";
    system(exec.text());
    return 1;
#endif
}

FXint IrcTabItem::getNickColor(const FXString &nick)
{
    //10 is first colored nick style
    return 10+nick.hash()%8;
}

FXColor IrcTabItem::getIrcColor(FXint code)
{
    switch(code){
        case 0:
            return fxcolorfromname("white");
        case 1:
            return fxcolorfromname("black");
        case 2:
            return FXRGB(0,0,128); //blue
        case 3:
            return FXRGB(0,128,0); //green
        case 4:
            return FXRGB(255,0,0); //lightred
        case 5:
            return FXRGB(128,0,64); //brown
        case 6:
            return FXRGB(128,0,128); //purple
        case 7:
            return FXRGB(255,128,64); //orange
        case 8:
            return FXRGB(255,255,0); //yellow
        case 9:
            return FXRGB(128,255,0); //lightgreen
        case 10:
            return FXRGB(0,128,128); //cyan
        case 11:
            return FXRGB(0,255,255); //lightcyan
        case 12:
            return FXRGB(0,0,255); //lightblue
        case 13:
            return FXRGB(255,0,255); //pink
        case 14:
            return FXRGB(128,128,128); //grey
        case 15:
            return FXRGB(192,192,192); //lightgrey
        default:
            return m_colors.text;
    }
}

FXint IrcTabItem::hiliteStyleExist(FXColor foreColor, FXColor backColor, FXuint style)
{
    for(FXint i=0; i<m_textStyleList.no(); i++)
    {
        if(m_textStyleList[i].normalForeColor == foreColor
                && m_textStyleList[i].normalBackColor == backColor
                && m_textStyleList[i].style == style)
            return i+1;
    }
    return -1;
}

void IrcTabItem::createHiliteStyle(FXColor foreColor, FXColor backColor, FXuint style)
{
    dxHiliteStyle nstyle = {foreColor,backColor,getApp()->getSelforeColor(),getApp()->getSelbackColor(),style,FALSE};
    m_textStyleList.append(nstyle);
    m_text->setHiliteStyles(m_textStyleList.data());
}

dxStringArray IrcTabItem::cutText(FXString text, FXint len)
{
    FXint textLen = text.length();
    FXint previous = 0;
    dxStringArray texts;
    while(textLen>len)
    {
        texts.append(text.mid(previous, len));
        previous += len;
        textLen -= len;
    }
    texts.append(text.mid(previous, len));
    return texts;    
}

void IrcTabItem::setCommandFocus()
{
    m_commandline->setFocus();
}

//for "handle" checking, if script contains "all". Send from dxirc.
void IrcTabItem::hasAllCommand(FXbool result)
{
    m_scriptHasAll = result;
}

//for "handle" checking, if script contains "mymsg". Send from dxirc.
void IrcTabItem::hasMyMsg(FXbool result)
{
    m_scriptHasMyMsg = result;
}

//check need of highlight in msg
FXbool IrcTabItem::highlightNeeded(const FXString &msg)
{
    FXint pos = msg.find(getNickName());
    if(pos==-1) return FALSE;
    FXbool before = TRUE;
    FXbool after = FALSE;
    if(pos) before = isDelimiter(msg[pos-1]);
    if(pos+getNickName().length() == msg.length()) after = TRUE;
    if(pos+getNickName().length() < msg.length()) after = isDelimiter(msg[pos+getNickName().length()]);
    return before && after;
}
