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
#include "utils.h"

#define TAB_ORIENT_MASK    (TAB_TOP|TAB_LEFT|TAB_RIGHT|TAB_BOTTOM)

FXDEFMAP(dxTabItem) dxTabItemMap[] = {
    FXMAPFUNC(SEL_PAINT, 0, dxTabItem::onPaint)
};

FXIMPLEMENT(dxTabItem, FXTabItem, dxTabItemMap, ARRAYNUMBER(dxTabItemMap))

dxTabItem::dxTabItem(dxTabBook *tab, const FXString &tabtext, FXIcon *icon, FXuint opts,
        FXint id)
    : FXTabItem(tab, tabtext, icon, opts), m_id(id)
{
    this->setIconPosition(ICON_BEFORE_TEXT);
}

dxTabItem::~dxTabItem()
{

}

long dxTabItem::onPaint(FXObject*,FXSelector,void*)
{
    dxTabBook *bar = (dxTabBook*)getParent();
    FXint tab = bar->indexOfChild(this)/2;
    FXint ntabs = (bar->numChildren()/2);
    FXint ctab = bar->getCurrent();
    FXDCWindow dc(this);
    FXint tw=0,th=0,iw=0,ih=0,tx,ty,ix,iy;
    dc.setForeground(backColor);
    dc.fillRectangle(0,0,width,height);
    switch(options&TAB_ORIENT_MASK){
        case TAB_TOP:
            if(tab==ctab) dc.setForeground(hiliteColor);
            else dc.setForeground(borderColor);
            dc.drawLine(0,0,width,0);
            dc.drawLine(width-1,0,width-1,height-2);
            if(tab==ctab || tab==0) dc.drawLine(0,0,0,height-2);
            else
            {
                dc.setForeground(makeHiliteColor(shadowColor));
                dc.drawLine(0,1,0,height-2);
            }
            break;
        case TAB_BOTTOM:
            if(tab==ctab) dc.setForeground(hiliteColor);
            else dc.setForeground(borderColor);
            dc.drawLine(0,height-1,width,height-1);
            dc.drawLine(width-1,0,width-1,height-2);
            if(tab==ctab || tab==0) dc.drawLine(0,0,0,height-2);
            else
            {
                dc.setForeground(makeHiliteColor(shadowColor));
                dc.drawLine(0,1,0,height-2);
            }
            break;
        case TAB_LEFT:
            if(tab==ctab) dc.setForeground(hiliteColor);
            else dc.setForeground(borderColor);
            dc.drawLine(0,0,0,height);
            dc.drawLine(0,height-1,width-2,height-1);
            if(tab==ctab || tab==0) dc.drawLine(0,0,width-2,0);
            else
            {
                dc.setForeground(makeHiliteColor(shadowColor));
                dc.drawLine(1,0,width-2,0);
            }
            break;
        case TAB_RIGHT:
            if(tab==ctab) dc.setForeground(hiliteColor);
            else dc.setForeground(borderColor);
            dc.drawLine(width-1,0,width-1,height);
            dc.drawLine(0,height-1,width-2,height-1);
            if(tab==ctab || tab==0) dc.drawLine(0,0,width-2,0);
            else
            {
                dc.setForeground(makeHiliteColor(shadowColor));
                dc.drawLine(1,0,width-2,0);
            }
            break;
    }

    /// last one or active one
    if((tab == (ntabs-1)) || tab==ctab)
    {
        if(tab!=ctab)
            fillVerticalGradient(dc,1,1,width-2,height-2,makeHiliteColor(shadowColor),makeHiliteColor(shadowColor));
        else
        {
            switch(options&TAB_ORIENT_MASK){
                case TAB_LEFT:
                    fillHorizontalGradient(dc,1,1,width-2,height-2,makeHiliteColor(backColor),backColor);
                    break;
                case TAB_RIGHT:
                    fillHorizontalGradient(dc,1,1,width-2,height-2,backColor,makeHiliteColor(backColor));
                    break;
                case TAB_TOP:
                    fillVerticalGradient(dc,1,1,width-2,height-2,makeHiliteColor(backColor),backColor);
                    break;
                case TAB_BOTTOM:
                    fillVerticalGradient(dc,1,1,width-2,height-2,backColor,makeHiliteColor(backColor));
                    break;
            }
        }
    }
    else
    {
        fillVerticalGradient(dc,1,1,width-2,height-2,makeHiliteColor(shadowColor),makeHiliteColor(shadowColor));
    }

    if(!label.empty())
    {
        tw=labelWidth(label);
        th=labelHeight(label);
    }
    if(icon)
    {
        iw=icon->getWidth();
        ih=icon->getHeight();
    }
        just_x(tx,ix,tw,iw);
        just_y(ty,iy,th,ih);
    if(icon)
    {
        if(isEnabled())
            dc.drawIcon(icon,ix,iy);
        else
            dc.drawIconSunken(icon,ix,iy);
    }
    if(!label.empty())
    {
        dc.setFont(font);
        if(isEnabled())
        {
            dc.setForeground(textColor);
            drawLabel(dc,label,hotoff,tx,ty,tw,th);
            if(hasFocus())
            {
                dc.drawFocusRectangle(border+1,border+1,width-2*border-2,height-2*border-2);
            }
        }
        else
        {
            dc.setForeground(hiliteColor);
            drawLabel(dc,label,hotoff,tx+1,ty+1,tw,th);
            dc.setForeground(shadowColor);
            drawLabel(dc,label,hotoff,tx,ty,tw,th);
        }
    }
    return 1;
}


