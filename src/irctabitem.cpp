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

#include "irctabitem.h"
#include "icons.h"
#include "config.h"
#include "i18n.h"

FXDEFMAP(dxText) dxTextMap[] = {
    FXMAPFUNC(SEL_MOTION, 0, dxText::onMotion)
};

FXIMPLEMENT(dxText, FXText, dxTextMap, ARRAYNUMBER(dxTextMap))

dxText::dxText(FXComposite *p, FXObject* tgt, FXSelector sel, FXuint opts)
    : FXText(p, tgt, sel, opts)
{

}

static inline FXint fxabs(FXint a) { return a<0?-a:a; }

long dxText::onMotion(FXObject*, FXSelector, void*ptr)
{
    FXEvent* event = (FXEvent*) ptr;
    FXint pos;
    FXint style = getStyle(getPosAt(event->win_x, event->win_y));
    if (style == 9)
        setDefaultCursor(getApp()->getDefaultCursor(DEF_HAND_CURSOR));
    else
        setDefaultCursor(getApp()->getDefaultCursor(DEF_TEXT_CURSOR));
    switch (mode)
    {
    case MOUSE_CHARS:
        if (startAutoScroll(event, FALSE)) return 1;
        if ((fxabs(event->win_x - event->click_x) > getApp()->getDragDelta()) || (fxabs(event->win_y - event->click_y) > getApp()->getDragDelta()))
        {
            pos = getPosAt(event->win_x, event->win_y);
            setCursorPos(pos, TRUE);
            extendSelection(cursorpos, SELECT_CHARS, TRUE);
        }
        return 1;
    case MOUSE_WORDS:
        if (startAutoScroll(event, FALSE)) return 1;
        if ((fxabs(event->win_x - event->click_x) > getApp()->getDragDelta()) || (fxabs(event->win_y - event->click_y) > getApp()->getDragDelta()))
        {
            pos = getPosAt(event->win_x, event->win_y);
            setCursorPos(pos, TRUE);
            extendSelection(cursorpos, SELECT_WORDS, TRUE);
        }
        return 1;
    case MOUSE_LINES:
        if (startAutoScroll(event, FALSE)) return 1;
        if ((fxabs(event->win_x - event->click_x) > getApp()->getDragDelta()) || (fxabs(event->win_y - event->click_y) > getApp()->getDragDelta()))
        {
            pos = getPosAt(event->win_x, event->win_y);
            setCursorPos(pos, TRUE);
            extendSelection(cursorpos, SELECT_LINES, TRUE);
        }
        return 1;
    case MOUSE_SCROLL:
        setPosition(event->win_x - grabx, event->win_y - graby);
        return 1;
    case MOUSE_DRAG:
        handle(this, FXSEL(SEL_DRAGGED, 0), ptr);
        return 1;
    case MOUSE_TRYDRAG:
        if (event->moved)
        {
            mode = MOUSE_NONE;
            if (handle(this, FXSEL(SEL_BEGINDRAG, 0), ptr))
            {
                mode = MOUSE_DRAG;
            }
        }
        return 1;
    }
    return 0;
}

FXDEFMAP(IrcTabItem) IrcTabItemMap[] = {
    FXMAPFUNC(SEL_COMMAND,              IrcTabItem::ID_COMMANDLINE,     IrcTabItem::OnCommandline),
    FXMAPFUNC(SEL_KEYPRESS,             IrcTabItem::ID_COMMANDLINE,     IrcTabItem::OnKeyPress),
    FXMAPFUNC(SEL_COMMAND,              IrcSocket::ID_SERVER,           IrcTabItem::OnIrcEvent),
    FXMAPFUNC(SEL_TIMEOUT,              IrcTabItem::ID_TIME,            IrcTabItem::OnTimeout),
    FXMAPFUNC(SEL_TIMEOUT,              IrcTabItem::ID_PTIME,           IrcTabItem::OnPipeTimeout),
    FXMAPFUNC(SEL_LEFTBUTTONRELEASE,    IrcTabItem::ID_TEXT,            IrcTabItem::OnLeftMouse),
    FXMAPFUNC(SEL_RIGHTBUTTONRELEASE,   IrcTabItem::ID_USERS,           IrcTabItem::OnRightMouse),
    FXMAPFUNC(SEL_COMMAND,              IrcTabItem::ID_NEWQUERY,        IrcTabItem::OnNewQuery),
    FXMAPFUNC(SEL_COMMAND,              IrcTabItem::ID_WHOIS,           IrcTabItem::OnWhois),
    FXMAPFUNC(SEL_COMMAND,              IrcTabItem::ID_OP,              IrcTabItem::OnOp),
    FXMAPFUNC(SEL_COMMAND,              IrcTabItem::ID_DEOP,            IrcTabItem::OnDeop),
    FXMAPFUNC(SEL_COMMAND,              IrcTabItem::ID_VOICE,           IrcTabItem::OnVoice),
    FXMAPFUNC(SEL_COMMAND,              IrcTabItem::ID_DEVOICE,         IrcTabItem::OnDevoice),
    FXMAPFUNC(SEL_COMMAND,              IrcTabItem::ID_KICK,            IrcTabItem::OnKick),
    FXMAPFUNC(SEL_COMMAND,              IrcTabItem::ID_BAN,             IrcTabItem::OnBan),
    FXMAPFUNC(SEL_COMMAND,              IrcTabItem::ID_KICKBAN,         IrcTabItem::OnKickban),
    FXMAPFUNC(SEL_COMMAND,              IrcTabItem::ID_TOPIC,           IrcTabItem::OnTopic),
    FXMAPFUNC(SEL_COMMAND,              dxPipe::ID_PIPE,                IrcTabItem::OnPipe),
};

FXIMPLEMENT(IrcTabItem, FXTabItem, IrcTabItemMap, ARRAYNUMBER(IrcTabItemMap))

IrcTabItem::IrcTabItem(FXTabBook *tab, const FXString &tabtext, FXIcon *ic=0, FXuint opts=TAB_TOP_NORMAL, TYPE typ=CHANNEL, IrcSocket *sock=NULL, FXbool oswnd=false, FXbool uswn=true, FXbool logg=false, FXString cmdlst="", FXString lpth="", FXint maxa=200, IrcColor clrs=IrcColor(), FXString nichar=":", FXFont *fnt=NULL, FXbool scmd=false, FXbool slst=false, FXbool cnick=false)
    : FXTabItem(tab, tabtext, ic, opts), parent(tab), server(sock), type(typ), usersShown(uswn), logging(logg), ownServerWindow(oswnd), sameCmd(scmd), sameList(slst), coloredNick(cnick), colors(clrs),
    commandsList(cmdlst), logPath(lpth), maxAway(maxa), nickCompletionChar(nichar), logstream(NULL)
{
    currentPosition = 0;
    historyMax = 25;
    numberUsers = 0;
    maxLen = 460;
    checkAway = false;
    iamOp = false;
    topic = _("No topic is set");
    editableTopic = true;
    pipe = NULL;
    sendPipe = false;

    if(type == CHANNEL && server->GetConnected())
    {
        server->SendMode(getText());
    }

    mainframe = new FXVerticalFrame(parent, FRAME_RAISED|LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y);

    splitter = new FXSplitter(mainframe, LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y|SPLITTER_REVERSED|SPLITTER_TRACKING);

    textframe = new FXVerticalFrame(splitter, FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    topicline = new FXTextField(textframe, 50, this, ID_TOPIC, FRAME_SUNKEN|TEXTFIELD_ENTER_ONLY|JUSTIFY_LEFT|LAYOUT_FILL_X);
    topicline->setText(topic);
    if(type != CHANNEL)
    {
        topicline->hide();
    }
    topicline->setFont(fnt);
    text = new dxText(textframe, this, ID_TEXT, FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y|TEXT_READONLY|TEXT_WORDWRAP|TEXT_SHOWACTIVE|TEXT_AUTOSCROLL);
    text->setFont(fnt);

    usersframe = new FXVerticalFrame(splitter, FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_FIX_WIDTH);
    users = new FXList(usersframe, this, ID_USERS, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    users->setSortFunc(FXList::ascendingCase);
    users->setScrollStyle(HSCROLLING_OFF);
    if(sameList) users->setFont(fnt);
    if(type != CHANNEL || !usersShown)
    {
        usersframe->hide();
        users->hide();
    }

    commandline = new FXTextField(mainframe, 25, this, ID_COMMANDLINE, TEXTFIELD_ENTER_ONLY|FRAME_SUNKEN|JUSTIFY_LEFT|LAYOUT_FILL_X|LAYOUT_BOTTOM, 0, 0, 0, 0, 1, 1, 1, 1);
    if(sameCmd) commandline->setFont(fnt);

    for(int i=0; i<17; i++)
    {
        textStyleList[i].normalForeColor = colors.text;
        textStyleList[i].normalBackColor = colors.back;
        textStyleList[i].selectForeColor = getApp()->getSelforeColor();
        textStyleList[i].selectBackColor = getApp()->getSelbackColor();
        textStyleList[i].hiliteForeColor = getApp()->getHiliteColor();
        textStyleList[i].hiliteBackColor = FXRGB(255, 128, 128); // from FXText.cpp
        textStyleList[i].activeBackColor = colors.back;
        textStyleList[i].style = 0;
    }
    //gray text - user commands
    textStyleList[0].normalForeColor = colors.user;
    //orange text - Actions
    textStyleList[1].normalForeColor = colors.action;
    //blue text - Notice
    textStyleList[2].normalForeColor = colors.notice;
    //red text - Errors
    textStyleList[3].normalForeColor = colors.error;
    //bold style
    textStyleList[4].style = FXText::STYLE_BOLD;
    //underline style
    textStyleList[5].style = FXText::STYLE_UNDERLINE;
    //bold & underline
    textStyleList[6].style = FXText::STYLE_UNDERLINE;
    textStyleList[6].style ^=FXText::STYLE_BOLD;
    //highlight text
    textStyleList[7].normalForeColor = colors.hilight;
    //link style
    textStyleList[8].normalForeColor = colors.link;
    textStyleList[8].style = FXText::STYLE_UNDERLINE;
    //next styles for colored nicks
    textStyleList[9].normalForeColor = FXRGB(196, 160, 0);
    textStyleList[10].normalForeColor = FXRGB(206, 92, 0);
    textStyleList[11].normalForeColor = FXRGB(143, 89, 2);
    textStyleList[12].normalForeColor = FXRGB(78, 154, 6);
    textStyleList[13].normalForeColor = FXRGB(32, 74, 135);
    textStyleList[14].normalForeColor = FXRGB(117, 80, 123);
    textStyleList[15].normalForeColor = FXRGB(164, 0, 0);
    textStyleList[16].normalForeColor = FXRGB(85, 87, 83);

    text->setStyled(TRUE);
    text->setHiliteStyles(textStyleList);

    text->setBackColor(colors.back);
    text->setActiveBackColor(colors.back);
    commandline->setBackColor(colors.back);
    topicline->setBackColor(colors.back);
    users->setBackColor(colors.back);
    text->setTextColor(colors.text);
    commandline->setTextColor(colors.text);
    commandline->setCursorColor(colors.text);
    topicline->setTextColor(colors.text);
    topicline->setCursorColor(colors.text);
    users->setTextColor(colors.text);

    this->setIconPosition(ICON_BEFORE_TEXT);
}

IrcTabItem::~IrcTabItem()
{
    this->StopLogging();
    if(pipe) pipe->StopCmd();
    pipeStrings.clear();
    getApp()->removeTimeout(this, ID_TIME);
    getApp()->removeTimeout(this, ID_PTIME);
}

void IrcTabItem::CreateGeom()
{
    mainframe->create();
    splitter->create();
    textframe->create();
    text->create();
    usersframe->create();
    users->create();
    commandline->create();
}

void IrcTabItem::ClearChat()
{
    text->removeText(0, text->getLength());
}

void IrcTabItem::ReparentTab()
{
    reparent(parent);
    mainframe->reparent(parent);
}

void IrcTabItem::HideUsers()
{
    usersShown = !usersShown;
    if(type == CHANNEL)
    {
        usersframe->hide();
        users->hide();
        splitter->setSplit(1, 0);
    }
}

void IrcTabItem::ShowUsers()
{
    usersShown = !usersShown;
    if(type == CHANNEL)
    {
        usersframe->show();
        users->show();
        splitter->recalc();
    }
}

void IrcTabItem::SetType(const TYPE &typ, const FXString &tabtext)
{
    if(typ == CHANNEL)
    {
        if(usersShown) usersframe->show();
        if(usersShown) users->show();        
        topicline->show();
        topicline->setText(topic);
        splitter->recalc();
        setText(tabtext);
        if(server->GetConnected()) server->SendMode(getText());
        type = typ;
    }
    else if(typ == SERVER || typ == QUERY)
    {
        usersframe->hide();
        users->hide();
        topicline->setText("");
        topicline->hide();
        topic = _("No topic is set");
        setText(tabtext);
        splitter->setSplit(1, 0);
        if(type == CHANNEL)
        {
            users->clearItems();
            numberUsers = 0;
        }
        type = typ;
    }
    this->StopLogging();
    if(type == SERVER) this->setIcon(servericon);
    else if(type == CHANNEL) this->setIcon(channelicon);
    else this->setIcon(queryicon);
}

void IrcTabItem::SetColor(IrcColor clrs)
{
    colors = clrs;
    SetTextColor(clrs.text);
    SetTextBackColor(clrs.back);
    SetUserColor(clrs.user);
    SetActionsColor(clrs.action);
    SetNoticeColor(clrs.notice);
    SetErrorColor(clrs.error);
    SetHilightColor(clrs.hilight);
    SetLinkColor(colors.link);
}

void IrcTabItem::SetTextBackColor(FXColor clr)
{
    for(int i=0; i<17; i++)
    {
        textStyleList[i].normalBackColor = clr;
        textStyleList[i].activeBackColor = clr;
    }
    text->setBackColor(clr);
    text->setActiveBackColor(clr);
    commandline->setBackColor(clr);
    topicline->setBackColor(clr);
    users->setBackColor(clr);
}

void IrcTabItem::SetTextColor(FXColor clr)
{
    textStyleList[4].normalForeColor = clr;
    textStyleList[5].normalForeColor = clr;
    textStyleList[6].normalForeColor = clr;
    text->setTextColor(clr);
    commandline->setTextColor(clr);
    commandline->setCursorColor(clr);
    topicline->setTextColor(clr);
    topicline->setCursorColor(clr);
    users->setTextColor(clr);
}

void IrcTabItem::SetUserColor(FXColor clr)
{
    textStyleList[0].normalForeColor = clr;
}

void IrcTabItem::SetActionsColor(FXColor clr)
{
    textStyleList[1].normalForeColor = clr;
}

void IrcTabItem::SetNoticeColor(FXColor clr)
{
    textStyleList[2].normalForeColor = clr;
}

void IrcTabItem::SetErrorColor(FXColor clr)
{
    textStyleList[3].normalForeColor = clr;
}

void IrcTabItem::SetHilightColor(FXColor clr)
{
    textStyleList[7].normalForeColor = clr;
}

void IrcTabItem::SetLinkColor(FXColor clr)
{
    textStyleList[8].normalForeColor = clr;
}

void IrcTabItem::SetCommandsList(FXString clst)
{
    commandsList = clst;
}

void IrcTabItem::SetMaxAway(FXint maxa)
{
    maxAway = maxa;
}

void IrcTabItem::SetLogging(FXbool log)
{
    logging = log;
}

void IrcTabItem::SetLogPath(FXString pth)
{
    logPath = pth;
    this->StopLogging();
}

void IrcTabItem::SetNickCompletionChar(FXString nichr)
{
    nickCompletionChar = nichr;
}

void IrcTabItem::SetIrcFont(FXFont *fnt)
{
    text->setFont(fnt);
    topicline->setFont(fnt);
    if(sameCmd) commandline->setFont(fnt);
    else commandline->setFont(getApp()->getNormalFont());
    if(sameList) users->setFont(fnt);
    else users->setFont(getApp()->getNormalFont());
}

void IrcTabItem::SetSameCmd(FXbool scmd)
{
    sameCmd = scmd;
}

void IrcTabItem::SetSameList(FXbool slst)
{
    sameList = slst;
}

void IrcTabItem::SetColoredNick(FXbool cnick)
{
    coloredNick = cnick;
}

void IrcTabItem::AppendIrcText(FXString msg)
{
    text->appendText("["+FXSystem::time("%H:%M:%S", FXSystem::now()) +"] ");
    if(msg.right(1) != " ") msg.append(" ");
    for(FXint i=0; i<msg.contains(' '); i++)
    {
        FXString sec = msg.section(' ',i);
        FXRex rxl("\\L");
        if(rxl.match(sec.left(1)) && ((comparecase(sec.mid(1,7), "http://")==0 && sec.length()>7)
                || (comparecase(sec.mid(1,8), "https://")==0 && sec.length()>8)
                || (comparecase(sec.mid(1,6), "ftp://")==0 && sec.length()>6)
                || (comparecase(sec.mid(1,4), "www.")==0 && sec.length()>4)))
        {
            text->appendText(sec.left(1));
            FXRex rxr("\\W");
            if(rxr.match(sec.right(1)))
            {
                text->appendStyledText(sec.mid(1,sec.length()-2),9);
                text->appendText(sec.right(1));
            }
            else
            {
                text->appendStyledText(sec.mid(1,sec.length()-1),9);
            }
            text->appendText(" ");
            continue;
        }
        if((comparecase(sec.left(7), "http://")==0 && sec.length()>7)
                || (comparecase(sec.left(8), "https://")==0 && sec.length()>8)
                || (comparecase(sec.left(6), "ftp://")==0 && sec.length()>6)
                || (comparecase(sec.left(4), "www.")==0 && sec.length()>4))
        {
            text->appendStyledText(sec,9);
            text->appendText(" ");
            continue;
        }
        else
        {
            FXRex rx("(\\002|\\003|\\037)");
            if(rx.match(sec)) //contains mirc colors,styles
            {
                sec = StripColors(sec, false);
                FXbool bold = false;
                FXbool under = false;
                FXint i = 0;
                while(sec[i] != '\0')
                {
                    if(sec[i] == '\002')
                    {
                        bold = !bold;
                    }
                    else if(sec[i] == '\037')
                    {
                        under = !under;
                    }
                    else
                    {
                        FXString txt;
                        txt += sec[i];
                        if(bold && under) text->appendStyledText(txt, 7);
                        else if(bold && !under) text->appendStyledText(txt, 5);
                        else if(!bold && under) text->appendStyledText(txt, 6);
                        else text->appendText(txt);
                    }
                    i++;
                }
                text->appendText(" ");
            }
            else
            {
                text->appendText(sec+" ");
            }
        }
    }
    text->appendText("\n");
    MakeLastRowVisible(false);
    this->LogLine(StripColors(msg, true));
}

void IrcTabItem::AppendIrcNickText(FXString nick, FXString msg, FXint color)
{
    text->appendText("["+FXSystem::time("%H:%M:%S", FXSystem::now()) +"] ");
    text->appendStyledText(nick+": ", color);
    if(msg.right(1) != " ") msg.append(" ");
    for(FXint i=0; i<msg.contains(' '); i++)
    {
        FXString sec = msg.section(' ',i);
        FXRex rxl("\\L");
        if(rxl.match(sec.left(1)) && ((comparecase(sec.mid(1,7), "http://")==0 && sec.length()>7)
                || (comparecase(sec.mid(1,8), "https://")==0 && sec.length()>8)
                || (comparecase(sec.mid(1,6), "ftp://")==0 && sec.length()>6)
                || (comparecase(sec.mid(1,4), "www.")==0 && sec.length()>4)))
        {
            text->appendText(sec.left(1));
            FXRex rxr("\\W");
            if(rxr.match(sec.right(1)))
            {
                text->appendStyledText(sec.mid(1,sec.length()-2),9);
                text->appendText(sec.right(1));
            }
            else
            {
                text->appendStyledText(sec.mid(1,sec.length()-1),9);
            }
            text->appendText(" ");
            continue;
        }
        if((comparecase(sec.left(7), "http://")==0 && sec.length()>7)
                || (comparecase(sec.left(8), "https://")==0 && sec.length()>8)
                || (comparecase(sec.left(6), "ftp://")==0 && sec.length()>6)
                || (comparecase(sec.left(4), "www.")==0 && sec.length()>4))
        {
            text->appendStyledText(sec,9);
            text->appendText(" ");
            continue;
        }
        else
        {
            FXRex rx("(\\002|\\003|\\037)");
            if(rx.match(sec)) //contains mirc colors,styles
            {
                sec = StripColors(sec, false);
                FXbool bold = false;
                FXbool under = false;
                FXint i = 0;
                while(sec[i] != '\0')
                {
                    if(sec[i] == '\002')
                    {
                        bold = !bold;
                    }
                    else if(sec[i] == '\037')
                    {
                        under = !under;
                    }
                    else
                    {
                        FXString txt;
                        txt += sec[i];
                        if(bold && under) text->appendStyledText(txt, 7);
                        else if(bold && !under) text->appendStyledText(txt, 5);
                        else if(!bold && under) text->appendStyledText(txt, 6);
                        else text->appendText(txt);
                    }
                    i++;
                }
                text->appendText(" ");
            }
            else
            {
                text->appendText(sec+" ");
            }
        }
    }
    text->appendText("\n");
    MakeLastRowVisible(false);
    this->LogLine(StripColors("<"+nick+"> "+msg, true));
}

void IrcTabItem::AppendIrcStyledText(FXString styled, FXint stylenum)
{
    text->appendText("["+FXSystem::time("%H:%M:%S", FXSystem::now()) +"] ");
    if(styled.right(1) != " ") styled.append(" ");
    for(FXint i=0; i<styled.contains(' '); i++)
    {
        FXString sec = StripColors(styled.section(' ',i), true);
        FXRex rxl("\\L");
        if(rxl.match(sec.left(1)) && ((comparecase(sec.mid(1,7), "http://")==0 && sec.length()>7)
                || (comparecase(sec.mid(1,8), "https://")==0 && sec.length()>8)
                || (comparecase(sec.mid(1,6), "ftp://")==0 && sec.length()>6)
                || (comparecase(sec.mid(1,4), "www.")==0 && sec.length()>4)))
        {
            text->appendStyledText(sec.left(1), stylenum);
            FXRex rxr("\\W");
            if(rxr.match(sec.right(1)))
            {
                text->appendStyledText(sec.mid(1,sec.length()-2),9);
                text->appendStyledText(sec.right(1), stylenum);
            }
            else
            {
                text->appendStyledText(sec.mid(1,sec.length()-1),9);
            }
            text->appendText(" ");
            continue;
        }
        if((comparecase(sec.left(7), "http://")==0 && sec.length()>7)
                || (comparecase(sec.left(8), "https://")==0 && sec.length()>8)
                || (comparecase(sec.left(6), "ftp://")==0 && sec.length()>6)
                || (comparecase(sec.left(4), "www.")==0 && sec.length()>4))
        {
            text->appendStyledText(sec,9);
            text->appendText(" ");
            continue;
        }
        else
        {
            text->appendStyledText(sec+" ", stylenum);
        }
    }    
    text->appendText("\n");
    MakeLastRowVisible(false);
    this->LogLine(StripColors(styled, true));
}

void IrcTabItem::StartLogging()
{
    if(logstream && FXStat::exists(logPath+PATHSEPSTRING+server->GetServerName()+PATHSEPSTRING+getText()+PATHSEPSTRING+FXSystem::time("%Y-%m-%d", FXSystem::now()))) return;

    if(logging && type != SERVER)
    {
        if(!FXStat::exists(logPath+PATHSEPSTRING+server->GetServerName())) FXDir::create(logPath+PATHSEPSTRING+server->GetServerName());
        if(!FXStat::exists(logPath+PATHSEPSTRING+server->GetServerName()+PATHSEPSTRING+getText())) FXDir::create(logPath+PATHSEPSTRING+server->GetServerName()+PATHSEPSTRING+getText());
        logstream = new std::ofstream(FXString(logPath+PATHSEPSTRING+server->GetServerName()+PATHSEPSTRING+getText()+PATHSEPSTRING+FXSystem::time("%Y-%m-%d", FXSystem::now())).text(), std::ios::out|std::ios::app);
    }
}

void IrcTabItem::StopLogging()
{
    if(logstream)
    {
        logstream->close();
        delete logstream;
        logstream = NULL;
    }
}

void IrcTabItem::LogLine(const FXString &line)
{
    if(logging && type != SERVER)
    {
        this->StartLogging();
        *logstream << "[" << FXSystem::time("%H:%M:%S", FXSystem::now()).text() << "] " << line.text() << std::endl;
    }
}

FXbool IrcTabItem::IsChannel(const FXString &text)
{
    if(text.length()) return text[0] == '#' || text[0] == '&' || text[0] == '+' || text[0] == '!';
    return false;
}

void IrcTabItem::MakeLastRowVisible(FXbool force)
{
    if(force) text->makePositionVisible(text->rowStart(text->getLength()));
    else
    {
        FXScrollBar *textScrollbar = text->verticalScrollBar();
        if(parent->getCurrent()*2 == parent->indexOfChild(this))
        {
            if((textScrollbar->getPosition()+textScrollbar->getHeight()+textScrollbar->getLine())*100 > textScrollbar->getRange()*95)
            {
                text->makePositionVisible(text->rowStart(text->getLength()));
            }
        }
        else text->makePositionVisible(text->rowStart(text->getLength()));
    }
}

long IrcTabItem::OnCommandline(FXObject *, FXSelector, void *)
{
    FXString commandtext = commandline->getText();
    commandsHistory.append(commandtext);
    currentPosition = commandsHistory.no();
    if (currentPosition > historyMax)
    {
        commandsHistory.erase(0);
        --currentPosition;
    }
    commandline->setText("");
    FXString command = (commandtext[0] == '/' ? commandtext.before(' ') : "");
    if(!utils::GetAlias(command).empty())
    {
        FXString acommand = utils::GetAlias(command);        
        FXint num = acommand.contains('/');
        if(num>1 && utils::IsCommand(acommand.section('/',2).before(' ')))
        {
            for(FXint i=1; i<=num; i++)
            {
                ProcessCommand(acommand.section('/',i).prepend('/').trim());
            }
        }
        else
        {
            if(acommand.contains("%s")) ProcessCommand(acommand.substitute("%s", commandtext.after(' ')));
            else ProcessCommand(acommand + (command == commandtext? "" : " "+commandtext.after(' ')));
        }
    }
    else ProcessCommand(commandtext);
    return 1;
}

FXbool IrcTabItem::ProcessCommand(const FXString& commandtext)
{
    FXString command = (commandtext[0] == '/' ? commandtext.after('/').before(' ').lower() : "");
    if(server->GetConnected())
    {
        if(commandtext[0] == '/')
        {
            if(command == "admin")
            {
                return server->SendAdmin(commandtext.after(' '));
            }
            if(command == "away")
            {
                if(commandtext.after(' ').length() > server->GetAwayLen())
                {
                    AppendIrcStyledText(FXStringFormat(_("Away message is too long. Max. away message length is %d."), server->GetAwayLen()), 4);
                    return false;
                }
                else
                    return server->SendAway(commandtext.after(' '));
            }
            if(command == "banlist")
            {
                FXString channel = commandtext.after(' ');
                if(channel.empty() && type == CHANNEL) return server->SendBanlist(getText());
                else if(!IsChannel(channel) && type != CHANNEL)
                {
                    AppendIrcStyledText(_("/banlist <channel>, shows banlist for channel."), 4);
                    return false;
                }
                else return server->SendBanlist(channel);
            }
            if(command == "connect")
            {
                if(commandtext.after(' ').empty())
                {
                    AppendIrcStyledText(_("/connect <server> [port] [nick] [password] [realname] [channels], connects for given server."), 4);
                    return false;
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
                    parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CSERVER), &srv);
                    return true;
                }
            }
            if(command == "commands")
            {
                FXString commandstr = _("Available commnads: ");
                for(FXint i=0; i < utils::CommandsNo(); i++)
                {
                    if(utils::CommandsAt(i) != "commands") commandstr += utils::CommandsAt(i).upper()+(i != utils::CommandsNo() - 1? ", " : "");
                }
                AppendIrcStyledText(commandstr, 3);
                return true;
            }
            if(command == "ctcp")
            {
                FXString to = commandtext.after(' ').before(' ');
                FXString msg = commandtext.after(' ', 2);
                if(to.empty() || msg.empty())
                {
                    AppendIrcStyledText(_("/ctcp <nick> <message>, sends a CTCP message to a user."), 4);
                    return false;
                }
                else if(msg.length() > maxLen-12-to.length())
                {
                    AppendIrcStyledText(FXStringFormat(_("Ctcp message is too long. Max. ctcp message length is %d."), maxLen-12-to.length()), 4);
                    return false;
                }
                else return server->SendCtcp(to, msg);
            }
            if(command == "deop")
            {
                FXString params = commandtext.after(' ');
                if(type == CHANNEL)
                {
                    if(params.empty())
                    {
                        AppendIrcStyledText(_("/deop <nicks>, removes operator status from one or more nicks."), 4);
                        return false;
                    }
                    else if(IsChannel(params) && params.after(' ').empty())
                    {
                        AppendIrcStyledText(_("/deop <channel> <nicks>, removes operator status from one or more nicks."), 4);
                        return false;
                    }
                    else if(IsChannel(params) && !params.after(' ').empty())
                    {
                        FXString channel = params.before(' ');
                        FXString nicks = params.after(' ');
                        FXString modeparams = utils::CreateModes('-', 'o', nicks);
                        return server->SendMode(channel+" "+modeparams);
                    }
                    else
                    {
                        FXString channel = getText();
                        FXString nicks = params;
                        FXString modeparams = utils::CreateModes('-', 'o', nicks);
                        return server->SendMode(channel+" "+modeparams);
                    }
                }
                else
                {
                    if(IsChannel(params) && !params.after(' ').empty())
                    {
                        FXString channel = params.before(' ');
                        FXString nicks = params.after(' ');
                        FXString modeparams = utils::CreateModes('-', 'o', nicks);
                        return server->SendMode(channel+" "+modeparams);
                    }
                    else
                    {
                        AppendIrcStyledText(_("/deop <channel> <nicks>, removes operator status from one or more nicks."), 4);
                        return false;
                    }
                }
            }
            if(command == "devoice")
            {
                FXString params = commandtext.after(' ');
                if(type == CHANNEL)
                {
                    if(params.empty())
                    {
                        AppendIrcStyledText(_("/devoice <nicks>, removes voice from one or more nicks."), 4);
                        return false;
                    }
                    else if(IsChannel(params) && params.after(' ').empty())
                    {
                        AppendIrcStyledText(_("/devoice <channel> <nicks>, removes voice from one or more nicks."), 4);
                        return false;
                    }
                    else if(IsChannel(params) && !params.after(' ').empty())
                    {
                        FXString channel = params.before(' ');
                        FXString nicks = params.after(' ');
                        FXString modeparams = utils::CreateModes('-', 'v', nicks);
                        return server->SendMode(channel+" "+modeparams);
                    }
                    else
                    {
                        FXString channel = getText();
                        FXString nicks = params;
                        FXString modeparams = utils::CreateModes('-', 'v', nicks);
                        return server->SendMode(channel+" "+modeparams);
                    }
                }
                else
                {
                    if(IsChannel(params) && !params.after(' ').empty())
                    {
                        FXString channel = params.before(' ');
                        FXString nicks = params.after(' ');
                        FXString modeparams = utils::CreateModes('-', 'v', nicks);
                        return server->SendMode(channel+" "+modeparams);
                    }
                    else
                    {
                        AppendIrcStyledText(_("/devoice <channel> <nicks>, removes voice from one or more nicks."), 4);
                        return false;
                    }
                }
            }
            if(command == "egg")
            {
                AppendIrcStyledText("ahoj sem pan Vajíčko", 3);
                AppendIrcStyledText("místo abys mně házel po Paroubeggovi", 3);
                AppendIrcStyledText("udělej si ze mně dobrý volský oko", 3);
                return true;
            }
            if(command == "exec")
            {
                FXString params = commandtext.after(' ');
                if(params.empty())
                {
                    AppendIrcStyledText(_("/exec [-o|-c] <command>, executes command, -o sends output to channel/query, -c closes running command."), 4);
                    return false;
                }
                else
                {
                    if(!pipe) pipe = new dxPipe(getApp(), this);
                    pipeStrings.clear();
                    if(params.before(' ').contains("-o"))
                    {
                        sendPipe = true;
                        pipe->ExecCmd(params.after(' '));
                    }
                    else if(params.before(' ').contains("-c"))
                    {
                        sendPipe = false;
                        pipe->StopCmd();
                    }
                    else
                    {
                        sendPipe = false;
                        pipe->ExecCmd(params);                        
                    }
                    return true;
                }
            }
            if(command == "help")
            {
                return ShowHelp(commandtext.after(' ').lower());
            }
            if(command == "invite")
            {
                FXString params = commandtext.after(' ');
                if(params.empty())
                {
                    AppendIrcStyledText(_("/invite <nick> <channel>, invites someone to a channel."), 4);
                    return false;
                }
                else if(IsChannel(params) && params.after(' ').empty())
                {
                    AppendIrcStyledText(_("/invite <nick> <channel>, invites someone to a channel."), 4);
                    return false;
                }
                else
                {
                    FXString nick = params.before(' ');
                    FXString channel = params.after(' ');
                    return server->SendInvite(nick, channel);
                }
            }
            if(command == "join")
            {
                FXString channel = commandtext.after(' ');
                if(!IsChannel(channel))
                {
                    AppendIrcStyledText(_("/join <channel>, joins a channel."), 4);
                    return false;
                }
                else return server->SendJoin(channel);
            }
            if(command == "kick")
            {
                FXString params = commandtext.after(' ');
                if(type == CHANNEL)
                {
                    if(params.empty())
                    {
                        AppendIrcStyledText(_("/kick <nick>, kicks a user from a channel."), 4);
                        return false;
                    }
                    else if(IsChannel(params) && params.after(' ').empty())
                    {
                        AppendIrcStyledText(_("/kick <channel> <nick>, kicks a user from a channel."), 4);
                        return false;
                    }
                    else if(IsChannel(params) && !params.after(' ').empty())
                    {
                        FXString channel = params.before(' ');
                        FXString nick = params.after(' ');
                        FXString reason = params.after(' ', 2);
                        if(reason.length() > server->GetKickLen())
                        {
                            AppendIrcStyledText(FXStringFormat(_("Reason of kick is too long. Max. reason length is %d."), server->GetKickLen()), 4);
                            return false;
                        }
                        else return server->SendKick(channel, nick, reason);
                    }
                    else
                    {
                        FXString channel = getText();
                        FXString nick = params.before(' ');
                        FXString reason = params.after(' ');
                        if(reason.length() > server->GetKickLen())
                        {
                            AppendIrcStyledText(FXStringFormat(_("Reason of kick is too long. Max. reason length is %d."), server->GetKickLen()), 4);
                            return false;
                        }
                        else return server->SendKick(channel, nick, reason);
                    }
                }
                else
                {
                    if(IsChannel(params) && !params.after(' ').empty())
                    {
                        FXString channel = params.before(' ');
                        FXString nick = params.after(' ');
                        FXString reason = params.after(' ', 2);
                        if(reason.length() > server->GetKickLen())
                        {
                            AppendIrcStyledText(FXStringFormat(_("Reason of kick is too long. Max. reason length is %d."), server->GetKickLen()), 4);
                            return false;
                        }
                        else return server->SendKick(channel, nick, reason);
                    }
                    else
                    {
                        AppendIrcStyledText(_("/kick <channel> <nick>, kicks a user from a channel."), 4);
                        return false;
                    }
                }
            }
            if(command == "kill")
            {
                FXString params = commandtext.after(' ');
                FXString nick = params.before(' ');
                FXString reason = params.after(' ');
                if(params.empty())
                {
                    AppendIrcStyledText(_("/kill <user> [reason], kills a user from the network."), 4);
                    return false;
                }
                if(reason.length() > maxLen-7-nick.length())
                {
                    AppendIrcStyledText(FXStringFormat(_("Reason of kill is too long. Max. reason length is %d."), maxLen-7-nick.length()), 4);
                    return false;
                }
                else return server->SendKill(nick, reason);
            }
            if(command == "list")
            {
                return server->SendList(commandtext.after(' '));
            }
            if(command == "me")
            {
                FXString params = commandtext.after(' ');
                if(type == CHANNEL)
                {
                    if(params.empty())
                    {
                        AppendIrcStyledText(_("/me <message>, sends the action to the current channel."), 4);
                        return false;
                    }
                    else if(IsChannel(params) && !params.after(' ').empty())
                    {
                        FXString channel = params.before(' ');
                        FXString message = params.after(' ');
                        if(channel == getText()) AppendIrcStyledText(server->GetNickName()+" "+message, 2);
                        if(message.length() > maxLen-19-channel.length())
                        {
                            dxStringArray messages = CutText(message, maxLen-19-channel.length());
                            FXbool result = true;
                            for(FXint i=0; i<messages.no(); i++)
                            {
                                result = server->SendMe(channel, messages[i]) &result;
                            }
                            return result;
                        }
                        else return server->SendMe(channel, message);
                    }
                    else
                    {                        
                        AppendIrcStyledText(server->GetNickName()+" "+params, 2);
                        if(params.length() > maxLen-19-getText().length())
                        {
                            dxStringArray messages = CutText(params, maxLen-19-getText().length());
                            FXbool result = true;
                            for(FXint i=0; i<messages.no(); i++)
                            {
                                result = server->SendMe(getText(), messages[i]) &result;
                            }
                            return result;
                        }
                        else return server->SendMe(getText(), params);
                    }
                }
                else
                {
                    if(!params.after(' ').empty())
                    {
                        FXString to = params.before(' ');
                        FXString message = params.after(' ');
                        if(message.length() > maxLen-19-to.length())
                        {
                            dxStringArray messages = CutText(message, maxLen-19-to.length());
                            FXbool result = true;
                            for(FXint i=0; i<messages.no(); i++)
                            {
                                result = server->SendMe(to, messages[i]) &result;
                            }
                            return result;
                        }
                        else return server->SendMe(to, message);
                    }
                    else
                    {
                        AppendIrcStyledText(_("/me <to> <message>, sends the action."), 4);
                        return false;
                    }
                }
            }
            if(command == "mode")
            {
                FXString params = commandtext.after(' ');
                if(params.empty())
                {
                    AppendIrcStyledText(_("/mode <channel> <modes>, sets modes for a channel."), 4);
                    return false;
                }
                else
                    return server->SendMode(params);
            }
            if(command == "msg")
            {
                FXString params = commandtext.after(' ');
                FXString to = params.before(' ');
                FXString message = params.after(' ');
                if(!to.empty() && !message.empty())
                {
                    if(to == getText())
                    {
                        if(coloredNick) AppendIrcNickText(server->GetNickName(), message, GetNickColor(server->GetNickName()));
                        AppendIrcText("<"+server->GetNickName()+"> "+message);
                    }
                    if(message.length() > maxLen-10-to.length())
                    {
                        dxStringArray messages = CutText(message, maxLen-10-to.length());
                        FXbool result = true;
                        for(FXint i=0; i<messages.no(); i++)
                        {
                            result = server->SendMsg(to, messages[i]) &result;
                        }
                        return result;
                    }
                    else return server->SendMsg(to, message);
                }
                else
                {
                    AppendIrcStyledText(_("/msg <nick/channel> <message>, sends a normal message."), 4);
                    return false;
                }
            }
            if(command == "names")
            {
                FXString params = commandtext.after(' ');
                if(type == CHANNEL)
                {
                    if(params.empty()) return server->SendNames(getText());
                    else return server->SendNames(params);
                }
                else
                {
                    if(params.empty())
                    {
                        AppendIrcStyledText(_("/names <channel>, for nicks on a channel."), 4);
                        return false;
                    }
                    else return server->SendNames(params);
                }
            }
            if(command == "nick")
            {
                FXString nick = commandtext.after(' ');
                if(nick.empty())
                {
                    AppendIrcStyledText(_("/nick <nick>, changes nick."), 4);
                    return false;
                }
                else if(nick.length() > server->GetNickLen())
                {
                    AppendIrcStyledText(FXStringFormat(_("Nick is too long. Max. nick length is %d."), server->GetNickLen()), 4);
                    return false;
                }
                else
                {
                    return server->SendNick(nick);
                }
            }
            if(command == "notice")
            {
                FXString params = commandtext.after(' ');
                FXString to = params.before(' ');
                FXString message = params.after(' ');
                if(!to.empty() && !message.empty())
                {
                    if(to == getText()) AppendIrcStyledText(FXStringFormat(_("%s's NOTICE: "), server->GetNickName().text())+message, 2);
                    if(message.length() > maxLen-9-to.length())
                    {
                        dxStringArray messages = CutText(message, maxLen-9-to.length());
                        FXbool result = true;
                        for(FXint i=0; i<messages.no(); i++)
                        {
                            result = server->SendNotice(to, messages[i]) &result;
                        }
                        return result;
                    }
                    return server->SendNotice(to, message);
                }
                else
                {
                    AppendIrcStyledText(_("/notice <nick/channel> <message>, sends a notice."), 4);
                    return false;
                }
            }
            if(command == "op")
            {
                FXString params = commandtext.after(' ');
                if(type == CHANNEL)
                {
                    if(params.empty())
                    {
                        AppendIrcStyledText(_("/op <nicks>, gives operator status for one or more nicks."), 4);
                        return false;
                    }
                    else if(IsChannel(params) && params.after(' ').empty())
                    {
                        AppendIrcStyledText(_("/op <channel> <nicks>, gives operator status for one or more nicks."), 4);
                        return false;
                    }
                    else if(IsChannel(params) && !params.after(' ').empty())
                    {
                        FXString channel = params.before(' ');
                        FXString nicks = params.after(' ');
                        FXString modeparams = utils::CreateModes('+', 'o', nicks);
                        return server->SendMode(channel+" "+modeparams);
                    }
                    else
                    {
                        FXString channel = getText();
                        FXString nicks = params;
                        FXString modeparams = utils::CreateModes('+', 'o', nicks);
                        return server->SendMode(channel+" "+modeparams);
                    }
                }
                else
                {
                    if(IsChannel(params) && !params.after(' ').empty())
                    {
                        FXString channel = params.before(' ');
                        FXString nicks = params.after(' ');
                        FXString modeparams = utils::CreateModes('+', 'o', nicks);
                        return server->SendMode(channel+" "+modeparams);
                    }
                    else
                    {
                        AppendIrcStyledText(_("/op <channel> <nicks>, gives operator status for one or more nicks."), 4);
                        return false;
                    }
                }
            }
            if(command == "oper")
            {
                FXString params = commandtext.after(' ');
                FXString login = params.before(' ');
                FXString password = params.after(' ');
                if(!login.empty() && !password.empty()) return server->SendOper(login, password);
                else
                {
                    AppendIrcStyledText(_("/oper <login> <password>, oper up."), 4);
                    return false;
                }
            }
            if(command == "part")
            {
                if(type == CHANNEL)
                {
                    if(commandtext.after(' ').empty()) return server->SendPart(getText());
                    else return server->SendPart(getText(), commandtext.after(' '));
                }
            }
            if(command == "query")
            {
                if(commandtext.after(' ').empty())
                {
                    AppendIrcStyledText(_("/query <nick>, opens query with nick."), 4);
                    return false;
                }
                else
                {
                    IrcEvent ev;
                    ev.eventType = IRC_QUERY;
                    ev.param1 = commandtext.after(' ').before(' ');
                    parent->getParent()->getParent()->handle(server, FXSEL(SEL_COMMAND, IrcSocket::ID_SERVER), &ev);
                    return true;
                }
            }
            if(command == "quit")
            {                
                //parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_TABQUIT), NULL);
                if(commandtext.after(' ').empty()) server->Disconnect();
                else server->Disconnect(commandtext.after(' '));
                return true;
            }
            if(command == "quote")
            {
                return server->SendQuote(commandtext.after(' '));
            }
            if(command == "say")
            {
                if (type != SERVER && !commandtext.after(' ').empty())
                {
                    if(coloredNick) AppendIrcNickText(server->GetNickName(), commandtext.after(' '), GetNickColor(server->GetNickName()));
                    else AppendIrcText("<"+server->GetNickName()+"> "+commandtext.after(' '));
                    if(commandtext.after(' ').length() > maxLen-10-getText().length())
                    {
                        dxStringArray messages = CutText(commandtext.after(' '), maxLen-10-getText().length());
                        FXbool result = true;
                        for(FXint i=0; i<messages.no(); i++)
                        {
                            result = server->SendMe(getText(), messages[i]) &result;
                        }
                        return result;
                    }
                    else return server->SendMsg(getText(), commandtext.after(' '));
                }
                return false;
            }
            if(command == "topic")
            {
                FXString params = commandtext.after(' ');
                if(type == CHANNEL)
                {
                    if(params.empty()) return server->SendTopic(getText());
                    else if(IsChannel(params) && !params.after(' ').empty())
                    {
                        FXString channel = params.before(' ');
                        FXString topic = params.after(' ');
                        if(topic.length() > server->GetTopicLen())
                        {
                            AppendIrcStyledText(FXStringFormat(_("Topic is too long. Max. topic length is %d."), server->GetTopicLen()), 4);
                            return false;
                        }
                        else return server->SendTopic(channel, topic);
                    }
                    else
                    {
                        if(params.length() > server->GetTopicLen())
                        {
                            AppendIrcStyledText(FXStringFormat(_("Topic is too long. Max. topic length is %d."), server->GetTopicLen()), 4);
                            return false;
                        }
                        else return server->SendTopic(getText(), params);
                    }
                }
                else
                {
                    if(IsChannel(params))
                    {
                        FXString channel = params.before(' ');
                        FXString topic = params.after(' ');
                        if(topic.length() > server->GetTopicLen())
                        {
                            AppendIrcStyledText(FXStringFormat(_("Topic is too long. Max. topic length is %d."), server->GetTopicLen()), 4);
                            return false;
                        }
                        else return server->SendTopic(channel, topic);
                    }
                    else
                    {
                        AppendIrcStyledText(_("/topic <channel> [topic], views or changes channel topic."), 4);
                        return false;
                    }
                }
            }
            if(command == "voice")
            {
                FXString params = commandtext.after(' ');
                if(type == CHANNEL)
                {
                    if(params.empty())
                    {
                        AppendIrcStyledText(_("/voice <nicks>, gives voice for one or more nicks."), 4);
                        return false;
                    }
                    else if(IsChannel(params) && params.after(' ').empty())
                    {
                        AppendIrcStyledText(_("/voice <channel> <nicks>, gives voice for one or more nicks."), 4);
                        return false;
                    }
                    else if(IsChannel(params) && !params.after(' ').empty())
                    {
                        FXString channel = params.before(' ');
                        FXString nicks = params.after(' ');
                        FXString modeparams = utils::CreateModes('+', 'v', nicks);
                        return server->SendMode(channel+" "+modeparams);
                    }
                    else
                    {
                        FXString channel = getText();
                        FXString nicks = params;
                        FXString modeparams = utils::CreateModes('+', 'v', nicks);
                        return server->SendMode(channel+" "+modeparams);
                    }
                }
                else
                {
                    if(IsChannel(params) && !params.after(' ').empty())
                    {
                        FXString channel = params.before(' ');
                        FXString nicks = params.after(' ');
                        FXString modeparams = utils::CreateModes('+', 'v', nicks);
                        return server->SendMode(channel+" "+modeparams);
                    }
                    else
                    {
                        AppendIrcStyledText(_("/voice <channel> <nicks>, gives voice for one or more nicks."), 4);
                        return false;
                    }
                }
            }
            if(command == "wallops")
            {
                FXString params = commandtext.after(' ');
                if(params.empty())
                {
                    AppendIrcStyledText(_("/wallops <message>, sends wallop message."), 4);
                    return false;
                }
                else
                {
                    if(params.length() > maxLen-9)
                    {
                        dxStringArray messages = CutText(params, maxLen-9);
                        FXbool result = true;
                        for(FXint i=0; i<messages.no(); i++)
                        {
                            result = server->SendWallops(messages[i]) &result;
                        }
                        return result;
                    }
                    else return server->SendWallops(params);
                }
            }
            if(command == "who")
            {
                FXString params = commandtext.after(' ');
                if(params.empty())
                {
                    AppendIrcStyledText(_("/who <mask> [o], searchs for mask on network, if o is supplied, only search for opers."), 4);
                    return false;
                }
                else return server->SendWho(params);
            }
            if(command == "whois")
            {
                FXString params = commandtext.after(' ');
                if(params.empty())
                {
                    AppendIrcStyledText(_("/whois <nick>, whois nick."), 4);
                    return false;
                }
                else return server->SendWhois(params);;
            }
            if(command == "whowas")
            {
                FXString params = commandtext.after(' ');
                if(params.empty())
                {
                    AppendIrcStyledText(_("/whowas <nick>, whowas nick."), 4);
                    return false;
                }
                else return server->SendWhowas(params);
            }
            AppendIrcStyledText(FXStringFormat(_("Unknown command '%s', type /commands for available commands"), command.text()), 4);
            return false;
        }
        else
        {
            if (command.empty() && type != SERVER && !commandtext.empty())
            {
                if(coloredNick) AppendIrcNickText(server->GetNickName(), commandtext, GetNickColor(server->GetNickName()));
                else AppendIrcText("<"+server->GetNickName()+"> "+commandtext);
                if(commandtext.length() > maxLen-10-getText().length())
                {
                    dxStringArray messages = CutText(commandtext, maxLen-10-getText().length());
                    FXbool result = true;
                    for(FXint i=0; i<messages.no(); i++)
                    {
                        result = server->SendMsg(getText(), messages[i]) &result;
                    }
                    return result;
                }
                else return server->SendMsg(getText(), commandtext);
            }
            return false;
        }
    }
    else
    {
        if(command == "connect")
        {            
            if(commandtext.after(' ').empty())
            {
                AppendIrcStyledText(_("/connect <server> [port] [nick] [password] [realname] [channels], connect for given server."), 4);
                return false;
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
                parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CSERVER), &srv);
            }
            return true;
        }
        if(command == "commands")
        {
            FXString commandstr = _("Available commnads: ");
            for(FXint i=0; i < utils::CommandsNo(); i++)
            {
                if(utils::CommandsAt(i) != "commands") commandstr += utils::CommandsAt(i).upper()+(i != utils::CommandsNo() - 1? ", " : "");
            }
            AppendIrcStyledText(commandstr, 3);           
            return true;
        }
        if(command == "exec")
        {
            FXString params = commandtext.after(' ');
            if(params.empty())
            {
                AppendIrcStyledText(_("/exec [-o|-c] <command>, execute command, -o send output to channel/query, -c close running command"), 4);
                return false;
            }
            else
            {
                if(!pipe)
                    pipe = new dxPipe(getApp(), this);
                pipeStrings.clear();
                if(params.before(' ').contains("-o"))
                {
                    sendPipe = true;                    
                    pipe->ExecCmd(params.after(' '));
                }
                else if(params.before(' ').contains("-c"))
                {
                    sendPipe = false;                    
                    pipe->StopCmd();
                }
                else
                {
                    sendPipe = false;
                    pipe->ExecCmd(params);
                }
                return true;
            }
        }
        if(command == "help")
        {
            return ShowHelp(commandtext.after(' ').lower());
        }
        else
        {
            AppendIrcStyledText(_("You aren't connected"), 4);
            parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);            
            return true;
        }
    }
    return false;
}

FXbool IrcTabItem::ShowHelp(FXString command)
{
    if(command == "admin")
    {
        AppendIrcText(_("ADMIN [server], finds information about administrator for current server or [server]."));
        return true;
    }
    if(command == "away")
    {
        AppendIrcText(_("AWAY [message], sets away status."));
        return true;
    }
    if(command == "banlist")
    {
        AppendIrcText(_("BANLIST <channel>, shows banlist for channel."));
        return true;
    }
    if(command == "connect")
    {
        AppendIrcText(_("CONNECT <server> [port] [nick] [password] [realname] [channels], connects for given server."));
        return true;
    }
    if(command == "ctcp")
    {
        AppendIrcText(_("CTCP <nick> <message>, sends a CTCP message to a user."));
        return true;
    }
    if(command == "deop")
    {
        AppendIrcText(_("DEOP <channel> <nicks>, removes operator status from one or more nicks."));
        return true;
    }
    if(command == "devoice")
    {
        AppendIrcText(_("DEVOICE <channel> <nicks>, removes voice from one or more nicks."));
        return true;
    }
#ifndef WIN32
    if(command == "exec")
    {
        AppendIrcText(_("EXEC [-o|-c] <command>, executes command, -o sends output to channel/query, -c closes running command."));
        return true;
    }
#endif
    if(command == "help")
    {
        AppendIrcText(_("HELP <command>, shows help for command."));
        return true;
    }
    if(command == "invite")
    {
        AppendIrcText(_("INVITE <nick> <channel>, invites someone to a channel."));
        return true;
    }
    if(command == "join")
    {
        AppendIrcText(_("JOIN <channel>, joins a channel."));
        return true;
    }
    if(command == "kick")
    {
        AppendIrcText(_("KICK <channel> <nick>, kicks a user from a channel."));
        return true;
    }
    if(command == "kill")
    {
        AppendIrcText(_("KILL <user> [reason], kills a user from the network."));
        return true;
    }
    if(command == "list")
    {
        AppendIrcText(_("LIST [channel], lists channels and their topics."));
        return true;
    }
    if(command == "me")
    {
        AppendIrcText(_("ME <to> <message>, sends the action."));
        return true;
    }
    if(command == "mode")
    {
        AppendIrcText(_("MODE <channel> <modes>, sets modes for a channel."));
        return true;
    }
    if(command == "msg")
    {
        AppendIrcText(_("MSG <nick/channel> <message>, sends a normal message."));
        return true;
    }
    if(command == "names")
    {
        AppendIrcText(_("NAMES <channel>, for nicks on a channel."));
        return true;
    }
    if(command == "nick")
    {
        AppendIrcText(_("NICK <nick>, changes nick."));
        return true;
    }
    if(command == "notice")
    {
        AppendIrcText(_("NOTICE <nick/channel> <message>, sends a notice."));
        return true;
    }
    if(command == "op")
    {
        AppendIrcText(_("OP <channel> <nicks>, gives operator status for one or more nicks."));
        return true;
    }
    if(command == "oper")
    {
        AppendIrcText(_("OPER <login> <password>, oper up."));
        return true;
    }
    if(command == "part")
    {
        AppendIrcText(_("PART <channel> [reason], leaves channel."));
        return true;
    }
    if(command == "query")
    {
        AppendIrcText(_("QUERY <nick>, opens query with nick."));
        return true;
    }
    if(command == "quit")
    {
        AppendIrcText(_("QUIT [reason], leaves server."));
        return true;
    }
    if(command == "quote")
    {
        AppendIrcText(_("QUOTE [text], sends text to server."));
        return true;
    }
    if(command == "say")
    {
        AppendIrcText(_("SAY [text], sends text to current tab."));
        return true;
    }
    if(command == "topic")
    {
        AppendIrcText(_("TOPIC [topic], sets or shows topic."));
        return true;
    }
    if(command == "voice")
    {
        AppendIrcText(_("VOICE <channel> <nicks>, gives voice for one or more nicks."));
        return true;
    }
    if(command == "wallops")
    {
        AppendIrcText(_("WALLOPS <message>, sends wallop message."));
        return true;
    }
    if(command == "who")
    {
        AppendIrcText(_("WHO <mask> [o], searchs for mask on network, if o is supplied, only search for opers."));
        return true;
    }
    if(command == "whois")
    {
        AppendIrcText(_("WHOIS <nick>, whois nick."));
        return true;
    }
    if(command == "whowas")
    {
        AppendIrcText(_("WHOWAS <nick>, whowas nick."));
        return true;
    }
    if(!utils::GetAlias(command[0] == '/' ? command:"/"+command).empty())
    {
        AppendIrcText(FXStringFormat("%s: %s", command.upper().text(), utils::GetAlias(command[0] == '/' ? command:"/"+command).text()));
        return true;
    }
    AppendIrcStyledText(FXStringFormat(_("Unknown command '%s', type /commands for available commands"), command.text()), 4);
    return false;
}

long IrcTabItem::OnKeyPress(FXObject *, FXSelector, void *ptr)
{
    if (commandline->hasFocus())
    {
        FXEvent* event = (FXEvent*)ptr;
        switch(event->code){
            case KEY_Tab:
                if(commandline->getText()[0] == '/' && commandline->getText().after(' ').empty())
                {
                    for (FXint i = 0; i < utils::CommandsNo(); i++)
                    {
                        if(comparecase(commandline->getText().after('/').before(' '), utils::CommandsAt(i)) == 0)
                        {
                            if((i+1) < utils::CommandsNo()) commandline->setText("/"+utils::CommandsAt(++i)+" ");
                            else commandline->setText("/"+utils::CommandsAt(0)+" ");
                            break;
                        }
                        else if(comparecase(commandline->getText().after('/'), utils::CommandsAt(i).left(commandline->getText().after('/').length())) == 0)
                        {
                            commandline->setText("/"+utils::CommandsAt(i)+" ");
                            break;
                        }
                    }
                    return 1;
                }
                if(commandline->getText()[0] != '/' && commandline->getText().after(' ').empty())
                {
                    for(FXint j = 0; j < users->getNumItems() ; j++)
                    {
                        if(comparecase(commandline->getText(), GetNick(j).left(commandline->getText().length())) == 0)
                        {
                            commandline->setText(GetNick(j)+nickCompletionChar+" ");
                        }
                        else if(comparecase(commandline->getText().section(nickCompletionChar, 0, 1), GetNick(j)) == 0)
                        {
                            if((j+1) < users->getNumItems()) commandline->setText(GetNick(++j)+nickCompletionChar+" ");
                            else commandline->setText(GetNick(0)+nickCompletionChar+" ");
                        }
                    }
                    return 1;
                }
                if(commandline->getText().find(' ') != -1)
                {
                    FXString line = commandline->getText().rbefore(' ')+" ";
                    FXString toCompletion = commandline->getText().rafter(' ');
                    for(FXint j = 0; j < users->getNumItems(); j++)
                    {
                        if(comparecase(toCompletion, GetNick(j)) == 0)
                        {
                            if((j+1) < users->getNumItems()) commandline->setText(line+GetNick(++j));
                            else commandline->setText(line+GetNick(0));
                            break;
                        }
                        else if(comparecase(toCompletion, GetNick(j).left(toCompletion.length())) == 0)
                        {
                            commandline->setText(line+GetNick(j));
                            break;
                        }
                    }
                    return 1;
                }
                return 1;
            case KEY_Up:
                if (currentPosition > 0)
                {
                    --currentPosition;
                    commandline->setText(commandsHistory.at(currentPosition));
                }
                return 1;
            case KEY_Down:
                if (currentPosition < commandsHistory.no()-1)
                {
                    ++currentPosition;
                    commandline->setText(commandsHistory.at(currentPosition));
                }
                else
                {
                    currentPosition = commandsHistory.no();
                    commandline->setText("");
                }
                return 1;
        }
    }
    return 0;
}

//Check is this tab current or first for server
FXbool IrcTabItem::IsFirst()
{
    FXint indexOfCurrent = parent->getCurrent()*2;
    FXbool hasCurrent = server->FindTarget((IrcTabItem *)parent->childAtIndex(indexOfCurrent));
    FXint indexOfThis = parent->indexOfChild(this);
    if(hasCurrent)
    {
        if(indexOfCurrent == indexOfThis) return TRUE;
        else return FALSE;
    }
    else
    {
        for (FXint i = 0; i<parent->numChildren(); i=i+2)
        {
            if(server->FindTarget((IrcTabItem *)parent->childAtIndex(i)))
            {
                if(i == indexOfThis) return TRUE;
                else return FALSE;
            }
        }
    }
    return FALSE;
}

FXbool IrcTabItem::IsCommandIgnored(const FXString &command)
{
    if(commandsList.contains(command)) return true;
    return false;
}

void IrcTabItem::AddUser(const FXString& user)
{
    FXRex rx("(\\*|\\!|@|\\+|\\%)");
    if((rx.match(user) ? users->findItem(user.mid(1, user.length()-1)) : users->findItem(user)) == -1)
    {
        FXchar status = user[0];
        switch(status) {
            case '*':
                users->appendItem(user.after('*'), irc_owner_icon);
                break;
            case '!':
                users->appendItem(user.after('!'), irc_admin_icon);
                break;
            case '@':
                users->appendItem(user.after('@'), irc_op_icon);
                break;
            case '+':
                users->appendItem(user.after('+'), irc_voice_icon);
                break;
            case '%':
                users->appendItem(user.after('%'), irc_halfop_icon);
                break;
            default:
                users->appendItem(user, irc_normal_icon);
        }
    }
    numberUsers++;
    users->sortItems();
}

void IrcTabItem::RemoveUser(const FXString& user)
{
    if(users->findItem(user) != -1)
    {
        users->removeItem(users->findItem(user));
    }
    numberUsers--;
    users->sortItems();
}

void IrcTabItem::ChangeNickUser(const FXString& nick, const FXString& newnick)
{
    FXint i = users->findItem(nick);
    if(i != -1)
    {
        if(users->getItemIcon(i) == irc_owner_icon || users->getItemIcon(i) == irc_away_owner_icon)
        {
            RemoveUser(nick);
            AddUser("*"+newnick);
        }
        else if(users->getItemIcon(i) == irc_admin_icon || users->getItemIcon(i) == irc_away_admin_icon)
        {
            RemoveUser(nick);
            AddUser("!"+newnick);
        }
        else if(users->getItemIcon(i) == irc_op_icon || users->getItemIcon(i) == irc_away_op_icon)
        {
            RemoveUser(nick);
            AddUser("@"+newnick);
        }
        else if(users->getItemIcon(i) == irc_voice_icon || users->getItemIcon(i) == irc_away_voice_icon)
        {
            RemoveUser(nick);
            AddUser("+"+newnick);
        }
        else if(users->getItemIcon(i) == irc_halfop_icon || users->getItemIcon(i) == irc_away_halfop_icon)
        {
            RemoveUser(nick);
            AddUser("%"+newnick);
        }
        else
        {
            RemoveUser(nick);
            AddUser(newnick);
        }
    }
}

long IrcTabItem::OnIrcEvent(FXObject *, FXSelector, void *data)
{
    IrcEvent *ev = (IrcEvent *) data;
    if(ev->eventType == IRC_PRIVMSG)
    {
        if((comparecase(ev->param2, getText()) == 0 && type == CHANNEL) || (ev->param1 == getText() && type == QUERY && ev->param2 == server->GetNickName()))
        {
            if(coloredNick)
            {
                if(ev->param3.contains(server->GetNickName())) AppendIrcStyledText(ev->param1+": "+ev->param3, 8);
                else AppendIrcNickText(ev->param1, ev->param3, GetNickColor(ev->param1));
            }
            else
            {
                if(ev->param3.contains(server->GetNickName())) AppendIrcStyledText("<"+ev->param1+"> "+ev->param3, 8);
                else AppendIrcText("<"+ev->param1+"> "+ev->param3);
            }
            if(FXRGB(255,0,0) != this->getTextColor() && parent->getCurrent()*2 != parent->indexOfChild(this))
            {
                if(ev->param3.contains(server->GetNickName()))
                {
                    this->setTextColor(FXRGB(255,0,0));
                    if(type == CHANNEL) this->setIcon(chnewm);
                }
                else this->setTextColor(FXRGB(0,0,255));
                if(type == QUERY) this->setIcon(unewm);                    
            }
            if((type == CHANNEL && ev->param3.contains(server->GetNickName())) || type == QUERY)
                parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_NEWMSG), NULL);
        }
        return 1;
    }
    if(ev->eventType == IRC_ACTION)
    {
        if((comparecase(ev->param2, getText()) == 0 && type == CHANNEL) || (ev->param1 == getText() && type == QUERY && ev->param2 == server->GetNickName()))
        {
            if(!IsCommandIgnored("me"))
            {
                AppendIrcStyledText(ev->param1+" "+ev->param3, 2);
                if(FXRGB(255,0,0) != this->getTextColor() && parent->getCurrent()*2 != parent->indexOfChild(this))
                {
                    if(ev->param3.contains(server->GetNickName()))
                    {
                        this->setTextColor(FXRGB(255,0,0));
                        if(type == CHANNEL) this->setIcon(chnewm);
                    }
                    else this->setTextColor(FXRGB(0,0,255));
                    if(type == QUERY) this->setIcon(unewm);
                }
                if((type == CHANNEL && ev->param3.contains(server->GetNickName())) || type == QUERY)
                    parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_NEWMSG), NULL);
            }
        }
        return 1;
    }
    if(ev->eventType == IRC_CTCPREPLY)
    {
        if(type == SERVER || IsFirst())
        {
            if(!IsCommandIgnored("ctcp")) AppendIrcStyledText(FXStringFormat(_("CTCP %s reply from %s: %s"), utils::GetParam(ev->param2, 1, false).text(), ev->param1.text(), utils::GetParam(ev->param2, 2, true).text()), 2);
        }
        return 1;
    }
    if(ev->eventType == IRC_CTCPREQUEST)
    {
        if(type == SERVER || IsFirst())
        {
            if(!IsCommandIgnored("ctcp")) AppendIrcStyledText(FXStringFormat(_("CTCP %s request from %s"), ev->param2.text(), ev->param1.text()), 2);
        }
        return 1;
    }
    if(ev->eventType == IRC_JOIN)
    {
        if(comparecase(ev->param2, getText()) == 0 && ev->param1 != server->GetNickName())
        {
            if(!IsCommandIgnored("join")) AppendIrcStyledText(FXStringFormat(_("%s has joined to %s"), ev->param1.text(), ev->param2.text()), 1);
            AddUser(ev->param1);
        }
        return 1;
    }
    if(ev->eventType == IRC_QUIT)
    {
        if(type == CHANNEL && users->findItem(ev->param1) != -1)
        {
            RemoveUser(ev->param1);
            if(ev->param2.empty())
            {
                if(!IsCommandIgnored("quit")) AppendIrcStyledText(FXStringFormat(_("%s has quit"), ev->param1.text()), 1);
            }
            else
            {
                if(!IsCommandIgnored("quit"))AppendIrcStyledText(FXStringFormat(_("%s has quit (%s)"), ev->param1.text(), +ev->param2.text()), 1);
            }
        }
        else if(type == QUERY && getText() == ev->param1)
        {
            AppendIrcStyledText(FXStringFormat(_("%s has quit"), ev->param1.text()), 1);
        }
        return 1;
    }
    if(ev->eventType == IRC_PART)
    {
        if(comparecase(ev->param2, getText()) == 0)
        {
            if(ev->param3.empty() && !IsCommandIgnored("part")) AppendIrcStyledText(FXStringFormat(_("%s has parted %s"), ev->param1.text(), ev->param2.text()), 1);
            else if(!IsCommandIgnored("part")) AppendIrcStyledText(FXStringFormat(_("%s has parted %s (%s)"), ev->param1.text(), ev->param2.text(), ev->param3.text()), 1);
            RemoveUser(ev->param1);
        }
        return 1;
    }
    if(ev->eventType == IRC_CHNOTICE)
    {
        if((comparecase(ev->param2, getText()) == 0 && type == CHANNEL) || (ev->param1 == getText() && type == QUERY && ev->param2 == server->GetNickName()))
        {
            if(!IsCommandIgnored("notice"))
            {
                AppendIrcStyledText(FXStringFormat(_("%s's NOTICE: %s"), ev->param1.text(), ev->param3.text()), 2);
                if(FXRGB(255,0,0) != this->getTextColor() && parent->getCurrent()*2 != parent->indexOfChild(this))
                {
                    if(ev->param3.contains(server->GetNickName()))
                    {
                        this->setTextColor(FXRGB(255,0,0));
                        if(type == CHANNEL) this->setIcon(chnewm);
                    }
                    else this->setTextColor(FXRGB(0,0,255));
                    if(type == QUERY) this->setIcon(unewm);
                }
                if((type == CHANNEL && ev->param3.contains(server->GetNickName())) || type == QUERY)
                    parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_NEWMSG), NULL);
            }
        }
        else if(type == SERVER || IsFirst())
        {
            if(!IsCommandIgnored("notice")) AppendIrcStyledText(FXStringFormat(_("%s's NOTICE: %s"), ev->param1.text(), ev->param3.text()), 3);
        }
        return 1;
    }
    if(ev->eventType == IRC_NOTICE)
    {
        if(type == SERVER || IsFirst())
        {
            if(ev->param1 == server->GetNickName() && !IsCommandIgnored("notice")) AppendIrcStyledText(FXStringFormat(_("NOTICE for you: %s"), ev->param2.text()), 3);
            else if(!IsCommandIgnored("notice")) AppendIrcStyledText(FXStringFormat(_("%s's NOTICE: %s"), ev->param1.text(), ev->param2.text()), 3);
        }
        return 1;
    }
    if(ev->eventType == IRC_NICK)
    {
        if(users->findItem(ev->param1) != -1)
        {
            if(ev->param2 == server->GetNickName() && !IsCommandIgnored("nick")) AppendIrcStyledText(FXStringFormat(_("You're now known as %s"), ev->param2.text()), 1);
            else if(!IsCommandIgnored("nick")) AppendIrcStyledText(FXStringFormat(_("%s is now known as %s"), ev->param1.text(), ev->param2.text()), 1);
            ChangeNickUser(ev->param1, ev->param2);
        }
        if(type == QUERY && ev->param1 == getText())
        {
            this->setText(ev->param2);
        }
        return 1;
    }
    if(ev->eventType == IRC_TOPIC)
    {
        if(comparecase(ev->param2, getText()) == 0)
        {
            AppendIrcText(FXStringFormat(_("%s Set new topic for %s: %s"), ev->param1.text(), ev->param2.text(), ev->param3.text()));
            topic = ev->param3;
            topicline->setText(topic);            
        }
        return 1;
    }
    if(ev->eventType == IRC_INVITE)
    {
        if(type == SERVER || IsFirst())
        {
            AppendIrcStyledText(FXStringFormat(_("%s invites you to: %s"), ev->param1.text(), ev->param3.text()), 3);
        }
        return 1;
    }
    if(ev->eventType == IRC_KICK)
    {
        if(comparecase(ev->param3, getText()) == 0)
        {
            if(ev->param2 != server->GetNickName())
            {
                if(ev->param4.empty()) AppendIrcStyledText(FXStringFormat(_("%s was kicked from %s by %s"), ev->param2.text(), ev->param3.text(), ev->param1.text()), 1);
                else AppendIrcStyledText(FXStringFormat(_("%s was kicked from %s by %s (%s)"), ev->param2.text(), ev->param3.text(), ev->param1.text(), ev->param4.text()), 1);
                RemoveUser(ev->param2);
            }
        }
        if(ev->param2 == server->GetNickName() && (type == SERVER || IsFirst()))
        {
            if(ev->param4.empty()) AppendIrcStyledText(FXStringFormat(_("You were kicked from %s by %s"), ev->param3.text(), ev->param1.text()), 1);
            else AppendIrcStyledText(FXStringFormat(_("You were kicked from %s by %s (%s)"), ev->param3.text(), ev->param1.text(), ev->param4.text()), 1);
        }
        return 1;
    }
    if(ev->eventType == IRC_MODE)
    {
        if(type == SERVER || IsFirst())
        {
            AppendIrcStyledText(FXStringFormat(_("Mode change [%s] for %s"), ev->param1.text(), ev->param2.text()), 1);
        }
        return 1;
    }
    if(ev->eventType == IRC_UMODE)
    {
        FXString moderator = ev->param1;
        FXString channel = ev->param2;
        FXString modes = ev->param3;
        FXString args = ev->param4;
        if(comparecase(channel, getText()) == 0)
        {
            FXbool sign = false;
            int argsiter = 1;
            for(int i =0; i < modes.count(); i++) {
                switch(modes[i]) {
                    case '+':
                        sign = true;
                        break;
                    case '-':
                        sign = false;
                        break;
                    case 'a': //admin
                    {
                        FXString nick = utils::GetParam(args, argsiter, false);
                        RemoveUser(nick);
                        if(sign)
                        {
                            AddUser("!"+nick);
                            if(!IsCommandIgnored("mode"))
                            {
                                if(nick == server->GetNickName()) AppendIrcStyledText(FXStringFormat(_("%s gave you admin"), moderator.text()), 1);
                                else AppendIrcStyledText(FXStringFormat(_("%s gave %s admin"), moderator.text(), nick.text()), 1);
                            }
                        }
                        else
                        {
                            AddUser(nick);
                            if(!IsCommandIgnored("mode"))
                            {
                                if(nick == server->GetNickName()) AppendIrcStyledText(FXStringFormat(_("%s removed you admin"), moderator.text()), 1);
                                else AppendIrcStyledText(FXStringFormat(_("%s removed %s admin"), moderator.text(), nick.text()), 1);
                            }
                        }
                        argsiter++;
                    }break;
                    case 'o': //op
                    {
                        FXString nick = utils::GetParam(args, argsiter, false);
                        RemoveUser(nick);
                        if(sign)
                        {
                            AddUser("@"+nick);
                            if(!IsCommandIgnored("mode"))
                            {
                                if(nick == server->GetNickName()) AppendIrcStyledText(FXStringFormat(_("%s gave you op"), moderator.text()), 1);
                                else AppendIrcStyledText(FXStringFormat(_("%s gave %s op"), moderator.text(), nick.text()), 1);
                            }
                        }
                        else
                        {
                            AddUser(nick);
                            if(!IsCommandIgnored("mode"))
                            {
                                if(nick == server->GetNickName()) AppendIrcStyledText(FXStringFormat(_("%s removed you op"), moderator.text()), 1);
                                else AppendIrcStyledText(FXStringFormat(_("%s removed %s op"), moderator.text(), nick.text()), 1);
                            }
                        }
                        if (server->GetNickName() == nick) sign ? iamOp = true : iamOp = false;
                        argsiter++;
                    }break;
                    case 'v': //voice
                    {
                        FXString nick = utils::GetParam(args, argsiter, false);
                        RemoveUser(nick);
                        if(sign)
                        {
                            AddUser("+"+nick);
                            if(!IsCommandIgnored("mode"))
                            {
                                if(nick == server->GetNickName()) AppendIrcStyledText(FXStringFormat(_("%s gave you voice"), moderator.text()), 1);
                                else AppendIrcStyledText(FXStringFormat(_("%s gave %s voice"), moderator.text(), nick.text()), 1);
                            }
                        }
                        else
                        {
                            AddUser(nick);
                            if(!IsCommandIgnored("mode"))
                            {
                                if(nick == server->GetNickName()) AppendIrcStyledText(FXStringFormat(_("%s removed you voice"), moderator.text()), 1);
                                else AppendIrcStyledText(FXStringFormat(_("%s removed %s voice"), moderator.text(), nick.text()), 1);
                            }
                        }
                        argsiter++;
                    }break;
                    case 'h': //halfop
                    {
                        FXString nick = utils::GetParam(args, argsiter, false);
                        RemoveUser(nick);
                        if(sign)
                        {
                            AddUser("%"+nick);
                            if(!IsCommandIgnored("mode"))
                            {
                                if(nick == server->GetNickName()) AppendIrcStyledText(FXStringFormat(_("%s gave you halfop"), moderator.text()), 1);
                                else AppendIrcStyledText(FXStringFormat(_("%s gave %s halfop"), moderator.text(), nick.text()), 1);
                            }
                        }
                        else
                        {
                            AddUser(nick);
                            if(!IsCommandIgnored("mode"))
                            {
                                if(nick == server->GetNickName()) AppendIrcStyledText(FXStringFormat(_("%s removed you halfop"), moderator.text()), 1);
                                else AppendIrcStyledText(FXStringFormat(_("%s removed %s halfop"), moderator.text(), nick.text()), 1);
                            }
                        }
                        argsiter++;
                    }break;
                    case 'b': //ban
                    {
                        FXString banmask = utils::GetParam(args, argsiter, false);
                        OnBan(banmask, sign, moderator);
                        argsiter++;
                    }break;
                    case 't': //topic settable by channel operator
                    {
                        sign ? editableTopic = false : editableTopic = true;
                    }
                    default:
                    {
                        AppendIrcStyledText(FXStringFormat(_("%s set Mode: %s"), moderator.text(), FXString(modes+" "+args).text()), 1);
                    }
                }
            }
        }
        return 1;
    }
    if(ev->eventType == IRC_CHMODE)
    {
        FXString channel = ev->param1;
        FXString modes = ev->param2;
        if(comparecase(channel, getText()) == 0)
        {
            if(modes.contains('t')) editableTopic = false;
        }
        return 1;
    }
    if(ev->eventType == IRC_SERVERREPLY)
    {
        if(ownServerWindow)
        {
            if(type == SERVER)
            {
                //this->setText(server->GetRealServerName());
                AppendIrcText(ev->param1);
                if(getApp()->getForeColor() == this->getTextColor() && parent->getCurrent()*2 != parent->indexOfChild(this)) this->setTextColor(FXRGB(0,0,255));
            }
        }
        else
        {
            if(IsFirst()) AppendIrcText(ev->param1);
        }
        return 1;
    }
    if(ev->eventType == IRC_CONNECT)
    {
        AppendIrcStyledText(ev->param1, 3);
        return 1;
    }
    if(ev->eventType == IRC_ERROR)
    {
        AppendIrcStyledText(ev->param1, 4);
    }
    if(ev->eventType == IRC_SERVERERROR)
    {
        if(ownServerWindow)
        {
            if(type == SERVER)
            {
                AppendIrcStyledText(ev->param1, 4);
                if(getApp()->getForeColor() == this->getTextColor() && parent->getCurrent()*2 != parent->indexOfChild(this)) this->setTextColor(FXRGB(0,0,255));
            }
        }
        else
        {
            if(IsFirst()) AppendIrcStyledText(ev->param1, 4);
        }
        return 1;
    }
    if(ev->eventType == IRC_DISCONNECT)
    {
        AppendIrcStyledText(ev->param1, 4);
    }
    if(ev->eventType == IRC_UNKNOWN)
    {
        if(ownServerWindow)
        {
            if(type == SERVER)
            {
                AppendIrcStyledText(FXStringFormat(_("Unhandled command '%s' params: %s"), ev->param1.text(), ev->param2.text()), 4);
                if(getApp()->getForeColor() == this->getTextColor() && parent->getCurrent()*2 != parent->indexOfChild(this)) this->setTextColor(FXRGB(0,0,255));
            }
        }
        else
        {
            if(IsFirst()) AppendIrcStyledText(FXStringFormat(_("Unhandled command '%s' params: %s"), ev->param1.text(), ev->param2.text()), 4);
        }
        return 1;
    }
    if(ev->eventType == IRC_301)
    {
        if(parent->getCurrent()*2 == parent->indexOfChild(this) || getText() == ev->param1)
        {
            if(!IsCommandIgnored("away")) AppendIrcStyledText(FXStringFormat(_("%s is away: %s"),ev->param1.text(), ev->param2.text()), 1);
        }
        return 1;
    }
    if(ev->eventType == IRC_305)
    {
        FXint i = users->findItem(server->GetNickName());
        if(i != -1)
        {
            AppendIrcStyledText(ev->param1, 1);
            if(users->getItemIcon(i) == irc_away_owner_icon)
            {
                users->setItemIcon(i, irc_owner_icon);
                return 1;
            }
            if(users->getItemIcon(i) == irc_away_admin_icon)
            {
                users->setItemIcon(i, irc_admin_icon);
                return 1;
            }
            if(users->getItemIcon(i) == irc_away_op_icon)
            {
                users->setItemIcon(i, irc_op_icon);
                return 1;
            }
            if(users->getItemIcon(i) == irc_away_voice_icon)
            {
                users->setItemIcon(i, irc_voice_icon);
                return 1;
            }
            if(users->getItemIcon(i) == irc_away_halfop_icon)
            {
                users->setItemIcon(i, irc_halfop_icon);
                return 1;
            }
            if(users->getItemIcon(i) == irc_away_normal_icon)
            {
                users->setItemIcon(i, irc_normal_icon);
                return 1;
            }
        }
        return 1;
    }
    if(ev->eventType == IRC_306)
    {
        FXint i = users->findItem(server->GetNickName());
        if(i != -1)
        {
            AppendIrcStyledText(ev->param1, 1);
            if(users->getItemIcon(i) == irc_owner_icon)
            {
                users->setItemIcon(i, irc_away_owner_icon);
                return 1;
            }
            if(users->getItemIcon(i) == irc_admin_icon)
            {
                users->setItemIcon(i, irc_away_admin_icon);
                return 1;
            }
            if(users->getItemIcon(i) == irc_op_icon)
            {
                users->setItemIcon(i, irc_away_op_icon);
                return 1;
            }
            if(users->getItemIcon(i) == irc_voice_icon)
            {
                users->setItemIcon(i, irc_away_voice_icon);
                return 1;
            }
            if(users->getItemIcon(i) == irc_halfop_icon)
            {
                users->setItemIcon(i, irc_away_halfop_icon);
                return 1;
            }
            if(users->getItemIcon(i) == irc_normal_icon)
            {
                users->setItemIcon(i, irc_away_normal_icon);
                return 1;
            }
        }
        return 1;
    }
    if(ev->eventType == IRC_331 || ev->eventType == IRC_332 || ev->eventType == IRC_333)
    {
        if(comparecase(ev->param1, getText()) == 0)
        {
            AppendIrcText(ev->param2);
            if(ev->eventType == IRC_331)
            {
                topic = StripColors(ev->param2, true);
                topicline->setText(topic);
            }
            if(ev->eventType == IRC_332)
            {
                topic = StripColors(utils::GetParam(ev->param2, 2, true, ':').after(' '), true);
                topicline->setText(topic);
            }
        }
        return 1;
    }
    if(ev->eventType == IRC_353)
    {
        FXString channel = ev->param1;
        FXString usersStr = ev->param2;
        FXString myNick = server->GetNickName();
        if(usersStr.right(1) != " ") usersStr.append(" ");
        if(comparecase(channel, getText()) == 0)
        {
            while (usersStr.contains(' '))
            {
                AddUser(usersStr.before(' '));
                if(usersStr[0] == '@' && usersStr.before(' ').after('@') == myNick) iamOp = true;
                usersStr = usersStr.after(' ');
            }
        }
        else
        {
            FXbool channelOn = false;
            for (FXint i = 0; i<parent->numChildren(); i=i+2)
            {
                if(server->FindTarget((IrcTabItem *)parent->childAtIndex(i)) && comparecase(((IrcTabItem *)parent->childAtIndex(i))->getText(), channel) == 0)
                {
                    channelOn = true;
                    break;
                }
            }
            if(!channelOn && !IsCommandIgnored("numeric")) AppendIrcText(FXStringFormat(_("Users on %s: %s"), channel.text(), usersStr.text()));
        }
        return 1;
    }
    if(ev->eventType == IRC_366)
    {
        if(comparecase(ev->param1, getText()) == 0)
        {
            server->SendWho(getText());
            server->AddIgnoreCommands("who "+getText());
            if(numberUsers < maxAway)
            {
                checkAway = true;
                getApp()->addTimeout(this, ID_TIME, 180000);
            }
        }
        return 1;
    }
    if(ev->eventType == IRC_372)
    {
        if(ownServerWindow)
        {
            if(type == SERVER)
            {
                AppendIrcText(ev->param1);
                if(getApp()->getForeColor() == this->getTextColor() && parent->getCurrent()*2 != parent->indexOfChild(this)) this->setTextColor(FXRGB(0,0,255));
            }
        }
        else
        {
            if(IsFirst()) AppendIrcText(ev->param1);
        }
        return 1;
    }
    if(ev->eventType == IRC_AWAY)
    {
        if(comparecase(ev->param1, getText()) == 0)
        {
            OnAway();
        }
        return 1;
    }
    if(ev->eventType == IRC_ENDMOTD)
    {
        MakeLastRowVisible(true);
        return 1;
    }
    return 1;
}

long IrcTabItem::OnPipe(FXObject*, FXSelector, void *ptr)
{
    FXString text = *(FXString*)ptr;
    if(coloredNick)
    {
        if(sendPipe && server->GetConnected()) AppendIrcNickText(server->GetNickName(), text, GetNickColor(server->GetNickName()));
        else AppendIrcText(text);
    }
    else AppendIrcText(sendPipe && server->GetConnected() ? "<"+server->GetNickName()+"> "+text : text);
    if(sendPipe && (type == CHANNEL || type == QUERY))
    {
        if(!getApp()->hasTimeout(this, ID_PTIME)) getApp()->addTimeout(this, ID_PTIME);
        pipeStrings.append(text);
    }
    return 1;
}

long IrcTabItem::OnTimeout(FXObject *, FXSelector, void*)
{
    getApp()->addTimeout(this, ID_TIME, 180000);
    if(checkAway)
    {
        if(type == CHANNEL && server->GetConnected())
        {
            server->SendWho(getText());
            server->AddIgnoreCommands("who "+getText());
        }
    }
    else
    {
        if(numberUsers < maxAway)
        {
            checkAway = true;
            if(type == CHANNEL && server->GetConnected())
            {
                server->SendWho(getText());
                server->AddIgnoreCommands("who "+getText());
            }
        }
    }
    return 1;
}

long IrcTabItem::OnPipeTimeout(FXObject*, FXSelector, void*)
{
    if(type == CHANNEL || type == QUERY)
    {
        if(pipeStrings.no() > 3)
        {
            if(pipeStrings[0].length() > maxLen-10-getText().length())
            {
                dxStringArray messages = CutText(pipeStrings[0], maxLen-10-getText().length());
                for(FXint i=0; i<messages.no(); i++)
                {
                    server->SendMsg(getText(), messages[i]);
                }
            }
            else server->SendMsg(getText(), pipeStrings[0]);
            pipeStrings.erase(0);
            getApp()->addTimeout(this, ID_PTIME, 3000);
        }
        else
        {
            while(pipeStrings.no())
            {
                if(pipeStrings[0].length() > maxLen-10-getText().length())
                {
                    dxStringArray messages = CutText(pipeStrings[0], maxLen-10-getText().length());
                    for(FXint i=0; i<messages.no(); i++)
                    {
                        server->SendMsg(getText(), messages[i]);
                    }
                }
                else server->SendMsg(getText(), pipeStrings[0]);
                pipeStrings.erase(0);
            }
        }
    }
    return 1;
}

void IrcTabItem::OnAway()
{
    if(checkAway)
    {
        for(FXint i = 0; i < users->getNumItems(); i++)
        {
            NickInfo nick = server->GetNickInfo(users->getItemText(i));
            if(nick.away)
            {
                if(users->getItemIcon(i) == irc_owner_icon) users->setItemIcon(i, irc_away_owner_icon);
                else if(users->getItemIcon(i) == irc_admin_icon) users->setItemIcon(i, irc_away_admin_icon);
                else if(users->getItemIcon(i) == irc_op_icon) users->setItemIcon(i, irc_away_op_icon);
                else if(users->getItemIcon(i) == irc_voice_icon) users->setItemIcon(i, irc_away_voice_icon);
                else if(users->getItemIcon(i) == irc_halfop_icon) users->setItemIcon(i, irc_away_halfop_icon);
                else if(users->getItemIcon(i) == irc_normal_icon) users->setItemIcon(i, irc_away_normal_icon);
            }
            else
            {
                if(users->getItemIcon(i) == irc_away_owner_icon) users->setItemIcon(i, irc_owner_icon);
                else if(users->getItemIcon(i) == irc_away_admin_icon) users->setItemIcon(i, irc_admin_icon);
                else if(users->getItemIcon(i) == irc_away_op_icon) users->setItemIcon(i, irc_op_icon);
                else if(users->getItemIcon(i) == irc_away_voice_icon) users->setItemIcon(i, irc_voice_icon);
                else if(users->getItemIcon(i) == irc_away_halfop_icon) users->setItemIcon(i, irc_halfop_icon);
                else if(users->getItemIcon(i) == irc_away_normal_icon) users->setItemIcon(i, irc_normal_icon);
            }
        }
    }
    else
    {
        for(FXint i = 0; i < users->getNumItems(); i++)
        {
            if(users->getItemIcon(i) == irc_away_owner_icon) users->setItemIcon(i, irc_owner_icon);
            else if(users->getItemIcon(i) == irc_away_admin_icon) users->setItemIcon(i, irc_admin_icon);
            else if(users->getItemIcon(i) == irc_away_op_icon) users->setItemIcon(i, irc_op_icon);
            else if(users->getItemIcon(i) == irc_away_voice_icon) users->setItemIcon(i, irc_voice_icon);
            else if(users->getItemIcon(i) == irc_away_halfop_icon) users->setItemIcon(i, irc_halfop_icon);
            else if(users->getItemIcon(i) == irc_away_normal_icon) users->setItemIcon(i, irc_normal_icon);
        }
    }
}

long IrcTabItem::OnLeftMouse(FXObject *, FXSelector, void *ptr)
{
    FXEvent* event = (FXEvent*)ptr;
    if(event->moved) return 1;
    FXint pos = text->getPosAt(event->win_x,event->win_y);
    FXint style = text->getStyle(pos);
#ifdef DEBUG
    fxmessage("Style:%d\n", style);
#endif
    if(style == 9)
    {
        text->setDelimiters(" ");
        FXString link;
        text->extractText(link, text->wordStart(pos), text->wordEnd(pos)-text->wordStart(pos));
        FXRex rxl("\\L");        
        if(rxl.match(link.left(1)))
        {
            link = link.mid(1, link.length()-1);
            FXRex rxr("\\W");
            if(rxr.match(link.right(1)))
                link = link.mid(0, link.length()-1);
        }
        LaunchLink(link);
        text->setDelimiters(FXText::textDelimiters);
    }
    return 1;
}

long IrcTabItem::OnRightMouse(FXObject *, FXSelector, void *ptr)
{
    //focus();
    FXEvent* event = (FXEvent*)ptr;
    if(event->moved) return 1;
    FXint index = users->getItemAt(event->win_x,event->win_y);
    if(index >= 0)
    {
        NickInfo nick = server->GetNickInfo(users->getItemText(index));
        nickOnRight = nick;
        FXString flagpath = DXIRC_DATADIR PATHSEPSTRING "icons" PATHSEPSTRING "flags";
        if(FXStat::exists(flagpath+PATHSEPSTRING+nick.host.rafter('.')+".png")) flagicon = MakeIcon(getApp(), flagpath, nick.host.rafter('.')+".png", true);
        else flagicon = MakeIcon(getApp(), flagpath, "unknown.png", true);
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
        new FXMenuCommand(&popup, FXStringFormat(_("User: %s@%s"), nick.user.text(), nick.host.text()), flagicon);
        new FXMenuCommand(&popup, FXStringFormat(_("Realname: %s"), nick.real.text()));
        if(nick.nick != server->GetNickName())
        {
            new FXMenuSeparator(&popup);
            new FXMenuCommand(&popup, _("Query"), NULL, this, ID_NEWQUERY);
            new FXMenuCommand(&popup, _("User information (WHOIS)"), NULL, this, ID_WHOIS);
            if(iamOp) new FXMenuCascade(&popup, _("Operator actions"), NULL, &opmenu);
        }
        popup.create();
        popup.popup(NULL,event->root_x,event->root_y);
        getApp()->runModalWhileShown(&popup);
    }
    return 1;
}

long IrcTabItem::OnNewQuery(FXObject *, FXSelector, void *)
{
    IrcEvent ev;
    ev.eventType = IRC_QUERY;
    ev.param1 = nickOnRight.nick;
    parent->getParent()->getParent()->handle(server, FXSEL(SEL_COMMAND, IrcSocket::ID_SERVER), &ev);
    return 1;
}

long IrcTabItem::OnWhois(FXObject *, FXSelector, void *)
{
    server->SendWhois(nickOnRight.nick);
    return 1;
}



long IrcTabItem::OnOp(FXObject *, FXSelector, void *)
{
    server->SendMode(getText()+" +o "+nickOnRight.nick);
    return 1;
}

long IrcTabItem::OnDeop(FXObject *, FXSelector, void *)
{
    server->SendMode(getText()+" -o "+nickOnRight.nick);
    return 1;
}

long IrcTabItem::OnVoice(FXObject *, FXSelector, void *)
{
    server->SendMode(getText()+" +v "+nickOnRight.nick);
    return 1;
}

long IrcTabItem::OnDevoice(FXObject *, FXSelector, void *)
{
    server->SendMode(getText()+" -v "+nickOnRight.nick);
    return 1;
}

long IrcTabItem::OnKick(FXObject *, FXSelector, void *)
{
    FXDialogBox kickDialog(this, _("Kick dialog"), DECOR_TITLE|DECOR_BORDER, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    FXVerticalFrame *contents = new FXVerticalFrame(&kickDialog, LAYOUT_SIDE_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0, 0, 0, 0, 10, 10, 10, 10, 0, 0);

    FXHorizontalFrame *kickframe = new FXHorizontalFrame(contents, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(kickframe, _("Kick reason:"), 0, JUSTIFY_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    FXTextField *reasonEdit = new FXTextField(kickframe, 25, NULL, 0, FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);

    FXHorizontalFrame *buttonframe = new FXHorizontalFrame(contents, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXButton(buttonframe, _("OK"), NULL, &kickDialog, FXDialogBox::ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0, 0, 0, 0, 32, 32, 5, 5);
    new FXButton(buttonframe, _("Cancel"), NULL, &kickDialog, FXDialogBox::ID_CANCEL, FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0, 0, 0, 0, 32, 32, 5, 5);

    if(kickDialog.execute(PLACEMENT_CURSOR))
    {
        server->SendKick(getText(), nickOnRight.nick, reasonEdit->getText());
    }
    return 1;
}

long IrcTabItem::OnBan(FXObject *, FXSelector, void *)
{
    FXDialogBox banDialog(this, _("Ban dialog"), DECOR_TITLE|DECOR_BORDER, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    FXVerticalFrame *contents = new FXVerticalFrame(&banDialog, LAYOUT_SIDE_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0, 0, 0, 0, 10, 10, 10, 10, 0, 0);

    FXHorizontalFrame *banframe = new FXHorizontalFrame(contents, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(banframe, _("Banmask:"), 0, JUSTIFY_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    FXTextField *banEdit = new FXTextField(banframe, 25, NULL, 0, FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    banEdit->setText(nickOnRight.nick+"!"+nickOnRight.user+"@"+nickOnRight.host);

    FXHorizontalFrame *buttonframe = new FXHorizontalFrame(contents, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXButton(buttonframe, _("OK"), NULL, &banDialog, FXDialogBox::ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0, 0, 0, 0, 32, 32, 5, 5);
    new FXButton(buttonframe, _("Cancel"), NULL, &banDialog, FXDialogBox::ID_CANCEL, FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0, 0, 0, 0, 32, 32, 5, 5);

    if(banDialog.execute(PLACEMENT_CURSOR))
    {
        server->SendMode(getText()+" +b "+banEdit->getText());
    }
    return 1;
}

long IrcTabItem::OnKickban(FXObject *, FXSelector, void *)
{
    FXDialogBox banDialog(this, _("Kick/Ban dialog"), DECOR_TITLE|DECOR_BORDER, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    FXVerticalFrame *contents = new FXVerticalFrame(&banDialog, LAYOUT_SIDE_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0, 0, 0, 0, 10, 10, 10, 10, 0, 0);

    FXHorizontalFrame *kickframe = new FXHorizontalFrame(contents, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(kickframe, _("Kick reason:"), 0, JUSTIFY_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    FXTextField *reasonEdit = new FXTextField(kickframe, 25, NULL, 0, FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);

    FXHorizontalFrame *banframe = new FXHorizontalFrame(contents, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(banframe, _("Banmask:"), 0, JUSTIFY_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    FXTextField *banEdit = new FXTextField(banframe, 25, NULL, 0, FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    banEdit->setText(nickOnRight.nick+"!"+nickOnRight.user+"@"+nickOnRight.host);

    FXHorizontalFrame *buttonframe = new FXHorizontalFrame(contents, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXButton(buttonframe, _("OK"), NULL, &banDialog, FXDialogBox::ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0, 0, 0, 0, 32, 32, 5, 5);
    new FXButton(buttonframe, _("Cancel"), NULL, &banDialog, FXDialogBox::ID_CANCEL, FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0, 0, 0, 0, 32, 32, 5, 5);

    if(banDialog.execute(PLACEMENT_CURSOR))
    {
        server->SendKick(getText(), nickOnRight.nick, reasonEdit->getText());
        server->SendMode(getText()+" +b "+banEdit->getText());
    }
    return 1;
}

long IrcTabItem::OnTopic(FXObject*, FXSelector, void*)
{
    if(editableTopic || iamOp)
    {
        server->SendTopic(getText(), topicline->getText());
    }
    else topicline->setText(topic);
    return 1;
}

void IrcTabItem::OnBan(const FXString &banmask, const FXbool &sign, const FXString &sender)
{
    if(sign)
    {
        FXString nicks = server->GetBannedNick(banmask);
        FXString myNick = server->GetNickName();
        while(nicks.contains(';'))
        {
            for(FXint i=users->getNumItems()-1; i>-1; i--)
            {
                if(nicks.before(';') == users->getItemText(i))
                {
                    if(users->getItemText(i) == myNick) AppendIrcStyledText(FXStringFormat(_("You was banned by %s"), sender.text()), 1);
                    else
                    {
                        if(!IsCommandIgnored("ban")) AppendIrcStyledText(FXStringFormat(_("%s was banned by %s"), users->getItemText(i).text(), sender.text()), 1);
                        //RemoveUser(users->getItemText(i));
                    }
                }
            }
            nicks = nicks.after(';');
        }
    }
}

FXString IrcTabItem::StripColors(const FXString &text, const FXbool stripOther)
{
    FXString newstr;
    FXbool color = false;
    FXint numbers;
    FXint i = 0;
    while(text[i] != '\0') {
        if(text[i] == '\017') //reset
        {
            color = false;
        }
        else if(stripOther && text[i] == '\002')
        {
            //remove bold mark
        }
        else if(stripOther && text[i] == '\037')
        {
            //remove underline mark
        }
        else if(text[i] == '\003') //color
        {
            color = true;
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
            color = false;
            newstr += text[i];
        }
        i++;
    }
    return newstr;
}

FXString IrcTabItem::GetNick(int i)
{
    return users->getItemText(i);
}

FXint IrcTabItem::LaunchLink(const FXString &link)
{
#ifdef WIN32
    return ((FXint)ShellExecuteA(NULL,"open",FXPath::enquote(link).text(),NULL,NULL,SW_NORMAL)) > 32;
#else
    static const char * browsers[]={/*"xdg-open",*/"firefox","konqueror","opera","netscape","dillo","open",NULL};
    FXString path = FXSystem::getExecPath();
    FXString exec;
    for(int i=0; browsers[i]!=NULL; i++)
    {
        exec = FXPath::search(path, browsers[i]);
        if(!exec.empty()) break;
    }
    if(exec.empty()) return 0;
    exec += " "+FXPath::enquote(link);
    pid_t pid = fork();
    if (pid == -1)
    { //Failure delivered to Parent Process
        return 0;
    }
    else if (pid == 0)
    { //Child Process
        int i = sysconf(_SC_OPEN_MAX);
        while (--i >= 3)
        {
            close(i);
        }

        execlp("/bin/sh", "sh", "-c", exec.text(), (char *) 0);
        exit(EXIT_FAILURE);
    }
    else
    { //Parent Process
        return 1;
    }
    return 1;
#endif
}

FXint IrcTabItem::GetNickColor(const FXString &nick)
{
    FXint color = 10;
    color += nick.hash()%8;
    return color;
}

dxStringArray IrcTabItem::CutText(FXString text, FXint len)
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
