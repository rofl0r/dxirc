/*
 *      logviewer.h
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

#ifndef LOGVIEWER_H
#define	LOGVIEWER_H

#include "defs.h"

class LogViewer;

class LogItem : public FXTreeItem
{
    FXDECLARE(LogItem)
    friend class LogViewer;
protected:
    FXFileAssoc *assoc; // File association
    LogItem *link; // Link to next item
    LogItem *list; // List of child items
    FXlong size; // File size (if a file)
    FXTime date; // Time of item
private:
    LogItem(const LogItem&);
    LogItem & operator=(const LogItem&);
protected:

    LogItem() : assoc(NULL), link(NULL), list(NULL), size(0L), date(0)
    {
    }
public:

    enum
    {
        FOLDER = 512, /// Directory item
        EXECUTABLE = 1024, /// Executable item
        SYMLINK = 2048, /// Symbolic linked item
        CHARDEV = 4096, /// Character special item
        BLOCKDEV = 8192, /// Block special item
        FIFO = 16384, /// FIFO item
        SOCK = 32768 /// Socket item
    };
public:

    /// Constructor

    LogItem(const FXString& text, FXIcon* oi = NULL, FXIcon* ci = NULL, void* ptr = NULL) : FXTreeItem(text, oi, ci, ptr), assoc(NULL), link(NULL), list(NULL), size(0), date(0)
    {
        state = HASITEMS;
    }

    /// Return true if this is a file item

    FXbool isFile() const
    {
        return (state & (FOLDER | BLOCKDEV | CHARDEV | FIFO | SOCK)) == 0;
    }

    /// Return true if this is a directory item

    FXbool isDirectory() const
    {
        return (state & FOLDER) != 0;
    }

    /// Return true if this is an executable item

    FXbool isExecutable() const
    {
        return (state & EXECUTABLE) != 0;
    }

    /// Return true if this is a symbolic link item

    FXbool isSymlink() const
    {
        return (state & SYMLINK) != 0;
    }

    /// Return true if this is a character device item

    FXbool isChardev() const
    {
        return (state & CHARDEV) != 0;
    }

    /// Return true if this is a block device item

    FXbool isBlockdev() const
    {
        return (state & BLOCKDEV) != 0;
    }

    /// Return true if this is an FIFO item

    FXbool isFifo() const
    {
        return (state & FIFO) != 0;
    }

    /// Return true if this is a socket

    FXbool isSocket() const
    {
        return (state & SOCK) != 0;
    }

    /// Return the file-association object for this item

    FXFileAssoc* getAssoc() const
    {
        return assoc;
    }

    /// Return the file size for this item

    FXlong getSize() const
    {
        return size;
    }

    /// Return the date for this item

    FXTime getDate() const
    {
        return date;
    }
};

class LogViewer: public FXTopWindow
{
    FXDECLARE(LogViewer)
public:
    LogViewer(FXApp *app, const FXString &lpath);
    virtual ~LogViewer();
    enum {
        ID_ADD = FXTopWindow::ID_LAST,
        ID_CLOSE,
        ID_LIST,
        ID_SEARCH,
        ID_SEARCHNEXT,
        ID_LAST
    };

    virtual void create();

    long OnClose(FXObject*,FXSelector,void*);
    long OnKeyPress(FXObject*,FXSelector,void*);
    long OnList(FXObject*,FXSelector,void*);
    long OnSearch(FXObject*,FXSelector,void*);
    long OnSearchNext(FXObject*,FXSelector,void*);

private:
    LogViewer() {}
    LogViewer(const LogViewer&);

    FXbool LoadFile(const FXString& file);
    void LoadTree();
    FXString GetItemPathname(const FXTreeItem* item);
    void ListChildItems(LogItem *par);

    FXVerticalFrame *listframe, *textframe;
    FXHorizontalFrame *content, *buttonframe;
    FXSplitter *splitter;
    FXTreeList *listHistory;
    FXButton *buttonClose;
    FXText *text;
    FXString logPath, searchstring;
};

#endif	/* LOGVIEWER_H */

