/*
 *      serverdialog.h
 *
 *      Copyright 2008 David Vachulka <david@konstrukce-cad.com>
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


#ifndef SERVERDIALOG_H
#define SERVERDIALOG_H

#include "defs.h"

class ServerDialog: public FXDialogBox
{
    FXDECLARE(ServerDialog)
    public:
        ServerDialog(FXMainWindow *owner, dxServerInfoArray servers);
        virtual ~ServerDialog();
        enum {
            ID_JOIN = FXTopWindow::ID_LAST,
            ID_LIST,
            ID_CANCEL,
            ID_ADD,
            ID_MODIFY,
            ID_DELETE,
            ID_SAVECLOSE,
            ID_LAST
        };

        dxServerInfoArray GetServers() { return serverList; }
        FXint GetIndexJoin() { return indexJoin; }

        long OnJoin(FXObject*,FXSelector,void*);
        long OnAdd(FXObject*,FXSelector,void*);
        long OnModify(FXObject*,FXSelector,void*);
        long OnDelete(FXObject*,FXSelector,void*);
        long OnCancel(FXObject*,FXSelector,void*);
        long OnSaveClose(FXObject*,FXSelector,void*);
        long OnListSelected(FXObject*,FXSelector,void*);
        long OnListDeselected(FXObject*,FXSelector,void*);
        long OnDoubleClick(FXObject*,FXSelector,void*);
        long OnKeyPress(FXObject*,FXSelector,void*);

    private:
        ServerDialog() {}
        ServerDialog(const ServerDialog&);

        dxServerInfoArray serverList;
        FXint indexJoin;
        FXVerticalFrame *contents, *listframe;
        FXHorizontalFrame *buttonframe, *serverframe;
        FXGroupBox *group;
        FXMatrix *matrix;
        FXList *names;
        FXTextField *hostname, *port, *passwd, *nick, *realname, *channels;
        FXText *commands;
        FXCheckButton *buttonAuto, *buttonSsl;
        FXButton *buttonJoin, *buttonAdd, *buttonModify, *buttonDelete, *buttonCancel, *buttonSaveClose;

        FXbool HostnameExist(const FXString&);
        void UpdateList();
        void UpdateDetails();
};

#endif // SERVERDIALOG_H
