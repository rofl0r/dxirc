/*
 *      dxtabbook.cpp
 *
 *      Copyright (C) 1997,2006 by Jeroen van der Zijp.   All Rights Reserved.
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

#define TABBOOK_MASK    (TABBOOK_SIDEWAYS|TABBOOK_BOTTOMTABS)
#define REVEAL_PIXELS   20
#define TABMAX          50

FXDEFMAP(dxTabBook) dxTabBookMap[] = {
    FXMAPFUNC(SEL_COMMAND,  dxTabBook::ID_BPREVIOUS,    dxTabBook::onPrevious),
    FXMAPFUNC(SEL_COMMAND,  dxTabBook::ID_BNEXT,        dxTabBook::onNext),
    FXMAPFUNC(SEL_COMMAND,  dxTabBook::ID_BHIDDEN,       dxTabBook::onHidden),
    FXMAPFUNCS(SEL_COMMAND, dxTabBook::ID_TAB, dxTabBook::ID_TAB+TABMAX, dxTabBook::onTab)
};

FXIMPLEMENT(dxTabBook, FXTabBook, dxTabBookMap, ARRAYNUMBER(dxTabBookMap))

dxTabBook::dxTabBook(FXComposite* p, FXObject* tgt, FXSelector sel, FXuint opts)
    : FXTabBook(p, tgt, sel, opts)
{
    m_buttonPrevious = new FXArrowButton(p, this, ID_BPREVIOUS, LAYOUT_FIX_X|LAYOUT_FIX_Y|ARROW_LEFT);
    m_buttonPrevious->setArrowColor(getApp()->getForeColor());
    m_buttonNext = new FXArrowButton(p, this, ID_BNEXT, LAYOUT_FIX_X|LAYOUT_FIX_Y|ARROW_RIGHT);
    m_buttonNext->setArrowColor(getApp()->getForeColor());
    m_buttonHidden = new FXArrowButton(p, this, ID_BHIDDEN, LAYOUT_FIX_X|LAYOUT_FIX_Y|ARROW_DOWN);
    m_buttonHidden->setArrowColor(getApp()->getForeColor());
}

long dxTabBook::onPrevious(FXObject*, FXSelector, void*)
{
    setCurrent(getCurrent()-1, TRUE);
    return 1;
}

long dxTabBook::onNext(FXObject*, FXSelector, void*)
{
    if(getCurrent()+1 < numTabs())
        setCurrent(getCurrent()+1, TRUE);
    else
        setCurrent(0, TRUE);
    return 1;
}

long dxTabBook::onHidden(FXObject*, FXSelector, void*)
{
    register FXint i;
    register FXWindow *tab;
    FXMenuPane popup(m_buttonHidden->getParent());
    for(tab = getFirst(), i = 0; tab && tab->getNext(); tab = tab->getNext()->getNext(), i++)
    {
        if(!tab->shown() && i<=TABMAX)
        {
            FXMenuCommand *menu = new FXMenuCommand(&popup, static_cast<FXTabItem*>(tab)->getText(), static_cast<FXTabItem*>(tab)->getIcon(), this, ID_TAB+i);
            menu->setTextColor(static_cast<FXTabItem*>(tab)->getTextColor());
        }
    }
    FXint x, y;
    FXuint button;
    getRoot()->getCursorPosition(x, y, button);
    popup.create();
    popup.popup(NULL, x, y);
    getApp()->runModalWhileShown(&popup);
    return 1;
}

long dxTabBook::onTab(FXObject*, FXSelector sel, void*)
{
    FXint i = FXSELID(sel)-FXSELID(ID_TAB);
    setCurrent(i, TRUE);
    return 1;
}

//Return the number of tabs
FXint dxTabBook::numTabs() const
{
    return numChildren()/2;
}

void dxTabBook::setCurrent(FXint panel,FXbool notify)
{
    if(panel != current)
    {
        FXint old = current;
        FXTabBook::setCurrent(panel,notify);
        FXWindow * window = childAtIndex(old<<1);
        if(window) window->update();
    }
}

// Recalculate layout
void dxTabBook::layout() {
    register FXint i, xx, yy, x, y, w, h, px, py, pw, ph, maxtabw, maxtabh;
    register FXint currentWidth, currentHeight, tabsWidth, tabsHeight;
    register FXint previoustabsWidth, previoustabsHeight;
    register FXWindow *raisepane = NULL;
    register FXWindow *raisetab = NULL;
    register FXWindow *pane, *tab;
    register FXuint hints;
    register FXbool hidden = FALSE;

    // Measure tabs again
    maxtabw = maxtabh = currentWidth = currentHeight = tabsWidth = tabsHeight 
            = previoustabsWidth = previoustabsHeight = 0;
    for(tab = getFirst(), i = 0; tab && tab->getNext(); tab = tab->getNext()->getNext(), i++)
    {
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
            if(i == current) {
                currentWidth = w;
                currentHeight = h;
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
        for(tab = getFirst(), yy = py + shift, i = 0; tab && tab->getNext(); tab = tab->getNext()->getNext(), i++)
        {
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
                if(previoustabsHeight+currentHeight > getHeight()-FXMAX3(m_buttonPrevious->getDefaultHeight(), m_buttonNext->getDefaultHeight(), m_buttonHidden->getDefaultHeight()))
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
                if((y+h > getHeight()-FXMAX3(m_buttonPrevious->getDefaultHeight(), m_buttonNext->getDefaultHeight(), m_buttonHidden->getDefaultHeight())) || hidden) {
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
        m_buttonPrevious->position(options & TABBOOK_BOTTOMTABS ? px+pw+2 : px-m_buttonPrevious->getDefaultWidth()-m_buttonNext->getDefaultWidth()-m_buttonHidden->getDefaultWidth(), yy+6, m_buttonPrevious->getDefaultWidth(), m_buttonPrevious->getDefaultHeight());
        m_buttonNext->position(options & TABBOOK_BOTTOMTABS ? px+pw+2+m_buttonPrevious->getDefaultWidth() : px-m_buttonNext->getDefaultWidth()-m_buttonHidden->getDefaultWidth(), yy+6, m_buttonNext->getDefaultWidth(), m_buttonNext->getDefaultHeight());
        m_buttonHidden->position(options & TABBOOK_BOTTOMTABS ? px+pw+2+m_buttonPrevious->getDefaultWidth()+m_buttonNext->getDefaultWidth() : px-m_buttonHidden->getDefaultWidth(), yy+6, m_buttonHidden->getDefaultWidth(), m_buttonHidden->getDefaultHeight());
        if(yy >= tabsHeight) {
            m_buttonPrevious->hide();
            m_buttonNext->hide();
            m_buttonHidden->hide();
        }
        else {
            m_buttonPrevious->raise();
            m_buttonPrevious->show();
            m_buttonNext->raise();
            m_buttonNext->show();
            m_buttonHidden->raise();
            m_buttonHidden->show();
        }
        if(!current)
            m_buttonPrevious->disable();
        else
            m_buttonPrevious->enable();
        if(current == numTabs()-1)
            m_buttonNext->disable();
        else
            m_buttonNext->enable();
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
        for(tab = getFirst(), xx = px + shift, i = 0; tab && tab->getNext(); tab = tab->getNext()->getNext(), i++)
        {
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
                if(previoustabsWidth+currentWidth > getWidth()-m_buttonPrevious->getDefaultWidth()-m_buttonNext->getDefaultWidth()-m_buttonHidden->getDefaultWidth()-6)
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
                if((x+w > getWidth()-m_buttonPrevious->getDefaultWidth()-m_buttonNext->getDefaultWidth()-m_buttonHidden->getDefaultWidth()-6) || hidden) {
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
        m_buttonPrevious->position(getWidth()-m_buttonPrevious->getDefaultWidth()-m_buttonNext->getDefaultWidth()-m_buttonHidden->getDefaultWidth(), options & TABBOOK_BOTTOMTABS ? py+ph+2 : py-FXMAX3(m_buttonPrevious->getDefaultHeight(), m_buttonNext->getDefaultHeight(), m_buttonHidden->getDefaultHeight()), m_buttonPrevious->getDefaultWidth(), m_buttonPrevious->getDefaultHeight());
        m_buttonNext->position(getWidth()-m_buttonNext->getDefaultWidth()-m_buttonHidden->getDefaultWidth(), options & TABBOOK_BOTTOMTABS ? py+ph+2 : py-FXMAX3(m_buttonPrevious->getDefaultHeight(), m_buttonNext->getDefaultHeight(), m_buttonHidden->getDefaultHeight()), m_buttonNext->getDefaultWidth(), m_buttonNext->getDefaultHeight());
        m_buttonHidden->position(getWidth()-m_buttonHidden->getDefaultWidth(), options & TABBOOK_BOTTOMTABS ? py+ph+2 : py-FXMAX3(m_buttonPrevious->getDefaultHeight(), m_buttonNext->getDefaultHeight(), m_buttonHidden->getDefaultHeight()), m_buttonHidden->getDefaultWidth(), m_buttonHidden->getDefaultHeight());
        if(xx >= tabsWidth) {
            m_buttonPrevious->hide();
            m_buttonNext->hide();
            m_buttonHidden->hide();
        }
        else {
            m_buttonPrevious->raise();
            m_buttonPrevious->show();
            m_buttonNext->raise();
            m_buttonNext->show();
            m_buttonHidden->raise();
            m_buttonHidden->show();
        }
        if(!current)
            m_buttonPrevious->disable();
        else
            m_buttonPrevious->enable();
        if(current == numTabs()-1)
            m_buttonNext->disable();
        else
            m_buttonNext->enable();
    }

    // Raise tab over panel and panel over all other tabs
    if(raisepane) raisepane->raise();
    if(raisetab) raisetab->raise();

    flags &= ~FLAG_DIRTY;
}

