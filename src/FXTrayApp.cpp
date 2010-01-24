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

#include "FXTrayApp.h"
#include "FXTrayIcon.h"
#ifndef WIN32
#include <X11/Xlib.h>
#define XEMBED_EMBEDDED_NOTIFY      0
#endif

namespace FX {

FXIMPLEMENT(FXTrayApp, FXApp, NULL, 0);

FXTrayApp::FXTrayApp(const FXString& name, const FXString& vendor) :
    FXApp(name, vendor)
{
#ifndef WIN32
    mTrayAtom = None;
#endif
}

void FXTrayApp::create()
{
    FXApp::create();

#ifdef WIN32
    mTaskbarRestartId = RegisterWindowMessage(TEXT("TaskbarCreated"));
#else
    Display* dpy = static_cast<Display*>(getDisplay() );
    int screen = DefaultScreen(dpy);
    Window root = RootWindow(dpy, screen);

    // notify new trays to us
    XSelectInput(dpy, root, StructureNotifyMask);
 
    // register some atoms to save bandwidth
    mManagerAtom = XInternAtom(dpy, "MANAGER", False);
    mXembedAtom = XInternAtom(dpy, "_XEMBED", False);
    createTrayAtom();
#endif
}

#ifdef WIN32
long FXTrayApp::dispatchEvent(FXID hwnd,unsigned int iMsg,unsigned int wParam,long lParam)
{
    // return TRUE; // event dispached successful
    if (iMsg == mTaskbarRestartId)
    {
        FXWindow* wnd = findWindowWithId(hwnd);
        if (wnd && wnd->isMemberOf(&FXTrayIcon::metaClass) ) {
            static_cast<FXTrayIcon*>(wnd)->mapToManager();
        }

        return TRUE;
    }

    return FXApp::dispatchEvent(hwnd, iMsg, wParam, lParam);
}
#else
void FXTrayApp::createTrayAtom()
{
    if (mTrayAtom != None)
        return;

    Display* dpy = static_cast<Display*>(getDisplay() );
    int screen = DefaultScreen(dpy);

    // Search for tray manager
    char atom_str[32];
    snprintf(atom_str, sizeof(atom_str),
        "_NET_SYSTEM_TRAY_S%d", screen);
    atom_str[sizeof(atom_str)-1] = '\0';

    mTrayAtom = XInternAtom(dpy, atom_str, False);
}

FXID FXTrayApp::getTrayManager()
{
    Display* dpy = static_cast<Display*>(getDisplay() );

    createTrayAtom();

    manager_id = XGetSelectionOwner(dpy, mTrayAtom);
    return manager_id;
}

bool FXTrayApp::dispatchEvent(FXRawEvent& ev)
{
    Display* dpy = static_cast<Display*>(getDisplay() );
    int screen = DefaultScreen(dpy);
    Window root = RootWindow(dpy, screen);

    // return TRUE; // event dispached successful
    if (ev.xany.type == ClientMessage &&
        ev.xclient.message_type == mManagerAtom &&
        static_cast<Atom>(ev.xclient.data.l[1]) == mTrayAtom)
    {
        manager_id = ev.xclient.data.l[2];
        //printf("New Tray 0x%lX\n", manager_id);

        XSelectInput(dpy, manager_id, StructureNotifyMask);

        // remap icons to new tray manager
        for(int i=0; i < mIcons.no(); i++) 
        {
            mIcons[i]->mapToManager();
        }

        return TRUE;
    }

    // warning: fluxbox just maps the window instead of sending this msg
    if (ev.xany.type == ClientMessage &&
        ev.xclient.message_type == mXembedAtom &&
        static_cast<Atom>(ev.xclient.data.l[1]) == XEMBED_EMBEDDED_NOTIFY)
    {
        //printf("Embeded\n");

        // the window was embed, so show it
        FXWindow* wnd = findWindowWithId(ev.xany.window);
        if (wnd) {
            wnd->show();
        }

        return TRUE;
    }

    if (ev.type == ReparentNotify)
    {
        FXWindow* wnd = findWindowWithId(ev.xreparent.window);
        if (wnd && wnd->isMemberOf(&FXTrayIcon::metaClass)) {

            if (ev.xreparent.parent == root) 
            {
                //printf("Icon lost\n");

                wnd->hide(); // doesn't work with fluxbox
                XUnmapWindow(dpy, ev.xreparent.window);

                return TRUE;
            } else {
                // tolerate non standard tray managers
                wnd->show();
            }
        }
    }

    return FXApp::dispatchEvent(ev);
}

void FXTrayApp::registerIcon(FXTrayIcon* ic)
{
    mIcons.append(ic);   
}

void FXTrayApp::unregisterIcon(FXTrayIcon* ic)
{
    mIcons.remove(ic);
}
#endif
}
