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
        ID_KEYS,
        ID_VALUES,
        ID_SAVECLOSE,
        ID_CANCEL,
        ID_LAST
    };

    long OnAdd(FXObject*,FXSelector,void*);
    long OnModify(FXObject*,FXSelector,void*);
    long OnDelete(FXObject*,FXSelector,void*);
    long OnCancel(FXObject*,FXSelector,void*);
    long OnSaveClose(FXObject*,FXSelector,void*);
    long OnListSelected(FXObject*,FXSelector,void*);
    long OnListDeselected(FXObject*,FXSelector,void*);
    long OnListChanged(FXObject*,FXSelector,void*);

private:
    AliasDialog() {}
    AliasDialog(const AliasDialog& orig);

    FXVerticalFrame *content;
    FXHorizontalFrame *buttonframe, *keyframe, *valueframe;
    FXList *keys, *values;
    FXButton *buttonAdd, *buttonModify, *buttonDelete, *buttonCancel, *buttonSaveClose;
    dxStringMap aliases;

    void UpdateList();
};

#endif	/* ALIASDIALOG_H */

