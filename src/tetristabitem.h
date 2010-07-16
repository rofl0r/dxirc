/*
 *      tetristabitem.h
 *
 *      Copyright 2009 David Vachulka <david@konstrukce-cad.com>
 *      Copyright (C) 2007-2008 Graeme Gott <graeme@gottcode.org>
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

#ifndef TETRISTABITEM_H
#define	TETRISTABITEM_H

#include "defs.h"
#include "dxtabbook.h"

static const FXint columns = 12;
static const FXint rows = 20;

struct Cell
{
    Cell(FXint x1 = -1, FXint y1 = -1)
    :	x(x1),
        y(y1)
    {
    }

    bool operator==(const Cell& cell) const
    {
        return x == cell.x && y == cell.y;
    }

    FXint x;
    FXint y;
};

class TetrisTabItem;

class Piece
{
public:
    Piece(FXint type, TetrisTabItem *parent);

    FXbool isValid() const { return m_valid; }
    FXbool moveLeft() { return move(-1, 0); }
    FXbool moveRight() { return move(1, 0); }
    FXbool moveDown() { return move(0, 1); }
    FXbool rotate();
    void drop();
    static void cells(Cell* cells, FXint type);

private:
    FXint m_type;
    TetrisTabItem *m_parent;
    Cell m_cells[4];
    Cell m_pivot;
    FXbool m_valid;

    FXbool move(FXint x, FXint y);
    FXbool updatePosition(const Cell* ucells);
};

class TetrisTabItem : public FXTabItem
{
    FXDECLARE(TetrisTabItem)
public:
    TetrisTabItem(dxTabBook*, const FXString&, FXIcon*, FXuint, FXint);
    virtual ~TetrisTabItem();
    enum {
        ID_GAMECANVAS = FXMainWindow::ID_LAST+400,
        ID_NEXTCANVAS,
        ID_TETRISTIMEOUT,
        ID_NEW,
        ID_PAUSE,
        ID_LAST
    };

    FXbool cell(FXint x, FXint y) const;
    void addCell(FXint x, FXint y, FXint type);
    void removeCell(FXint x, FXint y, FXbool update=TRUE);
    void findFullLines();
    void createGeom();
    void setColor(IrcColor);
    void setGameFocus();
    void newGame();
    void stopGame();
    void pauseResumeGame();
    void reparentTab();
    void redraw();
    void moveLeft();
    void moveRight();
    void rotate();
    void drop();
    FXbool isPauseEnable() { return m_pauseEnable; }
    FXbool isPaused() { return m_paused; }
    FXint getID() { return m_id; }

    long onPaint(FXObject*, FXSelector, void*);
    long onTimeout(FXObject*, FXSelector, void*);
    long onNewGame(FXObject*, FXSelector, void*);
    long onPauseGame(FXObject*, FXSelector, void*);

private:
    TetrisTabItem() {}

    dxTabBook *m_parent;
    FXVerticalFrame *m_mainframe, *m_gameframe, *m_otherframe;
    FXSplitter *m_splitter;
    FXCanvas *m_gamecanvas, *m_nextcanvas;
    FXLabel *m_levelLabel, *m_scoreLabel, *m_linesLabel;
    FXButton *m_newButton, *m_pauseButton;
    FXFont *m_messageFont;
    FXint m_cells[columns][rows];
    FXint m_fullLines[4];
    FXint m_apiece, m_anext;
    FXint m_removedLines, m_level, m_score, m_nextPiece, m_id;
    FXbool m_paused, m_done, m_pauseEnable;
    Piece *m_piece;
    FXColor m_penColor;

    void createPiece();
    void drawLines();
    void drawNextPiece(FXint type);
    void removeLines();
    void landPiece();
    void gameOver();
    void updateLabels();
    void updateCell(FXint x, FXint y);
};

#endif	/* TETRISTABITEM_H */

