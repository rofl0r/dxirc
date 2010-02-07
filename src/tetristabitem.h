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

    FXbool IsValid() const { return valid; }
    FXbool MoveLeft() { return Move(-1, 0); }
    FXbool MoveRight() { return Move(1, 0); }
    FXbool MoveDown() { return Move(0, 1); }
    FXbool Rotate();
    void Drop();
    static void Cells(Cell* cells, FXint type);

private:
    FXint type;
    TetrisTabItem *parent;
    Cell cells[4];
    Cell pivot;
    FXbool valid;

    FXbool Move(FXint x, FXint y);
    FXbool UpdatePosition(const Cell* ucells);
};

class TetrisTabItem : public FXTabItem
{
    FXDECLARE(TetrisTabItem)
public:
    TetrisTabItem(dxTabBook*, const FXString&, FXIcon*, FXuint);
    virtual ~TetrisTabItem();
    enum {
        ID_GAMECANVAS = FXMainWindow::ID_LAST+400,
        ID_NEXTCANVAS,
        ID_TETRISTIMEOUT,
        ID_NEW,
        ID_PAUSE,
        ID_LAST
    };

    FXbool Cell(FXint x, FXint y) const;
    void AddCell(FXint x, FXint y, FXint type);
    void RemoveCell(FXint x, FXint y);
    void FindFullLines();
    void CreateGeom();
    void SetColor(IrcColor);
    void SetGameFocus();
    void NewGame();
    void StopGame();
    void PauseResumeGame();
    void ReparentTab();
    void Redraw();
    void MoveLeft();
    void MoveRight();
    void Rotate();
    void Drop();
    FXbool IsPauseEnable() { return pauseEnable; }
    FXbool IsPaused() { return paused; }

    long OnPaint(FXObject*, FXSelector, void*);
    long OnTimeout(FXObject*, FXSelector, void*);
    long OnNewGame(FXObject*, FXSelector, void*);
    long OnPauseGame(FXObject*, FXSelector, void*);

private:
    TetrisTabItem() {}

    dxTabBook *parent;
    FXVerticalFrame *mainframe, *gameframe, *otherframe;
    FXSplitter *splitter;
    FXCanvas *gamecanvas, *nextcanvas;
    FXLabel *levelLabel, *scoreLabel, *linesLabel;
    FXButton *newButton, *pauseButton;
    FXFont *messageFont;
    FXint cells[columns][rows];
    FXint fullLines[4];
    FXint apiece, anext;
    FXint removedLines, level, score, nextPiece;
    FXbool paused, done, pauseEnable;
    Piece *piece;
    FXColor penColor;

    void CreatePiece();
    void DrawLines();
    void DrawNextPiece(FXint type);
    void RemoveLines();
    void LandPiece();
    void GameOver();
    void UpdateLabels();
};

#endif	/* TETRISTABITEM_H */

