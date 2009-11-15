/*
 *      scriptdialog.h
 *
 *      Copyright 2009 David Vachulka <david@konstrukce-cad.com>
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


#ifndef SCRIPTDIALOG_H
#define SCRIPTDIALOG_H

#include "defs.h"

class dxirc;

class ScriptDialog: public FXDialogBox
{
    FXDECLARE(ScriptDialog)
    public:
        ScriptDialog(dxirc *owner);
        virtual ~ScriptDialog();
        enum {
            ID_LIST = FXTopWindow::ID_LAST,
            ID_LOAD,
            ID_VIEW,
            ID_VIEWNEW,
            ID_UNLOAD,
            ID_CLOSE,
            ID_LAST
        };
        
        long OnLoad(FXObject*,FXSelector,void*);
        long OnView(FXObject*,FXSelector,void*);
        long OnViewNew(FXObject*,FXSelector,void*);
        long OnUnload(FXObject*,FXSelector,void*);
        long OnClose(FXObject*,FXSelector,void*);
        long OnList(FXObject*,FXSelector,void*);
        long OnKeyPress(FXObject*,FXSelector,void*);

    private:
        ScriptDialog() {}
        ScriptDialog(const ScriptDialog&);

        dxirc *irc;
        FXVerticalFrame *contents, *listframe;
        FXHorizontalFrame *buttonframe, *scriptframe;
        FXGroupBox *group;
        FXMatrix *matrix;
        FXList *names;
        FXTextField *name, *version;
        FXText *description, *path;
        FXButton *buttonLoad, *buttonView, *buttonUnload, *buttonClose, *buttonViewNew;
        
        void UpdateList();
        void UpdateDetails();
        void ViewFile(const FXString&);
};

#endif // SCRIPTDIALOG_H
