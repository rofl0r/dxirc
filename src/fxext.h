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

class dxEXTabItem : public FXTabItem
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

class dxEXMessageBox : public FXDialogBox
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

class dxEXInputDialog : public FXInputDialog
{
    FXDECLARE(dxEXInputDialog)
protected:
    dxEXInputDialog(){}
private:
    dxEXInputDialog(const dxEXInputDialog&);
    dxEXInputDialog &operator=(const dxEXInputDialog&);
    void initialize(const FXString& text,FXIcon* icon);
public:
    /// Construct input dialog box with given caption, icon, and prompt text
    dxEXInputDialog(FXWindow* owner,const FXString& caption,const FXString& label,FXIcon* icon=NULL,FXuint opts=INPUTDIALOG_STRING,FXint x=0,FXint y=0,FXint w=0,FXint h=0);
    /// Construct free floating input dialog box with given caption, icon, and prompt text
    dxEXInputDialog(FXApp* app,const FXString& caption,const FXString& label,FXIcon* icon=NULL,FXuint opts=INPUTDIALOG_STRING,FXint x=0,FXint y=0,FXint w=0,FXint h=0);

    /**
    * Prompt for a string, start with the initial value.
    * Return TRUE if the new value is accepted, and false otherwise.
    */
    static FXbool getString(FXString& result,FXWindow* owner,const FXString& caption,const FXString& label,FXIcon* icon=NULL);
    /**
    * Prompt for a string, in free floating window.
    */
    static FXbool getString(FXString& result,FXApp* app,const FXString& caption,const FXString& label,FXIcon* icon=NULL);
    /**
    * Prompt for an integer number, start with the given initial value.
    * Return TRUE if the new value is accepted, and false otherwise.
    * The input is constrained between lo and hi.
    */
    static FXbool getInteger(FXint& result,FXWindow* owner,const FXString& caption,const FXString& label,FXIcon* icon=NULL,FXint lo=-2147483647,FXint hi=2147483647);
    /**
    * Prompt for a integer number, in free floating window.
    */
    static FXbool getInteger(FXint& result,FXApp* app,const FXString& caption,const FXString& label,FXIcon* icon=NULL,FXint lo=-2147483647,FXint hi=2147483647);
    /**
    * Prompt for an real number, start with the given initial value.
    * Return TRUE if the new value is accepted, and false otherwise.
    * The input is constrained between lo and hi.
    */
    static FXbool getReal(FXdouble& result,FXWindow* owner,const FXString& caption,const FXString& label,FXIcon* icon=NULL,FXdouble lo=-1.797693134862315e+308,FXdouble hi=1.797693134862315e+308);
    /**
    * Prompt for a real number, in free floating window.
    */
    static FXbool getReal(FXdouble& result,FXApp* app,const FXString& caption,const FXString& label,FXIcon* icon=NULL,FXdouble lo=-1.797693134862315e+308,FXdouble hi=1.797693134862315e+308);
};

class dxEXFileDialog : public FXFileDialog
{
    FXDECLARE(dxEXFileDialog)
protected:
    dxEXFileDialog(){}
private:
    dxEXFileDialog(const dxEXFileDialog&);
    dxEXFileDialog &operator=(const dxEXFileDialog&);
public:
    dxEXFileDialog(FXWindow* owner,const FXString& name,FXuint opts=0,FXint x=0,FXint y=0,FXint w=500,FXint h=300);
    dxEXFileDialog(FXApp* a,const FXString& name,FXuint opts=0,FXint x=0,FXint y=0,FXint w=500,FXint h=300);
};

class dxEXDirDialog : public FXDirDialog
{
    FXDECLARE(dxEXDirDialog)
protected:
    dxEXDirDialog(){}
private:
    dxEXDirDialog(const dxEXDirDialog&);
    dxEXDirDialog &operator=(const dxEXDirDialog&);
public:
    dxEXDirDialog(FXWindow* owner,const FXString& name,FXuint opts=0,FXint x=0,FXint y=0,FXint w=500,FXint h=300);
    dxEXDirDialog(FXApp* a,const FXString& name,FXuint opts=0,FXint x=0,FXint y=0,FXint w=500,FXint h=300);
};

class dxEXFontDialog : public FXFontDialog
{
    FXDECLARE(dxEXFontDialog)
protected:
    dxEXFontDialog(){}
private:
    dxEXFontDialog(const dxEXFontDialog&);
    dxEXFontDialog &operator=(const dxEXFontDialog&);
public:
    dxEXFontDialog(FXWindow* owner,const FXString& name,FXuint opts=0,FXint x=0,FXint y=0,FXint w=650,FXint h=380);
};

class dxEXNotify : public FXShell
{
    FXDECLARE(dxEXNotify)
protected:
    FXuint m_delay;
    FXString m_title;
    FXString m_body;
    FXColor m_textColor;
    FXIcon *m_icon;
    FXFont *m_titleFont;
    FXFont *m_bodyFont;
    FXbool m_popped;
    FXint m_position; //0-lefttop,1-righttop,2-leftbottom,3-rightbottom
    dxEXNotify();
    virtual bool doesOverrideRedirect() const;
    FXint textHeight() const;
    FXint textWidth() const;
private:
    dxEXNotify(const dxEXNotify&);
    dxEXNotify &operator=(const dxEXNotify&);
#ifdef WIN32
    virtual const char* GetClass() const;
#endif
public:
    enum {
        ID_NOTIFY_HIDE = FXShell::ID_LAST,
        ID_LAST
    };
    dxEXNotify(FXApp *a, FXIcon *icon, FXString title, FXuint delay=5000);
    virtual ~dxEXNotify();

    virtual void create();
    virtual void detach();
    virtual void show();
    virtual FXint getDefaultWidth();
    virtual FXint getDefaultHeight();
    void setText(const FXString& text);
    FXString getText() const { return m_body; }
    void setFont(FXFont *fnt);
    FXFont* getFont() const { return m_bodyFont; }
    void setTextColor(FXColor clr);
    FXColor getTextColor() const { return m_textColor; }
    virtual void save(FXStream& store) const;
    virtual void load(FXStream& store);
    virtual bool doesSaveUnder() const;
    void notify(FXint pos=-1);

    long onPaint(FXObject*, FXSelector, void*);
    long onNotifyHide(FXObject*, FXSelector, void*);
};

#endif	/* FXEXT_H */

