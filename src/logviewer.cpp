/*
 *      logviewer.cpp
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
#include <string>
#include <fstream>

FXIMPLEMENT(LogItem, FXTreeItem, NULL, 0)

FXDEFMAP(LogViewer) LogViewerMap[] = {
    FXMAPFUNC(SEL_COMMAND,  LogViewer::ID_CLOSE,      LogViewer::OnClose),
    FXMAPFUNC(SEL_CLOSE,    0,                        LogViewer::OnClose),
    FXMAPFUNC(SEL_COMMAND,  LogViewer::ID_TREE,       LogViewer::OnTree),
    FXMAPFUNC(SEL_EXPANDED, LogViewer::ID_TREE,       LogViewer::OnTree),
    FXMAPFUNC(SEL_RIGHTBUTTONRELEASE,   LogViewer::ID_TREE, LogViewer::OnRightTree),
    FXMAPFUNC(SEL_COMMAND,  LogViewer::ID_RESET,      LogViewer::OnReset),
    FXMAPFUNC(SEL_COMMAND,  LogViewer::ID_SEARCH,     LogViewer::OnSearch),
    FXMAPFUNC(SEL_COMMAND,  LogViewer::ID_SEARCHNEXT, LogViewer::OnSearchNext),
    FXMAPFUNCS(SEL_COMMAND, LogViewer::ID_ALL, LogViewer::ID_FILE, LogViewer::OnCmdSearchOptions),
    FXMAPFUNC(SEL_COMMAND,  LogViewer::ID_PACK,       LogViewer::OnPack),
    FXMAPFUNC(SEL_COMMAND,  LogViewer::ID_UNPACK,     LogViewer::OnUnpack),
    FXMAPFUNC(SEL_COMMAND,  LogViewer::ID_DELETEITEM,     LogViewer::OnDelete),
    FXMAPFUNC(SEL_KEYPRESS, 0,                        LogViewer::OnKeyPress)
};

FXIMPLEMENT(LogViewer, FXTopWindow, LogViewerMap, ARRAYNUMBER(LogViewerMap))

LogViewer::LogViewer(FXApp *app, const FXString &lpath, FXFont *fnt)
        : FXTopWindow(app, _("dxirc - log viewer"), NULL, NULL, DECOR_ALL, 0,0,800,500, 0,0,0,0, 0,0), logPath(lpath)
{
    setIcon(bigicon);
    setMiniIcon(smallicon);

    all = TRUE;
    channel = FALSE;
    file = FALSE;
    icase = FALSE;
    treeLoaded = FALSE;
    targetAll.connect(all);
    targetAll.setTarget(this);
    targetAll.setSelector(ID_ALL);
    targetChannel.connect(channel);
    targetChannel.setTarget(this);
    targetChannel.setSelector(ID_CHANNEL);
    targetFile.connect(file);
    targetFile.setTarget(this);
    targetFile.setSelector(ID_FILE);
    targetIcase.connect(icase);

    buttonframe = new FXHorizontalFrame(this, LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X);
    buttonClose = new FXButton(buttonframe, _("C&lose"), NULL, this, ID_CLOSE, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 10,10,2,5);

    content = new FXHorizontalFrame(this,  LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y);

    splitter = new FXSplitter(content, LAYOUT_FILL_X | LAYOUT_FILL_Y | SPLITTER_REVERSED | SPLITTER_TRACKING);
    textframe = new FXVerticalFrame(splitter, FRAME_SUNKEN | FRAME_THICK | LAYOUT_FILL_X | LAYOUT_FILL_Y);
    text = new FXText(textframe, NULL, 0, FRAME_SUNKEN | LAYOUT_FILL_X | LAYOUT_FILL_Y | TEXT_WORDWRAP | TEXT_READONLY);
    text->setFont(fnt);
    text->setVisibleColumns(80);
    listframe = new FXVerticalFrame(splitter, FRAME_SUNKEN | FRAME_THICK | LAYOUT_FILL_X | LAYOUT_FILL_Y | LAYOUT_FIX_WIDTH);
    searchframe = new FXHorizontalFrame(listframe, FRAME_THICK | LAYOUT_FILL_X);
    searchfield = new FXTextField(searchframe, 15, this, ID_SEARCH, TEXTFIELD_ENTER_ONLY|FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    searchfield->setTipText(_("F3 for next"));
    searchfield->disable();
    buttonSearch = new FXButton(searchframe, _("&Search"), NULL, this, ID_SEARCH, BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 10,10,2,5);
    buttonSearch->disable();
    buttonReset = new FXButton(searchframe, _("&Reset"), NULL, this, ID_RESET, BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 10,10,2,5);
    buttonReset->disable();
    group = new FXGroupBox(listframe, _("Search options"), LAYOUT_SIDE_TOP|FRAME_GROOVE|LAYOUT_FILL_X, 0,0,0,0);
    buttonIcase = new FXCheckButton(group, _("&Ignore case"), &targetIcase, FXDataTarget::ID_VALUE);
    buttonIcase->disable();
    buttonFile = new FXRadioButton(group, _("Search file"), &targetFile, FXDataTarget::ID_VALUE);
    buttonFile->disable();
    buttonChannel = new FXRadioButton(group, _("Search channel/query"), &targetChannel, FXDataTarget::ID_VALUE);
    buttonChannel->disable();
    buttonAll = new FXRadioButton(group, _("Search all"), &targetAll, FXDataTarget::ID_VALUE);
    buttonAll->disable();
    treeframe = new FXVerticalFrame(listframe, FRAME_THICK | LAYOUT_FILL_X | LAYOUT_FILL_Y);
    treeHistory = new FXTreeList(treeframe, this, ID_TREE, TREELIST_SHOWS_BOXES | TREELIST_SHOWS_LINES | FRAME_SUNKEN | LAYOUT_FILL_X | LAYOUT_FILL_Y);
    treeHistory->setSortFunc(FXTreeList::ascendingCase);
    treeHistory->setScrollStyle(HSCROLLING_OFF);
    getAccelTable()->addAccel(MKUINT(KEY_F3, 0), this, FXSEL(SEL_COMMAND,LogViewer::ID_SEARCHNEXT));

    searchstring = "";
    itemOnRight = NULL;
}

LogViewer::~LogViewer()
{
}

void LogViewer::create()
{
    FXTopWindow::create();
    show(PLACEMENT_CURSOR);
    LoadTree();
    searchfield->disable();
    buttonSearch->disable();
    buttonReset->disable();
    buttonIcase->disable();
    buttonFile->disable();
    buttonChannel->disable();
    buttonAll->disable();
}

void LogViewer::SetFont(FXFont *fnt)
{
    text->setFont(fnt);
}

long LogViewer::OnClose(FXObject*, FXSelector, void*)
{
    treeHistory->clearItems(TRUE);
    text->removeText(0, text->getLength());
    searchfield->setText("");
    searchstring.clear();
    hide();
    return 1;
}

long LogViewer::OnSearch(FXObject*, FXSelector, void*)
{
    if(file)
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
                searchfield->setText("");
                FXuint flags = SEARCH_FORWARD|SEARCH_WRAP;
                if(icase) flags |= SEARCH_IGNORECASE;
                if(text->findText(searchstring, beg, end, pos, flags, 10))
                {
                    text->setAnchorPos(beg[0]);
                    text->extendSelection(end[0],SELECT_CHARS,TRUE);
                    text->setCursorPos(end[0],TRUE);
                    text->makePositionVisible(beg[0]);
                    text->makePositionVisible(end[0]);
                    buttonReset->enable();
                }
                else
                {
                    getApp()->beep();                    
                    FXMessageBox::information(this, MBOX_OK, _("Information"), _("'%s' wasn't found"), searchstring.text());
                }
                return 1;
            }
        }
    }
    else if(channel)
    {        
        FXint count = 0;
        FXRex rex;
        FXint rexmode = REX_VERBATIM;
        if(icase) rexmode |= REX_ICASE;
        if(!searchfield->getText().empty())
        {
            searchstring = searchfield->getText();
            searchfield->setText("");
            text->removeText(0, text->getLength());
            if(rex.parse(searchstring, rexmode) == REGERR_OK)
            {
                getApp()->beginWaitCursor();
                LogItem *item = (LogItem*)treeHistory->getCurrentItem();
                if(item->isFile()) item = (LogItem*)item->parent;
                if(!IsChannelItem(item)) return 0;
                LogItem *chitem = item;
                item = (LogItem*)chitem->first;
                while(item)
                {
                    std::ifstream stream(GetItemPathname(item).text());
                    std::string temp;
                    bool contain = FALSE;
                    while(std::getline(stream, temp))
                    {
                        if(rex.match(temp.c_str()))
                        {
                            contain = TRUE;
                            count++;
                            break;
                        }
                    }
                    if(contain) treeHistory->enableItem(item);
                    else treeHistory->disableItem(item);
                    item = (LogItem*)item->next;
                }
                getApp()->endWaitCursor();
                if(count == 0)
                {
                    FXMessageBox::information(this, MBOX_OK, _("Information"), _("'%s' wasn't found"), searchstring.text());
                    item = (LogItem*)chitem->first;
                    while(item)
                    {
                        treeHistory->enableItem(item);
                        item = (LogItem*)item->next;
                    }
                }
            }
            return 1;
        }
    }
    else //search over all files
    {
        //we need load full tree
        if(!treeLoaded)
        {
            getApp()->beginWaitCursor();
            LogItem *item = (LogItem*)treeHistory->getFirstItem();
            while(item!=NULL)
            {
                ListChildItems(item);
                item=(LogItem*)item->getBelow();
            }
            getApp()->endWaitCursor();
            treeLoaded = TRUE;
        }
        FXint count = 0;
        FXRex rex;
        FXint rexmode = REX_VERBATIM;
        if(icase) rexmode |= REX_ICASE;
        if(!searchfield->getText().empty())
        {
            searchstring = searchfield->getText();
            searchfield->setText("");
            text->removeText(0, text->getLength());
            if(rex.parse(searchstring, rexmode) == REGERR_OK)
            {
                getApp()->beginWaitCursor();
                LogItem *item = (LogItem*)treeHistory->getFirstItem();
                item = (LogItem*)item->first;
                while(item)
                {
                    if(item->isDirectory())
                    {
                        if(IsChannelItem(item))
                        {
                            FXint hidden = 0;
                            LogItem *fitem = (LogItem*)item->first;
                            while(fitem)
                            {
                                std::ifstream stream(GetItemPathname(fitem).text());
                                std::string temp;
                                bool contain = FALSE;
                                while(std::getline(stream, temp))
                                {
                                    if(rex.match(temp.c_str()))
                                    {
                                        contain = TRUE;
                                        count++;
                                        break;
                                    }
                                }
                                if(contain) treeHistory->enableItem(fitem);
                                else
                                {
                                    treeHistory->disableItem(fitem);
                                    hidden++;
                                }
                                fitem = (LogItem*)fitem->next;
                            }
                            if(item->getNumChildren() == hidden) treeHistory->disableItem(item);
                        }
                        if(item->first)
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
                getApp()->endWaitCursor();
                if(count == 0)
                {
                    FXMessageBox::information(this, MBOX_OK, _("Information"), _("'%s' wasn't found"), searchstring.text());
                    EnableAllItems();
                }
            }
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
        if(text->getLength())
        {
            FXuint flags = SEARCH_FORWARD|SEARCH_WRAP;
            if(icase) flags |= SEARCH_IGNORECASE;
            if(text->findText(searchstring, &beg, &end, pos, flags))
            {
                text->setAnchorPos(beg);
                text->extendSelection(end,SELECT_CHARS,TRUE);
                text->setCursorPos(end,TRUE);
                text->makePositionVisible(beg);
                text->makePositionVisible(end);
            }
            else getApp()->beep();
        }
    }
    return 1;
}

long LogViewer::OnReset(FXObject*, FXSelector, void*)
{
    searchstring.clear();
    text->setSelection(0,0);
    searchfield->setFocus();
    EnableAllItems();
    return 1;
}

long LogViewer::OnTree(FXObject*, FXSelector, void *ptr)
{
    LogItem *item = (LogItem*)ptr;
    if(item->isFile())
    {
        if(!LoadFile(GetItemPathname(item)))
        {
            LogItem *parent = (LogItem*)item->getParent();
            treeHistory->setCurrentItem(parent, TRUE);
            Scan();
            text->removeText(0, text->getLength());
            buttonIcase->enable();
            buttonFile->disable();
            file = FALSE;
            if(IsChannelItem(parent))
            {
                buttonChannel->enable();
                channel = TRUE;
                all = FALSE;
            }
            else
            {
                buttonChannel->disable();
                channel = FALSE;
                all = TRUE;
            }
            buttonAll->enable();
            searchfield->enable();
            buttonSearch->enable();
            if(!searchstring.empty()) buttonReset->enable();
            return 1;
        }
        OnSearchNext(NULL, 0, NULL);
        buttonIcase->enable();
        buttonFile->enable();
        buttonChannel->enable();
        buttonAll->enable();
        file = TRUE;
        channel = FALSE;
        all = FALSE;
    }
    else
    {
        text->removeText(0, text->getLength());
        Scan();
        buttonIcase->enable();
        buttonFile->disable();
        file = FALSE;
        if(IsChannelItem(item))
        {
            buttonChannel->enable();
            channel = TRUE;
            all = FALSE;
        }
        else
        {
            buttonChannel->disable();
            channel = FALSE;
            all = TRUE;
        }
        buttonAll->enable();
    }
    searchfield->enable();
    buttonSearch->enable();
    if(!searchstring.empty()) buttonReset->enable();
    return 1;
}

long LogViewer::OnRightTree(FXObject*, FXSelector, void *ptr)
{
    FXEvent* event = (FXEvent*)ptr;
    if(event->moved) return 1;
    LogItem *item = (LogItem*)treeHistory->getItemAt(event->win_x,event->win_y);
    if(item)
    {
        if(item == treeHistory->getFirstItem())
            return 1;
        itemOnRight = item;
        FXMenuPane popup(this);
        if(item->isExpanded())
            new FXMenuCommand(&popup, _("Collapse"), NULL, this, ID_PACK);
        else
            new FXMenuCommand(&popup, _("Expand"), NULL, this, ID_UNPACK);
        new FXMenuCommand(&popup, _("Delete"), NULL, this, ID_DELETEITEM);
        popup.create();
        popup.popup(NULL,event->root_x,event->root_y);
        getApp()->runModalWhileShown(&popup);
    }
    return 1;
}

long LogViewer::OnPack(FXObject*, FXSelector, void*)
{
    if(itemOnRight)
    {
        treeHistory->collapseTree(itemOnRight, TRUE);
    }
    return 1;
}

long LogViewer::OnUnpack(FXObject*, FXSelector, void*)
{
    if(itemOnRight)
    {
        Scan();
        treeHistory->expandTree(itemOnRight, TRUE);
    }
    return 1;
}

long LogViewer::OnDelete(FXObject*, FXSelector, void*)
{
    if(itemOnRight)
    {
        if(itemOnRight == treeHistory->getFirstItem())
            return 1;
        FXString message;
        if(itemOnRight->isDirectory())
        {
            if(itemOnRight->hasItems())
                message = FXStringFormat(_("Delete %s with all child items?\nThis cann't be UNDONE!"), itemOnRight->getText().text());
            else
                message = FXStringFormat(_("Delete %s?\nThis cann't be UNDONE!"), itemOnRight->getText().text());
        }
        else if(itemOnRight->isFile())
            message = FXStringFormat(_("Delete file %s?\nThis cann't be UNDONE!"), itemOnRight->getText().text());
        else
            return 1;
        if(FXMessageBox::question(this, MBOX_YES_NO, _("Question"), message.text()) == 1)
        {
            FXFile::removeFiles(GetItemPathname(itemOnRight), TRUE);
            Scan();
        }
        else
            return 1;
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

long LogViewer::OnCmdSearchOptions(FXObject*, FXSelector sel, void*)
{
    switch(FXSELID(sel))
    {
    case ID_ALL:
        channel = FALSE;
        file = FALSE;
        all = TRUE;
        break;
    case ID_CHANNEL:
        all = FALSE;
        file = FALSE;
        channel = TRUE;
        break;
    case ID_FILE:
        all = FALSE;
        channel = FALSE;
        file = TRUE;
        break;
    }
    return 1;
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

void LogViewer::EnableAllItems()
{
    LogItem *item = (LogItem*)treeHistory->getFirstItem();
    item = (LogItem*)item->first;
    while(item)
    {
        if(item->isDirectory())
        {
            if(IsChannelItem(item))
            {
                LogItem *fitem = (LogItem*)item->first;
                while(fitem)
                {
                    treeHistory->enableItem(fitem);
                    fitem = (LogItem*)fitem->next;
                }
            }
            treeHistory->enableItem(item);
            if(item->first)
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
}

FXString LogViewer::GetItemPathname(const FXTreeItem* item)
{
    FXString pathname;
    if (item)
    {
        if(item == treeHistory->getFirstItem())
            return logPath;
        while (1)
        {
            if(item!=treeHistory->getFirstItem()) pathname.prepend(item->getText());
            item = item->getParent();
            if(!item) break;
            if(item->getParent()) pathname.prepend(PATHSEP);
        }
        return pathname.prepend(logPath+PATHSEPSTRING);
    }
    return FXString::null;
}

FXbool LogViewer::IsChannelItem(const LogItem* item)
{
    if(item->hasItems())
    {
        item = (LogItem*)item->first;
        while(item)
        {
            if(item->isFile()) return TRUE;
            item = (LogItem*)item->next;
        }
    }
    return FALSE;
}

void LogViewer::LoadTree()
{
    LogItem *ritem = new LogItem(_("Logs"), NULL, NULL, NULL);
    ritem->setDraggable(FALSE);
    ritem->state = LogItem::FOLDER|LogItem::HASITEMS;
    treeHistory->appendItem(NULL, ritem, TRUE);
    getApp()->beginWaitCursor();
    ListChildItems(ritem);
    treeHistory->sortChildItems(ritem);
    treeHistory->expandTree(ritem, TRUE);
    getApp()->endWaitCursor();
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
            if (name[0] == '.') continue;

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
            if (info.isHidden()) continue;

#endif

            // If it is not a directory, and not showing files and matching pattern skip it
            if (!info.isDirectory() && !IsRightFile(pathname,name))continue;

            // Find it, and take it out from the old list if found
            for(pp = po; (item = *pp) != NULL; pp = &item->link)
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
            treeHistory->appendItem(par, item, TRUE);

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
    for(item = oldlist; item; item = link)
    {
        link = item->link;
        treeHistory->removeItem(item, TRUE);
    }

    // Now we know for sure whether we really have subitems or not
    if (par->first)
        par->state |= FXDirItem::HASITEMS;
    else
        par->state &= ~FXDirItem::HASITEMS;

    // Remember new list
    par->list = newlist;

    // Need to layout
    treeHistory->recalc();
}

FXbool LogViewer::IsRightFile(const FXString& path, const FXString& name)
{
    if(FXRex("^\\d\\d\\d\\d-\\d\\d-\\d\\d+$").match(name))
    {
        FXFile textfile(path,FXFile::Reading);
        FXint n;
        FXchar *txt;
        if(!textfile.isOpen()) return FALSE;
        if(!FXMALLOC(&txt,FXchar,10)) return FALSE;
        n = textfile.readBlock(txt,10);
        if(n<0)
        {
            FXFREE(&txt);
            return FALSE;
        }
        if(FXRex("^\\[\\d\\d:\\d\\d:\\d\\d\\]+$").match(txt,10))
        {
            FXFREE(&txt);
            return TRUE;
        }
        else
        {
            FXFREE(&txt);
            return FALSE;
        }
    }
    return FALSE;
}

//inspired by FXDirList
void LogViewer::Scan()
{
    FXString pathname;
    LogItem *item;
    FXStat info;
    if(!treeHistory->getFirstItem())
        LoadTree();
    item = (LogItem*)treeHistory->getFirstItem();
    getApp()->beginWaitCursor();
    while(item)
    {
        if(item->isDirectory() /*&& item->isExpanded()*/)
        {
          // Get the full path of the item
          pathname=GetItemPathname(item);
          // Stat this directory; should not fail as parent has been scanned already
          FXStat::statFile(pathname,info);
          // Get the mod date of the item
          FXTime newdate=info.touched();
          // date was changed
          if(item->date!=newdate || (!item->getNumChildren() && item->isExpanded()))
          {
              // And do the refresh
              ListChildItems(item);
              treeHistory->sortChildItems(item);
              // Remember when we did this
              item->date=newdate;
          }
          // Go deeper
          if(item->first)
          {
              item=(LogItem*)item->first;
              continue;
          }
        }
        // Go up
        while(!item->next && item->parent)
        {
            item=(LogItem*)item->parent;
        }
        // Go to next
        item=(LogItem*)item->next;
    }
    getApp()->endWaitCursor();
}


