/*
 *      dccdialog.h
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


#ifndef DCCDIALOG_H
#define DCCDIALOG_H

#include "defs.h"

class dxirc;

class DccDialog: public FXTopWindow
{
    FXDECLARE(DccDialog)
    public:
        DccDialog(FXApp *app, dxirc *win);
        virtual ~DccDialog();
        enum {
            ID_CLOSE = FXMainWindow::ID_LAST+150,
            ID_CLEAR,
            ID_CANCELED,
            ID_UTIME,
            ID_POPUPCANCEL,
            ID_DCCCANCEL,
            ID_TABLE,
            ID_LAST
        };

        virtual void create();
        
        long onClose(FXObject*,FXSelector,void*);
        long onKeyPress(FXObject*,FXSelector,void*);
        long onTimeout(FXObject*,FXSelector,void*);
        long onClear(FXObject*,FXSelector,void*);
        long onCanceled(FXObject*,FXSelector,void*);
        long onRightClick(FXObject*,FXSelector,void*);
        long onPopupCancel(FXObject*,FXSelector,void*);
        long onTableChanged(FXObject*,FXSelector,void*);

    private:
        DccDialog() {}
        DccDialog(const DccDialog&);

        dxirc *m_irc;
        FXVerticalFrame *m_contents, *m_tableframe;
        FXMatrix *m_matrix;
        FXHorizontalFrame *m_buttonframe;
        FXButton *m_buttonClose, *m_buttonClear, *m_buttonCanceled;
        FXTable *m_table;
        FXLabel *m_path, *m_ip, *m_port, *m_nick;
        FXGroupBox *m_group;
        FXint m_indexOnRight;

        void updateTable();
};

#endif // DCCDIALOG_H
