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
    FXMAPFUNC(SEL_COMMAND,      AliasDialog::ID_ADD,        AliasDialog::OnAdd),
    FXMAPFUNC(SEL_COMMAND,      AliasDialog::ID_MODIFY,     AliasDialog::OnModify),
    FXMAPFUNC(SEL_COMMAND,      AliasDialog::ID_DELETE,     AliasDialog::OnDelete),
    FXMAPFUNC(SEL_COMMAND,      AliasDialog::ID_CANCEL,     AliasDialog::OnCancel),
    FXMAPFUNC(SEL_CLOSE,        0,                          AliasDialog::OnCancel),
    FXMAPFUNC(SEL_COMMAND,      AliasDialog::ID_SAVECLOSE,  AliasDialog::OnSaveClose),
    FXMAPFUNC(SEL_KEYPRESS,     0,                          AliasDialog::OnKeyPress),
    FXMAPFUNC(SEL_COMMAND,      AliasDialog::ID_TABLE,      AliasDialog::OnTable),
};

FXIMPLEMENT(AliasDialog, FXDialogBox, AliasDialogMap, ARRAYNUMBER(AliasDialogMap))

AliasDialog::AliasDialog(FXMainWindow *owner)
        : FXDialogBox(owner, _("Aliases list"), DECOR_TITLE|DECOR_BORDER|DECOR_RESIZE, 0,0,0,0, 0,0,0,0, 0,0)
{
    aliases = utils::GetAliases();

    contents = new FXVerticalFrame(this, LAYOUT_FILL_X|LAYOUT_FILL_Y);

    tableframe = new FXVerticalFrame(contents, LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_THICK);
    table = new FXIconList(tableframe, this, ID_TABLE, ICONLIST_DETAILED|ICONLIST_BROWSESELECT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    table->appendHeader(_("Alias"), NULL, 150);
    table->appendHeader(_("Command"), NULL, 200);

    buttonframe = new FXHorizontalFrame(contents, LAYOUT_FILL_X);
    buttonAdd = new FXButton(buttonframe, _("&Add"), NULL, this, ID_ADD, FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 10,10,2,5);
    buttonModify = new FXButton(buttonframe, _("&Modify"), NULL, this, ID_MODIFY, FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 10,10,2,5);
    (FXint)aliases.size()? buttonModify->enable() : buttonModify->disable();
    buttonDelete = new FXButton(buttonframe, _("&Delete"), NULL, this, ID_DELETE, FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 10,10,2,5);
    (FXint)aliases.size()? buttonDelete->enable() : buttonDelete->disable();
    buttonSaveClose = new FXButton(buttonframe, _("&Save&&Close"), NULL, this, ID_SAVECLOSE, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 10,10,2,5);
    buttonCancel = new FXButton(buttonframe, _("&Cancel"), NULL, this, ID_CANCEL, FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 10,10,2,5);
}

AliasDialog::~AliasDialog()
{
}

void AliasDialog::create()
{
    FXDialogBox::create();
    UpdateTable();
    setHeight(350);
}

long AliasDialog::OnAdd(FXObject*, FXSelector, void*)
{
    FXDialogBox aliasEdit(this, _("Alias edit"), DECOR_TITLE|DECOR_BORDER, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    FXVerticalFrame *contents = new FXVerticalFrame(&aliasEdit, LAYOUT_SIDE_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0, 0, 0, 0, 10, 10, 10, 10, 0, 0);
    FXMatrix *matrix = new FXMatrix(contents, 2, MATRIX_BY_COLUMNS|LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(matrix, _("Alias:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *alias = new FXTextField(matrix, 25, NULL, 0, FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    new FXLabel(matrix, _("Command:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *command = new FXTextField(matrix, 25, NULL, 0, FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);

    FXHorizontalFrame *buttonframe = new FXHorizontalFrame(contents, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXButton(buttonframe, _("&OK"), NULL, &aliasEdit, FXDialogBox::ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0, 0, 0, 0, 32, 32, 5, 5);
    new FXButton(buttonframe, _("&Cancel"), NULL, &aliasEdit, FXDialogBox::ID_CANCEL, FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0, 0, 0, 0, 32, 32, 5, 5);

    if(aliasEdit.execute(PLACEMENT_OWNER))
    {
        if(alias->getText().empty() || command->getText().empty())
            return 1;
        if(alias->getText()[0] != '/')
        {
            FXMessageBox::information(this, MBOX_OK, _("Information"), _("Alias command has to start with '/'"));
            return 1;
        }
        if(alias->getText().contains(' '))
        {
            FXMessageBox::information(this, MBOX_OK, _("Information"), _("Alias command cann't contain space"));
            return 1;
        }
        if(CommandExist(alias->getText()))
        {
            FXMessageBox::information(this, MBOX_OK, _("Information"), _("Command '%s' already exist"), alias->getText().text());
            return 1;
        }
        aliases.insert(StringPair(alias->getText(), command->getText()));
        UpdateTable();
    }
    return 1;
}

long AliasDialog::OnModify(FXObject*, FXSelector, void*)
{
    FXint row = table->getCurrentItem();
    FXString oldkey = table->getItemText(row).before('\t');
    FXDialogBox aliasEdit(this, _("Alias edit"), DECOR_TITLE|DECOR_BORDER, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    FXVerticalFrame *contents = new FXVerticalFrame(&aliasEdit, LAYOUT_SIDE_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0, 0, 0, 0, 10, 10, 10, 10, 0, 0);
    FXMatrix *matrix = new FXMatrix(contents, 2, MATRIX_BY_COLUMNS|LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(matrix, _("Alias:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *alias = new FXTextField(matrix, 25, NULL, 0, FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    alias->setText(oldkey);
    new FXLabel(matrix, _("Command:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *command = new FXTextField(matrix, 25, NULL, 0, FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    command->setText(table->getItemText(row).after('\t'));

    FXHorizontalFrame *buttonframe = new FXHorizontalFrame(contents, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXButton(buttonframe, _("&OK"), NULL, &aliasEdit, FXDialogBox::ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0, 0, 0, 0, 32, 32, 5, 5);
    new FXButton(buttonframe, _("&Cancel"), NULL, &aliasEdit, FXDialogBox::ID_CANCEL, FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0, 0, 0, 0, 32, 32, 5, 5);

    if(aliasEdit.execute(PLACEMENT_OWNER))
        {
        if(alias->getText().empty() || command->getText().empty())
            return 1;
        if(alias->getText()[0] != '/')
        {
            FXMessageBox::information(this, MBOX_OK, _("Information"), _("Alias command has to start with '/'"));
            return 1;
        }
        if(alias->getText().contains(' '))
        {
            FXMessageBox::information(this, MBOX_OK, _("Information"), _("Alias command cann't contain space"));
            return 1;
        }
        if(CommandExist(alias->getText()) && comparecase(alias->getText(), oldkey))
        {
            FXMessageBox::information(this, MBOX_OK, _("Information"), _("Command '%s' already exist"), alias->getText().text());
            return 1;
        }
        aliases.erase(oldkey);
        aliases.insert(StringPair(alias->getText(), command->getText()));
        UpdateTable();
    }
    return 1;
}

long AliasDialog::OnDelete(FXObject*, FXSelector, void*)
{
    aliases.erase(table->getItemText(table->getCurrentItem()).before('\t'));
    if(!(FXint)aliases.size())
    {
        buttonModify->disable();
        buttonDelete->disable();
    }
    UpdateTable();
    return 1;
}

long AliasDialog::OnCancel(FXObject*, FXSelector, void*)
{
    getApp()->stopModal(this,FALSE);
    hide();
    return 1;
}

long AliasDialog::OnSaveClose(FXObject*, FXSelector, void*)
{
    utils::SetAliases(aliases);
    getApp()->stopModal(this,TRUE);
    hide();
    return 1;
}

long AliasDialog::OnKeyPress(FXObject *sender, FXSelector sel, void *ptr)
{
    if(FXTopWindow::onKeyPress(sender,sel,ptr)) return 1;
    if(((FXEvent*)ptr)->code == KEY_Escape)
    {
        handle(this,FXSEL(SEL_COMMAND,ID_CANCEL),NULL);
        return 1;
    }
    return 0;
}

long AliasDialog::OnTable(FXObject*, FXSelector, void *)
{
    buttonModify->enable();
    buttonDelete->enable();
    return 1;
}

void AliasDialog::UpdateTable()
{
    table->clearItems();
    StringIt it;
    for(it=aliases.begin(); it!=aliases.end(); it++)
    {
        table->appendItem((*it).first+"\t"+(*it).second);
    }
    if((FXint)aliases.size())
    {
        buttonModify->enable();
        buttonDelete->enable();
    }
}

FXbool AliasDialog::CommandExist(const FXString &command)
{
    StringIt it;
    for(it=aliases.begin(); it!=aliases.end(); it++)
    {
        if(!comparecase((*it).first, command)) return TRUE;
    }
    if(utils::IsCommand(command[0]=='/' ? command.after('/') : command)) return TRUE;
    return FALSE;
}
