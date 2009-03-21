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

#include <fx.h>
#include "aliasdialog.h"
#include "utils.h"
#include "config.h"
#include "i18n.h"

FXDEFMAP(AliasDialog) AliasDialogMap[] = {
    FXMAPFUNC(SEL_COMMAND,  AliasDialog::ID_ADD,        AliasDialog::OnAdd),
    FXMAPFUNC(SEL_COMMAND,  AliasDialog::ID_MODIFY,     AliasDialog::OnModify),
    FXMAPFUNC(SEL_COMMAND,  AliasDialog::ID_DELETE,     AliasDialog::OnDelete),
    FXMAPFUNC(SEL_COMMAND,  AliasDialog::ID_CANCEL,     AliasDialog::OnCancel),
    FXMAPFUNC(SEL_COMMAND,  AliasDialog::ID_SAVECLOSE,  AliasDialog::OnSaveClose),
    FXMAPFUNCS(SEL_SELECTED,    AliasDialog::ID_KEYS, AliasDialog::ID_VALUES,   AliasDialog::OnListSelected),
    FXMAPFUNCS(SEL_DESELECTED,  AliasDialog::ID_KEYS, AliasDialog::ID_VALUES,   AliasDialog::OnListDeselected),
    FXMAPFUNCS(SEL_CHANGED,     AliasDialog::ID_KEYS, AliasDialog::ID_VALUES,   AliasDialog::OnListChanged)
};

FXIMPLEMENT(AliasDialog, FXDialogBox, AliasDialogMap, ARRAYNUMBER(AliasDialogMap))

AliasDialog::AliasDialog(FXMainWindow *owner)
        : FXDialogBox(owner, _("Aliases list"), DECOR_TITLE|DECOR_BORDER, 0,0,0,0, 0,0,0,0, 0,0)
{
    aliases = utils::GetAliases();

    content = new FXVerticalFrame(this, LAYOUT_SIDE_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 10,10,10,10, 0,0);

    new FXLabel(content, _("Aliases"), NULL, JUSTIFY_LEFT);
    keyframe = new FXHorizontalFrame(content, FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    keys = new FXList(keyframe, this, ID_KEYS, LIST_SINGLESELECT|LAYOUT_FILL_X|LAYOUT_FILL_Y);

    new FXLabel(content, _("Commands"), NULL, JUSTIFY_LEFT);
    valueframe = new FXHorizontalFrame(content, FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    values = new FXList(valueframe, this, ID_VALUES, LIST_SINGLESELECT|LAYOUT_FILL_X|LAYOUT_FILL_Y);

    buttonframe = new FXHorizontalFrame(content, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    buttonAdd = new FXButton(buttonframe, _("Add"), NULL, this, ID_ADD, FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 10,10,2,5);
    buttonModify = new FXButton(buttonframe, _("Modify"), NULL, this, ID_MODIFY, FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 10,10,2,5);
    (FXint)aliases.size()? buttonModify->enable() : buttonModify->disable();
    buttonDelete = new FXButton(buttonframe, _("Delete"), NULL, this, ID_DELETE, FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 10,10,2,5);
    (FXint)aliases.size()? buttonDelete->enable() : buttonDelete->disable();
    buttonSaveClose = new FXButton(buttonframe, _("Save&&Close"), NULL, this, ID_SAVECLOSE, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 10,10,2,5);
    buttonCancel = new FXButton(buttonframe, _("Cancel"), NULL, this, ID_CANCEL, FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 10,10,2,5);

    UpdateList();
}

AliasDialog::~AliasDialog()
{
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
    new FXButton(buttonframe, _("OK"), NULL, &aliasEdit, FXDialogBox::ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0, 0, 0, 0, 32, 32, 5, 5);
    new FXButton(buttonframe, _("Cancel"), NULL, &aliasEdit, FXDialogBox::ID_CANCEL, FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0, 0, 0, 0, 32, 32, 5, 5);

    if(aliasEdit.execute(PLACEMENT_OWNER))
    {
        if(!alias->getText().empty() && !command->getText().empty() && alias->getText()[0]=='/' && !alias->getText().contains(' '))
        {
            aliases.insert(StringPair(alias->getText(), command->getText()));
            UpdateList();
        }
    }
    return 1;
}

long AliasDialog::OnModify(FXObject*, FXSelector, void*)
{
    FXint index = keys->getCurrentItem();
    FXString oldkey = keys->getItemText(index);
    FXDialogBox aliasEdit(this, _("Alias edit"), DECOR_TITLE|DECOR_BORDER, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    FXVerticalFrame *contents = new FXVerticalFrame(&aliasEdit, LAYOUT_SIDE_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0, 0, 0, 0, 10, 10, 10, 10, 0, 0);
    FXMatrix *matrix = new FXMatrix(contents, 2, MATRIX_BY_COLUMNS|LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(matrix, _("Alias:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *alias = new FXTextField(matrix, 25, NULL, 0, FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    alias->setText(oldkey);
    new FXLabel(matrix, _("Command:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *command = new FXTextField(matrix, 25, NULL, 0, FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    command->setText(values->getItemText(index));

    FXHorizontalFrame *buttonframe = new FXHorizontalFrame(contents, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXButton(buttonframe, _("OK"), NULL, &aliasEdit, FXDialogBox::ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0, 0, 0, 0, 32, 32, 5, 5);
    new FXButton(buttonframe, _("Cancel"), NULL, &aliasEdit, FXDialogBox::ID_CANCEL, FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0, 0, 0, 0, 32, 32, 5, 5);

    if(aliasEdit.execute(PLACEMENT_OWNER))
    {
        if(!alias->getText().empty() && !command->getText().empty() && alias->getText()[0]=='/' && !alias->getText().contains(' '))
        {
            aliases.erase(oldkey);
            aliases.insert(StringPair(alias->getText(), command->getText()));
            UpdateList();
        }
    }
    return 1;
}

long AliasDialog::OnDelete(FXObject*, FXSelector, void*)
{
    aliases.erase(keys->getItemText(keys->getCurrentItem()));
    if(!(FXint)aliases.size())
    {
        buttonModify->disable();
        buttonDelete->disable();
    }
    UpdateList();
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

long AliasDialog::OnListSelected(FXObject*, FXSelector, void *ptr)
{
    FXint i = (FXint)(FXival)ptr;
    keys->selectItem(i, true);
    values->selectItem(i, true);
    buttonModify->enable();
    buttonDelete->enable();
    return 1;
}

long AliasDialog::OnListDeselected(FXObject*, FXSelector, void*)
{
    buttonModify->disable();
    buttonDelete->disable();
    return 1;
}

long AliasDialog::OnListChanged(FXObject*, FXSelector, void *ptr)
{
    FXint i = (FXint)(FXival)ptr;
    keys->selectItem(i, true);
    values->selectItem(i, true);
    buttonModify->enable();
    buttonDelete->enable();
    return 1;
}

void AliasDialog::UpdateList()
{
    keys->clearItems();
    values->clearItems();
    StringIt it;
    for(it=aliases.begin(); it!=aliases.end(); it++)
    {
        keys->appendItem((*it).first);
        values->appendItem((*it).second);
    }
}