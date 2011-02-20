/*
 *      dcctabitem.cpp
 *
 *      Copyright 2010 David Vachulka <david@konstrukce-cad.com>
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

#include "dcctabitem.h"
#include "utils.h"
#include "tetristabitem.h"
#include "icons.h"
#include "config.h"
#include "i18n.h"
#include "ircengine.h"

FXDEFMAP(DccTabItem) DccTabItemMap[] = {
    FXMAPFUNC(SEL_COMMAND,          DccTabItem::ID_COMMANDLINE,     DccTabItem::onCommandline),
    FXMAPFUNC(SEL_KEYPRESS,         DccTabItem::ID_COMMANDLINE,     DccTabItem::onKeyPress),
    FXMAPFUNC(SEL_TIMEOUT,          DccTabItem::ID_PTIME,           DccTabItem::onPipeTimeout),
    FXMAPFUNC(SEL_TEXTLINK,         DccTabItem::ID_TEXT,            DccTabItem::onTextLink),
    FXMAPFUNC(SEL_COMMAND,          DccTabItem::ID_SPELL,           DccTabItem::onSpellLang),
    FXMAPFUNC(SEL_COMMAND,          dxPipe::ID_PIPE,                DccTabItem::onPipe),
    FXMAPFUNC(SEL_TIMEOUT,          DccTabItem::ID_CTIME,           DccTabItem::onCloseTimeout),
    FXMAPFUNC(SEL_COMMAND,          IrcEngine::ID_SERVER,           DccTabItem::onIrcEvent),
    FXMAPFUNC(SOCKET_CANREAD,       DccTabItem::ID_SOCKET,          DccTabItem::onSocketCanRead),
    FXMAPFUNC(SOCKET_CONNECTED,     DccTabItem::ID_SOCKET,          DccTabItem::onSocketConnected),
    FXMAPFUNC(SOCKET_DISCONNECTED,  DccTabItem::ID_SOCKET,          DccTabItem::onSocketDisconnected),
    FXMAPFUNC(SOCKET_ERR,           DccTabItem::ID_SOCKET,          DccTabItem::onSocketError),
    FXMAPFUNC(SOCKET_STARTACCEPT,   DccTabItem::ID_SOCKET,          DccTabItem::onSocketStartAccept),
    FXMAPFUNC(SOCKET_LISTEN,        DccTabItem::ID_SOCKET,          DccTabItem::onSocketListen)
};

FXIMPLEMENT(DccTabItem, dxTabItem, DccTabItemMap, ARRAYNUMBER(DccTabItemMap))

DccTabItem::DccTabItem(dxTabBook *tab, const FXString &mynick, const FXString &nick,
        const FXString &address, FXint portD, FXint portH, FXbool listen,
        FXIcon *icon, FXuint opts, FXint id, FXbool logging, FXString logPath, IrcColor colors,
        FXFont *font, FXbool sameCommand, FXbool coloredNick,
        FXbool stripColors, FXbool useSpell, FXbool showSpellCombo, IrcEngine *engine)
    : dxTabItem(tab, nick, icon, opts, id), m_parent(tab), m_logging(logging),
        m_sameCmd(sameCommand), m_coloredNick(coloredNick), m_listen(listen),
        m_stripColors(stripColors), m_useSpell(useSpell), m_showSpellCombo(showSpellCombo),
        m_colors(colors), m_logPath(logPath), m_logstream(NULL), m_mynick(mynick), m_nick(nick),
        m_address(address), m_portD(portD), m_portH(portH), m_engine(engine)
{
    m_currentPosition = 0;
    m_historyMax = 25;
    m_pipe = NULL;
    m_sendPipe = FALSE;
    m_scriptHasAll = FALSE;
    m_scriptHasMyMsg = FALSE;
    m_unreadColor = utils::instance().getColorIniEntry("SETTINGS", "unreadcolor", FXRGB(0,0,255));
    m_highlightColor = utils::instance().getColorIniEntry("SETTINGS", "highlightcolor", FXRGB(255,0,0));
    m_receiveRest = "";
    m_dccIP = utils::instance().getStringIniEntry("SETTINGS", "dccIP");
    m_dccTimeout = utils::instance().getIntIniEntry("SETTINGS", "dccTimeout", 66);

    m_mainframe = new FXVerticalFrame(m_parent, FRAME_RAISED|LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y);

    m_textframe = new FXVerticalFrame(m_mainframe, FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    m_text = new dxText(m_textframe, this, ID_TEXT, FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y|TEXT_READONLY|TEXT_WORDWRAP|TEXT_SHOWACTIVE|TEXT_AUTOSCROLL);
    m_text->setFont(font);
    m_text->setSelTextColor(getApp()->getSelforeColor());
    m_text->setSelBackColor(getApp()->getSelbackColor());

    m_commandframe = new FXHorizontalFrame(m_mainframe, LAYOUT_FILL_X, 0,0,0,0, 0,0,0,0);
    m_commandline = new dxTextField(m_commandframe, 25, this, ID_COMMANDLINE, TEXTFIELD_ENTER_ONLY|FRAME_SUNKEN|JUSTIFY_LEFT|LAYOUT_FILL_X|LAYOUT_BOTTOM, 0, 0, 0, 0, 1, 1, 1, 1);
    if(m_sameCmd) m_commandline->setFont(font);
    m_spellLangs = new FXComboBox(m_commandframe, 6, this, ID_SPELL, COMBOBOX_STATIC);
    m_spellLangs->setTipText(_("Spellchecking language list"));
    m_spellLangs->hide();
    if(m_sameCmd) m_spellLangs->setFont(font);
    if(m_useSpell && utils::instance().getLangsNum())
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
    m_text->setTextColor(m_colors.text);
    m_commandline->setTextColor(m_colors.text);
    m_commandline->setCursorColor(m_colors.text);
    m_spellLangs->setBackColor(m_colors.back);
    m_spellLangs->setTextColor(m_colors.text);

    m_socket = new dxSocket(getApp(), this, ID_SOCKET);
}

DccTabItem::~DccTabItem()
{
    this->stopLogging();
    if(m_pipe) m_pipe->stopCmd();
    m_pipeStrings.clear();
    getApp()->removeTimeout(this, ID_PTIME);
    m_socket->disconnect();
    delete m_socket;
}

void DccTabItem::createGeom()
{
    m_mainframe->create();
    m_commandline->setFocus();
    m_socket->setIsSSL(FALSE);
    if(m_listen)
    {
        IrcEvent ev;
        ev.eventType = IRC_CONNECT;
        ev.param1 = FXStringFormat(_("Listening on %s"), m_address.text());
        ev.time = FXSystem::now();
        writeIrcEvent(ev);
        if(m_socket->isIPv4(m_address))
        {
            if(!m_socket->isRoutableIP(FXUIntVal(m_socket->stringIPToBinary(m_address))))
            {
                ev.eventType = IRC_SERVERERROR;
                ev.param1 = FXStringFormat(_("%s isn't routable."), m_address.text());
                ev.time = FXSystem::now();
                writeIrcEvent(ev);
            }
            if(!m_engine->getMyUserHost().empty() || !m_dccIP.empty())
            {
                if(!m_dccIP.empty())
                {
                    m_address = m_dccIP;
                    ev.eventType = IRC_CONNECT;
                    ev.param1 = FXStringFormat(_("Trying IP from settings %s"), m_address.text());
                    ev.time = FXSystem::now();
                    writeIrcEvent(ev);
                }
                else
                {
                    m_address = m_engine->getMyUserHost();
                    ev.eventType = IRC_CONNECT;
                    ev.param1 = FXStringFormat(_("Trying IP from server %s"), m_address.text());
                    ev.time = FXSystem::now();
                    writeIrcEvent(ev);
                }
                if(m_socket->isIPv4(m_address) && !m_socket->isRoutableIP(FXUIntVal(m_socket->stringIPToBinary(m_address))))
                {
                    ev.eventType = IRC_SERVERERROR;
                    ev.param1 = FXStringFormat(_("%s isn't routable too, but will be used"), m_address.text());
                    ev.time = FXSystem::now();
                    writeIrcEvent(ev);
                }
            }
            else
            {
                ev.eventType = IRC_SERVERERROR;
                ev.param1 = _("But better doesn't exist.");
                ev.time = FXSystem::now();
                writeIrcEvent(ev);
            }
        }
    }
    m_socket->setHost(m_address, m_portD, m_portH);
    if(m_listen) m_socket->listenOn();
    else m_socket->connectTo();
}

void DccTabItem::clearChat()
{
    m_textStyleList.no(17);
    m_text->setHiliteStyles(m_textStyleList.data());
    m_text->clearText();
}

//usefull for set tab current
void DccTabItem::makeLastRowVisible()
{
    m_text->makeLastRowVisible(TRUE);
}

FXString DccTabItem::getSpellLang()
{
#ifdef HAVE_ENCHANT
    if(m_spellLangs->getNumItems()) return m_spellLangs->getItemText(m_spellLangs->getCurrentItem());
#endif
    return "";
}

void DccTabItem::reparentTab()
{
    reparent(m_parent);
    m_mainframe->reparent(m_parent);
}

void DccTabItem::setColor(IrcColor clrs)
{
    m_colors = clrs;
    setTextForeColor(clrs.text);
    setTextBackColor(clrs.back);
    setActionsColor(clrs.action);
    setNoticeColor(clrs.notice);
    setErrorColor(clrs.error);
    setHilightColor(clrs.hilight);
    setLinkColor(m_colors.link);
}

void DccTabItem::setTextBackColor(FXColor clr)
{
    for(FXint i=0; i<m_textStyleList.no(); i++)
    {
        m_textStyleList[i].normalBackColor = clr;
    }
    m_text->setBackColor(clr);
    m_commandline->setBackColor(clr);
    m_spellLangs->setBackColor(clr);
}

void DccTabItem::setTextForeColor(FXColor clr)
{
    m_textStyleList[4].normalForeColor = clr;
    m_textStyleList[5].normalForeColor = clr;
    m_textStyleList[6].normalForeColor = clr;
    m_text->setTextColor(clr);
    m_commandline->setTextColor(clr);
    m_commandline->setCursorColor(clr);
    m_spellLangs->setTextColor(clr);
}

void DccTabItem::setActionsColor(FXColor clr)
{
    m_textStyleList[1].normalForeColor = clr;
}

void DccTabItem::setNoticeColor(FXColor clr)
{
    m_textStyleList[2].normalForeColor = clr;
}

void DccTabItem::setErrorColor(FXColor clr)
{
    m_textStyleList[3].normalForeColor = clr;
}

void DccTabItem::setHilightColor(FXColor clr)
{
    m_textStyleList[7].normalForeColor = clr;
}

void DccTabItem::setLinkColor(FXColor clr)
{
    m_textStyleList[8].normalForeColor = clr;
}

void DccTabItem::setUnreadTabColor(FXColor clr)
{
    if(m_unreadColor!=clr)
    {
        FXbool update = this->getTextColor()==m_unreadColor;
        m_unreadColor = clr;
        if(update) this->setTextColor(m_unreadColor);
    }
}

void DccTabItem::setHighlightTabColor(FXColor clr)
{
    if(m_highlightColor!=clr)
    {
        FXbool update = this->getTextColor()==m_highlightColor;
        m_highlightColor = clr;
        if(update) this->setTextColor(m_highlightColor);
    }
}

void DccTabItem::setLogging(FXbool log)
{
    m_logging = log;
}

void DccTabItem::setLogPath(FXString pth)
{
    m_logPath = pth;
    this->stopLogging();
}

void DccTabItem::setIrcFont(FXFont *fnt)
{
    if(m_text->getFont() != fnt)
    {
        m_text->setFont(fnt);
        m_text->recalc();
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
}

void DccTabItem::setSameCmd(FXbool scmd)
{
    m_sameCmd = scmd;
}

void DccTabItem::setColoredNick(FXbool cnick)
{
    m_coloredNick = cnick;
}

void DccTabItem::setStripColors(FXbool sclr)
{
    m_stripColors = sclr;
}

void DccTabItem::setSmileys(FXbool smiley, dxSmileyArray nsmileys)
{
    m_text->setSmileys(smiley, nsmileys);
}

void DccTabItem::setUseSpell(FXbool useSpell)
{
    m_useSpell = useSpell;
    if(m_useSpell && utils::instance().getLangsNum())
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

void DccTabItem::setShowSpellCombo(FXbool showSpellCombo)
{
    if(m_showSpellCombo!=showSpellCombo)
    {
        m_showSpellCombo = showSpellCombo;
        if(m_showSpellCombo) m_spellLangs->show();
        else m_spellLangs->hide();
        m_commandframe->recalc();
    }
}

void DccTabItem::removeSmileys()
{
    m_text->removeSmileys();
}

//if highlight==TRUE, highlight tab
void DccTabItem::appendText(FXString msg, FXbool highlight, FXbool logLine)
{
    appendIrcText(msg, 0, FALSE, logLine);
    if(highlight && m_highlightColor != this->getTextColor() && m_parent->getCurrent()*2 != m_parent->indexOfChild(this))
    {
        if(msg.contains(getNickName()))
        {
            this->setTextColor(m_highlightColor);
        }
        else this->setTextColor(m_unreadColor);
        this->setIcon(ICO_DCCNEWMSG);
    }
}

void DccTabItem::appendIrcText(FXString msg, FXTime time, FXbool disableStrip, FXbool logLine)
{
    if(!time) time = FXSystem::now();
    m_text->appendText("["+FXSystem::time("%H:%M:%S", time) +"] ");
    appendLinkText(m_stripColors && !disableStrip ? stripColors(msg, FALSE) : msg, 0);
    if(logLine) this->logLine(stripColors(msg, TRUE), time);
}

void DccTabItem::appendIrcNickText(FXString nick, FXString msg, FXint style, FXTime time, FXbool logLine)
{
    if(!time) time = FXSystem::now();
    m_text->appendText("["+FXSystem::time("%H:%M:%S", time) +"] ");
    m_text->appendStyledText(nick+": ", style);
    appendLinkText(m_stripColors ? stripColors(msg, FALSE) : msg, 0);
    if(logLine) this->logLine(stripColors("<"+nick+"> "+msg, TRUE), time);
}

/* if highlight==TRUE, highlight tab
 * disableStrip is for dxirc.Print
*/
void DccTabItem::appendStyledText(FXString text, FXint style, FXbool highlight, FXbool disableStrip, FXbool logLine)
{
    if(style) appendIrcStyledText(text, style, 0, disableStrip, logLine);
    else appendIrcText(text, 0, disableStrip, logLine);
    if(highlight && m_highlightColor != this->getTextColor() && m_parent->getCurrent()*2 != m_parent->indexOfChild(this))
    {
        if(text.contains(getNickName()))
        {
            this->setTextColor(m_highlightColor);
        }
        else this->setTextColor(m_unreadColor);
        this->setIcon(ICO_DCCNEWMSG);
    }
}

void DccTabItem::appendIrcStyledText(FXString styled, FXint stylenum, FXTime time, FXbool disableStrip, FXbool logLine)
{
    if(!time) time = FXSystem::now();
    m_text->appendText("["+FXSystem::time("%H:%M:%S", time) +"] ");
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

void DccTabItem::appendLinkText(const FXString &txt, FXint stylenum)
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

void DccTabItem::startLogging()
{
    if(m_logstream && FXStat::exists(m_logPath+PATHSEPSTRING+getServerName()+PATHSEPSTRING+getText()+PATHSEPSTRING+FXSystem::time("%Y-%m-%d", FXSystem::now())))
        return;
    if(m_logging)
    {
        if(!FXStat::exists(m_logPath+PATHSEPSTRING+getServerName())) FXDir::create(m_logPath+PATHSEPSTRING+getServerName());
        if(!FXStat::exists(m_logPath+PATHSEPSTRING+getServerName()+PATHSEPSTRING+getText())) FXDir::create(m_logPath+PATHSEPSTRING+getServerName()+PATHSEPSTRING+getText());
        m_logstream = new std::ofstream(FXString(m_logPath+PATHSEPSTRING+getServerName()+PATHSEPSTRING+getText()+PATHSEPSTRING+FXSystem::time("%Y-%m-%d", FXSystem::now())).text(), std::ios::out|std::ios::app);
    }
}

void DccTabItem::stopLogging()
{
    if(m_logstream)
    {
        m_logstream->close();
        delete m_logstream;
        m_logstream = NULL;
    }
}

void DccTabItem::logLine(const FXString &line, const FXTime &time)
{
    if(m_logging)
    {
        this->startLogging();
        *m_logstream << "[" << FXSystem::time("%H:%M:%S", time).text() << "] " << line.text() << std::endl;
    }
}

long DccTabItem::onCommandline(FXObject *, FXSelector, void *)
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

FXbool DccTabItem::processLine(const FXString& commandtext)
{
    FXString command = (commandtext[0] == '/' ? commandtext.before(' ') : "");
    // Aliases don't work on dcctabitem
    //if(!utils::instance().getAlias(command).empty())
    //{
        //FXString acommand = utils::instance().getAlias(command);
        //if(acommand.contains("%s"))
            //acommand.substitute("%s", commandtext.after(' '));
        //else
            //acommand += commandtext.after(' ');
        //FXint num = acommand.contains("&&");
        //if(num)
        //{
            //FXbool result = FALSE;
            //for(FXint i=0; i<=acommand.contains('&'); i++)
            //{
                //if(!acommand.section('&',i).trim().empty()) result = processCommand(acommand.section('&',i).trim());
            //}
            //return result;
        //}
        //else
        //{
            //return processCommand(acommand);
        //}
    //}
    return processCommand(commandtext);
}

FXbool DccTabItem::processCommand(const FXString& commandtext)
{
    FXString command = (commandtext[0] == '/' ? commandtext.after('/').before(' ').lower() : "");
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
        appendIrcStyledText(utils::instance().availableDccCommands(), 3, FXSystem::now(), FALSE, FALSE);
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
        if(m_socket->isConnected())
        {
            appendIrcStyledText(getNickName()+" "+commandtext.after(' '), 2, FXSystem::now());
            return sendLine("\001ACTION "+commandtext.after(' ')+"\001\n");
        }
        else
        {
            appendIrcStyledText(_("You aren't connected"), 4, FXSystem::now(), FALSE, FALSE);
            return TRUE;
        }
    }
    if(command == "say")
    {
        if(m_socket->isConnected())
        {
            if(m_coloredNick) appendIrcNickText(getNickName(), commandtext.after(' '), getNickColor(getNickName()), FXSystem::now());
            else appendIrcNickText(getNickName(), commandtext.after(' '), 5, FXSystem::now());
            return sendLine(commandtext.after(' ')+"\n");
        }
        else
        {
            appendIrcStyledText(_("You aren't connected"), 4, FXSystem::now(), FALSE, FALSE);
            return TRUE;
        }
    }
    if(command == "tetris")
    {
        m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_NEWTETRIS), NULL);
        return TRUE;
    }
    if(commandtext[0] != '/')
    {
        if(m_socket->isConnected())
        {
            if(m_coloredNick) appendIrcNickText(getNickName(), commandtext, getNickColor(getNickName()), FXSystem::now());
            else appendIrcNickText(getNickName(), commandtext, 5, FXSystem::now());
            return sendLine(commandtext+"\n");
        }
        else
        {
            appendIrcStyledText(_("You aren't connected"), 4, FXSystem::now(), FALSE, FALSE);
            return TRUE;
        }
    }
    return FALSE;
}

FXbool DccTabItem::showHelp(FXString command)
{
    if(utils::instance().isScriptCommand(command))
    {
        appendIrcStyledText(utils::instance().getHelpText(command), 3, FXSystem::now(), FALSE, FALSE);
        return TRUE;
    }
    if(command == "commands")
    {
        appendIrcStyledText(_("COMMANDS, shows available commands"), 3, FXSystem::now(), FALSE, FALSE);
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
    if(command == "say")
    {
        appendIrcStyledText(_("SAY [text], sends text to current tab."), 3, FXSystem::now(), FALSE, FALSE);
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
    if(!utils::instance().getAlias(command[0] == '/' ? command:"/"+command).empty())
    {
        appendIrcStyledText(FXStringFormat("%s: %s", command.upper().text(), utils::instance().getAlias(command[0] == '/' ? command:"/"+command).text()), 3, FXSystem::now(), FALSE, FALSE);
        return TRUE;
    }
    if(command.empty()) appendIrcStyledText(_("Command is empty, type /commands for available commands"), 4, FXSystem::now(), FALSE, FALSE);
    else appendIrcStyledText(FXStringFormat(_("Unknown command '%s', type /commands for available commands"), command.text()), 4, FXSystem::now(), FALSE, FALSE);
    return FALSE;
}

long DccTabItem::onKeyPress(FXObject *, FXSelector, void *ptr)
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
                    for(FXint i = 0; i < utils::instance().dcccommandsNo(); i++)
                    {
                        if(comparecase(line.after('/').before(' '), utils::instance().dcccommandsAt(i)) == 0)
                        {
                            if((i+1) < utils::instance().dcccommandsNo()) m_commandline->setText("/"+utils::instance().dcccommandsAt(++i)+" ");
                            else m_commandline->setText("/"+utils::instance().dcccommandsAt(0)+" ");
                            break;
                        }
                        else if(comparecase(line.after('/'), utils::instance().dcccommandsAt(i).left(line.after('/').length())) == 0)
                        {
                            m_commandline->setText("/"+utils::instance().dcccommandsAt(i)+" ");
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
//            case KEY_k:
//            case KEY_K:
//                if(event->state&CONTROLMASK)
//                {
//                    FXint pos = m_commandline->getCursorPos();
//                    m_commandline->setText(line.insert(pos, "^C")); //color
//                    m_commandline->setCursorPos(pos+2);
//                    return 1;
//                }
//            case KEY_b:
//            case KEY_B:
//                if(event->state&CONTROLMASK)
//                {
//                    FXint pos = m_commandline->getCursorPos();
//                    m_commandline->setText(line.insert(pos, "^B")); //bold
//                    m_commandline->setCursorPos(pos+2);
//                    return 1;
//                }
//            case KEY_i:
//            case KEY_I:
//                if(event->state&CONTROLMASK)
//                {
//                    FXint pos = m_commandline->getCursorPos();
//                    m_commandline->setText(line.insert(pos, "^_")); //underline
//                    m_commandline->setCursorPos(pos+2);
//                    return 1;
//                }
//            case KEY_r:
//            case KEY_R:
//                if(event->state&CONTROLMASK)
//                {
//                    FXint pos = m_commandline->getCursorPos();
//                    m_commandline->setText(line.insert(pos, "^V")); //reverse
//                    m_commandline->setCursorPos(pos+2);
//                    return 1;
//                }
//            case KEY_o:
//            case KEY_O:
//                if(event->state&CONTROLMASK)
//                {
//                    FXint pos = m_commandline->getCursorPos();
//                    m_commandline->setText(line.insert(m_commandline->getCursorPos(), "^O")); //reset
//                    m_commandline->setCursorPos(pos+2);
//                    return 1;
//                }
        }
    }
    return 0;
}

long DccTabItem::onPipe(FXObject*, FXSelector, void *ptr)
{
    if(!m_socket->isConnected())
    {
        m_sendPipe = FALSE;
        m_pipeStrings.clear();
        m_pipe->stopCmd();
        return 1;
    }
    FXString text = *(FXString*)ptr;    
    if(m_sendPipe)
    {
        if(!getApp()->hasTimeout(this, ID_PTIME)) getApp()->addTimeout(this, ID_PTIME);
        m_pipeStrings.append(text);
    }
    else appendIrcText(text, FXSystem::now());
    return 1;
}

long DccTabItem::onIrcEvent(FXObject *, FXSelector, void *data)
{
    IrcEvent *ev = (IrcEvent *)data;
    if(ev->eventType == IRC_NICK)
    {
        if(m_mynick==ev->param1)
        {
            m_mynick = ev->param2;
            appendIrcStyledText(FXStringFormat(_("You're now known as %s"), m_mynick.text()), 1, ev->time, FALSE, FALSE);
        }
        if(m_nick==ev->param1)
        {
            m_nick = ev->param2;
            appendIrcStyledText(FXStringFormat(_("%s is now known as %s"), ev->param1.text(), ev->param2.text()), 1, ev->time, FALSE, FALSE);
            setText(m_nick);
            stopLogging();
        }
        return 1;
    }
    return 1;
}

long DccTabItem::onPipeTimeout(FXObject*, FXSelector, void*)
{
    if(m_pipeStrings.no() > 3)
    {
        if(m_pipeStrings[0].empty()) m_pipeStrings[0] = " ";
        if(m_coloredNick) appendIrcNickText(getNickName(), m_pipeStrings[0], getNickColor(getNickName()), FXSystem::now());
        else appendIrcNickText(getNickName(), m_pipeStrings[0], 5, FXSystem::now());
        sendLine(m_pipeStrings[0]+"\n");
        m_pipeStrings.erase(0);
        getApp()->addTimeout(this, ID_PTIME, 3000);
    }
    else
    {
        while(m_pipeStrings.no())
        {
            if(m_pipeStrings[0].empty()) m_pipeStrings[0] = " ";
            if(m_coloredNick) appendIrcNickText(getNickName(), m_pipeStrings[0], getNickColor(getNickName()), FXSystem::now());
            else appendIrcNickText(getNickName(), m_pipeStrings[0], 5, FXSystem::now());
            sendLine(m_pipeStrings[0]+"\n");
            m_pipeStrings.erase(0);
        }
    }
    return 1;
}

//for offered connectin to me
long DccTabItem::onCloseTimeout(FXObject*, FXSelector, void*)
{
    if(!m_socket->isConnected())
    {
        appendIrcStyledText(_("Connection closed. Client didn't connect in given timeout"), 4, FXSystem::now(), TRUE, FALSE);
        m_socket->disconnect();
    }
    return 1;
}

long DccTabItem::onTextLink(FXObject *, FXSelector, void *data)
{
    utils::instance().launchLink(static_cast<FXchar*>(data));
    return 1;
}

//handle change in spellLang combobox
long DccTabItem::onSpellLang(FXObject*, FXSelector, void*)
{
    m_commandline->setLanguage(m_spellLangs->getItemText(m_spellLangs->getCurrentItem()));
    m_commandline->setTipText(FXStringFormat(_("Current spellchecking language: %s"),m_spellLangs->getItemText(m_spellLangs->getCurrentItem()).text()));
    return 1;
}

FXString DccTabItem::stripColors(const FXString &text, const FXbool stripOther)
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

FXint DccTabItem::getNickColor(const FXString &nick)
{
    //10 is first colored nick style
    return 10+nick.hash()%8;
}

FXColor DccTabItem::getIrcColor(FXint code)
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

FXint DccTabItem::hiliteStyleExist(FXColor foreColor, FXColor backColor, FXuint style)
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

void DccTabItem::createHiliteStyle(FXColor foreColor, FXColor backColor, FXuint style)
{
    dxHiliteStyle nstyle = {foreColor,backColor,getApp()->getSelforeColor(),getApp()->getSelbackColor(),style,FALSE};
    m_textStyleList.append(nstyle);
    m_text->setHiliteStyles(m_textStyleList.data());
}

dxStringArray DccTabItem::cutText(FXString text, FXint len)
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

void DccTabItem::setCommandFocus()
{
    m_commandline->setFocus();
}

//for "handle" checking, if script contains "all". Send from dxirc.
void DccTabItem::hasAllCommand(FXbool result)
{
    m_scriptHasAll = result;
}

//for "handle" checking, if script contains "mymsg". Send from dxirc.
void DccTabItem::hasMyMsg(FXbool result)
{
    m_scriptHasMyMsg = result;
}

//check need of highlight in msg
FXbool DccTabItem::highlightNeeded(const FXString &msg)
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

FXbool DccTabItem::sendLine(const FXString& line)
{
    utils::instance().debugLine(FXStringFormat(">> %s", line.before('\n').text()));
    FXint size=0;
    if(m_socket->isConnected())
    {
        size = m_socket->write(line.text(), line.length());
        if(size != -1) return TRUE;
        else return FALSE;
    }
    else return FALSE;
}

void DccTabItem::writeIrcEvent(IrcEvent event)
{
    switch(event.eventType) {
        case IRC_DCCMSG:
        {
            FXbool needHighlight = FALSE;
            if(event.param1.contains(m_mynick))
                needHighlight = highlightNeeded(event.param1);
            if(needHighlight) appendIrcStyledText(m_nick+": "+event.param1, 8, event.time);
            else appendIrcNickText(m_nick, event.param1, m_coloredNick?getNickColor(m_nick):5, event.time);
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
        }break;
        case IRC_DCCACTION:
        {
            appendIrcStyledText(m_nick+" "+event.param1, 2, event.time);
            if(m_highlightColor != this->getTextColor() && m_parent->getCurrent()*2 != m_parent->indexOfChild(this))
            {
                if(event.param1.contains(m_mynick) && highlightNeeded(event.param1))
                {
                    this->setTextColor(m_highlightColor);
                }
                else this->setTextColor(m_unreadColor);
                this->setIcon(ICO_DCCNEWMSG);
                m_parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_NEWMSG), NULL);
            }
        }break;
        case IRC_CONNECT:
        {
            appendIrcStyledText(event.param1, 3, event.time, TRUE, FALSE);
        }break;
        case IRC_DISCONNECT:
        case IRC_SERVERERROR:
        {
            appendIrcStyledText(event.param1, 4, event.time, TRUE, FALSE);
            if(m_highlightColor != this->getTextColor() && m_parent->getCurrent()*2 != m_parent->indexOfChild(this)) this->setTextColor(m_unreadColor);
        }break;
        default:
            break;
    }
}

long DccTabItem::onSocketCanRead(FXObject*, FXSelector, void*)
{
    FXchar buffer[1024];
    FXString data = m_receiveRest;
    FXint size = 0;
    while((size = m_socket->read(buffer, 1023)) > 0)
    {
        buffer[size] = '\0';
        if(utils::instance().isUtf8(buffer, size)) data.append(buffer);
        else data.append(utils::instance().localeToUtf8(buffer));
        while(data.contains('\n'))
        {
            IrcEvent ev;
            ev.param1 = data.before('\n').before('\r');
            ev.time = FXSystem::now();
            if(ev.param1.contains('\001'))
            {
                ev.eventType = IRC_DCCACTION;
                ev.param1 = ev.param1.after('\001').before('\001').after(' ');
            }
            else
                ev.eventType = IRC_DCCMSG;
            utils::instance().debugLine(FXStringFormat("<< %s",ev.param1.text()));
            writeIrcEvent(ev);
            data = data.after('\n');
        }
        m_receiveRest = data;
    }
    return 1;
}

long DccTabItem::onSocketConnected(FXObject*, FXSelector, void*)
{
    IrcEvent ev;
    ev.eventType = IRC_CONNECT;
    ev.param1 = FXStringFormat(_("Connected to %s"), m_address.text());
    ev.time = FXSystem::now();
    writeIrcEvent(ev);
    return 1;
}

long DccTabItem::onSocketDisconnected(FXObject*, FXSelector, void*)
{
    m_sendPipe = FALSE;
    m_pipeStrings.clear();
    m_pipe->stopCmd();
    IrcEvent ev;
    ev.eventType = IRC_DISCONNECT;
    ev.param1 = FXStringFormat(_("Server %s was disconnected"), m_address.text());
    ev.time = FXSystem::now();
    writeIrcEvent(ev);
    return 1;
}

long DccTabItem::onSocketError(FXObject*, FXSelector, void *ptr)
{
    m_sendPipe = FALSE;
    m_pipeStrings.clear();
    m_pipe->stopCmd();
    SocketError *err = NULL;
    if(ptr) err = *((SocketError**)ptr);
    IrcEvent ev;
    ev.eventType = IRC_SERVERERROR;
    ev.time = FXSystem::now();
    if(err)
    {
        if(!err->errorStr.empty())
        {
            ev.param1 = err->errorStr;
            writeIrcEvent(ev);
        }
        switch(err->errorType){
            case UNABLEINIT:
            {
                ev.param1 = _("Unable to initiliaze socket");
                writeIrcEvent(ev);
                break;
            }
            case BADHOST:
            {
                ev.param1 = FXStringFormat(_("Bad host: %s"), m_address.text());
                writeIrcEvent(ev);
                break;
            }
            case UNABLECREATE:
            {
                ev.param1 = _("Unable to create socket");
                writeIrcEvent(ev);
                break;
            }
            case UNABLECONNECT:
            {
                ev.param1 = FXStringFormat(_("Unable to connect to: %s"), m_address.text());
                writeIrcEvent(ev);
                break;
            }
            case SSLUNCREATE:
            {
                ev.param1 = _("SSL creation error");
                writeIrcEvent(ev);
                break;
            }
            case SSLCONNECTERROR:
            {
                ev.param1 = _("SSL connect error");
                writeIrcEvent(ev);
                break;
            }
            case UNABLEBIND:
            {
                ev.param1 = _("Unable to bind socket");
                writeIrcEvent(ev);
                break;
            }
            case UNABLELISTEN:
            {
                ev.param1 = _("Unable to listen");
                writeIrcEvent(ev);
                break;
            }
            case UNABLEACCEPT:
            {
                ev.param1 = _("Unable to accept connection");
                writeIrcEvent(ev);
                break;
            }
            case UNABLESEND:
            {
                ev.param1 = _("Unable to send data");
                writeIrcEvent(ev);
                break;
            }
            case UNABLEREAD:
            {
                ev.param1 = FXStringFormat(_("Error in reading data from %s"), m_address.text());
                writeIrcEvent(ev);
                break;
            }
            case SSLZERO:
            {
                ev.param1 = _("SSL_read() returns zero - closing socket");
                writeIrcEvent(ev);
                break;
            }
            case SSLUNABLEREAD:
            {
                ev.param1 = _("SSL read problem");
                writeIrcEvent(ev);
                break;
            }
            case SSLABNORMAL:
            {
                ev.param1 = _("Abnormal value from SSL read");
                writeIrcEvent(ev);
                break;
            }
            case UNABLEREADBUFFER:
            {
                ev.param1 = _("Unable read data from buffer");
                writeIrcEvent(ev);
                break;
            }
            default:
                break;
        }
        delete err;
    }
    return 1;
}

long DccTabItem::onSocketStartAccept(FXObject*, FXSelector, void*)
{
    m_portD = m_socket->getPort();
    if(m_engine && m_listen) m_engine->sendCtcp(m_nick, "DCC CHAT chat "+m_socket->stringIPToBinary(m_address)+" "+FXStringVal(m_portD));
    getApp()->addTimeout(this, ID_CTIME, m_dccTimeout*1000);
    return 1;
}

long DccTabItem::onSocketListen(FXObject*, FXSelector, void*)
{
    IrcEvent ev;
    ev.eventType = IRC_CONNECT;
    ev.param1 = FXStringFormat(_("Someone connected from %s"), m_socket->getRemoteIP().text());
    ev.time = FXSystem::now();
    writeIrcEvent(ev);
    getApp()->removeTimeout(this, ID_CTIME);
    return 1;
}
