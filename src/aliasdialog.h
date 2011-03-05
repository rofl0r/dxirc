/*
 *      aliasdialog.h
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

#ifndef ALIASDIALOG_H
#define	ALIASDIALOG_H

#include "defs.h"
#include "fxext.h"

class AliasDialog: public FXDialogBox
{
    FXDECLARE(AliasDialog)
public:
    AliasDialog(FXMainWindow *owner);
    virtual ~AliasDialog();
    enum {
        ID_ADD = FXTopWindow::ID_LAST,
        ID_MODIFY,
        ID_DELETE,
        ID_TABLE,
        ID_SAVECLOSE,
        ID_CANCEL,
        ID_LAST
    };

    virtual void create();

    long onAdd(FXObject*,FXSelector,void*);
    long onModify(FXObject*,FXSelector,void*);
    long onDelete(FXObject*,FXSelector,void*);
    long onCancel(FXObject*,FXSelector,void*);
    long onSaveClose(FXObject*,FXSelector,void*);
    long onTable(FXObject*,FXSelector,void*);
    long onKeyPress(FXObject*,FXSelector,void*);

private:
    AliasDialog() {}
    AliasDialog(const AliasDialog& orig);

    FXVerticalFrame *m_contents, *m_tableframe;
    FXHorizontalFrame *m_buttonframe;
    FXIconList *m_table;
    dxEXButton *m_buttonAdd, *m_buttonModify, *m_buttonDelete, *m_buttonCancel, *m_buttonSaveClose;
    dxStringMap m_aliases;

    void updateTable();
    FXbool commandExist(const FXString&);
};

#endif	/* ALIASDIALOG_H */

