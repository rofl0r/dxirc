/*
 *      tetristabitem.cpp
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

#include "tetristabitem.h"
#include "config.h"
#include "i18n.h"

static const Cell types[][4] = {
    { Cell(0,0), Cell(0,1), Cell(0,2), Cell(0,3) },
    { Cell(0,0), Cell(0,1), Cell(1,1), Cell(1,2) },
    { Cell(1,0), Cell(0,1), Cell(1,1), Cell(0,2) },
    { Cell(0,0), Cell(0,1), Cell(1,1), Cell(0,2) },
    { Cell(0,0), Cell(1,0), Cell(1,1), Cell(1,2) },
    { Cell(0,0), Cell(1,0), Cell(0,1), Cell(0,2) },
    { Cell(0,0), Cell(1,0), Cell(0,1), Cell(1,1) }
};

static FXColor colors[] = {
    FXRGB(196, 160, 0),
    FXRGB(206, 92, 0),
    FXRGB(143, 89, 2),
    FXRGB(78, 154, 6),
    FXRGB(32, 74, 135),
    FXRGB(117, 80, 123),
    FXRGB(164, 0, 0)
};

static const FXint timeout = 333;
static const FXint basicscore = 13;

Piece::Piece(FXint type, TetrisTabItem* parent)
        : type(type), parent(parent), pivot(4,1), valid(FALSE)
{
    FXASSERT(type > 0 && type < 8);
    Cell position[4];
    Cells(position, type);
    for(int i = 0; i < 4; ++i) position[i].x += 5;
    if(UpdatePosition(position)) valid = TRUE;
}

FXbool Piece::Rotate()
{
    Cell rotated[4];
    for (int i = 0; i < 4; ++i)
    {
        FXint x = static_cast<FXint>(cells[i].x) - pivot.x;
        FXint y = static_cast<FXint>(cells[i].y) - pivot.y;
        rotated[i].x = y + pivot.x;
        rotated[i].y = -x + pivot.y;
        if(rotated[i].x > 9 || rotated[i].x < 0 || rotated[i].y > 19 || rotated[i].y < 0) return FALSE;
    }
    return UpdatePosition(rotated);
}

void Piece::Drop()
{
    for(FXint i = 0; i < rows; ++i) MoveDown();
}

void Piece::Cells(Cell* cells, FXint type)
{
    FXASSERT(cells != 0);
    FXASSERT(type > 0 && type < 8);
    const Cell* values = types[type - 1];
    for(FXint i = 0; i < 4; ++i) cells[i] = values[i];
}

FXbool Piece::Move(FXint x, FXint y)
{
    // Move cells
    Cell moved[4];
    for(int i = 0; i < 4; ++i)
    {
        moved[i].x = cells[i].x + x;
        moved[i].y = cells[i].y + y;
        if(moved[i].x > columns-1 || moved[i].x < 0 || moved[i].y > rows-1 || moved[i].y < 0) return FALSE;
    }

    FXbool success = UpdatePosition(moved);
    if (success)
    {
        pivot.x += x;
        pivot.y += y;
    }
    return success;
}

FXbool Piece::UpdatePosition(const Cell* ucells)
{
    // Check for collision of cells
    const Cell* cell = 0;
    FXbool solid = FALSE;
    for(int i = 0; i < 4; ++i)
    {
        cell = &ucells[i];
        solid = parent->Cell(cell->x, cell->y);
        if(solid)
        {
            for(int j = 0; j < 4; ++j)
            {
                if(*cell == cells[j]) solid = FALSE;
            }
        }
        if(solid) return FALSE;
    }

    // Move cells
    if(cells[0].x != -1)
    {
        for(int i = 0; i < 4; ++i)
        {
            parent->RemoveCell(cells[i].x, cells[i].y);
        }
    }
    for(int i = 0; i < 4; ++i)
    {
        parent->AddCell(ucells[i].x, ucells[i].y, type);
    }
    for(int i = 0; i < 4; ++i)
    {
        cells[i] = ucells[i];
    }

    return TRUE;
}

FXDEFMAP(TetrisTabItem) TetrisTabItemMap[] = {
    FXMAPFUNC(SEL_PAINT,      TetrisTabItem::ID_GAMECANVAS,         TetrisTabItem::OnPaint),
    FXMAPFUNC(SEL_PAINT,      TetrisTabItem::ID_NEXTCANVAS,         TetrisTabItem::OnPaint),
    FXMAPFUNC(SEL_TIMEOUT,    TetrisTabItem::ID_TETRISTIMEOUT,      TetrisTabItem::OnTimeout)
};

FXIMPLEMENT(TetrisTabItem, FXTabItem, TetrisTabItemMap, ARRAYNUMBER(TetrisTabItemMap))

TetrisTabItem::TetrisTabItem(FXTabBook *tab, const FXString &tabtext, FXIcon *ic=0, FXuint opts=TAB_TOP_NORMAL)
:       FXTabItem(tab, tabtext, ic, opts),
        parent(tab),
        removedLines(0),
        level(1),
        score(0),
        nextPiece(0),
        paused(FALSE),
        done(FALSE),
        pauseEnable(FALSE),
        piece(0),
        penColor(fxcolorfromname("black"))
{
    mainframe = new FXVerticalFrame(parent, FRAME_RAISED|LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y);

    splitter = new FXSplitter(mainframe, LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y|SPLITTER_REVERSED|SPLITTER_TRACKING);

    gameframe = new FXVerticalFrame(splitter, LAYOUT_FILL_X|LAYOUT_FILL_Y, 0, 0, 0, 0, 1, 1, 1, 1);
    gamecanvas = new FXCanvas(gameframe, this, ID_GAMECANVAS, FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    gamecanvas->setFocus();

    otherframe = new FXVerticalFrame(splitter, LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_FIX_WIDTH, 0, 0, 0, 0, 1, 1, 1, 1);
    new FXLabel(otherframe, _("Next piece:"));
    nextcanvas = new FXCanvas(otherframe, this, ID_NEXTCANVAS, LAYOUT_LEFT|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT);
    nextcanvas->setWidth(40);
    nextcanvas->setHeight(80);
    levelLabel = new FXLabel(otherframe, FXStringFormat(_("Level: %d"), level));
    scoreLabel = new FXLabel(otherframe, FXStringFormat(_("Score: %d"), score));
    linesLabel = new FXLabel(otherframe, FXStringFormat(_("Lines: %d"), removedLines));

    messageFont = new FXFont(getApp(), "helvetica", 25, FXFont::Bold, FXFont::Straight, FONTENCODING_DEFAULT, FXFont::NonExpanded, FXFont::Scalable|FXFont::Rotatable);

    for(FXint x=0; x<columns; ++x)
    {
        for(FXint y=0; y<rows; ++y)
        {
            cells[x][y] = 0;
        }
    }    
}

TetrisTabItem::~TetrisTabItem()
{
    delete messageFont;
}

FXbool TetrisTabItem::Cell(FXint x, FXint y) const
{
    FXASSERT(x >= 0 && x < columns);
    FXASSERT(y >= 0 && y < rows);
    return cells[x][y] != 0;
}

void TetrisTabItem::AddCell(FXint x, FXint y, FXint type)
{
    FXASSERT(x >= 0 && x < columns);
    FXASSERT(y >= 0 && y < rows);
    FXASSERT(type > 0 && type < 8);
    FXASSERT(cells[x][y] == 0);
    cells[x][y] = type;
}

void TetrisTabItem::RemoveCell(FXint x, FXint y)
{
    FXASSERT(x >= 0 && x < columns);
    FXASSERT(y >= 0 && y < rows);
    cells[x][y] = 0;
}

void TetrisTabItem::FindFullLines()
{
    // Empty list of full lines
    for(FXint i = 0; i < 4; ++i)
        fullLines[i] = -1;
    FXint pos = 0;
    // Find full lines
    FXbool full = FALSE;
    for(FXint row = 0; row < rows; ++row)
    {
        full = TRUE;
        for(FXint col = 0; col < columns; ++col)
        {
            if(cells[col][row] == 0) full = FALSE;
        }
        if(full)
        {
            fullLines[pos] = row;
            ++pos;
        }
    }
}

void TetrisTabItem::CreateGeom()
{
    mainframe->create();
    splitter->create();
    gameframe->create();
    gamecanvas->create();
    otherframe->create();
    nextcanvas->create();
    levelLabel->create();
    scoreLabel->create();
    linesLabel->create();
    messageFont->create();
}

void TetrisTabItem::SetColor(IrcColor color)
{
    gamecanvas->setBackColor(color.back);
    nextcanvas->setBackColor(color.back);
    penColor = color.text;
}

void TetrisTabItem::SetGameFocus()
{
    gamecanvas->setFocus();
}

void TetrisTabItem::NewGame()
{
    if(getApp()->hasTimeout(this, ID_TETRISTIMEOUT)) return;
    getApp()->removeTimeout(this, ID_TETRISTIMEOUT);
    delete piece;
    piece = 0;
    paused = FALSE;
    done = FALSE;
    pauseEnable = TRUE;
    removedLines = 0;
    level = 1;
    score = 0;
    nextPiece = rand()%7+1;
    for(FXint i=0; i<4; ++i)
	fullLines[i] = -1;
    for (int col = 0; col < columns; ++col)
    {
        for (int row = 0; row < rows; ++row)
        {
            cells[col][row] = 0;
        }
    }
    CreatePiece();
}

void TetrisTabItem::StopGame()
{
    getApp()->removeTimeout(this, ID_TETRISTIMEOUT);
    delete piece;
    piece = 0;
    paused = FALSE;
    done = FALSE;
    pauseEnable = FALSE;
    removedLines = 0;
    level = 1;
    score = 0;
    nextPiece = rand()%7+1;
    for(FXint i=0; i<4; ++i)
	fullLines[i] = -1;
    for (int col = 0; col < columns; ++col)
    {
        for (int row = 0; row < rows; ++row)
        {
            cells[col][row] = 0;
        }
    }
}

void TetrisTabItem::ReparentTab()
{
    reparent(parent);
    mainframe->reparent(parent);
}

void TetrisTabItem::MoveLeft()
{
    if(done || paused) return;
    if(!piece) return;
    piece->MoveLeft();
    Redraw();
}

void TetrisTabItem::MoveRight()
{
    if(done || paused) return;
    if(!piece) return;
    piece->MoveRight();
    Redraw();
}

void TetrisTabItem::Rotate()
{
    if(done || paused) return;
    if(!piece) return;
    piece->Rotate();
    Redraw();
}

void TetrisTabItem::Drop()
{
    if(done || paused) return;
    if(!piece) return;
    piece->Drop();
    LandPiece();
    Redraw();
}

void TetrisTabItem::CreatePiece()
{
    FXASSERT(piece == 0);
    piece = new Piece(nextPiece, this);
    if(!piece->IsValid())
    {
        delete piece;
        piece = 0;
        done = TRUE;
        GameOver();
        return;
    }
    nextPiece = rand()%7+1;
    Redraw();
    getApp()->addTimeout(this, ID_TETRISTIMEOUT, timeout-(level-1)*13);
}

void TetrisTabItem::DrawLines()
{
    FXDCWindow dc(gamecanvas);
    dc.setForeground(gamecanvas->getBackColor());
    dc.fillRectangle(0, 0, gamecanvas->getWidth(), gamecanvas->getHeight());
    dc.setLineWidth(2);
    dc.setForeground(penColor);
//    for(FXint i=0; i<(rows+1)*apiece; i+=apiece)
//    {
//        dc.drawLine(0,i,columns*apiece,i);
//    }
//    for(FXint i=0; i<(columns+1)*apiece; i+=apiece)
//    {
//        dc.drawLine(i,0,i,rows*apiece);
//    }
    dc.drawLine(0,0,0,rows*apiece);
    dc.drawLine(0,rows*apiece,columns*apiece,rows*apiece);
    dc.drawLine(columns*apiece,rows*apiece,columns*apiece,0);
    for(FXint x=0; x<columns; ++x)
    {
        for(FXint y=0; y<rows; ++y)
        {
            if(cells[x][y] != 0)
            {
                dc.setForeground(colors[cells[x][y]-1]);
                dc.fillRectangle(x*apiece+1, y*apiece+1, apiece-2, apiece-2);
            }
        }
    }
    dc.setFont(messageFont);
    dc.setForeground(penColor);
    if(done)
    {
        FXint width = messageFont->getTextWidth(_("GAME OVER"));
        FXint x = 2;
        if(width < apiece*columns-4) x = (apiece*columns-width)/2;
        dc.drawText(x, gamecanvas->getHeight()/2, _("GAME OVER"));
    }
    if(paused)
    {
        FXint width = messageFont->getTextWidth(_("PAUSED"));
        FXint x = 2;
        if(width < apiece*columns-4) x = (apiece*columns-width)/2;
        dc.drawText(x, gamecanvas->getHeight()/2, _("PAUSED"));
    }
}

void TetrisTabItem::DrawNextPiece(FXint type)
{
    if(type<1 || type>7) return;
    FXDCWindow dc(nextcanvas);
    dc.setForeground(nextcanvas->getBackColor());
    dc.fillRectangle(0, 0, nextcanvas->getWidth(), nextcanvas->getHeight());
    for(FXint i=0; i<4; ++i)
    {
        dc.setForeground(colors[type-1]);
        dc.fillRectangle(types[type - 1][i].x*anext+1, types[type - 1][i].y*anext+1, anext-2, anext-2);
    }
}

void TetrisTabItem::RemoveLines()
{
    getApp()->removeTimeout(this, ID_TETRISTIMEOUT);
    FXint deltascore = basicscore*level;
    // Loop through full lines
    for(FXint i = 0; i < 4; ++i)
    {
        FXint row = fullLines[i];
        if(row == -1)
            break;
        // Remove line
        for(FXint col = 0; col < 10; ++col)
        {
            RemoveCell(col, row);
        }
        ++removedLines;
        deltascore *= 3;
        // Shift board down
        for(; row > 0; --row)
        {
            for (FXint col = 0; col < columns; ++col)
            {
                cells[col][row] = cells[col][row - 1];
            }
        }
    }
    // Remove top line
    if(fullLines[0] != -1)
    {
        for(FXint col = 0; col < 10; ++col)
        {
            RemoveCell(col, 0);
        }
    }
    level = (removedLines / 10) + 1;
    score += deltascore;
    // Empty list of full lines
    for(FXint i = 0; i < 4; ++i)
        fullLines[i] = -1;
    UpdateLabels();
    // Add new piece
    CreatePiece();
}

void TetrisTabItem::LandPiece()
{
    getApp()->removeTimeout(this, ID_TETRISTIMEOUT);
    delete piece;
    piece = 0;
    FindFullLines();
    if(fullLines[0] != -1) RemoveLines();
    else CreatePiece();
}

void TetrisTabItem::PauseResumeGame()
{
    if(done) return;
    if(!pauseEnable) return;
    if(paused) getApp()->addTimeout(this, ID_TETRISTIMEOUT, timeout-(level-1)*13);
    else getApp()->removeTimeout(this, ID_TETRISTIMEOUT);
    paused = !paused;
    Redraw();
}

void TetrisTabItem::GameOver()
{
    getApp()->removeTimeout(this, ID_TETRISTIMEOUT);
    done = TRUE;
    pauseEnable = FALSE;
    Redraw();
}

void TetrisTabItem::UpdateLabels()
{
    levelLabel->setText(FXStringFormat(_("Level: %d"), level));
    scoreLabel->setText(FXStringFormat(_("Score: %d"), score));
    linesLabel->setText(FXStringFormat(_("Lines: %d"), removedLines));
}

void TetrisTabItem::Redraw()
{
    if(gamecanvas)
    {
        apiece = FXMIN(gamecanvas->getWidth()/columns, gamecanvas->getHeight()/rows);
        DrawLines();
    }
    if(nextcanvas)
    {
        anext = FXMIN(nextcanvas->getWidth()/2, nextcanvas->getHeight()/4);
        DrawNextPiece(nextPiece);
    }
}

long TetrisTabItem::OnPaint(FXObject *sender, FXSelector, void *ptr)
{
    FXEvent *ev=(FXEvent*)ptr;
    FXDCWindow dc((FXCanvas*)sender,ev);
    dc.setForeground(((FXCanvas*)sender)->getBackColor());
    dc.fillRectangle(ev->rect.x,ev->rect.y,ev->rect.w,ev->rect.h);
    Redraw();
    return 1;
}

long TetrisTabItem::OnTimeout(FXObject*, FXSelector, void*)
{    
    FXASSERT(piece != 0);
    if(piece->MoveDown())
    {
        DrawLines();
        getApp()->addTimeout(this, ID_TETRISTIMEOUT, timeout-(level-1)*13);
    }
    else LandPiece();
    return 1;
}

