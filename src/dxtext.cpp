/*
 *      dxtext.cpp
 *
 *      Copyright (C) 1998,2006 by Jeroen van der Zijp.   All Rights Reserved.
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

#include <new>
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "fxkeys.h"
#include "fxascii.h"
#include "fxunicode.h"
#include "FXHash.h"
#include "FXThread.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXRex.h"
#include "FXSize.h"
#include "FXPoint.h"
#include "FXRectangle.h"
#include "FXObject.h"
#include "FXRegistry.h"
#include "FXAccelTable.h"
#include "FXApp.h"
#include "FXDCWindow.h"
#include "FXFont.h"
#include "FXPNGIcon.h"
#include "FXScrollBar.h"
#include "FXIconSource.h"
#include "FX88591Codec.h"
#include "FXUTF16Codec.h"
#include "dxtext.h"

// Absolute value
static inline FXint fxabs(FXint a){ return a<0?-a:a; }

// checks is char word delimiter
// ~.,/\\`'!@#$%^&*()-=+{}|[]\":;<>?
static FXbool isDelimiter(FXchar c)
{
    switch(c) {
        case ' ':
        case '~':
        case '.':
        case ',':
        case '/':
        case '\\':
        case '`':
        case '\'':
        case '!':
        case '@':
        case '#':
        case '$':
        case '%':
        case '^':
        case '&':
        case '*':
        case '(':
        case ')':
        case '-':
        case '=':
        case '+':
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

FXDEFMAP(dxText) dxTextMap[] = {
    FXMAPFUNC(SEL_PAINT, 0, dxText::onPaint),
    FXMAPFUNC(SEL_MOTION, 0, dxText::onMotion),
    FXMAPFUNC(SEL_LEFTBUTTONPRESS, 0, dxText::onLeftBtnPress),
    FXMAPFUNC(SEL_LEFTBUTTONRELEASE, 0, dxText::onLeftBtnRelease),
    FXMAPFUNC(SEL_FOCUSIN, 0, dxText::onFocusIn),
    FXMAPFUNC(SEL_FOCUSOUT, 0, dxText::onFocusOut),
    FXMAPFUNC(SEL_UNGRABBED, 0, dxText::onUngrabbed),
    FXMAPFUNC(SEL_SELECTION_LOST, 0, dxText::onSelectionLost),
    FXMAPFUNC(SEL_SELECTION_GAINED, 0, dxText::onSelectionGained),
    FXMAPFUNC(SEL_SELECTION_REQUEST, 0, dxText::onSelectionRequest),
    FXMAPFUNC(SEL_CLIPBOARD_LOST, 0, dxText::onClipboardLost),
    FXMAPFUNC(SEL_CLIPBOARD_GAINED, 0, dxText::onClipboardGained),
    FXMAPFUNC(SEL_CLIPBOARD_REQUEST, 0, dxText::onClipboardRequest),
    FXMAPFUNC(SEL_KEYPRESS, 0, dxText::onKeyPress),
    FXMAPFUNC(SEL_KEYRELEASE, 0, dxText::onKeyRelease),
    FXMAPFUNC(SEL_COMMAND, dxText::ID_COPY_SEL, dxText::onCmdCopySel),
    FXMAPFUNC(SEL_COMMAND, dxText::ID_SELECT_ALL, dxText::onCmdSelectAll),
    FXMAPFUNC(SEL_COMMAND, dxText::ID_DESELECT_ALL, dxText::onCmdDeselectAll),
};

FXIMPLEMENT(dxText, FXScrollArea, dxTextMap, ARRAYNUMBER(dxTextMap))

// Deserialization
dxText::dxText()
{
    flags|=FLAG_ENABLED;
    font = NULL;
    hilitestyles = NULL;
    margintop = 0;
    marginbottom = 0;
    marginleft = 0;
    marginright = 0;
    wrapwidth = 80;
    nlines = 0;
    topline = 0;
    vrows = 0;
    vcols = 0;
    usesmiley = FALSE;
    cursorindex = 0;
    cursorpos = 0;
    selstartindex = 0;
    selstartpos = 0;
    selendindex = 0;
    selendpos = 0;
    mode = MOUSE_NONE;
}

// Construct and init
dxText::dxText(FXComposite *p,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb)
    : FXScrollArea(p, opts, x, y, w, h)
{
    flags|=FLAG_ENABLED;
    defaultCursor = getApp()->getDefaultCursor(DEF_TEXT_CURSOR);
    font = getApp()->getNormalFont();
    target = tgt;
    message = sel;
    hilitestyles = NULL;
    margintop = pt;
    marginbottom = pb;
    marginleft = pl;
    marginright = pr;
    wrapwidth = 80;
    nlines = 0;
    topline = 0;
    vrows = 0;
    vcols = 0;
    usesmiley = FALSE;
    cursorindex = 0;
    cursorpos = 0;
    selstartindex = 0;
    selstartpos = 0;
    selendindex = 0;
    selendpos = 0;
    textColor=getApp()->getForeColor();
    selbackColor=getApp()->getSelbackColor();
    seltextColor=getApp()->getSelforeColor();
    mode = MOUSE_NONE;
    contents.no(0);
    styles.no(0);
    smileys.no(0);
    lines.no(0);
}

//Destructor
dxText::~dxText()
{
    font=(FXFont*)-1L;
    hilitestyles=(dxHiliteStyle*)-1L;
}

//Create window
void dxText::create()
{
    FXScrollArea::create();
    font->create();
    recalc();
}

//Detach window
void dxText::detach()
{
    FXScrollArea::detach();
    font->detach();
}

// Recalculate layout
void dxText::layout()
{
    FXint fh = font->getFontHeight();
    FXint fw=font->getFontWidth();
    FXint oww=wrapwidth;
    // Compute new wrap width; needed to reflow text
    wrapwidth = width - marginleft - marginright - vertical->getDefaultWidth();
    // Wrap width changed, so reflow
    if(wrapwidth!=oww && wrapwidth/fw!=oww/fw) flags|=FLAG_RECALC;
    // Scrollbars adjusted
    FXScrollArea::layout();
    // Set line size based on font
    vertical->setLine(fh);
    // Force repaint
    update();
    // No more dirty
    flags &= ~FLAG_DIRTY;
}

// Propagate size change
void dxText::recalc()
{
    FXScrollArea::recalc();
    flags|=FLAG_RECALC;
}

// If window can have focus
bool dxText::canFocus() const
{
    return true;
}

// Set text color
void dxText::setTextColor(FXColor clr)
{
    if(textColor!=clr)
    {
        textColor = clr;
        update(0, 0, width, height);
    }
}

// Set select background color
void dxText::setSelBackColor(FXColor clr)
{
    if(clr != selbackColor)
    {
        selbackColor = clr;
        //updateRange(selstartpos, selendpos);
        update(0, 0, width, height);
    }
}

// Set selected text color
void dxText::setSelTextColor(FXColor clr)
{
    if (clr != seltextColor)
    {
        seltextColor = clr;
        //updateRange(selstartpos, selendpos);
        update(0, 0, width, height);
    }
}

// Set highlight styles
void dxText::setHiliteStyles(const dxHiliteStyle* styles)
{
    hilitestyles=styles;
    update();
}

//Change font
void dxText::setFont(FXFont* fnt)
{
    if(!fnt){ fxerror("%s::setFont: NULL font specified.\n",getClassName()); }
    if(font!=fnt)
    {
        font = fnt;
        recalc();
        update();
    }
}

//Change smileys
void dxText::setSmileys(FXbool smiley, dxSmileyArray nsmileys)
{
    if(smileys.no()) removeSmileys();
    usesmiley=smiley;
    smileys=nsmileys;
    if(usesmiley && smileys.no())
    {
        for(FXint i=0; i<contents.no(); i++)
        {
            createSmiley(&contents[i], &styles[i]);
        }
    }
    recompute();
    layout();
}

//remove smileys from text back to chars
void dxText::removeSmileys()
{
    for(FXint i=0; i<contents.no(); i++)
    {
        removeSmiley(&contents[i], &styles[i]);
    }
}

void dxText::appendText(const FXString& text)
{
    appendStyledText(text, 0);
}

void dxText::appendStyledText(FXString text, FXint style)
{
    register FXint last = 0;
    register FXint len = 0;
    if((text.contains('\n') && text[text.length()-1]!='\n') || text.contains('\n')>1)
    {
        if(text[text.length()-1]=='\n') last=1;
        len = text.contains('\n');
        for(FXint i=0; i<=len; i++)
        {
            if(i==len)
            {
                if(last) appendStyledText(text.section('\n',i)+"\n", style);
                else appendStyledText(text.section('\n',i), style);
            }
            else appendStyledText(text.section('\n',i)+"\n", style);
        }
        return;
    }
    FXString stylestr = "";
    for(FXint i=0; i<text.length(); i++)
    {
        stylestr.append(style);
    }
    if(usesmiley && smileys.no())
        createSmiley(&text, &stylestr);
    if(contents.no())
    {
        last = contents.no()-1;
        len = contents[last].length()-1;
        if(contents[last][len] == '\n')
        {
            contents.append(text);
            styles.append(stylestr);
        }
        else
        {
            contents[last].append(text);
            styles[last].append(stylestr);
        }
    }
    else
    {
        contents.append(text);
        styles.append(stylestr);
    }
    last = contents.no()-1;
    len = contents[last].length()-1;
    if(contents[last][len] == '\n')
    {
        lines.append(lineCount(last));
        nlines+=lines[last];
        FXScrollArea::layout();
        makeLastRowVisible();
        updateRange(last, last);
    }
}

void dxText::removeLine(FXint line)
{
    if(line<0 || line>=contents.no()) return;
    contents.erase(line);
    styles.erase(line);
    nlines-=lines[line];
    lines.erase(line);
    updateRange(FXMAX(0,line-1), FXMIN(contents.no()-1,line+1));
}

void dxText::removeLastLine()
{
    removeLine(contents.no()-1);
}

//Replace smiley in text by its code
void dxText::createSmiley(FXString* text, FXString* style)
{
    //length both strings have to be same
    if(text->length()!=style->length()) return;
    for(FXint i=0; i<smileys.no(); i++)
    {
        if(text->contains(smileys[i].text))
        {
            FXint pos = 0;
            FXint dist = 0;
            for(FXint j=1;;j++)
            {
                pos = text->find(smileys[i].text, pos);
                if(pos==-1) break;
                else
                {
                    style->replace(pos-dist, smileys[i].text.length(), i, 1);
                    dist = j*(smileys[i].text.length()-1);
                }
                pos++;
            }
            text->substitute(smileys[i].text, "\023");
        }
    }
}

//Smiley back to plain text
void dxText::removeSmiley(FXString* text, FXString* style)
{
    //length both strings have to be same
    if(text->length()!=style->length()) return;
    if(text->contains('\023'))
    {
        FXint pos=0;
        FXuint s=0;
        FXuint prev=0;
        FXuint next=0;
        FXuint stylenum;
        while(1)
        {
            pos=text->find('\023', pos);
            if(pos==-1) break;
            else
            {
                s|=style->at(pos);
                if(pos-1<0) prev=0;
                else prev|=style->at(pos-1);
                if(pos+1>=style->length()) next=0;
                else next|=style->at(pos+1);
                if(prev!=next) stylenum=next;
                else stylenum=prev;
                if(smileys.no() && s<(FXuint)smileys.no())
                {
                    text->replace(pos, 1, smileys[s].text);
                    style->replace(pos, 1, stylenum, smileys[s].text.length());
                }
                else
                {
                    text->replace(pos, ' ');
                    style->replace(pos, 0);
                }
            }
            s=prev=next=0;
            pos++;
        }
    }
}

//Smiley back to plain text; usefull for copy
FXString dxText::clearSmiley(FXint index, FXint pos, FXint len)
{
    if(index<0 || index>contents.no()-1) return "";
    if(pos<0) return "";
    FXString text = contents[index].mid(pos,len);
    FXString style = styles[index].mid(pos,len);
    if(text.contains('\023'))
    {
        FXint spos=0;
        FXuint s=0;
        FXuint prev=0;
        FXuint next=0;
        FXuint stylenum;
        while(1)
        {
            spos=text.find('\023', spos);
            if(spos==-1) break;
            else
            {
                s|=style.at(spos);
                if(spos-1<0) prev=0;
                else prev|=style.at(spos-1);
                if(spos+1>=style.length()) next=0;
                else next|=style.at(spos+1);
                if(prev!=next) stylenum=next;
                else stylenum=prev;
                if(smileys.no() && s<(FXuint)smileys.no())
                {
                    text.replace(spos, 1, smileys[s].text);
                    style.replace(spos, 1, stylenum, smileys[s].text.length());
                }
                else
                {
                    text.replace(spos, ' ');
                    style.replace(spos, 0);
                }
            }
            s=prev=next=0;
            spos++;
        }
    }
    return text;
}

// Grab range of text
void dxText::extractText(FXString& text, FXint startindex, FXint startpos, FXint endindex, FXint endpos)
{
    text.clear();
    if(startindex==endindex)
    {
        text = clearSmiley(startindex, startpos, endpos-startpos+1);
        return;
    }
    else
    {
        text += clearSmiley(startindex, startpos, contents[startindex].length()-startpos);
        for(FXint i=startindex+1; i<endindex; i++)
            text += clearSmiley(i, 0, contents[i].length());
        text += clearSmiley(endindex, 0, endpos+1);
    }
}

//return position of wrap
void dxText::wrapIndex(FXint index, FXint wrap, FXint &wrapstart, FXint &wrapend) const
{
    if(index<=0 && index>contents.no()-1) return;
    if(lines[index]==1)
    {
        wrapstart=0;
        wrapend=contents[index].length()-1;
        return;
    }
    wrapstart=0;
    wrapend=0;
    register FXint lw,cw;
    register FXint sp, sc;
    register FXint s=-1;
    register FXuint style;
    register FXint nwrap=0;
    lw=0;
    for (sp = sc = 0; sp < contents[index].length(); sp = contents[index].inc(sp), sc++)
    {
        if(contents[index].wc(sp)=='\023')
        {
            style = styleOf(index, sp);
            cw = smileyWidth(style&=~STYLE_SMILEY);
        }
        else cw = charWidth(contents[index].wc(sp), sc);
        if(contents[index][sc] == ' ') s=sc;
        if (lw+cw > wrapwidth)
        {
            nwrap++;
            wrapstart=wrapend;
            if(s!=-1)
            {
                wrapend=s+1;
                sp=sc=s+1;
                s=-1;
            }
            else
            {
                wrapend=sc;
            }
            lw=0;
            if(nwrap==wrap) return;
        }
        lw+=cw;
    }
    if(nwrap==wrap-1)
    {
        wrapstart=wrapend;
        wrapend=contents[index].length()-1;
        return;
    }
}

//Clear all dxText
void dxText::clearText()
{
    if(contents.no())
    {
        contents.clear();
        styles.clear();
        lines.clear();
        nlines=0;
        cursorindex = 0;
        cursorpos = 0;
        selstartindex = 0;
        selstartpos = 0;
        selendindex = 0;
        selendpos = 0;
        layout();
    }
}

// Extend selection
void dxText::setSelection(FXint index, FXint pos)
{
    register FXint oldstartindex = selstartindex;
    register FXint oldendindex = selendindex;
    register FXint oldstartpos = selstartpos;
    if(index<cursorindex)
    {
        selendindex = cursorindex;
        selstartindex = index;
        selendpos = cursorpos;
        selstartpos = pos;
    }
    else
    {
        selstartindex = cursorindex;
        selendindex = index;
        if(cursorindex==selendindex && pos<cursorpos)
        {
            selendpos = cursorpos;
            selstartpos = pos;
        }
        else
        {
            selstartpos = cursorpos;
            selendpos = pos;
        }
    }
    if(oldstartindex==selstartindex && oldstartpos==selstartpos)
    {
        if(selstartindex==selendindex && selstartindex==oldendindex)
            updateRange(selstartindex, selendindex);
        else
            updateRange(oldendindex, selendindex);
    }
    else
    {
        updateRange(oldstartindex, selstartindex);
        updateRange(oldendindex, selendindex);
    }
}

// Extend selection
void dxText::setSelection(FXint startindex, FXint startpos, FXint endindex, FXint endpos)
{
    if(startindex>endindex) return;
    if(startindex==endindex && startpos>endpos) return;
    register FXint oldstartindex = selstartindex;
    register FXint oldendindex = selendindex;
    selstartindex = startindex;
    selstartpos = startpos;
    selendindex = endindex;
    selendpos = endpos;
    updateRange(FXMIN(oldstartindex,selstartindex),FXMAX(oldendindex,selendindex));
}

// Kill the selection
void dxText::killSelection()
{
    if(hasSelection()) releaseSelection();
    updateRange(selstartindex, selendindex);
    selstartindex = 0;
    selstartpos = 0;
    selendindex = 0;
    selendpos = 0;
}

//Check index and pos in selection range
FXbool dxText::isSelection(FXint index, FXint pos) const
{
    if(!selstartindex&&!selendindex&&!selstartpos&&!selendpos) return FALSE;
    if(selstartindex==index)
    {
        if(selendindex>index && pos>=selstartpos) return TRUE;
        if(index==selendindex && selstartpos<=pos && pos<=selendpos) return TRUE;
    }
    if(selstartindex<index)
    {
        if(selendindex>index) return TRUE;
        if(index==selendindex && pos<=selendpos) return TRUE;
    }
    return FALSE;
}

// Repaint text range; beg and end are index
void dxText::updateRange(FXint beg, FXint end) const
{
    if(beg>=lines.no()) return;
    if(end>=lines.no()) return;
    register FXint y, i, t;
    if(beg>end){t=beg;beg=end;end=t;}
    if(beg<0) return;
    register FXint hh=font->getFontHeight();
    register FXint h=0;
    y = pos_y+margintop;
    for(i=0; i<beg; i++)
    {
        y+=lines[i]*hh;
    }
    for(i=beg; i<=end; i++)
    {
        h+=lines[i]*hh;
    }
    update(pos_x+marginleft, y, wrapwidth, h);
}

//set cursor type, index is of hilitestyle
void dxText::setCursorType(FXuint index)
{
    if(hilitestyles && index&STYLE_MASK)
    {
        if(index&STYLE_SMILEY) setDefaultCursor(getApp()->getDefaultCursor(DEF_TEXT_CURSOR));
        else
        {
            if(hilitestyles[(index&STYLE_MASK)-1].link) setDefaultCursor(getApp()->getDefaultCursor(DEF_HAND_CURSOR));
            else setDefaultCursor(getApp()->getDefaultCursor(DEF_TEXT_CURSOR));
        }
    }
    else setDefaultCursor(getApp()->getDefaultCursor(DEF_TEXT_CURSOR));
}

//wordstart for position
FXint dxText::wordStart(FXint index, FXint pos)
{
    if(index<0 && index>contents.no()-1) return 0;
    if(pos<=0) return 0;
    if(pos>=contents[index].length()) pos=contents[index].length()-1;
    if(isDelimiter(contents[index][pos])) return pos;
    while(0<pos)
    {
        if(isDelimiter(contents[index][pos-1])) return pos;
        pos--;
    }
    return 0;
}

//wordend for position
FXint dxText::wordEnd(FXint index, FXint pos)
{
    register FXint length = contents[index].length();
    if(index<0 && index>contents.no()-1) return 0;
    if(pos<=0) return 0;
    if(pos>=length) pos=length-1;
    if(isDelimiter(contents[index][pos])) return pos;
    while(pos<length-1)
    {
        if(isDelimiter(contents[index][pos+1])) return pos;
        pos++;
    }
    return length-1;
}

// Get default width
FXint dxText::getDefaultWidth()
{
    return 0 < vcols ? marginleft + marginright + vcols * font->getTextWidth("8", 1) : FXScrollArea::getDefaultWidth();
}

// Get default height
FXint dxText::getDefaultHeight()
{
    return 0 < vrows ? margintop + marginbottom + vrows * font->getFontHeight() : FXScrollArea::getDefaultHeight();
}

// Determine minimum content width of scroll area
FXint dxText::getContentWidth()
{
    if(flags&FLAG_RECALC) recompute();
    return 1;
}

// Determine minimum content height of scroll area
FXint dxText::getContentHeight()
{
    if(flags&FLAG_RECALC) recompute();
    return margintop + marginbottom + font->getFontHeight()*nlines;
}

// Fill fragment of background in given style
void dxText::fillBufferRect(FXDCWindow& dc, FXint x, FXint y, FXint w, FXint h, FXuint style) const
{
    register FXuint index = (style & STYLE_MASK);
    register FXuint usedstyle = style; // Style flags from style buffer
    register FXColor bgcolor, fgcolor;
    register FXbool link = FALSE;
    bgcolor = fgcolor = 0;
    if (hilitestyles && index) { // Get colors from style table
        usedstyle = hilitestyles[index - 1].style; // Style flags now from style table
        link = hilitestyles[index - 1].link; //link text now from style table
        if (style & STYLE_SELECTED) {
            bgcolor = hilitestyles[index - 1].selectBackColor;
            fgcolor = hilitestyles[index - 1].selectForeColor;
        } else {
            bgcolor = hilitestyles[index - 1].normalBackColor;
        }
        if (fgcolor == 0) { // Fall back to normal foreground color
            fgcolor = hilitestyles[index - 1].normalForeColor;
        }
    }
    if (bgcolor == 0) { // Fall back to default background colors
        if (style & STYLE_SELECTED) bgcolor = selbackColor;
        else bgcolor = backColor;
    }
    if (fgcolor == 0) { // Fall back to default foreground colors
        if (style & STYLE_SELECTED) fgcolor = seltextColor;
        if (fgcolor == 0) fgcolor = textColor; // Fall back to text color
    }
    dc.setForeground(bgcolor);
    dc.fillRectangle(x, y, w, h);
    if (usedstyle & STYLE_UNDERLINE || link) {
        dc.setForeground(fgcolor);
        dc.fillRectangle(x, y + font->getFontAscent() + 1, w, 1);
    }
    if (usedstyle & STYLE_STRIKEOUT) {
        dc.setForeground(fgcolor);
        dc.fillRectangle(x, y + font->getFontAscent() / 2, w, 1);
    }
}

//Draw smiley icon
void dxText::drawIcon(FXDCWindow& dc, FXint x, FXint y, FXint index) const
{
    for(FXint i=0; i<smileys.no(); i++)
    {
        if(i==index && smileys[i].icon) dc.drawIcon(smileys[i].icon, x, y);
    }
}

// Draw fragment of text in given style
void dxText::drawTextFragment(FXDCWindow& dc, FXint x, FXint y, FXint /*w*/, FXint /*h*/, const FXchar* text, FXint n, FXuint style) const
{
    register FXuint index=style&STYLE_MASK;
    register FXuint usedstyle=style;
    if(hilitestyles && index)
    {
        if(style&STYLE_SELECTED) dc.setForeground(hilitestyles[index-1].selectForeColor);
        else dc.setForeground(hilitestyles[index-1].normalForeColor);
        usedstyle=hilitestyles[index-1].style;
    }
    else
    {
        if(style&STYLE_SELECTED) dc.setForeground(seltextColor);
        else dc.setForeground(textColor);
    }
    y+=font->getFontAscent();
    dc.drawText(x,y,text,n);
    if(usedstyle&STYLE_BOLD) dc.drawText(x+1,y,text,n);
}

// Draw text line with correct style
void dxText::drawTextLine(FXDCWindow& dc, FXint line, FXint left, FXint right) const
{
    //register FXint index = (topline + line) % contents.no();
    register FXint index;
    register FXint i;
    register FXint n=0;
    register FXuint style;
    register FXint wrap;
    for(index=0; index<contents.no(); index++)
    {
        for(i=0, wrap=0; i<lines[index]&&n<=line; i++)
        {
            n++;
            wrap++;
        }
        if(n>line) break;
    }
    register FXint edge = pos_x + marginleft;
    register FXint h = font->getFontHeight();
    register FXint y = pos_y + margintop + line*h;
    register FXint x = 0;
    register FXint w = 0;
    register FXuint curstyle;
    register FXuint newstyle;
    register FXint numberSmiley = 0; //smiley number for one drawIcon
    register FXint cw, sp, ep, sc, ec;
    FXint wrapstart,wrapend;
    wrapIndex(index, wrap, wrapstart, wrapend);

    // Scan ahead till until we hit the end or the left edge
    //for (sp = sc = 0; sp < contents[index].length(); sp = contents[index].inc(sp), sc++)
    for (sp = sc = wrapstart; sp < wrapend; sp = contents[index].inc(sp), sc++)
    {
        if(contents[index].wc(sp)=='\023')
        {
            style = styleOf(index, sp);
            cw = smileyWidth(style&=~STYLE_SMILEY);
            //smiley have to draw
            if(x+edge<=left && x+edge+cw>=left) break;
        }
        else cw = charWidth(contents[index].wc(sp), sc);
        if (x + edge + cw >= left) break;
        x += cw;
    }
    // First style to display
    curstyle = styleOf(index, sp);
    // Draw until we hit the end or the right edge
    //for (ep = sp, ec = sc; ep < contents[index].length(); ep = contents[index].inc(ep), ec++)
    for (ep = sp, ec = sc; ep < wrapend; ep = contents[index].inc(ep), ec++)
    {
        newstyle = styleOf(index, ep);
        if (newstyle != curstyle)
        {
            if(curstyle&STYLE_SMILEY)
            {
                style = curstyle&=~STYLE_SMILEY;
                if(isSelection(index,ep))
                    fillBufferRect(dc,edge+x,y,numberSmiley*smileyWidth(style),h,STYLE_SELECTED);
                for(i=0; i<numberSmiley; i++)
                {
                    drawIcon(dc, edge+x+i*smileyWidth(style), y, style);
                }
            }
            else
            {
                fillBufferRect(dc,edge+x,y,w,h,curstyle);
                if(curstyle&STYLE_TEXT)
                    drawTextFragment(dc, edge + x, y, w, h, &contents[index][sp], ep - sp, curstyle);
            }
            curstyle = newstyle;
            sp = ep;
            x += w;
            w = 0;
            numberSmiley = 0;
        }
        if(newstyle&STYLE_SMILEY)
        {
            cw = smileyWidth(newstyle&=~STYLE_SMILEY);
            numberSmiley++;
        }
        else cw = charWidth(contents[index].wc(ep), ec);
        if (x + edge + w >= right) break;
        w += cw;
    }
    // Draw unfinished fragment
    if(curstyle&STYLE_SMILEY)
    {
        style = curstyle&=~STYLE_SMILEY;
        if(isSelection(index,ep))
            fillBufferRect(dc,edge+x,y,numberSmiley*smileyWidth(style),h,STYLE_SELECTED);
        for(i=0; i<numberSmiley; i++)
        {
            drawIcon(dc, edge+x+i*smileyWidth(style), y, style);
        }
    }
    else
    {
        fillBufferRect(dc,edge+x,y,w,h,curstyle);
        if(curstyle&STYLE_TEXT)
            drawTextFragment(dc, edge + x, y, w, h, &contents[index][sp], ep - sp, curstyle);
    }
    x += w;
    // Fill any left-overs outside of text
//    if (x + edge < right)
//    {
//        curstyle = styleOf(index, ep);
//        fillBufferRect(dc,edge+x,y,right-edge-x,h,curstyle);
//    }
}

// Repaint lines of text
void dxText::drawContents(FXDCWindow& dc, FXint x, FXint y, FXint w, FXint h) const
{
    register FXint hh=font->getFontHeight();
    register FXint yy=pos_y+margintop;
    register FXint tl=(y-yy)/hh;
    register FXint bl=(y+h-yy)/hh;
    register FXint ln;
    if(tl<0) tl=0;
    if(bl>=nlines) bl=nlines-1;
    FXTRACE((1,"tl=%d bl=%d\n",tl,bl));
    for(ln=tl; ln<=bl; ln++)
    {
        drawTextLine(dc,ln,x,x+w);
    }
}

// Character width
FXint dxText::charWidth(FXwchar ch, FXint col) const
{
    if (ch < ' ')
    {
        if (ch != '\t')
        {
            return font->getCharWidth('#');
        }
        return font->getCharWidth(' ')*(8 - col % 8);
    }
    return font->getCharWidth(ch);
}

// Smiley width
// style is style from struct dxSmiley
FXint dxText::smileyWidth(FXuint style) const
{
    if(!usesmiley) return 0;
    if(!smileys.no()) return 0;
    for(FXuint i=0; i<(FXuint)smileys.no(); i++)
    {
        if(i==style) return smileys[i].icon->getWidth();
    }
    return 0;
}

// Determine style
FXuint dxText::styleOf(FXint line, FXint pos) const
{
    register FXuint s=0;
    register FXchar ch;
    // Line after contents
    if(line>contents.no()-1) return s;
    // Blank part of line
    if(pos>=contents[line].length()) return s;
    // Special style for control characters
    ch=contents[line][pos];
    // Style for smiley icon
    if(ch == '\023') return (FXuchar)styles[line][pos]|STYLE_SMILEY;
    // Selected part of text
    if(isSelection(line,pos)) s|=STYLE_SELECTED;
    // Get value from style buffer
    if(styles.no()) s|=(FXuchar)styles[line][pos];
    // Tabs are just fill
    if(ch == '\t') return s;
    // Spaces are just fill
    if(ch == ' ') return s;
    // Newlines are just fill
    if(ch == '\n') return s;
    // Get special style for control codes
    if((FXuchar)ch < ' ') return s|STYLE_CONTROL|STYLE_TEXT;
    
    return s|STYLE_TEXT;
    //return s;
}

//Determine style for x,y
FXuint dxText::styleOfXY(FXint x, FXint y) const
{
    if(!contents.no()) return 0;
    register FXint line=(y-pos_y-margintop)/font->getFontHeight();
    if(line>=nlines) return 0;
    register FXint index, wrap, pos, sp, sc, i;
    register FXint cw=marginleft;
    register FXint n=0;
    register FXuint style;
    for(index=0; index<contents.no(); index++)
    {
        for(i=0, wrap=0; i<lines[index]&&n<=line; i++)
        {
            n++;
            wrap++;
        }
        if(n>line) break;
    }
    FXint wrapstart,wrapend;
    wrapIndex(index, wrap, wrapstart, wrapend);
    for (sp = sc = wrapstart; sp < wrapend; sp = contents[index].inc(sp), sc++)
    {
        if(contents[index].wc(sp)=='\023')
        {
            style = styleOf(index, sp);
            cw += smileyWidth(style&=~STYLE_SMILEY);
        }
        else cw += charWidth(contents[index].wc(sp), sc);
        if (cw >= x)
        {
            pos = sp;
            return styleOf(index, pos);
        }
    }
    return 0;
}

//Line index at y
FXint dxText::getIndexOfY(FXint y) const
{
    if(!contents.no()) return 0;
    register FXint line=(y-pos_y-margintop)/font->getFontHeight();
    if(line>=nlines) return contents.no()-1;
    register FXint index, wrap, i;
    register FXint n=0;
    for(index=0; index<contents.no(); index++)
    {
        for(i=0, wrap=0; i<lines[index]&&n<=line; i++)
        {
            n++;
            wrap++;
        }
        if(n>line) return index;
    }
    return 0;
}

// Localize position at x,y
FXint dxText::getPosOfXY(FXint x, FXint y) const
{
    if(!contents.no()) return 0;
    register FXint line=(y-pos_y-margintop)/font->getFontHeight();
    if(line>=nlines) return contents[contents.no()-1].length()-1;
    register FXint index, wrap, i, sp, sc;
    register FXint n=0;
    register FXint cw=marginleft;
    register FXuint style;
    for(index=0; index<contents.no(); index++)
    {
        for(i=0, wrap=0; i<lines[index]&&n<=line; i++)
        {
            n++;
            wrap++;
        }
        if(n>line) break;
    }
    FXint wrapstart,wrapend;
    wrapIndex(index, wrap, wrapstart, wrapend);
    for (sp = sc = wrapstart; sp < wrapend; sp = contents[index].inc(sp), sc++)
    {
        if(contents[index].wc(sp)=='\023')
        {
            style = styleOf(index, sp);
            cw += smileyWidth(style&=~STYLE_SMILEY);
        }
        else cw += charWidth(contents[index].wc(sp), sc);
        if (cw >= x) return sp;
    }
    return contents[index].length()-1;
}


// Move content
void dxText::moveContents(FXint x, FXint y)
{
    FXint dx = x - pos_x;
    FXint dy = y - pos_y;
    pos_x = x;
    pos_y = y;
    scroll(marginleft, margintop, viewport_w - marginleft - marginright, viewport_h - margintop - marginbottom, dx, dy);
}

// Completely reflow the text, because font, wrapwidth, or all of the
// text may have changed and everything needs to be recomputed
void dxText::recompute()
{
    lines.clear();
    nlines=0;
    for(FXint i=0; i<contents.no(); i++)
    {
        lines.append(lineCount(i));
        nlines+=lines[i];
    }
    makeLastRowVisible(TRUE);
    // Done with that
    flags&=~FLAG_RECALC;
}

//Text wraped line number
FXint dxText::lineCount(FXint line) const
{
    register FXint lw,cw;
    register FXint sp, sc;
    register FXint style=0;
    register FXint nline=1;
    register FXint s=-1;
    lw=0;
    if(line<0 || line>contents.no()-1) return 0;
    for (sp = sc = 0; sp < contents[line].length(); sp = contents[line].inc(sp), sc++)
    {
        if(contents[line].wc(sp)=='\023')
        {
            style = styleOf(line, sp);
            cw = smileyWidth(style&=~STYLE_SMILEY);
        }
        else cw = charWidth(contents[line].wc(sp), sc);
        if(contents[line][sc] == ' ') s=sc;
        if(lw+cw>wrapwidth)
        {
            nline++;
            if(s!=-1)
            {
                sp=sc=s+1;
                s=-1;
            }
            lw=0;
        }
        lw+=cw;
    }
    return nline;
}

//Draw the text
long dxText::onPaint(FXObject*, FXSelector, void *ptr)
{
    FXEvent *event = (FXEvent*)ptr;
    FXDCWindow dc(this, event);
    dc.setFont(font);
    dc.setForeground(backColor);
    dc.fillRectangle(event->rect.x, event->rect.y, event->rect.w, event->rect.h);

    dc.setClipRectangle(marginleft, margintop, viewport_w-marginright-marginleft, viewport_h-margintop-marginbottom);
    drawContents(dc, event->rect.x, event->rect.y, event->rect.w, event->rect.h);
    return 1;
}

// Handle real or simulated mouse motion
long dxText::onMotion(FXObject*,FXSelector,void* ptr)
{
    FXEvent* event=(FXEvent*)ptr;
    FXuint index=styleOfXY(event->win_x, event->win_y);
    setCursorType(index);
    if(mode==MOUSE_CHARS)
    {
        if(startAutoScroll(event,FALSE)) return 1;
        if((fxabs(event->win_x-event->click_x)>getApp()->getDragDelta())||(fxabs(event->win_y-event->click_y)>getApp()->getDragDelta()))
        {
            setSelection(getIndexOfY(event->win_y), getPosOfXY(event->win_x, event->win_y));
        }
    }
    return 1;
}

// Pressed left button
long dxText::onLeftBtnPress(FXObject*, FXSelector, void* ptr)
{
    FXEvent* event = (FXEvent*) ptr;
    FXint line = getIndexOfY(event->win_y);
    FXuint index=styleOfXY(event->win_x, event->win_y)&STYLE_MASK;
    flags &= ~FLAG_TIP;
    handle(this, FXSEL(SEL_FOCUS_SELF, 0), ptr);
    if (isEnabled()) {
        grab();
        if(target && target->tryHandle(this, FXSEL(SEL_LEFTBUTTONPRESS, message), ptr)) return 1;
        if(hilitestyles && index)
        {
            if(hilitestyles[index-1].link)
            {
                FXint start=getPosOfXY(event->win_x, event->win_y);
                FXint length=0;
                FXString link="";
                for(FXint i=start; i>=0; i--)
                {
                    if(!(styleOf(line, i)&STYLE_MASK)) break;
                    if(hilitestyles[(styleOf(line, i)&STYLE_MASK)-1].link) start = i;
                    else break;
                }
                for(FXint i=start; i<contents[line].length(); i++)
                {
                    if(!(styleOf(line, i)&STYLE_MASK))
                    {
                        if(styleOf(line, i)&STYLE_SMILEY)
                        {
                            if(i+1<contents[line].length() && hilitestyles[(styleOf(line, i+1)&STYLE_MASK)-1].link) length++;
                        }
                        else break;
                    }
                    else if(hilitestyles[(styleOf(line, i)&STYLE_MASK)-1].link) length++;
                    else break;
                }
                link = clearSmiley(line, start, length);
                if(target && target->tryHandle(this, FXSEL(SEL_TEXTLINK,message), (void*)link.text())) return 1;
            }
            else
            {
                //select characters
                if(event->click_count==1)
                {
                    if(event->state&SHIFTMASK)
                    {
                        setSelection(line, getPosOfXY(event->win_x, event->win_y));
                    }
                    else
                    {
                        killSelection();
                        cursorindex = line;
                        cursorpos = getPosOfXY(event->win_x, event->win_y);
                    }
                    mode = MOUSE_CHARS;
                }
                //select words
                else if(event->click_count==2)
                {
                    setSelection(line,wordStart(line,getPosOfXY(event->win_x, event->win_y)),line,wordEnd(line,getPosOfXY(event->win_x, event->win_y)));
                    mode = MOUSE_WORDS;
                }
                //select lines
                else
                {
                    setSelection(line,0,line,contents[line].length()-1);
                    mode = MOUSE_LINES;
                }
            }
        }
        else
        {
            //select characters
            if(event->click_count==1)
            {
                if(event->state&SHIFTMASK)
                {
                    setSelection(line, getPosOfXY(event->win_x, event->win_y));
                }
                else
                {
                    killSelection();
                    cursorindex = line;
                    cursorpos = getPosOfXY(event->win_x, event->win_y);
                }
                mode = MOUSE_CHARS;
            }
            //select words
            else if(event->click_count==2)
            {
                setSelection(line,wordStart(line,getPosOfXY(event->win_x, event->win_y)),line,wordEnd(line,getPosOfXY(event->win_x, event->win_y)));
                mode = MOUSE_WORDS;
            }
            //select lines
            else
            {
                setSelection(line,0,line,contents[line].length()-1);
                mode = MOUSE_LINES;
            }
        }
        return 1;
    }
    return 0;
}

// Released left button
long dxText::onLeftBtnRelease(FXObject*, FXSelector, void* ptr)
{
    if(isEnabled())
    {
        ungrab();
        mode = MOUSE_NONE;
        stopAutoScroll();
        if(target && target->tryHandle(this, FXSEL(SEL_LEFTBUTTONRELEASE, message), ptr)) return 1;
        return 1;
    }
    return 0;
}

// Gained focus
long dxText::onFocusIn(FXObject* sender,FXSelector sel,void* ptr)
{
    FXScrollArea::onFocusIn(sender,sel,ptr);
    return 1;
}

// Lost focus
long dxText::onFocusOut(FXObject* sender,FXSelector sel,void* ptr)
{
    FXScrollArea::onFocusOut(sender,sel,ptr);
    return 1;
}

// The widget lost the grab for some reason
long dxText::onUngrabbed(FXObject* sender,FXSelector sel,void* ptr)
{
    FXScrollArea::onUngrabbed(sender,sel,ptr);
    mode=MOUSE_NONE;
    flags|=FLAG_UPDATE;
    stopAutoScroll();
    return 1;
}

// We lost the selection somehow
long dxText::onSelectionLost(FXObject* sender,FXSelector sel,void* ptr)
{
    FXint what[2];
    FXScrollArea::onSelectionLost(sender,sel,ptr);
    if(target)
    {
        what[0]=selstartindex;
        what[1]=selendindex;
        target->tryHandle(this,FXSEL(SEL_DESELECTED,message),(void*)what);
    }
    updateRange(selstartindex,selendindex);
    selstartindex=0;
    selstartpos=0;
    selendindex=0;
    selendpos=0;
    return 1;
}

// We now really do have the selection
long dxText::onSelectionGained(FXObject* sender,FXSelector sel,void* ptr)
{
    FXScrollArea::onSelectionGained(sender,sel,ptr);
    return 1;
}

// Somebody wants our selection
long dxText::onSelectionRequest(FXObject* sender,FXSelector sel,void* ptr)
{
    FXEvent *event=(FXEvent*)ptr;
    // Perhaps the target wants to supply its own data for the selection
    if(FXScrollArea::onSelectionRequest(sender,sel,ptr)) return 1;
    // Recognize the request?
    if(event->target==stringType || event->target==textType || event->target==utf8Type || event->target==utf16Type)
    {
        FXString string;
        // Get selected fragment
        extractText(string,selstartindex,selstartpos,selendindex,selendpos);
        // Return text of the selection as UTF-8
        if(event->target==utf8Type)
        {
            FXTRACE((100,"Request UTF8\n"));
            setDNDData(FROM_SELECTION,event->target,string);
            return 1;
        }
        // Return text of the selection translated to 8859-1
        if(event->target==stringType || event->target==textType)
        {
            FX88591Codec ascii;
            FXTRACE((100,"Request ASCII\n"));
            string=ascii.utf2mb(string);
            setDNDData(FROM_SELECTION,event->target,string);
            return 1;
        }
        // Return text of the selection translated to UTF-16
        if(event->target==utf16Type)
        {
            FXUTF16LECodec unicode;           // FIXME maybe other endianness for unix
            FXTRACE((100,"Request UTF16\n"));
            string=unicode.utf2mb(string);
            setDNDData(FROM_SELECTION,event->target,string);
            return 1;
        }
    }
    return 0;
}

// We lost the selection somehow
long dxText::onClipboardLost(FXObject* sender,FXSelector sel,void* ptr)
{
    FXScrollArea::onClipboardLost(sender,sel,ptr);
    clipped.clear();
    return 1;
}

// We now really do have the selection
long dxText::onClipboardGained(FXObject* sender,FXSelector sel,void* ptr)
{
    FXScrollArea::onClipboardGained(sender,sel,ptr);
    return 1;
}

// Somebody wants our selection
long dxText::onClipboardRequest(FXObject* sender,FXSelector sel,void* ptr)
{
    FXEvent *event=(FXEvent*)ptr;
    // Try handling it in base class first
    if(FXScrollArea::onClipboardRequest(sender,sel,ptr)) return 1;
    // Requested data from clipboard
    if(event->target==stringType || event->target==textType || event->target==utf8Type || event->target==utf16Type)
    {
        FXString string=clipped;
        // Expand newlines to CRLF on Windows
#ifdef WIN32
        unixToDos(string);
#endif
        // Return clipped text as as UTF-8
        if(event->target==utf8Type)
        {
            FXTRACE((100,"Request UTF8\n"));
            setDNDData(FROM_CLIPBOARD,event->target,string);
            return 1;
        }
        // Return clipped text translated to 8859-1
        if(event->target==stringType || event->target==textType)
        {
            FX88591Codec ascii;
            FXTRACE((100,"Request ASCII\n"));
            setDNDData(FROM_CLIPBOARD,event->target,ascii.utf2mb(string));
            return 1;
        }
        // Return text of the selection translated to UTF-16
        if(event->target==utf16Type)
        {
            FXUTF16LECodec unicode;             // FIXME maybe other endianness for unix
            FXTRACE((100,"Request UTF16\n"));
            setDNDData(FROM_CLIPBOARD,event->target,unicode.utf2mb(string));
            return 1;
        }
    }
    return 0;
}

// Keyboard press
long dxText::onKeyPress(FXObject*, FXSelector, void* ptr)
{
    FXEvent* event=(FXEvent*)ptr;
    flags&=~FLAG_TIP;
    if(isEnabled())
    {
        FXTRACE((200,"%s::onKeyPress keysym=0x%04x state=%04x\n",getClassName(),event->code,event->state));
        if(target && target->tryHandle(this,FXSEL(SEL_KEYPRESS,message),ptr)) return 1;
        flags&=~FLAG_UPDATE;
        switch(event->code){
            case KEY_a:
                if(event->state&CONTROLMASK) handle(this,FXSEL(SEL_COMMAND,ID_SELECT_ALL),NULL);
                return 1;
            case KEY_c:
                if(event->state&CONTROLMASK) handle(this,FXSEL(SEL_COMMAND,ID_COPY_SEL),NULL);
                return 1;
            case KEY_F16: //Sun copy key
                handle(this,FXSEL(SEL_COMMAND,ID_COPY_SEL),NULL);
                return 1;
        }
    }
    return 0;
}

// Keyboard release
long dxText::onKeyRelease(FXObject*, FXSelector, void* ptr)
{
    FXEvent* event=(FXEvent*)ptr;
    if(isEnabled())
    {
        FXTRACE((200,"%s::onKeyRelease keysym=0x%04x state=%04x\n",getClassName(),event->code,event->state));
        if(target && target->tryHandle(this,FXSEL(SEL_KEYRELEASE,message),ptr)) return 1;
        return 1;
    }
    return 0;
}

// Copy
long dxText::onCmdCopySel(FXObject*,FXSelector,void*)
{
  if((selstartindex==selendindex && selstartpos<selendpos) || selstartindex<selendindex)
  {
    FXDragType types[4];
    types[0]=stringType;
    types[1]=textType;
    types[2]=utf8Type;
    types[3]=utf16Type;
    if(acquireClipboard(types,4))
    {
      FXASSERT(selstartpos<=selendpos);
      extractText(clipped,selstartindex,selstartpos,selendindex,selendpos);
    }
  }
  return 1;
}

// Select All
long dxText::onCmdSelectAll(FXObject*,FXSelector,void*)
{
    if(contents.no())
    {
        setSelection(0, 0, contents.no()-1, contents[contents.no()-1].length()-1);
    }
    return 1;
}

// Deselect All
long dxText::onCmdDeselectAll(FXObject*, FXSelector, void*)
{
    killSelection();
    return 1;
}

//Show last row, force for show over condition
void dxText::makeLastRowVisible(FXbool force)
{
    register FXfloat pos = vertical->getPosition()*1.0;
    register FXfloat max = (font->getFontHeight()*(nlines-1)-viewport_h)*0.95;
    if(pos>max || force)
    {
        setPosition(0, viewport_h-margintop-marginbottom-font->getFontHeight()*nlines);
        FXint x, y;
        FXuint button;
        getCursorPosition(x, y, button);
        if(button&LEFTBUTTONMASK || button&MIDDLEBUTTONMASK || button&RIGHTBUTTONMASK) return; //we don't need change cursor for button pressed
        if(x>=0 && x<=width && y>=0 && y<=height)
        {
            setCursorType(styleOfXY(x,y));
        }
    }
}


