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
#include "ircsocket.h"
#include "dxpipe.h"
#include "defs.h"

class dxirc;

class dxText: public FXText
{
    FXDECLARE(dxText)
public:
    dxText(FXComposite *p, FXObject *tgt, FXSelector sel, FXuint opts);
    long onMotion(FXObject*, FXSelector, void*);
private:
    dxText() {}
};

class DccSendDialog: public FXDialogBox
{
    FXDECLARE(DccSendDialog)
public:
    DccSendDialog(FXMainWindow *owner, FXString nick);
    virtual ~DccSendDialog();
    enum {
        ID_SEND = FXTopWindow::ID_LAST,
        ID_CANCEL,
        ID_FILE
    };

    long OnSend(FXObject*,FXSelector,void*);
    long OnCancel(FXObject*,FXSelector,void*);
    long OnFile(FXObject*,FXSelector,void*);
    long OnKeyPress(FXObject*,FXSelector,void*);

    FXuint execute (FXuint placement=PLACEMENT_CURSOR);
    FXString GetFilename() { return fileText->getText(); }
    FXbool GetPassive() { return checkPassive->getCheck(); }

private:
    DccSendDialog() {}
    DccSendDialog(const DccSendDialog&);

    FXVerticalFrame *mainFrame;
    FXHorizontalFrame *fileFrame, *passiveFrame, *buttonFrame;
    FXTextField *fileText;
    FXButton *buttonFile, *buttonSend, *buttonCancel;
    FXCheckButton *checkPassive;
};

class IrcTabItem: public FXTabItem
{
    FXDECLARE(IrcTabItem)
    friend class dxirc;
    public:
        IrcTabItem(FXTabBook*, const FXString&, FXIcon*, FXuint, TYPE, IrcSocket*, FXbool, FXbool, FXbool, FXString, FXString, FXint, IrcColor, FXString, FXFont*, FXbool, FXbool, FXbool);
        virtual ~IrcTabItem();
        enum {
            ID_COMMANDLINE = FXMainWindow::ID_LAST+25,
            ID_CDIALOG,
            ID_CQUIT,
            ID_TIME,
            ID_PTIME,
            ID_USERS,
            ID_NEWQUERY,
            ID_WHOIS,
            ID_OP,
            ID_DEOP,
            ID_VOICE,
            ID_DEVOICE,
            ID_KICK,
            ID_BAN,
            ID_KICKBAN,
            ID_TOPIC,
            ID_CSERVER,
            ID_NEXTTAB,
            ID_TEXT,
            ID_NEWMSG,
            ID_LUA,
            ID_COMMAND,
            ID_NEWTETRIS,
            ID_DCCCHAT, //dcc chat on right click
            ID_DCCSEND, //dcc sending on right click
            ID_LAST
        };

        void CreateGeom();
        void ClearChat();
        void HideUsers();
        void ShowUsers();
        FXString GetServerName() { return server->GetServerName(); }
        FXint GetServerPort() { return server->GetServerPort(); }
        FXString GetNickName() { return server->GetNickName(); }
        void SetType(const TYPE &typ, const FXString &tabtext);
        TYPE GetType() { return type; }
        void ReparentTab();        
        void SetColor(IrcColor);
        void SetCommandsList(FXString clst);
        void SetMaxAway(FXint maxa);
        void SetLogging(FXbool log);
        void SetLogPath(FXString pth);
        void SetNickCompletionChar(FXString nichr);
        void SetIrcFont(FXFont*);
        void SetSameCmd(FXbool);
        void SetSameList(FXbool);
        void SetColoredNick(FXbool);
        void SetCommandFocus();        

        long OnCommandline(FXObject *, FXSelector, void*);
        long OnKeyPress(FXObject *, FXSelector, void*);
        long OnIrcEvent(FXObject *, FXSelector, void*);
        long OnTimeout(FXObject *, FXSelector, void*);
        long OnPipeTimeout(FXObject *, FXSelector, void*);
        long OnRightMouse(FXObject *, FXSelector, void*);
        long OnDoubleclick(FXObject *, FXSelector, void*);
        long OnLeftMouse(FXObject *, FXSelector, void *);
        long OnNewQuery(FXObject *, FXSelector, void*);
        long OnWhois(FXObject *, FXSelector, void*);
        long OnOp(FXObject *, FXSelector, void*);
        long OnDeop(FXObject *, FXSelector, void*);
        long OnVoice(FXObject *, FXSelector, void*);
        long OnDevoice(FXObject *, FXSelector, void*);
        long OnKick(FXObject *, FXSelector, void*);
        long OnBan(FXObject *, FXSelector, void*);
        long OnKickban(FXObject *, FXSelector, void*);
        long OnTopic(FXObject *, FXSelector, void*);
        long OnPipe(FXObject *, FXSelector, void*);
        long OnDccChat(FXObject *, FXSelector, void*);
        long OnDccSend(FXObject *, FXSelector, void*);

    protected:        
        void AppendIrcText(FXString, FXbool);
        void AppendIrcStyledText(FXString, FXint, FXbool);
        FXbool ProcessLine(const FXString &);
        void MakeLastRowVisible(FXbool);
        void HasAllCommand(FXbool);

    private:
        IrcTabItem(){}

        FXTabBook *parent;
        IrcSocket *server;
        dxPipe *pipe;
        TYPE type;
        FXVerticalFrame *textframe, *usersframe, *mainframe;
        FXSplitter *splitter;
        dxText *text;
        FXList *users;
        FXTextField *commandline, *topicline;
        FXint currentPosition, historyMax, numberUsers;
        FXbool checkAway, iamOp, usersShown, logging, ownServerWindow;
        FXbool sameCmd, sameList, coloredNick, editableTopic, sendPipe, scriptHasAll;
        dxStringArray commandsHistory, pipeStrings;
        FXHiliteStyle textStyleList[17];
        IrcColor colors;
        NickInfo nickOnRight;
        FXString commandsList, logPath, topic;
        FXint maxAway, maxLen;
        FXString nickCompletionChar;
        std::ofstream *logstream;        
        

        FXString StripColors(const FXString &text, const FXbool stripOther);
        void AppendIrcText(FXString);
        void AppendIrcStyledText(FXString, FXint);
        void AppendIrcNickText(FXString, FXString, FXint);
        void AppendLinkText(const FXString &, FXint);
        FXString GetNick(int);
        FXbool IsFirst();
        FXbool IsCommandIgnored(const FXString &command);
        void AddUser(const FXString &user);
        void RemoveUser(const FXString &user);
        void ChangeNickUser(const FXString &nick, const FXString &newnick);
        void OnBan(const FXString &banmask, const FXbool &sign, const FXString &sender);
        void OnAway();
        void StartLogging();
        void StopLogging();
        void LogLine(const FXString &line);
        FXbool IsChannel(const FXString &text);
        void SetTextColor(FXColor);
        void SetTextBackColor(FXColor);
        void SetUserColor(FXColor);
        void SetActionsColor(FXColor);
        void SetNoticeColor(FXColor);
        void SetErrorColor(FXColor);
        void SetHilightColor(FXColor);
        void SetLinkColor(FXColor);        
        FXbool ProcessCommand(const FXString &);
        FXbool ShowHelp(FXString);
        FXint LaunchLink(const FXString &);
        FXint GetNickColor(const FXString &);
        dxStringArray CutText(FXString, FXint);
};

#endif // IRCTABITEM_H
