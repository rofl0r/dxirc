/*
 *      dxsocket.h
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


#ifndef DXSOCKET_H
#define DXSOCKET_H

#ifdef WIN32
#include <sys/types.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#define dxErrno WSAGetLastError()
const char *dxStrError(int x);
#else
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#define dxErrno errno
#define dxStrError strerror
typedef int SOCKET;
#define INVALID_SOCKET (-1)
#endif //WIN32

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

char *dx_inet_ntop(int af, const void *addr, char *buf, size_t size);
#ifdef HAVE_INET_NTOP
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#define dx_inet_ntop(af,addr,buf,size) inet_ntop(af,addr,buf,(socklen_t)size)
#endif
int dx_inet_pton(int, const char *, void *);
#ifdef HAVE_INET_PTON
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#define dx_inet_pton(x,y,z) inet_pton(x,y,z)
#endif

#include <fx.h>
#include "dxmessagechannel.h"

#ifdef HAVE_OPENSSL
#include <openssl/ssl.h>
#endif

#define BUFFER_SIZE 32768

enum {
    SOCKET_CONNECTED = FX::SEL_LAST,
    SOCKET_STARTACCEPT,
    SOCKET_LISTEN,
    SOCKET_DISCONNECTED,
    SOCKET_CANREAD, //new data available
    SOCKET_WRITTEN,
    SOCKET_ERR
};

enum SocketErrorType {
    NONEERROR,
    UNABLEINIT, // Unable to initiliaze socket on win
    BADHOST,
    UNABLECREATE, // Unable to create socket
    UNABLECONNECT, // Unable to connect
    SSLUNCREATE, // SSL creation error
    SSLCONNECTERROR, // SSL connect error
    UNABLEBIND, // Unable to bind socket
    UNABLELISTEN, // Unable to listen
    UNABLEACCEPT, // Unable to accept connection
    UNABLESEND, // Unable to send data
    SSLZERO, // SSL_read() returns zero
    SSLUNABLEREAD, // SSL read problem,
    SSLABNORMAL, // Abnormal value from SSL read
    UNABLEREAD, // Error in reading data
    UNABLEREADBUFFER //Error in reading data from inner buffer
};

struct SocketError {
    SocketErrorType errorType;
    FXString errorStr;
};

class dxSocket;

class dxConnectThread : public FXThread
{
    public:
        dxConnectThread(dxSocket *parent, FXApp *app);
        virtual ~dxConnectThread();
        FXMessageChannel m_feedback;

        void setListen(FXbool listen) { m_listen = listen; }
    protected:
        FXint run();
    private:
        dxSocket *m_socket;
        FXbool m_listen;
};

class dxSocket: public FXObject
{
    FXDECLARE(dxSocket)
    friend class dxConnectThread;
    public:
        dxSocket(FXApp *app, FXObject *tgt=NULL, FXSelector sel=0, FXbool isSSL=FALSE, FXbool listen=FALSE);
        virtual ~dxSocket();
        enum {
            ID_SOCKET = FXMainWindow::ID_LAST+200,
            ID_SSLTIME, //ssl on windows
            ID_LAST
        };

        void listenOn();
        void connectTo();
        FXint read(char *buffer, FXint len);
        FXint write(const char *buffer, size_t len);
        FXint write(const FXString &line);
        void startConnection();
        void setHost(const FXString &address, FXint portD, FXint portH=0);
        void disconnect();       
        FXbool isConnected() const { return m_connected; }
        FXbool isConnecting() const { return m_connecting; }
        int socketDescriptor() const { return m_sockfd; }
        FXString getLocalIP();
        FXString getHostname(const FXString &ip);
        FXString getIP(const FXString &hostname);
        FXString getRemoteIP();
        FXString stringIPToBinary(const FXString &ip);
        FXString binaryIPToString(const FXString &ip);
        FXbool isRoutableIP(FXuint ipaddr);
        void setTarget(FXObject *tgt);
        void setIsSSL(FXbool isSSL);
        FXString getHost() { return m_address; }
        FXint getPort() { return m_portD; }
        FXlong getBytesAvailable() { return m_readed; }
        FXbool isIPv4(const FXString &address);

        long onIORead(FXObject*, FXSelector, void*);
        long onIOWrite(FXObject*, FXSelector, void*);
    protected:
        dxSocket(){}

        FXint listenByThread();
        FXint connectByThread();
    private:
        FXApp *m_application;
        FXObject *m_target;
        FXSelector m_message;
        dxConnectThread *m_connect;
        FXbool m_connected, m_isSsl, m_connecting;
        #ifdef WIN32            
            WSAEVENT m_event;
        #endif
        sockaddr *m_sockaddr;
        SOCKET m_sockfd;
#ifdef HAVE_OPENSSL
        SSL_CTX *m_ctx;
        SSL *m_ssl;
        BIO *m_bio;
#endif
        FXString m_address, m_localIP, m_remoteIP;
        FXint m_portD, m_portH;
        FXchar *m_readBuffer;
        FXlong m_readed;
        FXchar *m_writeBuffer;
        FXlong m_written;

        void closeConnection();
        FXbool setSSL();
        FXint writeData();
        FXint readData();
        void addWriteInput();
        void setListenPort(sockaddr *ad);
        FXString getIPString(const struct sockaddr *sa);
        void shutdownSocket(SocketErrorType err);
        FXbool setNonBlocking();
};

#endif // DXSOCKET_H
