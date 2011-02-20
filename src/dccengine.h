/*
 *      ircengine.h
 *
 *      Copyright 2010 David Vachulka <david@konstrukce-cad.com>
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


#ifndef DCCENGINE_H
#define DCCENGINE_H

#include <fstream>
#include "ircengine.h"

class DccEngine : public FXObject
{
    FXDECLARE(DccEngine)
public:
    DccEngine(FXApp *app, FXObject *tgt, DccFile file, IrcEngine *engine);
    virtual ~DccEngine();
    enum {
        ID_SOCKET = IrcEngine::ID_LAST,
        ID_DCC,
        ID_CTIME, //timeout for check and disconnect offered connection
        ID_PTIME, //dccfile position
        ID_LAST
    };
    
    void startConnection();
    void disconnect();
    FXbool setDccPosition(FXulong position);
    FXbool isForResume(const FXString& nick, const FXString& name, FXint port);
    FXbool isForResume(FXint token);
    DccFile getDccFile() { return m_file; }
    FXbool hasDccFile(DccFile file) { return m_file == file; }

    long onCloseTimeout(FXObject *, FXSelector, void*);
    long onPositionTimeout(FXObject*, FXSelector, void*);
    long onSocketCanRead(FXObject*, FXSelector, void*);
    long onSocketWritten(FXObject*, FXSelector, void*);
    long onSocketConnected(FXObject*, FXSelector, void*);
    long onSocketDisconnected(FXObject*, FXSelector, void*);
    long onSocketError(FXObject*, FXSelector, void*);
    long onSocketStartAccept(FXObject*, FXSelector, void*);
    long onSocketListen(FXObject*, FXSelector, void*);
private:
    DccEngine() {}

    FXApp *m_application;
    FXObject *m_target;
    DccFile m_file;
    IrcEngine *m_engine;
    dxSocket *m_socket;
    dxIrcEventArray m_events;
    FXbool m_connected;
    std::ofstream m_receivedFile;
    std::ifstream m_sentFile;
    FXbool m_positionChanged; // data arrived or sended between updateDcc
    FXTime m_lastChange; // timestamp for m_positionChanged==TRUE
    FXulong m_dataAmount; // amount of arrived or sended data
    FXint m_dccTimeout;

    void sendEvent(IrcEventType, const FXString&);
    void sendEvent(IrcEventType, const FXString&, const FXString&, const FXString&, const FXString&);
    void readData();
    void writeData();
    void closeFile();
    void updateDcc();
};

#endif // DCCENGINE_H
