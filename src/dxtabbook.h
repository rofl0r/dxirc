/*
 *      dxtabbook.h
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

#ifndef _DXTABBOOK_H
#define _DXTABBOOK_H

#include <fx.h>

class dxTabBook : public FXTabBook
{
    FXDECLARE(dxTabBook)
public:
    dxTabBook(FXComposite* p, FXObject* tgt=NULL, FXSelector sel=0, FXuint opts=TABBOOK_NORMAL, FXint x=0, FXint y=0, FXint w=0, FXint h=0, FXint pl=DEFAULT_SPACING, FXint pr=DEFAULT_SPACING, FXint pt=DEFAULT_SPACING, FXint pb=DEFAULT_SPACING);
    enum {
        ID_BPREVIOUS = FXMainWindow::ID_LAST+200,
        ID_BNEXT,
        ID_BHIDDEN,
        ID_TAB
    };

    virtual void setCurrent(FXint panel,FXbool notify=false);
    void layout();
    FXint numTabs() const;

    long onPrevious(FXObject*, FXSelector, void*);
    long onNext(FXObject*, FXSelector, void*);
    long onHidden(FXObject*, FXSelector, void*);
    long onTab(FXObject*, FXSelector, void*);
private:
    dxTabBook() {}

    FXArrowButton *m_buttonPrevious, *m_buttonNext, *m_buttonHidden;
};

#endif	/* _DXTABBOOK_H */

