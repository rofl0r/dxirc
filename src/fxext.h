/*
 *      fxext.h
 *
 *      Copyright 2011 David Vachulka <david@konstrukce-cad.com>
 *      Copyright (C) 2009-2011 by Sander Jansen.
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

#ifndef FXEXT_H
#define	FXEXT_H

class dxEXButton : public FXButton
{
    FXDECLARE(dxEXButton)
protected:
    dxEXButton();
private:
    dxEXButton(const dxEXButton&);
    dxEXButton& operator=(const dxEXButton&);
public:
    long onPaint(FXObject*,FXSelector,void*);
public:
    dxEXButton(FXComposite* p,const FXString& text,FXIcon* ic=NULL,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=BUTTON_NORMAL,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_PAD,FXint pr=DEFAULT_PAD,FXint pt=DEFAULT_PAD,FXint pb=DEFAULT_PAD);
};

class dxEXToggleButton : public FXToggleButton
{
    FXDECLARE(dxEXToggleButton)
protected:
    dxEXToggleButton();
private:
    dxEXToggleButton(const dxEXToggleButton&);
    dxEXToggleButton& operator=(const dxEXToggleButton&);
public:
    long onPaint(FXObject*,FXSelector,void*);
public:
    dxEXToggleButton(FXComposite* p,const FXString& text1,const FXString& text2,FXIcon* icon1=NULL,FXIcon* icon2=NULL,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=TOGGLEBUTTON_NORMAL,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_PAD,FXint pr=DEFAULT_PAD,FXint pt=DEFAULT_PAD,FXint pb=DEFAULT_PAD);
};

class FXAPI dxEXTabItem : public FXTabItem
{
    FXDECLARE(dxEXTabItem)
protected:
    dxEXTabItem(){}
private:
    dxEXTabItem(const dxEXTabItem&);
    dxEXTabItem& operator=(const dxEXTabItem&);
public:
    long onPaint(FXObject*,FXSelector,void*);
public:
    /// Construct a tab item
    dxEXTabItem(FXTabBar* p,const FXString& text,FXIcon* ic=0,FXuint opts=TAB_TOP_NORMAL,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_PAD,FXint pr=DEFAULT_PAD,FXint pt=DEFAULT_PAD,FXint pb=DEFAULT_PAD);
};

class FXAPI dxEXMessageBox : public FXDialogBox
{
    FXDECLARE(dxEXMessageBox)
protected:
    dxEXMessageBox(){}
private:
    dxEXMessageBox(const dxEXMessageBox&);
    dxEXMessageBox &operator=(const dxEXMessageBox&);
    void initialize(const FXString& text,FXIcon* ic,FXuint whichbuttons);
public:
    /// Construct message box with given caption, icon, and message text
    dxEXMessageBox(FXWindow* owner,const FXString& caption,const FXString& text,FXIcon* ic=NULL,FXuint opts=0,FXint x=0,FXint y=0);
    /// Construct free floating message box with given caption, icon, and message text
    dxEXMessageBox(FXApp* app,const FXString& caption,const FXString& text,FXIcon* ic=NULL,FXuint opts=0,FXint x=0,FXint y=0);

    long onCmdClicked(FXObject*,FXSelector,void*);
    long onCmdCancel(FXObject*,FXSelector,void*);
    enum{
        ID_CLICKED_YES=FXDialogBox::ID_LAST,
        ID_CLICKED_NO,
        ID_CLICKED_OK,
        ID_CLICKED_CANCEL,
        ID_CLICKED_QUIT,
        ID_CLICKED_SAVE,
        ID_CLICKED_SKIP,
        ID_CLICKED_SKIPALL,
        ID_LAST
    };

    /**
    * Show a modal error message.
    * The text message may contain printf-tyle formatting commands.
    */
    static FXuint error(FXWindow* owner,FXuint opts,const char* caption,const char* message,...) FX_PRINTF(4,5) ;
    /**
    * Show modal error message, in free floating window.
    */
    static FXuint error(FXApp* app,FXuint opts,const char* caption,const char* message,...) FX_PRINTF(4,5) ;
    /**
    * Show a modal warning message
    * The text message may contain printf-tyle formatting commands.
    */
    static FXuint warning(FXWindow* owner,FXuint opts,const char* caption,const char* message,...) FX_PRINTF(4,5) ;
    /**
    * Show modal warning message, in free floating window.
    */
    static FXuint warning(FXApp* app,FXuint opts,const char* caption,const char* message,...) FX_PRINTF(4,5) ;
    /**
    * Show a modal question dialog
    * The text message may contain printf-tyle formatting commands.
    */
    static FXuint question(FXWindow* owner,FXuint opts,const char* caption,const char* message,...) FX_PRINTF(4,5) ;
    /**
    * Show modal question message, in free floating window.
    */
    static FXuint question(FXApp* app,FXuint opts,const char* caption,const char* message,...) FX_PRINTF(4,5) ;
    /**
    * Show a modal information dialog
    * The text message may contain printf-tyle formatting commands.
    */
    static FXuint information(FXWindow* owner,FXuint opts,const char* caption,const char* message,...) FX_PRINTF(4,5) ;
    /**
    * Show modal information message, in free floating window.
    */
    static FXuint information(FXApp* app,FXuint opts,const char* caption,const char* message,...) FX_PRINTF(4,5) ;
};

#endif	/* FXEXT_H */

