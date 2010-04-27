/*
 *      dxtext.h
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

#ifndef DXTEXT_H
#define	DXTEXT_H

#ifndef FXSCROLLAREA_H
#include "FXScrollArea.h"
#endif

#ifndef FXARRAY_H
#include "FXArray.h"
#endif

/// Highlight style entry
struct dxHiliteStyle {
  FXColor normalForeColor;            /// Normal text foreground color
  FXColor normalBackColor;            /// Normal text background color
  FXColor selectForeColor;            /// Selected text foreground color
  FXColor selectBackColor;            /// Selected text background color
  FXuint  style;                      /// Highlight text style
  FXbool  link;                       /// Link text for launch
};

// Smiley entry
struct dxSmiley {
    FXString text;                  //smiley text, e.g. :) etc.
    FXString path;                  //path to smiley image, needed for resize
    FXIcon *icon;                   //smiley icon
};

enum {
    SEL_TEXTLINK = SEL_LAST ///Link text clicked
};

typedef FXArray<FXString>  FXStringBuffer;
typedef FXArray<dxSmiley> dxSmileyArray;
typedef FXArray<FXint> dxFXintArray;

class dxText : public FXScrollArea
{
    FXDECLARE(dxText)
public:
    dxText(FXComposite *p,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=3,FXint pr=3,FXint pt=2,FXint pb=2);
    virtual ~dxText();
    enum {
    STYLE_UNDERLINE = 0x0001,   /// Underline text
    STYLE_STRIKEOUT = 0x0002,   /// Strike out text
    STYLE_BOLD      = 0x0004    /// Bold text
    };
    enum {
        ID_COPY_SEL = FXScrollArea::ID_LAST,
        ID_SELECT_ALL,
        ID_DESELECT_ALL
    };

    virtual void create();
    virtual void detach();
    virtual void layout();
    virtual void recalc();
    virtual bool canFocus() const;
    void setTextColor(FXColor clr);
    FXColor getTextColor() const { return textColor; }
    void setSelBackColor(FXColor clr);
    FXColor getSelBackColor() const { return selbackColor; }
    void setSelTextColor(FXColor clr);
    FXColor getSelTextColor() const { return seltextColor; }
    void setHiliteStyles(const dxHiliteStyle* styles);
    const dxHiliteStyle* getHiliteStyles() const { return hilitestyles; }
    void setFont(FXFont* fnt);
    FXFont* getFont() const { return font; }
    FXbool getUseSmiley() const { return usesmiley; }
    void setSmileys(FXbool smiley, dxSmileyArray nsmileys);
    void removeSmileys();
    virtual void appendText(const FXString &text);
    virtual void appendStyledText(FXString text, FXint style=0);
    virtual void clearText();
    virtual FXint getDefaultWidth();
    virtual FXint getDefaultHeight();
    virtual FXint getContentWidth();
    virtual FXint getContentHeight();
    void makeLastRowVisible(FXbool force=FALSE);
    void extractText(FXString& text,FXint startindex,FXint startpos, FXint endindex, FXint endpos);

    long onPaint(FXObject*, FXSelector, void*);
    long onMotion(FXObject*, FXSelector, void*);
    long onLeftBtnPress(FXObject*,FXSelector,void*);
    long onLeftBtnRelease(FXObject*,FXSelector,void*);
    long onFocusIn(FXObject*,FXSelector,void*);
    long onFocusOut(FXObject*,FXSelector,void*);
    long onUngrabbed(FXObject*,FXSelector,void*);
    long onSelectionLost(FXObject*,FXSelector,void*);
    long onSelectionGained(FXObject*,FXSelector,void*);
    long onSelectionRequest(FXObject*,FXSelector,void*);
    long onClipboardLost(FXObject*,FXSelector,void*);
    long onClipboardGained(FXObject*,FXSelector,void*);
    long onClipboardRequest(FXObject*,FXSelector,void*);
    long onKeyPress(FXObject*,FXSelector,void*);
    long onKeyRelease(FXObject*,FXSelector,void*);
    long onCmdCopySel(FXObject*,FXSelector,void*);
    long onCmdSelectAll(FXObject*,FXSelector,void*);
    long onCmdDeselectAll(FXObject*,FXSelector,void*);
protected:
    dxText();

    enum {
    STYLE_MASK      = 0x00FF,   // Mask color table
    STYLE_TEXT      = 0x0100,   // Draw some content
    STYLE_SELECTED  = 0x0200,   // Selected
    STYLE_CONTROL   = 0x0400,   // Control character
    STYLE_SMILEY    = 0x0800,   // Smiley icon
    };

    enum {
    MOUSE_NONE,                 // No mouse operation
    MOUSE_CHARS,                // Selecting characters
    MOUSE_WORDS                 // Selecting words
    };

    FXStringBuffer contents;                //Text data
    FXStringBuffer styles;                  //Text style
    dxFXintArray lines;                     //Real wraped lines for contents; index same as in contents
    const dxHiliteStyle *hilitestyles;      // Style definitions
    FXFont *font;                           //Text font
    FXint margintop;                        // Margins top
    FXint marginbottom;                     // Margin bottom
    FXint marginleft;                       // Margin left
    FXint marginright;                      // Margin right
    FXColor textColor;                      // Normal text color
    FXColor selbackColor;                   // Select background color
    FXColor seltextColor;                   // Select text color
    FXint wrapwidth;                        // Wrap width in pixels
    FXint nlines;                           // Real number lines
    FXint topline;                          // Where first line is in contents
    FXint vrows;                            // Default visible rows
    FXint vcols;                            // Default visible columns
    dxSmileyArray smileys;                  // smileys
    FXbool usesmiley;                       // dxText use smiley
    FXint cursorindex;                      // Cursor index in contents
    FXint cursorpos;                        // Cursor position
    FXint selstartindex;                    // Start selection index in contents
    FXint selstartpos;                      // Start selection position
    FXint selendindex;                      // End selection index in contents
    FXint selendpos;                        // End selection position
    FXuchar mode;                           // Mode widget is in
    FXString clipped;                       // Clipped text

    virtual void fillBufferRect(FXDCWindow& dc,FXint x,FXint y,FXint w,FXint h,FXuint style) const;
    virtual void drawTextFragment(FXDCWindow& dc, FXint x, FXint y, FXint w, FXint h, const FXchar *text, FXint n, FXuint style) const;
    virtual void drawTextLine(FXDCWindow& dc, FXint line, FXint left, FXint right) const;
    virtual void drawContents(FXDCWindow& dc, FXint x, FXint y, FXint w, FXint h) const;
    virtual void drawIcon(FXDCWindow& dc, FXint x, FXint y, FXint index) const;
    FXint charWidth(FXwchar ch, FXint col) const;
    FXint smileyWidth(FXuint style) const;
    FXuint styleOf(FXint line, FXint pos) const;
    FXuint styleOfXY(FXint x, FXint y) const;
    virtual void moveContents(FXint x,FXint y);
    FXint getIndexOfY(FXint y) const;
    FXint getPosOfXY(FXint x, FXint y) const;
    void recompute();
    FXint lineCount(FXint line) const;
    void createSmiley(FXString *text, FXString *style);
    void removeSmiley(FXString *text, FXString *style);
    FXString clearSmiley(FXint index, FXint pos, FXint len);
    void wrapIndex(FXint index, FXint wrap, FXint &wrapstart, FXint &wrapend) const;
    void setSelection(FXint index, FXint pos);
    void setSelection(FXint startindex, FXint startpos, FXint endindex, FXint endpos);
    void killSelection();
    void updateRange(FXint beg,FXint end) const;
    void setCursor(FXuint index);
private:
    dxText(const dxText&);
    dxText& operator=(const dxText&);
};

#endif	/* DXTEXT_H */

