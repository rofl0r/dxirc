/*
 *      dxpipe.h
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

#ifndef _DXPIPE_H
#define	_DXPIPE_H

#include <fx.h>
#ifndef WIN32
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#endif

class dxPipe : public FXObject
{
    FXDECLARE(dxPipe);
public:
    dxPipe(FXApp*, FXObject*);
    virtual ~dxPipe();

    enum
    {
        ID_READ = FXMainWindow::ID_LAST+300,
        ID_PIPE,
        ID_LAST
    };

    long OnIORead(FXObject*, FXSelector, void*);
    FXint ExecCmd(FXString);
    FXbool GetRunning() { return running; }
    void StopCmd();

private:
    dxPipe() {}
    dxPipe(const dxPipe& orig);

    int ReadData();

    FXApp *application;
    FXObject *target;
    FXint in[2], out[2];
    FXint pid;
    FXString strprev, command;
    FXbool running;
};

#endif	/* _DXPIPE_H */

