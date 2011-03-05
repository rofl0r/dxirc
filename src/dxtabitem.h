/*
 *      dxtabitem.h
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


#ifndef DXTABITEM_H
#define DXTABITEM_H

#include "defs.h"
#include "dxtabbook.h"
#include "dxtextfield.h"
#include "fxext.h"
#ifndef DXTEXT_H
#include "dxtext.h"
#endif

class dxirc;

class dxTabItem: public dxEXTabItem
{
    FXDECLARE(dxTabItem)
    friend class dxirc;
    public:
        dxTabItem(dxTabBook *tab, const FXString &tabtext, FXIcon *icon=0, FXuint opts=TAB_TOP_NORMAL, FXint id=0);
        virtual ~dxTabItem();

        virtual void createGeom() {}
        virtual void clearChat() {}
        virtual void hideUsers() {}
        virtual void showUsers() {}
        virtual FXString getServerName() { return ""; }
        virtual FXString getRealServerName() { return ""; }
        virtual FXint getServerPort() { return 0; }
        virtual FXString getNickName() { return "";}
        virtual void setType(const TYPE &typ, const FXString &tabtext) {}
        virtual TYPE getType() { return OTHER; }
        FXint getID() { return m_id; }
        virtual void reparentTab() {}
        virtual void setColor(IrcColor) {}
        virtual void setCommandsList(FXString clst) {}
        virtual void setMaxAway(FXint maxa) {}
        virtual void setLogging(FXbool log) {}
        virtual void setLogPath(FXString pth) {}
        virtual void setNickCompletionChar(FXString nichr) {}
        virtual void setIrcFont(FXFont*) {}
        virtual void setSameCmd(FXbool) {}
        virtual void setSameList(FXbool) {}
        virtual void setColoredNick(FXbool) {}
        virtual void setStripColors(FXbool) {}
        virtual void setCommandFocus() {}
        virtual void setSmileys(FXbool smiley, dxSmileyArray nsmileys) {}
        virtual void setUseSpell(FXbool useSpell) {}
        virtual void setShowSpellCombo(FXbool showSpellCombo) {}
        virtual void removeSmileys() {}
        virtual void makeLastRowVisible() {}
        virtual FXString getSpellLang() {return ""; }
        virtual void setUnreadTabColor(FXColor) {}
        virtual void setHighlightTabColor(FXColor) {}

    protected:
        dxTabItem(){}
        virtual void appendText(FXString msg, FXbool highlight, FXbool logLine=TRUE) {}
        virtual void appendStyledText(FXString text, FXint style, FXbool highlight, FXbool disableStrip=FALSE, FXbool logLine=TRUE) {}
        virtual FXbool processLine(const FXString &) { return FALSE; }
        virtual void hasAllCommand(FXbool) {}
        virtual void hasMyMsg(FXbool) {}

    private:
        FXint m_id;
};

#endif // DXTABITEM_H
