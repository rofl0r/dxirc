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
#include "utils.h"

FXIcon *ICO_BIG, *ICO_SMALL;
FXIcon *ICO_IRCOWNER, *ICO_IRCADMIN, *ICO_IRCOP, *ICO_IRCVOICE, *ICO_IRCHALFOP, *ICO_IRCNORMAL;
FXIcon *ICO_IRCAWAYOWNER, *ICO_IRCAWAYADMIN, *ICO_IRCAWAYOP, *ICO_IRCAWAYVOICE, *ICO_IRCAWAYHALFOP, *ICO_IRCAWAYNORMAL;
FXIcon *ICO_SERVERLIST, *ICO_CONNECT, *ICO_DISCONNECT, *ICO_QUIT, *ICO_CLOSE, *ICO_OPTION, *ICO_HELP;
FXIcon *ICO_SERVER, *ICO_CHANNEL, *ICO_QUERY, *ICO_CLEAR, *ICO_FLAG, *ICO_TRAY, *ICO_NEWMSG, *ICO_NEWFILE, *ICO_QUERYNEWMSG, *ICO_CHANNELNEWMSG, *ICO_CLOSEFOLDER, *ICO_OPENFOLDER, *ICO_FILE;
FXIcon *ICO_CANCEL, *ICO_FINISH, *ICO_DOWN, *ICO_UP;
FXIcon *ICO_DCC, *ICO_DCCNEWMSG, *ICO_LOGS, *ICO_SCRIPT, *ICO_TRANSFER, *ICO_PLAY;
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

/* created by reswrap from file small_dxirc.png */
const unsigned char small_dxirc[]={
  0x89,0x50,0x4e,0x47,0x0d,0x0a,0x1a,0x0a,0x00,0x00,0x00,0x0d,0x49,0x48,0x44,0x52,
  0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x10,0x08,0x06,0x00,0x00,0x00,0x1f,0xf3,0xff,
  0x61,0x00,0x00,0x00,0x01,0x73,0x52,0x47,0x42,0x00,0xae,0xce,0x1c,0xe9,0x00,0x00,
  0x00,0x06,0x62,0x4b,0x47,0x44,0x00,0xff,0x00,0xff,0x00,0xff,0xa0,0xbd,0xa7,0x93,
  0x00,0x00,0x00,0x09,0x70,0x48,0x59,0x73,0x00,0x00,0x0b,0x13,0x00,0x00,0x0b,0x13,
  0x01,0x00,0x9a,0x9c,0x18,0x00,0x00,0x00,0x07,0x74,0x49,0x4d,0x45,0x07,0xd8,0x0c,
  0x1e,0x12,0x2f,0x10,0x57,0x86,0xa6,0x58,0x00,0x00,0x00,0x19,0x74,0x45,0x58,0x74,
  0x43,0x6f,0x6d,0x6d,0x65,0x6e,0x74,0x00,0x43,0x72,0x65,0x61,0x74,0x65,0x64,0x20,
  0x77,0x69,0x74,0x68,0x20,0x47,0x49,0x4d,0x50,0x57,0x81,0x0e,0x17,0x00,0x00,0x00,
  0x5d,0x49,0x44,0x41,0x54,0x38,0xcb,0x63,0x64,0x40,0x03,0xef,0x35,0x35,0xff,0xc3,
  0xd8,0x82,0xd7,0xaf,0x33,0x12,0x12,0x67,0x44,0xd6,0x7c,0x86,0x81,0x01,0xae,0xe8,
  0x06,0x03,0x03,0x43,0x0c,0x54,0x1e,0x97,0x38,0x03,0x03,0x03,0x03,0x13,0x03,0x85,
  0x80,0x05,0xd9,0x79,0x77,0xaf,0x5f,0xc7,0xea,0x1d,0x5c,0xe2,0x0c,0x0c,0x0c,0x0c,
  0x8c,0xc8,0xce,0x23,0x15,0xdc,0xa0,0x86,0x17,0x28,0x32,0x20,0x86,0x81,0x81,0x71,
  0x34,0x16,0x46,0x63,0x01,0xaf,0x01,0x31,0x68,0x39,0x15,0x97,0x38,0x00,0xe1,0x6d,
  0x38,0x85,0x65,0xee,0x73,0x7f,0x00,0x00,0x00,0x00,0x49,0x45,0x4e,0x44,0xae,0x42,
  0x60,0x82
};

/* created by reswrap from file big_dxirc.png */
const unsigned char big_dxirc[]={
  0x89,0x50,0x4e,0x47,0x0d,0x0a,0x1a,0x0a,0x00,0x00,0x00,0x0d,0x49,0x48,0x44,0x52,
  0x00,0x00,0x00,0x30,0x00,0x00,0x00,0x30,0x08,0x06,0x00,0x00,0x00,0x57,0x02,0xf9,
  0x87,0x00,0x00,0x00,0x01,0x73,0x52,0x47,0x42,0x00,0xae,0xce,0x1c,0xe9,0x00,0x00,
  0x00,0x06,0x62,0x4b,0x47,0x44,0x00,0xff,0x00,0xff,0x00,0xff,0xa0,0xbd,0xa7,0x93,
  0x00,0x00,0x00,0x09,0x70,0x48,0x59,0x73,0x00,0x00,0x0b,0x13,0x00,0x00,0x0b,0x13,
  0x01,0x00,0x9a,0x9c,0x18,0x00,0x00,0x00,0x07,0x74,0x49,0x4d,0x45,0x07,0xd8,0x0c,
  0x1e,0x12,0x2a,0x38,0x1f,0x44,0xfa,0xe7,0x00,0x00,0x00,0x19,0x74,0x45,0x58,0x74,
  0x43,0x6f,0x6d,0x6d,0x65,0x6e,0x74,0x00,0x43,0x72,0x65,0x61,0x74,0x65,0x64,0x20,
  0x77,0x69,0x74,0x68,0x20,0x47,0x49,0x4d,0x50,0x57,0x81,0x0e,0x17,0x00,0x00,0x0b,
  0x5b,0x49,0x44,0x41,0x54,0x68,0xde,0xb5,0x99,0xcd,0x8f,0x5e,0xd9,0x51,0xc6,0x7f,
  0xe7,0x9c,0xfb,0xf9,0xda,0x4d,0xcf,0x80,0xc4,0x8a,0x96,0x23,0x59,0x49,0x68,0x88,
  0xa2,0x09,0x44,0x78,0x13,0x34,0xab,0x24,0x52,0x36,0xac,0x60,0xc7,0x3f,0x80,0x94,
  0x2c,0xd2,0x82,0x11,0xa0,0x0e,0x10,0x13,0x5a,0x11,0xb1,0x04,0x1a,0xa1,0x48,0x78,
  0x34,0x0a,0xcc,0x22,0x84,0x1d,0xe4,0x4f,0x40,0x9a,0x2c,0x50,0x3c,0x33,0xf6,0x58,
  0x86,0x61,0x10,0x36,0x5e,0x04,0x29,0x91,0xc0,0xee,0xf7,0x7d,0xef,0xc7,0xf9,0x62,
  0x71,0xeb,0x9c,0x39,0xdd,0xd3,0xed,0x69,0x3b,0xce,0x2b,0x5d,0xd9,0xb2,0xef,0xad,
  0x5b,0x55,0xa7,0xea,0xa9,0xe7,0xa9,0xab,0x78,0x86,0xdf,0x97,0x21,0x36,0x40,0xba,
  0xd2,0x6f,0x96,0xeb,0x27,0xf0,0xea,0xdf,0xc1,0x57,0x7e,0xd6,0x36,0x00,0xd4,0xd3,
  0x3a,0xbf,0x3e,0x3a,0x8a,0xeb,0x9b,0x37,0xf1,0xde,0xe7,0x4b,0x6b,0x4d,0xdb,0xb6,
  0x68,0xad,0xb9,0xf1,0xfe,0xfb,0x4c,0xc0,0x5f,0x3d,0xc1,0xf6,0xfa,0xe8,0x28,0xea,
  0xef,0x7d,0x8f,0x18,0x23,0xd3,0x34,0x11,0x63,0xa4,0xaa,0x2a,0xaa,0xaa,0x22,0xc6,
  0xc8,0xf5,0xdb,0xb7,0x3f,0xd2,0x46,0xfa,0xe9,0xa7,0x71,0xfe,0x7f,0xbf,0xf8,0xc5,
  0xb8,0x79,0xed,0x35,0x00,0x62,0x8c,0x68,0xad,0xd1,0x5a,0xa3,0x94,0x22,0x84,0xc0,
  0xdf,0xbc,0xff,0x3e,0xbf,0x7a,0x2a,0xa3,0x67,0xd9,0x70,0x6f,0xbc,0xc1,0x3c,0xcf,
  0x84,0x10,0x68,0xdb,0x16,0x6b,0x2d,0x00,0xde,0x7b,0xfe,0xf2,0xf6,0x6d,0x3e,0xad,
  0xd4,0x13,0x6d,0x3c,0x53,0x00,0xeb,0xa3,0xa3,0x18,0xee,0xdf,0x07,0x20,0x84,0x90,
  0x83,0x00,0xa8,0xaa,0x8a,0xbf,0xbe,0x7f,0x9f,0x5f,0x02,0x02,0xd0,0x3f,0xc1,0x46,
  0xff,0xe3,0x1f,0x53,0x55,0x15,0x5a,0x6b,0xd6,0xeb,0x35,0xce,0x39,0xe6,0x79,0xe6,
  0xd1,0xa3,0x47,0x7c,0xe3,0xdd,0x77,0xf9,0x95,0xba,0xa6,0xae,0xeb,0x73,0x6d,0x9c,
  0xfe,0x55,0x17,0xb9,0x69,0xfb,0xda,0x6b,0x31,0xfc,0xe8,0x47,0x54,0x2f,0xbf,0x8c,
  0x7e,0xf0,0xe0,0x83,0xe8,0xaf,0x5c,0xc1,0xec,0xed,0xf1,0xc7,0x87,0x87,0x7c,0x5a,
  0xfe,0x2d,0x7c,0x84,0x8d,0xe9,0xb3,0x9f,0x25,0x88,0x8d,0x0e,0xe0,0xca,0x15,0x7e,
  0x5e,0x6c,0xfc,0x3a,0xb0,0xb6,0x96,0x4e,0x5d,0xbc,0xb2,0x2f,0x14,0x40,0x7c,0xf4,
  0x88,0xb7,0x0f,0x0f,0x99,0x8b,0xf2,0xf0,0x40,0x2c,0x1a,0x49,0x03,0x46,0x1a,0xf0,
  0x3c,0x1b,0xff,0x7d,0x74,0x84,0x73,0x8e,0x18,0x23,0xb3,0xb5,0x8c,0x62,0x2f,0x8a,
  0x8d,0xc7,0xc0,0x8b,0x4a,0x31,0xc7,0x78,0xe1,0x00,0x2e,0x54,0x42,0xfe,0xe1,0xc3,
  0xec,0xbc,0x02,0xb6,0xf2,0x67,0xba,0x1a,0x60,0x2d,0x8e,0x34,0x4f,0xb0,0x11,0x42,
  0x60,0x23,0xb5,0xaf,0xe4,0xfe,0x49,0x4e,0xad,0x91,0xd2,0xb3,0x31,0x3e,0x15,0xb2,
  0x5c,0x2c,0x80,0x7b,0xf7,0xd0,0x72,0xf3,0x04,0xb4,0x80,0x13,0xe7,0x53,0xcd,0x5f,
  0x96,0x7b,0x8d,0x52,0x0c,0xe7,0xd8,0x08,0x21,0x60,0xa4,0x59,0xc7,0x22,0x01,0xc9,
  0x46,0x5d,0x9c,0xc6,0xf0,0xbc,0x9b,0xb8,0x93,0x12,0x59,0xc9,0x43,0xf5,0xa9,0x8c,
  0x57,0x40,0x5b,0x55,0xd8,0x18,0xcf,0x2d,0xa3,0x79,0x9e,0xb3,0xc3,0x95,0xd8,0xa9,
  0x0a,0x27,0x42,0x51,0x86,0xf3,0xb3,0xf6,0xc0,0x79,0x03,0x66,0x02,0x2e,0x49,0xe6,
  0xd3,0x09,0x50,0x64,0xaa,0x6d,0x1a,0x9a,0xa6,0x41,0x4d,0x13,0xde,0x5a,0x0e,0x0a,
  0x3b,0xe9,0xd7,0xf7,0x3d,0xf3,0xbc,0xb8,0x36,0x7a,0x9f,0x4f,0x20,0x39,0x6b,0x81,
  0x9d,0xaa,0xc2,0x3b,0x87,0x87,0x33,0x6d,0x9c,0x1e,0x76,0xd5,0x69,0x98,0x1b,0x5e,
  0x7f,0x1d,0x63,0x0c,0x31,0x46,0x42,0x08,0x38,0xb7,0xb8,0x5a,0xcb,0x3d,0xb5,0x52,
  0x78,0x69,0x32,0x57,0x94,0x4e,0xd3,0x34,0x68,0xad,0xb9,0x65,0x2d,0xdf,0xba,0x76,
  0x0d,0x62,0xc4,0x4d,0x13,0x55,0x55,0x81,0x31,0x6c,0x81,0xed,0x30,0x60,0x81,0xde,
  0x98,0x6c,0x6f,0x2a,0x6d,0x0b,0x44,0xff,0x1b,0xf0,0x8d,0x4f,0x7d,0x8a,0xb6,0xeb,
  0x40,0x6b,0x82,0xb5,0xc4,0x18,0x51,0x4a,0xe1,0xbd,0x27,0xc6,0xc8,0x9f,0xdf,0xb9,
  0xc3,0x8b,0xf0,0x65,0x5d,0x0e,0x98,0xe9,0x3b,0xdf,0xa1,0x69,0x1a,0x62,0x8c,0x18,
  0x63,0xa8,0xeb,0x9a,0xa6,0x69,0x30,0xfb,0xfb,0x78,0x40,0x29,0x85,0x52,0x8a,0x4a,
  0x6b,0x34,0xf0,0x8b,0x7d,0x4f,0x57,0x55,0x0c,0xc0,0x30,0x0c,0xfc,0xf0,0xf1,0x63,
  0xbe,0x7e,0xed,0x1a,0x18,0x03,0xde,0x53,0x35,0xcd,0x32,0x33,0xbc,0xc7,0xec,0xef,
  0xd3,0x77,0xdd,0x52,0x42,0x21,0x10,0xc4,0x5e,0x5b,0xd4,0x7c,0x6b,0x0c,0xff,0x1e,
  0x02,0x7f,0xfa,0x99,0xcf,0x2c,0x73,0xe2,0xf8,0x18,0x2f,0x27,0x16,0x42,0x40,0xcb,
  0xb4,0xfe,0xd6,0x9d,0x3b,0xfc,0x5a,0x5d,0xd3,0xa4,0xf2,0x5b,0x1f,0x1d,0xc5,0xf1,
  0xbd,0xf7,0x3e,0x80,0x3c,0x19,0xf1,0xdb,0xed,0x76,0x09,0x66,0x6f,0x8f,0x0a,0x18,
  0x63,0x64,0x08,0x81,0x4d,0x08,0x54,0xd2,0x8c,0xb3,0x73,0xcc,0xc0,0xdb,0xde,0xf3,
  0x47,0x9f,0xfb,0x1c,0x6e,0x1c,0xa5,0x9b,0x0d,0xc7,0x8f,0x1e,0xa1,0xab,0x0a,0x6b,
  0x2d,0x66,0x6f,0x0f,0x6b,0x2d,0x1a,0xd8,0xc6,0x88,0x05,0x5c,0x8c,0xf8,0xa2,0x2c,
  0xde,0xf6,0x9e,0x57,0x5e,0x7a,0x89,0x18,0x23,0x75,0xdf,0x73,0x79,0x67,0x07,0x63,
  0x0c,0xba,0xae,0x73,0x25,0xfc,0xc9,0xad,0x5b,0x7c,0x42,0x12,0xd9,0x03,0x55,0x1a,
  0x30,0xab,0xcf,0x7f,0x9e,0xf0,0xe0,0x41,0x6e,0x24,0x53,0x0c,0x2a,0xb5,0xbb,0x8b,
  0x2a,0x1a,0x75,0x76,0x0e,0xad,0x97,0xc3,0xdb,0xdd,0xd9,0x21,0x1e,0x1f,0x73,0x78,
  0xfd,0x3a,0xe3,0xc3,0x87,0x84,0x07,0x0f,0x72,0x4d,0x9b,0x17,0x5f,0x64,0x2c,0x6c,
  0x78,0xef,0x31,0x02,0x08,0x51,0x2e,0x27,0xef,0x8a,0xc0,0xe1,0xf5,0xeb,0x58,0xb1,
  0x71,0x02,0x69,0xae,0x5c,0xa1,0xda,0xdb,0xe3,0x0f,0x0e,0x0f,0x79,0x79,0xb5,0xca,
  0x7d,0x04,0xa0,0x36,0x37,0x6e,0xc4,0xff,0x78,0xe5,0x95,0x85,0x98,0xc5,0x48,0x94,
  0x63,0xd1,0x4a,0xe1,0x04,0x93,0x95,0x34,0x58,0x05,0xb4,0x92,0x8d,0xbe,0xef,0x19,
  0x86,0x81,0xdd,0xdd,0xdd,0xa5,0xce,0xe5,0xe4,0x42,0x08,0x6c,0x36,0x9b,0xcc,0x8f,
  0x4a,0xde,0x34,0xce,0x33,0x5a,0x4a,0xc7,0xcb,0x29,0x20,0x01,0xd4,0x32,0x7d,0xeb,
  0xba,0x26,0x84,0x40,0x5d,0x64,0x3d,0x3d,0xff,0xfa,0x3c,0xf3,0x1b,0x5a,0xd3,0x34,
  0x0d,0xd3,0x34,0xf1,0x5e,0x8c,0x68,0xff,0xf0,0xe1,0x92,0x19,0x63,0x70,0x40,0xa5,
  0x14,0x5a,0xa6,0x61,0xa5,0x14,0xa1,0x98,0xba,0xb5,0x31,0xd9,0x29,0x6b,0x2d,0xbb,
  0xbb,0xbb,0xcc,0xf3,0xcc,0x38,0x8e,0x8c,0xe3,0x48,0x08,0x81,0xed,0x76,0x8b,0xf7,
  0x1e,0xe7,0x1c,0x56,0x9a,0x6f,0x76,0x0e,0xe7,0xdc,0xf2,0xbc,0x94,0xa2,0x12,0x34,
  0xab,0xd2,0x25,0xfc,0x28,0x01,0xc2,0x38,0x8e,0x18,0x63,0x72,0x22,0x8c,0x31,0x34,
  0x12,0x60,0xea,0xd1,0x01,0xd0,0xfe,0xde,0x3d,0x42,0x8c,0x8c,0xce,0x51,0x4b,0x5d,
  0x2a,0xa5,0x32,0x36,0x5b,0xc9,0x90,0x02,0xac,0x20,0x80,0x15,0x03,0xc9,0x49,0x25,
  0xd9,0x0b,0x21,0x64,0x34,0x4a,0x14,0xd9,0x7b,0x4f,0x6d,0x0c,0x73,0x08,0x4c,0xde,
  0x63,0x05,0xef,0x33,0x28,0x00,0x5a,0xeb,0x8c,0x2e,0x5a,0xeb,0x7c,0x02,0xc9,0xd1,
  0xb6,0x6d,0x89,0x31,0xd2,0xcb,0xbd,0x29,0xa8,0x39,0xcd,0x01,0xad,0x14,0x36,0x46,
  0x2a,0x71,0x36,0x65,0x79,0x2d,0x0f,0x51,0x60,0xf1,0x5c,0x4c,0xd3,0x74,0x72,0xf3,
  0x3c,0x2f,0x0d,0xaa,0x35,0xd6,0x39,0x94,0xc0,0xe3,0x1c,0x02,0x0d,0x60,0xbc,0xc7,
  0x4b,0x69,0xb6,0x45,0x52,0x5c,0x8c,0x6c,0x80,0x55,0x08,0xd4,0x4a,0xe5,0x8c,0x27,
  0x9a,0x9e,0xe0,0x3c,0x5d,0x48,0xd0,0xc3,0x30,0xd0,0x34,0x0d,0x5e,0x4a,0x92,0x18,
  0x23,0x8d,0x64,0x66,0x90,0x9b,0x62,0x81,0xcf,0x89,0x9b,0x28,0xa5,0xa8,0xa4,0x9c,
  0xac,0xb5,0x4c,0xd6,0x32,0x4d,0xd3,0x92,0x8d,0x10,0x08,0x21,0x50,0xc9,0xd1,0x56,
  0x42,0x0f,0x12,0xe7,0xa9,0x8a,0xa1,0x55,0x17,0xc3,0x28,0x4f,0xf6,0xba,0xce,0xce,
  0x8f,0x82,0x64,0x31,0x46,0x9c,0x73,0x6c,0x36,0x9b,0xdc,0xb4,0xd6,0x5a,0x9a,0xa6,
  0xe1,0x87,0xf3,0xcc,0xab,0xa0,0x2a,0x8a,0xa6,0x4a,0x14,0x61,0x8c,0xf1,0x04,0xdf,
  0x71,0xe2,0xc0,0x24,0xa7,0x54,0x1b,0xb3,0x8c,0x71,0xa5,0x72,0x76,0x52,0x73,0x72,
  0x2a,0xf0,0x34,0xa8,0xb6,0x89,0x3e,0x17,0x14,0xc4,0x4b,0xf3,0xa6,0x92,0x43,0xe6,
  0x49,0xd3,0x34,0x38,0xe7,0xf0,0xde,0xd3,0xf7,0x3d,0xab,0xd5,0x8a,0xc7,0xdb,0x2d,
  0x03,0x60,0x8c,0xe1,0xce,0x38,0xf2,0x4d,0xc9,0xab,0x6e,0xbe,0xf0,0x05,0x8e,0x13,
  0x2e,0x17,0x84,0xcd,0x17,0x2f,0x73,0xc5,0xdf,0x6b,0x63,0x70,0xde,0x2f,0x83,0x45,
  0x5e,0xec,0xbd,0x27,0x16,0x65,0x15,0x0a,0xca,0x1d,0x0a,0xb6,0xe9,0xa5,0x7c,0x4a,
  0x54,0x9b,0xa5,0xe7,0x9c,0x73,0xb9,0xf6,0xd3,0x49,0xcc,0xde,0x63,0xad,0x65,0x18,
  0x06,0x56,0x4d,0xc3,0x0c,0xfc,0xeb,0x38,0xf2,0x67,0x85,0xd4,0xac,0xfc,0xc3,0x87,
  0x7c,0xff,0x0c,0xee,0x53,0x72,0x8e,0x19,0xf8,0x9d,0x54,0x0e,0x31,0x52,0x09,0x1a,
  0x4d,0xf3,0x4c,0x04,0xde,0x28,0x9e,0x7f,0x92,0x8d,0xdf,0x96,0xff,0x4f,0xb3,0xc6,
  0x02,0x9d,0x80,0xc1,0x77,0x81,0xc6,0x7b,0x1a,0xef,0x4f,0x3e,0x3f,0x4d,0xcb,0xf3,
  0x52,0xef,0xdf,0x3c,0xa5,0x93,0x2f,0x44,0xbd,0x37,0x37,0x6e,0xc4,0x1f,0x1c,0x1c,
  0xd0,0xcb,0x8b,0x2b,0x39,0xca,0x18,0x23,0x73,0x08,0xfc,0x03,0xf0,0x92,0x9c,0x9e,
  0x03,0x1e,0xc0,0x89,0x2c,0x25,0x1b,0x6f,0x1d,0x1c,0xe4,0x97,0xb6,0x52,0x72,0xe9,
  0x14,0xff,0x29,0x04,0xf6,0x80,0x9d,0xba,0x66,0xb0,0x96,0xff,0x3a,0xc3,0xc6,0x4f,
  0x25,0x68,0x7e,0x41,0xea,0x7e,0x25,0xac,0xd3,0x4b,0x19,0xd5,0x22,0xc0,0x83,0x64,
  0xd4,0x3f,0xc1,0x86,0x2a,0x70,0xdf,0x01,0xe1,0x94,0xf2,0x6a,0x60,0x99,0x17,0x4f,
  0x21,0x29,0x2f,0x2c,0x68,0x8c,0x31,0x5c,0xae,0xeb,0x05,0xd7,0x85,0xe4,0xa5,0xa6,
  0xed,0x81,0x4d,0x11,0xc0,0x79,0x82,0x26,0xa9,0x2f,0x05,0x34,0x5a,0xe7,0xf4,0xd6,
  0xf5,0xd2,0xf6,0xab,0xae,0x63,0x8a,0x91,0x39,0xc6,0xe7,0x2f,0x68,0xb4,0xd6,0x79,
  0x30,0x8d,0xe3,0xb8,0x0c,0x9f,0x02,0x9f,0xdb,0x42,0x1b,0x9f,0x27,0x46,0x12,0x48,
  0x44,0xc0,0xca,0xac,0x49,0x80,0x90,0x7e,0x97,0x9b,0x86,0x46,0xa9,0xe7,0x2f,0x68,
  0x94,0x52,0x34,0x42,0x8f,0xd3,0x32,0xaa,0xa9,0xaa,0xcc,0x57,0x74,0x01,0xb9,0xe7,
  0x89,0x91,0x9d,0xc4,0xb3,0x12,0x07,0x32,0x86,0xaa,0xaa,0x18,0xa6,0x29,0x63,0xfc,
  0x24,0x28,0xf6,0xcc,0x82,0x66,0x7d,0xf3,0x66,0x86,0x33,0xad,0x75,0x76,0xb0,0xaa,
  0xaa,0xfc,0xef,0x4d,0xd3,0x70,0x7c,0x7c,0x8c,0x11,0x6e,0x93,0x08,0xd9,0x08,0xdc,
  0x07,0xfe,0xf0,0x63,0x1f,0xcb,0x23,0x5f,0x15,0xf5,0x9c,0xa4,0xa4,0x2f,0x26,0xbe,
  0x73,0x8e,0xbe,0x6d,0x61,0x9a,0xd8,0x7a,0xcf,0xa5,0xaa,0xe2,0x2d,0xe7,0xf8,0xda,
  0xc7,0x3f,0x8e,0x13,0xd6,0xbb,0x5e,0xaf,0xe9,0xfb,0x9e,0xa6,0x69,0xb8,0x74,0xe9,
  0x12,0xba,0xef,0xf9,0xda,0x9b,0x6f,0x7e,0x58,0xd0,0xfc,0xdf,0xb7,0xbf,0xcd,0xf1,
  0xf1,0x71,0x66,0x95,0x31,0x46,0xfa,0xbe,0xc7,0xec,0xef,0xe7,0x69,0x98,0x4a,0x66,
  0xb5,0x5a,0xe5,0x53,0x18,0xc4,0xa1,0xff,0x01,0xbe,0x7a,0xf5,0xea,0xc2,0xe1,0x85,
  0x0a,0xa4,0x20,0xcc,0xfe,0x3e,0x95,0xf4,0x49,0x90,0xec,0x4f,0x62,0x6b,0x9a,0x26,
  0x06,0xa0,0xd7,0x9a,0x7b,0xce,0xf1,0xd5,0xab,0x57,0xf3,0x89,0x94,0x64,0xae,0xeb,
  0x3a,0x42,0x08,0xfc,0xc5,0x9b,0x6f,0x7e,0x58,0xd0,0x1c,0xdf,0xbd,0xcb,0x66,0xb3,
  0x61,0xb5,0x5a,0xa1,0x94,0xca,0x9c,0x3b,0x84,0x80,0xd9,0xdb,0xa3,0xae,0x6b,0xa6,
  0x69,0x42,0x29,0xc5,0x76,0xbb,0x65,0x18,0x86,0x1c,0xd0,0x0c,0xfc,0x27,0xf0,0x95,
  0x4f,0x7e,0x12,0x63,0x0c,0xdb,0xed,0x76,0x59,0x9d,0x28,0x45,0x55,0x55,0x18,0x63,
  0x16,0x1b,0xa2,0xe4,0x0c,0x30,0x79,0xcf,0xcf,0x75,0x1d,0x75,0x5d,0xd3,0x75,0x1d,
  0x33,0xf0,0x4e,0x08,0xfc,0xde,0xd5,0xab,0x54,0x52,0x9a,0x6d,0xdb,0x12,0x42,0xe0,
  0x85,0x17,0x5e,0xc8,0x8d,0xfe,0xf5,0x77,0xde,0xe1,0x97,0x8d,0xc1,0x18,0x73,0x52,
  0xd0,0xec,0x7e,0xe9,0x4b,0xec,0x9c,0x21,0x24,0xb2,0xa0,0x51,0x8a,0xad,0xd4,0x7e,
  0x5d,0xd7,0x8c,0xe3,0xb8,0x34,0xb4,0xa8,0xaa,0xdf,0xbf,0x7e,0x7d,0xd9,0xfd,0x3c,
  0x78,0xc0,0x0b,0xe7,0xd8,0x98,0x05,0x76,0x13,0x71,0x4c,0x0c,0x34,0x4d,0xf2,0xc3,
  0xc2,0x46,0x77,0x86,0x8d,0xb4,0x01,0x34,0xc6,0xe4,0x7d,0xaa,0xda,0xdc,0xb8,0x11,
  0xef,0x1e,0x1c,0x2c,0xdc,0xfa,0xd4,0x7a,0xa3,0x15,0xa6,0x9a,0x32,0x92,0x32,0xaa,
  0xb5,0x66,0x92,0xc6,0x0b,0x31,0xa2,0x85,0x49,0x3a,0xe7,0x08,0x22,0x7a,0xac,0xb5,
  0x39,0x6b,0x93,0xb5,0xd4,0x52,0x4e,0xde,0xfb,0x4c,0x8f,0x53,0x8d,0x8f,0xd6,0x66,
  0x5e,0xe5,0x84,0x1f,0x6d,0x63,0xa4,0x4b,0xbb,0x23,0x29,0xc3,0xbf,0x0f,0x81,0x6b,
  0x92,0x7d,0x80,0xbb,0xf3,0xbc,0x08,0x9a,0x72,0xa7,0x99,0x68,0xef,0x50,0x0c,0xa5,
  0x44,0xf6,0x12,0x6a,0x94,0x2b,0xf5,0xae,0x6d,0x4f,0xc0,0x69,0x2d,0x2f,0xf0,0x02,
  0xbd,0xc6,0x18,0x6a,0x63,0xd8,0x7a,0xbf,0x88,0xf8,0xb6,0x65,0x1c,0xc7,0x4c,0xd6,
  0xb4,0xd6,0x99,0x5a,0x5b,0x71,0x7e,0x2a,0x9c,0x2f,0x75,0x7a,0x03,0xb4,0x6d,0xbb,
  0xd0,0x0a,0xad,0x3f,0x10,0x34,0x73,0x41,0x7d,0x8d,0x04,0xb3,0x92,0x00,0x7c,0x8c,
  0xb9,0x1f,0x52,0xf6,0xbb,0xae,0x5b,0x9c,0x14,0xde,0xd2,0x35,0x0d,0xb3,0xfc,0x3d,
  0x51,0x03,0x23,0x53,0x35,0xc1,0xef,0x6e,0xd7,0x2d,0x65,0x38,0x8e,0xf9,0x24,0x66,
  0x69,0x60,0x63,0x0c,0x95,0x52,0x74,0x92,0x59,0x0a,0x48,0x0e,0x12,0xdc,0x56,0xb4,
  0x49,0x08,0x21,0x37,0x73,0x16,0x34,0x5d,0x91,0x71,0x93,0xf6,0x33,0x49,0x74,0x00,
  0xab,0xaa,0xca,0x68,0x90,0xf4,0xaf,0xf7,0x7e,0x69,0x3e,0xc9,0x46,0x2f,0x14,0x63,
  0x9e,0x67,0x9c,0x73,0x19,0xa5,0xb4,0xd6,0x59,0x5a,0x9e,0x35,0xd4,0x66,0xe0,0x92,
  0xa8,0xb0,0x10,0x02,0x56,0xee,0x33,0xc6,0x2c,0xd0,0x2c,0x73,0x21,0x9d,0xc6,0x7a,
  0x1c,0xf3,0xf0,0xf2,0x29,0x80,0x50,0xac,0xf9,0xe6,0x53,0x3b,0x4a,0x0d,0x6c,0x9c,
  0xe3,0xb2,0x88,0x6d,0x6b,0x6d,0xfe,0x30,0x91,0xca,0x2b,0x51,0xe0,0xcd,0x66,0x43,
  0xd7,0x75,0x59,0x23,0xa4,0xe0,0x92,0x2e,0x56,0xb2,0x3b,0xb5,0x31,0x9e,0x58,0x2f,
  0xa6,0x12,0x31,0xc6,0x50,0x89,0x7a,0x4b,0x36,0x0c,0xd0,0x08,0xe9,0xc3,0x7b,0x5a,
  0x61,0xc2,0xb7,0x63,0xe4,0x55,0x50,0x3a,0x65,0xa3,0x2e,0xa6,0x64,0x28,0x9c,0x77,
  0xc0,0x4a,0x98,0x67,0x1a,0x6e,0x65,0x03,0xe6,0x21,0xb8,0x5e,0xb3,0x5a,0xad,0x16,
  0x78,0xeb,0xfb,0x5c,0x5e,0xb3,0xb5,0xcc,0x22,0xe8,0x53,0x49,0xfa,0xa2,0xbf,0x34,
  0x30,0x58,0xbb,0xe8,0x06,0xd1,0x18,0x65,0xdd,0x0f,0x52,0x96,0xa3,0xf7,0x99,0x1f,
  0xbd,0x17,0xe3,0x49,0x41,0xd3,0xc8,0x80,0xa1,0x70,0x3a,0x4a,0x59,0x75,0x05,0xe5,
  0x4d,0x4e,0x27,0xa1,0x9d,0x88,0x98,0xb5,0x96,0xb6,0x6d,0x33,0x2c,0x4e,0xd3,0x84,
  0x4f,0x4d,0xdb,0x34,0x79,0x9e,0x84,0xa2,0x74,0x74,0x01,0x10,0xb5,0x48,0xd8,0xa4,
  0xb9,0x93,0xf3,0x5e,0x12,0x3b,0x79,0x4f,0x67,0x4c,0x5e,0x7e,0x7d,0x48,0xd0,0x7c,
  0xf7,0x49,0x62,0x44,0x3e,0x46,0xfc,0x6e,0xa1,0x01,0x2a,0xd9,0xb6,0x25,0x48,0xfc,
  0xdb,0x69,0xfa,0x68,0x41,0x13,0x23,0xbf,0x25,0xe0,0x10,0x8b,0x4d,0x47,0xda,0x45,
  0xfd,0x23,0xd0,0x08,0xd2,0x48,0x04,0x27,0x9f,0xf7,0xfe,0xa7,0x13,0x34,0x77,0x0f,
  0x0e,0x08,0xc0,0xee,0xa5,0x4b,0x0c,0xc3,0x90,0xd9,0xa9,0x52,0x8a,0x9b,0xc3,0xc0,
  0x6f,0xf6,0x7d,0x0e,0xee,0xad,0xf5,0xfa,0x4c,0x41,0xf3,0x2f,0x07,0x07,0x79,0x2b,
  0xd7,0x16,0x4c,0x52,0x03,0xdf,0x07,0x3e,0x21,0xbc,0xc9,0x18,0xc3,0x3d,0xe7,0x9e,
  0xaf,0xa0,0xa9,0xe5,0x08,0xe7,0x79,0xce,0x30,0xe8,0xbd,0x5f,0x44,0x38,0x64,0x8a,
  0x5d,0x52,0xe3,0xd3,0x36,0x2e,0x49,0xc6,0x92,0xa0,0x71,0x85,0xde,0x4e,0x10,0xbc,
  0x8d,0x91,0xad,0x73,0xcf,0x5f,0xd0,0x58,0xef,0xd9,0x91,0x69,0x3a,0x79,0x9f,0xe1,
  0xb4,0x69,0x1a,0x7a,0xa0,0xeb,0xba,0x5c,0xeb,0xe7,0x7e,0xa1,0x39,0xf5,0x7d,0xcd,
  0x9e,0x0a,0x20,0x2f,0x6c,0x7f,0x16,0x5f,0x68,0xa2,0x64,0x28,0x89,0x88,0xf4,0xb1,
  0x2e,0xa1,0xcd,0x38,0x8e,0x99,0xc5,0x3e,0x49,0xd0,0x3c,0x06,0x8e,0x4f,0x81,0xc6,
  0x71,0x81,0x3a,0x75,0x5d,0xe7,0xd3,0x7e,0xae,0x01,0xd8,0x62,0x6d,0x52,0xd7,0xf5,
  0x42,0xe2,0x04,0x69,0x48,0xcd,0x18,0xe3,0xa2,0x97,0xcf,0x3b,0x49,0xa9,0xfd,0xcb,
  0x82,0x2e,0xad,0x0c,0xd1,0x5d,0x49,0x4c,0x5d,0xd7,0x8c,0xb2,0xe1,0xf3,0xcf,0x3b,
  0x80,0x5e,0x26,0x71,0x08,0x61,0x41,0x1f,0x59,0x74,0xe5,0x2f,0x38,0x42,0x03,0x7e,
  0xb0,0x5e,0xf3,0xea,0x39,0xcd,0x57,0x17,0xe2,0xa7,0xdc,0x39,0xa5,0xbe,0x09,0x21,
  0xb0,0x6a,0x5b,0x6e,0x59,0x7b,0xae,0x8d,0x67,0x0a,0xc0,0xec,0xef,0x9f,0xa0,0x02,
  0x4e,0x94,0x59,0x9f,0x24,0xa1,0xbc,0xbc,0xdc,0x98,0x9d,0x65,0x23,0x05,0xe1,0x8b,
  0x55,0x63,0xa3,0x35,0x75,0x5d,0x33,0x48,0x09,0xbd,0x3b,0x4d,0xe7,0xda,0x78,0xe6,
  0x0f,0xdd,0x66,0x6f,0x6f,0x81,0x42,0x60,0x47,0x29,0x6a,0x91,0x9a,0x89,0x2e,0xcf,
  0xc0,0xed,0x62,0x3a,0x9e,0x67,0xa3,0xfc,0x18,0xae,0x84,0x79,0x2a,0xa5,0x98,0xac,
  0x65,0x06,0x6e,0x59,0xfb,0x54,0xce,0x5f,0x38,0x00,0xb5,0xbb,0xfb,0xc1,0xb0,0x2b,
  0x60,0x72,0x3d,0xcf,0xcc,0x72,0x8c,0x1f,0xf5,0x62,0xb5,0xbb,0xcb,0x3f,0x9f,0x1e,
  0x76,0x31,0xb2,0x16,0x56,0x7a,0x11,0x1b,0x67,0xfd,0xfe,0x1f,0xe4,0xdd,0x39,0xb0,
  0x2f,0x7e,0xc2,0xa7,0x00,0x00,0x00,0x00,0x49,0x45,0x4e,0x44,0xae,0x42,0x60,0x82
};

FXIcon* makeIcon(FXApp *app, const FXString path, const FXString name)
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
    else
    {
        icon = new FXPNGIcon(app, name == "small_dxirc.png" ? small_dxirc : empty);
        icon->create();
        return icon;
    }
    return NULL;
}

FXIcon* makeIcon(FXApp *app, const FXString path, const FXString name, const FXbool menu)
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
        else if(name == "big_dxirc.png") icon = new FXPNGIcon(app, big_dxirc);
        else if(name == "small_dxirc.png") icon = new FXPNGIcon(app, small_dxirc);
        else icon = new FXPNGIcon(app, empty);
        icon->create();
        return icon;
    }
    return NULL;
}

FXIcon* makeIcon(FXApp* app, const FXString& path, FXint size, FXColor color)
{
    FXIconSource iconsource(app);
    FXIcon *icon = NULL;
    icon = iconsource.loadScaledIconFile(path, size);
    if(icon)
    {
        icon->blend(color);
        icon->create();
        return icon;
    }
    else
    {
        icon = new FXPNGIcon(app, empty);
        icon->create();
        return icon;
    }
    return NULL;
}

FXIcon* makeAwayIcon(FXApp *app, const FXString path, const FXString name)
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

FXbool makeAllIcons(FXApp *app, const FXString &iniFile, const FXString &datadir)
{
    FXbool success = TRUE;
    FXString mainpath = datadir+PATHSEPSTRING+"icons";
    FXString flagpath = datadir+PATHSEPSTRING+"icons"+PATHSEPSTRING+"flags";
    FXSettings set;
    set.parseFile(iniFile, TRUE);
    FXString themepath = utils::instance().checkThemePath(set.readStringEntry("SETTINGS", "themePath", DXIRC_DATADIR PATHSEPSTRING "icons" PATHSEPSTRING "default"));
    menuColor = set.readColorEntry("SETTINGS", "basecolor", app->getBaseColor());
    textBackColor = set.readColorEntry("SETTINGS", "textBackColor", FXRGB(0,0,0));

    success = ((ICO_BIG = makeIcon(app, mainpath, "big_dxirc.png", TRUE)) != NULL) &success;
    success = ((ICO_SMALL = makeIcon(app, mainpath, "small_dxirc.png", TRUE)) != NULL) &success;
    success = ((ICO_IRCOWNER = makeIcon(app, themepath, "irc_owner.png", FALSE)) != NULL) &success;
    success = ((ICO_IRCADMIN = makeIcon(app, themepath, "irc_admin.png", FALSE)) != NULL) &success;
    success = ((ICO_IRCOP = makeIcon(app, themepath, "irc_op.png", FALSE)) != NULL) &success;
    success = ((ICO_IRCVOICE = makeIcon(app, themepath, "irc_voice.png", FALSE)) != NULL) &success;
    success = ((ICO_IRCHALFOP = makeIcon(app, themepath, "irc_halfop.png", FALSE)) != NULL) &success;
    success = ((ICO_IRCNORMAL = makeIcon(app, themepath, "irc_normal.png", FALSE)) != NULL) &success;
    success = ((ICO_IRCAWAYOWNER = makeAwayIcon(app, themepath, "irc_owner.png")) != NULL) &success;
    success = ((ICO_IRCAWAYADMIN = makeAwayIcon(app, themepath, "irc_admin.png")) != NULL) &success;
    success = ((ICO_IRCAWAYOP = makeAwayIcon(app, themepath, "irc_op.png")) != NULL) &success;
    success = ((ICO_IRCAWAYVOICE = makeAwayIcon(app, themepath, "irc_voice.png")) != NULL) &success;
    success = ((ICO_IRCAWAYHALFOP = makeAwayIcon(app, themepath, "irc_halfop.png")) != NULL) &success;
    success = ((ICO_IRCAWAYNORMAL = makeAwayIcon(app, themepath, "irc_normal.png")) != NULL) &success;
    success = ((ICO_SERVERLIST = makeIcon(app, mainpath, "server.png", TRUE)) != NULL) &success;
    success = ((ICO_CONNECT = makeIcon(app, mainpath, "connect.png", TRUE)) != NULL) &success;
    success = ((ICO_DISCONNECT = makeIcon(app, mainpath, "disconnect.png", TRUE)) != NULL) &success;
    success = ((ICO_QUIT = makeIcon(app, mainpath, "door_out.png", TRUE)) != NULL) &success;
    success = ((ICO_CLOSE = makeIcon(app, mainpath, "cross.png", TRUE)) != NULL) &success;
    success = ((ICO_OPTION = makeIcon(app, mainpath, "wrench.png", TRUE)) != NULL) &success;
    success = ((ICO_HELP = makeIcon(app, mainpath, "help.png", TRUE)) != NULL) &success;
    success = ((ICO_SERVER = makeIcon(app, mainpath, "world.png", TRUE)) != NULL) &success;
    success = ((ICO_CHANNEL = makeIcon(app, mainpath, "channel.png", TRUE)) != NULL) &success;
    success = ((ICO_QUERY = makeIcon(app, mainpath, "user.png", TRUE)) != NULL) &success;
    success = ((ICO_CLEAR = makeIcon(app, mainpath, "clear.png", TRUE)) != NULL) &success;
    success = ((ICO_FLAG = makeIcon(app, flagpath, "cz.png", TRUE)) != NULL) &success;
    success = ((ICO_TRAY = makeIcon(app, mainpath, "small_dxirc.png")) != NULL) &success;
    success = ((ICO_NEWMSG = makeIcon(app, mainpath, "newm.png")) != NULL) &success;
    success = ((ICO_NEWFILE = makeIcon(app, mainpath, "newfile.png")) != NULL) &success;
    success = ((ICO_QUERYNEWMSG = makeIcon(app, mainpath, "unewm.png", TRUE)) != NULL) &success;
    success = ((ICO_CHANNELNEWMSG = makeIcon(app, mainpath, "chnewm.png", TRUE)) != NULL) &success;
    success = ((ICO_CLOSEFOLDER = makeIcon(app, mainpath, "folder.png", TRUE)) != NULL) &success;
    success = ((ICO_OPENFOLDER = makeIcon(app, mainpath, "folder_go.png", TRUE)) != NULL) &success;
    success = ((ICO_FILE = makeIcon(app, mainpath, "page.png", TRUE)) != NULL) &success;
    success = ((ICO_CANCEL = makeIcon(app, mainpath, "cancel.png", TRUE)) != NULL) &success;
    success = ((ICO_FINISH = makeIcon(app, mainpath, "tick.png", TRUE)) != NULL) &success;
    success = ((ICO_DOWN = makeIcon(app, mainpath, "arrow_down.png", TRUE)) != NULL) &success;
    success = ((ICO_UP = makeIcon(app, mainpath, "arrow_up.png", TRUE)) != NULL) &success;
    success = ((ICO_DCC = makeIcon(app, mainpath, "dcc.png", TRUE)) != NULL) &success;
    success = ((ICO_DCCNEWMSG = makeIcon(app, mainpath, "dccnewm.png", TRUE)) != NULL) &success;
    success = ((ICO_LOGS = makeIcon(app, mainpath, "logs.png", TRUE)) != NULL) &success;
    success = ((ICO_SCRIPT = makeIcon(app, mainpath, "script.png", TRUE)) != NULL) &success;
    success = ((ICO_TRANSFER = makeIcon(app, mainpath, "file.png", TRUE)) != NULL) &success;
    success = ((ICO_PLAY = makeIcon(app, mainpath, "play.png", TRUE)) != NULL) &success;

    return success;
}
