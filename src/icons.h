/*
 *      ircsocket.h
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


#ifndef ICONS_H
#define ICONS_H

FXIcon* makeIcon(FXApp *app, const FXString path, const FXString name, const FXbool menu);
FXIcon* makeIcon(FXApp *app, const FXString path, const FXString name);
FXIcon* makeIcon(FXApp* app, const FXString& path, FXint size=16, FXColor color=FXRGB(0,0,0));
FXIcon* makeAwayIcon(FXApp *app, const FXString path, const FXString name);
FXbool makeAllIcons(FXApp *app, const FXString &iniFile, const FXString &datadir);

extern FXIcon *ICO_BIG;
extern FXIcon *ICO_SMALL;
extern FXIcon *ICO_IRCOWNER;
extern FXIcon *ICO_IRCADMIN;
extern FXIcon *ICO_IRCOP;
extern FXIcon *ICO_IRCVOICE;
extern FXIcon *ICO_IRCHALFOP;
extern FXIcon *ICO_IRCNORMAL;
extern FXIcon *ICO_IRCAWAYOWNER;
extern FXIcon *ICO_IRCAWAYADMIN;
extern FXIcon *ICO_IRCAWAYOP;
extern FXIcon *ICO_IRCAWAYVOICE;
extern FXIcon *ICO_IRCAWAYHALFOP;
extern FXIcon *ICO_IRCAWAYNORMAL;
extern FXIcon *ICO_SERVERLIST;
extern FXIcon *ICO_CONNECT;
extern FXIcon *ICO_DISCONNECT;
extern FXIcon *ICO_QUIT;
extern FXIcon *ICO_CLOSE;
extern FXIcon *ICO_OPTION;
extern FXIcon *ICO_HELP;
extern FXIcon *ICO_SERVER;
extern FXIcon *ICO_CHANNEL;
extern FXIcon *ICO_QUERY;
extern FXIcon *ICO_CLEAR;
extern FXIcon *ICO_FLAG;
extern FXIcon *ICO_TRAY;
extern FXIcon *ICO_NEWMSG;
extern FXIcon *ICO_NEWFILE;
extern FXIcon *ICO_QUERYNEWMSG;
extern FXIcon *ICO_CHANNELNEWMSG;
extern FXIcon *ICO_CLOSEFOLDER;
extern FXIcon *ICO_OPENFOLDER;
extern FXIcon *ICO_FILE;
extern FXIcon *ICO_CANCEL;
extern FXIcon *ICO_FINISH;
extern FXIcon *ICO_DOWN;
extern FXIcon *ICO_UP;
extern FXIcon *ICO_DCC;
extern FXIcon *ICO_DCCNEWMSG;
extern FXIcon *ICO_LOGS;
extern FXIcon *ICO_SCRIPT;
extern FXIcon *ICO_TRANSFER;
extern FXIcon *ICO_PLAY;

extern const unsigned char empty[];
extern const unsigned char admin[];
extern const unsigned char halfop[];
extern const unsigned char op[];
extern const unsigned char owner[];
extern const unsigned char voice[];
extern const unsigned char small_dxirc[];
extern const unsigned char big_dxirc[];

#endif // ICONS_H
