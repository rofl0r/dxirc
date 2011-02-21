/*
 *      dxsocket.cpp
 *
 *      Copyright (C) 1996-2001  Internet Software Consortium.
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

#define _WIN32_WINNT 0x0501 //code work only with windows xp and above; needed for getaddrinfo etc.

#include "dxsocket.h"
#include "config.h"
#ifdef HAVE_OPENSSL
#include <openssl/err.h>
#include <openssl/rand.h>
#include <fstream>
#include <ios>
#endif
#if ENABLE_NLS
#include <libintl.h>
#include <fcntl.h>
#define _(String) gettext (String)
#define gettext_noop(String) String
#define N_(String) gettext_noop (String)
#else
#define _(String) (String)
#define N_(String) (String)
#endif

#ifdef WIN32
#ifdef _MSC_VER
#define snprintf _snprintf
#if _MSC_VER < 1500
#define vsnprintf _vsnprintf
#else
#define ssize_t SSIZE_T
#define uint32_t UINT32
#endif
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#else
#define strncasecmp strnicmp
#define strcasecmp stricmp
#endif /* _MSC_VER */
// declared static so as to initialise the winsock2 DLL, just once
static FXint winsock2Initialised = 0;

// initialise the winsock2 DLL
FXbool winsock2init()
{
    if(winsock2Initialised == 0)
    {
        WSADATA wsa;
        if(WSAStartup(MAKEWORD(2,0),&wsa) != 0 || LOBYTE(wsa.wVersion) != 2)
            return FALSE;
    }
    winsock2Initialised++;
    return TRUE;
}

// and another - cleaup winsock2 DLL resources
void winsock2cleanup()
{
    winsock2Initialised--;
    if(winsock2Initialised == 0) WSACleanup();
}

const char *dxStrError(int x) 
{
static  char tmp[100];
    switch (x)
    {
    case 10004: return _("Interrupted function call.");
    case 10013: return _("Permission denied.");
    case 10014: return _("Bad address.");
    case 10022: return _("Invalid argument.");
    case 10024: return _("Too many open files.");
    case 10035: return _("Resource temporarily unavailable.");
    case 10036: return _("Operation now in progress.");
    case 10037: return _("Operation already in progress.");
    case 10038: return _("Socket operation on nonsocket.");
    case 10039: return _("Destination address required.");
    case 10040: return _("Message too long.");
    case 10041: return _("Protocol wrong type for socket.");
    case 10042: return _("Bad protocol option.");
    case 10043: return _("Protocol not supported.");
    case 10044: return _("Socket type not supported.");
    case 10045: return _("Operation not supported.");
    case 10046: return _("Protocol family not supported.");
    case 10047: return _("Address family not supported by protocol family.");
    case 10048: return _("Address already in use.");
    case 10049: return _("Cannot assign requested address.");
    case 10050: return _("Network is down.");
    case 10051: return _("Network is unreachable.");
    case 10052: return _("Network dropped connection on reset.");
    case 10053: return _("Software caused connection abort.");
    case 10054: return _("Connection reset by peer.");
    case 10055: return _("No buffer space available.");
    case 10056: return _("Socket is already connected.");
    case 10057: return _("Socket is not connected.");
    case 10058: return _("Cannot send after socket shutdown.");
    case 10060: return _("Connection timed out.");
    case 10061: return _("Connection refused.");
    case 10064: return _("Host is down.");
    case 10065: return _("No route to host.");
    case 10067: return _("Too many processes.");
    case 10091: return _("Network subsystem is unavailable.");
    case 10092: return _("Winsock.dll version out of range.");
    case 10093: return _("Successful WSAStartup not yet performed.");
    case 10101: return _("Graceful shutdown in progress.");
    case 10109: return _("Class type not found.");
    case 11001: return _("Host not found.");
    case 11002: return _("Nonauthoritative host not found.");
    case 11003: return _("This is a nonrecoverable error.");
    case 11004: return _("Valid name, no data record of requested type.");

    default:
        break;
    }
    sprintf(tmp, _("Winsock error code: %d"), x);
    return tmp;
}
#endif

#ifndef HAVE_INET_NTOP

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#include <string.h>
#include <errno.h>

#define IN6ADDRSZ       16
#define INADDRSZ         4
#define INT16SZ          2

/*
 * Format an IPv4 address, more or less like inet_ntoa().
 *
 * Returns `dst' (as a const)
 * Note:
 *  - uses no statics
 *  - takes a unsigned char* not an in_addr as input
 */
static char *inet_ntop4 (const unsigned char *src, char *dst, size_t size)
{
  char tmp[sizeof "255.255.255.255"];
  size_t len;

  FXASSERT(size >= 16);

  tmp[0] = '\0';
  (void)snprintf(tmp, sizeof(tmp), "%d.%d.%d.%d",
          ((int)((unsigned char)src[0])) & 0xff,
          ((int)((unsigned char)src[1])) & 0xff,
          ((int)((unsigned char)src[2])) & 0xff,
          ((int)((unsigned char)src[3])) & 0xff);

  len = strlen(tmp);
  if(len == 0 || len >= size)
  {
    return (NULL);
  }
  strcpy(dst, tmp);
  return dst;
}

#ifdef ENABLE_IPV6
/*
 * Convert IPv6 binary address into presentation (printable) format.
 */
static char *inet_ntop6 (const unsigned char *src, char *dst, size_t size)
{
  /*
   * Note that int32_t and int16_t need only be "at least" large enough
   * to contain a value of the specified size.  On some systems, like
   * Crays, there is no such thing as an integer variable with 16 bits.
   * Keep this in mind if you think this function should have been coded
   * to use pointer overlays.  All the world's not a VAX.
   */
  char tmp[sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255")];
  char *tp;
  struct {
    long base;
    long len;
  } best, cur;
  unsigned long words[IN6ADDRSZ / INT16SZ];
  int i;

  /* Preprocess:
   *  Copy the input (bytewise) array into a wordwise array.
   *  Find the longest run of 0x00's in src[] for :: shorthanding.
   */
  memset(words, '\0', sizeof(words));
  for (i = 0; i < IN6ADDRSZ; i++)
      words[i/2] |= (src[i] << ((1 - (i % 2)) << 3));

  best.base = -1;
  cur.base  = -1;
  best.len = 0;
  cur.len = 0;

  for (i = 0; i < (IN6ADDRSZ / INT16SZ); i++)
  {
    if(words[i] == 0)
    {
      if(cur.base == -1)
        cur.base = i, cur.len = 1;
      else
        cur.len++;
    }
    else if(cur.base != -1)
    {
      if(best.base == -1 || cur.len > best.len)
         best = cur;
      cur.base = -1;
    }
  }
  if((cur.base != -1) && (best.base == -1 || cur.len > best.len))
     best = cur;
  if(best.base != -1 && best.len < 2)
     best.base = -1;

  /* Format the result.
   */
  tp = tmp;
  for (i = 0; i < (IN6ADDRSZ / INT16SZ); i++)
  {
    /* Are we inside the best run of 0x00's?
     */
    if(best.base != -1 && i >= best.base && i < (best.base + best.len))
    {
      if(i == best.base)
         *tp++ = ':';
      continue;
    }

    /* Are we following an initial run of 0x00s or any real hex?
     */
    if(i != 0)
       *tp++ = ':';

    /* Is this address an encapsulated IPv4?
     */
    if(i == 6 && best.base == 0 &&
        (best.len == 6 || (best.len == 5 && words[5] == 0xffff)))
    {
      if(!inet_ntop4(src+12, tp, sizeof(tmp) - (tp - tmp)))
      {
        return (NULL);
      }
      tp += strlen(tp);
      break;
    }
    tp += snprintf(tp, 5, "%lx", words[i]);
  }

  /* Was it a trailing run of 0x00's?
   */
  if(best.base != -1 && (best.base + best.len) == (IN6ADDRSZ / INT16SZ))
     *tp++ = ':';
  *tp++ = '\0';

  /* Check for overflow, copy, and we're done.
   */
  if((size_t)(tp - tmp) > size)
  {
    return (NULL);
  }
  strcpy(dst, tmp);
  return dst;
}
#endif  /* ENABLE_IPV6 */

/*
 * Convert a network format address to presentation format.
 *
 * Returns pointer to presentation format address (`buf').
 * Returns NULL on error.
 */
char *dx_inet_ntop(int af, const void *src, char *buf, size_t size)
{
  switch (af) {
  case AF_INET:
    return inet_ntop4((const unsigned char*)src, buf, size);
#ifdef ENABLE_IPV6
  case AF_INET6:
    return inet_ntop6((const unsigned char*)src, buf, size);
#endif
  default:
    return NULL;
  }
}
#endif  /* HAVE_INET_NTOP */

#ifndef HAVE_INET_PTON

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#include <string.h>
#include <errno.h>

#define IN6ADDRSZ       16
#define INADDRSZ         4
#define INT16SZ          2

/*
 * WARNING: Don't even consider trying to compile this on a system where
 * sizeof(int) < 4.  sizeof(int) > 4 is fine; all the world's not a VAX.
 */

static int inet_pton4(const char *src, unsigned char *dst);
#ifdef ENABLE_IPV6
static int inet_pton6(const char *src, unsigned char *dst);
#endif

/* int
 * inet_pton(af, src, dst)
 *      convert from presentation format (which usually means ASCII printable)
 *      to network format (which is usually some kind of binary format).
 * return:
 *      1 if the address was valid for the specified address family
 *      0 if the address wasn't valid (`dst' is untouched in this case)
 *      -1 if some other error occurred (`dst' is untouched in this case, too)
 * notice:
 *      On Windows we store the error in the thread errno, not
 *      in the winsock error code. This is to avoid loosing the
 *      actual last winsock error. So use macro ERRNO to fetch the
 *      errno this funtion sets when returning (-1), not SOCKERRNO.
 * author:
 *      Paul Vixie, 1996.
 */
int
dx_inet_pton(int af, const char *src, void *dst)
{
  switch (af) {
  case AF_INET:
    return (inet_pton4(src, (unsigned char *)dst));
#ifdef ENABLE_IPV6
  case AF_INET6:
    return (inet_pton6(src, (unsigned char *)dst));
#endif
  default:
    return (-1);
  }
  /* NOTREACHED */
}

/* int
 * inet_pton4(src, dst)
 *      like inet_aton() but without all the hexadecimal and shorthand.
 * return:
 *      1 if `src' is a valid dotted quad, else 0.
 * notice:
 *      does not touch `dst' unless it's returning 1.
 * author:
 *      Paul Vixie, 1996.
 */
static int
inet_pton4(const char *src, unsigned char *dst)
{
  static const char digits[] = "0123456789";
  int saw_digit, octets, ch;
  unsigned char tmp[INADDRSZ], *tp;

  saw_digit = 0;
  octets = 0;
  tp = tmp;
  *tp = 0;
  while((ch = *src++) != '\0') {
    const char *pch;

    if((pch = strchr(digits, ch)) != NULL) {
      unsigned int val = *tp * 10 + (unsigned int)(pch - digits);

      if(saw_digit && *tp == 0)
        return (0);
      if(val > 255)
        return (0);
      *tp = (unsigned char)val;
      if(! saw_digit) {
        if(++octets > 4)
          return (0);
        saw_digit = 1;
      }
    }
    else if(ch == '.' && saw_digit) {
      if(octets == 4)
        return (0);
      *++tp = 0;
      saw_digit = 0;
    }
    else
      return (0);
  }
  if(octets < 4)
    return (0);
  memcpy(dst, tmp, INADDRSZ);
  return (1);
}

#ifdef ENABLE_IPV6
/* int
 * inet_pton6(src, dst)
 *      convert presentation level address to network order binary form.
 * return:
 *      1 if `src' is a valid [RFC1884 2.2] address, else 0.
 * notice:
 *      (1) does not touch `dst' unless it's returning 1.
 *      (2) :: in a full address is silently ignored.
 * credit:
 *      inspired by Mark Andrews.
 * author:
 *      Paul Vixie, 1996.
 */
static int
inet_pton6(const char *src, unsigned char *dst)
{
  static const char xdigits_l[] = "0123456789abcdef",
    xdigits_u[] = "0123456789ABCDEF";
  unsigned char tmp[IN6ADDRSZ], *tp, *endp, *colonp;
  const char *xdigits, *curtok;
  int ch, saw_xdigit;
  size_t val;

  memset((tp = tmp), 0, IN6ADDRSZ);
  endp = tp + IN6ADDRSZ;
  colonp = NULL;
  /* Leading :: requires some special handling. */
  if(*src == ':')
    if(*++src != ':')
      return (0);
  curtok = src;
  saw_xdigit = 0;
  val = 0;
  while((ch = *src++) != '\0') {
    const char *pch;

    if((pch = strchr((xdigits = xdigits_l), ch)) == NULL)
      pch = strchr((xdigits = xdigits_u), ch);
    if(pch != NULL) {
      val <<= 4;
      val |= (pch - xdigits);
      if(++saw_xdigit > 4)
        return (0);
      continue;
    }
    if(ch == ':') {
      curtok = src;
      if(!saw_xdigit) {
        if(colonp)
          return (0);
        colonp = tp;
        continue;
      }
      if(tp + INT16SZ > endp)
        return (0);
      *tp++ = (unsigned char) (val >> 8) & 0xff;
      *tp++ = (unsigned char) val & 0xff;
      saw_xdigit = 0;
      val = 0;
      continue;
    }
    if(ch == '.' && ((tp + INADDRSZ) <= endp) &&
        inet_pton4(curtok, tp) > 0) {
      tp += INADDRSZ;
      saw_xdigit = 0;
      break;    /* '\0' was seen by inet_pton4(). */
    }
    return (0);
  }
  if(saw_xdigit) {
    if(tp + INT16SZ > endp)
      return (0);
    *tp++ = (unsigned char) (val >> 8) & 0xff;
    *tp++ = (unsigned char) val & 0xff;
  }
  if(colonp != NULL) {
    /*
     * Since some memmove()'s erroneously fail to handle
     * overlapping regions, we'll do the shift by hand.
     */
    const ssize_t n = tp - colonp;
    ssize_t i;

    if(tp == endp)
      return (0);
    for (i = 1; i <= n; i++) {
      *(endp - i) = *(colonp + n - i);
      *(colonp + n - i) = 0;
    }
    tp = endp;
  }
  if(tp != endp)
    return (0);
  memcpy(dst, tmp, IN6ADDRSZ);
  return (1);
}
#endif /* ENABLE_IPV6 */

#endif /* HAVE_INET_PTON */


FXDEFMAP(dxSocket) dxSocketMap[] = {
    FXMAPFUNC(SEL_IO_READ, dxSocket::ID_SOCKET, dxSocket::onIORead),
    FXMAPFUNC(SEL_TIMEOUT, dxSocket::ID_SSLTIME, dxSocket::onIORead),
    FXMAPFUNC(SEL_IO_WRITE, dxSocket::ID_SOCKET, dxSocket::onIOWrite)
};

FXIMPLEMENT(dxSocket, FXObject, dxSocketMap, ARRAYNUMBER(dxSocketMap))

dxSocket::dxSocket(FXApp *app, FXObject *tgt, FXSelector sel, FXbool isSSL, FXbool listen)
    : m_application(app), m_target(tgt), m_message(sel), m_isSsl(isSSL)
#ifdef HAVE_OPENSSL
        ,m_ctx(NULL), m_ssl(NULL), m_bio(NULL)
#endif
{
#ifdef WIN32
    if(!winsock2init())
    {
        if(m_target)
        {
            SocketError *er = new SocketError;
            er->errorType = UNABLEINIT;
            er->errorStr = "";
            m_target->tryHandle(this, FXSEL(SOCKET_ERR, m_message), &er);
        }
        return;
    }
#endif
    m_connecting = FALSE;
    m_connected = FALSE;
    m_readBuffer = new FXchar[BUFFER_SIZE];
    m_readed = 0;
    m_writeBuffer = new FXchar[BUFFER_SIZE];
    m_written = 0;
    m_sockfd = INVALID_SOCKET;
    m_localIP = "";
    m_remoteIP = "";
    m_connect = new dxConnectThread(this, m_application);
}

dxSocket::~dxSocket()
{
#ifdef WIN32
    winsock2cleanup();
#endif
#ifdef HAVE_OPENSSL
    if(m_ssl)
    {
        SSL_free(m_ssl);
    }
#endif
    if(m_readBuffer)
        delete[] m_readBuffer;
    if(m_writeBuffer)
        delete[] m_writeBuffer;
    delete m_connect;
}

long dxSocket::onIORead(FXObject *, FXSelector, void *)
{
#ifdef WIN32
    if(m_isSsl)
        m_application->addTimeout(this, ID_SSLTIME, 100);
#endif
    if(m_connected)
    {
        readData();
    }
    return 1;
}

long dxSocket::onIOWrite(FXObject*, FXSelector, void*)
{
    if(m_written && m_connected)
        writeData();
    else
#ifdef WIN32
        m_application->removeInput((FXInputHandle)m_event, INPUT_WRITE);
#else
        m_application->removeInput(m_sockfd, INPUT_WRITE);
#endif
    return 1;
}

void dxSocket::setHost(const FXString& address, FXint portD, FXint portH)
{
    m_address = address;
    m_portD = portD;
    m_portH = portH;
}

void dxSocket::connectTo()
{
    if(m_connecting || m_connected)
        return;
    m_connecting = TRUE;
    m_connect->setListen(FALSE);
    m_connect->start();
}

//Start connection
FXint dxSocket::connectByThread()
{
    FXint err = 0;
    addrinfo hints;
    addrinfo *servinfo = NULL;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if((err = getaddrinfo(m_address.text(), FXStringVal(m_portD).text(), &hints, &servinfo)) != 0)
    {
        if(m_target)
        {
            SocketError *er = new SocketError;
            er->errorType = BADHOST;
            er->errorStr = gai_strerror(err);
            m_connect->m_feedback.message(m_target, FXSEL(SOCKET_ERR, m_message), &er, sizeof(SocketError*));
        }
        m_connecting = FALSE;
        m_connected = FALSE;
        return 0;
    }
    if(servinfo == NULL)
    {
        if(m_target)
        {
            SocketError *er = new SocketError;
            er->errorType = UNABLECONNECT;
            er->errorStr = "";
            m_connect->m_feedback.message(m_target, FXSEL(SOCKET_ERR, m_message), &er, sizeof(SocketError*));
        }
        m_connecting = FALSE;
        m_connected = FALSE;
        return 0;
    }
    if((m_sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1)
    {
        if(m_target)
        {
            SocketError *er = new SocketError;
            er->errorType = UNABLECREATE;
            er->errorStr = dxStrError(dxErrno);
            m_connect->m_feedback.message(m_target, FXSEL(SOCKET_ERR, m_message), &er, sizeof(SocketError*));
        }
        m_connecting = FALSE;
        m_connected = FALSE;
        freeaddrinfo(servinfo);
        return 0;
    }
    if(connect(m_sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1)
    {
#ifdef WIN32
        closesocket(m_sockfd);
#else
        close(m_sockfd);
#endif
        if(m_target)
        {
            SocketError *er = new SocketError;
            er->errorType = UNABLECONNECT;
            er->errorStr = "";
            m_connect->m_feedback.message(m_target, FXSEL(SOCKET_ERR, m_message), &er, sizeof(SocketError*));
        }
        m_connecting = FALSE;
        m_connected = FALSE;
        freeaddrinfo(servinfo);
        return 0;
    }
    m_sockaddr = servinfo->ai_addr;
    if(m_isSsl)
        setSSL();
#ifdef WIN32
    m_event = WSACreateEvent();
    WSAEventSelect(m_sockfd, m_event, FD_CONNECT|FD_READ|FD_CLOSE);
    if(m_isSsl) m_application->addTimeout(this, ID_SSLTIME, 500);
    else m_application->addInput((FXInputHandle)m_event, INPUT_READ, this, ID_SOCKET);
#else
    m_application->addInput((FXInputHandle)m_sockfd, INPUT_READ, this, ID_SOCKET);
#endif
    m_connected = TRUE;
    m_connecting = FALSE;
    if(m_target) m_connect->m_feedback.message(m_target, FXSEL(SOCKET_CONNECTED, m_message), NULL, 0);
    freeaddrinfo(servinfo);
    return 1;
}

FXbool dxSocket::setSSL()
{
#ifdef HAVE_OPENSSL
    FXint err = 0;
    SSLeay_add_ssl_algorithms();
    SSL_load_error_strings();
    m_ctx = SSL_CTX_new(SSLv23_client_method());
    SSL_CTX_set_options(m_ctx, SSL_OP_ALL);
    if(!m_ctx)
    {
        shutdownSocket(SSLUNCREATE);
        return FALSE;
    }
#ifdef WIN32
    int i,r;
    for(i=0; i<128; i++)
    {
        r = rand();
        RAND_seed((unsigned char *)&r, sizeof(r));
    }
#endif
    m_ssl = SSL_new(m_ctx);
    if(!m_ssl)
    {
        shutdownSocket(SSLUNCREATE);
        return FALSE;
    }
    m_bio = BIO_new_socket(m_sockfd, BIO_NOCLOSE);
    if(!m_bio)
    {
        shutdownSocket(SSLUNCREATE);
        return FALSE;
    }
    SSL_set_bio(m_ssl, m_bio, m_bio);
    err = SSL_connect(m_ssl);
    if(!err)
    {
        shutdownSocket(SSLCONNECTERROR);
        return FALSE;
    }
    return TRUE;
#else
    return FALSE;
#endif //HAVE_OPENSSL
    return FALSE;
}

void dxSocket::listenOn()
{
    if(m_connecting || m_connected)
        return;
    m_connecting = TRUE;
    m_connect->setListen(TRUE);
    m_connect->start();
}

//Start listening
FXint dxSocket::listenByThread()
{
    int sd;
    FXint err;
    addrinfo hints, *servinfo, *p;
    sockaddr_storage their_addr; // connector's address information
    socklen_t addrlen;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    const char *address;
    if(m_address.empty())
    {
        address = NULL;
    }
    else
        address = m_address.text();
    if((err = getaddrinfo(address, FXStringVal(m_portD).text(), &hints, &servinfo)) != 0)
    {
        if(m_target)
        {
            SocketError *er = new SocketError;
            er->errorType = BADHOST;
            er->errorStr = gai_strerror(err);
            m_connect->m_feedback.message(m_target, FXSEL(SOCKET_ERR, m_message), &er, sizeof(SocketError*));
        }
        m_connecting = FALSE;
        m_connected = FALSE;
        return 0;
    }
    for(p = servinfo; p != NULL; p = p->ai_next)
    {
        if((sd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            continue;
        }
        if(bind(sd, p->ai_addr, p->ai_addrlen) == -1)
        {
#ifdef WIN32
            closesocket(sd);
#else
            close(sd);
#endif
            continue;
        }
        break;
    }
    if(sd == -1)
    {
        if(m_target)
        {
            SocketError *er = new SocketError;
            er->errorType = UNABLECREATE;
            er->errorStr = dxStrError(dxErrno);
            m_connect->m_feedback.message(m_target, FXSEL(SOCKET_ERR, m_message), &er, sizeof(SocketError*));
        }
        m_connecting = FALSE;
        m_connected = FALSE;
        return 0;
    }
    if(p == NULL)
    {
        if(m_target)
        {
            SocketError *er = new SocketError;
            er->errorType = UNABLEBIND;
            er->errorStr = "";
            m_connect->m_feedback.message(m_target, FXSEL(SOCKET_ERR, m_message), &er, sizeof(SocketError*));
        }
        m_connecting = FALSE;
        m_connected = FALSE;
        return 0;
    }
    freeaddrinfo(servinfo);
    socklen_t len = sizeof(p->ai_addr);
    getsockname(sd, p->ai_addr, &len);
    setListenPort(p->ai_addr);
    if(listen(sd, 1) == -1)
    {
        if(m_target)
        {
            SocketError *er = new SocketError;
            er->errorType = UNABLELISTEN;
            er->errorStr = "";
            m_connect->m_feedback.message(m_target, FXSEL(SOCKET_ERR, m_message), &er, sizeof(SocketError*));
        }
        m_connecting = FALSE;
        m_connected = FALSE;
        return 0;
    }
    if(m_target) m_target->tryHandle(this, FXSEL(SOCKET_STARTACCEPT, m_message), (void*)(FXival)m_portD);
    addrlen = sizeof(their_addr);
    m_sockfd = accept(sd, (sockaddr *)&their_addr, &addrlen);
    if(m_sockfd == -1)
    {
        if(m_target)
        {
            SocketError *er = new SocketError;
            er->errorType = UNABLEACCEPT;
            er->errorStr = "";
            m_connect->m_feedback.message(m_target, FXSEL(SOCKET_ERR, m_message), &er, sizeof(SocketError*));
        }
        m_connecting = FALSE;
        m_connected = FALSE;
        return 0;
    }
    m_sockaddr = (sockaddr *)&their_addr;
#ifdef WIN32
    shutdown(sd, SD_BOTH);
    closesocket(sd);
#else
#ifndef SHUT_RDWR
#define SHUT_RDWR 2
#endif
    shutdown(sd, SHUT_RDWR);
    close(sd);
#endif
#ifdef WIN32
    m_event = WSACreateEvent();
    WSAEventSelect(m_sockfd, m_event, FD_CONNECT|FD_READ|FD_WRITE|FD_CLOSE); // sets non-blocking!!
    m_application->addInput((FXInputHandle)m_event, INPUT_READ, this, ID_SOCKET);
#else
    m_application->addInput((FXInputHandle)m_sockfd, INPUT_READ, this, ID_SOCKET);
#endif
    m_connected = TRUE;
    m_connecting = FALSE;
    if(m_target) m_connect->m_feedback.message(m_target, FXSEL(SOCKET_LISTEN, m_message), NULL, 0);
    return 1;
}

void dxSocket::disconnect()
{
    m_target = NULL;
    memset(m_readBuffer, 0, sizeof(m_readBuffer));
    m_readed = 0;
    memset(m_writeBuffer, 0, sizeof(m_writeBuffer));
    m_written = 0;
    closeConnection();
}

void dxSocket::closeConnection()
{
    if(!m_connected)
        return;
    shutdownSocket(NONEERROR);
#ifdef HAVE_OPENSSL
    if(m_isSsl && m_ssl)
    {
#ifndef WIN32
        //avoid to die on a SIGPIPE if the connection has close (SSL_shutdown can call send())
        signal(SIGPIPE, SIG_IGN);
#endif
        SSL_shutdown(m_ssl);
#ifndef WIN32
        //restore normal SIGPIPE behaviour
        signal(SIGPIPE, SIG_DFL);
#endif
        if(m_ssl)
        {            
            SSL_free(m_ssl);
            m_ssl = 0;
        }
        if(m_ctx)
        {
            SSL_CTX_free(m_ctx);
            m_ctx = 0;
        }
    }
#endif
    if(m_target) m_target->tryHandle(this, FXSEL(SOCKET_DISCONNECTED, m_message), NULL);
//    m_errstr = "";
}

FXbool dxSocket::setNonBlocking()
{
#ifdef WIN32
    unsigned long arg = 1;
    if(ioctlsocket(m_sockfd, FIONBIO, &arg) != 0)
    {
        if(m_target)
        {
            SocketError *er = new SocketError;
            er->errorType = UNABLECREATE;
            er->errorStr = _("Unable set socket flag");
            m_target->tryHandle(this, FXSEL(SOCKET_ERR, m_message), &er);
        }
        return FALSE;
    }
    return TRUE;
#else
    int oldFlag = fcntl(m_sockfd, F_GETFL, 0);
    if(fcntl(m_sockfd, F_SETFL, oldFlag | O_NONBLOCK) == -1)
    {
        if(m_target)
        {
            SocketError *er = new SocketError;
            er->errorType = UNABLECREATE;
            er->errorStr = _("Unable set socket flag");
            m_target->tryHandle(this, FXSEL(SOCKET_ERR, m_message), &er);
        }
        return FALSE;
    }
    return TRUE;
#endif
    return FALSE;
}

void dxSocket::shutdownSocket(SocketErrorType err)
{
#ifdef WIN32
    m_application->removeTimeout(this, ID_SSLTIME);
    shutdown(m_sockfd, SD_BOTH);
    closesocket(m_sockfd);
    if(m_event)
    {
        m_application->removeInput((FXInputHandle)m_event, INPUT_READ|INPUT_WRITE);
        WSACloseEvent(m_event);
        m_event = NULL;
    }
#else
#ifndef SHUT_RDWR
#define SHUT_RDWR 2
#endif
    shutdown(m_sockfd, SHUT_RDWR);
    close(m_sockfd);
    m_application->removeInput(m_sockfd, INPUT_READ|INPUT_WRITE);
#endif
    if(m_target && err)
    {
        SocketError *er = new SocketError;
        er->errorType = err;
        er->errorStr = "";
        if(m_target) m_target->tryHandle(this, FXSEL(SOCKET_ERR, m_message), &er);
    }
    m_connecting = FALSE;
    m_connected = FALSE;
}

FXint dxSocket::read(char* buffer, FXint len)
{
    FXint toRead;
    if(len<0)
    {
        if(m_target)
        {
            SocketError *er = new SocketError;
            er->errorType = UNABLEREADBUFFER;
            er->errorStr = "";
            m_target->tryHandle(this, FXSEL(SOCKET_ERR, m_message), &er);
        }
        return -1;
    }
    if(!m_readed)
    {
        return m_connected ? 0 : -1;
    }
    toRead = (len>m_readed) ? m_readed : len;
    memcpy(buffer, m_readBuffer, toRead);
    m_readed -= toRead;
    memmove(m_readBuffer, m_readBuffer+toRead, m_readed);
    return toRead;
}

FXint dxSocket::readData()
{
    if(!m_connected) return 0;
    int size = 0;
    if(m_readed >= BUFFER_SIZE) return size;
#ifdef HAVE_OPENSSL
    if(m_isSsl)
    {
        FXint err = 0;
        size = SSL_read(m_ssl, m_readBuffer+m_readed, BUFFER_SIZE-m_readed);
        if(size == -1)
        {
            err = SSL_get_error(m_ssl, size);
            switch (err)
            {
            case SSL_ERROR_NONE:
            case SSL_ERROR_WANT_READ:
            case SSL_ERROR_WANT_WRITE:
                break;
            case SSL_ERROR_ZERO_RETURN:
            {
                if(m_target)
                {
                    SocketError *er = new SocketError;
                    er->errorType = SSLZERO;
                    er->errorStr = "";
                    m_target->tryHandle(this, FXSEL(SOCKET_ERR, m_message), &er);
                }
                closeConnection();
            }break;
            case SSL_ERROR_SYSCALL:
            {
                if(m_target)
                {
                    SocketError *er = new SocketError;
                    er->errorType = SSLUNABLEREAD;
                    if(!ERR_get_error()) er->errorStr = ERR_error_string(err, NULL);
                    else er->errorStr = "";
                    m_target->tryHandle(this, FXSEL(SOCKET_ERR, m_message), &er);
                }
                closeConnection();
            }break;
            default:
                if(m_target)
                {
                    SocketError *er = new SocketError;
                    er->errorType = SSLUNABLEREAD;
                    er->errorStr = ERR_error_string(err, NULL);
                    m_target->tryHandle(this, FXSEL(SOCKET_ERR, m_message), &er);
                }
                closeConnection();
            }
        }
        else if(!size)
        {
            closeConnection();
        }
        else if(size > 0)
        {
            //buffer[size] = '\0';
            m_readed += size;
            if(m_target) m_target->tryHandle(this, FXSEL(SOCKET_CANREAD, m_message), NULL);
        }
        else
        {
            if(m_target)
            {
                SocketError *er = new SocketError;
                er->errorType = SSLABNORMAL;
                er->errorStr = "";
                m_target->tryHandle(this, FXSEL(SOCKET_ERR, m_message), &er);
            }
        }
    }
    else
#endif
    {
#ifdef WIN32
        WSANETWORKEVENTS network_events;
        WSAEnumNetworkEvents(m_sockfd, m_event, &network_events);
        if (network_events.lNetworkEvents&FD_READ)
        {
            size = recv(m_sockfd, m_readBuffer+m_readed, BUFFER_SIZE-m_readed, 0);
            if (size > 0)
            {
                //buffer[size] = '\0';
                m_readed += size;
                if(m_target) m_target->tryHandle(this, FXSEL(SOCKET_CANREAD, m_message), NULL);
            }
            else if (size < 0)
            {
                if(m_target)
                {
                    SocketError *er = new SocketError;
                    er->errorType = UNABLEREAD;
                    er->errorStr = dxStrError(dxErrno);
                    m_target->tryHandle(this, FXSEL(SOCKET_ERR, m_message), &er);
                }
                closeConnection();
            }
            else closeConnection();
        }
        //else if (network_events.lNetworkEvents&FD_CONNECT) ;
        else if (network_events.lNetworkEvents&FD_CLOSE) closeConnection();
#else
        size = recv(m_sockfd, m_readBuffer+m_readed, BUFFER_SIZE-m_readed, 0);
        if(size > 0)
        {
            //buffer[size] = '\0';
            m_readed += size;
            if(m_target) m_target->tryHandle(this, FXSEL(SOCKET_CANREAD, m_message), NULL);
        }
        else if(size < 0)
        {
            if(m_target)
            {
                SocketError *er = new SocketError;
                er->errorType = UNABLEREAD;
                er->errorStr = dxStrError(dxErrno);
                m_target->tryHandle(this, FXSEL(SOCKET_ERR, m_message), &er);
            }
            closeConnection();
        }
        else closeConnection();
#endif
    }
    return size;
}

#ifdef WIN32
// inet_aton for windows
int inet_aton(const char *address, in_addr *sock)
{
    int s;
    s = inet_addr(address);
    if ( s == 1 && strcmp (address, "255.255.255.225") )
    return 0;
    sock->s_addr = s;
    return 1;
}
#endif /* WIN32 */

FXint dxSocket::write(const char *buffer, size_t len, FXbool directly)
{
    if(!m_connected) return 0;
    if(directly) return writeToSocket(buffer, len);
    FXint toWrite;
    size_t avail;
    avail = BUFFER_SIZE - m_written;
    toWrite = (len>avail) ? avail : len;
    if(!toWrite) return 0;
    memcpy(m_writeBuffer+m_written, buffer, toWrite);
    m_written += toWrite;
    addWriteInput();
    return toWrite;
}

FXint dxSocket::write(const FXString &line, FXbool directly)
{
    return write(line.text(), line.length(), directly);
}

FXint dxSocket::writeData()
{
    if(!m_writeBuffer) return 0;
    if(!m_written) return 0;
    FXint size = writeToSocket(m_writeBuffer, m_written);
    if(size > 0)
    {
        m_written -= size;
        if(m_written) memmove(m_writeBuffer, m_writeBuffer+size, m_written);
    }
    return size;
}

FXint dxSocket::writeToSocket(const char *buffer, size_t len)
{
    FXint size=0;
    if (m_connected)
    {
        if(m_isSsl)
        {
#ifdef HAVE_OPENSSL
            FXint err = 0;
            size = SSL_write(m_ssl, buffer, len);
            if(size == -1)
            {
                err = SSL_get_error(m_ssl, size);
                if(err != SSL_ERROR_WANT_READ && err != SSL_ERROR_WANT_WRITE)
                {
                    closeConnection();
                    if(m_target)
                    {
                        SocketError *er = new SocketError;
                        er->errorType = UNABLESEND;
                        er->errorStr = ERR_error_string(err, NULL);
                        m_target->tryHandle(this, FXSEL(SOCKET_ERR, m_message), &er);
                    }
                }
            }
            if(!size)
            {
                err = SSL_get_error(m_ssl, size);
                closeConnection();
                if(m_target)
                {
                    SocketError *er = new SocketError;
                    er->errorType = UNABLESEND;
                    er->errorStr = ERR_error_string(err, NULL);
                    m_target->tryHandle(this, FXSEL(SOCKET_ERR, m_message), &er);
                }
            }
            else
            {
                if(m_target) m_target->tryHandle(this, FXSEL(SOCKET_WRITTEN, m_message), (void*)(FXival)size);
            }
#endif
        }
        else
        {
#ifdef WIN32
            size = send(m_sockfd, buffer, len, 0);
#else
            size = send(m_sockfd, buffer, len, MSG_NOSIGNAL|MSG_DONTWAIT);
#endif
            if(size == -1)
            {
#ifdef WIN32
                if(dxErrno != WSAEWOULDBLOCK)
#else
                if(dxErrno != EWOULDBLOCK)
#endif
                {
                    if(m_target)
                    {
                        SocketError *er = new SocketError;
                        er->errorType = UNABLESEND;
                        er->errorStr = dxStrError(dxErrno);
                        m_target->tryHandle(this, FXSEL(SOCKET_ERR, m_message), &er);
                    }
                    closeConnection();
                }
                size = 0;
            }
            else if(size > 0)
            {
                if(m_target) m_target->tryHandle(this, FXSEL(SOCKET_WRITTEN, m_message), (void*)(FXival)size);
            }
        }
        return size;
    }
    else
    {
        if(m_target)
        {
            SocketError *er = new SocketError;
            er->errorType = UNABLESEND;
            er->errorStr = _("Can't write data - socket is not connected");
            m_target->tryHandle(this, FXSEL(SOCKET_ERR, m_message), &er);
        }
        return size;
    }
    return size;
}

void dxSocket::addWriteInput()
{
#ifdef WIN32
    m_application->addInput((FXInputHandle)m_event, INPUT_WRITE, this, ID_SOCKET);
#else
    m_application->addInput((FXInputHandle)m_sockfd, INPUT_WRITE, this, ID_SOCKET);
#endif
}

void dxSocket::setTarget(FXObject* tgt)
{
    m_target = tgt;
}

void dxSocket::setIsSSL(FXbool isSSL)
{
    m_isSsl = isSSL;
}

//address in ASCII order
FXbool dxSocket::isIPv4(const FXString& address)
{
    if(address.contains(':'))
        return FALSE;
    if(address.contains('.') == 3)
    {
        FXRex rex("\\l");
        if(!rex.match(address))
            return TRUE;
    }
    return FALSE;
}

//ipaddr in byte order
FXbool dxSocket::isRoutableIP(FXuint ipaddr)
{
    if(!ipaddr) return FALSE;
    unsigned char * ip = (unsigned char *)&ipaddr;
    int a, b, c, d;
    d = (int)*ip++;
    c = (int)*ip++;
    b = (int)*ip++;
    a = (int)*ip;
    if(a == 0) return FALSE;    // old-style broadcast
    if(a == 10) return FALSE;   // Class A VPN
    if(a == 127) return FALSE;   // loopback
    if((a == 172) && (b >= 16) && (b <= 31)) return FALSE; // Class B VPN
    if((a == 192) && (b == 168)) return FALSE; // Class C VPN
    if((a == 169) && (b == 254)) return FALSE; // APIPA
    if((a == 192) && (b == 0) && (c == 2)) return FALSE; // Class B VPN
    if(a >= 224) return FALSE; // class D multicast and class E reserved
    return TRUE;
}

//address as "2130706433" or "2001:db8:63b3:1::3490"
FXString dxSocket::binaryIPToString(const FXString &ip)
{
    if(!ip.contains(':')) /* normal IPv4 address in 32bit number form */
    {
        in_addr addr;
        inet_aton(ip.text(), &addr);
        return inet_ntoa(addr);
    }
    return ip;
}

//Return IP usefull for DCC (byte order if IPv4)
//address as "127.0.0.1" or "2001:db8:63b3:1::3490"
FXString dxSocket::stringIPToBinary(const FXString &ip)
{
    if(isIPv4(ip))
    {
        return FXStringVal((FXulong)ntohl(inet_addr(ip.text())));
    }
#ifdef ENABLE_IPV6
    return ip;
#endif
    return "";
}

//m_portD is listen port
void dxSocket::setListenPort(sockaddr* ad)
{
    if(ad->sa_family == AF_INET)
    {
        m_portD = ntohs(((sockaddr_in*)ad)->sin_port);
    }
#ifdef ENABLE_IPV6
    else
    {
        m_portD = ntohs(((sockaddr_in6*)ad)->sin6_port);
    }
#endif
}

//return string in dots-and-numbers format
FXString dxSocket::getLocalIP()
{
    if(m_connected && m_localIP.empty())
    {
        sockaddr local;
        socklen_t len = sizeof(sockaddr);
        if(getsockname(m_sockfd, &local, &len))
            return "";
        m_localIP = getIPString(&local);
    }
    return m_localIP;
}

//return string in dots-and-numbers format
FXString dxSocket::getRemoteIP()
{
    if(m_connected && m_remoteIP.empty())
    {
        m_remoteIP = getIPString(m_sockaddr);
    }
    return m_remoteIP;
}

//address as "127.0.0.1" or "2001:db8:63b3:1::3490"
FXString dxSocket::getHostname(const FXString& ip)
{
    if(m_connected)
    {
        hostent *he;
        if(isIPv4(ip))
        {
            in_addr ipv4;
            dx_inet_pton(AF_INET, ip.text(), &ipv4);
            he = gethostbyaddr((char*)&ipv4, sizeof(ipv4), AF_INET);
            return he->h_name;
        }
#ifdef ENABLE_IPV6
        else
        {
            in6_addr ipv6;
            dx_inet_pton(AF_INET6, ip.text(), &ipv6);
            he = gethostbyaddr((char*)&ipv6, sizeof(ipv6), AF_INET6);
            return he->h_name;
        }
#endif
    }
    return "";
}

FXString dxSocket::getIP(const FXString& hostname)
{
    if(m_connected)
    {
        FXString result = "";
        FXint err = 0;
        addrinfo hints, *servinfo, *p;
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        if((err = getaddrinfo(hostname.text(), NULL, &hints, &servinfo)) != 0)
        {
            if(m_target)
            {
                SocketError *er = new SocketError;
                er->errorType = BADHOST;
                er->errorStr = gai_strerror(err);
                m_target->tryHandle(this, FXSEL(SOCKET_ERR, m_message), &er);
            }
            return result;
        }
        for(p = servinfo; p != NULL; p = p->ai_next)
        {
            if(p->ai_family == m_sockaddr->sa_family)
            {
                result = getIPString(p->ai_addr);
                break;
            }
        }
        freeaddrinfo(servinfo);
        return result;
    }
    return "";
}

FXString dxSocket::getIPString(const sockaddr* sa)
{
    if(!sa)
        return "";
    char s[46];
    if(sa->sa_family==AF_INET)
    {
        dx_inet_ntop(AF_INET, &(((sockaddr_in*)sa)->sin_addr), s, 46);
        return s;
    }
#ifdef ENABLE_IPV6
    if(sa->sa_family==AF_INET6)
    {
        dx_inet_ntop(AF_INET6, &(((sockaddr_in6*)sa)->sin6_addr), s, 46);
        return s;
    }
#endif
    return "";
}

dxConnectThread::dxConnectThread(dxSocket *parent, FXApp *app)
    : m_feedback(app), m_socket(parent)
{
}

dxConnectThread::~dxConnectThread()
{
}

FXint dxConnectThread::run()
{
    if(m_listen) return m_socket->listenByThread();
    else return m_socket->connectByThread();
}

