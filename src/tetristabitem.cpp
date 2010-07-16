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
        : m_type(type), m_parent(parent), m_pivot(4,1), m_valid(FALSE)
{
    FXASSERT(type > 0 && type < 8);
    Cell position[4];
    cells(position, type);
    for(int i = 0; i < 4; ++i) position[i].x += 5;
    if(updatePosition(position)) m_valid = TRUE;
}

FXbool Piece::rotate()
{
    Cell rotated[4];
    for(int i = 0; i < 4; ++i)
    {
        FXint x = static_cast<FXint>(m_cells[i].x) - m_pivot.x;
        FXint y = static_cast<FXint>(m_cells[i].y) - m_pivot.y;
        rotated[i].x = y + m_pivot.x;
        rotated[i].y = -x + m_pivot.y;
        if(rotated[i].x > 9 || rotated[i].x < 0 || rotated[i].y > 19 || rotated[i].y < 0) return FALSE;
    }
    return updatePosition(rotated);
}

void Piece::drop()
{
    for(FXint i = 0; i < rows; ++i) moveDown();
}

void Piece::cells(Cell* cells, FXint type)
{
    FXASSERT(cells != 0);
    FXASSERT(type > 0 && type < 8);
    const Cell* values = types[type - 1];
    for(FXint i = 0; i < 4; ++i) cells[i] = values[i];
}

FXbool Piece::move(FXint x, FXint y)
{
    // Move cells
    Cell moved[4];
    for(int i = 0; i < 4; ++i)
    {
        moved[i].x = m_cells[i].x + x;
        moved[i].y = m_cells[i].y + y;
        if(moved[i].x > columns-1 || moved[i].x < 0 || moved[i].y > rows-1 || moved[i].y < 0) return FALSE;
    }

    FXbool success = updatePosition(moved);
    if (success)
    {
        m_pivot.x += x;
        m_pivot.y += y;
    }
    return success;
}

FXbool Piece::updatePosition(const Cell* ucells)
{
    // Check for collision of cells
    const Cell* cell = 0;
    FXbool solid = FALSE;
    for(int i = 0; i < 4; ++i)
    {
        cell = &ucells[i];
        solid = m_parent->cell(cell->x, cell->y);
        if(solid)
        {
            for(int j = 0; j < 4; ++j)
            {
                if(*cell == m_cells[j]) solid = FALSE;
            }
        }
        if(solid) return FALSE;
    }

    // Move cells
    if(m_cells[0].x != -1)
    {
        for(int i = 0; i < 4; ++i)
        {
            m_parent->removeCell(m_cells[i].x, m_cells[i].y);
        }
    }
    for(int i = 0; i < 4; ++i)
    {
        m_parent->addCell(ucells[i].x, ucells[i].y, m_type);
    }
    for(int i = 0; i < 4; ++i)
    {
        m_cells[i] = ucells[i];
    }

    return TRUE;
}

FXDEFMAP(TetrisTabItem) TetrisTabItemMap[] = {
    FXMAPFUNC(SEL_PAINT,      TetrisTabItem::ID_GAMECANVAS,         TetrisTabItem::onPaint),
    FXMAPFUNC(SEL_PAINT,      TetrisTabItem::ID_NEXTCANVAS,         TetrisTabItem::onPaint),
    FXMAPFUNC(SEL_COMMAND,    TetrisTabItem::ID_NEW,                TetrisTabItem::onNewGame),
    FXMAPFUNC(SEL_COMMAND,    TetrisTabItem::ID_PAUSE,              TetrisTabItem::onPauseGame),
    FXMAPFUNC(SEL_TIMEOUT,    TetrisTabItem::ID_TETRISTIMEOUT,      TetrisTabItem::onTimeout)
};

FXIMPLEMENT(TetrisTabItem, FXTabItem, TetrisTabItemMap, ARRAYNUMBER(TetrisTabItemMap))

TetrisTabItem::TetrisTabItem(dxTabBook *tab, const FXString &tabtext, FXIcon *ic=0, FXuint opts=TAB_TOP_NORMAL, FXint id=0)
:       FXTabItem(tab, tabtext, ic, opts),
        m_parent(tab),
        m_removedLines(0),
        m_level(1),
        m_score(0),
        m_nextPiece(0),
        m_id(id),
        m_paused(FALSE),
        m_done(FALSE),
        m_pauseEnable(FALSE),
        m_piece(0),
        m_penColor(fxcolorfromname("black"))
{
    m_mainframe = new FXVerticalFrame(m_parent, FRAME_RAISED|LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y);

    m_splitter = new FXSplitter(m_mainframe, LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y|SPLITTER_REVERSED|SPLITTER_TRACKING);

    m_gameframe = new FXVerticalFrame(m_splitter, LAYOUT_FILL_X|LAYOUT_FILL_Y, 0, 0, 0, 0, 1, 1, 1, 1);
    m_gamecanvas = new FXCanvas(m_gameframe, this, ID_GAMECANVAS, FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    m_gamecanvas->setFocus();

    m_otherframe = new FXVerticalFrame(m_splitter, LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_FIX_WIDTH, 0, 0, 0, 0, 1, 1, 1, 1);
    new FXLabel(m_otherframe, _("Next piece:"));
    m_nextcanvas = new FXCanvas(m_otherframe, this, ID_NEXTCANVAS, LAYOUT_LEFT|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT);
    m_nextcanvas->setWidth(40);
    m_nextcanvas->setHeight(80);
    m_levelLabel = new FXLabel(m_otherframe, FXStringFormat(_("Level: %d"), m_level));
    m_scoreLabel = new FXLabel(m_otherframe, FXStringFormat(_("Score: %d"), m_score));
    m_linesLabel = new FXLabel(m_otherframe, FXStringFormat(_("Lines: %d"), m_removedLines));
    m_newButton = new FXButton(m_otherframe, _("&New game"), NULL, this, ID_NEW);
    m_pauseButton = new FXButton(m_otherframe, _("&Pause game"), NULL, this, ID_PAUSE);
    m_pauseButton->disable();

    m_messageFont = new FXFont(getApp(), "helvetica", 25, FXFont::Bold, FXFont::Straight, FONTENCODING_DEFAULT, FXFont::NonExpanded, FXFont::Scalable|FXFont::Rotatable);

    for(FXint x=0; x<columns; ++x)
    {
        for(FXint y=0; y<rows; ++y)
        {
            m_cells[x][y] = 0;
        }
    }    
}

TetrisTabItem::~TetrisTabItem()
{
    delete m_messageFont;
}

FXbool TetrisTabItem::cell(FXint x, FXint y) const
{
    FXASSERT(x >= 0 && x < columns);
    FXASSERT(y >= 0 && y < rows);
    return m_cells[x][y] != 0;
}

void TetrisTabItem::addCell(FXint x, FXint y, FXint type)
{
    FXASSERT(x >= 0 && x < columns);
    FXASSERT(y >= 0 && y < rows);
    FXASSERT(type > 0 && type < 8);
    FXASSERT(m_cells[x][y] == 0);
    m_cells[x][y] = type;
    updateCell(x,y);
}

void TetrisTabItem::removeCell(FXint x, FXint y, FXbool update)
{
    FXASSERT(x >= 0 && x < columns);
    FXASSERT(y >= 0 && y < rows);
    m_cells[x][y] = 0;
    if(update) updateCell(x,y);
}

void TetrisTabItem::updateCell(FXint x, FXint y)
{
    FXASSERT(x >= 0 && x < columns);
    FXASSERT(y >= 0 && y < rows);
    FXDCWindow dc(m_gamecanvas);
    if(m_cells[x][y] != 0)
        dc.setForeground(colors[m_cells[x][y]-1]);
    else
        dc.setForeground(m_gamecanvas->getBackColor());
    dc.fillRectangle(x*m_apiece+1, y*m_apiece+1, m_apiece-2, m_apiece-2);
}

void TetrisTabItem::findFullLines()
{
    // Empty list of full lines
    for(FXint i = 0; i < 4; ++i)
        m_fullLines[i] = -1;
    FXint pos = 0;
    // Find full lines
    FXbool full = FALSE;
    for(FXint row = 0; row < rows; ++row)
    {
        full = TRUE;
        for(FXint col = 0; col < columns; ++col)
        {
            if(m_cells[col][row] == 0) full = FALSE;
        }
        if(full)
        {
            m_fullLines[pos] = row;
            ++pos;
        }
    }
}

void TetrisTabItem::createGeom()
{
    m_mainframe->create();
    m_splitter->create();
    m_gameframe->create();
    m_gamecanvas->create();
    m_otherframe->create();
    m_nextcanvas->create();
    m_levelLabel->create();
    m_scoreLabel->create();
    m_linesLabel->create();
    m_messageFont->create();
}

void TetrisTabItem::setColor(IrcColor color)
{
    m_gamecanvas->setBackColor(color.back);
    m_nextcanvas->setBackColor(color.back);
    m_penColor = color.text;
}

void TetrisTabItem::setGameFocus()
{
    m_gamecanvas->setFocus();
}

void TetrisTabItem::newGame()
{
    if(getApp()->hasTimeout(this, ID_TETRISTIMEOUT)) return;
    getApp()->removeTimeout(this, ID_TETRISTIMEOUT);
    delete m_piece;
    m_piece = 0;
    m_paused = FALSE;
    m_done = FALSE;
    m_newButton->disable();
    m_pauseEnable = TRUE;
    m_pauseButton->enable();
    m_pauseButton->setText(_("&Pause game"));
    m_removedLines = 0;
    m_level = 1;
    m_score = 0;
    m_nextPiece = rand()%7+1;
    for(FXint i=0; i<4; ++i)
    m_fullLines[i] = -1;
    for(int col = 0; col < columns; ++col)
    {
        for(int row = 0; row < rows; ++row)
        {
            m_cells[col][row] = 0;
        }
    }
    createPiece();
    redraw();
    updateLabels();
}

void TetrisTabItem::stopGame()
{
    getApp()->removeTimeout(this, ID_TETRISTIMEOUT);
    delete m_piece;
    m_piece = 0;
    m_paused = FALSE;
    m_done = FALSE;
    m_newButton->enable();
    m_pauseEnable = FALSE;
    m_pauseButton->disable();
    m_removedLines = 0;
    m_level = 1;
    m_score = 0;
    m_nextPiece = rand()%7+1;
    for(FXint i=0; i<4; ++i)
    m_fullLines[i] = -1;
    for(int col = 0; col < columns; ++col)
    {
        for(int row = 0; row < rows; ++row)
        {
            m_cells[col][row] = 0;
        }
    }
}

void TetrisTabItem::reparentTab()
{
    reparent(m_parent);
    m_mainframe->reparent(m_parent);
}

void TetrisTabItem::moveLeft()
{
    if(m_done || m_paused) return;
    if(!m_piece) return;
    m_piece->moveLeft();
}

void TetrisTabItem::moveRight()
{
    if(m_done || m_paused) return;
    if(!m_piece) return;
    m_piece->moveRight();
}

void TetrisTabItem::rotate()
{
    if(m_done || m_paused) return;
    if(!m_piece) return;
    m_piece->rotate();
}

void TetrisTabItem::drop()
{
    if(m_done || m_paused) return;
    if(!m_piece) return;
    m_piece->drop();
    landPiece();
}

void TetrisTabItem::createPiece()
{
    FXASSERT(m_piece == 0);
    m_piece = new Piece(m_nextPiece, this);
    if(!m_piece->isValid())
    {
        delete m_piece;
        m_piece = 0;
        m_done = TRUE;
        gameOver();
        return;
    }
    m_nextPiece = rand()%7+1;
    redraw();
    getApp()->addTimeout(this, ID_TETRISTIMEOUT, timeout-(m_level-1)*13);
}

void TetrisTabItem::drawLines()
{
    FXDCWindow dc(m_gamecanvas);
    dc.setForeground(m_gamecanvas->getBackColor());
    dc.fillRectangle(0, 0, m_gamecanvas->getWidth(), m_gamecanvas->getHeight());
    dc.setLineWidth(2);
    dc.setForeground(m_penColor);
    dc.drawLine(0,0,0,rows*m_apiece);
    dc.drawLine(0,rows*m_apiece,columns*m_apiece,rows*m_apiece);
    dc.drawLine(columns*m_apiece,rows*m_apiece,columns*m_apiece,0);
    for(FXint x=0; x<columns; ++x)
    {
        for(FXint y=0; y<rows; ++y)
        {
            if(m_cells[x][y] != 0)
            {
                dc.setForeground(colors[m_cells[x][y]-1]);
                dc.fillRectangle(x*m_apiece+1, y*m_apiece+1, m_apiece-2, m_apiece-2);
            }
        }
    }
    dc.setForeground(m_penColor);
    if(!m_done && !m_piece)
    {
        dc.setFont(getApp()->getNormalFont());
        dc.drawText(2, getApp()->getNormalFont()->getFontHeight()+2, _("Keys for playing:"));
        dc.drawText(2, 2*(getApp()->getNormalFont()->getFontHeight()+2), _("n .. new game"));
        dc.drawText(2, 3*(getApp()->getNormalFont()->getFontHeight()+2), _("p .. pause game"));
        dc.drawText(2, 4*(getApp()->getNormalFont()->getFontHeight()+2), _("i .. rotate piece"));
        dc.drawText(2, 5*(getApp()->getNormalFont()->getFontHeight()+2), _("l .. move piece right"));
        dc.drawText(2, 6*(getApp()->getNormalFont()->getFontHeight()+2), _("k .. drop piece"));
        dc.drawText(2, 7*(getApp()->getNormalFont()->getFontHeight()+2), _("j .. move piece left"));
    }
    dc.setFont(m_messageFont);
    if(m_done)
    {
        FXint width = m_messageFont->getTextWidth(_("GAME OVER"));
        FXint x = 2;
        if(width < m_apiece*columns-4) x = (m_apiece*columns-width)/2;
        dc.drawText(x, m_gamecanvas->getHeight()/2, _("GAME OVER"));
    }
    if(m_paused)
    {
        FXint width = m_messageFont->getTextWidth(_("PAUSED"));
        FXint x = 2;
        if(width < m_apiece*columns-4) x = (m_apiece*columns-width)/2;
        dc.drawText(x, m_gamecanvas->getHeight()/2, _("PAUSED"));
    }
}

void TetrisTabItem::drawNextPiece(FXint type)
{
    if(type<1 || type>7) return;
    FXDCWindow dc(m_nextcanvas);
    dc.setForeground(m_nextcanvas->getBackColor());
    dc.fillRectangle(0, 0, m_nextcanvas->getWidth(), m_nextcanvas->getHeight());
    for(FXint i=0; i<4; ++i)
    {
        dc.setForeground(colors[type-1]);
        dc.fillRectangle(types[type - 1][i].x*m_anext+1, types[type - 1][i].y*m_anext+1, m_anext-2, m_anext-2);
    }
}

void TetrisTabItem::removeLines()
{
    getApp()->removeTimeout(this, ID_TETRISTIMEOUT);
    FXint deltascore = basicscore*m_level;
    // Loop through full lines
    for(FXint i = 0; i < 4; ++i)
    {
        FXint row = m_fullLines[i];
        if(row == -1)
            break;
        // Remove line
        for(FXint col = 0; col < 10; ++col)
        {
            removeCell(col, row, FALSE); //not need update canvas, CreatePiece do it
        }
        ++m_removedLines;
        deltascore *= 3;
        // Shift board down
        for(; row > 0; --row)
        {
            for(FXint col = 0; col < columns; ++col)
            {
                m_cells[col][row] = m_cells[col][row - 1];
            }
        }
    }
    // Remove top line
    if(m_fullLines[0] != -1)
    {
        for(FXint col = 0; col < 10; ++col)
        {
            removeCell(col, 0, FALSE); //not need update canvas, CreatePiece do it
        }
    }
    m_level = (m_removedLines / 10) + 1;
    m_score += deltascore;
    // Empty list of full lines
    for(FXint i = 0; i < 4; ++i)
        m_fullLines[i] = -1;
    updateLabels();
    // Add new piece
    createPiece();
}

void TetrisTabItem::landPiece()
{
    getApp()->removeTimeout(this, ID_TETRISTIMEOUT);
    delete m_piece;
    m_piece = 0;
    findFullLines();
    if(m_fullLines[0] != -1) removeLines();
    else createPiece();
}

void TetrisTabItem::pauseResumeGame()
{
    if(m_done) return;
    if(!m_pauseEnable) return;
    if(m_paused)
    {
        getApp()->addTimeout(this, ID_TETRISTIMEOUT, timeout-(m_level-1)*13);
        m_pauseButton->setText(_("&Pause game"));
    }
    else
    {
        getApp()->removeTimeout(this, ID_TETRISTIMEOUT);
        m_newButton->enable();
        m_pauseButton->setText(_("&Resume game"));
    }
    m_paused = !m_paused;
    redraw();
}

void TetrisTabItem::gameOver()
{
    getApp()->removeTimeout(this, ID_TETRISTIMEOUT);
    m_done = TRUE;
    m_newButton->enable();
    m_pauseEnable = FALSE;
    m_pauseButton->disable();
    redraw();
}

void TetrisTabItem::updateLabels()
{
    m_levelLabel->setText(FXStringFormat(_("Level: %d"), m_level));
    m_scoreLabel->setText(FXStringFormat(_("Score: %d"), m_score));
    m_linesLabel->setText(FXStringFormat(_("Lines: %d"), m_removedLines));
}

void TetrisTabItem::redraw()
{
    if(m_gamecanvas)
    {
        m_apiece = FXMIN(m_gamecanvas->getWidth()/columns, m_gamecanvas->getHeight()/rows);
        drawLines();
    }
    if(m_nextcanvas)
    {
        m_anext = FXMIN(m_nextcanvas->getWidth()/2, m_nextcanvas->getHeight()/4);
        drawNextPiece(m_nextPiece);
    }
}

long TetrisTabItem::onPaint(FXObject *sender, FXSelector, void *ptr)
{
    FXEvent *ev=(FXEvent*)ptr;
    FXDCWindow dc((FXCanvas*)sender,ev);
    dc.setForeground(((FXCanvas*)sender)->getBackColor());
    dc.fillRectangle(ev->rect.x,ev->rect.y,ev->rect.w,ev->rect.h);
    redraw();
    return 1;
}

long TetrisTabItem::onTimeout(FXObject*, FXSelector, void*)
{    
    FXASSERT(m_piece != 0);
    if(m_piece->moveDown())
    {
        getApp()->addTimeout(this, ID_TETRISTIMEOUT, timeout-(m_level-1)*13);
    }
    else landPiece();
    return 1;
}

long TetrisTabItem::onNewGame(FXObject*, FXSelector, void*)
{
    newGame();
    return 1;
}

long TetrisTabItem::onPauseGame(FXObject*, FXSelector, void*)
{
    pauseResumeGame();
    return 1;
}

