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

static const FXListSortFunc sortfuncs[] = {
    FXList::ascendingCase,
    FXList::descendingCase,
    FXList::ascending,
    FXList::descending
};

FXDEFMAP(IrcTabItem) IrcTabItemMap[] = {
    FXMAPFUNC(SEL_COMMAND,              IrcTabItem::ID_COMMANDLINE,     IrcTabItem::OnCommandline),
    FXMAPFUNC(SEL_KEYPRESS,             IrcTabItem::ID_COMMANDLINE,     IrcTabItem::OnKeyPress),
    FXMAPFUNC(SEL_COMMAND,              IrcSocket::ID_SERVER,           IrcTabItem::OnIrcEvent),
    FXMAPFUNC(SEL_TIMEOUT,              IrcTabItem::ID_TIME,            IrcTabItem::OnTimeout),
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
};

FXIMPLEMENT(IrcTabItem, FXTabItem, IrcTabItemMap, ARRAYNUMBER(IrcTabItemMap))

IrcTabItem::IrcTabItem(FXTabBook *tab, const FXString &tabtext, FXIcon *ic=0, FXuint opts=TAB_TOP_NORMAL, TYPE typ=CHANNEL, IrcSocket *sock=NULL, FXbool oswnd=false, FXbool uswn=true, FXbool logg=false, FXString cmdlst="", FXString lpth="", FXint maxa=200, IrcColor clrs=IrcColor(), FXString nichar=":", FXFont *fnt=NULL, FXbool scmd=false, FXbool slst=false)
    : FXTabItem(tab, tabtext, ic, opts), parent(tab), server(sock), type(typ), usersShown(uswn), logging(logg), ownServerWindow(oswnd), sameCmd(scmd), sameList(slst), colors(clrs),
    commandsList(cmdlst), logPath(lpth), maxAway(maxa), nickCompletionChar(nichar), logstream(NULL)
{
    currentPosition = 0;
    historyMax = 25;
    numberUsers = 0;
    checkAway = false;
    iamOp = false;
    topic = _("No topic is set");
    editableTopic = true;

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
    text = new FXText(textframe, NULL, 0, FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y|TEXT_READONLY|TEXT_WORDWRAP|TEXT_SHOWACTIVE|TEXT_AUTOSCROLL);
    text->setFont(fnt);

    usersframe = new FXVerticalFrame(splitter, FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_FIX_WIDTH);
    users = new FXList(usersframe, this, ID_USERS, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    users->setSortFunc(sortfuncs[0]);
    users->setScrollStyle(HSCROLLING_OFF);
    if(sameList) users->setFont(fnt);
    if(type != CHANNEL || !usersShown)
    {
        usersframe->hide();
        users->hide();
    }

    commandline = new FXTextField(mainframe, 25, this, ID_COMMANDLINE, TEXTFIELD_ENTER_ONLY|FRAME_SUNKEN|JUSTIFY_LEFT|LAYOUT_FILL_X|LAYOUT_BOTTOM, 0, 0, 0, 0, 1, 1, 1, 1);
    if(sameCmd) commandline->setFont(fnt);

    for(int i=0; i<8; i++)
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
    getApp()->removeTimeout(this, ID_TIME);
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
}

void IrcTabItem::SetTextBackColor(FXColor clr)
{
    for(int i=0; i<8; i++)
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
    if(sameList) users->setFont(fnt);
}

void IrcTabItem::SetSameCmd(FXbool scmd)
{
    sameCmd = scmd;
}

void IrcTabItem::SetSameList(FXbool slst)
{
    sameList = slst;
}

void IrcTabItem::AppendIrcText(FXString msg)
{
    text->appendText("["+FXSystem::time("%H:%M:%S", FXSystem::now()) +"] ");
    FXRex rx("(\\002|\\003|\\037)");
    if(rx.match(msg)) //contains mirc colors,styles
    {
        msg = StripColors(msg, false);
        FXbool bold = false;
        FXbool under = false;
        FXint i = 0;
        while(msg[i] != '\0') {
            if(msg[i] == '\002') {
                bold = !bold;
            }
            else if(msg[i] == '\037') {
                under = !under;
            }
            else {
                FXString txt;
                txt += msg[i];
                if(bold && under) text->appendStyledText(txt, 7);
                else if(bold && !under) text->appendStyledText(txt, 5);
                else if(!bold && under) text->appendStyledText(txt, 6);
                else text->appendText(txt);
            }
            i++;
        }
        text->appendText("\n");
        MakeLastRowVisible(false);
    }
    else {
        text->appendText(msg+"\n");
        MakeLastRowVisible(false);
    }
    this->LogLine(StripColors(msg, true));
}

void IrcTabItem::AppendIrcStyledText(FXString styled, FXint stylenum)
{
    text->appendText("["+FXSystem::time("%H:%M:%S", FXSystem::now()) +"] ");
    text->appendStyledText(StripColors(styled, true)+"\n", stylenum);
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
        if(IsCurrent())
        {
            if((textScrollbar->getPosition()+textScrollbar->getHeight()+textScrollbar->getLine())*100 > textScrollbar->getRange()*95)
            {
                text->makePositionVisible(text->rowStart(text->getLength()));
            }
        }
        else text->makePositionVisible(text->rowStart(text->getLength()));
    }
}

FXString IrcTabItem::CreateModes(FXchar sign, FXchar mode, FXString nicks)
{
    FXString modes;
    modes += sign;
    FXString tomode;
    if(!nicks.contains(' ')) return modes+mode+" "+nicks;
    if(nicks.right(1) != " ") nicks.append(" ");
    while(nicks.contains(' '))
    {
        modes += mode;
        tomode += nicks.before(' ')+" ";
        nicks = nicks.after(' ');
    }
    return modes+" "+tomode;
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
    FXString command = (commandtext[0] == '/' ? commandtext.after('/').before(' ').lower() : "");
    if(server->GetConnected())
    {
        if(commandtext[0] == '/')
        {
            if(command == "admin")
            {
                server->SendAdmin(commandtext.after(' '));
                commandline->setText("");
                return 1;
            }
            if(command == "away")
            {
                server->SendAway(commandtext.after(' '));
                commandline->setText("");
                return 1;
            }
            if(command == "banlist")
            {
                FXString channel = commandtext.after(' ');
                if(channel.empty() && type == CHANNEL) server->SendBanlist(getText());
                else if(!IsChannel(channel) && type != CHANNEL) AppendIrcStyledText(_("/banlist <channel>, see banlist for channel."), 4);
                else server->SendBanlist(channel);
                commandline->setText("");
                return 1;
            }
            if(command == "commands")
            {
                FXString commandstr = _("Available commnads: ");
                for(FXint i=0; i < server->commands.no(); i++)
                {
                    if(server->commands[i] != "commands") commandstr += server->commands[i].upper()+(i != server->commands.no() - 1? ", " : "");
                }
                AppendIrcStyledText(commandstr, 3);
                commandline->setText("");
                return 1;
            }
            if(command == "ctcp")
            {
                FXString to = commandtext.after(' ').before(' ');
                FXString msg = commandtext.after(' ', 2);
                if(to.empty() || msg.empty()) AppendIrcStyledText(_("/ctcp <nick> <message>, sends a CTCP message to a user."), 4);
                else server->SendCtcp(to, msg);
                commandline->setText("");
                return 1;
            }
            if(command == "deop")
            {
                FXString params = commandtext.after(' ');
                if(type == CHANNEL)
                {
                    if(params.empty()) AppendIrcStyledText(_("/deop <nicks>, remove operator status from one or more nicks."), 4);
                    else if(IsChannel(params) && params.after(' ').empty()) AppendIrcStyledText(_("/deop <channel> <nicks>, remove operator status from one or more nicks."), 4);
                    else if(IsChannel(params) && !params.after(' ').empty())
                    {
                        FXString channel = params.before(' ');
                        FXString nicks = params.after(' ');
                        FXString modeparams = CreateModes('-', 'o', nicks);
                        server->SendMode(channel+" "+modeparams);
                    }
                    else
                    {
                        FXString channel = getText();
                        FXString nicks = params;
                        FXString modeparams = CreateModes('-', 'o', nicks);
                        server->SendMode(channel+" "+modeparams);
                    }
                }
                else
                {
                    if(IsChannel(params) && !params.after(' ').empty())
                    {
                        FXString channel = params.before(' ');
                        FXString nicks = params.after(' ');
                        FXString modeparams = CreateModes('-', 'o', nicks);
                        server->SendMode(channel+" "+modeparams);
                    }
                    else AppendIrcStyledText(_("/deop <channel> <nicks>, remove operator status from one or more nicks."), 4);
                }
                commandline->setText("");
                return 1;
            }
            if(command == "devoice")
            {
                FXString params = commandtext.after(' ');
                if(type == CHANNEL)
                {
                    if(params.empty()) AppendIrcStyledText(_("/devoice <nicks>, remove voice from one or more nicks."), 4);
                    else if(IsChannel(params) && params.after(' ').empty()) AppendIrcStyledText(_("/devoice <channel> <nicks>, remove voice from one or more nicks."), 4);
                    else if(IsChannel(params) && !params.after(' ').empty())
                    {
                        FXString channel = params.before(' ');
                        FXString nicks = params.after(' ');
                        FXString modeparams = CreateModes('-', 'v', nicks);
                        server->SendMode(channel+" "+modeparams);
                    }
                    else
                    {
                        FXString channel = getText();
                        FXString nicks = params;
                        FXString modeparams = CreateModes('-', 'v', nicks);
                        server->SendMode(channel+" "+modeparams);
                    }
                }
                else
                {
                    if(IsChannel(params) && !params.after(' ').empty())
                    {
                        FXString channel = params.before(' ');
                        FXString nicks = params.after(' ');
                        FXString modeparams = CreateModes('-', 'v', nicks);
                        server->SendMode(channel+" "+modeparams);
                    }
                    else AppendIrcStyledText(_("/devoice <channel> <nicks>, remove voice from one or more nicks."), 4);
                }
                commandline->setText("");
                return 1;
            }
            if(command == "invite")
            {
                FXString params = commandtext.after(' ');
                if(params.empty()) AppendIrcStyledText(_("/invite <nick> <channel>, invites someone to a channel."), 4);
                else if(IsChannel(params) && params.after(' ').empty()) AppendIrcStyledText(_("/invite <nick> <channel>, invites someone to a channel."), 4);
                else
                {
                    FXString nick = params.before(' ');
                    FXString channel = params.after(' ');
                    server->SendInvite(nick, channel);
                }
                commandline->setText("");
                return 1;
            }
            if(command == "join")
            {
                FXString channel = commandtext.after(' ');
                if(!IsChannel(channel)) AppendIrcStyledText(_("/join <channel>, join a channel."), 4);
                else server->SendJoin(channel);
                commandline->setText("");
                return 1;
            }
            if(command == "kick")
            {
                FXString params = commandtext.after(' ');
                if(type == CHANNEL)
                {
                    if(params.empty()) AppendIrcStyledText(_("/kick <nick>, kick a user from a channel."), 4);
                    else if(IsChannel(params) && params.after(' ').empty()) AppendIrcStyledText(_("/kick <channel> <nick>, kick a user from a channel."), 4);
                    else if(IsChannel(params) && !params.after(' ').empty())
                    {
                        FXString channel = params.before(' ');
                        FXString nick = params.after(' ');
                        FXString reason = params.after(' ', 2);
                        server->SendKick(channel, nick, reason);
                    }
                    else
                    {
                        FXString channel = getText();
                        FXString nick = params.before(' ');
                        FXString reason = params.after(' ');
                        server->SendKick(channel, nick, reason);
                    }
                }
                else
                {
                    if(IsChannel(params) && !params.after(' ').empty())
                    {
                        FXString channel = params.before(' ');
                        FXString nick = params.after(' ');
                        FXString reason = params.after(' ', 2);
                        server->SendKick(channel, nick, reason);
                    }
                    else AppendIrcStyledText(_("/kick <channel> <nick>, kick a user from a channel."), 4);
                }
                commandline->setText("");
                return 1;
            }
            if(command == "kill")
            {
                FXString params = commandtext.after(' ');
                FXString nick = params.before(' ');
                FXString reason = params.after(' ');
                if(params.empty()) AppendIrcStyledText(_("/kill <user> [reason], kill a user from the network."), 4);
                else server->SendKill(nick, reason);
                commandline->setText("");
                return 1;
            }
            if(command == "list")
            {
                server->SendList(commandtext.after(' '));
                commandline->setText("");
                return 1;
            }
            if(command == "me")
            {
                FXString params = commandtext.after(' ');
                if(type == CHANNEL)
                {
                    if(params.empty()) AppendIrcStyledText(_("/me <message>, sends the action to the current channel."), 4);
                    else if(IsChannel(params) && !params.after(' ').empty())
                    {
                        FXString channel = params.before(' ');
                        FXString message = params.after(' ');
                        server->SendMe(channel, message);
                        if(channel == getText()) AppendIrcStyledText(server->GetNickName()+" "+message, 2);
                    }
                    else
                    {
                        server->SendMe(getText(), params);
                        AppendIrcStyledText(server->GetNickName()+" "+params, 2);
                    }
                }
                else
                {
                    if(!params.after(' ').empty())
                    {
                        FXString to = params.before(' ');
                        FXString message = params.after(' ');
                        server->SendMe(to, message);
                    }
                    else AppendIrcStyledText(_("/me <to> <message>, sends the action."), 4);
                }
                commandline->setText("");
                return 1;
            }
            if(command == "msg")
            {
                FXString params = commandtext.after(' ');
                FXString to = params.before(' ');
                FXString message = params.after(' ');
                if(!to.empty() && !message.empty())
                {
                    server->SendMsg(to, message);
                    if(to == getText()) AppendIrcText("<"+server->GetNickName()+"> "+message);
                }
                else AppendIrcStyledText(_("/msg <nick/channel> <message>, sends a normal message."), 4);
                commandline->setText("");
                return 1;
            }
            if(command == "names")
            {
                FXString params = commandtext.after(' ');
                if(type == CHANNEL)
                {
                    if(params.empty()) server->SendNames(getText());
                    else server->SendNames(params);
                }
                else
                {
                    if(params.empty()) AppendIrcStyledText(_("/names <channel>, for nicks on a channel."), 4);
                    else server->SendNames(params);
                }
                commandline->setText("");
                return 1;
            }
            if(command == "nick")
            {
                FXString nick = commandtext.after(' ');
                if(nick.empty()) AppendIrcStyledText(_("/nick <nick>, change nick."), 4);
                else
                {
                    server->SendNick(nick);
                }
                commandline->setText("");
                return 1;
            }
            if(command == "notice")
            {
                FXString params = commandtext.after(' ');
                FXString to = params.before(' ');
                FXString message = params.after(' ');
                if(!to.empty() && !message.empty())
                {
                    server->SendNotice(to, message);
                    if(to == getText()) AppendIrcStyledText(FXStringFormat(_("%s's NOTICE: "), server->GetNickName().text())+message, 2);
                }
                else AppendIrcStyledText(_("/notice <nick/channel> <message>, sends a notice."), 4);
                commandline->setText("");
                return 1;
            }
            if(command == "op")
            {
                FXString params = commandtext.after(' ');
                if(type == CHANNEL)
                {
                    if(params.empty()) AppendIrcStyledText(_("/op <nicks>, give operator status for one or more nicks."), 4);
                    else if(IsChannel(params) && params.after(' ').empty()) AppendIrcStyledText(_("/op <channel> <nicks>, give operator status for one or more nicks."), 4);
                    else if(IsChannel(params) && !params.after(' ').empty())
                    {
                        FXString channel = params.before(' ');
                        FXString nicks = params.after(' ');
                        FXString modeparams = CreateModes('+', 'o', nicks);
                        server->SendMode(channel+" "+modeparams);
                    }
                    else
                    {
                        FXString channel = getText();
                        FXString nicks = params;
                        FXString modeparams = CreateModes('+', 'o', nicks);
                        server->SendMode(channel+" "+modeparams);
                    }
                }
                else
                {
                    if(IsChannel(params) && !params.after(' ').empty())
                    {
                        FXString channel = params.before(' ');
                        FXString nicks = params.after(' ');
                        FXString modeparams = CreateModes('+', 'o', nicks);
                        server->SendMode(channel+" "+modeparams);
                    }
                    else AppendIrcStyledText(_("/op <channel> <nicks>, give operator status for one or more nicks."), 4);
                }
                commandline->setText("");
                return 1;
            }
            if(command == "oper")
            {
                FXString params = commandtext.after(' ');
                FXString login = params.before(' ');
                FXString password = params.after(' ');
                if(!login.empty() && !password.empty()) server->SendOper(login, password);
                else AppendIrcStyledText(_("/oper <login> <password>, oper up."), 4);
                commandline->setText("");
                return 1;
            }
            if(command == "part")
            {
                if(type == CHANNEL)
                {
                    if(commandtext.after(' ').empty()) server->SendPart(getText());
                    else server->SendPart(getText(), commandtext.after(' '));
                }
                commandline->setText("");
                return 1;
            }
            if(command == "quit")
            {
                commandline->setText("");
                if(commandtext.after(' ').empty()) server->Disconnect();
                else server->Disconnect(commandtext.after(' '));
                parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_TABQUIT), NULL);
                return 1;
            }
            if(command == "raw")
            {
                server->SendRaw(commandtext.after(' '));
                commandline->setText("");
                return 1;
            }
            if(command == "topic")
            {
                FXString params = commandtext.after(' ');
                if(type == CHANNEL)
                {
                    if(params.empty()) server->SendTopic(getText());
                    else if(IsChannel(params) && !params.after(' ').empty())
                    {
                        FXString channel = params.before(' ');
                        FXString topic = params.after(' ');
                        server->SendTopic(channel, topic);
                    }
                    else server->SendTopic(getText(), params);
                }
                else {
                    if(IsChannel(params))
                    {
                        FXString channel = params.before(' ');
                        FXString topic = params.after(' ');
                        server->SendTopic(channel, topic);
                    }
                    else AppendIrcStyledText(_("/topic <channel> [topic], view or change channel topic."), 4);
                }
                commandline->setText("");
                return 1;
            }
            if(command == "voice")
            {
                FXString params = commandtext.after(' ');
                if(type == CHANNEL)
                {
                    if(params.empty()) AppendIrcStyledText(_("/voice <nicks>, give voice for one or more nicks."), 4);
                    else if(IsChannel(params) && params.after(' ').empty()) AppendIrcStyledText(_("/voice <channel> <nicks>, give voice for one or more nicks."), 4);
                    else if(IsChannel(params) && !params.after(' ').empty())
                    {
                        FXString channel = params.before(' ');
                        FXString nicks = params.after(' ');
                        FXString modeparams = CreateModes('+', 'v', nicks);
                        server->SendMode(channel+" "+modeparams);
                    }
                    else
                    {
                        FXString channel = getText();
                        FXString nicks = params;
                        FXString modeparams = CreateModes('+', 'v', nicks);
                        server->SendMode(channel+" "+modeparams);
                    }
                }
                else
                {
                    if(IsChannel(params) && !params.after(' ').empty())
                    {
                        FXString channel = params.before(' ');
                        FXString nicks = params.after(' ');
                        FXString modeparams = CreateModes('+', 'v', nicks);
                        server->SendMode(channel+" "+modeparams);
                    }
                    else AppendIrcStyledText(_("/voice <channel> <nicks>, give voice for one or more nicks."), 4);
                }
                commandline->setText("");
                return 1;
            }
            if(command == "wallops")
            {
                FXString params = commandtext.after(' ');
                if(params.empty()) AppendIrcStyledText(_("/wallops <message>, send wallop message."), 4);
                else server->SendWallops(params);
                commandline->setText("");
                return 1;
            }
            if(command == "who")
            {
                FXString params = commandtext.after(' ');
                if(params.empty()) AppendIrcStyledText(_("/who <mask> [o], search for mask on network, if o is supplied, only search for opers."), 4);
                else server->SendWho(params);
                commandline->setText("");
                return 1;
            }
            if(command == "whois")
            {
                FXString params = commandtext.after(' ');
                if(params.empty()) AppendIrcStyledText(_("/whois <nick>, whois nick."), 4);
                else server->SendWhois(params);
                commandline->setText("");
                return 1;
            }
            if(command == "whowas")
            {
                FXString params = commandtext.after(' ');
                if(params.empty()) AppendIrcStyledText(_("/whowas <nick>, whowas nick."), 4);
                else server->SendWhowas(params);
                commandline->setText("");
                return 1;
            }
            AppendIrcStyledText(FXStringFormat(_("Unknown command '%s', type /commands for available commands"), command.text()), 4);
            commandline->setText("");
            return 1;
        }
        else
        {
            if (command.empty() && type != SERVER && !commandtext.empty())
            {
                AppendIrcText("<"+server->GetNickName()+"> "+commandtext);
                server->SendMsg(getText(), commandtext);
                commandline->setText("");
                return 1;
            }
        }
    }
    else
    {
        if(command == "commands")
        {
            FXString commandstr = _("Available commnads: ");
            for(FXint i=0; i < server->commands.no(); i++)
            {
                if(server->commands[i] != "commands") commandstr += server->commands[i].upper()+(i != server->commands.no() - 1? ", " : "");
            }
            AppendIrcStyledText(commandstr, 3);
            commandline->setText("");
            return 1;
        }
        else
        {
            AppendIrcStyledText(_("You aren't connected"), 4);
            parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CONNECT), NULL);
            commandline->setText("");
            return 1;
        }
    }
    commandline->setText("");
    return 1;
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
                    for (FXint i = 0; i < server->commands.no(); i++)
                    {
                        if(commandline->getText().after('/').before(' ').lower() == server->commands.at(i).lower())
                        {
                            if((i+1) < server->commands.no()) commandline->setText("/"+server->commands.at(++i)+" ");
                            else commandline->setText("/"+server->commands.at(0)+" ");
                            break;
                        }
                        else if(commandline->getText().after('/').lower() == server->commands.at(i).left(commandline->getText().after('/').length()).lower())
                        {
                            commandline->setText("/"+server->commands.at(i)+" ");
                            break;
                        }
                    }
                    return 1;
                }
                if(commandline->getText()[0] != '/' && commandline->getText().after(' ').empty())
                {
                    for(FXint j = 0; j < users->getNumItems() ; j++)
                    {
                        if(commandline->getText().lower() == GetNick(j).left(commandline->getText().length()).lower())
                        {
                            commandline->setText(GetNick(j)+nickCompletionChar+" ");
                        }
                        else if(commandline->getText().section(nickCompletionChar, 0, 1).lower() == GetNick(j).lower())
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
                        if(toCompletion.lower() == GetNick(j).lower())
                        {
                            if((j+1) < users->getNumItems()) commandline->setText(line+GetNick(++j));
                            else commandline->setText(line+GetNick(0));
                            break;
                        }
                        else if(toCompletion.lower() == GetNick(j).left(toCompletion.length()).lower())
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

FXbool IrcTabItem::IsCurrent()
{
    FXint index = parent->getCurrent()*2;
    if((IrcTabItem *)parent->childAtIndex(index) == this) return true;
    return false;
}

FXbool IrcTabItem::IsNoCurrent()
{
    FXint index = parent->getCurrent()*2;
    if(((IrcTabItem *)parent->childAtIndex(index))->GetServerName() == server->GetServerName()) return false;
    return true;
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
    if(users->findItem(nick) != -1)
    {
        RemoveUser(nick);
        AddUser(newnick);
    }
}

long IrcTabItem::OnIrcEvent(FXObject *, FXSelector, void *data)
{
    IrcEvent *ev = (IrcEvent *) data;
    if(ev->eventType == IRC_PRIVMSG)
    {
        if((ev->param2.lower() == getText().lower() && type == CHANNEL) || (ev->param1 == getText() && type == QUERY && ev->param2 == server->GetNickName()))
        {
            if(ev->param3.contains(server->GetNickName())) AppendIrcStyledText("<"+ev->param1+"> "+ev->param3, 8);
            else AppendIrcText("<"+ev->param1+"> "+ev->param3);
            if(FXRGB(255,0,0) != this->getTextColor() && !IsCurrent())
            {
                if(ev->param3.contains(server->GetNickName())) this->setTextColor(FXRGB(255,0,0));
                else this->setTextColor(FXRGB(0,0,255));
            }
        }
        return 1;
    }
    if(ev->eventType == IRC_ACTION)
    {
        if((ev->param2.lower() == getText().lower() && type == CHANNEL) || (ev->param1 == getText() && type == QUERY && ev->param2 == server->GetNickName()))
        {
            if(!IsCommandIgnored("me"))
            {
                AppendIrcStyledText(ev->param1+" "+ev->param3, 2);
                if(FXRGB(255,0,0) != this->getTextColor() && !IsCurrent())
                {
                    if(ev->param3.contains(server->GetNickName())) this->setTextColor(FXRGB(255,0,0));
                    else this->setTextColor(FXRGB(0,0,255));
                }
            }
        }
        return 1;
    }
    if(ev->eventType == IRC_CTCPREPLY)
    {
        if(type == SERVER || IsCurrent() || IsNoCurrent())
        {
            if(!IsCommandIgnored("ctcp")) AppendIrcStyledText(FXStringFormat(_("CTCP %s reply from %s: %s"), GetParam(ev->param2, 1, false).text(), ev->param1.text(), GetParam(ev->param2, 2, true).text()), 2);
        }
        return 1;
    }
    if(ev->eventType == IRC_CTCPREQUEST)
    {
        if(type == SERVER || IsCurrent() || IsNoCurrent())
        {
            if(!IsCommandIgnored("ctcp")) AppendIrcStyledText(FXStringFormat(_("CTCP %s request from %s"), ev->param2.text(), ev->param1.text()), 2);
        }
        return 1;
    }
    if(ev->eventType == IRC_JOIN)
    {
        if(ev->param2.lower() == getText().lower() && ev->param1 != server->GetNickName())
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
        if(ev->param2.lower() == getText().lower())
        {
            if(ev->param3.empty() && !IsCommandIgnored("part")) AppendIrcStyledText(FXStringFormat(_("%s has parted %s"), ev->param1.text(), ev->param2.text()), 1);
            else if(!IsCommandIgnored("part")) AppendIrcStyledText(FXStringFormat(_("%s has parted %s (%s)"), ev->param1.text(), ev->param2.text(), ev->param3.text()), 1);
            RemoveUser(ev->param1);
        }
        return 1;
    }
    if(ev->eventType == IRC_CHNOTICE)
    {
        if((ev->param2.lower() == getText().lower() && type == CHANNEL) || (ev->param1 == getText() && type == QUERY && ev->param2 == server->GetNickName()))
        {
            if(!IsCommandIgnored("notice"))
            {
                AppendIrcStyledText(FXStringFormat(_("%s's NOTICE: %s"), ev->param1.text(), ev->param3.text()), 2);
                if(FXRGB(255,0,0) != this->getTextColor() && !IsCurrent())
                {
                    if(ev->param3.contains(server->GetNickName())) this->setTextColor(FXRGB(255,0,0));
                    else this->setTextColor(FXRGB(0,0,255));
                }
            }
        }
        else if(type == SERVER || IsCurrent() || IsNoCurrent())
        {
            if(!IsCommandIgnored("notice")) AppendIrcStyledText(FXStringFormat(_("%s's NOTICE: %s"), ev->param1.text(), ev->param3.text()), 3);
        }
        return 1;
    }
    if(ev->eventType == IRC_NOTICE)
    {
        if(type == SERVER || IsCurrent() || IsNoCurrent())
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
            if(ev->param2 == server->GetNickName() && !IsCommandIgnored("nick")) AppendIrcStyledText(FXStringFormat(_("you changes nick to %s"), ev->param2.text()), 1);
            else if(!IsCommandIgnored("nick")) AppendIrcStyledText(FXStringFormat(_("%s changes nick to %s"), ev->param1.text(), ev->param2.text()), 1);
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
        if(ev->param2.lower() == getText().lower())
        {
            AppendIrcText(FXStringFormat(_("%s Set new topic for %s: %s"), ev->param1.text(), ev->param2.text(), ev->param3.text()));
            topic = ev->param3;
            topicline->setText(topic);            
        }
        return 1;
    }
    if(ev->eventType == IRC_INVITE)
    {
        if(type == SERVER || IsCurrent() || IsNoCurrent())
        {
            AppendIrcStyledText(FXStringFormat(_("%s invites you to: %s"), ev->param1.text(), ev->param3.text()), 3);
        }
        return 1;
    }
    if(ev->eventType == IRC_KICK)
    {
        if(ev->param3.lower() == getText().lower())
        {
            if(ev->param2 != server->GetNickName())
            {
                if(ev->param4.empty()) AppendIrcStyledText(FXStringFormat(_("%s was kicked from: %s"), ev->param2.text(), ev->param3.text()), 1);
                else AppendIrcStyledText(FXStringFormat(_("%s was kicked from: %s (%s)"), ev->param2.text(), ev->param3.text(), ev->param4.text()), 1);
                RemoveUser(ev->param2);
            }
        }
        if(ev->param2 == server->GetNickName() && (type == SERVER || IsCurrent() || IsNoCurrent()))
        {
            if(ev->param4.empty()) AppendIrcStyledText(FXStringFormat(_("you was kicked from: %s"), ev->param3.text()), 1);
            else AppendIrcStyledText(FXStringFormat(_("you was kicked from: %s (%s)"), ev->param3.text(), ev->param4.text()), 1);
        }
        return 1;
    }
    if(ev->eventType == IRC_MODE)
    {
        if(type == SERVER || IsCurrent() || IsNoCurrent())
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
        if(channel.lower() == getText().lower())
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
                        FXString nick = GetParam(args, argsiter, false);
                        RemoveUser(nick);
                        sign ? AddUser("!"+nick) : AddUser(nick);
                        argsiter++;
                    }break;
                    case 'o': //op
                    {
                        FXString nick = GetParam(args, argsiter, false);
                        RemoveUser(nick);
                        sign ? AddUser("@"+nick) : AddUser(nick);
                        if (server->GetNickName() == nick) sign ? iamOp = true : iamOp = false;
                        argsiter++;
                    }break;
                    case 'v': //voice
                    {
                        FXString nick = GetParam(args, argsiter, false);
                        RemoveUser(nick);
                        sign ? AddUser("+"+nick) : AddUser(nick);
                        argsiter++;
                    }break;
                    case 'h': //halfop
                    {
                        FXString nick = GetParam(args, argsiter, false);
                        RemoveUser(nick);
                        sign ? AddUser("%"+nick) : AddUser(nick);
                        argsiter++;
                    }break;
                    case 'b': //ban
                    {
                        FXString banmask = GetParam(args, argsiter, false);
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
        if(channel.lower() == getText().lower())
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
                if(FXRGB(0,0,0) == this->getTextColor() && !IsCurrent()) this->setTextColor(FXRGB(0,0,255));
            }
        }
        else
        {
            if(IsCurrent() || IsNoCurrent()) AppendIrcText(ev->param1);
        }
        return 1;
    }
    if(ev->eventType == IRC_CONNECT)
    {
        if(type == SERVER || IsCurrent() || IsNoCurrent())
        {
            AppendIrcStyledText(ev->param1, 3);
        }
        return 1;
    }
    if(ev->eventType == IRC_SERVERERROR || ev->eventType == IRC_ERROR || ev->eventType == IRC_DISCONNECT)
    {
        if(ownServerWindow)
        {
            if(type == SERVER)
            {
                AppendIrcStyledText(ev->param1, 4);
                if(FXRGB(0,0,0) == this->getTextColor() && !IsCurrent()) this->setTextColor(FXRGB(0,0,255));
            }
        }
        else
        {
            if(IsCurrent() || IsNoCurrent()) AppendIrcStyledText(ev->param1, 4);
        }
        return 1;
    }
    if(ev->eventType == IRC_UNKNOWN)
    {
        if(ownServerWindow)
        {
            if(type == SERVER)
            {
                AppendIrcStyledText(FXStringFormat(_("Unhandled command '%s' params: %s"), ev->param1.text(), ev->param2.text()), 4);
                if(FXRGB(0,0,0) == this->getTextColor() && !IsCurrent()) this->setTextColor(FXRGB(0,0,255));
            }
        }
        else
        {
            if(IsCurrent() || IsNoCurrent()) AppendIrcStyledText(FXStringFormat(_("Unhandled command '%s' params: %s"), ev->param1.text(), ev->param2.text()), 4);
        }
        return 1;
    }
    if(ev->eventType == IRC_301)
    {
        if(users->findItem(ev->param1) == -1)
        {
            AppendIrcStyledText(FXStringFormat(_("%s is away: %s"),ev->param1.text(), ev->param2.text()), 1);
        }
        return 1;
    }
    if(ev->eventType == IRC_331 || ev->eventType == IRC_332 || ev->eventType == IRC_333)
    {
        if(ev->param1.lower() == getText().lower())
        {
            AppendIrcText(ev->param2);
            if(ev->eventType == IRC_331)
            {
                topic = StripColors(ev->param2, true);
                topicline->setText(topic);
            }
            if(ev->eventType == IRC_332)
            {
                topic = StripColors(GetParam(ev->param2, 2, true, ':').after(' '), true);
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
        if(channel.lower() == getText().lower())
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
                if(server->FindTarget((IrcTabItem *)parent->childAtIndex(i)) && ((IrcTabItem *)parent->childAtIndex(i))->getText().lower() == channel.lower())
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
        if(ev->param1.lower() == getText().lower())
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
                if(FXRGB(0,0,0) == this->getTextColor() && !IsCurrent()) this->setTextColor(FXRGB(0,0,255));
            }
        }
        else
        {
            if(IsCurrent() || IsNoCurrent()) AppendIrcText(ev->param1);
        }
        return 1;
    }
    if(ev->eventType == IRC_AWAY)
    {
        if(ev->param1.lower() == getText().lower())
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
    ev.ircSocket = server;
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
    new FXButton(buttonframe, _("Cancel"), NULL, &kickDialog, FXDialogBox::ID_CANCEL, BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0, 0, 0, 0, 32, 32, 5, 5);

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
    new FXButton(buttonframe, _("Cancel"), NULL, &banDialog, FXDialogBox::ID_CANCEL, BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0, 0, 0, 0, 32, 32, 5, 5);

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
    new FXButton(buttonframe, _("Cancel"), NULL, &banDialog, FXDialogBox::ID_CANCEL, BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0, 0, 0, 0, 32, 32, 5, 5);

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

FXString IrcTabItem::GetParam(FXString toParse, int n, bool toEnd)
{
    return GetParam(toParse, n, toEnd, ' ');
}

FXString IrcTabItem::GetParam(FXString toParse, int n, bool toEnd, const FXchar &separator)
{
    if (toEnd) {
        return toParse.after(separator, n-1);
    }
    else {
        return toParse.before(separator, n).rafter(separator);
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
