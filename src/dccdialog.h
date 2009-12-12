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
        
        long OnClose(FXObject*,FXSelector,void*);
        long OnKeyPress(FXObject*,FXSelector,void*);
        long OnTimeout(FXObject*,FXSelector,void*);
        long OnClear(FXObject*,FXSelector,void*);
        long OnCanceled(FXObject*,FXSelector,void*);
        long OnRightClick(FXObject*,FXSelector,void*);
        long OnPopupCancel(FXObject*,FXSelector,void*);

    private:
        DccDialog() {}
        DccDialog(const DccDialog&);

        dxirc *irc;
        FXVerticalFrame *contents, *tableframe;
        FXMatrix *matrix;
        FXHorizontalFrame *buttonframe;
        FXButton *buttonClose, *buttonClear, *buttonCanceled;
        FXTable *table;
        FXLabel *path, *ip, *port;
        FXGroupBox *group;
        FXint indexOnRight;

        void UpdateTable();
};

#endif // DCCDIALOG_H
