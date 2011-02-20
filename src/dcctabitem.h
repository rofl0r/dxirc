/*
 *      dcctabitem.h
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


#ifndef DCCTABITEM_H
#define DCCTABITEM_H

#include <fstream>
#include "dxtabitem.h"
#include "dxpipe.h"
#ifndef DXSOCKET_H
#include "dxsocket.h"
#endif

class dxirc;
class IrcEngine;

class DccTabItem: public dxTabItem
{
    FXDECLARE(DccTabItem)
    friend class dxirc;
    public:
        DccTabItem(dxTabBook *tab, const FXString &mynick, const FXString &nick, const FXString &address="0", FXint portD=0, FXint portH=0, FXbool listen=FALSE, FXIcon *icon=0, FXuint opts=TAB_TOP_NORMAL, FXint id=0, FXbool logging=FALSE, FXString logPath="", IrcColor colorss=IrcColor(), FXFont *font=NULL, FXbool sameCommand=FALSE, FXbool coloredNick=FALSE, FXbool stripColors=TRUE, FXbool useSpell=TRUE, FXbool showSpellCombo=FALSE, IrcEngine *engine=NULL);
        virtual ~DccTabItem();
        enum {
            ID_COMMANDLINE = FXMainWindow::ID_LAST+25,
            ID_PTIME,
            ID_NEXTTAB,
            ID_TEXT,
            ID_SPELL,
            ID_MYMSG,
            ID_COMMAND,
            ID_SOCKET,
            ID_CTIME, //timeout for check and disconnect offered connection
            ID_LUA,
            ID_NEWTETRIS,
            ID_NEWMSG,
            ID_LAST
        };

        void createGeom();
        void clearChat();
        FXString getServerName() { return m_address; }
        FXint getServerPort() { return m_portD; }
        FXString getNickName() { return m_mynick; }
        FXString getRealServerName() { return m_address; }
        void reparentTab();
        void setColor(IrcColor);
        void setLogging(FXbool log);
        void setLogPath(FXString pth);
        void setIrcFont(FXFont*);
        void setSameCmd(FXbool);
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
        FXbool getConnected() { return m_socket->isConnected(); }
        void disconnect() { m_socket->disconnect(); }

        long onCommandline(FXObject *, FXSelector, void*);
        long onKeyPress(FXObject *, FXSelector, void*);
        long onPipeTimeout(FXObject *, FXSelector, void*);
        long onCloseTimeout(FXObject *, FXSelector, void*);
        long onTextLink(FXObject *, FXSelector, void *);
        long onPipe(FXObject *, FXSelector, void*);
        long onSpellLang(FXObject *, FXSelector, void*);
        long onSocketCanRead(FXObject*, FXSelector, void*);
        long onSocketConnected(FXObject*, FXSelector, void*);
        long onSocketDisconnected(FXObject*, FXSelector, void*);
        long onSocketError(FXObject*, FXSelector, void*);
        long onSocketStartAccept(FXObject*, FXSelector, void*);
        long onSocketListen(FXObject*, FXSelector, void*);
        long onIrcEvent(FXObject*, FXSelector, void*);

    protected:        
        void appendText(FXString msg, FXbool highlight, FXbool logLine=TRUE);
        void appendStyledText(FXString text, FXint style, FXbool highlight, FXbool disableStrip=FALSE, FXbool logLine=TRUE);
        FXbool processLine(const FXString &);
        void hasAllCommand(FXbool);
        void hasMyMsg(FXbool);

    private:
        DccTabItem(){}

        dxTabBook *m_parent;
        dxSocket *m_server;
        dxPipe *m_pipe;
        FXVerticalFrame *m_textframe, *m_mainframe;
        FXHorizontalFrame *m_commandframe;
        dxText *m_text;
        dxTextField *m_commandline;
        FXint m_currentPosition, m_historyMax, m_dccTimeout;
        FXbool m_logging, m_sameCmd, m_coloredNick, m_sendPipe, m_listen;
        FXbool m_scriptHasAll, m_scriptHasMyMsg, m_stripColors, m_useSpell, m_showSpellCombo;
        dxStringArray m_commandsHistory, m_pipeStrings;
        dxHiliteArray m_textStyleList;
        IrcColor m_colors;
        FXString m_logPath;
        std::ofstream *m_logstream;
        FXComboBox *m_spellLangs;
        FXColor m_unreadColor, m_highlightColor;
        FXString m_mynick, m_nick, m_address, m_receiveRest, m_dccIP;
        FXint m_portD, m_portH;
        dxSocket *m_socket;
        IrcEngine *m_engine;
        
        FXString stripColors(const FXString &text, const FXbool stripOther);
        void appendIrcText(FXString msg, FXTime time, FXbool disableStrip=FALSE, FXbool logLine=TRUE);
        void appendIrcStyledText(FXString styled, FXint stylenum, FXTime time, FXbool disableStrip=FALSE, FXbool logLine=TRUE);
        void appendIrcNickText(FXString nick, FXString msg, FXint style, FXTime time, FXbool logLine=TRUE);
        void appendLinkText(const FXString &, FXint);
        void startLogging();
        void stopLogging();
        void logLine(const FXString &line, const FXTime &time);
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
        FXbool sendLine(const FXString &line);
        void writeIrcEvent(IrcEvent event);
};

#endif // DCCCTABITEM_H
