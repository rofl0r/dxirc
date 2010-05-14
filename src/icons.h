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

FXIcon* MakeIcon(FXApp *app, const FXString path, const FXString name, const FXbool menu);
FXIcon* MakeIcon(FXApp *app, const FXString path, const FXString name);
FXIcon* MakeIcon(FXApp* app, const FXString& path, FXint size=16, FXColor color=FXRGB(0,0,0));
FXIcon* MakeAwayIcon(FXApp *app, const FXString path, const FXString name);
FXbool MakeAllIcons(FXApp *app, const FXString &iniFile, const FXString &datadir);

extern FXIcon *bigicon;
extern FXIcon *smallicon;
extern FXIcon *irc_owner_icon;
extern FXIcon *irc_admin_icon;
extern FXIcon *irc_op_icon;
extern FXIcon *irc_voice_icon;
extern FXIcon *irc_halfop_icon;
extern FXIcon *irc_normal_icon;
extern FXIcon *irc_away_owner_icon;
extern FXIcon *irc_away_admin_icon;
extern FXIcon *irc_away_op_icon;
extern FXIcon *irc_away_voice_icon;
extern FXIcon *irc_away_halfop_icon;
extern FXIcon *irc_away_normal_icon;
extern FXIcon *serverlisticon;
extern FXIcon *connecticon;
extern FXIcon *disconnecticon;
extern FXIcon *quiticon;
extern FXIcon *closeicon;
extern FXIcon *optionicon;
extern FXIcon *helpicon;
extern FXIcon *servericon;
extern FXIcon *channelicon;
extern FXIcon *queryicon;
extern FXIcon *clearicon;
extern FXIcon *flagicon;
extern FXIcon *trayicon;
extern FXIcon *newm;
extern FXIcon *newfile;
extern FXIcon *unewm;
extern FXIcon *chnewm;
extern FXIcon *foldericon;
extern FXIcon *ofoldericon;
extern FXIcon *fileicon;
extern FXIcon *cancelicon;
extern FXIcon *finishicon;
extern FXIcon *downicon;
extern FXIcon *upicon;
extern FXIcon *dccicon;
extern FXIcon *dccnewm;
extern FXIcon *logsicon;
extern FXIcon *scripticon;
extern FXIcon *transfericon;
extern FXIcon *playicon;

extern const unsigned char empty[];
extern const unsigned char admin[];
extern const unsigned char halfop[];
extern const unsigned char op[];
extern const unsigned char owner[];
extern const unsigned char voice[];
extern const unsigned char small_dxirc[];
extern const unsigned char big_dxirc[];

#endif // ICONS_H
