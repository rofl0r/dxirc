/*
 *      aliasdialog.cpp
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

#include "logviewer.h"
#include "icons.h"
#include "config.h"
#include "i18n.h"

FXIMPLEMENT(LogItem, FXTreeItem, NULL, 0)

FXDEFMAP(SearchDialog) SearchDialogMap[] = {
    FXMAPFUNC(SEL_COMMAND,  SearchDialog::ID_CLOSE,     SearchDialog::OnClose),
    FXMAPFUNC(SEL_COMMAND,  SearchDialog::ID_SEARCH,    SearchDialog::OnClose),
    FXMAPFUNC(SEL_CLOSE,    0,                          SearchDialog::OnClose)
};

FXIMPLEMENT(SearchDialog, FXDialogBox, SearchDialogMap, ARRAYNUMBER(SearchDialogMap))

SearchDialog::SearchDialog(FXWindow* owner)
        : FXDialogBox(owner, _("Search"), DECOR_TITLE|DECOR_BORDER, 0,0,0,0, 0,0,0,0, 0,0)
{
    contents = new FXVerticalFrame(this, LAYOUT_SIDE_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 10,10,10,10, 0,0);
    new FXLabel(contents, _("Search for:"), NULL, JUSTIFY_LEFT);
    searchtext = new FXTextField(contents, 25, this, ID_SEARCH, TEXTFIELD_ENTER_ONLY|FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    FXHorizontalFrame *buttonframe = new FXHorizontalFrame(contents,LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXButton(buttonframe, _("&Search"), NULL, this, FXDialogBox::ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 32,32,5,5);
    new FXButton(buttonframe, _("Cancel"), NULL, this, FXDialogBox::ID_CANCEL, BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 32,32,5,5);
}

SearchDialog::~SearchDialog()
{

}

long SearchDialog::OnClose(FXObject*, FXSelector, void*)
{
    getApp()->stopModal(this, TRUE);
    hide();
    return 1;
}

FXuint SearchDialog::execute(FXuint placement)
{
    create();
    searchtext->setFocus();
    show(placement);
    return getApp()->runModalFor(this);
}

FXDEFMAP(LogViewer) LogViewerMap[] = {
    FXMAPFUNC(SEL_COMMAND,  LogViewer::ID_CLOSE,      LogViewer::OnClose),
    FXMAPFUNC(SEL_CLOSE,    0,                        LogViewer::OnClose),
    FXMAPFUNC(SEL_COMMAND,  LogViewer::ID_LIST,       LogViewer::OnList),
    FXMAPFUNC(SEL_COMMAND,  LogViewer::ID_SEARCH,     LogViewer::OnSearch),
    FXMAPFUNC(SEL_COMMAND,  LogViewer::ID_SEARCHNEXT, LogViewer::OnSearchNext),
    FXMAPFUNC(SEL_KEYPRESS, 0,                        LogViewer::OnKeyPress)
};

FXIMPLEMENT(LogViewer, FXTopWindow, LogViewerMap, ARRAYNUMBER(LogViewerMap))

LogViewer::LogViewer(FXApp *app, const FXString &lpath)
        : FXTopWindow(app, _("dxirc - log viewer"), NULL, NULL, DECOR_ALL, 0,0,800,500, 0,0,0,0, 0,0), logPath(lpath)
{
    setIcon(smallicon);

    buttonframe = new FXHorizontalFrame(this, LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X);
    buttonClose = new FXButton(buttonframe, _("&Close"), NULL, this, ID_CLOSE, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 10,10,2,5);

    content = new FXHorizontalFrame(this,  LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y);

    splitter = new FXSplitter(content, LAYOUT_FILL_X | LAYOUT_FILL_Y | SPLITTER_REVERSED | SPLITTER_TRACKING);
    textframe = new FXVerticalFrame(splitter, FRAME_SUNKEN | FRAME_THICK | LAYOUT_FILL_X | LAYOUT_FILL_Y);
    text = new FXText(textframe, NULL, 0, FRAME_SUNKEN | LAYOUT_FILL_X | LAYOUT_FILL_Y | TEXT_WORDWRAP | TEXT_READONLY);
    text->setVisibleColumns(80);
    listframe = new FXVerticalFrame(splitter, FRAME_SUNKEN | FRAME_THICK | LAYOUT_FILL_X | LAYOUT_FILL_Y | LAYOUT_FIX_WIDTH);
    searchframe = new FXHorizontalFrame(listframe, FRAME_SUNKEN | FRAME_THICK | LAYOUT_FILL_X);
    searchfield = new FXTextField(searchframe, 25, this, ID_SEARCH, TEXTFIELD_ENTER_ONLY|FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    searchfield->setTipText(_("F3 for next"));
    searchfield->disable();
    buttonSearch = new FXButton(searchframe, _("&Search"), NULL, this, ID_SEARCH, BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X);
    buttonSearch->disable();
    treeframe = new FXVerticalFrame(listframe, FRAME_SUNKEN | FRAME_THICK | LAYOUT_FILL_X | LAYOUT_FILL_Y);
    listHistory = new FXTreeList(treeframe, this, ID_LIST, TREELIST_SHOWS_BOXES | TREELIST_SHOWS_LINES | FRAME_SUNKEN | LAYOUT_FILL_X | LAYOUT_FILL_Y);
    listHistory->setSortFunc(FXTreeList::ascendingCase);
    listHistory->setScrollStyle(HSCROLLING_OFF);

    getAccelTable()->addAccel(MKUINT(KEY_F3, 0), this, FXSEL(SEL_COMMAND,LogViewer::ID_SEARCHNEXT));

    searchstring = "";
}

LogViewer::~LogViewer()
{
}

void LogViewer::create()
{
    FXTopWindow::create();
    show(PLACEMENT_CURSOR);
    LoadTree();
}

long LogViewer::OnClose(FXObject*, FXSelector, void*)
{
    listHistory->clearItems(TRUE);
    text->removeText(0, text->getLength());
    searchstring.clear();
    hide();
    return 1;
}

long LogViewer::OnSearch(FXObject*, FXSelector, void*)
{
    FXint beg[10];
    FXint end[10];
    FXint pos;
    if(text->getLength())
    { 
        pos = text->getCursorPos();
        if(!searchfield->getText().empty())
        {
            searchstring = searchfield->getText();
            if(text->findText(searchstring, beg, end, pos, SEARCH_FORWARD|SEARCH_WRAP|SEARCH_IGNORECASE, 10))
            {
                text->setAnchorPos(beg[0]);
                text->extendSelection(end[0],SELECT_CHARS,TRUE);
                text->setCursorPos(end[0],TRUE);
                text->makePositionVisible(beg[0]);
                text->makePositionVisible(end[0]);
            }
            else
                getApp()->beep();
            return 1;
        }
    }
    return 0;
}

long LogViewer::OnSearchNext(FXObject*, FXSelector, void*)
{
    FXint beg, end;
    FXint pos = text->getCursorPos();

    if(!searchstring.empty())
    {
        if(text->findText(searchstring, &beg, &end, pos, SEARCH_FORWARD|SEARCH_WRAP|SEARCH_IGNORECASE))
        {
            text->setAnchorPos(beg);
            text->extendSelection(end,SELECT_CHARS,TRUE);
            text->setCursorPos(end,TRUE);
            text->makePositionVisible(beg);
            text->makePositionVisible(end);
        }
        else
            getApp()->beep();
    }
    return 1;
}

long LogViewer::OnList(FXObject*, FXSelector, void *ptr)
{
    FXDirItem *item = (FXDirItem*)ptr;
    if(item->isFile())
    {
        LoadFile(GetItemPathname(item));
        searchfield->enable();
        buttonSearch->enable();
    }
    else
    {
        text->removeText(0, text->getLength());
        searchfield->disable();
        buttonSearch->disable();
    }
    return 1;
}

long LogViewer::OnKeyPress(FXObject *sender, FXSelector sel, void *ptr)
{
    FXEvent *event = (FXEvent*)ptr;
    if(event->code == KEY_Escape)
    {
        handle(this, FXSEL(SEL_COMMAND, ID_CLOSE), NULL);
        return 1;
    }
    else
    {
        if(FXTopWindow::onKeyPress(sender,sel,ptr))
            return 1;
    }
    return 0;
}

// Load file thanks for xfe again
FXbool LogViewer::LoadFile(const FXString& file)
{
    FXFile textfile(file,FXFile::Reading);
    FXint size, n, c, i, j;
    FXchar *txt;

    // Opened file?
    if(!textfile.isOpen())
    {
        FXMessageBox::error(this,MBOX_OK,_("Error Loading File"),_("Unable to open file: %s"),file.text());
        return FALSE;
    }

    // Get file size
    size=textfile.size();

    // Make buffer to load file
    if(!FXMALLOC(&txt,FXchar,size))
    {
        FXMessageBox::error(this,MBOX_OK,_("Error Loading File"),_("File is too big: %s (%d bytes)"),file.text(),size);
        return FALSE;
    }

    // Set wait cursor
    getApp()->beginWaitCursor();

    // Read the file
    n=textfile.readBlock(txt,size);
    if(n<0)
    {
        FXFREE(&txt);
        FXMessageBox::error(this,MBOX_OK,_("Error Loading File"),_("Unable to read file: %s"),file.text());
        getApp()->endWaitCursor();
        return FALSE;
    }

    // Strip carriage returns
    for(i=j=0; j<n; j++)
    {
        c=txt[j];
        if(c!='\r')
            txt[i++]=c;
    }
    n=i;

    // Set text
    text->setText(txt,n);
    FXFREE(&txt);

    // Kill wait cursor
    getApp()->endWaitCursor();

    return TRUE;
}

FXString LogViewer::GetItemPathname(const FXTreeItem* item)
{
    FXString pathname;
    if (item)
    {
        if(item == listHistory->getFirstItem())
            return logPath;
        while (1)
        {
            if(item!=listHistory->getFirstItem()) pathname.prepend(item->getText());
            item = item->getParent();
            if(!item) break;
            if(item->getParent()) pathname.prepend(PATHSEP);
        }
    }
    return pathname.prepend(logPath+PATHSEPSTRING);
}

void LogViewer::LoadTree()
{
    FXString pathname;
    LogItem *item;
    FXStat info;
    //listHistory->appendItem(NULL, "Logs");
    LogItem *ritem = new LogItem(_("Logs"), NULL, NULL, NULL);
    ritem->setDraggable(FALSE);
    ritem->state = LogItem::FOLDER|LogItem::HASITEMS;
    listHistory->appendItem(NULL, ritem, TRUE);
    listHistory->sortRootItems();
    item=(LogItem*)listHistory->getFirstItem();
    while (item)
    {
        //if (item->isDirectory() && item->isExpanded())
        if (item->isDirectory())
        {
            pathname = GetItemPathname(item);
            FXStat::statFile(pathname, info);
            FXTime newdate = info.touched();
            /*if (force || (item->date != newdate) || (counter == 0))
            {*/
                ListChildItems(item);
                listHistory->sortChildItems(item);
                item->date = newdate;
            //}
            if (item->first)
            {
                item = (LogItem*)item->first;
                continue;
            }
        }
        while (!item->next && item->parent)
        {
            item = (LogItem*) item->parent;
        }
        item = (LogItem*) item->next;
    }
    listHistory->expandTree(ritem, TRUE);
}

// modified FXDirList::listChildItems(FXDirItem *par)
void LogViewer::ListChildItems(LogItem *par)
{
    LogItem *oldlist, *newlist, **po, **pp, **pn, *item, *link;
    FXIcon *openicon;
    FXIcon *closedicon;
    FXFileAssoc *fileassoc;
    FXString pathname;
    FXString directory;
    FXString name;
    FXStat info;
    FXint islink;
    FXDir dir;

    // Path to parent node
    directory = GetItemPathname(par);

    // Build new insert-order list
    oldlist = par->list;
    newlist = NULL;

    // Assemble lists
    po = &oldlist;
    pn = &newlist;

    // Assume not a link
    islink = FALSE;

    // Managed to open directory
    if (dir.open(directory))
    {

        // Process directory entries
        while (dir.next())
        {

            // Get name of entry
            name = dir.name();

            // A dot special file?
            if (name[0] == '.' && (name[1] == 0 || (name[1] == '.' && name[2] == 0))) continue;

            // Hidden file or directory normally not shown
            if (name[0] == '.' && !(options & DIRLIST_SHOWHIDDEN)) continue;

            // Build full pathname of entry
            pathname = directory;
            if (!ISPATHSEP(pathname[pathname.length() - 1])) pathname += PATHSEPSTRING;
            pathname += name;

#ifndef WIN32

            // Get file/link info
            if (!FXStat::statLink(pathname, info)) continue;

            // If its a link, get the info on file itself
            islink = info.isLink();
            if (islink && !FXStat::statFile(pathname, info)) continue;

#else

            // Get file/link info
            if (!FXStat::statFile(pathname, info)) continue;

            // Hidden file or directory normally not shown
            if (info.isHidden() && !(options & DIRLIST_SHOWHIDDEN)) continue;

#endif

            // If it is not a directory, and not showing files and matching pattern skip it
            //if (!info.isDirectory() && FXPath::match("*", name))continue;

            // Find it, and take it out from the old list if found
            for (pp = po; (item = *pp) != NULL; pp = &item->link)
            {
                if (compare(item->label, name) == 0)
                {
                    *pp = item->link;
                    item->link = NULL;
                    po = pp;
                    goto fnd;
                }
            }

            // Not found; prepend before list
            //item = (LogItem*) listHistory->appendItem(par, name, ofoldericon, foldericon, NULL, TRUE);
            item = new LogItem(name, ofoldericon, foldericon, NULL);
            listHistory->appendItem(par, item, TRUE);

            // Next gets hung after this one
fnd:
            *pn = item;
            pn = &item->link;

            // Item flags
            if (info.isExecutable())
            {
                item->state |= FXDirItem::EXECUTABLE;
            }
            else
            {
                item->state &= ~FXDirItem::EXECUTABLE;
            }
            if (info.isDirectory())
            {
                item->state |= FXDirItem::FOLDER;
                item->state &= ~FXDirItem::EXECUTABLE;
            }
            else
            {
                item->state &= ~(FXDirItem::FOLDER | FXDirItem::HASITEMS);
            }
            if (info.isCharacter())
            {
                item->state |= FXDirItem::CHARDEV;
                item->state &= ~FXDirItem::EXECUTABLE;
            }
            else
            {
                item->state &= ~FXDirItem::CHARDEV;
            }
            if (info.isBlock())
            {
                item->state |= FXDirItem::BLOCKDEV;
                item->state &= ~FXDirItem::EXECUTABLE;
            }
            else
            {
                item->state &= ~FXDirItem::BLOCKDEV;
            }
            if (info.isFifo())
            {
                item->state |= FXDirItem::FIFO;
                item->state &= ~FXDirItem::EXECUTABLE;
            }
            else
            {
                item->state &= ~FXDirItem::FIFO;
            }
            if (info.isSocket())
            {
                item->state |= FXDirItem::SOCK;
                item->state &= ~FXDirItem::EXECUTABLE;
            }
            else
            {
                item->state &= ~FXDirItem::SOCK;
            }
            if (islink)
            {
                item->state |= FXDirItem::SYMLINK;
            }
            else
            {
                item->state &= ~FXDirItem::SYMLINK;
            }

            // We can drag items only if we can drop them to
            item->setDraggable(FALSE);

            // Assume no associations
            fileassoc = NULL;

            // Determine icons and type
            if (item->isDirectory())
            {
                openicon = ofoldericon;
                closedicon = foldericon;
            }
            else
            {
                openicon = fileicon;
                closedicon = fileicon;
            }

            // Update item information
            item->openIcon = openicon;
            item->closedIcon = closedicon;
            item->size = info.size();
            //item->assoc = fileassoc;
            item->date = info.modified();

            // Create item
            if (id()) item->create();
        }

        // Close it
        dir.close();
    }

    // Wipe items remaining in list:- they have disappeared!!
    for (item = oldlist; item; item = link)
    {
        link = item->link;
        listHistory->removeItem(item, TRUE);
    }

    // Now we know for sure whether we really have subitems or not
    if (par->first)
        par->state |= FXDirItem::HASITEMS;
    else
        par->state &= ~FXDirItem::HASITEMS;

    // Remember new list
    par->list = newlist;

    // Need to layout
    listHistory->recalc();
}



