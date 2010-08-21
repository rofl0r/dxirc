/*
 *      dxpipe.cpp
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

#include "dxpipe.h"
#include "utils.h"
#include "config.h"
#include "i18n.h"

FXDEFMAP(dxPipe) dxPipeMap[] = {
    FXMAPFUNC(SEL_IO_READ,      dxPipe::ID_READ,     dxPipe::onIORead)
};

FXIMPLEMENT(dxPipe, FXObject, dxPipeMap, ARRAYNUMBER(dxPipeMap))

dxPipe::dxPipe(FXApp *app, FXObject *tgt)
        : m_application(app), m_target(tgt)
{
    m_running = FALSE;
}

dxPipe::~dxPipe()
{
}

long dxPipe::onIORead(FXObject*, FXSelector, void*)
{
    readData();
    return 1;
}

FXint dxPipe::execCmd(FXString cmd)
{
    m_command = cmd;
    m_strprev = "";
#ifndef WIN32
    if (pipe(m_in) == -1 || pipe(m_out) == -1)
        return -1;

    m_pid = fork();
    if (m_pid == -1)
        return -1;

    m_running = TRUE;
    m_application->addInput(m_out[0], INPUT_READ, this, ID_READ);

    if (m_pid == 0)
    {
        char *args[4];
        ::close(m_out[0]);
        ::dup2(m_out[1],STDOUT_FILENO);
        ::dup2(m_out[1],STDERR_FILENO);
        ::close(m_in[1]);
        ::dup2(m_in[0],STDIN_FILENO);
        args[0] = (char*) "sh";
        args[1] = (char*) "-c";
        args[2] = (char*) m_command.text();
        args[3] = NULL;
        execvp(args[0], args);
        _exit(1);
    }
    else
    {
        ::close(m_out[1]);
        ::close(m_in[0]);
    }
#endif
    return 1;
}

void dxPipe::stopCmd()
{
#ifndef WIN32
    if(m_running)
    {
        kill(m_pid, SIGKILL);
        FXString send = FXStringFormat(_("Command %s stopped"), m_command.text());
        m_target->handle(this, FXSEL(SEL_COMMAND, ID_PIPE), &send);
    }
    m_running = FALSE;
#endif
}

int dxPipe::readData()
{   
#ifndef WIN32
    FXchar buffer[2048];
    int size, status;

    FXString data = m_strprev;

    size = read(m_out[0], buffer, sizeof(buffer)-1);
    if (size > 0)
    {
        buffer[size] = '\0';
        if (utils::instance().isUtf8(buffer, size)) data.append(buffer);
        else data.append(utils::instance().localeToUtf8(buffer));
        if(!data.contains('\n'))
        {
            m_target->handle(this, FXSEL(SEL_COMMAND, ID_PIPE), &data);
        }
        else
        {
            while (data.contains('\n'))
            {
                FXString send = data.before('\n').before('\r');
                m_target->handle(this, FXSEL(SEL_COMMAND, ID_PIPE), &send);
                data = data.after('\n');
            }
            m_strprev = data;
        }
    }
    else if(size == 0)
    {
        waitpid(m_pid, &status, 0);
        m_running = FALSE;
        m_application->removeInput(m_out[0], INPUT_READ);
    }
    else
    {
        m_running = FALSE;
        m_application->removeInput(m_out[0], INPUT_READ);
        _exit(-1);
    }
    return size;
#else
    return 0;
#endif
}
