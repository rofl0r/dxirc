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
#define LOGVIEWER_H

#include "defs.h"
#include "fxext.h"

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

    /// Return TRUE if this is a file item

    FXbool isFile() const
    {
        return (state & (FOLDER | BLOCKDEV | CHARDEV | FIFO | SOCK)) == 0;
    }

    /// Return TRUE if this is a directory item

    FXbool isDirectory() const
    {
        return (state & FOLDER) != 0;
    }

    /// Return TRUE if this is an executable item

    FXbool isExecutable() const
    {
        return (state & EXECUTABLE) != 0;
    }

    /// Return TRUE if this is a symbolic link item

    FXbool isSymlink() const
    {
        return (state & SYMLINK) != 0;
    }

    /// Return TRUE if this is a character device item

    FXbool isChardev() const
    {
        return (state & CHARDEV) != 0;
    }

    /// Return TRUE if this is a block device item

    FXbool isBlockdev() const
    {
        return (state & BLOCKDEV) != 0;
    }

    /// Return TRUE if this is an FIFO item

    FXbool isFifo() const
    {
        return (state & FIFO) != 0;
    }

    /// Return TRUE if this is a socket

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
    LogViewer(FXApp *app, const FXString &lpath, FXFont *fnt);
    virtual ~LogViewer();
    enum {
        ID_ADD = FXTopWindow::ID_LAST,
        ID_CLOSE,
        ID_TREE,
        ID_RESET,
        ID_ALL,
        ID_CHANNEL,
        ID_FILE,
        ID_SEARCH,
        ID_SEARCHNEXT,
        ID_PACK,
        ID_UNPACK,
        ID_DELETEITEM,
        ID_LAST
    };

    virtual void create();

    long onClose(FXObject*,FXSelector,void*);
    long onKeyPress(FXObject*,FXSelector,void*);
    long onTree(FXObject*,FXSelector,void*);
    long onRightTree(FXObject*,FXSelector,void*);
    long onSearch(FXObject*,FXSelector,void*);
    long onSearchNext(FXObject*,FXSelector,void*);
    long onReset(FXObject*,FXSelector,void*);
    long onCmdSearchOptions(FXObject*,FXSelector,void*);
    long onPack(FXObject*,FXSelector,void*);
    long onUnpack(FXObject*,FXSelector,void*);
    long onDelete(FXObject*,FXSelector,void*);
    void setFont(FXFont*);

private:
    LogViewer() {}
    LogViewer(const LogViewer&);

    FXbool loadFile(const FXString& file);
    FXbool isChannelItem(const LogItem *item);
    void loadTree();
    FXString getItemPathname(const FXTreeItem* item);
    void listChildItems(LogItem *par);
    void enableAllItems();
    FXbool isRightFile(const FXString &path, const FXString &name);
    void scan();

    FXVerticalFrame *m_listframe, *m_textframe, *m_treeframe;
    FXHorizontalFrame *m_content, *m_buttonframe, *m_searchframe;
    FXSplitter *m_splitter;
    FXTreeList *m_treeHistory;
    FXTextField *m_searchfield;
    dxEXButton *m_buttonClose, *m_buttonSearch, *m_buttonReset;
    FXGroupBox *m_group;
    FXRadioButton *m_buttonAll, *m_buttonChannel, *m_buttonFile;
    FXCheckButton *m_buttonIcase;
    FXText *m_text;
    LogItem *m_itemOnRight;
    FXString m_logPath, m_searchstring;
    FXDataTarget m_targetAll, m_targetChannel, m_targetFile, m_targetIcase;
    FXbool m_all, m_channel, m_file, m_icase, m_treeLoaded;
};

#endif  /* LOGVIEWER_H */

