/*
 *      dccdialog.cpp
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

#include "dccdialog.h"
#include "icons.h"
#include "config.h"
#include "i18n.h"
#include "dxirc.h"
#include "utils.h"

FXDEFMAP(DccDialog) DccDialogMap[] = {
    FXMAPFUNC(SEL_COMMAND,              DccDialog::ID_CLOSE,        DccDialog::OnClose),
    FXMAPFUNC(SEL_CLOSE,                0,                          DccDialog::OnClose),
    FXMAPFUNC(SEL_KEYPRESS,             0,                          DccDialog::OnKeyPress),
    FXMAPFUNC(SEL_TIMEOUT,              DccDialog::ID_UTIME,        DccDialog::OnTimeout),
    FXMAPFUNC(SEL_COMMAND,              DccDialog::ID_CLEAR,        DccDialog::OnClear),
    FXMAPFUNC(SEL_COMMAND,              DccDialog::ID_CANCELED,     DccDialog::OnCanceled),
    FXMAPFUNC(SEL_RIGHTBUTTONRELEASE,   DccDialog::ID_TABLE,        DccDialog::OnRightClick),
    FXMAPFUNC(SEL_COMMAND,              DccDialog::ID_POPUPCANCEL,  DccDialog::OnPopupCancel)
};

FXIMPLEMENT(DccDialog, FXTopWindow, DccDialogMap, ARRAYNUMBER(DccDialogMap))

DccDialog::DccDialog(FXApp *app, dxirc *win)
    : FXTopWindow(app, _("DCC transfers"), NULL, NULL, DECOR_ALL, 0,0,0,0, 0,0,0,0, 0,0), irc(win)
{
    setIcon(bigicon);
    setMiniIcon(smallicon);

    contents = new FXVerticalFrame(this, LAYOUT_SIDE_TOP|FRAME_NONE|LAYOUT_FILL_X|LAYOUT_FILL_Y);

    tableframe = new FXVerticalFrame(contents, FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 0,0,0,0);
    table = new FXTable(tableframe, this, ID_TABLE, TABLE_COL_SIZABLE|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 2,2,2,2);
    table->setVisibleRows(5);
    table->setVisibleColumns(5);
    table->setTableSize(0, 5);
    table->setColumnText(0, _("File"));
    table->setColumnText(1, _("Size"));
    table->setColumnText(2, _("Position"));
    table->setColumnText(3, _("Speed"));
    table->setColumnText(4, _("Remaining time"));
    table->setEditable(FALSE);
    table->showHorzGrid(FALSE);
    table->showVertGrid(FALSE);
    table->setRowHeaderWidth(0);

    group = new FXGroupBox(contents, _("Details"), FRAME_GROOVE|LAYOUT_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 4,4,4,4, 4,4);
    matrix = new FXMatrix(group, 2, MATRIX_BY_COLUMNS|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(matrix, _("Path:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    path = new FXLabel(matrix, "", NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN);
    new FXLabel(matrix, _("IP address:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    ip = new FXLabel(matrix, "", NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN);
    new FXLabel(matrix, _("Port:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    port = new FXLabel(matrix, "", NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN);

    buttonframe = new FXHorizontalFrame(contents, LAYOUT_FILL_X);
    buttonClose = new FXButton(buttonframe, _("C&lose"), NULL, this, ID_CLOSE, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,5);
    buttonClear = new FXButton(buttonframe, _("Clear &finished"), NULL, this, ID_CLEAR, FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,5);
    buttonCanceled = new FXButton(buttonframe, _("Clear &canceled"), NULL, this, ID_CANCELED, FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,5);
}

DccDialog::~DccDialog()
{

}

void DccDialog::create()
{
    FXTopWindow::create();
    show(PLACEMENT_CURSOR);
    UpdateTable();
    getApp()->addTimeout(this, ID_UTIME, 1000);
}

long DccDialog::OnClose(FXObject*,FXSelector,void*)
{
    getApp()->removeTimeout(this, ID_UTIME);
    hide();    
    return 1;
}

long DccDialog::OnKeyPress(FXObject *sender,FXSelector sel,void *ptr)
{
    FXEvent *event = (FXEvent*)ptr;
    if(event->code == KEY_Escape)
    {
        handle(this, FXSEL(SEL_COMMAND, ID_CLOSE), NULL);
        return 1;
    }
    else
    {
        if(FXTopWindow::onKeyPress(sender,sel,ptr))
            return 1;
    }
    return 0;
}

long DccDialog::OnTimeout(FXObject*, FXSelector, void*)
{
    UpdateTable();
    getApp()->addTimeout(this, ID_UTIME, 1000);
    return 1;
}

long DccDialog::OnClear(FXObject*, FXSelector, void*)
{
    if(!irc->dccfilesList.no())
        return 1;
    for(FXint i=irc->dccfilesList.no()-1; i>-1; i--)
    {
        if(irc->dccfilesList[i].type == DCC_IN  || irc->dccfilesList[i].type == DCC_PIN)
        {
            if(irc->dccfilesList[i].currentPosition >= irc->dccfilesList[i].size)
            {
                irc->dccfilesList.erase(i);
            }
        }
        else
        {
            if(irc->dccfilesList[i].finishedPosition >= irc->dccfilesList[i].size)
            {
                irc->dccfilesList.erase(i);
            }
        }
    }
    UpdateTable();
    return 1;
}

long DccDialog::OnCanceled(FXObject*, FXSelector, void*)
{
    if(!irc->dccfilesList.no())
        return 1;
    for(FXint i=irc->dccfilesList.no()-1; i>-1; i--)
    {
        if(irc->dccfilesList[i].canceled)
            irc->dccfilesList.erase(i);
    }
    UpdateTable();
    return 1;
}

long DccDialog::OnRightClick(FXObject*, FXSelector, void *ptr)
{
    FXEvent* event = (FXEvent*)ptr;
    if(event->moved) return 1;
    indexOnRight = table->rowAtY(event->win_y);
    if(indexOnRight == irc->dccfilesList.no()
            || irc->dccfilesList[indexOnRight].currentPosition >= irc->dccfilesList[indexOnRight].size
            || irc->dccfilesList[indexOnRight].canceled)
        return 1;
    FXMenuPane popup(this);
    new FXMenuCommand(&popup, _("&Cancel"), NULL, this, ID_POPUPCANCEL);
    popup.create();
    popup.popup(NULL, event->root_x, event->root_y);
    getApp()->runModalWhileShown(&popup);
    return 1;
}

long DccDialog::OnPopupCancel(FXObject*, FXSelector, void*)
{
    irc->handle(this, FXSEL(SEL_COMMAND, DccDialog::ID_DCCCANCEL), (void*)(FXival)indexOnRight);
    return 1;
}

void DccDialog::UpdateTable()
{
    FXbool canceled = FALSE;
    FXbool finished = FALSE;
    if(!irc->dccfilesList.no())
    {
        buttonClear->disable();
        buttonCanceled->disable();
        table->removeRows(0, table->getNumRows());
        group->hide();
        contents->recalc();
        return;
    }
    if(!shown())
        return;
    if(table->getNumRows() < irc->dccfilesList.no())
        table->insertRows(0, irc->dccfilesList.no()-table->getNumRows());
    if(table->getNumRows() > irc->dccfilesList.no())
        table->removeRows(0, table->getNumRows()-irc->dccfilesList.no());
    for(FXint i=0; i<irc->dccfilesList.no(); i++)
    {
        if(irc->dccfilesList[i].type == DCC_IN  || irc->dccfilesList[i].type == DCC_PIN)
        {
            if(irc->dccfilesList[i].currentPosition >= irc->dccfilesList[i].size)
            {
                if(irc->dccfilesList[i].canceled)
                {
                    table->setItemIcon(i, 0, cancelicon);
                    canceled = TRUE;
                }
                else
                {
                    table->setItemIcon(i, 0, finishicon);
                    finished = TRUE;
                }
                table->setItemIconPosition(i, 0, FXTableItem::BEFORE);
                table->setItemJustify(i, 0, FXTableItem::LEFT);
            }
            else
            {
                if(irc->dccfilesList[i].canceled)
                {
                    table->setItemIcon(i, 0, cancelicon);
                    canceled = TRUE;
                }
                else table->setItemIcon(i, 0, downicon);
                table->setItemIconPosition(i, 0, FXTableItem::BEFORE);
                table->setItemJustify(i, 0, FXTableItem::LEFT);
            }
        }
        else
        {
            if(irc->dccfilesList[i].finishedPosition >= irc->dccfilesList[i].size)
            {
                if(irc->dccfilesList[i].canceled)
                {
                    table->setItemIcon(i, 0, cancelicon);
                    canceled = TRUE;
                }
                else
                {
                    table->setItemIcon(i, 0, finishicon);
                    finished = TRUE;
                }
                table->setItemIconPosition(i, 0, FXTableItem::BEFORE);
                table->setItemJustify(i, 0, FXTableItem::LEFT);
            }
            else
            {
                if(irc->dccfilesList[i].canceled)
                {
                    table->setItemIcon(i, 0, cancelicon);
                    canceled = TRUE;
                }
                else table->setItemIcon(i, 0, upicon);
                table->setItemIconPosition(i, 0, FXTableItem::BEFORE);
                table->setItemJustify(i, 0, FXTableItem::LEFT);
            }
        }
        table->setItemText(i, 0, irc->dccfilesList[i].path.rafter(PATHSEP));
        table->setItemText(i, 1, utils::GetFileSize(irc->dccfilesList[i].size));
        if(irc->dccfilesList[i].currentPosition < irc->dccfilesList[i].size)
            table->setItemText(i, 2, utils::GetFileSize(irc->dccfilesList[i].currentPosition));
        else
            table->setItemText(i, 2, "--:--");
        if(irc->dccfilesList[i].currentPosition >= irc->dccfilesList[i].size || irc->dccfilesList[i].canceled)
        {
            table->setItemText(i, 3, "--:--");
            table->setItemText(i, 4, "--:--");
        }
        else
        {
            if(!irc->dccfilesList[i].currentPosition)
            {
                table->setItemText(i, 3, "--:--");
                table->setItemText(i, 4, "--:--");
            }
            else
            {
                table->setItemText(i, 3, utils::GetSpeed(irc->dccfilesList[i].currentPosition-irc->dccfilesList[i].previousPostion));
                table->setItemText(i, 4, utils::GetRemaining(irc->dccfilesList[i].size-irc->dccfilesList[i].currentPosition, irc->dccfilesList[i].currentPosition-irc->dccfilesList[i].previousPostion));
            }
        }
    }
    if(finished) buttonClear->enable();
    else buttonClear->disable();
    if(canceled) buttonCanceled->enable();
    else buttonCanceled->disable();
    path->setText(irc->dccfilesList[table->getCurrentRow()].path);
    ip->setText(irc->dccfilesList[table->getCurrentRow()].ip);
    port->setText(FXStringVal(irc->dccfilesList[table->getCurrentRow()].port));
    group->show();
    contents->recalc();
}

