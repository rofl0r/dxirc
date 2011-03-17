/*
 *      irctabitem.h
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


#ifndef IRCTABITEM_H
#define IRCTABITEM_H

#include <fstream>
#include "ircengine.h"
#include "dxtabitem.h"
#include "dxpipe.h"
#include "defs.h"
#ifndef DXTEXT_H
#include "dxtext.h"
#endif
#include "fxext.h"

class dxirc;

class DccSendDialog: public FXDialogBox
{
    FXDECLARE(DccSendDialog)
public:
    DccSendDialog(FXMainWindow *owner, FXString nick);
    virtual ~DccSendDialog();

    long onSend(FXObject*,FXSelector,void*);
    long onCancel(FXObject*,FXSelector,void*);
    long onFile(FXObject*,FXSelector,void*);
    long onKeyPress(FXObject*,FXSelector,void*);

    FXuint execute(FXuint placement=PLACEMENT_CURSOR);
    FXString getFilename() { return m_fileText->getText(); }
    FXbool getPassive() { return m_checkPassive->getCheck(); }

private:
    DccSendDialog() {}
    DccSendDialog(const DccSendDialog&);

    FXVerticalFrame *m_mainFrame;
    FXHorizontalFrame *m_fileFrame, *m_passiveFrame, *m_buttonFrame;
    FXTextField *m_fileText;
    dxEXButton *m_buttonFile, *m_buttonSend, *m_buttonCancel;
    FXCheckButton *m_checkPassive;
};

class IrcTabItem;

class NickList : public FXList
{
    FXDECLARE(NickList)
public:
    NickList(FXComposite *p, IrcTabItem *tgt=NULL, FXSelector sel=0, FXuint opts=LIST_NORMAL, FXint x=0, FXint y=0, FXint w=0, FXint h=0);
    long onQueryTip(FXObject*,FXSelector,void*);
protected:
    NickList() {}
private:
    NickList(const NickList&);
    NickList& operator=(const NickList&);
    IrcTabItem *m_parent;
};

class NickListItem : public FXListItem
{
    FXDECLARE(NickListItem)
    friend class NickList;
    friend class IrcTabItem;
public:
    NickListItem(const FXString& text, NickList *parent, IrcTabItem *tab, UserMode mode=NONE, FXIcon* ic=NULL, void* ptr=NULL):FXListItem(text, ic, ptr),m_mode(mode),m_tab(tab),m_parent(parent){}
protected:
    NickListItem() {}

    void setUserMode(UserMode mode);
    UserMode getUserMode() const { return m_mode; }
    void changeAway(FXbool away);
private:
    NickListItem(const NickListItem&);
    NickListItem& operator=(const NickListItem&);
    UserMode m_mode;
    IrcTabItem *m_tab;
    NickList *m_parent;
};

class IrcTabItem: public dxTabItem
{
    FXDECLARE(IrcTabItem)
    friend class dxirc;
    friend class NickList;
    friend class NickListItem;
    public:
        IrcTabItem(dxTabBook *tab, const FXString &tabtext, FXIcon *icon=0, FXuint opts=TAB_TOP_NORMAL, FXint id=0, TYPE type=CHANNEL, IrcEngine *socket=NULL, FXbool ownServerWindow=FALSE, FXbool usersShown=TRUE, FXbool logging=FALSE, FXString commandsList="", FXString logPath="", FXint maxAway=200, IrcColor colorss=IrcColor(), FXString nickChar=":", FXFont *font=NULL, FXbool sameCommand=FALSE, FXbool sameList=FALSE, FXbool coloredNick=FALSE, FXbool stripColors=TRUE, FXbool useSpell=TRUE, FXbool showSpellCombo=FALSE);
        virtual ~IrcTabItem();

        void createGeom();
        void clearChat();
        void hideUsers();
        void showUsers();
        FXString getServerName() { return m_engine ? m_engine->getServerName() : ""; }
        FXint getServerPort() { return m_engine ? m_engine->getServerPort() : 0; }
        FXString getNickName() { return m_engine ? m_engine->getNickName() : ""; }
        FXString getRealServerName() { return m_engine ? m_engine->getRealServerName() : ""; }
        void setType(const TYPE &typ, const FXString &tabtext);
        TYPE getType() { return m_type; }
        void reparentTab();
        void setColor(IrcColor);
        void setCommandsList(FXString clst);
        void setMaxAway(FXint maxa);
        void setLogging(FXbool log);
        void setLogPath(FXString pth);
        void setNickCompletionChar(FXString nichr);
        void setIrcFont(FXFont*);
        void setSameCmd(FXbool);
        void setSameList(FXbool);
        void setColoredNick(FXbool);
        void setStripColors(FXbool);
        void setCommandFocus();
        void setSmileys(FXbool smiley, dxSmileyArray nsmileys);
        void setUseSpell(FXbool useSpell);
        void setShowSpellCombo(FXbool showSpellCombo);
        void removeSmileys();
        void makeLastRowVisible();
        FXString getSpellLang();
        void setUnreadTabColor(FXColor);
        void setHighlightTabColor(FXColor);
        void checkAway();

        long onCommandline(FXObject *, FXSelector, void*);
        long onKeyPress(FXObject *, FXSelector, void*);
        long onIrcEvent(FXObject *, FXSelector, void*);
        long onEggTimeout(FXObject *, FXSelector, void*);
        long onPipeTimeout(FXObject *, FXSelector, void*);
        long onRightMouse(FXObject *, FXSelector, void*);
        long onDoubleclick(FXObject *, FXSelector, void*);
        long onTextLink(FXObject *, FXSelector, void *);
        long onNewQuery(FXObject *, FXSelector, void*);
        long onWhois(FXObject *, FXSelector, void*);
        long onOp(FXObject *, FXSelector, void*);
        long onDeop(FXObject *, FXSelector, void*);
        long onVoice(FXObject *, FXSelector, void*);
        long onDevoice(FXObject *, FXSelector, void*);
        long onKick(FXObject *, FXSelector, void*);
        long onBan(FXObject *, FXSelector, void*);
        long onKickban(FXObject *, FXSelector, void*);
        long onIgnore(FXObject *, FXSelector, void*);
        long onTopic(FXObject *, FXSelector, void*);
        long onTopicLink(FXObject *, FXSelector, void*);
        long onPipe(FXObject *, FXSelector, void*);
        long onDccChat(FXObject *, FXSelector, void*);
        long onDccSend(FXObject *, FXSelector, void*);
        long onSetAway(FXObject *, FXSelector, void*);
        long onRemoveAway(FXObject *, FXSelector, void*);
        long onSpellLang(FXObject *, FXSelector, void*);

    protected:        
        void appendText(FXString msg, FXbool highlight, FXbool logLine=TRUE);
        void appendStyledText(FXString text, FXint style, FXbool highlight, FXbool disableStrip=FALSE, FXbool logLine=TRUE);
        FXbool processLine(const FXString &);
        void hasAllCommand(FXbool);
        void hasMyMsg(FXbool);

        IrcEngine *m_engine;
    private:
        IrcTabItem(){}

        dxTabBook *m_parent;
        dxPipe *m_pipe;
        TYPE m_type;
        FXVerticalFrame *m_textframe, *m_usersframe, *m_mainframe;
        FXHorizontalFrame *m_commandframe;
        FXSplitter *m_splitter;
        dxText *m_text;
        NickList *m_users;
        dxTextField *m_commandline;
        dxTextField *m_topicline;
        FXint m_currentPosition, m_historyMax, m_numberUsers, m_pics;
        FXbool m_iamOp, m_usersShown, m_logging, m_ownServerWindow;
        FXbool m_sameCmd, m_sameList, m_coloredNick, m_editableTopic, m_sendPipe;
        FXbool m_scriptHasAll, m_scriptHasMyMsg, m_stripColors, m_useSpell, m_showSpellCombo;
        dxStringArray m_commandsHistory, m_pipeStrings;
        dxHiliteArray m_textStyleList;
        IrcColor m_colors;
        NickInfo m_nickOnRight;
        FXString m_commandsList, m_logPath, m_topic;
        FXint m_maxAway, m_maxLen;
        FXString m_nickCompletionChar;
        std::ofstream *m_logstream;
        FXComboBox *m_spellLangs;
        FXColor m_unreadColor, m_highlightColor;
        

        void onIrcPrivmsg(IrcEvent *ev);
        void onIrcAction(IrcEvent *ev);
        void onIrcCtpcReply(IrcEvent *ev);
        void onIrcCtcpRequest(IrcEvent *ev);
        void onIrcJoin(IrcEvent *ev);
        void onIrcQuit(IrcEvent *ev);
        void onIrcPart(IrcEvent *ev);
        void onIrcChnotice(IrcEvent *ev);
        void onIrcNotice(IrcEvent *ev);
        void onIrcNick(IrcEvent *ev);
        void onIrcTopic(IrcEvent *ev);
        void onIrcInvite(IrcEvent *ev);
        void onIrcKick(IrcEvent *ev);
        void onIrcMode(IrcEvent *ev);
        void onIrcUmode(IrcEvent *ev);
        void onIrcChmode(IrcEvent *ev);
        void onIrcServerReply(IrcEvent *ev);
        void onIrcConnect(IrcEvent *ev);
        void onIrcError(IrcEvent *ev);
        void onIrcServerError(IrcEvent *ev);
        void onIrcDisconnect(IrcEvent *ev);
        void onIrcReconnect(IrcEvent *ev);
        void onIrcUnknown(IrcEvent *ev);
        void onIrc301(IrcEvent *ev);
        void onIrc305(IrcEvent *ev);
        void onIrc306(IrcEvent *ev);
        void onIrc331332333(IrcEvent *ev);
        void onIrc353(IrcEvent *ev);
        void onIrc366(IrcEvent *ev);
        void onIrc372(IrcEvent *ev);
        void onIrcAway(IrcEvent *ev);
        void onIrcEndMotd();
        FXString stripColors(const FXString &text, const FXbool stripOther);
        void appendIrcText(FXString msg, FXTime time, FXbool disableStrip=FALSE, FXbool logLine=TRUE);
        void appendIrcStyledText(FXString styled, FXint stylenum, FXTime time, FXbool disableStrip=FALSE, FXbool logLine=TRUE);
        void appendIrcNickText(FXString nick, FXString msg, FXint style, FXTime time, FXbool logLine=TRUE);
        void appendLinkText(const FXString &, FXint);
        FXString getNick(FXint);
        FXbool isRightForServerMsg();
        FXbool isCommandIgnored(const FXString &command);
        void addUser(const FXString &user, UserMode mode);
        void removeUser(const FXString &user);
        void changeNickUser(const FXString &nick, const FXString &newnick);
        void setUserMode(const FXString &nick, UserMode mode);
        UserMode getUserMode(const FXchar mode);
        void onBan(const FXString &banmask, const FXbool &sign, const FXString &sender, const FXTime &time);
        void onAway();
        void startLogging();
        void stopLogging();
        void logLine(const FXString &line, const FXTime &time);
        FXbool isChannel(const FXString &text);
        void setTextForeColor(FXColor);
        void setTextBackColor(FXColor);
        void setUserColor(FXColor);
        void setActionsColor(FXColor);
        void setNoticeColor(FXColor);
        void setErrorColor(FXColor);
        void setHilightColor(FXColor);
        void setLinkColor(FXColor);
        FXbool processCommand(const FXString &);
        FXbool showHelp(FXString);
        FXint getNickColor(const FXString &);
        FXColor getIrcColor(FXint code);
        FXint hiliteStyleExist(FXColor foreColor, FXColor backColor, FXuint style);
        void createHiliteStyle(FXColor foreColor, FXColor backColor, FXuint style);
        dxStringArray cutText(FXString, FXint);
        FXbool highlightNeeded(const FXString &);
};

#endif // IRCTABITEM_H
