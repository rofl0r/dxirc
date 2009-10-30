/*
FoxTray, system tray icon for the FOX Toolkit

Copyright (c) 2008, Hendrik Rittich
All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are
met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in 
      the documentation and/or other materials provided with the 
      distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef FX_TRAY_ICON_H
#define FX_TRAY_ICON_H

#include "fx.h"
#ifdef WIN32
#include <windows.h>
#include <shellapi.h>
#endif

namespace FX {

enum {
    TRAY_MENU_ON_LEFT = 0x0001,
    TRAY_MENU_ON_RIGHT = 0x0002,
    TRAY_CMD_ON_LEFT = 0x0004,
    TRAY_CMD_ON_RIGHT = 0x0008
};

#ifdef WIN32
class FXTrayIcon : public FXTopWindow
{
    FXDECLARE(FXTrayIcon)
public:
    enum {
        ID_POPTIMEOUT = FXTopWindow::ID_LAST,
        ID_LAST
    };
    FXTrayIcon(FXApp* app, const FXString& text, FXIcon* icon, 
        FXPopup* pup = NULL, FXObject* target = 0, FXSelector sel = 0,
        FXuint opts = TRAY_MENU_ON_LEFT);
    ~FXTrayIcon();

    void create();

    void mapToManager();

    long onEvent(FXObject*, FXSelector, void* ptr);
    long onTimeout(FXObject*, FXSelector, void*);

    void setMenu(FXPopup* pup) { mPup = pup; }
    FXPopup* getMenu() const { return mPup; }

    void setIcon(FXIcon* icon);
    inline FXIcon* getIcon() const { return mIcon; } 

    void setText(const FXString& text);
    inline const FXString& getText() const { return mTooltip; }

    static HICON createMswIcon(FXIcon* icon);
private:
    HICON mWIcon;
    FXIcon* mIcon;
    DWORD mTrayID;

    FXPopup* mPup;
    FXuint mOpts;
    FXString mTooltip;

    static DWORD sTrayIconCount;

    FXTrayIcon()
    {}

    void setupNotifyData(NOTIFYICONDATA* data);
};
#else
class FXTrayIcon : public FXTopWindow
{
    FXDECLARE(FXTrayIcon)
public: 
    /** @cond DEV */
    enum {
        ID_BUTTON = FXTopWindow::ID_LAST,
        ID_POPTIMEOUT,
        ID_LAST
    };
    /** @endcond */

    /** Create a tray icon.
     @param app the current FXApp instance
     @param text the tool tip text
     @param icon the icon to display in the tray
     @param pup the popup menu to display
     @param target send messages to
     @param sel id 
     @param opts options. See @ref tray_options .
     */
    FXTrayIcon(FXApp* app, const FXString& text, FXIcon* icon, 
            FXPopup* pup = NULL, FXObject* target = 0, FXSelector sel = 0,
            FXuint opts = TRAY_MENU_ON_LEFT);

    ~FXTrayIcon(); 

    /** Create the server side resource of the tray icon. */
    void create();

    /** @cond DEV */
    void mapToManager(); 

    long onLeft(FXObject*, FXSelector, void*);
    long onRight(FXObject* obj, FXSelector, void* ptr);
    long onClose(FXObject*, FXSelector, void*);
    long onTimeout(FXObject*, FXSelector, void*);
    /** @endcond */

    /** Set the popup menu. */
    void setMenu(FXPopup* pup) { mPup = pup; }
    /** Get the popup menu. */
    FXPopup* getMenu() const { return mPup; }

    /** Change the icon in the tray. */
    void setIcon(FXIcon* icon);
    /** Get the icon. */
    FXIcon* getIcon() const;

    /** Change the tool tip text. */
    void setText(const FXString& text);
    /** Get the tool tip text. */
    const FXString& getText() const;

private:
    FXPopup* mPup;
    FXuint mOpts;
    FXButton* btn;

    void popup(FXint x, FXint y);
    FXTrayIcon() {}
};
#endif
}

#endif
