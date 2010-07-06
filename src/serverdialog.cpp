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

#include "serverdialog.h"
#include "config.h"
#include "i18n.h"

FXDEFMAP(ServerDialog) ServerDialogMap[] = {
    FXMAPFUNC(SEL_COMMAND,          ServerDialog::ID_JOIN,      ServerDialog::onJoin),
    FXMAPFUNC(SEL_COMMAND,          ServerDialog::ID_ADD,       ServerDialog::onAdd),
    FXMAPFUNC(SEL_COMMAND,          ServerDialog::ID_MODIFY,    ServerDialog::onModify),
    FXMAPFUNC(SEL_COMMAND,          ServerDialog::ID_DELETE,    ServerDialog::onDelete),
    FXMAPFUNC(SEL_COMMAND,          ServerDialog::ID_CANCEL,    ServerDialog::onCancel),
    FXMAPFUNC(SEL_CLOSE,            0,                          ServerDialog::onCancel),
    FXMAPFUNC(SEL_COMMAND,          ServerDialog::ID_SAVECLOSE, ServerDialog::onSaveClose),
    FXMAPFUNC(SEL_COMMAND,          ServerDialog::ID_LIST,      ServerDialog::onList),
    FXMAPFUNC(SEL_DOUBLECLICKED,    ServerDialog::ID_LIST,      ServerDialog::onDoubleClick),
    FXMAPFUNC(SEL_KEYPRESS,         0,                          ServerDialog::onKeyPress),
};

FXIMPLEMENT(ServerDialog, FXDialogBox, ServerDialogMap, ARRAYNUMBER(ServerDialogMap))

ServerDialog::ServerDialog(FXMainWindow *owner, dxServerInfoArray servers)
    : FXDialogBox(owner, _("Servers list"), DECOR_TITLE|DECOR_BORDER, 0,0,0,0, 0,0,0,0, 0,0), m_serverList(servers), m_indexJoin(-1)
{
    m_contents = new FXVerticalFrame(this, LAYOUT_SIDE_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 10,10,10,10, 0,0);

    m_serverframe = new FXHorizontalFrame(m_contents, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    m_listframe = new FXVerticalFrame(m_serverframe, FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    m_names = new FXList(m_listframe, this, ID_LIST, LIST_BROWSESELECT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    m_names->setScrollStyle(HSCROLLING_OFF);
    updateList();

    m_group = new FXGroupBox(m_serverframe, _("Details"), FRAME_GROOVE|LAYOUT_RIGHT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 4,4,4,4, 4,4);
    m_matrix = new FXMatrix(m_group,2,MATRIX_BY_COLUMNS|LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y);

    new FXLabel(m_matrix, _("Hostname:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    m_hostname = new FXTextField(m_matrix, 25, NULL, 0, TEXTFIELD_READONLY|FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    m_hostname->setBackColor(getApp()->getBaseColor());

    new FXLabel(m_matrix, _("Port:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    m_port = new FXTextField(m_matrix, 25, NULL, 0, TEXTFIELD_READONLY|TEXTFIELD_INTEGER|FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    m_port->setBackColor(getApp()->getBaseColor());

    new FXLabel(m_matrix, _("Password:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    m_passwd = new FXTextField(m_matrix, 25, NULL, 0, TEXTFIELD_READONLY|TEXTFIELD_PASSWD|FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    m_passwd->setBackColor(getApp()->getBaseColor());

    new FXLabel(m_matrix, _("Nickname:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    m_nick = new FXTextField(m_matrix, 25, NULL, 0, TEXTFIELD_READONLY|FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    m_nick->setBackColor(getApp()->getBaseColor());

    new FXLabel(m_matrix, _("Realname:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    m_realname = new FXTextField(m_matrix, 25, NULL, 0, TEXTFIELD_READONLY|FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    m_realname->setBackColor(getApp()->getBaseColor());

    new FXLabel(m_matrix, _("Channel(s):"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXHorizontalFrame *channelsbox=new FXHorizontalFrame(m_matrix, LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|FRAME_SUNKEN|FRAME_THICK,0,0,0,0, 0,0,0,0);
    m_channels = new FXText(channelsbox, NULL, 0, TEXT_READONLY|TEXT_WORDWRAP|LAYOUT_FILL_X|LAYOUT_FILL_X);
    m_channels->setVisibleRows(3);
    m_channels->setVisibleColumns(25);
    m_channels->setBackColor(getApp()->getBaseColor());

    new FXLabel(m_matrix, _("Commands on connection:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXHorizontalFrame *commandsbox=new FXHorizontalFrame(m_matrix, LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|FRAME_SUNKEN|FRAME_THICK,0,0,0,0, 0,0,0,0);
    m_commands = new FXText(commandsbox, NULL, 0, TEXT_READONLY|TEXT_WORDWRAP|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    m_commands->setVisibleRows(4);
    m_commands->setVisibleColumns(25);
    m_commands->setBackColor(getApp()->getBaseColor());

#ifdef HAVE_OPENSSL
    new FXLabel(m_matrix, _("Use SSL:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    m_usessl = new FXTextField(m_matrix, 25, NULL, 0, TEXTFIELD_READONLY|FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    m_usessl->setBackColor(getApp()->getBaseColor());
#endif

    new FXLabel(m_matrix, _("Auto connect:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    m_autoconnect = new FXTextField(m_matrix, 25, NULL, 0, TEXTFIELD_READONLY|FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    m_autoconnect->setBackColor(getApp()->getBaseColor());

    m_buttonframe = new FXHorizontalFrame(m_contents, LAYOUT_FILL_X|LAYOUT_FILL_Y);

    m_buttonCancel = new FXButton(m_buttonframe, _("&Cancel"), NULL, this, ID_CANCEL, FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,5);
    m_buttonSaveClose = new FXButton(m_buttonframe, _("&Save&&Close"), NULL, this, ID_SAVECLOSE, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,5);
    m_buttonDelete = new FXButton(m_buttonframe, _("&Delete"), NULL, this, ID_DELETE, FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,5);
    m_serverList.no()? m_buttonDelete->enable() : m_buttonDelete->disable();
    m_buttonModify = new FXButton(m_buttonframe, _("&Modify"), NULL, this, ID_MODIFY, FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,5);
    m_serverList.no()? m_buttonModify->enable() : m_buttonModify->disable();
    m_buttonAdd = new FXButton(m_buttonframe, _("&Add"), NULL, this, ID_ADD, FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,5);
    m_buttonJoin = new FXButton(m_buttonframe, _("&Join"), NULL, this, ID_JOIN, FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0,0,0,0, 10,10,2,5);
    m_serverList.no()? m_buttonJoin->enable() : m_buttonJoin->disable();

    updateDetails();
}

ServerDialog::~ServerDialog()
{

}

long ServerDialog::onJoin(FXObject*,FXSelector,void*)
{
    getApp()->stopModal(this,TRUE);
    hide();
    m_indexJoin = m_names->getCurrentItem();
    return 1;
}

long ServerDialog::onSaveClose(FXObject*,FXSelector,void*)
{
    getApp()->stopModal(this,TRUE);
    hide();
    m_indexJoin = -1;
    return 1;
}

long ServerDialog::onAdd(FXObject*,FXSelector,void*)
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
    channel->setTipText(_("Channels need to be comma separated"));

    new FXLabel(matrix, _("Commands on connection:\tOne command on one line"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXHorizontalFrame *commandsbox=new FXHorizontalFrame(matrix, LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|FRAME_SUNKEN|FRAME_THICK,0,0,0,0, 0,0,0,0);
    FXText *command = new FXText(commandsbox, NULL, 0, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    command->setVisibleRows(4);
    command->setVisibleColumns(25);
    command->setTipText(_("One command on one line"));

#ifdef HAVE_OPENSSL
    new FXLabel(matrix, _("Use SSL:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXCheckButton *buttonSsl = new FXCheckButton(matrix, "", NULL, 0);
#endif

    new FXLabel(matrix, _("Auto connect:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXCheckButton *buttonAuto = new FXCheckButton(matrix, "", NULL, 0);

    FXHorizontalFrame *buttonframe = new FXHorizontalFrame(contents, LAYOUT_FILL_X|LAYOUT_FILL_Y|PACK_UNIFORM_WIDTH);
    new FXButton(buttonframe, _("&Cancel"), NULL, &serverEdit, FXDialogBox::ID_CANCEL, FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0, 0, 0, 0, 10, 10, 2, 5);
    new FXButton(buttonframe, _("&OK"), NULL, &serverEdit, FXDialogBox::ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0, 0, 0, 0, 10, 10, 2, 5);

    if(serverEdit.execute(PLACEMENT_OWNER))
    {
        if(!hostname->getText().empty() && !nick->getText().empty() && !hostnameExist(hostname->getText(), port->getValue(), nick->getText()))
        {
            ServerInfo server;
            server.hostname = hostname->getText();
            server.port = port->getValue();
            server.nick = nick->getText();
            realname->getText().length() ? server.realname = realname->getText() : server.realname = nick->getText();
            server.passwd = passwd->getText();
            (channel->getText().length()>1) ? server.channels = channel->getText() : server.channels = "";
            (command->getText().length()) ? server.commands = command->getText() : server.commands = "";
#ifdef HAVE_OPENSSL
            server.useSsl = buttonSsl->getCheck();
#else
            server.useSsl = FALSE;
#endif
            server.autoConnect = buttonAuto->getCheck();
            m_serverList.append(server);
        }
    }

    updateList();
    updateDetails();

    return 1;
}

long ServerDialog::onModify(FXObject*,FXSelector,void*)
{
    FXint index = m_names->getCurrentItem();
    FXString oldhostname = m_serverList[index].hostname;
    FXint oldport = m_serverList[index].port;
    FXString oldnick = m_serverList[index].nick;

    FXDialogBox serverEdit(this, _("Server edit"), DECOR_TITLE|DECOR_BORDER, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    FXVerticalFrame *contents = new FXVerticalFrame(&serverEdit, LAYOUT_SIDE_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0, 0, 0, 0, 10, 10, 10, 10, 0, 0);
    FXMatrix *matrix = new FXMatrix(contents, 2, MATRIX_BY_COLUMNS|LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y);

    new FXLabel(matrix, _("Hostname:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *hostname = new FXTextField(matrix, 25, NULL, 0, FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    hostname->setText(m_serverList[index].hostname);

    new FXLabel(matrix, _("Port:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXSpinner *port = new FXSpinner(matrix, 23, NULL, 0, FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    port->setRange(0, 65536);
    port->setValue(m_serverList[index].port);

    new FXLabel(matrix, _("Password:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *passwd = new FXTextField(matrix, 25, NULL, 0, FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    passwd->setText(m_serverList[index].passwd);

    new FXLabel(matrix, _("Nickname:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *nick = new FXTextField(matrix, 25, NULL, 0, FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    nick->setText(m_serverList[index].nick);

    new FXLabel(matrix, _("Realname:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *realname = new FXTextField(matrix, 25, NULL, 0, FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    realname->setText(m_serverList[index].realname);

    new FXLabel(matrix, _("Channel(s):\tChannels need to be comma separated"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXTextField *channel = new FXTextField(matrix, 25, NULL, 0, FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    channel->setText(m_serverList[index].channels);
    channel->setTipText(_("Channels need to be comma separated"));

    new FXLabel(matrix, _("Commands on connection:\tOne command on one line"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXHorizontalFrame *commandsbox=new FXHorizontalFrame(matrix, LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|FRAME_SUNKEN|FRAME_THICK,0,0,0,0, 0,0,0,0);
    FXText *command = new FXText(commandsbox, NULL, 0, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    command->setVisibleRows(4);
    command->setVisibleColumns(25);
    command->setText(m_serverList[index].commands);
    command->setTipText(_("One command on one line"));

#ifdef HAVE_OPENSSL
    new FXLabel(matrix, _("Use SSL:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXCheckButton *buttonSsl = new FXCheckButton(matrix, "", NULL, 0);
    buttonSsl->setCheck(m_serverList[index].useSsl);
#endif

    new FXLabel(matrix, _("Auto connect:"), NULL, JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
    FXCheckButton *buttonAuto = new FXCheckButton(matrix, "", NULL, 0);
    buttonAuto->setCheck(m_serverList[index].autoConnect);

    FXHorizontalFrame *buttonframe = new FXHorizontalFrame(contents, LAYOUT_FILL_X|LAYOUT_FILL_Y|PACK_UNIFORM_WIDTH);
    new FXButton(buttonframe, _("&Cancel"), NULL, &serverEdit, FXDialogBox::ID_CANCEL, FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0, 0, 0, 0, 10, 10, 2, 5);
    new FXButton(buttonframe, _("&OK"), NULL, &serverEdit, FXDialogBox::ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT, 0, 0, 0, 0, 10, 10, 2, 5);

    if(serverEdit.execute(PLACEMENT_OWNER))
    {
        if(!hostname->getText().empty() && !nick->getText().empty() && (!hostnameExist(hostname->getText(), port->getValue(), nick->getText()) || (oldhostname==hostname->getText() && oldport==port->getValue() && oldnick == nick->getText())))
        {
            ServerInfo server;
            server.hostname = hostname->getText();
            server.port = port->getValue();
            server.nick = nick->getText();
            realname->getText().length() ? server.realname = realname->getText() : server.realname = nick->getText();
            server.passwd = passwd->getText();
            (channel->getText().length()>1) ? server.channels = channel->getText() : server.channels = "";
            (command->getText().length()) ? server.commands = command->getText() : server.commands = "";
#ifdef HAVE_OPENSSL
            server.useSsl = buttonSsl->getCheck();
#else
            server.useSsl = FALSE;
#endif
            server.autoConnect = buttonAuto->getCheck();
            m_serverList[index] = server;
        }
    }

    updateList();
    updateDetails();

    return 1;
}

long ServerDialog::onDelete(FXObject*,FXSelector,void*)
{
    FXint index = m_names->getCurrentItem();
    m_serverList.erase(index);
    updateList();
    updateDetails();
    return 1;
}

long ServerDialog::onCancel(FXObject*,FXSelector,void*)
{
    getApp()->stopModal(this,FALSE);
    hide();
    return 1;
}

long ServerDialog::onKeyPress(FXObject *sender,FXSelector sel,void *ptr)
{
    if(FXTopWindow::onKeyPress(sender,sel,ptr)) return 1;
    if(((FXEvent*)ptr)->code == KEY_Escape)
    {
        handle(this,FXSEL(SEL_COMMAND,ID_CANCEL),NULL);
        return 1;
    }
    return 0;
}

long ServerDialog::onList(FXObject*,FXSelector,void*)
{
    updateDetails();

    m_buttonJoin->enable();
    m_buttonModify->enable();
    m_buttonDelete->enable();

    return 1;
}

long ServerDialog::onDoubleClick(FXObject*, FXSelector, void *ptr)
{
    FXint index = (FXint)(FXival)ptr;
    if(0 <= index)
    {
        getApp()->stopModal(this,TRUE);
        hide();
        m_indexJoin = index;
    }
    return 1;
}

FXbool ServerDialog::hostnameExist(const FXString &hostname, const FXint &port, const FXString &nick)
{
    for(FXint i=0; i < m_serverList.no(); i++)
    {
        if(m_serverList[i].hostname == hostname && m_serverList[i].port == port && m_serverList[i].nick == nick) return TRUE;
    }
    return FALSE;
}

void ServerDialog::updateList()
{
    m_names->clearItems();
    for(FXint i =0; i < m_serverList.no(); i++)
    {
        m_names->appendItem(m_serverList[i].hostname);
    }
}

void ServerDialog::updateDetails()
{
    if(m_names->getNumItems())
    {
        FXint index = m_names->getCurrentItem();
        m_hostname->setText(m_serverList[index].hostname);
            if(m_serverList[index].channels.length()>18) m_hostname->setTipText(m_serverList[index].hostname);
            else m_hostname->setTipText("");
        m_port->setText(FXStringVal(m_serverList[index].port));
            if(FXStringVal(m_serverList[index].port).length()>18) m_port->setTipText(FXStringVal(m_serverList[index].port));
            else m_port->setTipText("");
        m_nick->setText(m_serverList[index].nick);
            if(m_serverList[index].nick.length()>18) m_nick->setTipText(m_serverList[index].nick);
            else m_nick->setTipText("");
        m_realname->setText(m_serverList[index].realname);
            if(m_serverList[index].realname.length()>18) m_realname->setTipText(m_serverList[index].realname);
            else m_realname->setTipText("");
        m_passwd->setText(m_serverList[index].passwd);
            if(m_serverList[index].passwd.length()>18) m_passwd->setTipText(m_serverList[index].passwd);
            else m_passwd->setTipText("");
        m_channels->setText(m_serverList[index].channels);
            if(m_channels->getNumRows()>3) m_channels->setTipText(m_serverList[index].channels);
            else m_channels->setTipText("");
        m_commands->setText(m_serverList[index].commands);
            if(m_commands->getNumRows()>4) m_commands->setTipText(m_serverList[index].commands);
            else m_commands->setTipText("");
#ifdef HAVE_OPENSSL
        m_serverList[index].useSsl ? m_usessl->setText(_("Yes")) : m_usessl->setText(_("No"));
#endif
        m_serverList[index].autoConnect ? m_autoconnect->setText(_("Yes")) : m_autoconnect->setText(_("No"));
    }
    else
    {
        m_hostname->setText("");
        m_port->setText("");
        m_nick->setText("");
        m_realname->setText("");
        m_passwd->setText("");
        m_channels->setText("");
        m_commands->setText("");
#ifdef HAVE_OPENSSL
        m_usessl->setText("");
#endif
        m_autoconnect->setText("");
        m_buttonJoin->disable();
        m_buttonModify->disable();
        m_buttonDelete->disable();
    }
}

