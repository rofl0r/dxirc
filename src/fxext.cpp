/*
 *      fxext.cpp
 *
 *      Copyright 2011 David Vachulka <david@konstrukce-cad.com>
 *      Copyright (C) 2009-2011 by Sander Jansen.
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

#include "defs.h"
#include "fxext.h"

#define TAB_ORIENT_MASK (TAB_TOP|TAB_LEFT|TAB_RIGHT|TAB_BOTTOM)
#define HORZ_PAD 30
#define VERT_PAD 2
#define MBOX_BUTTON_MASK (MBOX_OK|MBOX_OK_CANCEL|MBOX_YES_NO|MBOX_YES_NO_CANCEL|MBOX_QUIT_CANCEL|MBOX_QUIT_SAVE_CANCEL|MBOX_SAVE_CANCEL_DONTSAVE)

/* created by reswrap from file erroricon.gif */
const unsigned char erroricon[]={
  0x47,0x49,0x46,0x38,0x37,0x61,0x20,0x00,0x20,0x00,0xf1,0x00,0x00,0x80,0x80,0x80,
  0x80,0x00,0x00,0xff,0x00,0x00,0xff,0xff,0xff,0x2c,0x00,0x00,0x00,0x00,0x20,0x00,
  0x20,0x00,0x00,0x02,0x91,0x84,0x8f,0x10,0xcb,0x9b,0x0f,0x0f,0x13,0xb4,0x52,0x16,
  0x73,0xb0,0x9c,0x87,0x9c,0x2c,0xdd,0x58,0x39,0xe0,0x46,0xa6,0x97,0xa6,0xb6,0xc2,
  0x07,0xa1,0x2e,0x09,0x87,0xdd,0xa0,0xe2,0x9e,0xcd,0x0d,0x3e,0xe9,0xd3,0x59,0x6a,
  0x8a,0x5e,0x50,0x58,0x39,0x22,0x5f,0x08,0x59,0xf2,0x68,0x51,0x76,0x6a,0xce,0x28,
  0x54,0x20,0x1d,0xc1,0xaa,0x56,0x65,0x30,0xb5,0xcd,0x79,0x97,0xbb,0xa2,0xf8,0x0a,
  0x36,0x70,0x8d,0x68,0x9a,0xfa,0x9c,0x75,0x9b,0x47,0xe3,0xaf,0x7c,0x8d,0xbd,0xc6,
  0xcb,0xf3,0xa7,0x9d,0xb2,0xb7,0x22,0xc1,0xf6,0x43,0xb8,0x44,0xc4,0xf5,0xc7,0x36,
  0xc5,0x13,0x95,0xc3,0xf8,0x80,0x37,0xc3,0x14,0x33,0x29,0x57,0x69,0x39,0x04,0xa2,
  0x20,0xa9,0x45,0x74,0x32,0xf9,0xb9,0xc9,0x89,0x87,0x31,0x1a,0xd1,0xd0,0x70,0xba,
  0xca,0xda,0xea,0x6a,0x50,0x00,0x00,0x3b
};

/* created by reswrap from file warningicon.gif */
const unsigned char warningicon[]={
  0x47,0x49,0x46,0x38,0x37,0x61,0x20,0x00,0x20,0x00,0xf2,0x00,0x00,0x80,0x80,0x80,
  0x80,0x80,0x00,0xff,0xff,0x00,0xc0,0xc0,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x2c,0x00,0x00,0x00,0x00,0x20,0x00,0x20,0x00,0x00,0x03,
  0x8f,0x08,0xba,0xbc,0xf1,0x2d,0xca,0xa9,0x82,0x10,0x83,0xd0,0x2d,0xed,0xc5,0x1a,
  0x27,0x02,0xde,0x17,0x8e,0x54,0xf9,0x65,0x68,0xfa,0xbd,0x67,0xcb,0xa8,0x2f,0x2b,
  0xcf,0x6f,0x1e,0xcb,0x74,0x6e,0xdf,0xa5,0x0c,0x81,0x30,0x30,0xdd,0x48,0xb0,0xe1,
  0x70,0xb5,0x13,0xa9,0x94,0x4b,0x63,0x8b,0x06,0x25,0xd4,0x9a,0xae,0xa4,0x52,0x87,
  0xea,0x55,0x7d,0xd8,0x48,0x2f,0xa0,0xf4,0x3a,0x73,0x17,0x21,0x11,0xfd,0x9b,0xf4,
  0x2e,0x5b,0xb4,0x20,0x5c,0x91,0x5f,0xc8,0x6f,0x50,0x56,0x9e,0x29,0xda,0xc3,0x79,
  0x26,0x76,0x69,0x58,0x81,0x83,0x5c,0x0d,0x86,0x20,0x6b,0x76,0x6d,0x48,0x87,0x50,
  0x90,0x38,0x87,0x73,0x71,0x8d,0x27,0x8a,0x8b,0x7e,0x83,0x98,0x94,0x9e,0x30,0x0e,
  0x9f,0x9f,0x6d,0x99,0x9e,0x8e,0x24,0x55,0xa9,0xaa,0xa9,0x47,0xad,0xae,0x22,0x09,
  0x00,0x3b
};

/* created by reswrap from file questionicon.gif */
const unsigned char questionicon[]={
  0x47,0x49,0x46,0x38,0x37,0x61,0x20,0x00,0x20,0x00,0xf2,0x00,0x00,0x80,0x80,0x80,
  0xc0,0xc0,0xc0,0xff,0xff,0xff,0x00,0x00,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x2c,0x00,0x00,0x00,0x00,0x20,0x00,0x20,0x00,0x00,0x03,
  0x97,0x08,0xba,0xdc,0xfe,0xf0,0x05,0x41,0x6b,0x88,0x58,0xcd,0xca,0xf9,0xcd,0xcc,
  0xd6,0x8d,0x16,0x08,0x90,0x02,0x30,0x0c,0x80,0xc8,0x11,0x18,0x19,0xac,0xf4,0xe0,
  0x56,0xf0,0x23,0xdb,0x54,0x7d,0x53,0xb9,0x10,0x69,0xc5,0xa9,0xc9,0x82,0x0a,0x94,
  0x51,0xb0,0x1c,0x21,0x7f,0x3d,0x9a,0x60,0x26,0x3d,0x2e,0x50,0xd3,0xd5,0xa4,0x49,
  0x0a,0x62,0x3d,0x55,0xac,0xf7,0x1b,0x1d,0x90,0x05,0x63,0xb2,0xf6,0x9c,0xfe,0x12,
  0xd9,0x0b,0xe8,0xd9,0x7a,0xfd,0x52,0xe5,0xb8,0x06,0x9e,0x19,0xa6,0x33,0xdc,0x7d,
  0x4e,0x0f,0x04,0x58,0x77,0x62,0x11,0x84,0x73,0x82,0x18,0x04,0x7b,0x23,0x01,0x48,
  0x88,0x8d,0x67,0x90,0x91,0x19,0x04,0x98,0x3f,0x95,0x96,0x26,0x0a,0x98,0x93,0x40,
  0x9d,0x26,0x89,0x68,0xa2,0x20,0xa4,0x9c,0xa6,0x0d,0x89,0xa9,0xaa,0x0c,0x84,0xad,
  0xae,0x0b,0x98,0xb2,0xb5,0xb6,0xaa,0x09,0x00,0x3b
};

/* created by reswrap from file infoicon.gif */
const unsigned char infoicon[]={
  0x47,0x49,0x46,0x38,0x37,0x61,0x20,0x00,0x20,0x00,0xf2,0x00,0x00,0x80,0x80,0x80,
  0xc0,0xc0,0xc0,0xff,0xff,0xff,0x00,0x00,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x2c,0x00,0x00,0x00,0x00,0x20,0x00,0x20,0x00,0x00,0x03,
  0x8c,0x08,0xba,0xdc,0xfe,0xf0,0x05,0x41,0x6b,0x88,0x58,0xcd,0xca,0xf9,0xcd,0xcc,
  0xd6,0x8d,0x16,0x08,0x90,0xc1,0xa0,0x0e,0x22,0x47,0x60,0xa4,0xb0,0xae,0xf1,0xfb,
  0xc4,0xf2,0x3c,0xe0,0x76,0x88,0xa7,0xab,0x56,0x27,0xd0,0x53,0xe0,0x8e,0xa4,0xa2,
  0x10,0x79,0x24,0x2e,0x8e,0x3a,0x15,0xb2,0xc7,0xd4,0x4d,0x9f,0x48,0xeb,0x91,0x9a,
  0x9d,0x5d,0x8d,0x5d,0xda,0x76,0xb1,0xec,0x68,0x63,0x4e,0x2c,0xee,0x9c,0x6c,0x94,
  0x2b,0xec,0x61,0x11,0x3c,0x8a,0x8a,0x3b,0x73,0x05,0x81,0x39,0x86,0xec,0xf9,0x23,
  0x79,0x0d,0x04,0x6f,0x68,0x82,0x0e,0x04,0x84,0x4c,0x44,0x87,0x7e,0x89,0x4b,0x8c,
  0x8d,0x20,0x89,0x8a,0x14,0x92,0x26,0x0c,0x7f,0x02,0x97,0x98,0x7a,0x15,0x9c,0x9d,
  0x00,0x7f,0xa0,0xa1,0x7b,0xa4,0xa5,0xa7,0xa1,0xaa,0xab,0x19,0x09,0x00,0x3b
};

/*from Goggles Music Manager
thanks Sander Jansen
Fill horizontal gradient rectangle */
void fillHorizontalGradient(FXDCWindow &dc, FXint x, FXint y, FXint w, FXint h, FXColor left, FXColor right)
{
    register FXint rr,gg,bb,dr,dg,db,r1,g1,b1,r2,g2,b2,xl,xh,xx,dx,n,t;
    if(0<w && 0<h)
    {
        r1=FXREDVAL(left);
        r2=FXREDVAL(right);
        g1=FXGREENVAL(left);
        g2=FXGREENVAL(right);
        b1=FXBLUEVAL(left);
        b2=FXBLUEVAL(right);
        dr=r2-r1;
        dg=g2-g1;
        db=b2-b1;
        n=FXABS(dr);
        if((t=FXABS(dg))>n) n=t;
        if((t=FXABS(db))>n) n=t;
        n++;
        if(n>w) n=w;
        if(n>128) n=128;
        rr=(r1<<16)+32767;
        gg=(g1<<16)+32767;
        bb=(b1<<16)+32767;
        xx=32767;
        dr=(dr<<16)/n;
        dg=(dg<<16)/n;
        db=(db<<16)/n;
        dx=(w<<16)/n;
        do
        {
            xl=xx>>16;
            xx+=dx;
            xh=xx>>16;
            dc.setForeground(FXRGB(rr>>16,gg>>16,bb>>16));
            dc.fillRectangle(x+xl,y,xh-xl,h);
            rr+=dr;
            gg+=dg;
            bb+=db;
        } while(xh<w);
    }
}

/*from Goggles Music Manager
thanks Sander Jansen
Fill vertical gradient rectangle */
void fillVerticalGradient(FXDCWindow &dc, FXint x, FXint y, FXint w, FXint h, FXColor top, FXColor bottom)
{
    register FXint rr,gg,bb,dr,dg,db,r1,g1,b1,r2,g2,b2,yl,yh,yy,dy,n,t;
    if(0<w && 0<h)
    {
        r1=FXREDVAL(top);
        r2=FXREDVAL(bottom);
        g1=FXGREENVAL(top);
        g2=FXGREENVAL(bottom);
        b1=FXBLUEVAL(top);
        b2=FXBLUEVAL(bottom);
        dr=r2-r1;
        dg=g2-g1;
        db=b2-b1;
        n=FXABS(dr);
        if((t=FXABS(dg))>n) n=t;
        if((t=FXABS(db))>n) n=t;
        n++;
        if(n>h) n=h;
        if(n>128) n=128;
        rr=(r1<<16)+32767;
        gg=(g1<<16)+32767;
        bb=(b1<<16)+32767;
        yy=32767;
        dr=(dr<<16)/n;
        dg=(dg<<16)/n;
        db=(db<<16)/n;
        dy=(h<<16)/n;
        do
        {
            yl=yy>>16;
            yy+=dy;
            yh=yy>>16;
            dc.setForeground(FXRGB(rr>>16,gg>>16,bb>>16));
            dc.fillRectangle(x,y+yl,w,yh-yl);
            rr+=dr;
            gg+=dg;
            bb+=db;
        } while(yh<h);
    }
}

FXDEFMAP(dxEXButton) dxEXButtonMap[]={
    FXMAPFUNC(SEL_PAINT,0,dxEXButton::onPaint)
};

FXIMPLEMENT(dxEXButton,FXButton,dxEXButtonMap,ARRAYNUMBER(dxEXButtonMap))

dxEXButton::dxEXButton()
{
}

dxEXButton::dxEXButton(FXComposite* p,const FXString& text,FXIcon* ic,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb) : FXButton(p,text,ic,tgt,sel,opts,x,y,w,h,pl,pr,pt,pb)
{
}

// Handle repaint
long dxEXButton::onPaint(FXObject*, FXSelector, void* ptr)
{
    FXint tw = 0, th = 0, iw = 0, ih = 0, tx, ty, ix, iy;
    FXEvent*ev = (FXEvent*) ptr;
    FXDCWindow dc(this, ev);

    //  shadowColor         = gm_make_shadow_color(baseColor);
    FXColor top = makeHiliteColor(backColor);
    FXColor bottom = makeHiliteColor(shadowColor);
    FXColor shade = makeHiliteColor(shadowColor);
    FXColor bordercolor = shadowColor;

    FXPoint basebackground[4] = {FXPoint(0, 0), FXPoint(width - 1, 0), FXPoint(0, height - 1), FXPoint(width - 1, height - 1)};

    FXPoint bordershade[16] = {FXPoint(0, 1), FXPoint(1, 0), FXPoint(1, 2), FXPoint(2, 1),
    FXPoint(width - 2, 0), FXPoint(width - 1, 1), FXPoint(width - 3, 1), FXPoint(width - 2, 2),
    FXPoint(0, height - 2), FXPoint(1, height - 1), FXPoint(1, height - 3), FXPoint(2, height - 2),
    FXPoint(width - 1, height - 2), FXPoint(width - 2, height - 1), FXPoint(width - 2, height - 3), FXPoint(width - 3, height - 2)};
    FXPoint bordercorners[4] = {FXPoint(1, 1), FXPoint(1, height - 2), FXPoint(width - 2, 1), FXPoint(width - 2, height - 2)};

    if(options & BUTTON_TOOLBAR && (!underCursor() || !isEnabled()))
    {
        dc.setForeground(baseColor);
        dc.fillRectangle(0, 0, width, height);
    }
    else if(state == STATE_UP && ((options & BUTTON_TOOLBAR) == 0 || (options & BUTTON_TOOLBAR && underCursor())))
    {

        /// Outside Background
        dc.setForeground(baseColor);
        dc.drawPoints(basebackground, 4);

        /// Border
        dc.setForeground(bordercolor);
        dc.drawRectangle(2, 0, width - 5, 0);
        dc.drawRectangle(2, height - 1, width - 5, height - 1);
        dc.drawRectangle(0, 2, 0, height - 5);
        dc.drawRectangle(width - 1, 2, 0, height - 5);
        dc.drawPoints(bordercorners, 4);
        dc.setForeground(shade);
        dc.drawPoints(bordershade, 16);

        fillVerticalGradient(dc, 2, 1, width - 4, height - 2, top, bottom);
        dc.setForeground(top);
        dc.drawRectangle(1, 3, 0, height - 7);
        dc.setForeground(bottom);
        dc.drawRectangle(width - 2, 3, 0, height - 7);
    }
    else
    {
        /// Outside Background
        dc.setForeground(baseColor);
        dc.drawPoints(basebackground, 4);

        /// Border
        dc.setForeground(bordercolor);
        dc.drawRectangle(2, 0, width - 5, 0);
        dc.drawRectangle(2, height - 1, width - 5, height - 1);
        dc.drawRectangle(0, 2, 0, height - 5);
        dc.drawRectangle(width - 1, 2, 0, height - 5);
        dc.drawPoints(bordercorners, 4);
        dc.setForeground(shade);
        dc.drawPoints(bordershade, 16);

        dc.setForeground(baseColor);
        dc.fillRectangle(2, 1, width - 4, height - 2);


        //dc.setForeground(FXRGB(0xdc,0xd4,0xc9));
        //dc.fillRectangle(2,1,width-4,height-2);
    }

    // Place text & icon
    if(!label.empty())
    {
        tw = labelWidth(label);
        th = labelHeight(label);
    }
    if(icon)
    {
        iw = icon->getWidth();
        ih = icon->getHeight();
    }

    just_x(tx, ix, tw, iw);
    just_y(ty, iy, th, ih);

    // Shift a bit when pressed
    if(state && (options & (FRAME_RAISED | FRAME_SUNKEN)))
    {
        ++tx;
        ++ty;
        ++ix;
        ++iy;
    }

    // Draw enabled state
    if(isEnabled())
    {
        if(icon)
        {
            dc.drawIcon(icon, ix, iy);
        }
        if(!label.empty())
        {
            dc.setFont(font);
            dc.setForeground(textColor);
            drawLabel(dc, label, hotoff, tx, ty, tw, th);
        }
        if(hasFocus())
        {
            dc.drawFocusRectangle(border + 1, border + 1, width - 2 * border - 2, height - 2 * border - 2);
        }
    }
    // Draw grayed-out state
    else
    {
        if(icon)
        {
            dc.drawIconSunken(icon, ix, iy);
        }
        if(!label.empty())
        {
            dc.setFont(font);
            dc.setForeground(hiliteColor);
            drawLabel(dc, label, hotoff, tx + 1, ty + 1, tw, th);
            dc.setForeground(shadowColor);
            drawLabel(dc, label, hotoff, tx, ty, tw, th);
        }
    }
    return 1;
}

FXDEFMAP(dxEXToggleButton) dxEXToggleButtonMap[]={
    FXMAPFUNC(SEL_PAINT,0,dxEXToggleButton::onPaint)
};

FXIMPLEMENT(dxEXToggleButton,FXToggleButton,dxEXToggleButtonMap,ARRAYNUMBER(dxEXToggleButtonMap))


dxEXToggleButton::dxEXToggleButton()
{
}

dxEXToggleButton::dxEXToggleButton(FXComposite* p,const FXString& text1,const FXString& text2,FXIcon* ic1,FXIcon* ic2,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb) : FXToggleButton(p,text1,text2,ic1,ic2,tgt,sel,opts,x,y,w,h,pl,pr,pt,pb)
{
}

// Handle repaint
long dxEXToggleButton::onPaint(FXObject*, FXSelector, void* ptr)
{
    FXint tw = 0, th = 0, iw = 0, ih = 0, tx, ty, ix, iy;
    FXEvent *ev = (FXEvent*) ptr;
    FXDCWindow dc(this, ev);

    FXColor top = makeHiliteColor(backColor);
    FXColor bottom = makeHiliteColor(shadowColor);
    FXColor shade = makeHiliteColor(shadowColor);
    FXColor bordercolor = shadowColor;

    FXPoint basebackground[4] = {FXPoint(0, 0), FXPoint(width - 1, 0), FXPoint(0, height - 1), FXPoint(width - 1, height - 1)};

    FXPoint bordershade[16] = {FXPoint(0, 1), FXPoint(1, 0), FXPoint(1, 2), FXPoint(2, 1),
    FXPoint(width - 2, 0), FXPoint(width - 1, 1), FXPoint(width - 3, 1), FXPoint(width - 2, 2),
    FXPoint(0, height - 2), FXPoint(1, height - 1), FXPoint(1, height - 3), FXPoint(2, height - 2),
    FXPoint(width - 1, height - 2), FXPoint(width - 2, height - 1), FXPoint(width - 2, height - 3), FXPoint(width - 3, height - 2)};
    FXPoint bordercorners[4] = {FXPoint(1, 1), FXPoint(1, height - 2), FXPoint(width - 2, 1), FXPoint(width - 2, height - 2)};

    // Got a border at all?
    if(options & (FRAME_RAISED | FRAME_SUNKEN))
    {
        // Toolbar style
        if(options & TOGGLEBUTTON_TOOLBAR)
        {
            // Enabled and cursor inside and down
            if(down || ((options & TOGGLEBUTTON_KEEPSTATE) && state))
            {
                /// Outside Background
                dc.setForeground(baseColor);
                dc.drawPoints(basebackground, 4);

                /// Border
                dc.setForeground(bordercolor);
                dc.drawRectangle(2, 0, width - 5, 0);
                dc.drawRectangle(2, height - 1, width - 5, height - 1);
                dc.drawRectangle(0, 2, 0, height - 5);
                dc.drawRectangle(width - 1, 2, 0, height - 5);
                dc.drawPoints(bordercorners, 4);
                dc.setForeground(shade);
                dc.drawPoints(bordershade, 16);

                dc.setForeground(baseColor);
                dc.fillRectangle(2, 1, width - 4, height - 2);
            }
            // Enabled and cursor inside, and up
            else if(isEnabled() && underCursor())
            {
                /// Outside Background
                dc.setForeground(baseColor);
                dc.drawPoints(basebackground, 4);

                /// Border
                dc.setForeground(bordercolor);
                dc.drawRectangle(2, 0, width - 5, 0);
                dc.drawRectangle(2, height - 1, width - 5, height - 1);
                dc.drawRectangle(0, 2, 0, height - 5);
                dc.drawRectangle(width - 1, 2, 0, height - 5);
                dc.drawPoints(bordercorners, 4);
                dc.setForeground(shade);
                dc.drawPoints(bordershade, 16);

                fillVerticalGradient(dc, 2, 1, width - 4, height - 2, top, bottom);
                dc.setForeground(top);
                dc.drawRectangle(1, 3, 0, height - 7);
                dc.setForeground(bottom);
                dc.drawRectangle(width - 2, 3, 0, height - 7);
            }
            // Disabled or unchecked or not under cursor
            else
            {
                dc.setForeground(backColor);
                dc.fillRectangle(0, 0, width, height);
            }
        }
        // Normal style
        else
        {
            // Draw sunken if pressed
            if(down || ((options & TOGGLEBUTTON_KEEPSTATE) && state))
            {
                dc.setForeground(hiliteColor);
                dc.fillRectangle(border, border, width - border * 2, height - border * 2);
                if (options & FRAME_THICK) drawDoubleSunkenRectangle(dc, 0, 0, width, height);
                else drawSunkenRectangle(dc, 0, 0, width, height);
            }
            // Draw raised if not currently pressed down
            else
            {
                dc.setForeground(backColor);
                dc.fillRectangle(border, border, width - border * 2, height - border * 2);
                if(options & FRAME_THICK) drawDoubleRaisedRectangle(dc, 0, 0, width, height);
                else drawRaisedRectangle(dc, 0, 0, width, height);
            }

        }
    }
    // No borders
    else
    {
        dc.setForeground(backColor);
        dc.fillRectangle(0, 0, width, height);
    }

    // Place text & icon
    if(state && !altlabel.empty())
    {
        tw = labelWidth(altlabel);
        th = labelHeight(altlabel);
    }
    else if(!label.empty())
    {
        tw = labelWidth(label);
        th = labelHeight(label);
    }
    if(state && alticon)
    {
        iw = alticon->getWidth();
        ih = alticon->getHeight();
    }
    else if(icon)
    {
        iw = icon->getWidth();
        ih = icon->getHeight();
    }

    just_x(tx, ix, tw, iw);
    just_y(ty, iy, th, ih);

    // Shift a bit when pressed
    if((down || ((options & TOGGLEBUTTON_KEEPSTATE) && state)) && (options & (FRAME_RAISED | FRAME_SUNKEN)))
    {
        ++tx;
        ++ty;
        ++ix;
        ++iy;
    }

    // Draw enabled state
    if(isEnabled())
    {
        if(state && alticon)
        {
            dc.drawIcon(alticon, ix, iy);
        }
        else if(icon)
        {
            dc.drawIcon(icon, ix, iy);
        }
        if(state && !altlabel.empty())
        {
            dc.setFont(font);
            dc.setForeground(textColor);
            drawLabel(dc, altlabel, althotoff, tx, ty, tw, th);
        }
        else if(!label.empty())
        {
            dc.setFont(font);
            dc.setForeground(textColor);
            drawLabel(dc, label, hotoff, tx, ty, tw, th);
        }
        if (hasFocus())
        {
            dc.drawFocusRectangle(border + 1, border + 1, width - 2 * border - 2, height - 2 * border - 2);
        }
    }
    // Draw grayed-out state
    else
    {
        if(state && alticon)
        {
            dc.drawIconSunken(alticon, ix, iy);
        }
        else if(icon)
        {
            dc.drawIconSunken(icon, ix, iy);
        }
        if(state && !altlabel.empty())
        {
            dc.setFont(font);
            dc.setForeground(hiliteColor);
            drawLabel(dc, altlabel, althotoff, tx + 1, ty + 1, tw, th);
            dc.setForeground(shadowColor);
            drawLabel(dc, altlabel, althotoff, tx, ty, tw, th);
        }
        else if(!label.empty())
        {
            dc.setFont(font);
            dc.setForeground(hiliteColor);
            drawLabel(dc, label, hotoff, tx + 1, ty + 1, tw, th);
            dc.setForeground(shadowColor);
            drawLabel(dc, label, hotoff, tx, ty, tw, th);
        }
    }
    return 1;
}

FXDEFMAP(dxEXTabItem) dxEXTabItemMap[]={
    FXMAPFUNC(SEL_PAINT,0,dxEXTabItem::onPaint),
};

FXIMPLEMENT(dxEXTabItem,FXTabItem,dxEXTabItemMap,ARRAYNUMBER(dxEXTabItemMap))

dxEXTabItem::dxEXTabItem(FXTabBar* p,const FXString& text,FXIcon* ic,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint /*pt*/,FXint /*pb*/) : FXTabItem(p,text,ic,opts,x,y,w,h,pl,pr,5,5)
{
}

long dxEXTabItem::onPaint(FXObject*,FXSelector,void*)
{
    FXTabBook *bar = (FXTabBook*)getParent();
    FXint tab = bar->indexOfChild(this)/2;
    FXint ntabs = (bar->numChildren()/2);
    FXint ctab = bar->getCurrent();
    FXDCWindow dc(this);
    FXint tw=0,th=0,iw=0,ih=0,tx,ty,ix,iy;
    dc.setForeground(backColor);
    dc.fillRectangle(0,0,width,height);
    switch(options&TAB_ORIENT_MASK){
        case TAB_TOP:
            dc.setForeground(shadowColor);
            dc.drawLine(0,0,width,0);
            dc.drawLine(width-1,0,width-1,height-2);
            if(tab==ctab || tab==0)
            {
                if(tab==ctab) dc.setForeground(hiliteColor);
                dc.drawLine(0,1,0,tab==0?height-1:height-2);
            }
            else
            {
                dc.setForeground(makeHiliteColor(shadowColor));
                dc.drawLine(0,1,0,height-2);
            }
            break;
        case TAB_BOTTOM:
            dc.setForeground(shadowColor);
            dc.drawLine(0,height-1,width,height-1);
            dc.drawLine(width-1,1,width-1,height-2);
            if(tab==ctab || tab==0)
            {
                if(tab==ctab) dc.setForeground(hiliteColor);
                dc.drawLine(0,tab==0?0:1,0,height-2);
            }
            else
            {
                dc.setForeground(makeHiliteColor(shadowColor));
                dc.drawLine(0,1,0,height-2);
            }
            break;
        case TAB_LEFT:
            dc.setForeground(shadowColor);
            dc.drawLine(0,0,0,height);
            dc.drawLine(0,height-1,width-2,height-1);
            if(tab==ctab || tab==0)
            {
                if(tab==ctab) dc.setForeground(hiliteColor);
                dc.drawLine(1,0,tab==0?width-1:width-2,0);
            }
            else
            {
                dc.setForeground(makeHiliteColor(shadowColor));
                dc.drawLine(1,0,width-2,0);
            }
            break;
        case TAB_RIGHT:
            dc.setForeground(shadowColor);
            dc.drawLine(width-1,0,width-1,height);
            dc.drawLine(1,height-1,width-2,height-1);
            if(tab==ctab || tab==0)
            {
                if(tab==ctab) dc.setForeground(hiliteColor);
                dc.drawLine(tab==0?0:1,0,width-2,0);
            }
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

FXDEFMAP(dxEXMessageBox) dxEXMessageBoxMap[]={
    FXMAPFUNC(SEL_COMMAND, dxEXMessageBox::ID_CANCEL,dxEXMessageBox::onCmdCancel),
    FXMAPFUNCS(SEL_COMMAND,dxEXMessageBox::ID_CLICKED_YES,dxEXMessageBox::ID_CLICKED_SKIPALL,dxEXMessageBox::onCmdClicked)
};

FXIMPLEMENT(dxEXMessageBox,FXDialogBox,dxEXMessageBoxMap,ARRAYNUMBER(dxEXMessageBoxMap))

// Construct message box with given caption, icon, and message text
dxEXMessageBox::dxEXMessageBox(FXWindow* owner,const FXString& caption,const FXString& text,FXIcon* ic,FXuint opts,FXint x,FXint y):
  FXDialogBox(owner,caption,opts|DECOR_TITLE|DECOR_BORDER,x,y,0,0, 0,0,0,0, 4,4)
{
    initialize(text,ic,opts&MBOX_BUTTON_MASK);
}


// Construct free floating message box with given caption, icon, and message text
dxEXMessageBox::dxEXMessageBox(FXApp* a,const FXString& caption,const FXString& text,FXIcon* ic,FXuint opts,FXint x,FXint y):
  FXDialogBox(a,caption,opts|DECOR_TITLE|DECOR_BORDER,x,y,0,0, 0,0,0,0, 4,4)
{
    initialize(text,ic,opts&MBOX_BUTTON_MASK);
}


// Build contents
void dxEXMessageBox::initialize(const FXString& text, FXIcon* ic, FXuint whichbuttons) {
    dxEXButton *initial;
    FXVerticalFrame* content = new FXVerticalFrame(this, LAYOUT_FILL_X | LAYOUT_FILL_Y);
    FXHorizontalFrame* info = new FXHorizontalFrame(content, LAYOUT_TOP | LAYOUT_LEFT | LAYOUT_FILL_X | LAYOUT_FILL_Y, 0, 0, 0, 0, 10, 10, 10, 10);
    new FXLabel(info, FXString::null, ic, ICON_BEFORE_TEXT | LAYOUT_TOP | LAYOUT_LEFT | LAYOUT_FILL_X | LAYOUT_FILL_Y);
    new FXLabel(info, text, NULL, JUSTIFY_LEFT | ICON_BEFORE_TEXT | LAYOUT_TOP | LAYOUT_LEFT | LAYOUT_FILL_X | LAYOUT_FILL_Y);
    new FXHorizontalSeparator(content, SEPARATOR_GROOVE | LAYOUT_TOP | LAYOUT_LEFT | LAYOUT_FILL_X);
    FXHorizontalFrame* buttons = new FXHorizontalFrame(content, LAYOUT_TOP | LAYOUT_LEFT | LAYOUT_FILL_X | PACK_UNIFORM_WIDTH, 0, 0, 0, 0, 10, 10, 5, 5);
    if(whichbuttons == MBOX_OK)
    {
        initial = new dxEXButton(buttons, tr("&OK"), NULL, this, ID_CLICKED_OK, BUTTON_INITIAL | BUTTON_DEFAULT | FRAME_RAISED | FRAME_THICK | LAYOUT_TOP | LAYOUT_LEFT | LAYOUT_CENTER_X, 0, 0, 0, 0, HORZ_PAD, HORZ_PAD, VERT_PAD, VERT_PAD);
        initial->setFocus();
    }
    else if(whichbuttons == MBOX_OK_CANCEL)
    {
        initial = new dxEXButton(buttons, tr("&OK"), NULL, this, ID_CLICKED_OK, BUTTON_INITIAL | BUTTON_DEFAULT | FRAME_RAISED | FRAME_THICK | LAYOUT_TOP | LAYOUT_LEFT | LAYOUT_CENTER_X, 0, 0, 0, 0, HORZ_PAD, HORZ_PAD, VERT_PAD, VERT_PAD);
        new dxEXButton(buttons, tr("&Cancel"), NULL, this, ID_CLICKED_CANCEL, BUTTON_DEFAULT | FRAME_RAISED | FRAME_THICK | LAYOUT_TOP | LAYOUT_LEFT | LAYOUT_CENTER_X, 0, 0, 0, 0, HORZ_PAD, HORZ_PAD, VERT_PAD, VERT_PAD);
        initial->setFocus();
    }
    else if(whichbuttons == MBOX_YES_NO)
    {
        initial = new dxEXButton(buttons, tr("&Yes"), NULL, this, ID_CLICKED_YES, BUTTON_INITIAL | BUTTON_DEFAULT | FRAME_RAISED | FRAME_THICK | LAYOUT_TOP | LAYOUT_LEFT | LAYOUT_CENTER_X, 0, 0, 0, 0, HORZ_PAD, HORZ_PAD, VERT_PAD, VERT_PAD);
        new dxEXButton(buttons, tr("&No"), NULL, this, ID_CLICKED_NO, BUTTON_DEFAULT | FRAME_RAISED | FRAME_THICK | LAYOUT_TOP | LAYOUT_LEFT | LAYOUT_CENTER_X, 0, 0, 0, 0, HORZ_PAD, HORZ_PAD, VERT_PAD, VERT_PAD);
        initial->setFocus();
    }
    else if(whichbuttons == MBOX_YES_NO_CANCEL)
    {
        initial = new dxEXButton(buttons, tr("&Yes"), NULL, this, ID_CLICKED_YES, BUTTON_INITIAL | BUTTON_DEFAULT | FRAME_RAISED | FRAME_THICK | LAYOUT_TOP | LAYOUT_LEFT | LAYOUT_CENTER_X, 0, 0, 0, 0, HORZ_PAD, HORZ_PAD, VERT_PAD, VERT_PAD);
        new dxEXButton(buttons, tr("&No"), NULL, this, ID_CLICKED_NO, BUTTON_DEFAULT | FRAME_RAISED | FRAME_THICK | LAYOUT_TOP | LAYOUT_LEFT | LAYOUT_CENTER_X, 0, 0, 0, 0, HORZ_PAD, HORZ_PAD, VERT_PAD, VERT_PAD);
        new dxEXButton(buttons, tr("&Cancel"), NULL, this, ID_CLICKED_CANCEL, BUTTON_DEFAULT | FRAME_RAISED | FRAME_THICK | LAYOUT_TOP | LAYOUT_LEFT | LAYOUT_CENTER_X, 0, 0, 0, 0, HORZ_PAD, HORZ_PAD, VERT_PAD, VERT_PAD);
        initial->setFocus();
    }
    else if(whichbuttons == MBOX_QUIT_CANCEL)
    {
        initial = new dxEXButton(buttons, tr("&Quit"), NULL, this, ID_CLICKED_QUIT, BUTTON_INITIAL | BUTTON_DEFAULT | BUTTON_DEFAULT | FRAME_RAISED | FRAME_THICK | LAYOUT_TOP | LAYOUT_LEFT | LAYOUT_CENTER_X, 0, 0, 0, 0, HORZ_PAD, HORZ_PAD, VERT_PAD, VERT_PAD);
        new dxEXButton(buttons, tr("&Cancel"), NULL, this, ID_CLICKED_CANCEL, BUTTON_DEFAULT | FRAME_RAISED | FRAME_THICK | LAYOUT_TOP | LAYOUT_LEFT | LAYOUT_CENTER_X, 0, 0, 0, 0, HORZ_PAD, HORZ_PAD, VERT_PAD, VERT_PAD);
        initial->setFocus();
    }
    else if(whichbuttons == MBOX_QUIT_SAVE_CANCEL)
    {
        new dxEXButton(buttons, tr("&Quit"), NULL, this, ID_CLICKED_QUIT, BUTTON_DEFAULT | FRAME_RAISED | FRAME_THICK | LAYOUT_TOP | LAYOUT_LEFT | LAYOUT_CENTER_X, 0, 0, 0, 0, HORZ_PAD, HORZ_PAD, VERT_PAD, VERT_PAD);
        initial = new dxEXButton(buttons, tr("&Save"), NULL, this, ID_CLICKED_SAVE, BUTTON_INITIAL | BUTTON_DEFAULT | FRAME_RAISED | FRAME_THICK | LAYOUT_TOP | LAYOUT_LEFT | LAYOUT_CENTER_X, 0, 0, 0, 0, HORZ_PAD, HORZ_PAD, VERT_PAD, VERT_PAD);
        new dxEXButton(buttons, tr("&Cancel"), NULL, this, ID_CLICKED_CANCEL, BUTTON_DEFAULT | FRAME_RAISED | FRAME_THICK | LAYOUT_TOP | LAYOUT_LEFT | LAYOUT_CENTER_X, 0, 0, 0, 0, HORZ_PAD, HORZ_PAD, VERT_PAD, VERT_PAD);
        initial->setFocus();
    }
    else if(whichbuttons == MBOX_SKIP_SKIPALL_CANCEL)
    {
        initial = new dxEXButton(buttons, tr("&Skip"), NULL, this, ID_CLICKED_SKIP, BUTTON_INITIAL | BUTTON_DEFAULT | FRAME_RAISED | FRAME_THICK | LAYOUT_TOP | LAYOUT_LEFT | LAYOUT_CENTER_X, 0, 0, 0, 0, HORZ_PAD, HORZ_PAD, VERT_PAD, VERT_PAD);
        new dxEXButton(buttons, tr("Skip &All"), NULL, this, ID_CLICKED_SKIPALL, BUTTON_DEFAULT | FRAME_RAISED | FRAME_THICK | LAYOUT_TOP | LAYOUT_LEFT | LAYOUT_CENTER_X, 0, 0, 0, 0, HORZ_PAD, HORZ_PAD, VERT_PAD, VERT_PAD);
        new dxEXButton(buttons, tr("&Cancel"), NULL, this, ID_CLICKED_CANCEL, BUTTON_DEFAULT | FRAME_RAISED | FRAME_THICK | LAYOUT_TOP | LAYOUT_LEFT | LAYOUT_CENTER_X, 0, 0, 0, 0, HORZ_PAD, HORZ_PAD, VERT_PAD, VERT_PAD);
        initial->setFocus();
    }
    else if (whichbuttons == MBOX_SAVE_CANCEL_DONTSAVE)
    {
        buttons->setPackingHints(PACK_NORMAL);
        new dxEXButton(buttons, tr("&Don't Save"), NULL, this, ID_CLICKED_NO, BUTTON_DEFAULT | FRAME_RAISED | FRAME_THICK | LAYOUT_TOP | LAYOUT_CENTER_X, 0, 0, 0, 0, 15, 15, VERT_PAD, VERT_PAD);
        FXHorizontalFrame *buttons3 = new FXHorizontalFrame(buttons, LAYOUT_RIGHT | PACK_UNIFORM_WIDTH, 0, 0, 0, 0, 0, 0, 0, 0);
        new dxEXButton(buttons3, tr("&Cancel"), NULL, this, ID_CLICKED_CANCEL, BUTTON_DEFAULT | FRAME_RAISED | FRAME_THICK | LAYOUT_TOP | LAYOUT_LEFT, 0, 0, 0, 0, 15, 15, VERT_PAD, VERT_PAD);
        initial = new dxEXButton(buttons3, tr("&Save"), NULL, this, ID_CLICKED_SAVE, BUTTON_INITIAL | BUTTON_DEFAULT | FRAME_RAISED | FRAME_THICK | LAYOUT_TOP | LAYOUT_LEFT, 0, 0, 0, 0, 15, 15, VERT_PAD, VERT_PAD);
        initial->setFocus();
    }
}

// Close dialog with a cancel
long dxEXMessageBox::onCmdClicked(FXObject*,FXSelector sel,void*)
{
    getApp()->stopModal(this,MBOX_CLICKED_YES+(FXSELID(sel)-ID_CLICKED_YES));
    hide();
    return 1;
}

// Close dialog with a cancel
long dxEXMessageBox::onCmdCancel(FXObject* sender,FXSelector,void* ptr)
{
    return dxEXMessageBox::onCmdClicked(sender,FXSEL(SEL_COMMAND,ID_CLICKED_CANCEL),ptr);
}

FXuint dxEXMessageBox::error(FXWindow* owner,FXuint opts,const char* caption,const char* message,...)
{
    FXGIFIcon icon(owner->getApp(),erroricon);
    va_list arguments;
    va_start(arguments,message);
    dxEXMessageBox box(owner,caption,FXStringVFormat(message,arguments),&icon,opts|DECOR_TITLE|DECOR_BORDER);
    va_end(arguments);
    return box.execute(PLACEMENT_OWNER);
}


// Show a modal error message, in free floating window
FXuint dxEXMessageBox::error(FXApp* app,FXuint opts,const char* caption,const char* message,...)
{
    FXGIFIcon icon(app,erroricon);
    va_list arguments;
    va_start(arguments,message);
    dxEXMessageBox box(app,caption,FXStringVFormat(message,arguments),&icon,opts|DECOR_TITLE|DECOR_BORDER);
    va_end(arguments);
    return box.execute(PLACEMENT_SCREEN);
}



// Show a modal warning message
FXuint dxEXMessageBox::warning(FXWindow* owner,FXuint opts,const char* caption,const char* message,...)
{
    FXGIFIcon icon(owner->getApp(),warningicon);
    va_list arguments;
    va_start(arguments,message);
    dxEXMessageBox box(owner,caption,FXStringVFormat(message,arguments),&icon,opts|DECOR_TITLE|DECOR_BORDER);
    va_end(arguments);
    return box.execute(PLACEMENT_OWNER);
}


// Show a modal warning message, in free floating window
FXuint dxEXMessageBox::warning(FXApp* app,FXuint opts,const char* caption,const char* message,...)
{
    FXGIFIcon icon(app,warningicon);
    va_list arguments;
    va_start(arguments,message);
    dxEXMessageBox box(app,caption,FXStringVFormat(message,arguments),&icon,opts|DECOR_TITLE|DECOR_BORDER);
    va_end(arguments);
    return box.execute(PLACEMENT_SCREEN);
}


// Show a modal question dialog
FXuint dxEXMessageBox::question(FXWindow* owner,FXuint opts,const char* caption,const char* message,...)
{
    FXGIFIcon icon(owner->getApp(),questionicon);
    va_list arguments;
    va_start(arguments,message);
    dxEXMessageBox box(owner,caption,FXStringVFormat(message,arguments),&icon,opts|DECOR_TITLE|DECOR_BORDER);
    va_end(arguments);
    return box.execute(PLACEMENT_OWNER);
}


// Show a modal question dialog, in free floating window
FXuint dxEXMessageBox::question(FXApp* app,FXuint opts,const char* caption,const char* message,...)
{
    FXGIFIcon icon(app,questionicon);
    va_list arguments;
    va_start(arguments,message);
    dxEXMessageBox box(app,caption,FXStringVFormat(message,arguments),&icon,opts|DECOR_TITLE|DECOR_BORDER);
    va_end(arguments);
    return box.execute(PLACEMENT_SCREEN);
}


// Show a modal information dialog
FXuint dxEXMessageBox::information(FXWindow* owner,FXuint opts,const char* caption,const char* message,...)
{
    FXGIFIcon icon(owner->getApp(),infoicon);
    va_list arguments;
    va_start(arguments,message);
    dxEXMessageBox box(owner,caption,FXStringVFormat(message,arguments),&icon,opts|DECOR_TITLE|DECOR_BORDER);
    va_end(arguments);
    return box.execute(PLACEMENT_OWNER);
}


// Show a modal information dialog, in free floating window
FXuint dxEXMessageBox::information(FXApp* app,FXuint opts,const char* caption,const char* message,...)
{
    FXGIFIcon icon(app,infoicon);
    va_list arguments;
    va_start(arguments,message);
    dxEXMessageBox box(app,caption,FXStringVFormat(message,arguments),&icon,opts|DECOR_TITLE|DECOR_BORDER);
    va_end(arguments);
    return box.execute(PLACEMENT_SCREEN);
}
