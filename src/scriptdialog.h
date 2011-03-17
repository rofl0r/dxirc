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
#include "fxext.h"

class dxirc;

class ScriptDialog: public FXDialogBox
{
    FXDECLARE(ScriptDialog)
    public:
        ScriptDialog(dxirc *owner);
        virtual ~ScriptDialog();
        
        long onLoad(FXObject*,FXSelector,void*);
        long onView(FXObject*,FXSelector,void*);
        long onViewNew(FXObject*,FXSelector,void*);
        long onUnload(FXObject*,FXSelector,void*);
        long onClose(FXObject*,FXSelector,void*);
        long onList(FXObject*,FXSelector,void*);
        long onKeyPress(FXObject*,FXSelector,void*);

    private:
        ScriptDialog() {}
        ScriptDialog(const ScriptDialog&);

        dxirc *m_irc;
        FXVerticalFrame *m_contents, *m_listframe;
        FXHorizontalFrame *m_buttonframe, *m_scriptframe;
        FXGroupBox *m_group;
        FXMatrix *m_matrix;
        FXList *m_names;
        FXTextField *m_name, *m_version;
        FXText *m_description, *m_path;
        dxEXButton *m_buttonLoad, *m_buttonView, *m_buttonUnload, *m_buttonClose, *m_buttonViewNew;
        
        void updateList();
        void updateDetails();
        void viewFile(const FXString&);
};

#endif // SCRIPTDIALOG_H
