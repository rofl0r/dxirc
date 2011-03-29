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
    FXMAPFUNC(SEL_COMMAND,          ScriptDialog_LOAD,      ScriptDialog::onLoad),
    FXMAPFUNC(SEL_COMMAND,          ScriptDialog_VIEW,      ScriptDialog::onView),
    FXMAPFUNC(SEL_COMMAND,          ScriptDialog_VIEWNEW,   ScriptDialog::onViewNew),
    FXMAPFUNC(SEL_COMMAND,          ScriptDialog_UNLOAD,    ScriptDialog::onUnload),
    FXMAPFUNC(SEL_COMMAND,          ScriptDialog_CLOSE,     ScriptDialog::onClose),
    FXMAPFUNC(SEL_COMMAND,          ScriptDialog_LIST,      ScriptDialog::onList),
    FXMAPFUNC(SEL_KEYPRESS,         0,                      ScriptDialog::onKeyPress)
};

FXIMPLEMENT(ScriptDialog, FXDialogBox, ScriptDialogMap, ARRAYNUMBER(ScriptDialogMap))

ScriptDialog::ScriptDialog(FXMainWindow *owner)
    : FXDialogBox(owner, _("Scripts list"), DECOR_TITLE|DECOR_BORDER, 0,0,0,0, 0,0,0,0, 0,0)
{
    m_contents = new FXVerticalFrame(this, LAYOUT_SIDE_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 10,10,10,10, 0,0);

    m_scriptframe = new FXHorizontalFrame(m_contents, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    m_listframe = new FXVerticalFrame(m_scriptframe, FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    m_names = new FXList(m_listframe, this, ScriptDialog_LIST, LIST_BROWSESELECT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    m_names->setScrollStyle(HSCROLLING_OFF);

    m_group = new FXGroupBox(m_scriptframe, _("Details"), FRAME_GROOVE|LAYOUT_RIGHT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 4,4,4,4, 4,4);
    m_matrix = new FXMatrix(m_group,2,MATRIX_BY_COLUMNS|LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y);

    new FXLabel(m_matrix, _("Name:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    m_name = new FXTextField(m_matrix, 45, NULL, 0, TEXTFIELD_READONLY|FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    m_name->setBackColor(getApp()->getBaseColor());

    new FXLabel(m_matrix, _("Version:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    m_version = new FXTextField(m_matrix, 45, NULL, 0, TEXTFIELD_READONLY|TEXTFIELD_INTEGER|FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    m_version->setBackColor(getApp()->getBaseColor());

    new FXLabel(m_matrix, _("Description:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXHorizontalFrame *descriptionbox=new FXHorizontalFrame(m_matrix, LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|FRAME_SUNKEN|FRAME_THICK,0,0,0,0, 0,0,0,0);
    m_description = new FXText(descriptionbox, NULL, 0, TEXT_READONLY|TEXT_WORDWRAP|LAYOUT_FILL_X|LAYOUT_FILL_X);
    m_description->setVisibleRows(3);
    m_description->setVisibleColumns(45);
    m_description->setBackColor(getApp()->getBaseColor());

    new FXLabel(m_matrix, _("Path:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXHorizontalFrame *pathbox=new FXHorizontalFrame(m_matrix, LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|FRAME_SUNKEN|FRAME_THICK,0,0,0,0, 0,0,0,0);
    m_path = new FXText(pathbox, NULL, 0, TEXT_READONLY|TEXT_WORDWRAP|LAYOUT_FILL_X|LAYOUT_FILL_X);
    m_path->setVisibleRows(2);
    m_path->setVisibleColumns(45);
    m_path->setDelimiters("/");
    m_path->setBackColor(getApp()->getBaseColor());

    m_buttonframe = new FXHorizontalFrame(m_contents, LAYOUT_FILL_X|LAYOUT_FILL_Y);

    m_buttonClose = new dxEXButton(m_buttonframe, _("C&lose"), NULL, this, ScriptDialog_CLOSE, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,2);
    m_buttonUnload = new dxEXButton(m_buttonframe, _("&Unload"), NULL, this, ScriptDialog_UNLOAD, FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,2);
    m_buttonViewNew = new dxEXButton(m_buttonframe, _("View &script"), NULL, this, ScriptDialog_VIEWNEW, FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,2);
    m_buttonView = new dxEXButton(m_buttonframe, _("&View current"), NULL, this, ScriptDialog_VIEW, FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,2);
    m_buttonLoad = new dxEXButton(m_buttonframe, _("L&oad"), NULL, this, ScriptDialog_LOAD, FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,2);

    updateList();
    updateDetails();
}

ScriptDialog::~ScriptDialog()
{

}

long ScriptDialog::onClose(FXObject*,FXSelector,void*)
{
    getApp()->stopModal(this,TRUE);
    hide();    
    return 1;
}

long ScriptDialog::onLoad(FXObject*,FXSelector,void*)
{
    dxEXFileDialog file(this, _("Load lua script"));
    file.setPatternList(_("Lua scripts (*.lua)"));
    if(file.execute(PLACEMENT_CURSOR))
    {
        dxirc::instance()->loadLuaScript(file.getFilename());
    }
    updateList();
    updateDetails();

    return 1;
}

long ScriptDialog::onView(FXObject*,FXSelector,void*)
{
    FXint index = m_names->getCurrentItem();
    if(index == -1) return 0;
    viewFile(dxirc::instance()->getScripts()[index].path);
    return 1;
}

long ScriptDialog::onViewNew(FXObject*, FXSelector, void*)
{
    dxEXFileDialog file(this, _("Load lua script"));
    file.setPatternList(_("Lua scripts (*.lua)"));
    if(file.execute(PLACEMENT_CURSOR))
    {
        viewFile(file.getFilename());
    }
    return 1;
}

long ScriptDialog::onUnload(FXObject*,FXSelector,void*)
{
    FXint index = m_names->getCurrentItem();
    if(index == -1) return 0;
    dxirc::instance()->unloadLuaScript(dxirc::instance()->getScripts()[index].name);
    updateList();
    updateDetails();
    return 1;
}

long ScriptDialog::onKeyPress(FXObject *sender,FXSelector sel,void *ptr)
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

long ScriptDialog::onList(FXObject*,FXSelector,void*)
{
    updateDetails();

    return 1;
}

void ScriptDialog::updateList()
{
    m_names->clearItems();
    for(FXint i=0; i<dxirc::instance()->getScripts().no(); i++)
    {
        m_names->appendItem(dxirc::instance()->getScripts()[i].name);
    }
    if(dxirc::instance()->getScripts().no())
    {
        m_buttonView->enable();
        m_buttonUnload->enable();
    }
    else
    {
        m_buttonView->disable();
        m_buttonUnload->disable();
    }
}

void ScriptDialog::updateDetails()
{
    FXint index = m_names->getCurrentItem();
    if(index == -1)
    {
        m_name->setText("");
        m_version->setText("");
        m_description->setText("");
        m_path->setText("");
        return;
    }
    m_name->setText(dxirc::instance()->getScripts()[index].name);
        if(dxirc::instance()->getScripts()[index].name.length()>18) m_name->setTipText(dxirc::instance()->getScripts()[index].name);
        else m_name->setTipText("");
    m_version->setText(dxirc::instance()->getScripts()[index].version);
        if(dxirc::instance()->getScripts()[index].version.length()>18) m_version->setTipText(dxirc::instance()->getScripts()[index].version);
        else m_version->setTipText("");
    m_description->setText(dxirc::instance()->getScripts()[index].description);
        if(m_description->getNumRows()>3) m_description->setTipText(dxirc::instance()->getScripts()[index].description);
        else m_description->setTipText("");
    m_path->setText(dxirc::instance()->getScripts()[index].path);
        if(m_description->getNumRows()>2) m_path->setTipText(dxirc::instance()->getScripts()[index].path);
        else m_path->setTipText("");
    recalc();
}

void ScriptDialog::viewFile(const FXString &file)
{
    FXDialogBox dialog(this, file, MBOX_OK|DECOR_TITLE|DECOR_BORDER|DECOR_RESIZE);
    FXVerticalFrame *contents = new FXVerticalFrame(&dialog, LAYOUT_SIDE_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 10,10,10,10, 0,0);

    FXHorizontalFrame *textbox=new FXHorizontalFrame(contents,LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN|FRAME_THICK,0,0,0,0, 0,0,0,0);
    FXText *text=new FXText(textbox,NULL,0,TEXT_READONLY|TEXT_WORDWRAP|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    text->setVisibleRows(35);
    text->setVisibleColumns(90);

    new dxEXButton(contents, _("C&lose"), NULL, &dialog, FXDialogBox::ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 10,10,2,2);

    FXFile textfile(file,FXFile::Reading);
    FXint size, n, c, i, j;
    FXchar *txt;

    // Opened file?
    if(!textfile.isOpen())
    {
        dxEXMessageBox::error(this,MBOX_OK,_("Error Loading File"),_("Unable to open file: %s"),file.text());
        return;
    }

    // Get file size
    size=textfile.size();

    // Make buffer to load file
    if(!FXMALLOC(&txt,FXchar,size))
    {
        dxEXMessageBox::error(this,MBOX_OK,_("Error Loading File"),_("File is too big: %s (%d bytes)"),file.text(),size);
        return;
    }

    // Set wait cursor
    getApp()->beginWaitCursor();

    // Read the file
    n=textfile.readBlock(txt,size);
    if(n<0)
    {
        FXFREE(&txt);
        dxEXMessageBox::error(this,MBOX_OK,_("Error Loading File"),_("Unable to read file: %s"),file.text());
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

