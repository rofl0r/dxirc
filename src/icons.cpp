/*
 *      icons.cpp
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

#include <fx.h>
#include <FXPNGIcon.h>

#include "icons.h"

FXIcon *bigicon, *smallicon;
FXIcon *irc_owner_icon, *irc_admin_icon, *irc_op_icon, *irc_voice_icon, *irc_halfop_icon, *irc_normal_icon;
FXIcon *irc_away_owner_icon, *irc_away_admin_icon, *irc_away_op_icon, *irc_away_voice_icon, *irc_away_halfop_icon, *irc_away_normal_icon;
FXIcon *serverlisticon, *connecticon, *disconnecticon, *quiticon, *closeicon, *optionicon, *helpicon;
FXIcon *servericon, *channelicon, *queryicon, *clearicon, *flagicon, *trayicon, *newm, *unewm, *chnewm, *foldericon, *ofoldericon, *fileicon;
FXColor menuColor, textBackColor;

/* created by reswrap from file empty.png */
const unsigned char empty[]={
  0x2f,0x2a,0x20,0x47,0x65,0x6e,0x65,0x72,0x61,0x74,0x65,0x64,0x20,0x62,0x79,0x20,
  0x72,0x65,0x73,0x77,0x72,0x61,0x70,0x20,0x76,0x65,0x72,0x73,0x69,0x6f,0x6e,0x20,
  0x34,0x2e,0x30,0x2e,0x30,0x20,0x2a,0x2f,0x0a,0x0a
};

/* created by reswrap from file pokus/admin.png */
const unsigned char admin[]={
  0x89,0x50,0x4e,0x47,0x0d,0x0a,0x1a,0x0a,0x00,0x00,0x00,0x0d,0x49,0x48,0x44,0x52,
  0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x10,0x08,0x06,0x00,0x00,0x00,0x1f,0xf3,0xff,
  0x61,0x00,0x00,0x00,0x01,0x73,0x52,0x47,0x42,0x00,0xae,0xce,0x1c,0xe9,0x00,0x00,
  0x00,0x06,0x62,0x4b,0x47,0x44,0x00,0xff,0x00,0xff,0x00,0xff,0xa0,0xbd,0xa7,0x93,
  0x00,0x00,0x00,0x09,0x70,0x48,0x59,0x73,0x00,0x00,0x0b,0x13,0x00,0x00,0x0b,0x13,
  0x01,0x00,0x9a,0x9c,0x18,0x00,0x00,0x00,0x07,0x74,0x49,0x4d,0x45,0x07,0xd8,0x0b,
  0x1e,0x0c,0x39,0x26,0x20,0x3c,0xd4,0x7c,0x00,0x00,0x00,0x19,0x74,0x45,0x58,0x74,
  0x43,0x6f,0x6d,0x6d,0x65,0x6e,0x74,0x00,0x43,0x72,0x65,0x61,0x74,0x65,0x64,0x20,
  0x77,0x69,0x74,0x68,0x20,0x47,0x49,0x4d,0x50,0x57,0x81,0x0e,0x17,0x00,0x00,0x00,
  0x1f,0x49,0x44,0x41,0x54,0x38,0xcb,0x63,0x60,0x18,0xf6,0xe0,0x3f,0x21,0x05,0x4c,
  0x94,0xda,0x30,0x6a,0x00,0x15,0x0c,0xa0,0x39,0x38,0xca,0x30,0x0a,0x08,0x02,0x00,
  0x2c,0x8a,0x01,0xd1,0x19,0xb2,0xf9,0x3e,0x00,0x00,0x00,0x00,0x49,0x45,0x4e,0x44,
  0xae,0x42,0x60,0x82
};

/* created by reswrap from file pokus/halfop.png */
const unsigned char halfop[]={
  0x89,0x50,0x4e,0x47,0x0d,0x0a,0x1a,0x0a,0x00,0x00,0x00,0x0d,0x49,0x48,0x44,0x52,
  0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x10,0x08,0x06,0x00,0x00,0x00,0x1f,0xf3,0xff,
  0x61,0x00,0x00,0x00,0x01,0x73,0x52,0x47,0x42,0x00,0xae,0xce,0x1c,0xe9,0x00,0x00,
  0x00,0x06,0x62,0x4b,0x47,0x44,0x00,0xff,0x00,0xff,0x00,0xff,0xa0,0xbd,0xa7,0x93,
  0x00,0x00,0x00,0x09,0x70,0x48,0x59,0x73,0x00,0x00,0x0b,0x13,0x00,0x00,0x0b,0x13,
  0x01,0x00,0x9a,0x9c,0x18,0x00,0x00,0x00,0x07,0x74,0x49,0x4d,0x45,0x07,0xd8,0x0b,
  0x1e,0x0d,0x00,0x1c,0xe9,0x76,0xea,0x43,0x00,0x00,0x00,0x19,0x74,0x45,0x58,0x74,
  0x43,0x6f,0x6d,0x6d,0x65,0x6e,0x74,0x00,0x43,0x72,0x65,0x61,0x74,0x65,0x64,0x20,
  0x77,0x69,0x74,0x68,0x20,0x47,0x49,0x4d,0x50,0x57,0x81,0x0e,0x17,0x00,0x00,0x00,
  0x93,0x49,0x44,0x41,0x54,0x38,0xcb,0xed,0xd1,0x4f,0x0a,0x41,0x61,0x14,0x86,0xf1,
  0x5f,0x8c,0xd9,0x02,0xcb,0x60,0x62,0x11,0x64,0x19,0xfe,0x64,0x21,0x66,0x56,0x61,
  0x21,0x77,0x66,0x05,0x0a,0x49,0xca,0x9c,0x22,0xca,0xe4,0xa8,0xdb,0xd7,0xbd,0xee,
  0x1d,0xcb,0x33,0xfa,0xde,0x77,0x70,0x3a,0xdf,0x73,0xf8,0x39,0x86,0xb8,0x60,0x92,
  0xeb,0xfa,0xc8,0xea,0x0e,0x38,0x60,0x80,0x5d,0xae,0x5b,0x63,0x5c,0x77,0xc0,0x13,
  0x4d,0xdc,0x23,0x77,0xb1,0x8d,0xae,0x90,0x46,0x92,0x4f,0xb1,0xf2,0x29,0xf2,0x0c,
  0x2b,0xbc,0xea,0x6e,0x30,0xc7,0x2d,0x1c,0xb4,0x71,0x44,0x0b,0x53,0x5c,0x13,0x37,
  0x95,0x2c,0xb0,0x8c,0xf7,0xbe,0xc0,0xcd,0x57,0x9a,0xf1,0xf7,0x4e,0xe4,0x47,0xe2,
  0xa6,0xd4,0xc1,0x87,0x11,0x36,0x71,0x15,0x38,0x27,0x6e,0x2a,0xc9,0xd0,0x2b,0x71,
  0xf3,0x27,0xe1,0x0d,0x40,0xe7,0x1e,0x12,0x99,0xe4,0x1e,0x10,0x00,0x00,0x00,0x00,
  0x49,0x45,0x4e,0x44,0xae,0x42,0x60,0x82
};

/* created by reswrap from file pokus/op.png */
const unsigned char op[]={
  0x89,0x50,0x4e,0x47,0x0d,0x0a,0x1a,0x0a,0x00,0x00,0x00,0x0d,0x49,0x48,0x44,0x52,
  0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x10,0x08,0x06,0x00,0x00,0x00,0x1f,0xf3,0xff,
  0x61,0x00,0x00,0x00,0x01,0x73,0x52,0x47,0x42,0x00,0xae,0xce,0x1c,0xe9,0x00,0x00,
  0x00,0x06,0x62,0x4b,0x47,0x44,0x00,0xff,0x00,0xff,0x00,0xff,0xa0,0xbd,0xa7,0x93,
  0x00,0x00,0x00,0x09,0x70,0x48,0x59,0x73,0x00,0x00,0x0b,0x13,0x00,0x00,0x0b,0x13,
  0x01,0x00,0x9a,0x9c,0x18,0x00,0x00,0x00,0x07,0x74,0x49,0x4d,0x45,0x07,0xd8,0x0b,
  0x1e,0x0c,0x37,0x20,0x57,0xdc,0x5c,0xc7,0x00,0x00,0x00,0x19,0x74,0x45,0x58,0x74,
  0x43,0x6f,0x6d,0x6d,0x65,0x6e,0x74,0x00,0x43,0x72,0x65,0x61,0x74,0x65,0x64,0x20,
  0x77,0x69,0x74,0x68,0x20,0x47,0x49,0x4d,0x50,0x57,0x81,0x0e,0x17,0x00,0x00,0x00,
  0xe8,0x49,0x44,0x41,0x54,0x38,0xcb,0xdd,0xd3,0xbf,0x2b,0x04,0x70,0x18,0xc7,0xf1,
  0xd7,0xb9,0x24,0x57,0x5c,0x32,0x5c,0xd9,0xae,0x9b,0x6f,0x30,0x58,0x64,0xba,0xa2,
  0x0c,0xea,0x2c,0x36,0xcb,0x75,0x9b,0x32,0xc8,0x72,0xff,0x80,0xc9,0xc0,0xe2,0x2f,
  0xb0,0x59,0xec,0x7e,0xc4,0x62,0xb0,0xe8,0xae,0x4c,0xa4,0x84,0x28,0xd4,0x49,0x11,
  0x29,0x96,0xe7,0xea,0xa6,0x2f,0x99,0xe4,0xa9,0x67,0x79,0xea,0xf3,0x7e,0xfa,0x7c,
  0xbf,0x9f,0x87,0x7f,0x5b,0x73,0xd8,0x47,0x1b,0x6f,0x38,0xc5,0x32,0xb2,0x3f,0x11,
  0xaf,0xe2,0x0e,0x75,0x14,0xd0,0x87,0x31,0xec,0x62,0x1b,0x3d,0x29,0xf1,0x38,0x1e,
  0x51,0xc2,0x34,0xce,0xf0,0x8c,0x75,0xdc,0xe3,0x00,0x0b,0x29,0xc0,0x26,0x96,0x50,
  0xc6,0x25,0x46,0x91,0xc3,0x46,0x6c,0x9f,0xc0,0x49,0x0a,0x70,0x85,0x22,0xb6,0xc2,
  0x42,0xa7,0x56,0xd0,0x08,0x3b,0xaf,0x29,0xc0,0x07,0x7a,0xc3,0xc6,0x70,0xd7,0x7c,
  0x0f,0x15,0x0c,0xe0,0x29,0x05,0x68,0x63,0x08,0xef,0xe8,0x8f,0x59,0x16,0x0f,0x18,
  0x8c,0x77,0xd9,0x49,0x01,0x0e,0x51,0xc5,0x31,0x16,0x43,0xb4,0x86,0x17,0x8c,0xa0,
  0x89,0xa9,0x14,0x60,0x1e,0x2d,0x4c,0xe2,0x06,0xd7,0x98,0xc5,0x39,0x6e,0x51,0xfb,
  0x2e,0x03,0x99,0xf8,0x89,0x26,0x66,0x90,0xff,0x4d,0x0a,0x33,0xb1,0xe9,0x28,0x32,
  0xf0,0xd9,0xd5,0x17,0x7f,0xef,0x68,0xbe,0x00,0x3d,0x37,0x31,0x38,0x8c,0x56,0x59,
  0x55,0x00,0x00,0x00,0x00,0x49,0x45,0x4e,0x44,0xae,0x42,0x60,0x82
};

/* created by reswrap from file pokus/owner.png */
const unsigned char owner[]={
  0x89,0x50,0x4e,0x47,0x0d,0x0a,0x1a,0x0a,0x00,0x00,0x00,0x0d,0x49,0x48,0x44,0x52,
  0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x10,0x08,0x06,0x00,0x00,0x00,0x1f,0xf3,0xff,
  0x61,0x00,0x00,0x00,0x01,0x73,0x52,0x47,0x42,0x00,0xae,0xce,0x1c,0xe9,0x00,0x00,
  0x00,0x06,0x62,0x4b,0x47,0x44,0x00,0xff,0x00,0xff,0x00,0xff,0xa0,0xbd,0xa7,0x93,
  0x00,0x00,0x00,0x09,0x70,0x48,0x59,0x73,0x00,0x00,0x0b,0x13,0x00,0x00,0x0b,0x13,
  0x01,0x00,0x9a,0x9c,0x18,0x00,0x00,0x00,0x07,0x74,0x49,0x4d,0x45,0x07,0xd8,0x0b,
  0x1e,0x0c,0x3a,0x25,0x92,0x18,0xd6,0x05,0x00,0x00,0x00,0x19,0x74,0x45,0x58,0x74,
  0x43,0x6f,0x6d,0x6d,0x65,0x6e,0x74,0x00,0x43,0x72,0x65,0x61,0x74,0x65,0x64,0x20,
  0x77,0x69,0x74,0x68,0x20,0x47,0x49,0x4d,0x50,0x57,0x81,0x0e,0x17,0x00,0x00,0x00,
  0x51,0x49,0x44,0x41,0x54,0x38,0xcb,0x63,0x60,0x18,0x05,0x83,0x1f,0xfc,0x27,0xa4,
  0x80,0x19,0x87,0xf8,0x34,0x06,0x06,0x06,0x25,0x06,0x06,0x06,0x4f,0x06,0x06,0x86,
  0xf7,0x0c,0x0c,0x0c,0x09,0x0c,0x0c,0x0c,0x5b,0x49,0xb1,0x59,0x90,0x81,0x81,0xe1,
  0x38,0xd4,0x05,0x27,0x19,0x18,0x18,0x84,0x70,0x29,0x64,0xc2,0x21,0x3e,0x97,0x81,
  0x81,0x61,0x39,0x94,0xbd,0x98,0x81,0x81,0x61,0x36,0xcd,0xc2,0x60,0x14,0x50,0x01,
  0x00,0x00,0x9f,0xc1,0x0a,0x3d,0x48,0x3a,0x17,0x7a,0x00,0x00,0x00,0x00,0x49,0x45,
  0x4e,0x44,0xae,0x42,0x60,0x82
};

/* created by reswrap from file pokus/voice.png */
const unsigned char voice[]={
  0x89,0x50,0x4e,0x47,0x0d,0x0a,0x1a,0x0a,0x00,0x00,0x00,0x0d,0x49,0x48,0x44,0x52,
  0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x10,0x08,0x06,0x00,0x00,0x00,0x1f,0xf3,0xff,
  0x61,0x00,0x00,0x00,0x01,0x73,0x52,0x47,0x42,0x00,0xae,0xce,0x1c,0xe9,0x00,0x00,
  0x00,0x06,0x62,0x4b,0x47,0x44,0x00,0xff,0x00,0xff,0x00,0xff,0xa0,0xbd,0xa7,0x93,
  0x00,0x00,0x00,0x09,0x70,0x48,0x59,0x73,0x00,0x00,0x0b,0x13,0x00,0x00,0x0b,0x13,
  0x01,0x00,0x9a,0x9c,0x18,0x00,0x00,0x00,0x07,0x74,0x49,0x4d,0x45,0x07,0xd8,0x0b,
  0x1e,0x0c,0x3b,0x10,0xdd,0xb0,0x23,0x67,0x00,0x00,0x00,0x19,0x74,0x45,0x58,0x74,
  0x43,0x6f,0x6d,0x6d,0x65,0x6e,0x74,0x00,0x43,0x72,0x65,0x61,0x74,0x65,0x64,0x20,
  0x77,0x69,0x74,0x68,0x20,0x47,0x49,0x4d,0x50,0x57,0x81,0x0e,0x17,0x00,0x00,0x00,
  0x20,0x49,0x44,0x41,0x54,0x38,0xcb,0x63,0x60,0x18,0x05,0x34,0x07,0xff,0x09,0x29,
  0x60,0xa2,0xd4,0x06,0x46,0x12,0x6c,0x66,0x1c,0x9c,0x5e,0x18,0x05,0x83,0x01,0x00,
  0x00,0xce,0xae,0x03,0x04,0x63,0x8f,0x6b,0x2e,0x00,0x00,0x00,0x00,0x49,0x45,0x4e,
  0x44,0xae,0x42,0x60,0x82
};

FXIcon* MakeIcon(FXApp *app, const FXString path, const FXString name)
{
    FXIcon *icon = NULL;
    FXString file = FXPath::search(path, name);
    if(!file.empty())
    {
        icon = new FXPNGIcon(app);
        if(icon)
        {
            FXFileStream stream;
            if(stream.open(file, FXStreamLoad))
            {
                icon->loadPixels(stream);
                icon->create();
                stream.close();
                return icon;
            }
        }
        delete icon;
    }
    return NULL;
}

FXIcon* MakeIcon(FXApp *app, const FXString path, const FXString name, const FXbool menu)
{
    FXIcon *icon = NULL;
    FXString file = FXPath::search(path, name);
    if(!file.empty())
    {
        icon = new FXPNGIcon(app);
        if(icon)
        {
            FXFileStream stream;
            if(stream.open(file, FXStreamLoad))
            {
                icon->loadPixels(stream);
                icon->blend(menu ? menuColor : textBackColor);
                icon->create();
                stream.close();
                return icon;
            }
        }
        delete icon;
    }
    else
    {
        if(name == "irc_owner.png") icon = new FXPNGIcon(app, owner);
        else if(name == "irc_admin.png") icon = new FXPNGIcon(app, admin);
        else if(name == "irc_op.png") icon = new FXPNGIcon(app, op);
        else if(name == "irc_voice.png") icon = new FXPNGIcon(app, voice);
        else if(name == "irc_halfop.png") icon = new FXPNGIcon(app, halfop);
        else icon = new FXPNGIcon(app, empty);
        icon->create();
        return icon;
    }
    return NULL;
}

FXIcon* MakeAwayIcon(FXApp *app, const FXString path, const FXString name)
{
    FXIcon *iconAway = NULL;
    FXString fileAway = FXPath::search(path, "irc_away.png");
    if(!fileAway.empty())
    {
        iconAway = new FXPNGIcon(app);
        if(iconAway)
        {
            FXFileStream stream;
            if(stream.open(fileAway, FXStreamLoad))
            {
                iconAway->loadPixels(stream);
                iconAway->blend(textBackColor);
                iconAway->create();
                stream.close();
            }
        }
    }
    else
    {
        iconAway = new FXPNGIcon(app, empty);
        iconAway->create();
    }
    FXIcon *icon = NULL;
    FXString file = FXPath::search(path, name);
    if(!file.empty())
    {
        icon = new FXPNGIcon(app);
        if(icon)
        {
            FXFileStream stream;
            if(stream.open(file, FXStreamLoad))
            {
                icon->loadPixels(stream);
                icon->blend(textBackColor);
                icon->create();
                stream.close();
                FXDCWindow dc(icon);
                dc.drawIcon(iconAway, 0, 0);
                return icon;
            }
        }
        delete icon;
    }
    else
    {
        if(name == "irc_owner.png") icon = new FXPNGIcon(app, owner);
        else if(name == "irc_admin.png") icon = new FXPNGIcon(app, admin);
        else if(name == "irc_op.png") icon = new FXPNGIcon(app, op);
        else if(name == "irc_voice.png") icon = new FXPNGIcon(app, voice);
        else if(name == "irc_halfop.png") icon = new FXPNGIcon(app, halfop);
        else icon = new FXPNGIcon(app, empty);
        icon->create();
        FXDCWindow dc(icon);
        dc.drawIcon(iconAway, 0, 0);
        return icon;
    }
    return NULL;
}

FXString CheckThemePath(const FXString &path)
{
    if(path == "internal") return path;
    else
    {
        const char *themeDefaultPath = DXIRC_DATADIR PATHSEPSTRING "icons" PATHSEPSTRING "default";
        if(FXStat::exists(path)) return path;
        return FXString(themeDefaultPath);
    }
}

FXbool MakeAllIcons(FXApp *app, const FXString &iniFile)
{
    FXbool success = true;
    FXString mainpath = DXIRC_DATADIR PATHSEPSTRING "icons";
    FXString flagpath = DXIRC_DATADIR PATHSEPSTRING "icons" PATHSEPSTRING "flags";
    FXSettings set;
    set.parseFile(iniFile, true);
    FXString themepath = CheckThemePath(set.readStringEntry("SETTINGS", "themePath", DXIRC_DATADIR PATHSEPSTRING "icons" PATHSEPSTRING "default"));
    menuColor = set.readColorEntry("SETTINGS", "basecolor", app->getBaseColor());
    textBackColor = set.readColorEntry("SETTINGS", "textBackColor", FXRGB(255,255,255));

    success = ((bigicon = MakeIcon(app, mainpath, "big_dxirc.png", true)) != NULL) &success;
    success = ((smallicon = MakeIcon(app, mainpath, "small_dxirc.png", true)) != NULL) &success;
    success = ((irc_owner_icon = MakeIcon(app, themepath, "irc_owner.png", false)) != NULL) &success;
    success = ((irc_admin_icon = MakeIcon(app, themepath, "irc_admin.png", false)) != NULL) &success;
    success = ((irc_op_icon = MakeIcon(app, themepath, "irc_op.png", false)) != NULL) &success;
    success = ((irc_voice_icon = MakeIcon(app, themepath, "irc_voice.png", false)) != NULL) &success;
    success = ((irc_halfop_icon = MakeIcon(app, themepath, "irc_halfop.png", false)) != NULL) &success;
    success = ((irc_normal_icon = MakeIcon(app, themepath, "irc_normal.png", false)) != NULL) &success;
    success = ((irc_away_owner_icon = MakeAwayIcon(app, themepath, "irc_owner.png")) != NULL) &success;
    success = ((irc_away_admin_icon = MakeAwayIcon(app, themepath, "irc_admin.png")) != NULL) &success;
    success = ((irc_away_op_icon = MakeAwayIcon(app, themepath, "irc_op.png")) != NULL) &success;
    success = ((irc_away_voice_icon = MakeAwayIcon(app, themepath, "irc_voice.png")) != NULL) &success;
    success = ((irc_away_halfop_icon = MakeAwayIcon(app, themepath, "irc_halfop.png")) != NULL) &success;
    success = ((irc_away_normal_icon = MakeAwayIcon(app, themepath, "irc_normal.png")) != NULL) &success;
    success = ((serverlisticon = MakeIcon(app, mainpath, "server.png", true)) != NULL) &success;
    success = ((connecticon = MakeIcon(app, mainpath, "connect.png", true)) != NULL) &success;
    success = ((disconnecticon = MakeIcon(app, mainpath, "disconnect.png", true)) != NULL) &success;
    success = ((quiticon = MakeIcon(app, mainpath, "door_out.png", true)) != NULL) &success;
    success = ((closeicon = MakeIcon(app, mainpath, "cross.png", true)) != NULL) &success;
    success = ((optionicon = MakeIcon(app, mainpath, "wrench.png", true)) != NULL) &success;
    success = ((helpicon = MakeIcon(app, mainpath, "help.png", true)) != NULL) &success;
    success = ((servericon = MakeIcon(app, mainpath, "world.png", true)) != NULL) &success;
    success = ((channelicon = MakeIcon(app, mainpath, "channel.png", true)) != NULL) &success;
    success = ((queryicon = MakeIcon(app, mainpath, "user.png", true)) != NULL) &success;
    success = ((clearicon = MakeIcon(app, mainpath, "clear.png", true)) != NULL) &success;
    success = ((flagicon = MakeIcon(app, flagpath, "cz.png", true)) != NULL) &success;
    success = ((trayicon = MakeIcon(app, mainpath, "small_dxirc.png")) != NULL) &success;
    success = ((newm = MakeIcon(app, mainpath, "newm.png")) != NULL) &success;
    success = ((unewm = MakeIcon(app, mainpath, "unewm.png", true)) != NULL) &success;
    success = ((chnewm = MakeIcon(app, mainpath, "chnewm.png", true)) != NULL) &success;
    success = ((foldericon = MakeIcon(app, mainpath, "folder.png", true)) != NULL) &success;
    success = ((ofoldericon = MakeIcon(app, mainpath, "folder_go.png", true)) != NULL) &success;
    success = ((fileicon = MakeIcon(app, mainpath, "page.png", true)) != NULL) &success;


    return success;
}
