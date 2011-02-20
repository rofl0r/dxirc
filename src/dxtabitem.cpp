/*
 *      dxtabitem.cpp
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

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#endif
#include "dxtabitem.h"
#include "tetristabitem.h"
#include "icons.h"
#include "config.h"
#include "i18n.h"

//FXDEFMAP(dxTabItem) dxTabItemMap[] = {
//};

FXIMPLEMENT(dxTabItem, FXTabItem, 0, 0)

dxTabItem::dxTabItem(dxTabBook *tab, const FXString &tabtext, FXIcon *icon, FXuint opts,
        FXint id)
    : FXTabItem(tab, tabtext, icon, opts), m_id(id)
{
    this->setIconPosition(ICON_BEFORE_TEXT);
}

dxTabItem::~dxTabItem()
{

}


