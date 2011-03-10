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
#define HORZ_PAD 20
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

// This's from Xfe, thanks
// Draw rectangle with gradient effect
// Default is vertical gradient
static void drawGradientRectangle(FXDC& dc,FXColor upper,FXColor lower,FXint x,FXint y,FXint w,FXint h, FXbool vert=TRUE)
{
    register FXint rr,gg,bb,dr,dg,db,r1,g1,b1,r2,g2,b2,yl,yh,yy,dy,n,t,ww;
    const FXint MAXSTEPS=128;
    if (0<w && 0<h)
    {
        // Horizontal gradient : exchange w and h
        if (!vert)
        {
            ww=w;
            w=h;
            h=ww;
        }

        dc.setStipple(STIPPLE_NONE);
        dc.setFillStyle(FILL_SOLID);

        r1=FXREDVAL(upper);
        r2=FXREDVAL(lower);
        dr=r2-r1;
        g1=FXGREENVAL(upper);
        g2=FXGREENVAL(lower);
        dg=g2-g1;
        b1=FXBLUEVAL(upper);
        b2=FXBLUEVAL(lower);
        db=b2-b1;

        n=FXABS(dr);
        if ((t=FXABS(dg))>n)
            n=t;
        if ((t=FXABS(db))>n)
            n=t;
        n++;
        if (n>h)
            n=h;
        if (n>MAXSTEPS)
            n=MAXSTEPS;
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

            // Vertical gradient
            if (vert)
                dc.fillRectangle(x,y+yl,w,yh-yl);

            // Horizontal gradient
            else
                dc.fillRectangle(x+yl,y,yh-yl,w);

            rr+=dr;
            gg+=dg;
            bb+=db;
        }
        while (yh<h);
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
        dc.setForeground(baseColor);
        dc.fillRectangle(0, 0, width, height);

        /// Outside Background
        dc.setForeground(backColor);
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

        drawGradientRectangle(dc, top, bottom, 2, 1, width-4, height-2);
        dc.setForeground(top);
        dc.drawRectangle(1, 3, 0, height - 7);
        dc.setForeground(bottom);
        dc.drawRectangle(width - 2, 3, 0, height - 7);
    }
    else
    {
        dc.setForeground(FXRGB(0.9*FXREDVAL(backColor),0.9*FXGREENVAL(backColor),0.9*FXBLUEVAL(backColor)));
        dc.fillRectangle(0,0,width,height);

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

                drawGradientRectangle(dc, top, bottom, 2, 1, width-4, height-2);
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
                dc.drawLine(0,1,0,tab==0?height:height-2);
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
            dc.drawLine(width-1,1,width-1,height-1);
            if(tab==ctab || tab==0)
            {
                if(tab==ctab) dc.setForeground(hiliteColor);
                dc.drawLine(0,tab==0?0:1,0,height-2);
            }
            else
            {
                dc.setForeground(makeHiliteColor(shadowColor));
                dc.drawLine(0,1,0,height-1);
            }
            break;
        case TAB_LEFT:
            dc.setForeground(shadowColor);
            dc.drawLine(0,0,0,height);
            dc.drawLine(0,height-1,width-2,height-1);
            if(tab==ctab || tab==0)
            {
                if(tab==ctab) dc.setForeground(hiliteColor);
                dc.drawLine(1,0,tab==0?width:width-2,0);
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
            drawGradientRectangle(dc,makeHiliteColor(shadowColor),makeHiliteColor(shadowColor),1,1,width-2,height-2);
        else
        {
            switch(options&TAB_ORIENT_MASK){
                case TAB_LEFT:
                    drawGradientRectangle(dc,makeHiliteColor(backColor),backColor,1,1,width-2,height-2,FALSE);
                    break;
                case TAB_RIGHT:
                    drawGradientRectangle(dc,backColor,makeHiliteColor(backColor),1,1,width-2,height-2,FALSE);
                    break;
                case TAB_TOP:
                    drawGradientRectangle(dc,makeHiliteColor(backColor),backColor,1,1,width-2,height-2);
                    break;
                case TAB_BOTTOM:
                    drawGradientRectangle(dc,backColor,makeHiliteColor(backColor),1,1,width-2,height-2);
                    break;
            }
        }
    }
    else
    {
        drawGradientRectangle(dc,makeHiliteColor(shadowColor),makeHiliteColor(shadowColor),1,1,width-2,height-2);
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

// Object implementation
FXIMPLEMENT(dxEXInputDialog,FXInputDialog,NULL,0)

// Create input dialog box
dxEXInputDialog::dxEXInputDialog(FXWindow* owner,const FXString& caption,const FXString& label,FXIcon* icon,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXInputDialog(owner,caption,label,icon,opts,x,y,w,h)
{
    initialize(label,icon);
}

// Create free floating input dialog box
dxEXInputDialog::dxEXInputDialog(FXApp* app,const FXString& caption,const FXString& label,FXIcon* icon,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXInputDialog(app,caption,label,icon,opts,x,y,w,h)
{
    initialize(label,icon);
}

// Build contents
void dxEXInputDialog::initialize(const FXString& label,FXIcon* icon)
{
    FXWindow * window=NULL;
    while((window=getFirst())!=NULL) delete window;
    
    FXuint textopts=TEXTFIELD_ENTER_ONLY|FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X;
    FXHorizontalFrame* buttons=new FXHorizontalFrame(this,LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X|PACK_UNIFORM_WIDTH,0,0,0,0,0,0,0,0);
    new dxEXButton(buttons,tr("&OK"),NULL,this,ID_ACCEPT,BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_Y|LAYOUT_RIGHT,0,0,0,0,HORZ_PAD,HORZ_PAD,VERT_PAD,VERT_PAD);
    new dxEXButton(buttons,tr("&Cancel"),NULL,this,ID_CANCEL,BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_Y|LAYOUT_RIGHT,0,0,0,0,HORZ_PAD,HORZ_PAD,VERT_PAD,VERT_PAD);
    new FXHorizontalSeparator(this,SEPARATOR_GROOVE|LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X);
    FXHorizontalFrame* toppart=new FXHorizontalFrame(this,LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_CENTER_Y,0,0,0,0, 0,0,0,0, 10,10);
    new FXLabel(toppart,FXString::null,icon,ICON_BEFORE_TEXT|JUSTIFY_CENTER_X|JUSTIFY_CENTER_Y|LAYOUT_FILL_Y|LAYOUT_FILL_X);
    FXVerticalFrame* entry=new FXVerticalFrame(toppart,LAYOUT_FILL_X|LAYOUT_CENTER_Y,0,0,0,0, 0,0,0,0);
    new FXLabel(entry,label,NULL,JUSTIFY_LEFT|ICON_BEFORE_TEXT|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FILL_X);
    if(options&INPUTDIALOG_PASSWORD) textopts|=TEXTFIELD_PASSWD;
    if(options&INPUTDIALOG_INTEGER) textopts|=TEXTFIELD_INTEGER|JUSTIFY_RIGHT;
    if(options&INPUTDIALOG_REAL) textopts|=TEXTFIELD_REAL|JUSTIFY_RIGHT;
    input=new FXTextField(entry,20,this,ID_ACCEPT,textopts,0,0,0,0, 8,8,4,4);
    limlo=1.0;
    limhi=0.0;
}

// Obtain a string
FXbool dxEXInputDialog::getString(FXString& result,FXWindow* owner,const FXString& caption,const FXString& label,FXIcon* icon)
{
    dxEXInputDialog inputdialog(owner,caption,label,icon,INPUTDIALOG_STRING,0,0,0,0);
    inputdialog.setText(result);
    if(inputdialog.execute())
    {
        result=inputdialog.getText();
        return TRUE;
    }
    return FALSE;
}

// Obtain a string, in free floating window
FXbool dxEXInputDialog::getString(FXString& result,FXApp* app,const FXString& caption,const FXString& label,FXIcon* icon)
{
    dxEXInputDialog inputdialog(app,caption,label,icon,INPUTDIALOG_STRING,0,0,0,0);
    inputdialog.setText(result);
    if(inputdialog.execute())
    {
        result=inputdialog.getText();
        return TRUE;
    }
    return FALSE;
}

// Obtain an integer
FXbool dxEXInputDialog::getInteger(FXint& result,FXWindow* owner,const FXString& caption,const FXString& label,FXIcon* icon,FXint lo,FXint hi)
{
    dxEXInputDialog inputdialog(owner,caption,label,icon,INPUTDIALOG_INTEGER,0,0,0,0);
    inputdialog.setLimits(lo,hi);
    inputdialog.setText(FXStringVal(FXCLAMP(lo,result,hi)));
    if(inputdialog.execute())
    {
        result=FXIntVal(inputdialog.getText());
        return TRUE;
    }
    return FALSE;
}

// Obtain an integer, in free floating window
FXbool dxEXInputDialog::getInteger(FXint& result,FXApp* app,const FXString& caption,const FXString& label,FXIcon* icon,FXint lo,FXint hi)
{
    dxEXInputDialog inputdialog(app,caption,label,icon,INPUTDIALOG_INTEGER,0,0,0,0);
    inputdialog.setLimits(lo,hi);
    inputdialog.setText(FXStringVal(FXCLAMP(lo,result,hi)));
    if(inputdialog.execute())
    {
        result=FXIntVal(inputdialog.getText());
        return TRUE;
    }
    return FALSE;
}

// Obtain a real
FXbool dxEXInputDialog::getReal(FXdouble& result,FXWindow* owner,const FXString& caption,const FXString& label,FXIcon* icon,FXdouble lo,FXdouble hi)
{
    dxEXInputDialog inputdialog(owner,caption,label,icon,INPUTDIALOG_REAL,0,0,0,0);
    inputdialog.setLimits(lo,hi);
    inputdialog.setText(FXStringVal(FXCLAMP(lo,result,hi),10));
    if(inputdialog.execute())
    {
        result=FXDoubleVal(inputdialog.getText());
        return TRUE;
    }
    return FALSE;
}

// Obtain a real, in free floating window
FXbool dxEXInputDialog::getReal(FXdouble& result,FXApp* app,const FXString& caption,const FXString& label,FXIcon* icon,FXdouble lo,FXdouble hi)
{
    dxEXInputDialog inputdialog(app,caption,label,icon,INPUTDIALOG_REAL,0,0,0,0);
    inputdialog.setLimits(lo,hi);
    inputdialog.setText(FXStringVal(FXCLAMP(lo,result,hi),10));
    if(inputdialog.execute())
    {
        result=FXDoubleVal(inputdialog.getText());
        return TRUE;
    }
    return FALSE;
}

class dxEXFileSelector : public FXFileSelector
{
    FXDECLARE(dxEXFileSelector)
protected:
    dxEXFileSelector(){}
private:
    dxEXFileSelector(const dxEXFileSelector&);
    dxEXFileSelector &operator=(const dxEXFileSelector&);
public:
    dxEXFileSelector(FXComposite *p,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0);

    long onCmdNew(FXObject*,FXSelector,void*);
    long onCmdDelete(FXObject*,FXSelector,void*);
    long onCmdMove(FXObject*,FXSelector,void*);
    long onCmdCopy(FXObject*,FXSelector,void*);
    long onCmdLink(FXObject*,FXSelector,void*);
};

FXDEFMAP(dxEXFileSelector) dxEXFileSelectorMap[]={
    FXMAPFUNC(SEL_COMMAND,FXFileSelector::ID_NEW,dxEXFileSelector::onCmdNew),
    FXMAPFUNC(SEL_COMMAND,FXFileSelector::ID_DELETE,dxEXFileSelector::onCmdDelete),
    FXMAPFUNC(SEL_COMMAND,FXFileSelector::ID_MOVE,dxEXFileSelector::onCmdMove),
    FXMAPFUNC(SEL_COMMAND,FXFileSelector::ID_COPY,dxEXFileSelector::onCmdCopy),
    FXMAPFUNC(SEL_COMMAND,FXFileSelector::ID_LINK,dxEXFileSelector::onCmdLink)
};

FXIMPLEMENT(dxEXFileSelector,FXFileSelector,dxEXFileSelectorMap,ARRAYNUMBER(dxEXFileSelectorMap));

dxEXFileSelector::dxEXFileSelector(FXComposite *p,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h):FXFileSelector(p,tgt,sel,opts,x,y,w,h)
{
    FXWindow * window=NULL;
    while((window=getFirst())!=NULL) delete window;
    delete bookmarkmenu; bookmarkmenu=NULL;

    FXAccelTable *table=getShell()->getAccelTable();

    navbuttons=new FXHorizontalFrame(this,LAYOUT_SIDE_TOP|LAYOUT_FILL_X,0,0,0,0, 0,0,0,0, 0,0);
    entryblock=new FXMatrix(this,3,MATRIX_BY_COLUMNS|LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X,0,0,0,0,0,0,0,0);
    new FXLabel(entryblock,tr("&File Name:"),NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y);
    filename=new FXTextField(entryblock,25,this,ID_ACCEPT,TEXTFIELD_ENTER_ONLY|LAYOUT_FILL_COLUMN|LAYOUT_FILL_X|FRAME_SUNKEN|FRAME_THICK|LAYOUT_CENTER_Y);
    new dxEXButton(entryblock,tr("&OK"),NULL,this,ID_ACCEPT,BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_FILL_X,0,0,0,0,20,20);
    accept=new FXButton(navbuttons,FXString::null,NULL,NULL,0,LAYOUT_FIX_X|LAYOUT_FIX_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT,0,0,0,0, 0,0,0,0);
    new FXLabel(entryblock,tr("File F&ilter:"),NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y);

    FXHorizontalFrame *filterframe=new FXHorizontalFrame(entryblock,LAYOUT_FILL_COLUMN|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 0,0,0,0);
    filefilter=new FXComboBox(filterframe,10,this,ID_FILEFILTER,COMBOBOX_STATIC|LAYOUT_FILL_X|FRAME_SUNKEN|FRAME_THICK|LAYOUT_CENTER_Y);
    filefilter->setNumVisible(4);
    readonly=new FXCheckButton(filterframe,tr("Read Only"),NULL,0,ICON_BEFORE_TEXT|JUSTIFY_LEFT|LAYOUT_CENTER_Y);
    cancel=new dxEXButton(entryblock,tr("&Cancel"),NULL,NULL,0,BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_FILL_X,0,0,0,0,20,20);

    fileboxframe=new FXHorizontalFrame(this,LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN|FRAME_THICK,0,0,0,0,0,0,0,0);
    filebox=new FXFileList(fileboxframe,this,ID_FILELIST,ICONLIST_MINI_ICONS|ICONLIST_BROWSESELECT|ICONLIST_AUTOSIZE|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(navbuttons,tr("Directory:"),NULL,LAYOUT_CENTER_Y);
    dirbox=new FXDirBox(navbuttons,this,ID_DIRTREE,DIRBOX_NO_OWN_ASSOC|FRAME_LINE|LAYOUT_FILL_X|LAYOUT_CENTER_Y,0,0,0,0,1,1,1,1);
    dirbox->setNumVisible(5);
    dirbox->setAssociations(filebox->getAssociations());

    bookmarkmenu=new FXMenuPane(this,POPUP_SHRINKWRAP);
    new FXMenuCommand(bookmarkmenu,tr("&Set bookmark\t\tBookmark current directory."),markicon,this,ID_BOOKMARK);
    new FXMenuCommand(bookmarkmenu,tr("&Clear bookmarks\t\tClear bookmarks."),clearicon,&bookmarks,FXRecentFiles::ID_CLEAR);
    FXMenuSeparator* sep1=new FXMenuSeparator(bookmarkmenu);
    sep1->setTarget(&bookmarks);
    sep1->setSelector(FXRecentFiles::ID_ANYFILES);
    new FXMenuCommand(bookmarkmenu,FXString::null,NULL,&bookmarks,FXRecentFiles::ID_FILE_1);
    new FXMenuCommand(bookmarkmenu,FXString::null,NULL,&bookmarks,FXRecentFiles::ID_FILE_2);
    new FXMenuCommand(bookmarkmenu,FXString::null,NULL,&bookmarks,FXRecentFiles::ID_FILE_3);
    new FXMenuCommand(bookmarkmenu,FXString::null,NULL,&bookmarks,FXRecentFiles::ID_FILE_4);
    new FXMenuCommand(bookmarkmenu,FXString::null,NULL,&bookmarks,FXRecentFiles::ID_FILE_5);
    new FXMenuCommand(bookmarkmenu,FXString::null,NULL,&bookmarks,FXRecentFiles::ID_FILE_6);
    new FXMenuCommand(bookmarkmenu,FXString::null,NULL,&bookmarks,FXRecentFiles::ID_FILE_7);
    new FXMenuCommand(bookmarkmenu,FXString::null,NULL,&bookmarks,FXRecentFiles::ID_FILE_8);
    new FXMenuCommand(bookmarkmenu,FXString::null,NULL,&bookmarks,FXRecentFiles::ID_FILE_9);
    new FXMenuCommand(bookmarkmenu,FXString::null,NULL,&bookmarks,FXRecentFiles::ID_FILE_10);
    new FXFrame(navbuttons,LAYOUT_FIX_WIDTH,0,0,4,1);
    new dxEXButton(navbuttons,tr("\tGo up one directory\tMove up to higher directory."),updiricon,this,ID_DIRECTORY_UP,BUTTON_TOOLBAR|FRAME_RAISED,0,0,0,0, 3,3,3,3);
    new dxEXButton(navbuttons,tr("\tGo to home directory\tBack to home directory."),homeicon,this,ID_HOME,BUTTON_TOOLBAR|FRAME_RAISED,0,0,0,0, 3,3,3,3);
    new dxEXButton(navbuttons,tr("\tGo to work directory\tBack to working directory."),workicon,this,ID_WORK,BUTTON_TOOLBAR|FRAME_RAISED,0,0,0,0, 3,3,3,3);
    FXMenuButton *bookmenu=new FXMenuButton(navbuttons,tr("\tBookmarks\tVisit bookmarked directories."),markicon,bookmarkmenu,MENUBUTTON_NOARROWS|MENUBUTTON_ATTACH_LEFT|MENUBUTTON_TOOLBAR|FRAME_RAISED,0,0,0,0, 3,3,3,3);
    bookmenu->setTarget(this);
    bookmenu->setSelector(ID_BOOKMENU);
    new dxEXButton(navbuttons,tr("\tCreate new directory\tCreate new directory."),newicon,this,ID_NEW,BUTTON_TOOLBAR|FRAME_RAISED,0,0,0,0, 3,3,3,3);
    new dxEXButton(navbuttons,tr("\tShow list\tDisplay directory with small icons."),listicon,filebox,FXFileList::ID_SHOW_MINI_ICONS,BUTTON_TOOLBAR|FRAME_RAISED,0,0,0,0, 3,3,3,3);
    new dxEXButton(navbuttons,tr("\tShow icons\tDisplay directory with big icons."),iconsicon,filebox,FXFileList::ID_SHOW_BIG_ICONS,BUTTON_TOOLBAR|FRAME_RAISED,0,0,0,0, 3,3,3,3);
    new dxEXButton(navbuttons,tr("\tShow details\tDisplay detailed directory listing."),detailicon,filebox,FXFileList::ID_SHOW_DETAILS,BUTTON_TOOLBAR|FRAME_RAISED,0,0,0,0, 3,3,3,3);
    new dxEXToggleButton(navbuttons,tr("\tShow hidden files\tShow hidden files and directories."),tr("\tHide Hidden Files\tHide hidden files and directories."),hiddenicon,shownicon,filebox,FXFileList::ID_TOGGLE_HIDDEN,TOGGLEBUTTON_TOOLBAR|FRAME_RAISED,0,0,0,0, 3,3,3,3);
    bookmarks.setTarget(this);
    bookmarks.setSelector(ID_VISIT);
    readonly->hide();
    if(table)
    {
        table->addAccel(MKUINT(KEY_BackSpace,0),this,FXSEL(SEL_COMMAND,ID_DIRECTORY_UP));
        table->addAccel(MKUINT(KEY_Delete,0),this,FXSEL(SEL_COMMAND,ID_DELETE));
        table->addAccel(MKUINT(KEY_h,CONTROLMASK),this,FXSEL(SEL_COMMAND,ID_HOME));
        table->addAccel(MKUINT(KEY_w,CONTROLMASK),this,FXSEL(SEL_COMMAND,ID_WORK));
        table->addAccel(MKUINT(KEY_n,CONTROLMASK),this,FXSEL(SEL_COMMAND,ID_NEW));
        table->addAccel(MKUINT(KEY_a,CONTROLMASK),filebox,FXSEL(SEL_COMMAND,FXFileList::ID_SELECT_ALL));
        table->addAccel(MKUINT(KEY_b,CONTROLMASK),filebox,FXSEL(SEL_COMMAND,FXFileList::ID_SHOW_BIG_ICONS));
        table->addAccel(MKUINT(KEY_s,CONTROLMASK),filebox,FXSEL(SEL_COMMAND,FXFileList::ID_SHOW_MINI_ICONS));
        table->addAccel(MKUINT(KEY_l,CONTROLMASK),filebox,FXSEL(SEL_COMMAND,FXFileList::ID_SHOW_DETAILS));
    }
    setSelectMode(SELECTFILE_ANY);    // For backward compatibility, this HAS to be the default!
    //setPatternList(allfiles);
    setDirectory(FXSystem::getCurrentDirectory());
    filebox->setFocus();
    accept->hide();
    navigable=TRUE;
}

// Create new directory
long dxEXFileSelector::onCmdNew(FXObject*,FXSelector,void*)
{
    FXString dir=filebox->getDirectory();
    FXString name="DirectoryName";
    if(dxEXInputDialog::getString(name,this,tr("Create New Directory"),tr("Create new directory with name: ")))
    {
        FXString dirname=FXPath::absolute(dir,name);
        if(FXStat::exists(dirname))
        {
            dxEXMessageBox::error(this,MBOX_OK,tr("Already Exists"),tr("File or directory %s already exists.\n"),dirname.text());
            return 1;
        }
        if(!FXDir::create(dirname))
        {
            dxEXMessageBox::error(this,MBOX_OK,tr("Cannot Create"),tr("Cannot create directory %s.\n"),dirname.text());
            return 1;
        }
        setDirectory(dirname);
    }
    return 1;
}

// Copy file or directory
long dxEXFileSelector::onCmdCopy(FXObject*,FXSelector,void*)
{
    FXString *filenamelist=getSelectedFiles();
    FXString copymessage;
    if(filenamelist)
    {
        for(FXint i=0; !filenamelist[i].empty(); i++)
        {
            copymessage.format(tr("Copy file from location:\n\n%s\n\nto location: "),filenamelist[i].text());
            dxEXInputDialog inputdialog(this,tr("Copy File"),copymessage,NULL,INPUTDIALOG_STRING,0,0,0,0);
            inputdialog.setText(FXPath::absolute(FXPath::directory(filenamelist[i]),"CopyOf"+FXPath::name(filenamelist[i])));
            inputdialog.setNumColumns(60);
            if(inputdialog.execute())
            {
                FXString newname=inputdialog.getText();
                if(!FXFile::copyFiles(filenamelist[i],newname,FALSE))
                {
                    if(dxEXMessageBox::error(this,MBOX_YES_NO,tr("Error Copying File"),tr("Unable to copy file:\n\n%s  to:  %s\n\nContinue with operation?"),filenamelist[i].text(),newname.text())==MBOX_CLICKED_NO) break;
                }
            }
        }
        delete [] filenamelist;
    }
    return 1;
}

// Move file or directory
long dxEXFileSelector::onCmdMove(FXObject*,FXSelector,void*)
{
    FXString *filenamelist=getSelectedFiles();
    FXString movemessage;
    if(filenamelist)
    {
        for(FXint i=0; !filenamelist[i].empty(); i++)
        {
            movemessage.format(tr("Move file from location:\n\n%s\n\nto location: "),filenamelist[i].text());
            dxEXInputDialog inputdialog(this,tr("Move File"),movemessage,NULL,INPUTDIALOG_STRING,0,0,0,0);
            inputdialog.setText(filenamelist[i]);
            inputdialog.setNumColumns(60);
            if(inputdialog.execute())
            {
                FXString newname=inputdialog.getText();
                if(!FXFile::moveFiles(filenamelist[i],newname,FALSE))
                {
                    if(dxEXMessageBox::error(this,MBOX_YES_NO,tr("Error Moving File"),tr("Unable to move file:\n\n%s  to:  %s\n\nContinue with operation?"),filenamelist[i].text(),newname.text())==MBOX_CLICKED_NO) break;
                }
            }
        }
        delete [] filenamelist;
    }
    return 1;
}

// Link file or directory
long dxEXFileSelector::onCmdLink(FXObject*,FXSelector,void*)
{
    FXString *filenamelist=getSelectedFiles();
    FXString linkmessage;
    if(filenamelist)
    {
        for(FXint i=0; !filenamelist[i].empty(); i++)
        {
            linkmessage.format(tr("Link file from location:\n\n%s\n\nto location: "),filenamelist[i].text());
            dxEXInputDialog inputdialog(this,tr("Link File"),linkmessage,NULL,INPUTDIALOG_STRING,0,0,0,0);
            inputdialog.setText(FXPath::absolute(FXPath::directory(filenamelist[i]),"LinkTo"+FXPath::name(filenamelist[i])));
            inputdialog.setNumColumns(60);
            if(inputdialog.execute())
            {
                FXString newname=inputdialog.getText();
                if(!FXFile::symlink(filenamelist[i],newname))
                {
                    if(dxEXMessageBox::error(this,MBOX_YES_NO,tr("Error Linking File"),tr("Unable to link file:\n\n%s  to:  %s\n\nContinue with operation?"),filenamelist[i].text(),newname.text())==MBOX_CLICKED_NO) break;
                }
            }
        }
        delete [] filenamelist;
    }
    return 1;
}

// Delete file or directory
long dxEXFileSelector::onCmdDelete(FXObject*,FXSelector,void*)
{
    FXString *filenamelist=getSelectedFiles();
    FXuint answer;
    if(filenamelist)
    {
        for(FXint i=0; !filenamelist[i].empty(); i++)
        {
            answer=dxEXMessageBox::warning(this,MBOX_YES_NO_CANCEL,tr("Deleting files"),tr("Are you sure you want to delete the file:\n\n%s"),filenamelist[i].text());
            if(answer==MBOX_CLICKED_CANCEL) break;
            if(answer==MBOX_CLICKED_NO) continue;
            if(!FXFile::removeFiles(filenamelist[i],TRUE))
            {
                if(dxEXMessageBox::error(this,MBOX_YES_NO,tr("Error Deleting File"),tr("Unable to delete file:\n\n%s\n\nContinue with operation?"),filenamelist[i].text())==MBOX_CLICKED_NO) break;
            }
        }
        delete [] filenamelist;
    }
    return 1;
}

FXIMPLEMENT(dxEXFileDialog,FXFileDialog,NULL,0);

// Construct file fialog box
dxEXFileDialog::dxEXFileDialog(FXWindow* owner,const FXString& name,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXFileDialog(owner,name,opts,x,y,w,h)
{
    delete filebox;
    filebox=new dxEXFileSelector(this,NULL,0,LAYOUT_FILL_X|LAYOUT_FILL_Y);
    filebox->acceptButton()->setTarget(this);
    filebox->acceptButton()->setSelector(FXDialogBox::ID_ACCEPT);
    filebox->cancelButton()->setTarget(this);
    filebox->cancelButton()->setSelector(FXDialogBox::ID_CANCEL);
}

// Construct free-floating file dialog box
dxEXFileDialog::dxEXFileDialog(FXApp* a,const FXString& name,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXFileDialog(a,name,opts,x,y,w,h)
{
    delete filebox;
    filebox=new dxEXFileSelector(this,NULL,0,LAYOUT_FILL_X|LAYOUT_FILL_Y);
    filebox->acceptButton()->setTarget(this);
    filebox->acceptButton()->setSelector(FXDialogBox::ID_ACCEPT);
    filebox->cancelButton()->setTarget(this);
    filebox->cancelButton()->setSelector(FXDialogBox::ID_CANCEL);
}

class dxEXDirSelector : public FXDirSelector
{
    FXDECLARE(dxEXDirSelector)
protected:
    dxEXDirSelector(){}
private:
    dxEXDirSelector(const dxEXDirSelector&);
    dxEXDirSelector &operator=(const dxEXDirSelector&);
public:
    dxEXDirSelector(FXComposite *p,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0);

    long onCmdNew(FXObject*,FXSelector,void*);
    long onCmdDelete(FXObject*,FXSelector,void*);
    long onCmdMove(FXObject*,FXSelector,void*);
    long onCmdCopy(FXObject*,FXSelector,void*);
    long onCmdLink(FXObject*,FXSelector,void*);
};

FXDEFMAP(dxEXDirSelector) dxEXDirSelectorMap[]={
    FXMAPFUNC(SEL_COMMAND,FXDirSelector::ID_NEW,dxEXDirSelector::onCmdNew),
    FXMAPFUNC(SEL_COMMAND,FXDirSelector::ID_DELETE,dxEXDirSelector::onCmdDelete),
    FXMAPFUNC(SEL_COMMAND,FXDirSelector::ID_MOVE,dxEXDirSelector::onCmdMove),
    FXMAPFUNC(SEL_COMMAND,FXDirSelector::ID_COPY,dxEXDirSelector::onCmdCopy),
    FXMAPFUNC(SEL_COMMAND,FXDirSelector::ID_LINK,dxEXDirSelector::onCmdLink)
};

FXIMPLEMENT(dxEXDirSelector,FXDirSelector,dxEXDirSelectorMap,ARRAYNUMBER(dxEXDirSelectorMap));

dxEXDirSelector::dxEXDirSelector(FXComposite *p,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h):FXDirSelector(p,tgt,sel,opts,x,y,w,h)
{
    FXWindow * window=NULL;
    while((window=getFirst())!=NULL) delete window;

    FXString currentdirectory=FXSystem::getCurrentDirectory();
    FXAccelTable *table=getShell()->getAccelTable();
    FXHorizontalFrame *buttons=new FXHorizontalFrame(this,LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X|PACK_UNIFORM_WIDTH);
    accept=new dxEXButton(buttons,tr("&OK"),NULL,NULL,0,BUTTON_INITIAL|BUTTON_DEFAULT|LAYOUT_RIGHT|FRAME_RAISED|FRAME_THICK,0,0,0,0,20,20);
    cancel=new dxEXButton(buttons,tr("&Cancel"),NULL,NULL,0,BUTTON_DEFAULT|LAYOUT_RIGHT|FRAME_RAISED|FRAME_THICK,0,0,0,0,20,20);
    FXHorizontalFrame *field=new FXHorizontalFrame(this,LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X);
    new FXLabel(field,tr("&Directory:"),NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y);
    dirname=new FXTextField(field,25,this,ID_DIRNAME,LAYOUT_FILL_X|LAYOUT_CENTER_Y|FRAME_SUNKEN|FRAME_THICK);
    FXHorizontalFrame *frame=new FXHorizontalFrame(this,LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN|FRAME_THICK,0,0,0,0,0,0,0,0);
    dirbox=new FXDirList(frame,this,ID_DIRLIST,LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_TOP|TREELIST_SHOWS_LINES|TREELIST_SHOWS_BOXES|TREELIST_BROWSESELECT);
    mrufiles.setTarget(this);
    mrufiles.setSelector(ID_VISIT);
    if(table)
    {
        table->addAccel(MKUINT(KEY_BackSpace,0),this,FXSEL(SEL_COMMAND,ID_DIRECTORY_UP));
        table->addAccel(MKUINT(KEY_h,CONTROLMASK),this,FXSEL(SEL_COMMAND,ID_HOME));
        table->addAccel(MKUINT(KEY_w,CONTROLMASK),this,FXSEL(SEL_COMMAND,ID_WORK));
    }
    dirbox->setDirectory(currentdirectory);
    dirname->setText(currentdirectory);
    dirbox->setFocus();
}

// Create new directory
long dxEXDirSelector::onCmdNew(FXObject*,FXSelector,void*)
{
    FXString dir=dirbox->getDirectory();
    FXString name="DirectoryName";
    if(dxEXInputDialog::getString(name,this,tr("Create New Directory"),"Create new directory in: "+dir,NULL))
    {
        FXString dirname=FXPath::absolute(dir,name);
        if(FXStat::exists(dirname))
        {
            dxEXMessageBox::error(this,MBOX_OK,tr("Already Exists"),"File or directory %s already exists.\n",dirname.text());
            return 1;
        }
        if(!FXDir::create(dirname))
        {
            dxEXMessageBox::error(this,MBOX_OK,tr("Cannot Create"),"Cannot create directory %s.\n",dirname.text());
            return 1;
        }
        setDirectory(dirname);
    }
    return 1;
}

// Copy file or directory
long dxEXDirSelector::onCmdCopy(FXObject*,FXSelector,void*)
{
    FXString oldname=dirbox->getCurrentFile();
    FXString newname=FXPath::directory(oldname)+PATHSEPSTRING "CopyOf"+FXPath::name(oldname);
    dxEXInputDialog inputdialog(this,tr("Copy File"),"Copy file from location:\n\n"+oldname+"\n\nto location:",NULL,INPUTDIALOG_STRING,0,0,0,0);
    inputdialog.setText(newname);
    inputdialog.setNumColumns(60);
    if(inputdialog.execute())
    {
        newname=inputdialog.getText();
        if(!FXFile::copyFiles(oldname,newname,FALSE))
        {
            dxEXMessageBox::error(this,MBOX_OK,tr("Error Copying File"),"Unable to copy file:\n\n%s  to:  %s.",oldname.text(),newname.text());
        }
    }
    return 1;
}


// Move file or directory
long dxEXDirSelector::onCmdMove(FXObject*,FXSelector,void*)
{
    FXString oldname=dirbox->getCurrentFile();
    FXString newname=oldname;
    dxEXInputDialog inputdialog(this,tr("Move File"),"Move file from location:\n\n"+oldname+"\n\nto location:",NULL,INPUTDIALOG_STRING,0,0,0,0);
    inputdialog.setText(newname);
    inputdialog.setNumColumns(60);
    if(inputdialog.execute())
    {
        newname=inputdialog.getText();
        if(!FXFile::moveFiles(oldname,newname,FALSE))
        {
            dxEXMessageBox::error(this,MBOX_OK,tr("Error Moving File"),"Unable to move file:\n\n%s  to:  %s.",oldname.text(),newname.text());
        }
    }
    return 1;
}


// Link file or directory
long dxEXDirSelector::onCmdLink(FXObject*,FXSelector,void*)
{
    FXString oldname=dirbox->getCurrentFile();
    FXString newname=FXPath::directory(oldname)+PATHSEPSTRING "LinkTo"+FXPath::name(oldname);
    dxEXInputDialog inputdialog(this,tr("Link File"),"Link file from location:\n\n"+oldname+"\n\nto location:",NULL,INPUTDIALOG_STRING,0,0,0,0);
    inputdialog.setText(newname);
    inputdialog.setNumColumns(60);
    if(inputdialog.execute())
    {
        newname=inputdialog.getText();
        if(!FXFile::symlink(oldname,newname))
        {
            dxEXMessageBox::error(this,MBOX_YES_NO,tr("Error Linking File"),"Unable to link file:\n\n%s  to:  %s.",oldname.text(),newname.text());
        }
    }
    return 1;
}


// Delete file or directory
long dxEXDirSelector::onCmdDelete(FXObject*,FXSelector,void*)
{
    FXString fullname=dirbox->getCurrentFile();
    if(MBOX_CLICKED_YES==dxEXMessageBox::warning(this,MBOX_YES_NO,tr("Deleting file"),"Are you sure you want to delete the file:\n\n%s",fullname.text()))
    {
        if(!FXFile::removeFiles(fullname,TRUE))
        {
            dxEXMessageBox::error(this,MBOX_YES_NO,tr("Error Deleting File"),"Unable to delete file:\n\n%s.",fullname.text());
        }
    }
    return 1;
}

FXIMPLEMENT(dxEXDirDialog,FXDirDialog,NULL,0);

// Construct file fialog box
dxEXDirDialog::dxEXDirDialog(FXWindow* owner,const FXString& name,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXDirDialog(owner,name,opts,x,y,w,h)
{
    delete dirbox;
    dirbox=new dxEXDirSelector(this,NULL,0,LAYOUT_FILL_X|LAYOUT_FILL_Y);
    dirbox->acceptButton()->setTarget(this);
    dirbox->acceptButton()->setSelector(FXDialogBox::ID_ACCEPT);
    dirbox->cancelButton()->setTarget(this);
    dirbox->cancelButton()->setSelector(FXDialogBox::ID_CANCEL);
}

// Construct free-floating directory dialog box
dxEXDirDialog::dxEXDirDialog(FXApp* a,const FXString& name,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXDirDialog(a,name,opts,x,y,w,h)
{
    delete dirbox;
    dirbox=new dxEXDirSelector(this,NULL,0,LAYOUT_FILL_X|LAYOUT_FILL_Y);
    dirbox->acceptButton()->setTarget(this);
    dirbox->acceptButton()->setSelector(FXDialogBox::ID_ACCEPT);
    dirbox->cancelButton()->setTarget(this);
    dirbox->cancelButton()->setSelector(FXDialogBox::ID_CANCEL);
}

class dxEXFontSelector : public FXFontSelector
{
    FXDECLARE(dxEXFontSelector)
protected:
    dxEXFontSelector(){}
private:
    dxEXFontSelector(const dxEXFontSelector&);
    dxEXFontSelector &operator=(const dxEXFontSelector&);
public:
    dxEXFontSelector(FXComposite *p,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0);
};

FXIMPLEMENT(dxEXFontSelector,FXFontSelector,NULL,0);

dxEXFontSelector::dxEXFontSelector(FXComposite *p,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXFontSelector(p,tgt,sel,opts,x,y,w,h)
{
    FXWindow * window=NULL;
    while((window=getFirst())!=NULL) delete window;

    // Bottom side
    FXHorizontalFrame *buttons=new FXHorizontalFrame(this,LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X);
    accept=new dxEXButton(buttons,tr("&Accept"),NULL,NULL,0,BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT,0,0,0,0,20,20);
    cancel=new dxEXButton(buttons,tr("&Cancel"),NULL,NULL,0,BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT,0,0,0,0,20,20);

    // Left side
    FXMatrix *controls=new FXMatrix(this,3,LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FIX_HEIGHT,0,0,0,160, DEFAULT_SPACING,DEFAULT_SPACING,DEFAULT_SPACING,DEFAULT_SPACING, DEFAULT_SPACING,0);

    // Font families, to be filled later
    new FXLabel(controls,tr("&Family:"),NULL,JUSTIFY_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_COLUMN);
    family=new FXTextField(controls,10,NULL,0,TEXTFIELD_READONLY|FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_COLUMN);
    FXHorizontalFrame *familyframe=new FXHorizontalFrame(controls,FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_Y|LAYOUT_FILL_X|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW,0,0,0,0, 0,0,0,0);
    familylist=new FXList(familyframe,this,ID_FAMILY,LIST_BROWSESELECT|LAYOUT_FILL_Y|LAYOUT_FILL_X|HSCROLLER_NEVER|VSCROLLER_ALWAYS);

    // Initial focus on list
    familylist->setFocus();

    // Font weights
    new FXLabel(controls,tr("&Weight:"),NULL,JUSTIFY_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_COLUMN);
    weight=new FXTextField(controls,4,NULL,0,TEXTFIELD_READONLY|FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_COLUMN);
    FXHorizontalFrame *weightframe=new FXHorizontalFrame(controls,FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW|LAYOUT_FILL_COLUMN,0,0,0,0, 0,0,0,0);
    weightlist=new FXList(weightframe,this,ID_WEIGHT,LIST_BROWSESELECT|LAYOUT_FILL_Y|LAYOUT_FILL_X|HSCROLLER_NEVER|VSCROLLER_ALWAYS);

    // Font styles
    new FXLabel(controls,tr("&Style:"),NULL,JUSTIFY_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_COLUMN);
    style=new FXTextField(controls,6,NULL,0,TEXTFIELD_READONLY|FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_COLUMN);
    FXHorizontalFrame *styleframe=new FXHorizontalFrame(controls,FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW|LAYOUT_FILL_COLUMN,0,0,0,0, 0,0,0,0);
    stylelist=new FXList(styleframe,this,ID_STYLE,LIST_BROWSESELECT|LAYOUT_FILL_Y|LAYOUT_FILL_X|HSCROLLER_NEVER|VSCROLLER_ALWAYS);

    // Font sizes, to be filled later
    new FXLabel(controls,tr("Si&ze:"),NULL,JUSTIFY_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_COLUMN);
    size=new FXTextField(controls,2,this,ID_SIZE_TEXT,FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_COLUMN);
    FXHorizontalFrame *sizeframe=new FXHorizontalFrame(controls,FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW|LAYOUT_FILL_COLUMN,0,0,0,0, 0,0,0,0);
    sizelist=new FXList(sizeframe,this,ID_SIZE,LIST_BROWSESELECT|LAYOUT_FILL_Y|LAYOUT_FILL_X|HSCROLLER_NEVER|VSCROLLER_ALWAYS);

    FXMatrix *attributes=new FXMatrix(this,2,LAYOUT_SIDE_TOP|LAYOUT_FILL_X,0,0,0,0, DEFAULT_SPACING,DEFAULT_SPACING,DEFAULT_SPACING,DEFAULT_SPACING, DEFAULT_SPACING,0);

    // Character set choice
    new FXLabel(attributes,tr("Character Set:"),NULL,LAYOUT_CENTER_Y|LAYOUT_FILL_COLUMN);
    charset=new FXComboBox(attributes,8,this,ID_CHARSET,COMBOBOX_STATIC|FRAME_SUNKEN|FRAME_THICK|LAYOUT_CENTER_Y|LAYOUT_FILL_COLUMN);
    charset->setNumVisible(10);
    charset->appendItem(tr("Any"),(void*)0);
    charset->appendItem(tr("West European"),(void*)FONTENCODING_WESTEUROPE);
    charset->appendItem(tr("East European"),(void*)FONTENCODING_EASTEUROPE);
    charset->appendItem(tr("South European"),(void*)FONTENCODING_SOUTHEUROPE);
    charset->appendItem(tr("North European"),(void*)FONTENCODING_NORTHEUROPE);
    charset->appendItem(tr("Cyrillic"),(void*)FONTENCODING_CYRILLIC);
    charset->appendItem(tr("Arabic"),(void*)FONTENCODING_ARABIC);
    charset->appendItem(tr("Greek"),(void*)FONTENCODING_GREEK);
    charset->appendItem(tr("Hebrew"),(void*)FONTENCODING_HEBREW);
    charset->appendItem(tr("Turkish"),(void*)FONTENCODING_TURKISH);
    charset->appendItem(tr("Nordic"),(void*)FONTENCODING_NORDIC);
    charset->appendItem(tr("Thai"),(void*)FONTENCODING_THAI);
    charset->appendItem(tr("Baltic"),(void*)FONTENCODING_BALTIC);
    charset->appendItem(tr("Celtic"),(void*)FONTENCODING_CELTIC);
    charset->appendItem(tr("Russian"),(void*)FONTENCODING_KOI8);
    charset->appendItem(tr("Central European (cp1250)"),(void*)FONTENCODING_CP1250);
    charset->appendItem(tr("Russian (cp1251)"),(void*)FONTENCODING_CP1251);
    charset->appendItem(tr("Latin1 (cp1252)"),(void*)FONTENCODING_CP1252);
    charset->appendItem(tr("Greek (cp1253)"),(void*)FONTENCODING_CP1253);
    charset->appendItem(tr("Turkish (cp1254)"),(void*)FONTENCODING_CP1254);
    charset->appendItem(tr("Hebrew (cp1255)"),(void*)FONTENCODING_CP1255);
    charset->appendItem(tr("Arabic (cp1256)"),(void*)FONTENCODING_CP1256);
    charset->appendItem(tr("Baltic (cp1257)"),(void*)FONTENCODING_CP1257);
    charset->appendItem(tr("Vietnam (cp1258)"),(void*)FONTENCODING_CP1258);
    charset->appendItem(tr("Thai (cp874)"),(void*)FONTENCODING_CP874);
    charset->appendItem(tr("UNICODE"),(void*)FONTENCODING_UNICODE);
    charset->setCurrentItem(0);

    // Set width
    new FXLabel(attributes,tr("Set Width:"),NULL,LAYOUT_CENTER_Y|LAYOUT_FILL_COLUMN);
    setwidth=new FXComboBox(attributes,9,this,ID_SETWIDTH,COMBOBOX_STATIC|FRAME_SUNKEN|FRAME_THICK|LAYOUT_CENTER_Y|LAYOUT_FILL_COLUMN);
    setwidth->setNumVisible(10);
    setwidth->appendItem(tr("Any"),(void*)0);
    setwidth->appendItem(tr("Ultra condensed"),(void*)FXFont::UltraCondensed);
    setwidth->appendItem(tr("Extra condensed"),(void*)FXFont::ExtraCondensed);
    setwidth->appendItem(tr("Condensed"),(void*)FXFont::Condensed);
    setwidth->appendItem(tr("Semi condensed"),(void*)FXFont::SemiCondensed);
    setwidth->appendItem(tr("Normal"),(void*)FXFont::NonExpanded);
    setwidth->appendItem(tr("Semi expanded"),(void*)FXFont::SemiExpanded);
    setwidth->appendItem(tr("Expanded"),(void*)FXFont::Expanded);
    setwidth->appendItem(tr("Extra expanded"),(void*)FXFont::ExtraExpanded);
    setwidth->appendItem(tr("Ultra expanded"),(void*)FXFont::UltraExpanded);
    setwidth->setCurrentItem(0);

    // Pitch
    new FXLabel(attributes,tr("Pitch:"),NULL,LAYOUT_CENTER_Y|LAYOUT_FILL_COLUMN);
    pitch=new FXComboBox(attributes,5,this,ID_PITCH,COMBOBOX_STATIC|FRAME_SUNKEN|FRAME_THICK|LAYOUT_CENTER_Y|LAYOUT_FILL_COLUMN);
    pitch->setNumVisible(3);
    pitch->appendItem(tr("Any"),(void*)0);
    pitch->appendItem(tr("Fixed"),(void*)FXFont::Fixed);
    pitch->appendItem(tr("Variable"),(void*)FXFont::Variable);
    pitch->setCurrentItem(0);

    // Check for scalable
    new FXFrame(attributes,FRAME_NONE|LAYOUT_FILL_COLUMN);
    scalable=new FXCheckButton(attributes,tr("Scalable:"),this,ID_SCALABLE,JUSTIFY_NORMAL|TEXT_BEFORE_ICON|LAYOUT_CENTER_Y|LAYOUT_FILL_COLUMN);

    // Check for all (X11) fonts
#ifndef WIN32
    new FXFrame(attributes,FRAME_NONE|LAYOUT_FILL_COLUMN);
    allfonts=new FXCheckButton(attributes,tr("All Fonts:"),this,ID_ALLFONTS,JUSTIFY_NORMAL|TEXT_BEFORE_ICON|LAYOUT_CENTER_Y|LAYOUT_FILL_COLUMN);
#else
    allfonts=NULL;
#endif

    // Preview
    FXVerticalFrame *bottom=new FXVerticalFrame(this,LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, DEFAULT_SPACING,DEFAULT_SPACING,DEFAULT_SPACING,DEFAULT_SPACING, 0,0);
    new FXLabel(bottom,tr("Preview:"),NULL,JUSTIFY_LEFT|LAYOUT_FILL_X);
    FXHorizontalFrame *box=new FXHorizontalFrame(bottom,LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN|FRAME_THICK,0,0,0,0, 0,0,0,0, 0,0);
    FXScrollWindow *scroll=new FXScrollWindow(box,LAYOUT_FILL_X|LAYOUT_FILL_Y);
    preview=new FXLabel(scroll,"ABCDEFGHIJKLMNOPQRSTUVWXYZ\nabcdefghijklmnopqrstuvwxyz\n0123456789",NULL,JUSTIFY_CENTER_X|JUSTIFY_CENTER_Y);
    preview->setBackColor(getApp()->getBackColor());

    strncpy(selected.face,"helvetica",sizeof(selected.face));
    selected.size=90;
    selected.weight=FXFont::Bold;
    selected.slant=0;
    selected.encoding=FONTENCODING_USASCII;
    selected.setwidth=0;
    selected.flags=0;
    previewfont=NULL;
}

FXIMPLEMENT(dxEXFontDialog,FXFontDialog,NULL,0);

// Construct file fialog box
dxEXFontDialog::dxEXFontDialog(FXWindow* owner,const FXString& name,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXFontDialog(owner,name,opts,x,y,w,h)
{
    delete fontbox;
    fontbox=new dxEXFontSelector(this,NULL,0,LAYOUT_FILL_X|LAYOUT_FILL_Y);
    fontbox->acceptButton()->setTarget(this);
    fontbox->acceptButton()->setSelector(FXDialogBox::ID_ACCEPT);
    fontbox->cancelButton()->setTarget(this);
    fontbox->cancelButton()->setSelector(FXDialogBox::ID_CANCEL);
}
