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
    FXMAPFUNC(SEL_COMMAND,              DccDialog_CLOSE,        DccDialog::onClose),
    FXMAPFUNC(SEL_CLOSE,                0,                      DccDialog::onClose),
    FXMAPFUNC(SEL_KEYPRESS,             0,                      DccDialog::onKeyPress),
    FXMAPFUNC(SEL_TIMEOUT,              DccDialog_UTIME,        DccDialog::onTimeout),
    FXMAPFUNC(SEL_COMMAND,              DccDialog_CLEAR,        DccDialog::onClear),
    FXMAPFUNC(SEL_COMMAND,              DccDialog_CANCELED,     DccDialog::onCanceled),
    FXMAPFUNC(SEL_RIGHTBUTTONRELEASE,   DccDialog_TABLE,        DccDialog::onRightClick),
    FXMAPFUNC(SEL_CHANGED,              DccDialog_TABLE,        DccDialog::onTableChanged),
    FXMAPFUNC(SEL_COMMAND,              DccDialog_POPUPCANCEL,  DccDialog::onPopupCancel)
};

FXIMPLEMENT(DccDialog, FXTopWindow, DccDialogMap, ARRAYNUMBER(DccDialogMap))

DccDialog::DccDialog(FXApp *app)
    : FXTopWindow(app, _("DCC transfers"), NULL, NULL, DECOR_ALL, 0,0,0,0, 0,0,0,0, 0,0)
{
    setIcon(ICO_BIG);
    setMiniIcon(ICO_SMALL);

    m_contents = new FXVerticalFrame(this, LAYOUT_SIDE_TOP|FRAME_NONE|LAYOUT_FILL_X|LAYOUT_FILL_Y);

    m_tableframe = new FXVerticalFrame(m_contents, FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 0,0,0,0);
    m_table = new FXTable(m_tableframe, this, DccDialog_TABLE, TABLE_COL_SIZABLE|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 2,2,2,2);
    m_table->setVisibleRows(5);
    m_table->setVisibleColumns(6);
    m_table->setTableSize(0, 6);
    m_table->setColumnText(0, _("File"));
    m_table->setColumnText(1, _("Size"));
    m_table->setColumnText(2, _("Position"));
    m_table->setColumnText(3, "%");
    m_table->setColumnText(4, _("Speed"));
    m_table->setColumnText(5, _("Remaining time"));
    m_table->setEditable(FALSE);
    m_table->showHorzGrid(FALSE);
    m_table->showVertGrid(FALSE);
    m_table->setRowHeaderWidth(0);

    m_group = new FXGroupBox(m_contents, _("Details"), FRAME_GROOVE|LAYOUT_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 4,4,4,4, 4,4);
    m_matrix = new FXMatrix(m_group, 2, MATRIX_BY_COLUMNS|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(m_matrix, _("Path:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    m_path = new FXLabel(m_matrix, "", NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN);
    new FXLabel(m_matrix, _("IP address:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    m_ip = new FXLabel(m_matrix, "", NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN);
    new FXLabel(m_matrix, _("Port:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    m_port = new FXLabel(m_matrix, "", NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN);
    new FXLabel(m_matrix, _("Nick:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    m_nick = new FXLabel(m_matrix, "", NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN);

    m_buttonframe = new FXHorizontalFrame(m_contents, LAYOUT_FILL_X);
    m_buttonClose = new dxEXButton(m_buttonframe, _("C&lose"), NULL, this, DccDialog_CLOSE, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,2);
    m_buttonClear = new dxEXButton(m_buttonframe, _("Clear &finished"), NULL, this, DccDialog_CLEAR, FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,2);
    m_buttonCanceled = new dxEXButton(m_buttonframe, _("Clear &canceled"), NULL, this, DccDialog_CANCELED, FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,2);
}

DccDialog::~DccDialog()
{

}

void DccDialog::create()
{
    FXTopWindow::create();
    show(PLACEMENT_CURSOR);
    updateTable();
    getApp()->addTimeout(this, DccDialog_UTIME, 1000);
}

long DccDialog::onClose(FXObject*,FXSelector,void*)
{
    getApp()->removeTimeout(this, DccDialog_UTIME);
    hide();    
    return 1;
}

long DccDialog::onKeyPress(FXObject *sender,FXSelector sel,void *ptr)
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

long DccDialog::onTimeout(FXObject*, FXSelector, void*)
{
    updateTable();
    getApp()->addTimeout(this, DccDialog_UTIME, 1000);
    return 1;
}

long DccDialog::onClear(FXObject*, FXSelector, void*)
{
    if(!dxirc::instance()->getDccFilesList().no())
        return 1;
    for(FXint i=dxirc::instance()->getDccFilesList().no()-1; i>-1; i--)
    {
        if(dxirc::instance()->getDccFilesList()[i].type == DCC_IN  || dxirc::instance()->getDccFilesList()[i].type == DCC_PIN)
        {
            if(dxirc::instance()->getDccFilesList()[i].currentPosition >= dxirc::instance()->getDccFilesList()[i].size)
            {
                dxirc::instance()->getDccFilesList().erase(i);
            }
        }
        else
        {
            if(dxirc::instance()->getDccFilesList()[i].finishedPosition >= dxirc::instance()->getDccFilesList()[i].size)
            {
                dxirc::instance()->getDccFilesList().erase(i);
            }
        }
    }
    updateTable();
    return 1;
}

long DccDialog::onCanceled(FXObject*, FXSelector, void*)
{
    if(!dxirc::instance()->getDccFilesList().no())
        return 1;
    for(FXint i=dxirc::instance()->getDccFilesList().no()-1; i>-1; i--)
    {
        if(dxirc::instance()->getDccFilesList()[i].canceled)
            dxirc::instance()->getDccFilesList().erase(i);
    }
    updateTable();
    return 1;
}

long DccDialog::onRightClick(FXObject*, FXSelector, void *ptr)
{
    FXEvent* event = (FXEvent*)ptr;
    if(event->moved) return 1;
    m_indexOnRight = m_table->rowAtY(event->win_y);
    if(m_indexOnRight >= dxirc::instance()->getDccFilesList().no())
        return 1;
    if(dxirc::instance()->getDccFilesList()[m_indexOnRight].canceled)
        return 1;
    FXMenuPane popup(this);
    new FXMenuCommand(&popup, _("&Cancel"), NULL, this, DccDialog_POPUPCANCEL);
    popup.create();
    popup.popup(NULL, event->root_x, event->root_y);
    getApp()->runModalWhileShown(&popup);
    return 1;
}

long DccDialog::onPopupCancel(FXObject*, FXSelector, void*)
{
    dxirc::instance()->handle(this, FXSEL(SEL_COMMAND, DccDialog_DCCCANCEL), (void*)(FXival)m_indexOnRight);
    return 1;
}

long DccDialog::onTableChanged(FXObject*, FXSelector, void *ptr)
{
    FXTablePos *tp = (FXTablePos*)ptr;
    m_contents->recalc();
    m_path->setText(dxirc::instance()->getDccFilesList()[tp->row].path);
    m_ip->setText(dxirc::instance()->getDccFilesList()[tp->row].ip);
    m_port->setText(FXStringVal(dxirc::instance()->getDccFilesList()[tp->row].port));
    m_nick->setText(dxirc::instance()->getDccFilesList()[tp->row].nick);
    return 1;
}

void DccDialog::updateTable()
{
    FXbool canceled = FALSE;
    FXbool finished = FALSE;
    if(!dxirc::instance()->getDccFilesList().no())
    {
        m_buttonClear->disable();
        m_buttonCanceled->disable();
        m_table->removeRows(0, m_table->getNumRows());
        m_group->hide();
        m_contents->recalc();
        return;
    }
    if(!shown())
        return;
    if(m_table->getNumRows() < dxirc::instance()->getDccFilesList().no())
        m_table->insertRows(0, dxirc::instance()->getDccFilesList().no()-m_table->getNumRows());
    if(m_table->getNumRows() > dxirc::instance()->getDccFilesList().no())
        m_table->removeRows(0, m_table->getNumRows()-dxirc::instance()->getDccFilesList().no());
    for(FXint i=0; i<dxirc::instance()->getDccFilesList().no(); i++)
    {
        if(dxirc::instance()->getDccFilesList()[i].type == DCC_IN  || dxirc::instance()->getDccFilesList()[i].type == DCC_PIN)
        {
            if(dxirc::instance()->getDccFilesList()[i].currentPosition >= dxirc::instance()->getDccFilesList()[i].size)
            {
                if(dxirc::instance()->getDccFilesList()[i].canceled)
                {
                    m_table->setItemIcon(i, 0, ICO_CANCEL);
                    canceled = TRUE;
                }
                else
                {
                    m_table->setItemIcon(i, 0, ICO_FINISH);
                    finished = TRUE;
                }
                m_table->setItemIconPosition(i, 0, FXTableItem::BEFORE);
                m_table->setItemJustify(i, 0, FXTableItem::LEFT);
            }
            else
            {
                if(dxirc::instance()->getDccFilesList()[i].canceled)
                {
                    m_table->setItemIcon(i, 0, ICO_CANCEL);
                    canceled = TRUE;
                }
                else m_table->setItemIcon(i, 0, ICO_DOWN);
                m_table->setItemIconPosition(i, 0, FXTableItem::BEFORE);
                m_table->setItemJustify(i, 0, FXTableItem::LEFT);
            }
        }
        else
        {
            if(dxirc::instance()->getDccFilesList()[i].finishedPosition >= dxirc::instance()->getDccFilesList()[i].size)
            {
                if(dxirc::instance()->getDccFilesList()[i].canceled)
                {
                    m_table->setItemIcon(i, 0, ICO_CANCEL);
                    canceled = TRUE;
                }
                else
                {
                    m_table->setItemIcon(i, 0, ICO_FINISH);
                    finished = TRUE;
                }
                m_table->setItemIconPosition(i, 0, FXTableItem::BEFORE);
                m_table->setItemJustify(i, 0, FXTableItem::LEFT);
            }
            else
            {
                if(dxirc::instance()->getDccFilesList()[i].canceled)
                {
                    m_table->setItemIcon(i, 0, ICO_CANCEL);
                    canceled = TRUE;
                }
                else m_table->setItemIcon(i, 0, ICO_UP);
                m_table->setItemIconPosition(i, 0, FXTableItem::BEFORE);
                m_table->setItemJustify(i, 0, FXTableItem::LEFT);
            }
        }
        m_table->setItemText(i, 0, dxirc::instance()->getDccFilesList()[i].path.rafter(PATHSEP));
        m_table->setItemText(i, 1, utils::instance().getFileSize(dxirc::instance()->getDccFilesList()[i].size));
        if(dxirc::instance()->getDccFilesList()[i].currentPosition < dxirc::instance()->getDccFilesList()[i].size)
        {
            m_table->setItemText(i, 2, utils::instance().getFileSize(dxirc::instance()->getDccFilesList()[i].currentPosition));
            m_table->setItemText(i, 3, utils::instance().getPercentPosition(dxirc::instance()->getDccFilesList()[i].size, dxirc::instance()->getDccFilesList()[i].currentPosition));
        }
        else
        {
            m_table->setItemText(i, 2, utils::instance().getFileSize(dxirc::instance()->getDccFilesList()[i].size));
            m_table->setItemText(i, 3, "100%");
        }
        if(dxirc::instance()->getDccFilesList()[i].currentPosition >= dxirc::instance()->getDccFilesList()[i].size || dxirc::instance()->getDccFilesList()[i].canceled)
        {
            m_table->setItemText(i, 4, "--:--");
            m_table->setItemText(i, 5, "--:--");
        }
        else
        {
            if(!dxirc::instance()->getDccFilesList()[i].currentPosition)
            {
                m_table->setItemText(i, 4, "--:--");
                m_table->setItemText(i, 5, "--:--");
            }
            else
            {
                m_table->setItemText(i, 4, utils::instance().getSpeed(dxirc::instance()->getDccFilesList()[i].speed));
                m_table->setItemText(i, 5, utils::instance().getRemaining(dxirc::instance()->getDccFilesList()[i].size-dxirc::instance()->getDccFilesList()[i].currentPosition, dxirc::instance()->getDccFilesList()[i].speed));
            }
        }
    }
    if(finished) m_buttonClear->enable();
    else m_buttonClear->disable();
    if(canceled) m_buttonCanceled->enable();
    else m_buttonCanceled->disable();
    m_path->setText(dxirc::instance()->getDccFilesList()[m_table->getCurrentRow()].path);
    m_ip->setText(dxirc::instance()->getDccFilesList()[m_table->getCurrentRow()].ip);
    m_port->setText(FXStringVal(dxirc::instance()->getDccFilesList()[m_table->getCurrentRow()].port));
    m_nick->setText(dxirc::instance()->getDccFilesList()[m_table->getCurrentRow()].nick);
    m_group->show();
    m_contents->recalc();
}

