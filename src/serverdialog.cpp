/*
 *      serverdialog.cpp
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

#include <fxkeys.h>

#include "serverdialog.h"
#include "config.h"
#include "i18n.h"

FXDEFMAP(ServerDialog) ServerDialogMap[] = {
    FXMAPFUNC(SEL_COMMAND,          ServerDialog::ID_JOIN,      ServerDialog::OnJoin),
    FXMAPFUNC(SEL_COMMAND,          ServerDialog::ID_ADD,       ServerDialog::OnAdd),
    FXMAPFUNC(SEL_COMMAND,          ServerDialog::ID_MODIFY,    ServerDialog::OnModify),
    FXMAPFUNC(SEL_COMMAND,          ServerDialog::ID_DELETE,    ServerDialog::OnDelete),
    FXMAPFUNC(SEL_COMMAND,          ServerDialog::ID_CANCEL,    ServerDialog::OnCancel),
    FXMAPFUNC(SEL_CLOSE,            0,                          ServerDialog::OnCancel),
    FXMAPFUNC(SEL_COMMAND,          ServerDialog::ID_SAVECLOSE, ServerDialog::OnSaveClose),
    FXMAPFUNC(SEL_COMMAND,          ServerDialog::ID_LIST,      ServerDialog::OnListSelected),
    FXMAPFUNC(SEL_DESELECTED,       ServerDialog::ID_LIST,      ServerDialog::OnListDeselected),
    FXMAPFUNC(SEL_DOUBLECLICKED,    ServerDialog::ID_LIST,      ServerDialog::OnDoubleClick),
    FXMAPFUNC(SEL_KEYPRESS,         0,                          ServerDialog::OnKeyPress),
};

FXIMPLEMENT(ServerDialog, FXDialogBox, ServerDialogMap, ARRAYNUMBER(ServerDialogMap))

ServerDialog::ServerDialog(FXMainWindow *owner, FXServerInfoArray servers)
    : FXDialogBox(owner, _("Servers list"), DECOR_TITLE|DECOR_BORDER, 0,0,0,0, 0,0,0,0, 0,0), serverList(servers), indexJoin(-1)
{
    contents = new FXVerticalFrame(this, LAYOUT_SIDE_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 10,10,10,10, 0,0);

    serverframe = new FXHorizontalFrame(contents, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    listframe = new FXVerticalFrame(serverframe, FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    names = new FXList(listframe, this, ID_LIST, LIST_SINGLESELECT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    names->setScrollStyle(HSCROLLING_OFF);
    UpdateList();

    group = new FXGroupBox(serverframe, _("Details"), FRAME_GROOVE|LAYOUT_RIGHT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 4,4,4,4, 4,4);
    matrix = new FXMatrix(group,2,MATRIX_BY_COLUMNS|LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y);

    new FXLabel(matrix, _("Hostname:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    hostname = new FXTextField(matrix, 25, NULL, 0, TEXTFIELD_READONLY|FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);

    new FXLabel(matrix, _("Port:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    port = new FXTextField(matrix, 25, NULL, 0, TEXTFIELD_READONLY|TEXTFIELD_INTEGER|FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);

    new FXLabel(matrix, _("Password:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    passwd = new FXTextField(matrix, 25, NULL, 0, TEXTFIELD_READONLY|TEXTFIELD_PASSWD|FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);

    new FXLabel(matrix, _("Nickname:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    nick = new FXTextField(matrix, 25, NULL, 0, TEXTFIELD_READONLY|FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);

    new FXLabel(matrix, _("Realname:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    realname = new FXTextField(matrix, 25, NULL, 0, TEXTFIELD_READONLY|FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);

    new FXLabel(matrix, _("Channel(s):"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    channels = new FXTextField(matrix, 25, NULL, 0, TEXTFIELD_READONLY|FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);

    new FXLabel(matrix, _("Commands on connection:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXHorizontalFrame *commandsbox=new FXHorizontalFrame(matrix, LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|FRAME_SUNKEN|FRAME_THICK,0,0,0,0, 0,0,0,0);
    commands = new FXText(commandsbox, NULL, 0, TEXT_READONLY|TEXT_WORDWRAP|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    commands->setVisibleRows(4);
    commands->setVisibleColumns(25);

    new FXLabel(matrix, _("Auto connect:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    buttonAuto = new FXCheckButton(matrix, "", NULL, 0);
    buttonAuto->disable();

    buttonframe = new FXHorizontalFrame(contents, LAYOUT_FILL_X|LAYOUT_FILL_Y);

    buttonJoin = new FXButton(buttonframe, _("Join"), NULL, this, ID_JOIN, BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 10,10,2,5);
    serverList.no()? buttonJoin->enable() : buttonJoin->disable();

    buttonAdd = new FXButton(buttonframe, _("Add"), NULL, this, ID_ADD, BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 10,10,2,5);

    buttonModify = new FXButton(buttonframe, _("Modify"), NULL, this, ID_MODIFY, BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 10,10,2,5);
    serverList.no()? buttonModify->enable() : buttonModify->disable();

    buttonDelete = new FXButton(buttonframe, _("Delete"), NULL, this, ID_DELETE, BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 10,10,2,5);
    serverList.no()? buttonDelete->enable() : buttonDelete->disable();

    buttonSaveClose = new FXButton(buttonframe, _("Save&&Close"), NULL, this, ID_SAVECLOSE, BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 10,10,2,5);

    buttonCancel = new FXButton(buttonframe, _("Cancel"), NULL, this, ID_CANCEL, BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 10,10,2,5);

    UpdateDetails();
}

ServerDialog::~ServerDialog()
{

}

long ServerDialog::OnJoin(FXObject*,FXSelector,void*)
{
    getApp()->stopModal(this,TRUE);
    hide();
    indexJoin = names->getCurrentItem();
    return 1;
}

long ServerDialog::OnSaveClose(FXObject*,FXSelector,void*)
{
    getApp()->stopModal(this,TRUE);
    hide();
    indexJoin = -1;
    return 1;
}

long ServerDialog::OnAdd(FXObject*,FXSelector,void*)
{
    FXDialogBox serverEdit(this, _("Server edit"), DECOR_TITLE|DECOR_BORDER, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    FXVerticalFrame *contents = new FXVerticalFrame(&serverEdit, LAYOUT_SIDE_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0, 0, 0, 0, 10, 10, 10, 10, 0, 0);
    FXMatrix *matrix = new FXMatrix(contents, 2, MATRIX_BY_COLUMNS|LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y);

    new FXLabel(matrix, _("Hostname:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *hostname = new FXTextField(matrix, 25, NULL, 0, FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    hostname->setText("localhost");

    new FXLabel(matrix, _("Port:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXSpinner *port = new FXSpinner(matrix, 23, NULL, 0, FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    port->setRange(0, 65536);
    port->setValue(6667);

    new FXLabel(matrix, _("Password:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *passwd = new FXTextField(matrix, 25, NULL, 0, FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);

    new FXLabel(matrix, _("Nickname:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *nick = new FXTextField(matrix, 25, NULL, 0, FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    nick->setText(FXSystem::currentUserName());

    new FXLabel(matrix, _("Realname:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *realname = new FXTextField(matrix, 25, NULL, 0, FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);

    new FXLabel(matrix, _("Channel(s):\tChannels need to be comma separated"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *channel = new FXTextField(matrix, 25, NULL, 0, FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    channel->setText("#");

    new FXLabel(matrix, _("Commands on connection:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXHorizontalFrame *commandsbox=new FXHorizontalFrame(matrix, LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|FRAME_SUNKEN|FRAME_THICK,0,0,0,0, 0,0,0,0);
    FXText *command = new FXText(commandsbox, NULL, 0, TEXT_WORDWRAP|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    command->setVisibleRows(4);
    command->setVisibleColumns(25);

    new FXLabel(matrix, _("Auto connect:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXCheckButton *buttonAuto = new FXCheckButton(matrix, "", NULL, 0);

    FXHorizontalFrame *buttonframe = new FXHorizontalFrame(contents, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXButton(buttonframe, _("OK"), NULL, &serverEdit, FXDialogBox::ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0, 0, 0, 0, 32, 32, 5, 5);
    new FXButton(buttonframe, _("Cancel"), NULL, &serverEdit, FXDialogBox::ID_CANCEL, BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0, 0, 0, 0, 32, 32, 5, 5);

    if(serverEdit.execute(PLACEMENT_OWNER))
    {
        if(!hostname->getText().empty() && !nick->getText().empty() && !HostnameExist(hostname->getText()))
        {
            ServerInfo server;
            server.hostname = hostname->getText();
            server.port = port->getValue();
            server.nick = nick->getText();
            realname->getText().length() ? server.realname = realname->getText() : server.realname = nick->getText();
            server.passwd = passwd->getText();
            (channel->getText().length()>1) ? server.channels = channel->getText() : server.channels = "";
            (command->getText().length()) ? server.commands = command->getText() : server.commands = "";
            server.autoConnect = buttonAuto->getCheck();
            serverList.append(server);
        }
    }

    UpdateList();
    UpdateDetails();

    return 1;
}

long ServerDialog::OnModify(FXObject*,FXSelector,void*)
{
    FXint index = names->getCurrentItem();
    FXString oldhostname = serverList[index].hostname;

    FXDialogBox serverEdit(this, _("Server edit"), DECOR_TITLE|DECOR_BORDER, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    FXVerticalFrame *contents = new FXVerticalFrame(&serverEdit, LAYOUT_SIDE_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0, 0, 0, 0, 10, 10, 10, 10, 0, 0);
    FXMatrix *matrix = new FXMatrix(contents, 2, MATRIX_BY_COLUMNS|LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y);

    new FXLabel(matrix, _("Hostname:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *hostname = new FXTextField(matrix, 25, NULL, 0, FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    hostname->setText(serverList[index].hostname);

    new FXLabel(matrix, _("Port:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXSpinner *port = new FXSpinner(matrix, 23, NULL, 0, FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    port->setRange(0, 65536);
    port->setValue(serverList[index].port);

    new FXLabel(matrix, _("Password:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *passwd = new FXTextField(matrix, 25, NULL, 0, FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    passwd->setText(serverList[index].passwd);

    new FXLabel(matrix, _("Nickname:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *nick = new FXTextField(matrix, 25, NULL, 0, FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    nick->setText(serverList[index].nick);

    new FXLabel(matrix, _("Realname:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *realname = new FXTextField(matrix, 25, NULL, 0, FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    realname->setText(serverList[index].realname);

    new FXLabel(matrix, _("Channel(s):\tChannels need to be comma separated"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *channel = new FXTextField(matrix, 25, NULL, 0, FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    channel->setText(serverList[index].channels);

    new FXLabel(matrix, _("Commands on connection:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXHorizontalFrame *commandsbox=new FXHorizontalFrame(matrix, LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|FRAME_SUNKEN|FRAME_THICK,0,0,0,0, 0,0,0,0);
    FXText *command = new FXText(commandsbox, NULL, 0, TEXT_WORDWRAP|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    command->setVisibleRows(4);
    command->setVisibleColumns(25);
    command->setText(serverList[index].commands);

    new FXLabel(matrix, _("Auto connect:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXCheckButton *buttonAuto = new FXCheckButton(matrix, "", NULL, 0);
    buttonAuto->setCheck(serverList[index].autoConnect);

    FXHorizontalFrame *buttonframe = new FXHorizontalFrame(contents, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXButton(buttonframe, _("OK"), NULL, &serverEdit, FXDialogBox::ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0, 0, 0, 0, 32, 32, 5, 5);
    new FXButton(buttonframe, _("Cancel"), NULL, &serverEdit, FXDialogBox::ID_CANCEL, BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0, 0, 0, 0, 32, 32, 5, 5);

    if(serverEdit.execute(PLACEMENT_OWNER))
    {
        if(!hostname->getText().empty() && !nick->getText().empty() && (!HostnameExist(hostname->getText()) || oldhostname==hostname->getText()))
        {
            ServerInfo server;
            server.hostname = hostname->getText();
            server.port = port->getValue();
            server.nick = nick->getText();
            realname->getText().length() ? server.realname = realname->getText() : server.realname = nick->getText();
            server.passwd = passwd->getText();
            (channel->getText().length()>1) ? server.channels = channel->getText() : server.channels = "";
            (command->getText().length()) ? server.commands = command->getText() : server.commands = "";
            server.autoConnect = buttonAuto->getCheck();
            serverList[index] = server;
        }
    }

    UpdateList();
    UpdateDetails();

    return 1;
}

long ServerDialog::OnDelete(FXObject*,FXSelector,void*)
{
    FXint index = names->getCurrentItem();
    serverList.erase(index);
    UpdateList();
    UpdateDetails();
    return 1;
}

long ServerDialog::OnCancel(FXObject*,FXSelector,void*)
{
    getApp()->stopModal(this,FALSE);
    hide();
    return 1;
}

long ServerDialog::OnKeyPress(FXObject *sender,FXSelector sel,void *ptr)
{
    if(FXTopWindow::onKeyPress(sender,sel,ptr)) return 1;
    if(((FXEvent*)ptr)->code == KEY_Escape)
    {
        handle(this,FXSEL(SEL_COMMAND,ID_CANCEL),NULL);
        return 1;
    }
    return 0;
}

long ServerDialog::OnListSelected(FXObject*,FXSelector,void*)
{
    UpdateDetails();

    buttonJoin->enable();
    buttonModify->enable();
    buttonDelete->enable();

    return 1;
}

long ServerDialog::OnListDeselected(FXObject*,FXSelector,void*)
{
    UpdateDetails();

    buttonJoin->disable();
    buttonModify->disable();
    buttonDelete->disable();

    return 1;
}

long ServerDialog::OnDoubleClick(FXObject*, FXSelector, void *ptr)
{
    FXint index = (FXint)(FXival)ptr;
    if(0 <= index)
    {
        getApp()->stopModal(this,TRUE);
        hide();
        indexJoin = index;
    }
    return 1;
}

FXbool ServerDialog::HostnameExist(const FXString &name)
{
    for(FXint i=0; i < serverList.no(); i++)
    {
        if(serverList[i].hostname == name) return true;
    }
    return false;
}

void ServerDialog::UpdateList()
{
    names->clearItems();
    for(FXint i =0; i < serverList.no(); i++)
    {
        names->appendItem(serverList[i].hostname);
    }
}

void ServerDialog::UpdateDetails()
{
    if(names->getNumItems())
    {
        FXint index = names->getCurrentItem();
        hostname->setText(serverList[index].hostname);
            if(serverList[index].channels.length()>18) hostname->setTipText(serverList[index].hostname);
            else hostname->setTipText("");
        port->setText(FXStringVal(serverList[index].port));
            if(FXStringVal(serverList[index].port).length()>18) port->setTipText(FXStringVal(serverList[index].port));
            else port->setTipText("");
        nick->setText(serverList[index].nick);
            if(serverList[index].nick.length()>18) nick->setTipText(serverList[index].nick);
            else nick->setTipText("");
        realname->setText(serverList[index].realname);
            if(serverList[index].realname.length()>18) realname->setTipText(serverList[index].realname);
            else realname->setTipText("");
        passwd->setText(serverList[index].passwd);
            if(serverList[index].passwd.length()>18) passwd->setTipText(serverList[index].passwd);
            else passwd->setTipText("");
        channels->setText(serverList[index].channels);
            if(serverList[index].channels.length()>18) channels->setTipText(serverList[index].channels);
            else channels->setTipText("");
        commands->setText(serverList[index].commands);
            if(commands->getNumRows()>4) commands->setTipText(serverList[index].commands);
            else commands->setTipText("");
        buttonAuto->setCheck(serverList[index].autoConnect);
    }
    else
    {
        hostname->setText("");
        port->setText("");
        nick->setText("");
        realname->setText("");
        passwd->setText("");
        channels->setText("");
        buttonAuto->setCheck(false);
        buttonJoin->disable();
        buttonModify->disable();
        buttonDelete->disable();
    }
}

