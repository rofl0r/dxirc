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

    long onIORead(FXObject*, FXSelector, void*);
    FXint execCmd(FXString);
    FXbool getRunning() { return m_running; }
    void stopCmd();

private:
    dxPipe() {}
    dxPipe(const dxPipe& orig);

    int readData();

    FXApp *m_application;
    FXObject *m_target;
    FXint m_in[2], m_out[2];
    FXint m_pid;
    FXString m_strprev, m_command;
    FXbool m_running;
};

#endif	/* _DXPIPE_H */

