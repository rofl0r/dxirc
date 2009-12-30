/*
 *      dxtabbook.h
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

#ifndef _DXTABBOOK_H
#define	_DXTABBOOK_H

#include <fx.h>

class dxTabBook : public FXTabBook
{
    FXDECLARE(dxTabBook)
public:
    dxTabBook(FXComposite* p, FXObject* tgt=NULL, FXSelector sel=0, FXuint opts=TABBOOK_NORMAL);
    enum {
        ID_BPREVIOUS = FXMainWindow::ID_LAST+200,
        ID_BNEXT
    };

    void layout();
    FXint numTabs() const;

    long OnPrevious(FXObject*, FXSelector, void*);
    long OnNext(FXObject*, FXSelector, void*);
private:
    dxTabBook() {}

    FXButton *buttonPrevious, *buttonNext;
};

#endif	/* _DXTABBOOK_H */

