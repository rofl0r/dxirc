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
#include "fxext.h"

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

        dxServerInfoArray getServers() { return m_serverList; }
        FXint getIndexJoin() { return m_indexJoin; }

        long onJoin(FXObject*,FXSelector,void*);
        long onAdd(FXObject*,FXSelector,void*);
        long onModify(FXObject*,FXSelector,void*);
        long onDelete(FXObject*,FXSelector,void*);
        long onCancel(FXObject*,FXSelector,void*);
        long onSaveClose(FXObject*,FXSelector,void*);
        long onList(FXObject*,FXSelector,void*);
        long onDoubleClick(FXObject*,FXSelector,void*);
        long onKeyPress(FXObject*,FXSelector,void*);

    private:
        ServerDialog() {}
        ServerDialog(const ServerDialog&);

        dxServerInfoArray m_serverList;
        FXint m_indexJoin;
        FXVerticalFrame *m_contents, *m_listframe;
        FXHorizontalFrame *m_buttonframe, *m_serverframe;
        FXGroupBox *m_group;
        FXMatrix *m_matrix;
        FXList *m_names;
        FXTextField *m_hostname, *m_port, *m_passwd, *m_nick, *m_realname, *m_autoconnect, *m_usessl;
        FXText *m_channels, *m_commands;
        dxEXButton *m_buttonJoin, *m_buttonAdd, *m_buttonModify, *m_buttonDelete, *m_buttonCancel, *m_buttonSaveClose;

        FXbool hostnameExist(const FXString&, const FXint&, const FXString&);
        void updateList();
        void updateDetails();
};

#endif // SERVERDIALOG_H
