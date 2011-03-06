/*
 *      aliasdialog.cpp
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

#include "aliasdialog.h"
#include "utils.h"
#include "config.h"
#include "i18n.h"

FXDEFMAP(AliasDialog) AliasDialogMap[] = {
    FXMAPFUNC(SEL_COMMAND,      AliasDialog::ID_ADD,        AliasDialog::onAdd),
    FXMAPFUNC(SEL_COMMAND,      AliasDialog::ID_MODIFY,     AliasDialog::onModify),
    FXMAPFUNC(SEL_COMMAND,      AliasDialog::ID_DELETE,     AliasDialog::onDelete),
    FXMAPFUNC(SEL_COMMAND,      AliasDialog::ID_CANCEL,     AliasDialog::onCancel),
    FXMAPFUNC(SEL_CLOSE,        0,                          AliasDialog::onCancel),
    FXMAPFUNC(SEL_COMMAND,      AliasDialog::ID_SAVECLOSE,  AliasDialog::onSaveClose),
    FXMAPFUNC(SEL_KEYPRESS,     0,                          AliasDialog::onKeyPress),
    FXMAPFUNC(SEL_COMMAND,      AliasDialog::ID_TABLE,      AliasDialog::onTable),
};

FXIMPLEMENT(AliasDialog, FXDialogBox, AliasDialogMap, ARRAYNUMBER(AliasDialogMap))

AliasDialog::AliasDialog(FXMainWindow *owner)
        : FXDialogBox(owner, _("Aliases list"), DECOR_TITLE|DECOR_BORDER|DECOR_RESIZE, 0,0,0,0, 0,0,0,0, 0,0)
{
    m_aliases = utils::instance().getAliases();

    m_contents = new FXVerticalFrame(this, LAYOUT_FILL_X|LAYOUT_FILL_Y);

    m_tableframe = new FXVerticalFrame(m_contents, LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_THICK);
    m_table = new FXIconList(m_tableframe, this, ID_TABLE, ICONLIST_DETAILED|ICONLIST_BROWSESELECT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    m_table->appendHeader(_("Alias"), NULL, 150);
    m_table->appendHeader(_("Command"), NULL, 200);

    m_buttonframe = new FXHorizontalFrame(m_contents, LAYOUT_FILL_X);
    m_buttonCancel = new dxEXButton(m_buttonframe, _("&Cancel"), NULL, this, ID_CANCEL, FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,2);
    m_buttonSaveClose = new dxEXButton(m_buttonframe, _("&Save&&Close"), NULL, this, ID_SAVECLOSE, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,2);
    m_buttonDelete = new dxEXButton(m_buttonframe, _("&Delete"), NULL, this, ID_DELETE, FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,2);
    (FXint)m_aliases.size()? m_buttonDelete->enable() : m_buttonDelete->disable();
    m_buttonModify = new dxEXButton(m_buttonframe, _("&Modify"), NULL, this, ID_MODIFY, FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,2);
    (FXint)m_aliases.size()? m_buttonModify->enable() : m_buttonModify->disable();
    m_buttonAdd = new dxEXButton(m_buttonframe, _("&Add"), NULL, this, ID_ADD, FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,2);
}

AliasDialog::~AliasDialog()
{
}

void AliasDialog::create()
{
    FXDialogBox::create();
    updateTable();
    setHeight(350);
}

long AliasDialog::onAdd(FXObject*, FXSelector, void*)
{
    FXDialogBox aliasEdit(this, _("Alias edit"), DECOR_TITLE|DECOR_BORDER, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    FXVerticalFrame *contents = new FXVerticalFrame(&aliasEdit, LAYOUT_SIDE_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0, 0, 0, 0, 10, 10, 10, 10, 0, 0);
    FXMatrix *matrix = new FXMatrix(contents, 2, MATRIX_BY_COLUMNS|LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(matrix, _("Alias:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *alias = new FXTextField(matrix, 25, NULL, 0, FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    new FXLabel(matrix, _("Command:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *command = new FXTextField(matrix, 25, NULL, 0, FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);

    FXHorizontalFrame *buttonframe = new FXHorizontalFrame(contents, LAYOUT_FILL_X|LAYOUT_FILL_Y|PACK_UNIFORM_WIDTH);
    new dxEXButton(buttonframe, _("&Cancel"), NULL, &aliasEdit, FXDialogBox::ID_CANCEL, FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0, 0, 0, 0, 10, 10, 2, 2);
    new dxEXButton(buttonframe, _("&OK"), NULL, &aliasEdit, FXDialogBox::ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0, 0, 0, 0, 10, 10, 2, 2);

    if(aliasEdit.execute(PLACEMENT_OWNER))
    {
        if(alias->getText().empty() || command->getText().empty())
            return 1;
        if(alias->getText()[0] != '/')
        {
            dxEXMessageBox::information(this, MBOX_OK, _("Information"), _("Alias command has to start with '/'"));
            return 1;
        }
        if(alias->getText().contains(' '))
        {
            dxEXMessageBox::information(this, MBOX_OK, _("Information"), _("Alias command cann't contain space"));
            return 1;
        }
        if(commandExist(alias->getText()))
        {
            dxEXMessageBox::information(this, MBOX_OK, _("Information"), _("Command '%s' already exist"), alias->getText().text());
            return 1;
        }
        m_aliases.insert(StringPair(alias->getText(), command->getText()));
        updateTable();
    }
    return 1;
}

long AliasDialog::onModify(FXObject*, FXSelector, void*)
{
    FXint row = m_table->getCurrentItem();
    FXString oldkey = m_table->getItemText(row).before('\t');
    FXDialogBox aliasEdit(this, _("Alias edit"), DECOR_TITLE|DECOR_BORDER, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    FXVerticalFrame *contents = new FXVerticalFrame(&aliasEdit, LAYOUT_SIDE_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0, 0, 0, 0, 10, 10, 10, 10, 0, 0);
    FXMatrix *matrix = new FXMatrix(contents, 2, MATRIX_BY_COLUMNS|LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(matrix, _("Alias:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *alias = new FXTextField(matrix, 25, NULL, 0, FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    alias->setText(oldkey);
    new FXLabel(matrix, _("Command:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *command = new FXTextField(matrix, 25, NULL, 0, FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    command->setText(m_table->getItemText(row).after('\t'));

    FXHorizontalFrame *buttonframe = new FXHorizontalFrame(contents, LAYOUT_FILL_X|LAYOUT_FILL_Y|PACK_UNIFORM_WIDTH);
    new dxEXButton(buttonframe, _("&Cancel"), NULL, &aliasEdit, FXDialogBox::ID_CANCEL, FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0, 0, 0, 0, 10, 10, 2, 2);
    new dxEXButton(buttonframe, _("&OK"), NULL, &aliasEdit, FXDialogBox::ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0, 0, 0, 0, 10, 10, 2, 2);

    if(aliasEdit.execute(PLACEMENT_OWNER))
        {
        if(alias->getText().empty() || command->getText().empty())
            return 1;
        if(alias->getText()[0] != '/')
        {
            dxEXMessageBox::information(this, MBOX_OK, _("Information"), _("Alias command has to start with '/'"));
            return 1;
        }
        if(alias->getText().contains(' '))
        {
            dxEXMessageBox::information(this, MBOX_OK, _("Information"), _("Alias command cann't contain space"));
            return 1;
        }
        if(commandExist(alias->getText()) && comparecase(alias->getText(), oldkey))
        {
            dxEXMessageBox::information(this, MBOX_OK, _("Information"), _("Command '%s' already exist"), alias->getText().text());
            return 1;
        }
        m_aliases.erase(oldkey);
        m_aliases.insert(StringPair(alias->getText(), command->getText()));
        updateTable();
    }
    return 1;
}

long AliasDialog::onDelete(FXObject*, FXSelector, void*)
{
    m_aliases.erase(m_table->getItemText(m_table->getCurrentItem()).before('\t'));
    if(!(FXint)m_aliases.size())
    {
        m_buttonModify->disable();
        m_buttonDelete->disable();
    }
    updateTable();
    return 1;
}

long AliasDialog::onCancel(FXObject*, FXSelector, void*)
{
    getApp()->stopModal(this,FALSE);
    hide();
    return 1;
}

long AliasDialog::onSaveClose(FXObject*, FXSelector, void*)
{
    utils::instance().setAliases(m_aliases);
    getApp()->stopModal(this,TRUE);
    hide();
    return 1;
}

long AliasDialog::onKeyPress(FXObject *sender, FXSelector sel, void *ptr)
{
    if(FXTopWindow::onKeyPress(sender,sel,ptr)) return 1;
    if(((FXEvent*)ptr)->code == KEY_Escape)
    {
        handle(this,FXSEL(SEL_COMMAND,ID_CANCEL),NULL);
        return 1;
    }
    return 0;
}

long AliasDialog::onTable(FXObject*, FXSelector, void *)
{
    m_buttonModify->enable();
    m_buttonDelete->enable();
    return 1;
}

void AliasDialog::updateTable()
{
    m_table->clearItems();
    StringIt it;
    for(it=m_aliases.begin(); it!=m_aliases.end(); it++)
    {
        m_table->appendItem((*it).first+"\t"+(*it).second);
    }
    if((FXint)m_aliases.size())
    {
        m_buttonModify->enable();
        m_buttonDelete->enable();
    }
}

FXbool AliasDialog::commandExist(const FXString &command)
{
    StringIt it;
    for(it=m_aliases.begin(); it!=m_aliases.end(); it++)
    {
        if(!comparecase((*it).first, command)) return TRUE;
    }
    if(utils::instance().isCommand(command[0]=='/' ? command.after('/') : command)) return TRUE;
    return FALSE;
}
