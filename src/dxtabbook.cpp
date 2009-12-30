/*
 *      dxtabbook.cpp
 *
 *      Copyright 2009 David Vachulka <david@konstrukce-cad.com>
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

#include "dxtabbook.h"

#define TABBOOK_MASK       (TABBOOK_SIDEWAYS|TABBOOK_BOTTOMTABS)
#define REVEAL_PIXELS      20

FXDEFMAP(dxTabBook) dxTabBookMap[] = {
    FXMAPFUNC(SEL_COMMAND,  dxTabBook::ID_BPREVIOUS,    dxTabBook::OnPrevious),
    FXMAPFUNC(SEL_COMMAND,  dxTabBook::ID_BNEXT,        dxTabBook::OnNext)
};

FXIMPLEMENT(dxTabBook, FXTabBook, dxTabBookMap, ARRAYNUMBER(dxTabBookMap))

dxTabBook::dxTabBook(FXComposite* p, FXObject* tgt, FXSelector sel, FXuint opts)
    : FXTabBook(p, tgt, sel, opts)
{
    buttonPrevious = new FXButton(p, "<", NULL, this, ID_BPREVIOUS, LAYOUT_FIX_X|LAYOUT_FIX_Y);
    buttonNext = new FXButton(p, ">", NULL, this, ID_BNEXT, LAYOUT_FIX_X|LAYOUT_FIX_Y);
}

long dxTabBook::OnPrevious(FXObject*, FXSelector, void*)
{
    setCurrent(getCurrent()-1, TRUE);
    return 1;
}

long dxTabBook::OnNext(FXObject*, FXSelector, void*)
{
    if(getCurrent()+1 < numTabs())
        setCurrent(getCurrent()+1, TRUE);
    else
        setCurrent(0, TRUE);
    return 1;
}

//Get number of tabs
FXint dxTabBook::numTabs() const
{
    return numChildren()/2;
}

// Recalculate layout
void dxTabBook::layout() {
    register FXint i, xx, yy, x, y, w, h, px, py, pw, ph, maxtabw, maxtabh;
    register FXint currentWidth, currentHeight, tabsWidth, tabsHeight;
    register FXint previoustabsWidth, previoustabsHeight, nexttabsHeight;
    register FXWindow *raisepane = NULL;
    register FXWindow *raisetab = NULL;
    register FXWindow *pane, *tab;
    register FXuint hints;
    register FXbool hidden = FALSE;

    // Measure tabs again
    maxtabw = maxtabh = currentWidth = currentHeight = tabsWidth = tabsHeight 
            = previoustabsWidth = previoustabsHeight 
            = nexttabsHeight= 0;
    for (tab = getFirst(), i = 0; tab && tab->getNext(); tab = tab->getNext()->getNext(), i++) {
            hints = tab->getLayoutHints();
            if (hints & LAYOUT_FIX_WIDTH) w = tab->getWidth();
            else w = tab->getDefaultWidth();
            if (hints & LAYOUT_FIX_HEIGHT) h = tab->getHeight();
            else h = tab->getDefaultHeight();
            tabsWidth += w;
            tabsHeight += h;
            if(i < current) {
                previoustabsWidth += w;
                previoustabsHeight += h;
            }
            else if(i == current) {
                currentWidth = w;
                currentHeight = h;
            }
            else {
                nexttabsHeight += h;
            }
            if (w > maxtabw) maxtabw = w;
            if (h > maxtabh) maxtabh = h;
    }

    // Left or right tabs
    if (options & TABBOOK_SIDEWAYS) {

        // Place panel
        px = (options & TABBOOK_BOTTOMTABS) ? border + padleft : border + padleft + maxtabw - 2;
        py = border + padtop;
        pw = width - padleft - padright - (border << 1) - maxtabw + 2;
        ph = height - padtop - padbottom - (border << 1);

        // Place all of the children
        shift = 0;
        for (tab = getFirst(), yy = py + shift, i = 0; tab && tab->getNext(); tab = tab->getNext()->getNext(), i++) {
            pane = tab->getNext();
            pane->position(px, py, pw, ph);
            hints = tab->getLayoutHints();
            if (hints & LAYOUT_FIX_WIDTH) w = tab->getWidth();
            else if (options & PACK_UNIFORM_WIDTH) w = maxtabw;
            else w = tab->getDefaultWidth();
            if (hints & LAYOUT_FIX_HEIGHT) h = tab->getHeight();
            else if (options & PACK_UNIFORM_HEIGHT) h = maxtabh;
            else h = tab->getDefaultHeight();
            if (i < current) {
                y = yy + 2;
                //if (y + h > py + ph - 2) y = py + ph - 2 - h;
                //if (y < py + 2) y = py + 2;
                if (options & TABBOOK_BOTTOMTABS)
                    tab->position(px + pw - 4, y + 2, w, h);
                else
                    tab->position(px - w + 4, y + 2, w, h);
                tab->raise();
                if(previoustabsHeight+currentHeight > getHeight()-buttonPrevious->getDefaultHeight()-buttonNext->getDefaultHeight())
                    tab->hide();
                else
                    tab->show();
                pane->hide();
                previoustabsHeight -= h;
                if(tab->shown()) yy += h;
            } else if (i > current) {
                y = yy + 2;
                //if (y + h > py + ph - 2) y = py + ph - h - 2;
                //if (y < py + 2) y = py + 2;
                if (options & TABBOOK_BOTTOMTABS)
                    tab->position(px + pw - 4, y + 2, w, h);
                else
                    tab->position(px - w + 4, y + 2, w, h);
                tab->lower();
                if(y+h > getHeight()-buttonPrevious->getDefaultHeight()-buttonNext->getDefaultHeight() || hidden) {
                    tab->hide();
                    hidden = TRUE;
                }
                else
                    tab->show();
                pane->hide();
                if(tab->shown()) yy += h;
            } else {
                y = yy;
                //if (y + h > py + ph - 2) y = py + ph - h - 2;
                //if (y < py) y = py;
                if (options & TABBOOK_BOTTOMTABS)
                    tab->position(px + pw - 2, y, w, h);
                else
                    tab->position(px - w + 2, y, w, h);
                tab->show();
                pane->show();
                raisepane = pane;
                raisetab = tab;
                yy += h - 3;
            }
        }
        buttonPrevious->position(options & TABBOOK_BOTTOMTABS ? px+pw+2 : px-buttonPrevious->getDefaultWidth()-buttonNext->getDefaultWidth(), yy+6, buttonPrevious->getDefaultWidth(), buttonPrevious->getDefaultHeight());
        buttonNext->position(options & TABBOOK_BOTTOMTABS ? px+pw+2+buttonPrevious->getDefaultWidth() : px-buttonNext->getDefaultWidth(), yy+6, buttonNext->getDefaultWidth(), buttonNext->getDefaultHeight());
        if(yy >= tabsHeight) {
            buttonPrevious->hide();
            buttonNext->hide();
        }
        else {
            buttonPrevious->raise();
            buttonPrevious->show();
            buttonNext->raise();
            buttonNext->show();
        }
        if(!current)
            buttonPrevious->disable();
        else
            buttonPrevious->enable();
        if(current == numTabs()-1)
            buttonNext->disable();
        else
            buttonNext->enable();
    }
    // Top or bottom tabs
    else {

        // Place panel
        px = border + padleft;
        py = (options & TABBOOK_BOTTOMTABS) ? border + padtop : border + padtop + maxtabh - 2;
        pw = width - padleft - padright - (border << 1);
        ph = height - padtop - padbottom - (border << 1) - maxtabh + 2;

        // Place all of the children
        shift = 0;
        for (tab = getFirst(), xx = px + shift, i = 0; tab && tab->getNext(); tab = tab->getNext()->getNext(), i++) {
            pane = tab->getNext();
            pane->position(px, py, pw, ph);
            hints = tab->getLayoutHints();
            if (hints & LAYOUT_FIX_WIDTH) w = tab->getWidth();
            else if (options & PACK_UNIFORM_WIDTH) w = maxtabw;
            else w = tab->getDefaultWidth();
            if (hints & LAYOUT_FIX_HEIGHT) h = tab->getHeight();
            else if (options & PACK_UNIFORM_HEIGHT) h = maxtabh;
            else h = tab->getDefaultHeight();
            if (i < current) {
                x = xx + 2;
                //if (x + w > px + pw - 2) x = px + pw - 2 - w;
                //if (x < px + 2) x = px + 2;
                if (options & TABBOOK_BOTTOMTABS)
                    tab->position(x, py + ph - 4, w, h);
                else
                    tab->position(x, py - h + 4, w, h);
                tab->raise();
                if(previoustabsWidth+currentWidth > getWidth()-buttonPrevious->getDefaultWidth()-buttonNext->getDefaultWidth())
                    tab->hide();
                else
                    tab->show();
                pane->hide();
                previoustabsWidth -= w;
                if(tab->shown()) xx += w;
            } else if (i > current) {
                x = xx + 2;
                //if (x + w > px + pw - 2) x = px + pw - w - 2;
                //if (x < px + 2) x = px + 2;
                if (options & TABBOOK_BOTTOMTABS)
                    tab->position(x, py + ph - 4, w, h);
                else
                    tab->position(x, py - h + 4, w, h);
                tab->lower();
                if(x+w > getWidth()-buttonPrevious->getDefaultWidth()-buttonNext->getDefaultWidth() || hidden) {
                    tab->hide();
                    hidden = TRUE;
                }
                else
                    tab->show();
                pane->hide();
                if(tab->shown()) xx += w;
            } else {
                x = xx;
                //if (x + w > px + pw - 2) x = px + pw - w - 2;
                //if (x < px) x = px;
                if (options & TABBOOK_BOTTOMTABS)
                    tab->position(x, py + ph - 2, w, h);
                else
                    tab->position(x, py - h + 2, w, h);
                tab->show();
                pane->show();
                raisepane = pane;
                raisetab = tab;
                xx += w - 3;
            }
        }
        buttonPrevious->position(xx+5, options & TABBOOK_BOTTOMTABS ? py+ph+2 : py-buttonPrevious->getDefaultHeight(), buttonPrevious->getDefaultWidth(), buttonPrevious->getDefaultHeight());
        buttonNext->position(xx+5+buttonPrevious->getDefaultWidth(), options & TABBOOK_BOTTOMTABS ? py+ph+2 : py-buttonPrevious->getDefaultHeight(), buttonNext->getDefaultWidth(), buttonNext->getDefaultHeight());
        if(xx >= tabsWidth) {
            buttonPrevious->hide();
            buttonNext->hide();
        }
        else {
            buttonPrevious->raise();
            buttonPrevious->show();
            buttonNext->raise();
            buttonNext->show();
        }
        if(!current)
            buttonPrevious->disable();
        else
            buttonPrevious->enable();
        if(current == numTabs()-1)
            buttonNext->disable();
        else
            buttonNext->enable();
    }

    // Raise tab over panel and panel over all other tabs
    if(raisepane) raisepane->raise();
    if(raisetab) raisetab->raise();

    flags &= ~FLAG_DIRTY;
}

