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
 #include <windows.h>
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
    FXTextCodec* GetCodec();
    FXString LocaleToUtf8(const FXchar *buffer);
    FXString LocaleToUtf8(const FXString &text);
    void SetIniFile(const FXString &file);
    FXString GetIniFile();
    FXString GetParam(FXString, FXint, FXbool);
    FXString GetParam(FXString, FXint, FXbool, const FXchar&);
    FXString RemoveSpaces(const FXString&);
    FXString CreateModes(FXchar sign, FXchar mode, FXString nicks);
    void SetAlias();
    void SetAliases(dxStringMap);
    FXString GetAlias(FXString key);
    dxStringMap GetAliases();
    FXint CommandsNo();
    FXString CommandsAt(FXint);
    FXbool IsCommand(const FXString&);
    FXbool IsUtf8(const FXchar* string, FXuint length);
    void AddScriptCommand(LuaScriptCommand command);
    FXbool RemoveScriptCommand(const FXString &command);
    FXbool RemoveScriptCommands(const FXString &script);
    FXbool IsScriptCommand(const FXString &command);
    FXString GetHelpText(const FXString &command);
    FXString GetFuncname(const FXString &command);
    FXString GetScriptName(const FXString &command);
    FXString CheckThemePath(const FXString&);
    FXString CheckThemesList(const FXString&);
    FXString Encrypt(const FXString&);
    FXString Decrypt(const FXString&);
}


#endif	/* UTILS_H */

