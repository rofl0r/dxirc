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
    FXMAPFUNC(SEL_IO_READ,      dxPipe::ID_READ,     dxPipe::OnIORead)
};

FXIMPLEMENT(dxPipe, FXObject, dxPipeMap, ARRAYNUMBER(dxPipeMap))

dxPipe::dxPipe(FXApp *app, FXObject *tgt)
        : application(app), target(tgt)
{
    running = FALSE;
}

dxPipe::~dxPipe()
{
}

long dxPipe::OnIORead(FXObject*, FXSelector, void*)
{
    ReadData();
    return 1;
}

FXint dxPipe::ExecCmd(FXString cmd)
{
    command = cmd;
    strprev = "";
#ifndef WIN32
    if (pipe(in) == -1 || pipe(out) == -1)
        return -1;

    pid = fork();
    if (pid == -1)
        return -1;

    running = TRUE;
    application->addInput(out[0], INPUT_READ, this, ID_READ);

    if (pid == 0)
    {
        char *args[4];
        ::close(out[0]);
        ::dup2(out[1],STDOUT_FILENO);
        ::dup2(out[1],STDERR_FILENO);
        ::close(in[1]);
        ::dup2(in[0],STDIN_FILENO);
        args[0] = (char*) "sh";
        args[1] = (char*) "-c";
        args[2] = (char*) command.text();
        args[3] = NULL;
        execvp(args[0], args);
        _exit(1);
    }
    else
    {
        ::close(out[1]);
        ::close(in[0]);
    }
#endif
    return 1;
}

void dxPipe::StopCmd()
{
#ifndef WIN32
    if(running)
    {
        kill(pid, SIGKILL);
        FXString send = FXStringFormat(_("Command %s stopped"), command.text());
        target->handle(this, FXSEL(SEL_COMMAND, ID_PIPE), &send);
    }
    running = FALSE;
#endif
}

int dxPipe::ReadData()
{   
#ifndef WIN32
    FXchar buffer[2048];
    int size, status;

    FXString data = strprev;

    size = read(out[0], buffer, sizeof(buffer)-1);
    if (size > 0)
    {
        buffer[size] = '\0';
        if (utils::IsUtf8(buffer, size)) data.append(buffer);
        else data.append(utils::LocaleToUtf8(buffer));
        if(!data.contains('\n'))
        {
            target->handle(this, FXSEL(SEL_COMMAND, ID_PIPE), &data);
        }
        else
        {
            while (data.contains('\n'))
            {
                FXString send = data.before('\n').before('\r');
                target->handle(this, FXSEL(SEL_COMMAND, ID_PIPE), &send);
                data = data.after('\n');
            }
            strprev = data;
        }
    }
    else if(size == 0)
    {
        waitpid(pid, &status, 0);
        running = FALSE;
        application->removeInput(out[0], INPUT_READ);
    }
    else
    {
        running = FALSE;
        application->removeInput(out[0], INPUT_READ);
        _exit(-1);
    }
    return size;
#else
    return 0;
#endif
}
