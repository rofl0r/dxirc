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

FXDEFMAP(DccSendDialog) DccSendDialogMap[] = {
    FXMAPFUNC(SEL_COMMAND,  DccSendDialog::ID_FILE,     DccSendDialog::OnFile),
    FXMAPFUNC(SEL_COMMAND,  DccSendDialog::ID_SEND,     DccSendDialog::OnSend),
    FXMAPFUNC(SEL_COMMAND,  DccSendDialog::ID_CANCEL,   DccSendDialog::OnCancel),
    FXMAPFUNC(SEL_CLOSE,    0,                          DccSendDialog::OnCancel),
    FXMAPFUNC(SEL_KEYPRESS, 0,                          DccSendDialog::OnKeyPress)
};

FXIMPLEMENT(DccSendDialog, FXDialogBox, DccSendDialogMap, ARRAYNUMBER(DccSendDialogMap))

DccSendDialog::DccSendDialog(FXMainWindow* owner, FXString nick)
        : FXDialogBox(owner, FXStringFormat(_("Send file to %s"), nick.text()), DECOR_RESIZE|DECOR_TITLE|DECOR_BORDER, 0,0,0,0, 0,0,0,0, 0,0)
{
    mainFrame = new FXVerticalFrame(this, LAYOUT_FILL_X|LAYOUT_FILL_Y);

    fileFrame = new FXHorizontalFrame(mainFrame, LAYOUT_FILL_X);
    new FXLabel(fileFrame, _("File:"));
    fileText = new FXTextField(fileFrame, 25, NULL, 0, TEXTFIELD_READONLY|FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_X);
    buttonFile = new FXButton(fileFrame, "...", NULL, this, ID_FILE, FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 10,10,2,5);

    passiveFrame = new FXHorizontalFrame(mainFrame, LAYOUT_FILL_X);
    checkPassive = new FXCheckButton(passiveFrame, _("Send passive"), NULL, 0);

    buttonFrame = new FXHorizontalFrame(mainFrame, LAYOUT_FILL_X|PACK_UNIFORM_WIDTH);
    buttonCancel = new FXButton(buttonFrame, _("&Cancel"), NULL, this, ID_CANCEL, FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,5);
    buttonSend = new FXButton(buttonFrame, _("&Send file"), NULL, this, ID_SEND, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,5);
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
        fileText->setText(dialog.getFilename());
    }
    return getApp()->runModalFor(this);
}

long DccSendDialog::OnFile(FXObject*, FXSelector, void*)
{
    FXFileDialog dialog(this, _("Select file"));
    if(dialog.execute())
    {
        fileText->setText(dialog.getFilename());
    }
    return 1;
}

long DccSendDialog::OnSend(FXObject*, FXSelector, void*)
{
    getApp()->stopModal(this,TRUE);
    hide();
    return 1;
}

long DccSendDialog::OnCancel(FXObject*, FXSelector, void*)
{
    getApp()->stopModal(this,FALSE);
    hide();
    return 1;
}

long DccSendDialog::OnKeyPress(FXObject *sender, FXSelector sel, void *ptr)
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
    FXMAPFUNC(SEL_COMMAND,              IrcTabItem::ID_COMMANDLINE,     IrcTabItem::OnCommandline),
    FXMAPFUNC(SEL_KEYPRESS,             IrcTabItem::ID_COMMANDLINE,     IrcTabItem::OnKeyPress),
    FXMAPFUNC(SEL_COMMAND,              IrcSocket::ID_SERVER,           IrcTabItem::OnIrcEvent),
    FXMAPFUNC(SEL_TIMEOUT,              IrcTabItem::ID_TIME,            IrcTabItem::OnTimeout),
    FXMAPFUNC(SEL_TIMEOUT,              IrcTabItem::ID_PTIME,           IrcTabItem::OnPipeTimeout),
    FXMAPFUNC(SEL_TIMEOUT,              IrcTabItem::ID_ETIME,           IrcTabItem::OnEggTimeout),
    FXMAPFUNC(SEL_LEFTBUTTONRELEASE,    IrcTabItem::ID_TEXT,            IrcTabItem::OnLeftMouse),
    FXMAPFUNC(SEL_RIGHTBUTTONRELEASE,   IrcTabItem::ID_USERS,           IrcTabItem::OnRightMouse),
    FXMAPFUNC(SEL_DOUBLECLICKED,        IrcTabItem::ID_USERS,           IrcTabItem::OnDoubleclick),
    FXMAPFUNC(SEL_COMMAND,              IrcTabItem::ID_NEWQUERY,        IrcTabItem::OnNewQuery),
    FXMAPFUNC(SEL_COMMAND,              IrcTabItem::ID_WHOIS,           IrcTabItem::OnWhois),
    FXMAPFUNC(SEL_COMMAND,              IrcTabItem::ID_DCCCHAT,         IrcTabItem::OnDccChat),
    FXMAPFUNC(SEL_COMMAND,              IrcTabItem::ID_DCCSEND,         IrcTabItem::OnDccSend),
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

IrcTabItem::IrcTabItem(dxTabBook *tab, const FXString &tabtext, FXIcon *ic, FXuint opts, TYPE typ, IrcSocket *sock, FXbool oswnd, FXbool uswn, FXbool logg, FXString cmdlst, FXString lpth, FXint maxa, IrcColor clrs, FXString nichar, FXFont *fnt, FXbool scmd, FXbool slst, FXbool cnick, FXbool sclr)
    : FXTabItem(tab, tabtext, ic, opts), parent(tab), server(sock), type(typ), usersShown(uswn), logging(logg), ownServerWindow(oswnd), sameCmd(scmd), sameList(slst), coloredNick(cnick), stripColors(sclr),
        colors(clrs), commandsList(cmdlst), logPath(lpth), maxAway(maxa), nickCompletionChar(nichar), logstream(NULL)
{
    currentPosition = 0;
    historyMax = 25;
    numberUsers = 0;
    maxLen = 460;
    checkAway = FALSE;
    iamOp = FALSE;
    topic = _("No topic is set");
    editableTopic = TRUE;
    pipe = NULL;
    sendPipe = FALSE;
    scriptHasAll = FALSE;
    scriptHasMyMsg = FALSE;

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

    FXHiliteStyle style = {colors.text,colors.back,getApp()->getSelforeColor(),getApp()->getSelbackColor(),getApp()->getHiliteColor(),FXRGB(255, 128, 128),colors.back,0};
    for(int i=0; i<17; i++)
    {
        textStyleList.append(style);
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
    text->setHiliteStyles(textStyleList.data());

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
    text->layout();
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
    for(FXint i=0; i<FXMIN(17,textStyleList.no()); i++)
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
    if(text->getFont() != fnt)
    {
        text->setFont(fnt);
        text->recalc();
    }
    if(topicline->getFont() != fnt)
    {
        topicline->setFont(fnt);
        topicline->recalc();
    }
    if(sameCmd && commandline->getFont() != fnt)
    {
        commandline->setFont(fnt);
        commandline->recalc();
    }
    else
    {
        commandline->setFont(getApp()->getNormalFont());
        commandline->recalc();
    }
    if(sameList && users->getFont() != fnt)
    {
        users->setFont(fnt);
        users->recalc();
    }
    else
    {
        users->setFont(getApp()->getNormalFont());
        users->recalc();
    }
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

void IrcTabItem::SetStripColors(FXbool sclr)
{
    stripColors = sclr;
}

//if highlight==TRUE, highlight tab
void IrcTabItem::AppendText(FXString msg, FXbool highlight)
{
    AppendIrcText(msg, 0);
    if(highlight && FXRGB(255,0,0) != this->getTextColor() && parent->getCurrent()*2 != parent->indexOfChild(this))
    {
        if(msg.contains(server->GetNickName()))
        {
            this->setTextColor(FXRGB(255,0,0));
            if(type == CHANNEL) this->setIcon(chnewm);
        }
        else this->setTextColor(FXRGB(0,0,255));
        if(type == QUERY) this->setIcon(unewm);
    }
}

void IrcTabItem::AppendIrcText(FXString msg, FXTime time, FXbool disableStrip)
{
    if(!time) time = FXSystem::now();
    if(type != OTHER) text->appendText("["+FXSystem::time("%H:%M:%S", time) +"] ");
    AppendLinkText(stripColors && !disableStrip ? StripColors(msg, FALSE) : msg, 0);
    MakeLastRowVisible(FALSE);
    this->LogLine(StripColors(msg, TRUE), time);
}

void IrcTabItem::AppendIrcNickText(FXString nick, FXString msg, FXint style, FXTime time)
{
    if(!time) time = FXSystem::now();
    if(type != OTHER) text->appendText("["+FXSystem::time("%H:%M:%S", time) +"] ");
    text->appendStyledText(nick+": ", style);
    AppendLinkText(stripColors ? StripColors(msg, FALSE) : msg, 0);
    MakeLastRowVisible(FALSE);
    this->LogLine(StripColors("<"+nick+"> "+msg, TRUE), time);
}

/* if highlight==TRUE, highlight tab
 * disableStrip is for dxirc.Print
*/
void IrcTabItem::AppendStyledText(FXString text, FXint style, FXbool highlight, FXbool disableStrip)
{
    if(style) AppendIrcStyledText(text, style, 0, disableStrip);
    else AppendIrcText(text, 0, disableStrip);
    if(highlight && FXRGB(255,0,0) != this->getTextColor() && parent->getCurrent()*2 != parent->indexOfChild(this))
    {
        if(type != OTHER && text.contains(server->GetNickName()))
        {
            this->setTextColor(FXRGB(255,0,0));
            if(type == CHANNEL) this->setIcon(chnewm);
        }
        else this->setTextColor(FXRGB(0,0,255));
        if(type == QUERY) this->setIcon(unewm);
    }
}

void IrcTabItem::AppendIrcStyledText(FXString styled, FXint stylenum, FXTime time, FXbool disableStrip)
{
    if(!time) time = FXSystem::now();
    if(type != OTHER) text->appendText("["+FXSystem::time("%H:%M:%S", time) +"] ");
    AppendLinkText(stripColors && !disableStrip ? StripColors(styled, TRUE) : styled, stylenum);
    MakeLastRowVisible(FALSE);
    this->LogLine(StripColors(styled, TRUE), time);
}

static FXbool Badchar(FXchar c)
{
    switch(c) {
        case ' ':
        case ',':
        case '\0':
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

void IrcTabItem::AppendLinkText(const FXString &txt, FXint stylenum)
{
    FXint i = 0;
    FXint linkLength = 0;
    FXbool bold = FALSE;
    FXbool under = FALSE;
    FXint lastStyle = stylenum;
    FXColor foreColor = colors.text;
    FXColor backColor = colors.back;
    FXString normalText = "";
    FXint length = txt.length();
    while(i<length)
    {
        if(txt[i]=='h' && !comparecase(txt.mid(i,7),"http://"))
        {
            if(!normalText.empty())
            {
                text->appendStyledText(normalText, lastStyle);
                normalText.clear();
            }
            for(FXint j=i; j<length; j++)
            {
                if(Badchar(txt[j]))
                {
                    break;
                }
                linkLength++;
            }
            text->appendStyledText(txt.mid(i, linkLength), linkLength>7 ? 9 : stylenum);
            i+=linkLength-1;
            linkLength=0;
        }
        else if(txt[i]=='h' && !comparecase(txt.mid(i,8),"https://"))
        {
            if(!normalText.empty())
            {
                text->appendStyledText(normalText, lastStyle);
                normalText.clear();
            }
            for(FXint j=i; j<length; j++)
            {
                if(Badchar(txt[j]))
                {
                    break;
                }
                linkLength++;
            }
            text->appendStyledText(txt.mid(i, linkLength), linkLength>8 ? 9 : stylenum);
            i+=linkLength-1;
            linkLength=0;
        }
        else if(txt[i]=='f' && !comparecase(txt.mid(i,6),"ftp://"))
        {
            if(!normalText.empty())
            {
                text->appendStyledText(normalText, lastStyle);
                normalText.clear();
            }
            for(FXint j=i; j<length; j++)
            {
                if(Badchar(txt[j]))
                {
                    break;
                }
                linkLength++;
            }
            text->appendStyledText(txt.mid(i, linkLength), linkLength>6 ? 9 : stylenum);
            i+=linkLength-1;
            linkLength=0;
        }
        else if(txt[i]=='w' && !comparecase(txt.mid(i,4),"www."))
        {
            if(!normalText.empty())
            {
                text->appendStyledText(normalText, lastStyle);
                normalText.clear();
            }
            for(FXint j=i; j<length; j++)
            {
                if(Badchar(txt[j]))
                {
                    break;
                }
                linkLength++;
            }
            text->appendStyledText(txt.mid(i, linkLength), linkLength>4 ? 9 : stylenum);
            i+=linkLength-1;
            linkLength=0;
        }
        else
        {
            if(txt[i] == '\002') //bold
            {
                if(!normalText.empty())
                {
                    text->appendStyledText(normalText, lastStyle);
                    normalText.clear();
                }
                bold = !bold;
                FXuint style;
                if(bold && under) style = FXText::STYLE_BOLD|FXText::STYLE_UNDERLINE;
                else if(bold && !under) style = FXText::STYLE_BOLD;
                else if(!bold && under) style = FXText::STYLE_UNDERLINE;
                lastStyle = HiliteStyleExist(foreColor, backColor, style);
                if(lastStyle == -1)
                {
                    CreateHiliteStyle(foreColor, backColor, style);
                    lastStyle = textStyleList.no();
                }
            }
            else if(txt[i] == '\037') //underline
            {
                if(!normalText.empty())
                {
                    text->appendStyledText(normalText, lastStyle);
                    normalText.clear();
                }
                under = !under;
                FXuint style;
                if(bold && under) style = FXText::STYLE_BOLD|FXText::STYLE_UNDERLINE;
                else if(bold && !under) style = FXText::STYLE_BOLD;
                else if(!bold && under) style = FXText::STYLE_UNDERLINE;
                lastStyle = HiliteStyleExist(foreColor, backColor, style);
                if(lastStyle == -1)
                {
                    CreateHiliteStyle(foreColor, backColor, style);
                    lastStyle = textStyleList.no();
                }
            }
            else if(txt[i] == '\003') //color
            {
                if(!normalText.empty())
                {
                    text->appendStyledText(normalText, lastStyle);
                    normalText.clear();
                }
                FXbool isHexColor = FALSE;
                if(i+1<length)
                {
                    if(txt[i+1] == '#') isHexColor = TRUE;
                }
                FXbool back = FALSE;
                backColor = colors.back;
                if(isHexColor?i+8:i+3<length)
                {
                    if(txt[isHexColor?i+8:i+3] == ',')
                    {
                        back = TRUE;
                        backColor = isHexColor ? FXRGB(FXIntVal(txt.mid(i+10,2),16),FXIntVal(txt.mid(i+12,2),16),FXIntVal(txt.mid(i+14,2),16)) : GetIrcColor(FXIntVal(txt.mid(i+4,2)));
                    }
                }
                FXuint style = 0;
                if(bold && under) style = FXText::STYLE_BOLD|FXText::STYLE_UNDERLINE;
                else if(bold && !under) style = FXText::STYLE_BOLD;
                else if(!bold && under) style = FXText::STYLE_UNDERLINE;
                foreColor = isHexColor ? FXRGB(FXIntVal(txt.mid(i+2,2),16),FXIntVal(txt.mid(i+4,2),16),FXIntVal(txt.mid(i+6,2),16)) : GetIrcColor(FXIntVal(txt.mid(i+1,2)));
                lastStyle = HiliteStyleExist(foreColor, backColor, style);
                if(lastStyle == -1)
                {
                    CreateHiliteStyle(foreColor, backColor, style);
                    lastStyle = textStyleList.no();
                };
                if(back) isHexColor ? i+=15 : i+=5;
                else isHexColor ? i+=7 : i+=2;
            }
            else if(txt[i] == '\017') //reset
            {
                if(!normalText.empty())
                {
                    text->appendStyledText(normalText, lastStyle);
                    normalText.clear();
                }
                bold = FALSE;
                under = FALSE;
                foreColor = colors.text;
                backColor = colors.back;
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
        text->appendStyledText(normalText, lastStyle);
    }
    text->appendText("\n");
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

void IrcTabItem::LogLine(const FXString &line, const FXTime &time)
{
    if(logging && type != SERVER)
    {
        this->StartLogging();
        *logstream << "[" << FXSystem::time("%H:%M:%S", time).text() << "] " << line.text() << std::endl;
    }
}

FXbool IrcTabItem::IsChannel(const FXString &text)
{
    if(text.length()) return server->GetChanTypes().contains(text[0]);
    return FALSE;
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
    if(commandtext.empty())
        return 1;
    commandsHistory.append(commandtext);
    if (commandsHistory.no() > historyMax)
        commandsHistory.erase(0);
    currentPosition = commandsHistory.no()-1;
    commandline->setText("");
    for(FXint i=0; i<=commandtext.contains('\n'); i++)
    {
        FXString text = commandtext.section('\n', i).before('\r');
        if(comparecase(text.after('/').before(' '), "quit") == 0 ||
                comparecase(text.after('/').before(' '), "lua") == 0)
        {
            ProcessLine(text);
            return 1;
        }
#ifdef HAVE_LUA
        if(text[0] != '/') parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_MYMSG), &text);
        parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_COMMAND), &text);
        if(text[0] == '/' && !scriptHasAll) ProcessLine(text);
        else if(!scriptHasMyMsg && !scriptHasAll) ProcessLine(text);
#else
        ProcessLine(text);
#endif
    }
    return 1;
}

FXbool IrcTabItem::ProcessLine(const FXString& commandtext)
{
    FXString command = (commandtext[0] == '/' ? commandtext.before(' ') : "");
    if(!utils::GetAlias(command).empty())
    {
        FXString acommand = utils::GetAlias(command);
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
                if(!acommand.section('&',i).trim().empty()) result = ProcessCommand(acommand.section('&',i).trim());
            }
            return result;
        }
        else
        {
            return ProcessCommand(acommand);
        }
    }
    return ProcessCommand(commandtext);
}

FXbool IrcTabItem::ProcessCommand(const FXString& commandtext)
{
    FXString command = (commandtext[0] == '/' ? commandtext.after('/').before(' ').lower() : "");
    if(type == DCCCHAT)
    {
        if(server->GetConnected())
        {
            if(command == "me")
            {
                AppendIrcStyledText(server->GetNickName()+" "+commandtext.after(' '), 2, FXSystem::now());
                return server->SendDccChatText("\001ACTION "+commandtext.after(' ')+"\001");
            }
            if(command == "say")
            {
                if(coloredNick) AppendIrcNickText(server->GetNickName(), commandtext.after(' '), GetNickColor(server->GetNickName()), FXSystem::now());
                else AppendIrcNickText(server->GetNickName(), commandtext.after(' '), 5, FXSystem::now());
                return server->SendDccChatText(commandtext.after(' '));
            }
            if(commandtext[0] != '/')
            {
                if(coloredNick) AppendIrcNickText(server->GetNickName(), commandtext, GetNickColor(server->GetNickName()), FXSystem::now());
                else AppendIrcNickText(server->GetNickName(), commandtext, 5, FXSystem::now());
                return server->SendDccChatText(commandtext);
            }
        }
        else
            AppendIrcStyledText(_("You aren't connected"), 4, FXSystem::now());
        return TRUE;
    }
    if(type == OTHER)
    {
        if(utils::IsScriptCommand(command))
        {
            LuaRequest lua;
            lua.type = LUA_COMMAND;
            lua.text = commandtext.after('/');
            parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_LUA), &lua);
            return TRUE;
        }
        if(command == "commands")
        {
            AppendIrcStyledText(utils::AvailableScriptCommands(), 3, FXSystem::now());
            return TRUE;
        }
        if(command == "egg")
        {
            text->removeText(0, text->getLength());
            text->layout();
            text->appendStyledText(FXString("ahoj sem pan Vajíčko,\n"), 3);
            text->appendStyledText(FXString("a dnes Vám přináším killer feature VODOTRYSK!!!\n"), 3);
            getApp()->addTimeout(this, ID_ETIME, 1000);
            pics = 0;
            return TRUE;
        }
        if(command == "help")
        {
            return ShowHelp(commandtext.after(' ').lower().trim());
        }
        if(command == "tetris")
        {
            parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_NEWTETRIS), NULL);
            return TRUE;
        }
        return TRUE;
    }
    if(commandtext[0] == '/')
    {
        if(utils::IsScriptCommand(command))
        {
            LuaRequest lua;
            lua.type = LUA_COMMAND;
            lua.text = commandtext.after('/');
            parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_LUA), &lua);
            return TRUE;
        }
        if(command == "admin")
        {
            if(server->GetConnected()) return server->SendAdmin(commandtext.after(' '));
            else
            {
                AppendIrcStyledText(_("You aren't connected"), 4, FXSystem::now());
                parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "away")
        {
            if(server->GetConnected())
            {
                if(commandtext.after(' ').length() > server->GetAwayLen())
                {
                    AppendIrcStyledText(FXStringFormat(_("Warning: Away message is too long. Max. away message length is %d."), server->GetAwayLen()), 4, FXSystem::now());
                    return server->SendAway(commandtext.after(' '));
                }
                else
                    return server->SendAway(commandtext.after(' '));
            }
            else
            {
                AppendIrcStyledText(_("You aren't connected"), 4, FXSystem::now());
                parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "banlist")
        {
            if(server->GetConnected())
            {
                FXString channel = commandtext.after(' ');
                if(channel.empty() && type == CHANNEL) return server->SendBanlist(getText());
                else if(!IsChannel(channel) && type != CHANNEL)
                {
                    AppendIrcStyledText(_("/banlist <channel>, shows banlist for channel."), 4, FXSystem::now());
                    return FALSE;
                }
                else return server->SendBanlist(channel);
            }
            else
            {
                AppendIrcStyledText(_("You aren't connected"), 4, FXSystem::now());
                parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "connect")
        {
            if(commandtext.after(' ').empty())
            {
                AppendIrcStyledText(_("/connect <server> [port] [nick] [password] [realname] [channels], connects for given server."), 4, FXSystem::now());
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
                parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CSERVER), &srv);
                return TRUE;
            }
        }
        if(command == "commands")
        {
            AppendIrcStyledText(utils::AvailableCommands(), 3, FXSystem::now());
            return TRUE;
        }
        if(command == "ctcp")
        {
            if(server->GetConnected())
            {
                FXString to = commandtext.after(' ').before(' ');
                FXString msg = commandtext.after(' ', 2);
                if(to.empty() || msg.empty())
                {
                    AppendIrcStyledText(_("/ctcp <nick> <message>, sends a CTCP message to a user."), 4, FXSystem::now());
                    return FALSE;
                }
                else if(msg.length() > maxLen-12-to.length())
                {
                    AppendIrcStyledText(FXStringFormat(_("Warning: ctcp message is too long. Max. ctcp message length is %d."), maxLen-12-to.length()), 4, FXSystem::now());
                    return server->SendCtcp(to, msg);
                }
                else return server->SendCtcp(to, msg);
            }
            else
            {
                AppendIrcStyledText(_("You aren't connected"), 4, FXSystem::now());
                parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "cycle")
        {
            if(server->GetConnected())
            {
                if(type == CHANNEL)
                {
                    if(IsChannel(commandtext.after(' ')))
                    {
                        FXString channel = commandtext.after(' ').before(' ');
                        FXString reason = commandtext.after(' ', 2);
                        reason.empty() ? server->SendPart(channel) : server->SendPart(channel, reason);
                        return server->SendJoin(channel);
                    }
                    else
                    {
                        commandtext.after(' ').empty() ? server->SendPart(getText()) : server->SendPart(getText(), commandtext.after(' '));
                        return server->SendJoin(getText());
                    }
                }
                else
                {
                    if(IsChannel(commandtext.after(' ')))
                    {
                        FXString channel = commandtext.after(' ').before(' ');
                        FXString reason = commandtext.after(' ', 2);
                        reason.empty() ? server->SendPart(channel) : server->SendPart(channel, reason);
                        return server->SendJoin(channel);
                    }
                    else
                    {
                        AppendIrcStyledText(_("/cycle <channel> [message], leaves and join channel."), 4, FXSystem::now());
                        return FALSE;
                    }
                }
            }
            else
            {
                AppendIrcStyledText(_("You aren't connected"), 4, FXSystem::now());
                parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "dcc")
        {
            if(server->GetConnected())
            {
                FXString dccCommand = utils::GetParam(commandtext, 2, FALSE).lower();
                if(dccCommand == "chat")
                {
                    FXString nick = utils::GetParam(commandtext, 3, FALSE);
                    if(!comparecase(nick, "chat"))
                    {
                        AppendIrcStyledText(_("Nick for chat wasn't entered."), 4, FXSystem::now());
                        return FALSE;
                    }
                    if(!comparecase(nick, server->GetNickName()))
                    {
                        AppendIrcStyledText(_("Chat with yourself isn't good idea."), 4, FXSystem::now());
                        return FALSE;
                    }
                    IrcEvent ev;
                    ev.eventType = IRC_DCCSERVER;
                    ev.param1 = nick;
                    parent->getParent()->getParent()->handle(server, FXSEL(SEL_COMMAND, IrcSocket::ID_SERVER), &ev);
                    return TRUE;
                }
                else if(dccCommand == "send")
                {
                    FXString nick = utils::GetParam(commandtext, 3, FALSE);
                    FXString file = utils::GetParam(commandtext, 4, TRUE);
                    if(!comparecase(nick, "send"))
                    {
                        AppendIrcStyledText(_("Nick for sending file wasn't entered."), 4, FXSystem::now());
                        return FALSE;
                    }
                    if(!comparecase(nick, server->GetNickName()))
                    {
                        AppendIrcStyledText(_("Sending to yourself isn't good idea."), 4, FXSystem::now());
                        return FALSE;
                    }
                    if(!comparecase(nick, file))
                    {
                        AppendIrcStyledText(_("Filename wasn't entered"), 4, FXSystem::now());
                        return FALSE;
                    }
                    if(!FXStat::exists(file))
                    {
                        AppendIrcStyledText(FXStringFormat(_("File '%s' doesn't exist"), file.text()), 4, FXSystem::now());
                        return FALSE;
                    }
                    IrcEvent ev;
                    ev.eventType = IRC_DCCOUT;
                    ev.param1 = nick;
                    ev.param2 = file;
                    parent->getParent()->getParent()->handle(server, FXSEL(SEL_COMMAND, IrcSocket::ID_SERVER), &ev);
                    return TRUE;
                }
                else if(dccCommand == "psend")
                {
                    FXString nick = utils::GetParam(commandtext, 3, FALSE);
                    FXString file = utils::GetParam(commandtext, 4, TRUE);
                    if(!comparecase(nick, "psend"))
                    {
                        AppendIrcStyledText(_("Nick for sending file wasn't entered."), 4, FXSystem::now());
                        return FALSE;
                    }
                    if(!comparecase(nick, server->GetNickName()))
                    {
                        AppendIrcStyledText(_("Sending to yourself isn't good idea."), 4, FXSystem::now());
                        return FALSE;
                    }
                    if(!comparecase(nick, file))
                    {
                        AppendIrcStyledText(_("Filename wasn't entered"), 4, FXSystem::now());
                        return FALSE;
                    }
                    if(!FXStat::exists(file))
                    {
                        AppendIrcStyledText(FXStringFormat(_("File '%s' doesn't exist"), file.text()), 4, FXSystem::now());
                        return FALSE;
                    }
                    IrcEvent ev;
                    ev.eventType = IRC_DCCPOUT;
                    ev.param1 = nick;
                    ev.param2 = file;
                    parent->getParent()->getParent()->handle(server, FXSEL(SEL_COMMAND, IrcSocket::ID_SERVER), &ev);
                    return TRUE;
                }
                else
                {
                    AppendIrcStyledText(FXStringFormat(_("'%s' isn't dcc command <chat|send|psend>"), dccCommand.text()), 4, FXSystem::now());
                    return FALSE;
                }
            }
            else
            {
                AppendIrcStyledText(_("You aren't connected"), 4, FXSystem::now());
                parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "deop")
        {
            if(server->GetConnected())
            {
                FXString params = commandtext.after(' ');
                if(type == CHANNEL)
                {
                    if(params.empty())
                    {
                        AppendIrcStyledText(_("/deop <nicks>, removes operator status from one or more nicks."), 4, FXSystem::now());
                        return FALSE;
                    }
                    else if(IsChannel(params) && params.after(' ').empty())
                    {
                        AppendIrcStyledText(_("/deop <channel> <nicks>, removes operator status from one or more nicks."), 4, FXSystem::now());
                        return FALSE;
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
                        AppendIrcStyledText(_("/deop <channel> <nicks>, removes operator status from one or more nicks."), 4, FXSystem::now());
                        return FALSE;
                    }
                }
            }
            else
            {
                AppendIrcStyledText(_("You aren't connected"), 4, FXSystem::now());
                parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "devoice")
        {
            if(server->GetConnected())
            {
                FXString params = commandtext.after(' ');
                if(type == CHANNEL)
                {
                    if(params.empty())
                    {
                        AppendIrcStyledText(_("/devoice <nicks>, removes voice from one or more nicks."), 4, FXSystem::now());
                        return FALSE;
                    }
                    else if(IsChannel(params) && params.after(' ').empty())
                    {
                        AppendIrcStyledText(_("/devoice <channel> <nicks>, removes voice from one or more nicks."), 4, FXSystem::now());
                        return FALSE;
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
                        AppendIrcStyledText(_("/devoice <channel> <nicks>, removes voice from one or more nicks."), 4, FXSystem::now());
                        return FALSE;
                    }
                }
            }
            else
            {
                AppendIrcStyledText(_("You aren't connected"), 4, FXSystem::now());
                parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "disconnect")
        {
            if(server->GetConnected())
            {
                if(commandtext.after(' ').empty()) server->Disconnect();
                else server->Disconnect(commandtext.after(' '));
                return TRUE;
            }
            else
            {
                server->CloseConnection(TRUE);
                return TRUE;
            }
        }
        if(command == "dxirc")
        {
            for(FXint i=0; i<5+rand()%5; i++)
            {
                AppendIrcText("", FXSystem::now());
            }
            AppendIrcText("     __  __         _", FXSystem::now());
            AppendIrcText("  _/__//__/|_      | |       _", FXSystem::now());
            AppendIrcText(" /_| |_| |/_/|   __| |__  __|_| _ _  ___", FXSystem::now());
            AppendIrcText(" |_   _   _|/   / _  |\\ \\/ /| || '_)/ __)", FXSystem::now());
            AppendIrcText(" /_| |_| |/_/| | (_| | |  | | || | | (__", FXSystem::now());
            AppendIrcText(" |_   _   _|/   \\____|/_/\\_\\|_||_|  \\___)", FXSystem::now());
            AppendIrcText("   |_|/|_|/     (c) 2008~ David Vachulka", FXSystem::now());
            AppendIrcText("   http://dxirc.org", FXSystem::now());
            for(FXint i=0; i<5+rand()%5; i++)
            {
                AppendIrcText("", FXSystem::now());
            }
            return TRUE;
        }
        if(command == "egg")
        {
            text->removeText(0, text->getLength());
            text->layout();
            text->appendStyledText(FXString("ahoj sem pan Vajíčko,\n"), 3);
            text->appendStyledText(FXString("a dnes Vám přináším killer feature VODOTRYSK!!!\n"), 3);
            getApp()->addTimeout(this, ID_ETIME, 1000);
            pics = 0;
            return TRUE;
        }
        if(command == "exec")
        {
            FXString params = commandtext.after(' ');
            if(params.empty())
            {
                AppendIrcStyledText(_("/exec [-o|-c] <command>, executes command, -o sends output to channel/query, -c closes running command."), 4, FXSystem::now());
                return FALSE;
            }
            else
            {
                if(!pipe) pipe = new dxPipe(getApp(), this);
                pipeStrings.clear();
                if(params.before(' ').contains("-o"))
                {
                    sendPipe = TRUE;
                    pipe->ExecCmd(params.after(' '));
                }
                else if(params.before(' ').contains("-c"))
                {
                    sendPipe = FALSE;
                    pipeStrings.clear();
                    pipe->StopCmd();
                }
                else
                {
                    sendPipe = FALSE;
                    pipe->ExecCmd(params);
                }
                return TRUE;
            }
        }
        if(command == "help")
        {
            return ShowHelp(commandtext.after(' ').lower().trim());
        }
        if(command == "ignore")
        {
            FXString ignorecommand = commandtext.after(' ').before(' ');
            FXString ignoretext = commandtext.after(' ').after(' ');
            if(ignorecommand.empty())
            {
                AppendIrcStyledText(_("/ignore <list|addcmd|rmcmd|addusr|rmusr> [command|user] [channel] [server]"), 4, FXSystem::now());
                return FALSE;
            }
            if(comparecase(ignorecommand, "list")==0)
            {
                AppendIrcStyledText(_("Ignored commands:"), 7, FXSystem::now());
                if(commandsList.empty()) AppendIrcText(_("No ignored commands"), FXSystem::now());
                else AppendIrcText(commandsList.rbefore(';'), FXSystem::now());
                AppendIrcStyledText(_("Ignored users:"), 7, FXSystem::now());
                dxIgnoreUserArray users = server->GetUsersList();
                if(!users.no()) AppendIrcText(_("No ignored users"), FXSystem::now());
                else
                {
                    for(FXint i=0; i<users.no(); i++)
                    {
                        AppendIrcText(FXStringFormat(_("%s on channel(s): %s and server(s): %s"), users[i].nick.text(), users[i].channel.text(), users[i].server.text()), FXSystem::now());
                    }
                }
                return TRUE;
            }
            else if(comparecase(ignorecommand, "addcmd")==0)
            {
                if(ignoretext.empty())
                {
                    AppendIrcStyledText(_("/ignore addcmd <command>, adds command to ignored commands."), 4, FXSystem::now());
                    return FALSE;
                }
                parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_ADDICOMMAND), &ignoretext);
                return TRUE;
            }
            else if(comparecase(ignorecommand, "rmcmd")==0)
            {
                if(ignoretext.empty())
                {
                    AppendIrcStyledText(_("/ignore rmcmd <command>, removes command from ignored commands."), 4, FXSystem::now());
                    return FALSE;
                }
                parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_RMICOMMAND), &ignoretext);
                return TRUE;
            }
            else if(comparecase(ignorecommand, "addusr")==0)
            {
                if(ignoretext.empty())
                {
                    AppendIrcStyledText(_("/ignore addusr <user> [channel] [server], adds user to ignored users."), 4, FXSystem::now());
                    return FALSE;
                }
                parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_ADDIUSER), &ignoretext);
                return TRUE;
            }
            else if(comparecase(ignorecommand, "rmusr")==0)
            {
                if(ignoretext.empty())
                {
                    AppendIrcStyledText(_("/ignore rmusr <user>, removes user from ignored users."), 4, FXSystem::now());
                    return FALSE;
                }
                parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_RMIUSER), &ignoretext);
                return TRUE;
            }
            else
            {
                AppendIrcStyledText(FXStringFormat(_("'%s' isn't <list|addcmd|rmcmd|addusr|rmusr>"), ignorecommand.text()), 4, FXSystem::now());
                return FALSE;
            }
        }
        if(command == "invite")
        {
            if(server->GetConnected())
            {
                FXString params = commandtext.after(' ');
                if(params.empty())
                {
                    AppendIrcStyledText(_("/invite <nick> <channel>, invites someone to a channel."), 4, FXSystem::now());
                    return FALSE;
                }
                else if(IsChannel(params) && params.after(' ').empty())
                {
                    AppendIrcStyledText(_("/invite <nick> <channel>, invites someone to a channel."), 4, FXSystem::now());
                    return FALSE;
                }
                else
                {
                    FXString nick = params.before(' ');
                    FXString channel = params.after(' ');
                    return server->SendInvite(nick, channel);
                }
            }
            else
            {
                AppendIrcStyledText(_("You aren't connected"), 4, FXSystem::now());
                parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "join")
        {
            if(server->GetConnected())
            {
                FXString channel = commandtext.after(' ');
                if(!IsChannel(channel))
                {
                    AppendIrcStyledText(_("/join <channel>, joins a channel."), 4, FXSystem::now());
                    AppendIrcStyledText(FXStringFormat(_("'%c' isn't valid char for channel."), channel[0]), 4, FXSystem::now());
                    return FALSE;
                }
                else return server->SendJoin(channel);
            }
            else
            {
                AppendIrcStyledText(_("You aren't connected"), 4, FXSystem::now());
                parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "kick")
        {
            if(server->GetConnected())
            {
                FXString params = commandtext.after(' ');
                if(type == CHANNEL)
                {
                    if(params.empty())
                    {
                        AppendIrcStyledText(_("/kick <nick>, kicks a user from a channel."), 4, FXSystem::now());
                        return FALSE;
                    }
                    else if(IsChannel(params) && params.after(' ').empty())
                    {
                        AppendIrcStyledText(_("/kick <channel> <nick>, kicks a user from a channel."), 4, FXSystem::now());
                        return FALSE;
                    }
                    else if(IsChannel(params) && !params.after(' ').empty())
                    {
                        FXString channel = params.before(' ');
                        FXString nick = params.after(' ');
                        FXString reason = params.after(' ', 2);
                        if(reason.length() > server->GetKickLen())
                        {
                            AppendIrcStyledText(FXStringFormat(_("Warning: reason of kick is too long. Max. reason length is %d."), server->GetKickLen()), 4, FXSystem::now());
                            return server->SendKick(channel, nick, reason);
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
                            AppendIrcStyledText(FXStringFormat(_("Warning: reason of kick is too long. Max. reason length is %d."), server->GetKickLen()), 4, FXSystem::now());
                            return server->SendKick(channel, nick, reason);
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
                            AppendIrcStyledText(FXStringFormat(_("Warning: reason of kick is too long. Max. reason length is %d."), server->GetKickLen()), 4, FXSystem::now());
                            return server->SendKick(channel, nick, reason);
                        }
                        else return server->SendKick(channel, nick, reason);
                    }
                    else
                    {
                        AppendIrcStyledText(_("/kick <channel> <nick>, kicks a user from a channel."), 4, FXSystem::now());
                        return FALSE;
                    }
                }
            }
            else
            {
                AppendIrcStyledText(_("You aren't connected"), 4, FXSystem::now());
                parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "kill")
        {
            if(server->GetConnected())
            {
                FXString params = commandtext.after(' ');
                FXString nick = params.before(' ');
                FXString reason = params.after(' ');
                if(params.empty())
                {
                    AppendIrcStyledText(_("/kill <user> [reason], kills a user from the network."), 4, FXSystem::now());
                    return FALSE;
                }
                if(reason.length() > maxLen-7-nick.length())
                {
                    AppendIrcStyledText(FXStringFormat(_("Warning: reason of kill is too long. Max. reason length is %d."), maxLen-7-nick.length()), 4, FXSystem::now());
                    return server->SendKill(nick, reason);
                }
                else return server->SendKill(nick, reason);
            }
            else
            {
                AppendIrcStyledText(_("You aren't connected"), 4, FXSystem::now());
                parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "list")
        {
            if(server->GetConnected()) return server->SendList(commandtext.after(' '));
            else
            {
                AppendIrcStyledText(_("You aren't connected"), 4, FXSystem::now());
                parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
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
                AppendIrcStyledText(_("/lua <help|load|unload|list> [scriptpath|scriptname]"), 4, FXSystem::now());
                return FALSE;
            }
            if(comparecase(luacommand, "help")==0)
            {
                AppendIrcStyledText(FXStringFormat(_("For help about Lua scripting visit: %s"), LUA_HELP_PATH), 3, FXSystem::now());
                return TRUE;
            }
            else if(comparecase(luacommand, "load")==0) lua.type = LUA_LOAD;
            else if(comparecase(luacommand, "unload")==0) lua.type = LUA_UNLOAD;
            else if(comparecase(luacommand, "list")==0) lua.type = LUA_LIST;            
            else
            {
                AppendIrcStyledText(FXStringFormat(_("'%s' isn't <help|load|unload|list>"), luacommand.text()), 4, FXSystem::now());
                return FALSE;
            }
            lua.text = luatext;
            parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_LUA), &lua);
            return TRUE;
#else
            AppendIrcStyledText(_("dxirc is compiled without support for Lua scripting"), 4, FXSystem::now());
            return FALSE;
#endif
        }
        if(command == "me")
        {
            if(server->GetConnected())
            {
                FXString params = commandtext.after(' ');
                if(type == CHANNEL)
                {
                    if(params.empty())
                    {
                        AppendIrcStyledText(_("/me <message>, sends the action to the current channel."), 4, FXSystem::now());
                        return FALSE;
                    }
                    else if(IsChannel(params) && !params.after(' ').empty())
                    {
                        FXString channel = params.before(' ');
                        FXString message = params.after(' ');
                        if(channel == getText())
                        {
                            AppendIrcStyledText(server->GetNickName()+" "+message, 2, FXSystem::now());
                            IrcEvent ev;
                            ev.eventType = IRC_ACTION;
                            ev.param1 = server->GetNickName();
                            ev.param2 = channel;
                            ev.param3 = message;
                            parent->getParent()->getParent()->handle(server, FXSEL(SEL_COMMAND, IrcSocket::ID_SERVER), &ev);
                        }
                        if(message.length() > maxLen-19-channel.length())
                        {
                            dxStringArray messages = CutText(message, maxLen-19-channel.length());
                            FXbool result = TRUE;
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
                        AppendIrcStyledText(server->GetNickName()+" "+params, 2, FXSystem::now());
                        IrcEvent ev;
                        ev.eventType = IRC_ACTION;
                        ev.param1 = server->GetNickName();
                        ev.param2 = getText();
                        ev.param3 = params;
                        parent->getParent()->getParent()->handle(server, FXSEL(SEL_COMMAND, IrcSocket::ID_SERVER), &ev);
                        if(params.length() > maxLen-19-getText().length())
                        {
                            dxStringArray messages = CutText(params, maxLen-19-getText().length());
                            FXbool result = TRUE;
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
                            FXbool result = TRUE;
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
                        AppendIrcStyledText(_("/me <to> <message>, sends the action."), 4, FXSystem::now());
                        return FALSE;
                    }
                }
            }
            else
            {
                AppendIrcStyledText(_("You aren't connected"), 4, FXSystem::now());
                parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "mode")
        {
            if(server->GetConnected())
            {
                FXString params = commandtext.after(' ');
                if(params.empty())
                {
                    AppendIrcStyledText(_("/mode <channel> <modes>, sets modes for a channel."), 4, FXSystem::now());
                    return FALSE;
                }
                else
                    return server->SendMode(params);
            }
            else
            {
                AppendIrcStyledText(_("You aren't connected"), 4, FXSystem::now());
                parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "msg")
        {
            if(server->GetConnected())
            {
                FXString params = commandtext.after(' ');
                FXString to = params.before(' ');
                FXString message = params.after(' ');
                if(!to.empty() && !message.empty())
                {
                    if(to == getText())
                    {
                        if(coloredNick) AppendIrcNickText(server->GetNickName(), message, GetNickColor(server->GetNickName()), FXSystem::now());
                        else AppendIrcNickText(server->GetNickName(), message, 5, FXSystem::now());
                        IrcEvent ev;
                        ev.eventType = IRC_PRIVMSG;
                        ev.param1 = server->GetNickName();
                        ev.param2 = to;
                        ev.param3 = message;
                        parent->getParent()->getParent()->handle(server, FXSEL(SEL_COMMAND, IrcSocket::ID_SERVER), &ev);
                    }
                    if(message.length() > maxLen-10-to.length())
                    {
                        dxStringArray messages = CutText(message, maxLen-10-to.length());
                        FXbool result = TRUE;
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
                    AppendIrcStyledText(_("/msg <nick/channel> <message>, sends a normal message."), 4, FXSystem::now());
                    return FALSE;
                }
            }
            else
            {
                AppendIrcStyledText(_("You aren't connected"), 4, FXSystem::now());
                parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "names")
        {
            if(server->GetConnected())
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
                        AppendIrcStyledText(_("/names <channel>, for nicks on a channel."), 4, FXSystem::now());
                        return FALSE;
                    }
                    else return server->SendNames(params);
                }
            }
            else
            {
                AppendIrcStyledText(_("You aren't connected"), 4, FXSystem::now());
                parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "nick")
        {
            if(server->GetConnected())
            {
                FXString nick = commandtext.after(' ');
                if(nick.empty())
                {
                    AppendIrcStyledText(_("/nick <nick>, changes nick."), 4, FXSystem::now());
                    return FALSE;
                }
                else if(nick.length() > server->GetNickLen())
                {
                    AppendIrcStyledText(FXStringFormat(_("Warning: nick is too long. Max. nick length is %d."), server->GetNickLen()), 4, FXSystem::now());
                    return server->SendNick(nick);
                }
                else
                {
                    return server->SendNick(nick);
                }
            }
            else
            {
                AppendIrcStyledText(_("You aren't connected"), 4, FXSystem::now());
                parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "notice")
        {
            if(server->GetConnected())
            {
                FXString params = commandtext.after(' ');
                FXString to = params.before(' ');
                FXString message = params.after(' ');
                if(!to.empty() && !message.empty())
                {
                    AppendIrcStyledText(FXStringFormat(_("NOTICE to %s: %s"), to.text(), message.text()), 2, FXSystem::now());
                    if(message.length() > maxLen-9-to.length())
                    {
                        dxStringArray messages = CutText(message, maxLen-9-to.length());
                        FXbool result = TRUE;
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
                    AppendIrcStyledText(_("/notice <nick/channel> <message>, sends a notice."), 4, FXSystem::now());
                    return FALSE;
                }
            }
            else
            {
                AppendIrcStyledText(_("You aren't connected"), 4, FXSystem::now());
                parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "op")
        {
            if(server->GetConnected())
            {
                FXString params = commandtext.after(' ');
                if(type == CHANNEL)
                {
                    if(params.empty())
                    {
                        AppendIrcStyledText(_("/op <nicks>, gives operator status for one or more nicks."), 4, FXSystem::now());
                        return FALSE;
                    }
                    else if(IsChannel(params) && params.after(' ').empty())
                    {
                        AppendIrcStyledText(_("/op <channel> <nicks>, gives operator status for one or more nicks."), 4, FXSystem::now());
                        return FALSE;
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
                        AppendIrcStyledText(_("/op <channel> <nicks>, gives operator status for one or more nicks."), 4, FXSystem::now());
                        return FALSE;
                    }
                }
            }
            else
            {
                AppendIrcStyledText(_("You aren't connected"), 4, FXSystem::now());
                parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "oper")
        {
            if(server->GetConnected())
            {
                FXString params = commandtext.after(' ');
                FXString login = params.before(' ');
                FXString password = params.after(' ');
                if(!login.empty() && !password.empty()) return server->SendOper(login, password);
                else
                {
                    AppendIrcStyledText(_("/oper <login> <password>, oper up."), 4, FXSystem::now());
                    return FALSE;
                }
            }
            else
            {
                AppendIrcStyledText(_("You aren't connected"), 4, FXSystem::now());
                parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "part")
        {
            if(server->GetConnected())
            {
                if(type == CHANNEL)
                {
                    if(commandtext.after(' ').empty()) return server->SendPart(getText());
                    else return server->SendPart(getText(), commandtext.after(' '));
                }
                else
                {
                    if(IsChannel(commandtext.after(' ')))
                    {
                        FXString channel = commandtext.after(' ').before(' ');
                        FXString reason = commandtext.after(' ', 2);
                        if(reason.empty()) return server->SendPart(channel);
                        else return server->SendPart(channel, reason);
                    }
                    else
                    {
                        AppendIrcStyledText(_("/part <channel> [reason], leaves channel."), 4, FXSystem::now());
                        return FALSE;
                    }
                }
            }
            else
            {
                AppendIrcStyledText(_("You aren't connected"), 4, FXSystem::now());
                parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "query")
        {
            if(server->GetConnected())
            {
                if(commandtext.after(' ').empty())
                {
                    AppendIrcStyledText(_("/query <nick>, opens query with nick."), 4, FXSystem::now());
                    return FALSE;
                }
                else
                {
                    IrcEvent ev;
                    ev.eventType = IRC_QUERY;
                    ev.param1 = commandtext.after(' ').before(' ');
                    ev.param2 = server->GetNickName();
                    parent->getParent()->getParent()->handle(server, FXSEL(SEL_COMMAND, IrcSocket::ID_SERVER), &ev);
                    return TRUE;
                }
            }
            else
            {
                AppendIrcStyledText(_("You aren't connected"), 4, FXSystem::now());
                parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "quit")
        {
            parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CQUIT), NULL);
            return TRUE;
        }
        if(command == "quote")
        {
            if(server->GetConnected()) return server->SendQuote(commandtext.after(' '));
            else
            {
                AppendIrcStyledText(_("You aren't connected"), 4, FXSystem::now());
                parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "say")
        {
            if(server->GetConnected())
            {
                if (type != SERVER && !commandtext.after(' ').empty())
                {
                    if(coloredNick) AppendIrcNickText(server->GetNickName(), commandtext.after(' '), GetNickColor(server->GetNickName()), FXSystem::now());
                    else AppendIrcNickText(server->GetNickName(), commandtext.after(' '), 5, FXSystem::now());
                    IrcEvent ev;
                    ev.eventType = IRC_PRIVMSG;
                    ev.param1 = server->GetNickName();
                    ev.param2 = getText();
                    ev.param3 = commandtext.after(' ');
                    parent->getParent()->getParent()->handle(server, FXSEL(SEL_COMMAND, IrcSocket::ID_SERVER), &ev);
                    if(commandtext.after(' ').length() > maxLen-10-getText().length())
                    {
                        dxStringArray messages = CutText(commandtext.after(' '), maxLen-10-getText().length());
                        FXbool result = TRUE;
                        for(FXint i=0; i<messages.no(); i++)
                        {
                            result = server->SendMsg(getText(), messages[i]) &result;
                        }
                        return result;
                    }
                    else return server->SendMsg(getText(), commandtext.after(' '));
                }
                return FALSE;
            }
            else
            {
                AppendIrcStyledText(_("You aren't connected"), 4, FXSystem::now());
                parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "stats")
        {
            if(server->GetConnected())
                return server->SendStats(commandtext.after(' '));
            else
            {
                AppendIrcStyledText(_("You aren't connected"), 4, FXSystem::now());
                parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "tetris")
        {
            parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_NEWTETRIS), NULL);
            return TRUE;
        }
        if(command == "time")
        {
            if(server->GetConnected()) return server->SendQuote("TIME");
            else
            {
                AppendIrcStyledText(_("You aren't connected"), 4, FXSystem::now());
                parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "topic")
        {
            if(server->GetConnected())
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
                            AppendIrcStyledText(FXStringFormat(_("Warning: topic is too long. Max. topic length is %d."), server->GetTopicLen()), 4, FXSystem::now());
                            return server->SendTopic(channel, topic);
                        }
                        else return server->SendTopic(channel, topic);
                    }
                    else
                    {
                        if(params.length() > server->GetTopicLen())
                        {
                            AppendIrcStyledText(FXStringFormat(_("Warning: topic is too long. Max. topic length is %d."), server->GetTopicLen()), 4, FXSystem::now());
                            return server->SendTopic(getText(), params);
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
                            AppendIrcStyledText(FXStringFormat(_("Warning: topic is too long. Max. topic length is %d."), server->GetTopicLen()), 4, FXSystem::now());
                            return server->SendTopic(channel, params);
                        }
                        else return server->SendTopic(channel, topic);
                    }
                    else
                    {
                        AppendIrcStyledText(_("/topic <channel> [topic], views or changes channel topic."), 4, FXSystem::now());
                        return FALSE;
                    }
                }
            }
            else
            {
                AppendIrcStyledText(_("You aren't connected"), 4, FXSystem::now());
                parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "voice")
        {
            if(server->GetConnected())
            {
                FXString params = commandtext.after(' ');
                if(type == CHANNEL)
                {
                    if(params.empty())
                    {
                        AppendIrcStyledText(_("/voice <nicks>, gives voice for one or more nicks."), 4, FXSystem::now());
                        return FALSE;
                    }
                    else if(IsChannel(params) && params.after(' ').empty())
                    {
                        AppendIrcStyledText(_("/voice <channel> <nicks>, gives voice for one or more nicks."), 4, FXSystem::now());
                        return FALSE;
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
                        AppendIrcStyledText(_("/voice <channel> <nicks>, gives voice for one or more nicks."), 4, FXSystem::now());
                        return FALSE;
                    }
                }
            }
            else
            {
                AppendIrcStyledText(_("You aren't connected"), 4, FXSystem::now());
                parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "wallops")
        {
            if(server->GetConnected())
            {
                FXString params = commandtext.after(' ');
                if(params.empty())
                {
                    AppendIrcStyledText(_("/wallops <message>, sends wallop message."), 4, FXSystem::now());
                    return FALSE;
                }
                else
                {
                    if(params.length() > maxLen-9)
                    {
                        dxStringArray messages = CutText(params, maxLen-9);
                        FXbool result = TRUE;
                        for(FXint i=0; i<messages.no(); i++)
                        {
                            result = server->SendWallops(messages[i]) &result;
                        }
                        return result;
                    }
                    else return server->SendWallops(params);
                }
            }
            else
            {
                AppendIrcStyledText(_("You aren't connected"), 4, FXSystem::now());
                parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "who")
        {
            if(server->GetConnected())
            {
                FXString params = commandtext.after(' ');
                if(params.empty())
                {
                    AppendIrcStyledText(_("/who <mask> [o], searchs for mask on network, if o is supplied, only search for opers."), 4, FXSystem::now());
                    return FALSE;
                }
                else return server->SendWho(params);
            }
            else
            {
                AppendIrcStyledText(_("You aren't connected"), 4, FXSystem::now());
                parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "whoami")
        {
            if(server->GetConnected()) return server->SendWhoami();
            else
            {
                AppendIrcStyledText(_("You aren't connected"), 4, FXSystem::now());
                parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "whois")
        {
            if(server->GetConnected())
            {
                FXString params = commandtext.after(' ');
                if(params.empty())
                {
                    AppendIrcStyledText(_("/whois <nick>, whois nick."), 4, FXSystem::now());
                    return FALSE;
                }
                else return server->SendWhois(params);
            }
            else
            {
                AppendIrcStyledText(_("You aren't connected"), 4, FXSystem::now());
                parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        if(command == "whowas")
        {
            if(server->GetConnected())
            {
                FXString params = commandtext.after(' ');
                if(params.empty())
                {
                    AppendIrcStyledText(_("/whowas <nick>, whowas nick."), 4, FXSystem::now());
                    return FALSE;
                }
                else return server->SendWhowas(params);
            }
            else
            {
                AppendIrcStyledText(_("You aren't connected"), 4, FXSystem::now());
                parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
                return TRUE;
            }
        }
        AppendIrcStyledText(FXStringFormat(_("Unknown command '%s', type /commands for available commands"), command.text()), 4, FXSystem::now());
        return FALSE;
    }
    else
    {
        if (command.empty() && type != SERVER && !commandtext.empty() && server->GetConnected())
        {
            if(coloredNick) AppendIrcNickText(server->GetNickName(), commandtext, GetNickColor(server->GetNickName()), FXSystem::now());
            else AppendIrcNickText(server->GetNickName(), commandtext, 5, FXSystem::now());
            IrcEvent ev;
            ev.eventType = IRC_PRIVMSG;
            ev.param1 = server->GetNickName();
            ev.param2 = getText();
            ev.param3 = commandtext;
            parent->getParent()->getParent()->handle(server, FXSEL(SEL_COMMAND, IrcSocket::ID_SERVER), &ev);
            if(commandtext.length() > maxLen-10-getText().length())
            {
                dxStringArray messages = CutText(commandtext, maxLen-10-getText().length());
                FXbool result = TRUE;
                for(FXint i=0; i<messages.no(); i++)
                {
                    result = server->SendMsg(getText(), messages[i]) &result;
                }
                return result;
            }
            else return server->SendMsg(getText(), commandtext);
        }
        if(!server->GetConnected())
        {
            AppendIrcStyledText(_("You aren't connected"), 4, FXSystem::now());
            parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_CDIALOG), NULL);
            return TRUE;
        }
        return FALSE;
    }
    return FALSE;
}

FXbool IrcTabItem::ShowHelp(FXString command)
{
    if(utils::IsScriptCommand(command))
    {
        AppendIrcStyledText(utils::GetHelpText(command), 3, FXSystem::now());
        return TRUE;
    }
    if(command == "admin")
    {
        AppendIrcStyledText(_("ADMIN [server], finds information about administrator for current server or [server]."), 3, FXSystem::now());
        return TRUE;
    }
    if(command == "away")
    {
        AppendIrcStyledText(_("AWAY [message], sets away status."), 3, FXSystem::now());
        return TRUE;
    }
    if(command == "banlist")
    {
        AppendIrcStyledText(_("BANLIST <channel>, shows banlist for channel."), 3, FXSystem::now());
        return TRUE;
    }
    if(command == "connect")
    {
        AppendIrcStyledText(_("CONNECT <server> [port] [nick] [password] [realname] [channels], connects for given server."), 3, FXSystem::now());
        return TRUE;
    }
    if(command == "ctcp")
    {
        AppendIrcStyledText(_("CTCP <nick> <message>, sends a CTCP message to a user."), 3, FXSystem::now());
        return TRUE;
    }
    if(command == "cycle")
    {
        AppendIrcStyledText(_("CYCLE <channel> [message], leaves and join channel."), 3, FXSystem::now());
        return TRUE;
    }
    if(command == "dcc")
    {
        AppendIrcStyledText(_("DCC chat <nick>, starts DCC chat."), 3, FXSystem::now());
        AppendIrcStyledText(_("DCC send <nick> <filename>, sends file over DCC."), 3, FXSystem::now());
        AppendIrcStyledText(_("DCC psend <nick> <filename>, sends file passive over DCC."), 3, FXSystem::now());
        AppendIrcStyledText(_("More information about passive DCC on http://en.wikipedia.org/wiki/Direct_Client-to-Client#Passive_DCC"), 3, FXSystem::now());
        return TRUE;
    }
    if(command == "deop")
    {
        AppendIrcStyledText(_("DEOP <channel> <nicks>, removes operator status from one or more nicks."), 3, FXSystem::now());
        return TRUE;
    }
    if(command == "devoice")
    {
        AppendIrcStyledText(_("DEVOICE <channel> <nicks>, removes voice from one or more nicks."), 3, FXSystem::now());
        return TRUE;
    }
    if(command == "disconnect")
    {
        AppendIrcStyledText(_("DISCONNECT [reason], leaves server."), 3, FXSystem::now());
        return TRUE;
    }
#ifndef WIN32
    if(command == "exec")
    {
        AppendIrcStyledText(_("EXEC [-o|-c] <command>, executes command, -o sends output to channel/query, -c closes running command."), 3, FXSystem::now());
        return TRUE;
    }
#endif
    if(command == "help")
    {
        AppendIrcStyledText(_("HELP <command>, shows help for command."), 3, FXSystem::now());
        return TRUE;
    }
    if(command == "ignore")
    {
        AppendIrcStyledText(_("IGNORE list, shows list ignored commands and users."), 3, FXSystem::now());
        AppendIrcStyledText(_("IGNORE addcmd <command>, adds command to ignored commands."), 3, FXSystem::now());
        AppendIrcStyledText(_("IGNORE rmcmd <command>, removes command from ignored commands."), 3, FXSystem::now());
        AppendIrcStyledText(_("IGNORE addusr <user> [channel] [server], adds user to ignored users."), 3, FXSystem::now());
        AppendIrcStyledText(_("IGNORE rmusr <user>, removes user from ignored users."), 3, FXSystem::now());
        return TRUE;
    }
    if(command == "invite")
    {
        AppendIrcStyledText(_("INVITE <nick> <channel>, invites someone to a channel."), 3, FXSystem::now());
        return TRUE;
    }
    if(command == "join")
    {
        AppendIrcStyledText(_("JOIN <channel>, joins a channel."), 3, FXSystem::now());
        return TRUE;
    }
    if(command == "kick")
    {
        AppendIrcStyledText(_("KICK <channel> <nick>, kicks a user from a channel."), 3, FXSystem::now());
        return TRUE;
    }
    if(command == "kill")
    {
        AppendIrcStyledText(_("KILL <user> [reason], kills a user from the network."), 3, FXSystem::now());
        return TRUE;
    }
    if(command == "list")
    {
        AppendIrcStyledText(_("LIST [channel], lists channels and their topics."), 3, FXSystem::now());
        return TRUE;
    }
#ifdef HAVE_LUA
    if(command == "lua")
    {
        AppendIrcStyledText(_("LUA help, shows help for lua scripting."), 3, FXSystem::now());
        AppendIrcStyledText(_("LUA load <path>, loads script."), 3, FXSystem::now());
        AppendIrcStyledText(_("Example: /lua load /home/dvx/test.lua"), 3, FXSystem::now());
        AppendIrcStyledText(_("LUA unload <name>, unloads script."), 3, FXSystem::now());
        AppendIrcStyledText(_("Example: /lua unload test"), 3, FXSystem::now());
        AppendIrcStyledText(_("LUA list, shows list of loaded scripts"), 3, FXSystem::now());
        return TRUE;
    }
#else
    if(command == "lua")
    {
        AppendIrcStyledText(_("dxirc is compiled without support for Lua scripting"), 4, FXSystem::now());
        return TRUE;
    }
#endif
    if(command == "me")
    {
        AppendIrcStyledText(_("ME <to> <message>, sends the action."), 3, FXSystem::now());
        return TRUE;
    }
    if(command == "mode")
    {
        AppendIrcStyledText(_("MODE <channel> <modes>, sets modes for a channel."), 3, FXSystem::now());
        return TRUE;
    }
    if(command == "msg")
    {
        AppendIrcStyledText(_("MSG <nick/channel> <message>, sends a normal message."), 3, FXSystem::now());
        return TRUE;
    }
    if(command == "names")
    {
        AppendIrcStyledText(_("NAMES <channel>, for nicks on a channel."), 3, FXSystem::now());
        return TRUE;
    }
    if(command == "nick")
    {
        AppendIrcStyledText(_("NICK <nick>, changes nick."), 3, FXSystem::now());
        return TRUE;
    }
    if(command == "notice")
    {
        AppendIrcStyledText(_("NOTICE <nick/channel> <message>, sends a notice."), 3, FXSystem::now());
        return TRUE;
    }
    if(command == "op")
    {
        AppendIrcStyledText(_("OP <channel> <nicks>, gives operator status for one or more nicks."), 3, FXSystem::now());
        return TRUE;
    }
    if(command == "oper")
    {
        AppendIrcStyledText(_("OPER <login> <password>, oper up."), 3, FXSystem::now());
        return TRUE;
    }
    if(command == "part")
    {
        AppendIrcStyledText(_("PART <channel> [reason], leaves channel."), 3, FXSystem::now());
        return TRUE;
    }
    if(command == "query")
    {
        AppendIrcStyledText(_("QUERY <nick>, opens query with nick."), 3, FXSystem::now());
        return TRUE;
    }
    if(command == "quit")
    {
        AppendIrcStyledText(_("QUIT, closes application."), 3, FXSystem::now());
        return TRUE;
    }
    if(command == "quote")
    {
        AppendIrcStyledText(_("QUOTE [text], sends text to server."), 3, FXSystem::now());
        return TRUE;
    }
    if(command == "say")
    {
        AppendIrcStyledText(_("SAY [text], sends text to current tab."), 3, FXSystem::now());
        return TRUE;
    }
    if(command == "stats")
    {
        AppendIrcStyledText(_("STATS <type>, shows some irc server usage statistics. Available types vary slightly per server; some common ones are:"), 3, FXSystem::now());
        AppendIrcStyledText(_("c - shows C and N lines for a given server.  These are the names of the servers that are allowed to connect."), 3, FXSystem::now());
        AppendIrcStyledText(_("h - shows H and L lines for a given server (Hubs and Leaves)."), 3, FXSystem::now());
        AppendIrcStyledText(_("k - show K lines for a server.  This shows who is not allowed to connect and possibly at what time they are not allowed to connect."), 3, FXSystem::now());
        AppendIrcStyledText(_("i - shows I lines. This is who CAN connect to a server."), 3, FXSystem::now());
        AppendIrcStyledText(_("l - shows information about amount of information passed to servers and users."), 3, FXSystem::now());
        AppendIrcStyledText(_("m - shows a count for the number of times the various commands have been used since the server was booted."), 3, FXSystem::now());
        AppendIrcStyledText(_("o - shows the list of authorized operators on the server."), 3, FXSystem::now());
        AppendIrcStyledText(_("p - shows online operators and their idle times."), 3, FXSystem::now());
        AppendIrcStyledText(_("u - shows the uptime for a server."), 3, FXSystem::now());
        AppendIrcStyledText(_("y - shows Y lines, which lists the various connection classes for a given server."), 3, FXSystem::now());

        return TRUE;
    }
    if(command == "tetris")
    {
        AppendIrcStyledText(_("TETRIS, start small easteregg."), 3, FXSystem::now());
        AppendIrcStyledText(_("Keys for playing:"), 3, FXSystem::now());
        AppendIrcStyledText(_("N .. new game"), 3, FXSystem::now());
        AppendIrcStyledText(_("P .. pause game"), 3, FXSystem::now());
        AppendIrcStyledText(_("Num5 .. rotate piece"), 3, FXSystem::now());
        AppendIrcStyledText(_("Num3 .. move piece right"), 3, FXSystem::now());
        AppendIrcStyledText(_("Num2 .. drop piece"), 3, FXSystem::now());
        AppendIrcStyledText(_("Num1 .. move piece left"), 3, FXSystem::now());
        return TRUE;
    }
    if(command == "time")
    {
        AppendIrcStyledText(_("TIME, displays the time of day, local to server."), 3, FXSystem::now());
        return TRUE;
    }
    if(command == "topic")
    {
        AppendIrcStyledText(_("TOPIC [topic], sets or shows topic."), 3, FXSystem::now());
        return TRUE;
    }
    if(command == "voice")
    {
        AppendIrcStyledText(_("VOICE <channel> <nicks>, gives voice for one or more nicks."), 3, FXSystem::now());
        return TRUE;
    }
    if(command == "wallops")
    {
        AppendIrcStyledText(_("WALLOPS <message>, sends wallop message."), 3, FXSystem::now());
        return TRUE;
    }
    if(command == "who")
    {
        AppendIrcStyledText(_("WHO <mask> [o], searchs for mask on network, if o is supplied, only search for opers."), 3, FXSystem::now());
        return TRUE;
    }
    if(command == "whoami")
    {
        AppendIrcStyledText(_("WHOAMI, whois about you."), 3, FXSystem::now());
        return TRUE;
    }
    if(command == "whois")
    {
        AppendIrcStyledText(_("WHOIS <nick>, whois nick."), 3, FXSystem::now());
        return TRUE;
    }
    if(command == "whowas")
    {
        AppendIrcStyledText(_("WHOWAS <nick>, whowas nick."), 3, FXSystem::now());
        return TRUE;
    }
    if(!utils::GetAlias(command[0] == '/' ? command:"/"+command).empty())
    {
        AppendIrcStyledText(FXStringFormat("%s: %s", command.upper().text(), utils::GetAlias(command[0] == '/' ? command:"/"+command).text()), 3, FXSystem::now());
        return TRUE;
    }
    if(command.empty()) AppendIrcStyledText(_("Command is empty, type /commands for available commands"), 4, FXSystem::now());
    else AppendIrcStyledText(FXStringFormat(_("Unknown command '%s', type /commands for available commands"), command.text()), 4, FXSystem::now());
    return FALSE;
}

long IrcTabItem::OnKeyPress(FXObject *, FXSelector, void *ptr)
{
    if (commandline->hasFocus())
    {
        FXEvent* event = (FXEvent*)ptr;
        FXString line = commandline->getText();
        switch(event->code){
            case KEY_Tab:
                if(event->state&CONTROLMASK)
                {
                    parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_NEXTTAB), NULL);
                    return 1;
                }
                if(commandline->getText()[0] == '/' && commandline->getText().after(' ').empty())
                {
                    for(FXint i = 0; i < utils::CommandsNo(); i++)
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
                    FXint curpos;
                    line[commandline->getCursorPos()] == ' ' ? curpos = commandline->getCursorPos()-1 : curpos = commandline->getCursorPos();
                    FXint pos = line.rfind(' ', curpos)+1;
                    FXint n = line.find(' ', curpos)>0 ? line.find(' ', curpos)-pos : line.length()-pos;
                    FXString toCompletion = line.mid(pos, n);
                    for(FXint j = 0; j < users->getNumItems(); j++)
                    {
                        if(comparecase(toCompletion, GetNick(j)) == 0)
                        {
                            if((j+1) < users->getNumItems())
                            {
                                commandline->setText(line.replace(pos, n, GetNick(j+1)));
                                commandline->setCursorPos(pos+GetNick(j+1).length());
                            }
                            else
                            {
                                commandline->setText(line.replace(pos, n, GetNick(0)));
                                commandline->setCursorPos(pos+GetNick(0).length());
                            }
                            break;
                        }
                        else if(comparecase(toCompletion, GetNick(j).left(toCompletion.length())) == 0)
                        {
                            commandline->setText(line.replace(pos, n, GetNick(j)));
                            commandline->setCursorPos(pos+GetNick(j).length());
                            break;
                        }
                    }
                    return 1;
                }
                return 1;
            case KEY_Up:
                if(currentPosition!=-1 && currentPosition<commandsHistory.no())
                {
                    if(!line.empty() && line!=commandsHistory[currentPosition])
                    {
                        commandsHistory.append(line);
                        if(commandsHistory.no() > historyMax)
                            commandsHistory.erase(0);
                        currentPosition = commandsHistory.no()-1;
                    }
                    if(currentPosition > 0 && !line.empty())
                        --currentPosition;
                    commandline->setText(commandsHistory[currentPosition]);
                }
                return 1;
            case KEY_Down:
                if(currentPosition!=-1 && currentPosition<commandsHistory.no())
                {
                    if(!line.empty() && line!=commandsHistory[currentPosition])
                    {
                        commandsHistory.append(line);
                        if(commandsHistory.no() > historyMax)
                            commandsHistory.erase(0);
                        currentPosition = commandsHistory.no()-1;
                    }
                    if(currentPosition < commandsHistory.no()-1)
                    {
                        ++currentPosition;
                        commandline->setText(commandsHistory[currentPosition]);
                    }
                    else
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
    FXbool hasCurrent = server->FindTarget(static_cast<IrcTabItem*>(parent->childAtIndex(indexOfCurrent)));
    FXint indexOfThis = parent->indexOfChild(this);
    if(hasCurrent)
    {
        if(indexOfCurrent == indexOfThis) return TRUE;
        else return FALSE;
    }
    else
    {
        for(FXint i = 0; i<parent->numChildren(); i+=2)
        {
            if(server->FindTarget(static_cast<IrcTabItem*>(parent->childAtIndex(i))))
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
    if(commandsList.contains(command)) return TRUE;
    return FALSE;
}

void IrcTabItem::AddUser(const FXString& user)
{
    if(user[0] == server->GetAdminPrefix()
            && users->findItem(user.after(server->GetAdminPrefix())) == -1)
    {
        users->appendItem(user.after(server->GetAdminPrefix()), irc_admin_icon);
        numberUsers++;
        users->sortItems();
        return;
    }
    if(user[0] == server->GetOwnerPrefix()
            && users->findItem(user.after(server->GetOwnerPrefix())) == -1)
    {
        users->appendItem(user.after(server->GetOwnerPrefix()), irc_owner_icon);
        numberUsers++;
        users->sortItems();
        return;
    }
    if(user[0] == server->GetOpPrefix()
            && users->findItem(user.after(server->GetOpPrefix())) == -1)
    {
        users->appendItem(user.after(server->GetOpPrefix()), irc_op_icon);
        numberUsers++;
        users->sortItems();
        return;
    }
    if(user[0] == server->GetVoicePrefix()
            && users->findItem(user.after(server->GetVoicePrefix())) == -1)
    {
        users->appendItem(user.after(server->GetVoicePrefix()), irc_voice_icon);
        numberUsers++;
        users->sortItems();
        return;
    }
    if(user[0] == server->GetHalfopPrefix()
            && users->findItem(user.after(server->GetHalfopPrefix())) == -1)
    {
        users->appendItem(user.after(server->GetHalfopPrefix()), irc_halfop_icon);
        numberUsers++;
        users->sortItems();
        return;
    }
    if(user[0] == server->GetAdminPrefix()
            || user[0] == server->GetOwnerPrefix()
            || user[0] == server->GetOpPrefix()
            || user[0] == server->GetVoicePrefix()
            || user[0] == server->GetHalfopPrefix())
        return;
    if(users->findItem(user) == -1)
    {
        users->appendItem(user, irc_normal_icon);
        numberUsers++;
        users->sortItems();
        return;
    }
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
            AddUser(server->GetOwnerPrefix()+newnick);
        }
        else if(users->getItemIcon(i) == irc_admin_icon || users->getItemIcon(i) == irc_away_admin_icon)
        {
            RemoveUser(nick);
            AddUser(server->GetAdminPrefix()+newnick);
        }
        else if(users->getItemIcon(i) == irc_op_icon || users->getItemIcon(i) == irc_away_op_icon)
        {
            RemoveUser(nick);
            AddUser(server->GetOpPrefix()+newnick);
        }
        else if(users->getItemIcon(i) == irc_voice_icon || users->getItemIcon(i) == irc_away_voice_icon)
        {
            RemoveUser(nick);
            AddUser(server->GetVoicePrefix()+newnick);
        }
        else if(users->getItemIcon(i) == irc_halfop_icon || users->getItemIcon(i) == irc_away_halfop_icon)
        {
            RemoveUser(nick);
            AddUser(server->GetHalfopPrefix()+newnick);
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
        OnIrcPrivmsg(ev);
        return 1;
    }
    if(ev->eventType == IRC_ACTION)
    {
        OnIrcAction(ev);
        return 1;
    }
    if(ev->eventType == IRC_CTCPREPLY)
    {
        OnIrcCtpcReply(ev);
        return 1;
    }
    if(ev->eventType == IRC_CTCPREQUEST)
    {
        OnIrcCtcpRequest(ev);
        return 1;
    }
    if(ev->eventType == IRC_DCCMSG && type == DCCCHAT)
    {
        OnIrcDccMsg(ev);
        return 1;
    }
    if(ev->eventType == IRC_DCCACTION && type == DCCCHAT)
    {
        OnIrcDccAction(ev);
        return 1;
    }
    if(ev->eventType == IRC_JOIN)
    {
        OnIrcJoin(ev);
        return 1;
    }
    if(ev->eventType == IRC_QUIT)
    {
        OnIrcQuit(ev);
        return 1;
    }
    if(ev->eventType == IRC_PART)
    {
        OnIrcPart(ev);
        return 1;
    }
    if(ev->eventType == IRC_CHNOTICE)
    {
        OnIrcChnotice(ev);
        return 1;
    }
    if(ev->eventType == IRC_NOTICE)
    {
        OnIrcNotice(ev);
        return 1;
    }
    if(ev->eventType == IRC_NICK)
    {
        OnIrcNick(ev);
        return 1;
    }
    if(ev->eventType == IRC_TOPIC)
    {
        OnIrcTopic(ev);
        return 1;
    }
    if(ev->eventType == IRC_INVITE)
    {
        OnIrcInvite(ev);
        return 1;
    }
    if(ev->eventType == IRC_KICK)
    {
        OnIrcKick(ev);
        return 1;
    }
    if(ev->eventType == IRC_MODE)
    {
        OnIrcMode(ev);
        return 1;
    }
    if(ev->eventType == IRC_UMODE)
    {
        OnIrcUmode(ev);
        return 1;
    }
    if(ev->eventType == IRC_CHMODE)
    {
        OnIrcChmode(ev);
        return 1;
    }
    if(ev->eventType == IRC_SERVERREPLY)
    {
        OnIrcServerReply(ev);
        return 1;
    }
    if(ev->eventType == IRC_CONNECT)
    {
        OnIrcConnect(ev);
        return 1;
    }
    if(ev->eventType == IRC_ERROR)
    {
        OnIrcError(ev);
        return 1;
    }
    if(ev->eventType == IRC_SERVERERROR)
    {
        OnIrcServerError(ev);
        return 1;
    }
    if(ev->eventType == IRC_DISCONNECT)
    {
        OnIrcDisconnect(ev);
        return 1;
    }
    if(ev->eventType == IRC_RECONNECT)
    {
        OnIrcReconnect(ev);
        return 1;
    }
    if(ev->eventType == IRC_UNKNOWN)
    {
        OnIrcUnknown(ev);
        return 1;
    }
    if(ev->eventType == IRC_301)
    {
        OnIrc301(ev);
        return 1;
    }
    if(ev->eventType == IRC_305)
    {
        OnIrc305(ev);
        return 1;
    }
    if(ev->eventType == IRC_306)
    {
        OnIrc306(ev);
        return 1;
    }
    if(ev->eventType == IRC_331 || ev->eventType == IRC_332 || ev->eventType == IRC_333)
    {
        OnIrc331332333(ev);
        return 1;
    }
    if(ev->eventType == IRC_353)
    {
        OnIrc353(ev);
        return 1;
    }
    if(ev->eventType == IRC_366)
    {
        OnIrc366(ev);
        return 1;
    }
    if(ev->eventType == IRC_372)
    {
        OnIrc372(ev);
        return 1;
    }
    if(ev->eventType == IRC_AWAY)
    {
        OnIrcAway(ev);
        return 1;
    }
    if(ev->eventType == IRC_ENDMOTD)
    {
        OnIrcEndMotd();
        return 1;
    }
    return 1;
}

//handle IrcEvent IRC_PRIVMSG
void IrcTabItem::OnIrcPrivmsg(IrcEvent* ev)
{
    if((comparecase(ev->param2, getText()) == 0 && type == CHANNEL) || (ev->param1 == getText() && type == QUERY && ev->param2 == server->GetNickName()))
    {
        if(coloredNick)
        {
            if(ev->param3.contains(server->GetNickName())) AppendIrcStyledText(ev->param1+": "+ev->param3, 8, ev->time);
            else AppendIrcNickText(ev->param1, ev->param3, GetNickColor(ev->param1), ev->time);
        }
        else
        {
            if(ev->param3.contains(server->GetNickName())) AppendIrcStyledText(ev->param1+": "+ev->param3, 8, ev->time);
            else AppendIrcNickText(ev->param1, ev->param3, 5, ev->time);
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
}

//handle IrcEvent IRC_ACTION
void IrcTabItem::OnIrcAction(IrcEvent* ev)
{
    if((comparecase(ev->param2, getText()) == 0 && type == CHANNEL) || (ev->param1 == getText() && type == QUERY && ev->param2 == server->GetNickName()))
    {
        if(!IsCommandIgnored("me"))
        {
            AppendIrcStyledText(ev->param1+" "+ev->param3, 2, ev->time);
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
}

//handle IrcEvent IRC_CTCPREPLY
void IrcTabItem::OnIrcCtpcReply(IrcEvent* ev)
{
    if(type == SERVER || IsFirst())
    {
        if(!IsCommandIgnored("ctcp"))
        {
            AppendIrcStyledText(FXStringFormat(_("CTCP %s reply from %s: %s"), utils::GetParam(ev->param2, 1, FALSE).text(), ev->param1.text(), utils::GetParam(ev->param2, 2, TRUE).text()), 2, ev->time);
            if(FXRGB(255,0,0) != this->getTextColor() && parent->getCurrent()*2 != parent->indexOfChild(this)) this->setTextColor(FXRGB(0,0,255));
        }
    }
}

//handle IrcEvent IRC_CTCPREQUEST
void IrcTabItem::OnIrcCtcpRequest(IrcEvent* ev)
{
    if(type == SERVER || IsFirst())
    {
        if(!IsCommandIgnored("ctcp"))
        {
            AppendIrcStyledText(FXStringFormat(_("CTCP %s request from %s"), ev->param2.text(), ev->param1.text()), 2, ev->time);
            if(FXRGB(255,0,0) != this->getTextColor() && parent->getCurrent()*2 != parent->indexOfChild(this)) this->setTextColor(FXRGB(0,0,255));
        }
    }
}

//handle IrcEvent IRC_DCCMSG
void IrcTabItem::OnIrcDccMsg(IrcEvent* ev)
{
    if(coloredNick)
    {
        if(ev->param1.contains(server->GetNickName())) AppendIrcStyledText(getText()+": "+ev->param1, 8, ev->time);
        else AppendIrcNickText(getText(), ev->param1, GetNickColor(getText()), ev->time);
    }
    else
    {
        if(ev->param1.contains(server->GetNickName())) AppendIrcStyledText(getText()+": "+ev->param1, 8, ev->time);
        else AppendIrcNickText(getText(), ev->param1, 5, ev->time);
    }
    if(FXRGB(255,0,0) != this->getTextColor() && parent->getCurrent()*2 != parent->indexOfChild(this))
    {
        if(ev->param1.contains(server->GetNickName()))
        {
            this->setTextColor(FXRGB(255,0,0));
        }
        else this->setTextColor(FXRGB(0,0,255));
        this->setIcon(dccnewm);
        parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_NEWMSG), NULL);
    }
}

//handle IrcEvent IRC_DCCACTION
void IrcTabItem::OnIrcDccAction(IrcEvent* ev)
{
    AppendIrcStyledText(getText()+" "+ev->param1, 2, ev->time);
    if(FXRGB(255,0,0) != this->getTextColor() && parent->getCurrent()*2 != parent->indexOfChild(this))
    {
        if(ev->param1.contains(server->GetNickName()))
        {
            this->setTextColor(FXRGB(255,0,0));
        }
        else this->setTextColor(FXRGB(0,0,255));
        this->setIcon(dccnewm);
        parent->getParent()->getParent()->handle(this, FXSEL(SEL_COMMAND, ID_NEWMSG), NULL);
    }
}

//handle IrcEvent IRC_JOIN
void IrcTabItem::OnIrcJoin(IrcEvent* ev)
{
    if(comparecase(ev->param2, getText()) == 0 && ev->param1 != server->GetNickName())
    {
        if(!IsCommandIgnored("join") && !server->IsUserIgnored(ev->param1, getText())) AppendIrcStyledText(FXStringFormat(_("%s has joined to %s"), ev->param1.text(), ev->param2.text()), 1, ev->time);
        AddUser(ev->param1);
    }
}

//handle IrcEvent IRC_QUIT
void IrcTabItem::OnIrcQuit(IrcEvent* ev)
{
    if(type == CHANNEL && users->findItem(ev->param1) != -1)
    {
        RemoveUser(ev->param1);
        if(ev->param2.empty())
        {
            if(!IsCommandIgnored("quit") && !server->IsUserIgnored(ev->param1, getText())) AppendIrcStyledText(FXStringFormat(_("%s has quit"), ev->param1.text()), 1, ev->time);
        }
        else
        {
            if(!IsCommandIgnored("quit") && !server->IsUserIgnored(ev->param1, getText()))AppendIrcStyledText(FXStringFormat(_("%s has quit (%s)"), ev->param1.text(), +ev->param2.text()), 1, ev->time);
        }
    }
    else if(type == QUERY && getText() == ev->param1)
    {
        AppendIrcStyledText(FXStringFormat(_("%s has quit"), ev->param1.text()), 1, ev->time);
    }
}

//handle IrcEvent IRC_PART
void IrcTabItem::OnIrcPart(IrcEvent* ev)
{
    if(comparecase(ev->param2, getText()) == 0)
    {
        if(ev->param3.empty() && !IsCommandIgnored("part") && !server->IsUserIgnored(ev->param1, getText())) AppendIrcStyledText(FXStringFormat(_("%s has parted %s"), ev->param1.text(), ev->param2.text()), 1, ev->time);
        else if(!IsCommandIgnored("part") && !server->IsUserIgnored(ev->param1, getText())) AppendIrcStyledText(FXStringFormat(_("%s has parted %s (%s)"), ev->param1.text(), ev->param2.text(), ev->param3.text()), 1, ev->time);
        RemoveUser(ev->param1);
    }
}

//handle IrcEvent IRC_CHNOTICE
void IrcTabItem::OnIrcChnotice(IrcEvent* ev)
{
    if((comparecase(ev->param2, getText()) == 0 && type == CHANNEL) || (ev->param1 == getText() && type == QUERY && ev->param2 == server->GetNickName()))
    {
        if(!IsCommandIgnored("notice"))
        {
            AppendIrcStyledText(FXStringFormat(_("%s's NOTICE: %s"), ev->param1.text(), ev->param3.text()), 2, ev->time);
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
        if(!IsCommandIgnored("notice"))
        {
            AppendIrcStyledText(FXStringFormat(_("%s's NOTICE: %s"), ev->param1.text(), ev->param3.text()), 3, ev->time);
            if(FXRGB(255,0,0) != this->getTextColor() && parent->getCurrent()*2 != parent->indexOfChild(this)) this->setTextColor(FXRGB(0,0,255));
        }
    }
}

//handle IrcEvent IRC_NOTICE
void IrcTabItem::OnIrcNotice(IrcEvent* ev)
{
    if(type == SERVER || IsFirst())
    {
        if(ev->param1 == server->GetNickName() && !IsCommandIgnored("notice"))
        {
            AppendIrcStyledText(FXStringFormat(_("NOTICE for you: %s"), ev->param2.text()), 3, ev->time);
            if(FXRGB(255,0,0) != this->getTextColor() && parent->getCurrent()*2 != parent->indexOfChild(this)) this->setTextColor(FXRGB(0,0,255));
        }
        else if(!IsCommandIgnored("notice"))
        {
            AppendIrcStyledText(FXStringFormat(_("%s's NOTICE: %s"), ev->param1.text(), ev->param2.text()), 3, ev->time);
            if(FXRGB(255,0,0) != this->getTextColor() && parent->getCurrent()*2 != parent->indexOfChild(this)) this->setTextColor(FXRGB(0,0,255));
        }
    }
}

//handle IrcEvent IRC_NICK
void IrcTabItem::OnIrcNick(IrcEvent* ev)
{
    if(users->findItem(ev->param1) != -1)
    {
        if(ev->param2 == server->GetNickName() && !IsCommandIgnored("nick")) AppendIrcStyledText(FXStringFormat(_("You're now known as %s"), ev->param2.text()), 1, ev->time);
        else if(!IsCommandIgnored("nick") && !server->IsUserIgnored(ev->param1, getText())) AppendIrcStyledText(FXStringFormat(_("%s is now known as %s"), ev->param1.text(), ev->param2.text()), 1, ev->time);
        ChangeNickUser(ev->param1, ev->param2);
    }
    if(type == QUERY && ev->param1 == getText())
    {
        this->setText(ev->param2);
    }
}

//handle IrcEvent IRC_TOPIC
void IrcTabItem::OnIrcTopic(IrcEvent* ev)
{
    if(comparecase(ev->param2, getText()) == 0)
    {
        AppendIrcText(FXStringFormat(_("%s Set new topic for %s: %s"), ev->param1.text(), ev->param2.text(), ev->param3.text()), ev->time);
        topic = ev->param3;
        topicline->setText(topic);
    }
}

//handle IrcEvent IRC_INVITE
void IrcTabItem::OnIrcInvite(IrcEvent* ev)
{
    if(type == SERVER || IsFirst())
    {
        AppendIrcStyledText(FXStringFormat(_("%s invites you to: %s"), ev->param1.text(), ev->param3.text()), 3, ev->time);
        if(FXRGB(255,0,0) != this->getTextColor() && parent->getCurrent()*2 != parent->indexOfChild(this)) this->setTextColor(FXRGB(0,0,255));
    }
}

//handle IrcEvent IRC_KICK
void IrcTabItem::OnIrcKick(IrcEvent* ev)
{
    if(comparecase(ev->param3, getText()) == 0)
    {
        if(ev->param2 != server->GetNickName())
        {
            if(ev->param4.empty()) AppendIrcStyledText(FXStringFormat(_("%s was kicked from %s by %s"), ev->param2.text(), ev->param3.text(), ev->param1.text()), 1, ev->time);
            else AppendIrcStyledText(FXStringFormat(_("%s was kicked from %s by %s (%s)"), ev->param2.text(), ev->param3.text(), ev->param1.text(), ev->param4.text()), 1, ev->time);
            RemoveUser(ev->param2);
        }
    }
    if(ev->param2 == server->GetNickName() && (type == SERVER || IsFirst()))
    {
        if(ev->param4.empty()) AppendIrcStyledText(FXStringFormat(_("You were kicked from %s by %s"), ev->param3.text(), ev->param1.text()), 1, ev->time);
        else AppendIrcStyledText(FXStringFormat(_("You were kicked from %s by %s (%s)"), ev->param3.text(), ev->param1.text(), ev->param4.text()), 1, ev->time);
        if(FXRGB(255,0,0) != this->getTextColor() && parent->getCurrent()*2 != parent->indexOfChild(this)) this->setTextColor(FXRGB(0,0,255));
    }
}

//handle IrcEvent IRC_MODE
void IrcTabItem::OnIrcMode(IrcEvent* ev)
{
    if(type == SERVER || IsFirst())
    {
        AppendIrcStyledText(FXStringFormat(_("Mode change [%s] for %s"), ev->param1.text(), ev->param2.text()), 1, ev->time);
        if(FXRGB(255,0,0) != this->getTextColor() && parent->getCurrent()*2 != parent->indexOfChild(this)) this->setTextColor(FXRGB(0,0,255));
    }
}

//handle IrcEvent IRC_UMODE
void IrcTabItem::OnIrcUmode(IrcEvent* ev)
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
                    FXString nick = utils::GetParam(args, argsiter, FALSE);
                    RemoveUser(nick);
                    if(sign)
                    {
                        AddUser(server->GetAdminPrefix()+nick);
                        if(!IsCommandIgnored("mode") && !server->IsUserIgnored(nick, getText()))
                        {
                            if(nick == server->GetNickName()) AppendIrcStyledText(FXStringFormat(_("%s gave you admin"), moderator.text()), 1, ev->time);
                            else AppendIrcStyledText(FXStringFormat(_("%s gave %s admin"), moderator.text(), nick.text()), 1, ev->time);
                        }
                    }
                    else
                    {
                        AddUser(nick);
                        if(!IsCommandIgnored("mode") && !server->IsUserIgnored(nick, getText()))
                        {
                            if(nick == server->GetNickName()) AppendIrcStyledText(FXStringFormat(_("%s removed you admin"), moderator.text()), 1, ev->time);
                            else AppendIrcStyledText(FXStringFormat(_("%s removed %s admin"), moderator.text(), nick.text()), 1, ev->time);
                        }
                    }
                    argsiter++;
                }break;
                case 'o': //op
                {
                    FXString nick = utils::GetParam(args, argsiter, FALSE);
                    RemoveUser(nick);
                    if(sign)
                    {
                        AddUser(server->GetOpPrefix()+nick);
                        if(!IsCommandIgnored("mode") && !server->IsUserIgnored(nick, getText()))
                        {
                            if(nick == server->GetNickName()) AppendIrcStyledText(FXStringFormat(_("%s gave you op"), moderator.text()), 1, ev->time);
                            else AppendIrcStyledText(FXStringFormat(_("%s gave %s op"), moderator.text(), nick.text()), 1, ev->time);
                        }
                    }
                    else
                    {
                        AddUser(nick);
                        if(!IsCommandIgnored("mode") && !server->IsUserIgnored(nick, getText()))
                        {
                            if(nick == server->GetNickName()) AppendIrcStyledText(FXStringFormat(_("%s removed you op"), moderator.text()), 1, ev->time);
                            else AppendIrcStyledText(FXStringFormat(_("%s removed %s op"), moderator.text(), nick.text()), 1, ev->time);
                        }
                    }
                    if (server->GetNickName() == nick) sign ? iamOp = TRUE : iamOp = FALSE;
                    argsiter++;
                }break;
                case 'v': //voice
                {
                    FXString nick = utils::GetParam(args, argsiter, FALSE);
                    RemoveUser(nick);
                    if(sign)
                    {
                        AddUser(server->GetVoicePrefix()+nick);
                        if(!IsCommandIgnored("mode") && !server->IsUserIgnored(nick, getText()))
                        {
                            if(nick == server->GetNickName()) AppendIrcStyledText(FXStringFormat(_("%s gave you voice"), moderator.text()), 1, ev->time);
                            else AppendIrcStyledText(FXStringFormat(_("%s gave %s voice"), moderator.text(), nick.text()), 1, ev->time);
                        }
                    }
                    else
                    {
                        AddUser(nick);
                        if(!IsCommandIgnored("mode") && !server->IsUserIgnored(nick, getText()))
                        {
                            if(nick == server->GetNickName()) AppendIrcStyledText(FXStringFormat(_("%s removed you voice"), moderator.text()), 1, ev->time);
                            else AppendIrcStyledText(FXStringFormat(_("%s removed %s voice"), moderator.text(), nick.text()), 1, ev->time);
                        }
                    }
                    argsiter++;
                }break;
                case 'h': //halfop
                {
                    FXString nick = utils::GetParam(args, argsiter, FALSE);
                    RemoveUser(nick);
                    if(sign)
                    {
                        AddUser(server->GetHalfopPrefix()+nick);
                        if(!IsCommandIgnored("mode") && !server->IsUserIgnored(nick, getText()))
                        {
                            if(nick == server->GetNickName()) AppendIrcStyledText(FXStringFormat(_("%s gave you halfop"), moderator.text()), 1, ev->time);
                            else AppendIrcStyledText(FXStringFormat(_("%s gave %s halfop"), moderator.text(), nick.text()), 1, ev->time);
                        }
                    }
                    else
                    {
                        AddUser(nick);
                        if(!IsCommandIgnored("mode") && !server->IsUserIgnored(nick, getText()))
                        {
                            if(nick == server->GetNickName()) AppendIrcStyledText(FXStringFormat(_("%s removed you halfop"), moderator.text()), 1, ev->time);
                            else AppendIrcStyledText(FXStringFormat(_("%s removed %s halfop"), moderator.text(), nick.text()), 1, ev->time);
                        }
                    }
                    argsiter++;
                }break;
                case 'b': //ban
                {
                    FXString banmask = utils::GetParam(args, argsiter, FALSE);
                    OnBan(banmask, sign, moderator, ev->time);
                    argsiter++;
                }break;
                case 't': //topic settable by channel operator
                {
                    sign ? editableTopic = FALSE : editableTopic = TRUE;
                }
                default:
                {
                    AppendIrcStyledText(FXStringFormat(_("%s set Mode: %s"), moderator.text(), FXString(modes+" "+args).text()), 1, ev->time);
                }
            }
        }
    }
}

//handle IrcEvent IRC_CHMODE
void IrcTabItem::OnIrcChmode(IrcEvent* ev)
{
    FXString channel = ev->param1;
    FXString modes = ev->param2;
    if(comparecase(channel, getText()) == 0)
    {
        if(modes.contains('t')) editableTopic = FALSE;
    }
}

//handle IrcEvent IRC_SERVERREPLY
void IrcTabItem::OnIrcServerReply(IrcEvent* ev)
{
    if(ownServerWindow)
    {
        if(type == SERVER)
        {
            //this->setText(server->GetRealServerName());
            AppendIrcText(ev->param1, ev->time);
            if(getApp()->getForeColor() == this->getTextColor() && parent->getCurrent()*2 != parent->indexOfChild(this)) this->setTextColor(FXRGB(0,0,255));
        }
    }
    else
    {
        if(IsFirst())
        {
            AppendIrcText(ev->param1, ev->time);
            if(FXRGB(255,0,0) != this->getTextColor() && parent->getCurrent()*2 != parent->indexOfChild(this)) this->setTextColor(FXRGB(0,0,255));
        }
    }
}

//handle IrcEvent IRC_CONNECT
void IrcTabItem::OnIrcConnect(IrcEvent* ev)
{
    AppendIrcStyledText(ev->param1, 3, ev->time);
}

//handle IrcEvent IRC_ERROR
void IrcTabItem::OnIrcError(IrcEvent* ev)
{
    AppendIrcStyledText(ev->param1, 4, ev->time);
}

//handle IrcEvent IRC_SERVERERROR
void IrcTabItem::OnIrcServerError(IrcEvent* ev)
{
    if(ownServerWindow)
    {
        if(type == SERVER)
        {
            AppendIrcStyledText(ev->param1, 4, ev->time);
            if(getApp()->getForeColor() == this->getTextColor() && parent->getCurrent()*2 != parent->indexOfChild(this)) this->setTextColor(FXRGB(0,0,255));
        }
    }
    else
    {
        if(IsFirst())
        {
            AppendIrcStyledText(ev->param1, 4, ev->time);
            if(FXRGB(255,0,0) != this->getTextColor() && parent->getCurrent()*2 != parent->indexOfChild(this)) this->setTextColor(FXRGB(0,0,255));
        }
    }
}

//handle IrcEvent IRC_DISCONNECT
void IrcTabItem::OnIrcDisconnect(IrcEvent* ev)
{
    AppendIrcStyledText(ev->param1, 4, ev->time);
    if(ownServerWindow)
    {
        if(type == SERVER)
        {
            if(parent->getCurrent()*2 != parent->indexOfChild(this)) this->setTextColor(FXRGB(255,0,0));
        }
    }
    else
    {
        if(IsFirst())
        {
            if(parent->getCurrent()*2 != parent->indexOfChild(this)) this->setTextColor(FXRGB(255,0,0));
        }
    }
    if(type == CHANNEL)
        users->clearItems();
}

//handle IrcEvent IRC_RECONNECT
void IrcTabItem::OnIrcReconnect(IrcEvent* ev)
{
    AppendIrcStyledText(ev->param1, 4, ev->time);
    if(ownServerWindow)
    {
        if(type == SERVER)
        {
            if(parent->getCurrent()*2 != parent->indexOfChild(this)) this->setTextColor(FXRGB(255,0,0));
        }
    }
    else
    {
        if(IsFirst())
        {
            if(parent->getCurrent()*2 != parent->indexOfChild(this)) this->setTextColor(FXRGB(255,0,0));
        }
    }
    if(type == CHANNEL)
        users->clearItems();
}

//handle IrcEvent IRC_UNKNOWN
void IrcTabItem::OnIrcUnknown(IrcEvent* ev)
{
    if(ownServerWindow)
    {
        if(type == SERVER)
        {
            AppendIrcStyledText(FXStringFormat(_("Unhandled command '%s' params: %s"), ev->param1.text(), ev->param2.text()), 4, ev->time);
            if(getApp()->getForeColor() == this->getTextColor() && parent->getCurrent()*2 != parent->indexOfChild(this)) this->setTextColor(FXRGB(0,0,255));
        }
    }
    else
    {
        if(IsFirst())
        {
            AppendIrcStyledText(FXStringFormat(_("Unhandled command '%s' params: %s"), ev->param1.text(), ev->param2.text()), 4, ev->time);
            if(FXRGB(255,0,0) != this->getTextColor() && parent->getCurrent()*2 != parent->indexOfChild(this)) this->setTextColor(FXRGB(0,0,255));
        }
    }
}

//handle IrcEvent IRC_301
void IrcTabItem::OnIrc301(IrcEvent* ev)
{
    if(parent->getCurrent()*2 == parent->indexOfChild(this) || getText() == ev->param1)
    {
        if(!IsCommandIgnored("away") && !server->IsUserIgnored(ev->param1, getText())) AppendIrcStyledText(FXStringFormat(_("%s is away: %s"),ev->param1.text(), ev->param2.text()), 1, ev->time);
    }
}

//handle IrcEvent IRC_305
void IrcTabItem::OnIrc305(IrcEvent* ev)
{
    FXint i = users->findItem(server->GetNickName());
    if(i != -1)
    {
        AppendIrcStyledText(ev->param1, 1, ev->time);
        if(users->getItemIcon(i) == irc_away_owner_icon)
        {
            users->setItemIcon(i, irc_owner_icon);
            return;
        }
        if(users->getItemIcon(i) == irc_away_admin_icon)
        {
            users->setItemIcon(i, irc_admin_icon);
            return;
        }
        if(users->getItemIcon(i) == irc_away_op_icon)
        {
            users->setItemIcon(i, irc_op_icon);
            return;
        }
        if(users->getItemIcon(i) == irc_away_voice_icon)
        {
            users->setItemIcon(i, irc_voice_icon);
            return;
        }
        if(users->getItemIcon(i) == irc_away_halfop_icon)
        {
            users->setItemIcon(i, irc_halfop_icon);
            return;
        }
        if(users->getItemIcon(i) == irc_away_normal_icon)
        {
            users->setItemIcon(i, irc_normal_icon);
            return;
        }
    }
}

//handle IrcEvent IRC_306
void IrcTabItem::OnIrc306(IrcEvent* ev)
{
    FXint i = users->findItem(server->GetNickName());
    if(i != -1)
    {
        AppendIrcStyledText(ev->param1, 1, ev->time);
        if(users->getItemIcon(i) == irc_owner_icon)
        {
            users->setItemIcon(i, irc_away_owner_icon);
            return;
        }
        if(users->getItemIcon(i) == irc_admin_icon)
        {
            users->setItemIcon(i, irc_away_admin_icon);
            return;
        }
        if(users->getItemIcon(i) == irc_op_icon)
        {
            users->setItemIcon(i, irc_away_op_icon);
            return;
        }
        if(users->getItemIcon(i) == irc_voice_icon)
        {
            users->setItemIcon(i, irc_away_voice_icon);
            return;
        }
        if(users->getItemIcon(i) == irc_halfop_icon)
        {
            users->setItemIcon(i, irc_away_halfop_icon);
            return;
        }
        if(users->getItemIcon(i) == irc_normal_icon)
        {
            users->setItemIcon(i, irc_away_normal_icon);
            return;
        }
    }
}

//handle IrcEvent IRC_331, IRC_332 and IRC_333
void IrcTabItem::OnIrc331332333(IrcEvent* ev)
{
    if(comparecase(ev->param1, getText()) == 0)
    {
        AppendIrcText(ev->param2, ev->time);
        if(ev->eventType == IRC_331)
        {
            topic = StripColors(ev->param2, TRUE);
            topicline->setText(topic);
        }
        if(ev->eventType == IRC_332)
        {
            topic = StripColors(utils::GetParam(ev->param2, 2, TRUE, ':').after(' '), TRUE);
            topicline->setText(topic);
        }
    }
}

//handle IrcEvent IRC_353
void IrcTabItem::OnIrc353(IrcEvent* ev)
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
            if(usersStr[0] == server->GetOpPrefix() && usersStr.before(' ').after(server->GetOpPrefix()) == myNick) iamOp = TRUE;
            usersStr = usersStr.after(' ');
        }
    }
    else
    {
        FXbool channelOn = FALSE;
        for(FXint i = 0; i<parent->numChildren(); i+=2)
        {
            if(server->FindTarget(static_cast<IrcTabItem*>(parent->childAtIndex(i))) && comparecase(static_cast<FXTabItem*>(parent->childAtIndex(i))->getText(), channel) == 0)
            {
                channelOn = TRUE;
                break;
            }
        }
        if(!channelOn && !IsCommandIgnored("numeric")) AppendIrcText(FXStringFormat(_("Users on %s: %s"), channel.text(), usersStr.text()), ev->time);
    }
}

//handle IrcEvent IRC_366
void IrcTabItem::OnIrc366(IrcEvent* ev)
{
    if(comparecase(ev->param1, getText()) == 0)
    {
        server->SendWho(getText());
        server->AddIgnoreCommands("who "+getText());
        if(numberUsers < maxAway)
        {
            checkAway = TRUE;
            getApp()->addTimeout(this, ID_TIME, 180000);
        }
    }
}

//handle IrcEvent IRC_372
void IrcTabItem::OnIrc372(IrcEvent* ev)
{
    if(ownServerWindow)
    {
        if(type == SERVER)
        {
            AppendIrcText(ev->param1, ev->time);
            if(getApp()->getForeColor() == this->getTextColor() && parent->getCurrent()*2 != parent->indexOfChild(this)) this->setTextColor(FXRGB(0,0,255));
        }
    }
    else
    {
        if(IsFirst())
        {
            AppendIrcText(ev->param1, ev->time);
            if(FXRGB(255,0,0) != this->getTextColor() && parent->getCurrent()*2 != parent->indexOfChild(this)) this->setTextColor(FXRGB(0,0,255));
        }
    }
}

//handle IrcEvent IRC_AWAY
void IrcTabItem::OnIrcAway(IrcEvent* ev)
{
    if(comparecase(ev->param1, getText()) == 0)
    {
        OnAway();
    }
}

//handle IrcEvent IRC_ENDMOTD
void IrcTabItem::OnIrcEndMotd()
{
    MakeLastRowVisible(TRUE);
}

long IrcTabItem::OnPipe(FXObject*, FXSelector, void *ptr)
{
    FXString text = *(FXString*)ptr;    
    if(sendPipe && (type == CHANNEL || type == QUERY))
    {
        if(!getApp()->hasTimeout(this, ID_PTIME)) getApp()->addTimeout(this, ID_PTIME);
        pipeStrings.append(text);
    }
    else AppendIrcText(text, FXSystem::now());
    return 1;
}

//timeout for checking away in channel
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
            checkAway = TRUE;
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
                    if(coloredNick) AppendIrcNickText(server->GetNickName(), messages[i], GetNickColor(server->GetNickName()), FXSystem::now());
                    else AppendIrcNickText(server->GetNickName(), messages[i], 5, FXSystem::now());
                    server->SendMsg(getText(), messages[i]);
                }
            }
            else
            {
                if(coloredNick) AppendIrcNickText(server->GetNickName(), pipeStrings[0], GetNickColor(server->GetNickName()), FXSystem::now());
                else AppendIrcNickText(server->GetNickName(), pipeStrings[0], 5, FXSystem::now());
                server->SendMsg(getText(), pipeStrings[0]);
            }
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
                        if(coloredNick) AppendIrcNickText(server->GetNickName(), messages[i], GetNickColor(server->GetNickName()), FXSystem::now());
                        else AppendIrcNickText(server->GetNickName(), messages[i], 5, FXSystem::now());
                        server->SendMsg(getText(), messages[i]);
                    }
                }
                else
                {
                    if(coloredNick) AppendIrcNickText(server->GetNickName(), pipeStrings[0], GetNickColor(server->GetNickName()), FXSystem::now());
                    else AppendIrcNickText(server->GetNickName(), pipeStrings[0], 5, FXSystem::now());
                    server->SendMsg(getText(), pipeStrings[0]);
                }
                pipeStrings.erase(0);
            }
        }
    }
    return 1;
}

long IrcTabItem::OnEggTimeout(FXObject*, FXSelector, void*)
{
    FXString pic1 = "     ,.    ,-.\n   ,'  `. /  ,--.\n  /      ' ,'  \\ `.\n '  .--, |/     '  \\\n |.'    `| ,-.  |   |\n /   ,-. |/   \\     |\n|   /   \\|     |    |\n|   | ,--!--.  |    |\n     (   |   )\n      `-----'\n";
    FXString pic2 = "     .,    .-,\n   .'  `, /  .-,.\n  /      \" .`  \\ ',\n |  ,-,. |/     |  \\\n ','    `| .,.  '   '\n /   .-, !/   \\     |\n'   / ,-\\|--.  '    |\n|   !'   |   `.|    !\n   (     |     )\n    `.       ,'\n      `-----'\n";
    FXString pic3 = "     ,.    .-.\n   ,'  '. /  ,--,\n  '      ' .'  \\ `,\n '  ,,-. !/     '  \\\n |,`    '| .-,  |   |\n /   ,-.-|/--.\\     |\n|  ,/   \\|    `|    '\n' / |    |     '\\   |\n (     ( | )     )\n  \\             /\n   `.         ,'\n     '-------'\n";
    if(pics<24)
    {
        
        getApp()->addTimeout(this, ID_ETIME, 222);
        text->removeText(0, text->getLength());
        text->layout();
        if((pics)%3==0)
        {
            text->appendStyledText(pic1, 3);
            pics++;
            return 1;
        }
        if((pics)%3==1)
        {
            text->appendStyledText(pic2, 3);
            pics++;
            return 1;
        }
        text->appendStyledText(pic3, 3);
        pics++;
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
        FXString word;
        FXString link = "";
        text->extractText(word, text->wordStart(pos), text->wordEnd(pos)-text->wordStart(pos));
        for(FXint i=text->wordStart(pos); i<text->wordEnd(pos); i++)
        {
            if(text->getStyle(i) == 9) link.append(word[i-text->wordStart(pos)]);
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
        if(FXStat::exists(flagpath+PATHSEPSTRING+nick.host.rafter('.')+".png")) flagicon = MakeIcon(getApp(), flagpath, nick.host.rafter('.')+".png", TRUE);
        else flagicon = MakeIcon(getApp(), flagpath, "unknown.png", TRUE);
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
            new FXMenuCommand(&popup, _("DCC chat"), NULL, this, ID_DCCCHAT);
            new FXMenuCommand(&popup, _("Send file"), NULL, this, ID_DCCSEND);
            if(iamOp) new FXMenuCascade(&popup, _("Operator actions"), NULL, &opmenu);
        }
        popup.create();
        popup.popup(NULL,event->root_x,event->root_y);
        getApp()->runModalWhileShown(&popup);
    }
    return 1;
}

long IrcTabItem::OnDoubleclick(FXObject*, FXSelector, void*)
{
    FXint index = users->getCursorItem();
    if(index >= 0)
    {
        if(users->getItemText(index) == server->GetNickName()) return 1;
        IrcEvent ev;
        ev.eventType = IRC_QUERY;
        ev.param1 = users->getItemText(index);
        ev.param2 = server->GetNickName();
        parent->getParent()->getParent()->handle(server, FXSEL(SEL_COMMAND, IrcSocket::ID_SERVER), &ev);
    }
    return 1;
}

long IrcTabItem::OnNewQuery(FXObject *, FXSelector, void *)
{
    IrcEvent ev;
    ev.eventType = IRC_QUERY;
    ev.param1 = nickOnRight.nick;
    ev.param2 = server->GetNickName();
    parent->getParent()->getParent()->handle(server, FXSEL(SEL_COMMAND, IrcSocket::ID_SERVER), &ev);
    return 1;
}

long IrcTabItem::OnWhois(FXObject *, FXSelector, void *)
{
    server->SendWhois(nickOnRight.nick);
    return 1;
}

long IrcTabItem::OnDccChat(FXObject*, FXSelector, void*)
{
    IrcEvent ev;
    ev.eventType = IRC_DCCSERVER;
    ev.param1 = nickOnRight.nick;
    parent->getParent()->getParent()->handle(server, FXSEL(SEL_COMMAND, IrcSocket::ID_SERVER), &ev);
    return 1;
}

long IrcTabItem::OnDccSend(FXObject*, FXSelector, void*)
{
    DccSendDialog dialog((FXMainWindow*)parent->getParent()->getParent(), nickOnRight.nick);
    if(dialog.execute())
    {
        IrcEvent ev;
        ev.eventType = dialog.GetPassive() ? IRC_DCCPOUT: IRC_DCCOUT;
        ev.param1 = nickOnRight.nick;
        ev.param2 = dialog.GetFilename();
        parent->getParent()->getParent()->handle(server, FXSEL(SEL_COMMAND, IrcSocket::ID_SERVER), &ev);
    }
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

    FXHorizontalFrame *buttonframe = new FXHorizontalFrame(contents, LAYOUT_FILL_X|LAYOUT_FILL_Y|PACK_UNIFORM_WIDTH);
    new FXButton(buttonframe, _("&Cancel"), NULL, &kickDialog, FXDialogBox::ID_CANCEL, FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0, 0, 0, 0, 10, 10, 2, 5);
    new FXButton(buttonframe, _("&OK"), NULL, &kickDialog, FXDialogBox::ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0, 0, 0, 0, 10, 10, 2, 5);

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

    FXHorizontalFrame *buttonframe = new FXHorizontalFrame(contents, LAYOUT_FILL_X|LAYOUT_FILL_Y|PACK_UNIFORM_WIDTH);
    new FXButton(buttonframe, _("&Cancel"), NULL, &banDialog, FXDialogBox::ID_CANCEL, FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0, 0, 0, 0, 10, 10, 2, 5);
    new FXButton(buttonframe, _("&OK"), NULL, &banDialog, FXDialogBox::ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0, 0, 0, 0, 10, 10, 2, 5);

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

    FXHorizontalFrame *buttonframe = new FXHorizontalFrame(contents, LAYOUT_FILL_X|LAYOUT_FILL_Y|PACK_UNIFORM_WIDTH);
    new FXButton(buttonframe, _("&Cancel"), NULL, &banDialog, FXDialogBox::ID_CANCEL, FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0, 0, 0, 0, 10, 10, 2, 5);
    new FXButton(buttonframe, _("&OK"), NULL, &banDialog, FXDialogBox::ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0, 0, 0, 0, 10, 10, 2, 5);

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
        if(topicline->getText().length() > server->GetTopicLen())
        {
            AppendIrcStyledText(FXStringFormat(_("Warning: topic is too long. Max. topic length is %d."), server->GetTopicLen()), 4, FXSystem::now());
            server->SendTopic(getText(), topicline->getText());
            return 1;
        }
        server->SendTopic(getText(), topicline->getText());
    }
    else topicline->setText(topic);
    return 1;
}

void IrcTabItem::OnBan(const FXString &banmask, const FXbool &sign, const FXString &sender, const FXTime &time)
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
                    if(users->getItemText(i) == myNick) AppendIrcStyledText(FXStringFormat(_("You was banned by %s"), sender.text()), 1, time);
                    else
                    {
                        if(!IsCommandIgnored("ban") && !server->IsUserIgnored(users->getItemText(i), getText())) AppendIrcStyledText(FXStringFormat(_("%s was banned by %s"), users->getItemText(i).text(), sender.text()), 1, time);
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

FXString IrcTabItem::GetNick(int i)
{
    return users->getItemText(i);
}

FXint IrcTabItem::LaunchLink(const FXString &link)
{
#ifdef WIN32
    return ((FXint)ShellExecuteA(NULL,"open",FXPath::enquote(link).text(),NULL,NULL,SW_SHOWNORMAL)) > 32;
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
    exec += " "+FXPath::enquote(link)+" &";
    system(exec.text());
    return 1;
#endif
}

FXint IrcTabItem::GetNickColor(const FXString &nick)
{
    //10 is first colored nick style
    return 10+nick.hash()%8;
}

FXColor IrcTabItem::GetIrcColor(FXint code)
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
            return colors.text;
    }
}

FXint IrcTabItem::HiliteStyleExist(FXColor foreColor, FXColor backColor, FXuint style)
{
    for(FXint i=0; i<textStyleList.no(); i++)
    {
        if(textStyleList[i].normalForeColor == foreColor
                && textStyleList[i].normalBackColor == backColor
                && textStyleList[i].style == style)
            return i+1;
    }
    return -1;
}

void IrcTabItem::CreateHiliteStyle(FXColor foreColor, FXColor backColor, FXuint style)
{
    FXHiliteStyle nstyle = {foreColor,backColor,getApp()->getSelforeColor(),getApp()->getSelbackColor(),getApp()->getHiliteColor(),FXRGB(255, 128, 128),colors.back,style};
    textStyleList.append(nstyle);
    text->setHiliteStyles(textStyleList.data());
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

void IrcTabItem::SetCommandFocus()
{
    commandline->setFocus();
}

//for "handle" checking, if script contains "all". Send from dxirc.
void IrcTabItem::HasAllCommand(FXbool result)
{
    scriptHasAll = result;
}

//for "handle" checking, if script contains "mymsg". Send from dxirc.
void IrcTabItem::HasMyMsg(FXbool result)
{
    scriptHasMyMsg = result;
}
