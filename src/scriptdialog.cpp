/*
 *      scriptdialog.cpp
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

#include "scriptdialog.h"
#include "config.h"
#include "i18n.h"
#include "dxirc.h"

FXDEFMAP(ScriptDialog) ScriptDialogMap[] = {
    FXMAPFUNC(SEL_COMMAND,          ScriptDialog::ID_LOAD,      ScriptDialog::OnLoad),
    FXMAPFUNC(SEL_COMMAND,          ScriptDialog::ID_VIEW,      ScriptDialog::OnView),
    FXMAPFUNC(SEL_COMMAND,          ScriptDialog::ID_VIEWNEW,   ScriptDialog::OnViewNew),
    FXMAPFUNC(SEL_COMMAND,          ScriptDialog::ID_UNLOAD,    ScriptDialog::OnUnload),
    FXMAPFUNC(SEL_COMMAND,          ScriptDialog::ID_CLOSE,     ScriptDialog::OnClose),
    FXMAPFUNC(SEL_COMMAND,          ScriptDialog::ID_LIST,      ScriptDialog::OnList),
    FXMAPFUNC(SEL_KEYPRESS,         0,                          ScriptDialog::OnKeyPress)
};

FXIMPLEMENT(ScriptDialog, FXDialogBox, ScriptDialogMap, ARRAYNUMBER(ScriptDialogMap))

ScriptDialog::ScriptDialog(dxirc *owner)
    : FXDialogBox(owner, _("Scripts list"), DECOR_TITLE|DECOR_BORDER, 0,0,0,0, 0,0,0,0, 0,0), irc(owner)
{
    contents = new FXVerticalFrame(this, LAYOUT_SIDE_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 10,10,10,10, 0,0);

    scriptframe = new FXHorizontalFrame(contents, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    listframe = new FXVerticalFrame(scriptframe, FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    names = new FXList(listframe, this, ID_LIST, LIST_BROWSESELECT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    names->setScrollStyle(HSCROLLING_OFF);    

    group = new FXGroupBox(scriptframe, _("Details"), FRAME_GROOVE|LAYOUT_RIGHT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 4,4,4,4, 4,4);
    matrix = new FXMatrix(group,2,MATRIX_BY_COLUMNS|LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y);

    new FXLabel(matrix, _("Name:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    name = new FXTextField(matrix, 45, NULL, 0, TEXTFIELD_READONLY|FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    name->setBackColor(getApp()->getBaseColor());

    new FXLabel(matrix, _("Version:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    version = new FXTextField(matrix, 45, NULL, 0, TEXTFIELD_READONLY|TEXTFIELD_INTEGER|FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    version->setBackColor(getApp()->getBaseColor());

    new FXLabel(matrix, _("Description:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXHorizontalFrame *descriptionbox=new FXHorizontalFrame(matrix, LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|FRAME_SUNKEN|FRAME_THICK,0,0,0,0, 0,0,0,0);
    description = new FXText(descriptionbox, NULL, 0, TEXT_READONLY|TEXT_WORDWRAP|LAYOUT_FILL_X|LAYOUT_FILL_X);
    description->setVisibleRows(3);
    description->setVisibleColumns(45);
    description->setBackColor(getApp()->getBaseColor());

    new FXLabel(matrix, _("Path:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXHorizontalFrame *pathbox=new FXHorizontalFrame(matrix, LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|FRAME_SUNKEN|FRAME_THICK,0,0,0,0, 0,0,0,0);
    path = new FXText(pathbox, NULL, 0, TEXT_READONLY|TEXT_WORDWRAP|LAYOUT_FILL_X|LAYOUT_FILL_X);
    path->setVisibleRows(2);
    path->setVisibleColumns(45);
    path->setDelimiters("/");
    path->setBackColor(getApp()->getBaseColor());

    buttonframe = new FXHorizontalFrame(contents, LAYOUT_FILL_X|LAYOUT_FILL_Y);    

    buttonClose = new FXButton(buttonframe, _("C&lose"), NULL, this, ID_CLOSE, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,5);
    buttonUnload = new FXButton(buttonframe, _("&Unload"), NULL, this, ID_UNLOAD, FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,5);
    buttonViewNew = new FXButton(buttonframe, _("View &script"), NULL, this, ID_VIEWNEW, FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,5);
    buttonView = new FXButton(buttonframe, _("&View current"), NULL, this, ID_VIEW, FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,5);
    buttonLoad = new FXButton(buttonframe, _("&Load"), NULL, this, ID_LOAD, FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,5);

    UpdateList();
    UpdateDetails();
}

ScriptDialog::~ScriptDialog()
{

}

long ScriptDialog::OnClose(FXObject*,FXSelector,void*)
{
    getApp()->stopModal(this,TRUE);
    hide();    
    return 1;
}

long ScriptDialog::OnLoad(FXObject*,FXSelector,void*)
{
    FXFileDialog file(this, _("Load lua script"));
    file.setPatternList(_("Lua scripts (*.lua)"));
    if(file.execute(PLACEMENT_CURSOR))
    {
        irc->LoadLuaScript(file.getFilename());
    }
    UpdateList();
    UpdateDetails();

    return 1;
}

long ScriptDialog::OnView(FXObject*,FXSelector,void*)
{
    FXint index = names->getCurrentItem();
    if(index == -1) return 0;
    ViewFile(irc->scripts[index].path);
    return 1;
}

long ScriptDialog::OnViewNew(FXObject*, FXSelector, void*)
{
    FXFileDialog file(this, _("Load lua script"));
    file.setPatternList(_("Lua scripts (*.lua)"));
    if(file.execute(PLACEMENT_CURSOR))
    {
        ViewFile(file.getFilename());
    }
    return 1;
}

long ScriptDialog::OnUnload(FXObject*,FXSelector,void*)
{
    FXint index = names->getCurrentItem();
    if(index == -1) return 0;
    irc->UnloadLuaScript(irc->scripts[index].name);
    UpdateList();
    UpdateDetails();
    return 1;
}

long ScriptDialog::OnKeyPress(FXObject *sender,FXSelector sel,void *ptr)
{
    if(FXTopWindow::onKeyPress(sender,sel,ptr)) return 1;
    if(((FXEvent*)ptr)->code == KEY_Escape)
    {
        getApp()->stopModal(this, FALSE);
        hide();
        return 1;
    }
    return 0;
}

long ScriptDialog::OnList(FXObject*,FXSelector,void*)
{
    UpdateDetails();

    return 1;
}

void ScriptDialog::UpdateList()
{
    names->clearItems();
    for(FXint i=0; i<irc->scripts.no(); i++)
    {
        names->appendItem(irc->scripts[i].name);
    }
    if(irc->scripts.no())
    {
        buttonView->enable();
        buttonUnload->enable();
    }
    else
    {
        buttonView->disable();
        buttonUnload->disable();
    }
}

void ScriptDialog::UpdateDetails()
{
    FXint index = names->getCurrentItem();
    if(index == -1)
    {
        name->setText("");
        version->setText("");
        description->setText("");
        path->setText("");
        return;
    }
    name->setText(irc->scripts[index].name);
        if(irc->scripts[index].name.length()>18) name->setTipText(irc->scripts[index].name);
        else name->setTipText("");
    version->setText(irc->scripts[index].version);
        if(irc->scripts[index].version.length()>18) version->setTipText(irc->scripts[index].version);
        else version->setTipText("");
    description->setText(irc->scripts[index].description);
        if(description->getNumRows()>3) description->setTipText(irc->scripts[index].description);
        else description->setTipText("");
    path->setText(irc->scripts[index].path);
        if(description->getNumRows()>2) path->setTipText(irc->scripts[index].path);
        else path->setTipText("");
    recalc();
}

void ScriptDialog::ViewFile(const FXString &file)
{
    FXDialogBox dialog(this, file, MBOX_OK|DECOR_TITLE|DECOR_BORDER|DECOR_RESIZE);
    FXVerticalFrame *contents = new FXVerticalFrame(&dialog, LAYOUT_SIDE_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 10,10,10,10, 0,0);

    FXHorizontalFrame *textbox=new FXHorizontalFrame(contents,LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN|FRAME_THICK,0,0,0,0, 0,0,0,0);
    FXText *text=new FXText(textbox,NULL,0,TEXT_READONLY|TEXT_WORDWRAP|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    text->setVisibleRows(35);
    text->setVisibleColumns(90);

    new FXButton(contents, _("C&lose"), NULL, &dialog, FXDialogBox::ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 10,10,2,5);

    FXFile textfile(file,FXFile::Reading);
    FXint size, n, c, i, j;
    FXchar *txt;

    // Opened file?
    if(!textfile.isOpen())
    {
        FXMessageBox::error(this,MBOX_OK,_("Error Loading File"),_("Unable to open file: %s"),file.text());
        return;
    }

    // Get file size
    size=textfile.size();

    // Make buffer to load file
    if(!FXMALLOC(&txt,FXchar,size))
    {
        FXMessageBox::error(this,MBOX_OK,_("Error Loading File"),_("File is too big: %s (%d bytes)"),file.text(),size);
        return;
    }

    // Set wait cursor
    getApp()->beginWaitCursor();

    // Read the file
    n=textfile.readBlock(txt,size);
    if(n<0)
    {
        FXFREE(&txt);
        FXMessageBox::error(this,MBOX_OK,_("Error Loading File"),_("Unable to read file: %s"),file.text());
        getApp()->endWaitCursor();
        return;
    }

    // Strip carriage returns
    for(i=j=0; j<n; j++)
    {
        c=txt[j];
        if(c!='\r')
            txt[i++]=c;
    }
    n=i;

    // Set text
    text->setText(txt,n);
    FXFREE(&txt);

    // Kill wait cursor
    getApp()->endWaitCursor();

    dialog.execute(PLACEMENT_CURSOR);
}

