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
#include "defs.h"

class IrcTabItem: public FXTabItem
{
    FXDECLARE(IrcTabItem)
    public:
        IrcTabItem(FXTabBook*, const FXString&, FXIcon*, FXuint, TYPE, IrcSocket*, FXbool, FXbool, FXbool, FXString, FXString, FXint, IrcColor, FXString, FXFont*, FXbool, FXbool);
        virtual ~IrcTabItem();
        enum {
            ID_COMMANDLINE = FXTabItem::ID_LAST,
            ID_CONNECT,
            ID_TABQUIT,
            ID_TIME,
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
            ID_LAST
        };

        void CreateGeom();
        void ClearChat();
        void HideUsers();
        void ShowUsers();
        FXString GetServerName() { return server->GetServerName(); }
        void SetType(const TYPE &typ, const FXString &tabtext);
        TYPE GetType() { return type; }
        void ReparentTab();
        void MakeLastRowVisible(FXbool);
        void SetColor(IrcColor);
        void SetCommandsList(FXString clst);
        void SetMaxAway(FXint maxa);
        void SetLogging(FXbool log);
        void SetLogPath(FXString pth);
        void SetNickCompletionChar(FXString nichr);
        void SetIrcFont(FXFont*);
        void SetSameCmd(FXbool);
        void SetSameList(FXbool);

        long OnCommandline(FXObject *, FXSelector, void*);
        long OnKeyPress(FXObject *, FXSelector, void*);
        long OnIrcEvent(FXObject *, FXSelector, void*);
        long OnTimeout(FXObject *, FXSelector, void*);
        long OnRightMouse(FXObject *, FXSelector, void*);
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

    private:
        IrcTabItem(){}

        FXTabBook *parent;
        IrcSocket *server;
        TYPE type;
        FXVerticalFrame *textframe, *usersframe, *mainframe;
        FXSplitter *splitter;
        FXText *text;
        FXList *users;
        FXTextField *commandline, *topicline;
        FXint currentPosition, historyMax, numberUsers;
        FXbool checkAway, iamOp, usersShown, logging, ownServerWindow, sameCmd, sameList, editableTopic;
        FXHistoryArray commandsHistory;
        FXHiliteStyle textStyleList[8];
        IrcColor colors;
        NickInfo nickOnRight;
        FXString commandsList, logPath, topic;
        FXint maxAway;
        FXString nickCompletionChar;
        std::ofstream *logstream;

        FXString GetParam(FXString toParse, int n, bool toEnd);
        FXString GetParam(FXString toParse, int n, bool toEnd, const FXchar &separator);
        FXString StripColors(const FXString &text, const FXbool stripOther);
        void AppendIrcText(FXString);
        void AppendIrcStyledText(FXString, FXint);
        FXString CreateModes(FXchar sign, FXchar mode, FXString nicks);
        FXString GetNick(int);
        FXbool IsCurrent();
        FXbool IsNoCurrent();
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
};

#endif // IRCTABITEM_H
