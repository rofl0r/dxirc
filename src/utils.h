/*
 *      utils.h
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

#ifndef UTILS_H
#define	UTILS_H

#ifdef WIN32
 #define WIN32_LEAN_AND_MEAN
 #include <windows.h>
 #include <mmsystem.h>
#endif
#include "defs.h"
#include <FX885910Codec.h>
#include <FX885911Codec.h>
#include <FX885913Codec.h>
#include <FX885914Codec.h>
#include <FX885915Codec.h>
#include <FX885916Codec.h>
#include <FX88591Codec.h>
#include <FX88592Codec.h>
#include <FX88593Codec.h>
#include <FX88594Codec.h>
#include <FX88595Codec.h>
#include <FX88596Codec.h>
#include <FX88597Codec.h>
#include <FX88598Codec.h>
#include <FX88599Codec.h>
#include <FXCP1250Codec.h>
#include <FXCP1251Codec.h>
#include <FXCP1252Codec.h>
#include <FXCP1253Codec.h>
#include <FXCP1254Codec.h>
#include <FXCP1255Codec.h>
#include <FXCP1256Codec.h>
#include <FXCP1257Codec.h>
#include <FXCP1258Codec.h>
#include <FXCP437Codec.h>
#include <FXCP850Codec.h>
#include <FXCP852Codec.h>
#include <FXCP855Codec.h>
#include <FXCP856Codec.h>
#include <FXCP857Codec.h>
#include <FXCP860Codec.h>
#include <FXCP861Codec.h>
#include <FXCP862Codec.h>
#include <FXCP863Codec.h>
#include <FXCP864Codec.h>
#include <FXCP865Codec.h>
#include <FXCP866Codec.h>
#include <FXCP869Codec.h>
#include <FXCP874Codec.h>
#include <FXKOI8RCodec.h>
#include <FXUTF8Codec.h>

namespace utils
{
    FXTextCodec* getCodec();
    FXString localeToUtf8(const FXchar *buffer);
    FXString localeToUtf8(const FXString &text);
    void setIniFile(const FXString &file);
    FXString getIniFile();
    FXString getParam(FXString, FXint, FXbool);
    FXString getParam(FXString, FXint, FXbool, const FXchar&);
    FXString removeSpaces(const FXString&);
    FXString removeNonalphanumeric(const FXString&);
    FXString createModes(FXchar sign, FXchar mode, FXString nicks);
    void setAlias();
    void setAliases(dxStringMap);
    FXString getAlias(FXString key);
    dxStringMap getAliases();
    FXint commandsNo();
    FXString commandsAt(FXint);
    FXbool isCommand(const FXString&);
    FXbool isUtf8(const FXchar* string, FXuint length);
    void addScriptCommand(LuaScriptCommand command);
    FXbool removeScriptCommand(const FXString &command);
    FXbool removeScriptCommands(const FXString &script);
    FXbool isScriptCommand(const FXString &command);
    FXString availableCommands();
    FXString availableScriptCommands();
    FXString getHelpText(const FXString &command);
    FXString getFuncname(const FXString &command);
    FXString getScriptName(const FXString &command);
    FXString checkThemePath(const FXString&);
    FXString checkThemesList(const FXString&);
    FXString encrypt(const FXString&);
    FXString decrypt(const FXString&);
    FXString getStringIniEntry(const FXchar *section,const FXchar *key,const FXchar *def=NULL);
    FXint getIntIniEntry(const FXchar *section, const FXchar *key, FXint def=0);
    FXbool getBoolIniEntry(const FXchar* section, const FXchar* key, FXbool def=FALSE);
    FXString getFileSize(FXlong);
    FXString getFileSize(const FXString&);
    FXString getSpeed(FXlong);
    FXString getRemaining(FXlong, FXlong);
    void playFile(const FXString&);
    void debugLine(const FXString&);
}


#endif	/* UTILS_H */

