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
    FXMAPFUNC(SEL_COMMAND,  LogViewer::ID_CLOSE,      LogViewer::onClose),
    FXMAPFUNC(SEL_CLOSE,    0,                        LogViewer::onClose),
    FXMAPFUNC(SEL_COMMAND,  LogViewer::ID_TREE,       LogViewer::onTree),
    FXMAPFUNC(SEL_EXPANDED, LogViewer::ID_TREE,       LogViewer::onTree),
    FXMAPFUNC(SEL_RIGHTBUTTONRELEASE,   LogViewer::ID_TREE, LogViewer::onRightTree),
    FXMAPFUNC(SEL_COMMAND,  LogViewer::ID_RESET,      LogViewer::onReset),
    FXMAPFUNC(SEL_COMMAND,  LogViewer::ID_SEARCH,     LogViewer::onSearch),
    FXMAPFUNC(SEL_COMMAND,  LogViewer::ID_SEARCHNEXT, LogViewer::onSearchNext),
    FXMAPFUNCS(SEL_COMMAND, LogViewer::ID_ALL, LogViewer::ID_FILE, LogViewer::onCmdSearchOptions),
    FXMAPFUNC(SEL_COMMAND,  LogViewer::ID_PACK,       LogViewer::onPack),
    FXMAPFUNC(SEL_COMMAND,  LogViewer::ID_UNPACK,     LogViewer::onUnpack),
    FXMAPFUNC(SEL_COMMAND,  LogViewer::ID_DELETEITEM,     LogViewer::onDelete),
    FXMAPFUNC(SEL_KEYPRESS, 0,                        LogViewer::onKeyPress)
};

FXIMPLEMENT(LogViewer, FXTopWindow, LogViewerMap, ARRAYNUMBER(LogViewerMap))

LogViewer::LogViewer(FXApp *app, const FXString &lpath, FXFont *fnt)
        : FXTopWindow(app, _("dxirc - log viewer"), NULL, NULL, DECOR_ALL, 0,0,800,500, 0,0,0,0, 0,0), m_logPath(lpath)
{
    setIcon(ICO_BIG);
    setMiniIcon(ICO_SMALL);

    m_all = TRUE;
    m_channel = FALSE;
    m_file = FALSE;
    m_icase = FALSE;
    m_treeLoaded = FALSE;
    m_targetAll.connect(m_all);
    m_targetAll.setTarget(this);
    m_targetAll.setSelector(ID_ALL);
    m_targetChannel.connect(m_channel);
    m_targetChannel.setTarget(this);
    m_targetChannel.setSelector(ID_CHANNEL);
    m_targetFile.connect(m_file);
    m_targetFile.setTarget(this);
    m_targetFile.setSelector(ID_FILE);
    m_targetIcase.connect(m_icase);

    m_buttonframe = new FXHorizontalFrame(this, LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X);
    m_buttonClose = new FXButton(m_buttonframe, _("C&lose"), NULL, this, ID_CLOSE, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 10,10,2,5);

    m_content = new FXHorizontalFrame(this,  LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y);

    m_splitter = new FXSplitter(m_content, LAYOUT_FILL_X | LAYOUT_FILL_Y | SPLITTER_REVERSED | SPLITTER_TRACKING);
    m_textframe = new FXVerticalFrame(m_splitter, FRAME_SUNKEN | FRAME_THICK | LAYOUT_FILL_X | LAYOUT_FILL_Y);
    m_text = new FXText(m_textframe, NULL, 0, FRAME_SUNKEN | LAYOUT_FILL_X | LAYOUT_FILL_Y | TEXT_WORDWRAP | TEXT_READONLY);
    m_text->setFont(fnt);
    m_text->setVisibleColumns(80);
    m_listframe = new FXVerticalFrame(m_splitter, FRAME_SUNKEN | FRAME_THICK | LAYOUT_FILL_X | LAYOUT_FILL_Y | LAYOUT_FIX_WIDTH);
    m_searchframe = new FXHorizontalFrame(m_listframe, FRAME_THICK | LAYOUT_FILL_X);
    m_searchfield = new FXTextField(m_searchframe, 15, this, ID_SEARCH, TEXTFIELD_ENTER_ONLY|FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    m_searchfield->setTipText(_("F3 for next"));
    m_searchfield->disable();
    m_buttonSearch = new FXButton(m_searchframe, _("&Search"), NULL, this, ID_SEARCH, BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 10,10,2,5);
    m_buttonSearch->disable();
    m_buttonReset = new FXButton(m_searchframe, _("&Reset"), NULL, this, ID_RESET, BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 10,10,2,5);
    m_buttonReset->disable();
    m_group = new FXGroupBox(m_listframe, _("Search options"), LAYOUT_SIDE_TOP|FRAME_GROOVE|LAYOUT_FILL_X, 0,0,0,0);
    m_buttonIcase = new FXCheckButton(m_group, _("&Ignore case"), &m_targetIcase, FXDataTarget::ID_VALUE);
    m_buttonIcase->disable();
    m_buttonFile = new FXRadioButton(m_group, _("Search file"), &m_targetFile, FXDataTarget::ID_VALUE);
    m_buttonFile->disable();
    m_buttonChannel = new FXRadioButton(m_group, _("Search channel/query"), &m_targetChannel, FXDataTarget::ID_VALUE);
    m_buttonChannel->disable();
    m_buttonAll = new FXRadioButton(m_group, _("Search all"), &m_targetAll, FXDataTarget::ID_VALUE);
    m_buttonAll->disable();
    m_treeframe = new FXVerticalFrame(m_listframe, FRAME_THICK | LAYOUT_FILL_X | LAYOUT_FILL_Y);
    m_treeHistory = new FXTreeList(m_treeframe, this, ID_TREE, TREELIST_SHOWS_BOXES | TREELIST_SHOWS_LINES | FRAME_SUNKEN | LAYOUT_FILL_X | LAYOUT_FILL_Y);
    m_treeHistory->setSortFunc(FXTreeList::ascendingCase);
    m_treeHistory->setScrollStyle(HSCROLLING_OFF);
    getAccelTable()->addAccel(MKUINT(KEY_F3, 0), this, FXSEL(SEL_COMMAND,LogViewer::ID_SEARCHNEXT));

    m_searchstring = "";
    m_itemOnRight = NULL;
}

LogViewer::~LogViewer()
{
}

void LogViewer::create()
{
    FXTopWindow::create();
    show(PLACEMENT_CURSOR);
    loadTree();
    m_searchfield->disable();
    m_buttonSearch->disable();
    m_buttonReset->disable();
    m_buttonIcase->disable();
    m_buttonFile->disable();
    m_buttonChannel->disable();
    m_buttonAll->disable();
}

void LogViewer::setFont(FXFont *fnt)
{
    m_text->setFont(fnt);
}

long LogViewer::onClose(FXObject*, FXSelector, void*)
{
    m_treeHistory->clearItems(TRUE);
    m_text->removeText(0, m_text->getLength());
    m_searchfield->setText("");
    m_searchstring.clear();
    hide();
    return 1;
}

long LogViewer::onSearch(FXObject*, FXSelector, void*)
{
    if(m_file)
    {
        FXint beg[10];
        FXint end[10];
        FXint pos;
        if(m_text->getLength())
        {
            pos = m_text->getCursorPos();
            if(!m_searchfield->getText().empty())
            {
                m_searchstring = m_searchfield->getText();
                m_searchfield->setText("");
                FXuint flags = SEARCH_FORWARD|SEARCH_WRAP;
                if(m_icase) flags |= SEARCH_IGNORECASE;
                if(m_text->findText(m_searchstring, beg, end, pos, flags, 10))
                {
                    m_text->setAnchorPos(beg[0]);
                    m_text->extendSelection(end[0],SELECT_CHARS,TRUE);
                    m_text->setCursorPos(end[0],TRUE);
                    m_text->makePositionVisible(beg[0]);
                    m_text->makePositionVisible(end[0]);
                    m_buttonReset->enable();
                }
                else
                {
                    getApp()->beep();                    
                    FXMessageBox::information(this, MBOX_OK, _("Information"), _("'%s' wasn't found"), m_searchstring.text());
                }
                return 1;
            }
        }
    }
    else if(m_channel)
    {        
        FXint count = 0;
        FXRex rex;
        FXint rexmode = REX_VERBATIM;
        if(m_icase) rexmode |= REX_ICASE;
        if(!m_searchfield->getText().empty())
        {
            m_searchstring = m_searchfield->getText();
            m_searchfield->setText("");
            m_text->removeText(0, m_text->getLength());
            if(rex.parse(m_searchstring, rexmode) == REGERR_OK)
            {
                getApp()->beginWaitCursor();
                LogItem *item = (LogItem*)m_treeHistory->getCurrentItem();
                if(item->isFile()) item = (LogItem*)item->parent;
                if(!isChannelItem(item)) return 0;
                LogItem *chitem = item;
                item = (LogItem*)chitem->first;
                while(item)
                {
                    std::ifstream stream(getItemPathname(item).text());
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
                    if(contain) m_treeHistory->enableItem(item);
                    else m_treeHistory->disableItem(item);
                    item = (LogItem*)item->next;
                }
                getApp()->endWaitCursor();
                if(count == 0)
                {
                    FXMessageBox::information(this, MBOX_OK, _("Information"), _("'%s' wasn't found"), m_searchstring.text());
                    item = (LogItem*)chitem->first;
                    while(item)
                    {
                        m_treeHistory->enableItem(item);
                        item = (LogItem*)item->next;
                    }
                }
            }
            return 1;
        }
    }
    else //search over all files
    {
        if(m_searchfield->getText().empty())
            return 1;
        //we need load full tree
        if(!m_treeLoaded)
        {
            getApp()->beginWaitCursor();
            LogItem *item = (LogItem*)m_treeHistory->getFirstItem();
            while(item!=NULL)
            {
                listChildItems(item);
                item=(LogItem*)item->getBelow();
            }
            getApp()->endWaitCursor();
            m_treeLoaded = TRUE;
        }
        FXint count = 0;
        FXRex rex;
        FXint rexmode = REX_VERBATIM;
        if(m_icase) rexmode |= REX_ICASE;
        if(!m_searchfield->getText().empty())
        {
            m_searchstring = m_searchfield->getText();
            m_searchfield->setText("");
            m_text->removeText(0, m_text->getLength());
            if(rex.parse(m_searchstring, rexmode) == REGERR_OK)
            {
                getApp()->beginWaitCursor();
                LogItem *item = (LogItem*)m_treeHistory->getFirstItem();
                item = (LogItem*)item->first;
                while(item)
                {
                    if(item->isDirectory())
                    {
                        if(isChannelItem(item))
                        {
                            FXint hidden = 0;
                            LogItem *fitem = (LogItem*)item->first;
                            while(fitem)
                            {
                                std::ifstream stream(getItemPathname(fitem).text());
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
                                if(contain) m_treeHistory->enableItem(fitem);
                                else
                                {
                                    m_treeHistory->disableItem(fitem);
                                    hidden++;
                                }
                                fitem = (LogItem*)fitem->next;
                            }
                            if(item->getNumChildren() == hidden) m_treeHistory->disableItem(item);
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
                    FXMessageBox::information(this, MBOX_OK, _("Information"), _("'%s' wasn't found"), m_searchstring.text());
                    enableAllItems();
                }
            }
            return 1;
        }
    }
    return 0;
}

long LogViewer::onSearchNext(FXObject*, FXSelector, void*)
{
    FXint beg, end;
    FXint pos = m_text->getCursorPos();

    if(!m_searchstring.empty())
    {
        if(m_text->getLength())
        {
            FXuint flags = SEARCH_FORWARD|SEARCH_WRAP;
            if(m_icase) flags |= SEARCH_IGNORECASE;
            if(m_text->findText(m_searchstring, &beg, &end, pos, flags))
            {
                m_text->setAnchorPos(beg);
                m_text->extendSelection(end,SELECT_CHARS,TRUE);
                m_text->setCursorPos(end,TRUE);
                m_text->makePositionVisible(beg);
                m_text->makePositionVisible(end);
            }
            else getApp()->beep();
        }
    }
    return 1;
}

long LogViewer::onReset(FXObject*, FXSelector, void*)
{
    m_searchstring.clear();
    m_text->setSelection(0,0);
    m_searchfield->setFocus();
    enableAllItems();
    return 1;
}

long LogViewer::onTree(FXObject*, FXSelector, void *ptr)
{
    LogItem *item = (LogItem*)ptr;
    if(item->isFile())
    {
        if(!loadFile(getItemPathname(item)))
        {
            LogItem *parent = (LogItem*)item->getParent();
            m_treeHistory->setCurrentItem(parent, TRUE);
            scan();
            m_text->removeText(0, m_text->getLength());
            m_buttonIcase->enable();
            m_buttonFile->disable();
            m_file = FALSE;
            if(isChannelItem(parent))
            {
                m_buttonChannel->enable();
                m_channel = TRUE;
                m_all = FALSE;
            }
            else
            {
                m_buttonChannel->disable();
                m_channel = FALSE;
                m_all = TRUE;
            }
            m_buttonAll->enable();
            m_searchfield->enable();
            m_buttonSearch->enable();
            if(!m_searchstring.empty()) m_buttonReset->enable();
            return 1;
        }
        onSearchNext(NULL, 0, NULL);
        m_buttonIcase->enable();
        m_buttonFile->enable();
        m_buttonChannel->enable();
        m_buttonAll->enable();
        m_file = TRUE;
        m_channel = FALSE;
        m_all = FALSE;
    }
    else
    {
        m_text->removeText(0, m_text->getLength());
        scan();
        m_buttonIcase->enable();
        m_buttonFile->disable();
        m_file = FALSE;
        if(isChannelItem(item))
        {
            m_buttonChannel->enable();
            m_channel = TRUE;
            m_all = FALSE;
        }
        else
        {
            m_buttonChannel->disable();
            m_channel = FALSE;
            m_all = TRUE;
        }
        m_buttonAll->enable();
    }
    m_searchfield->enable();
    m_buttonSearch->enable();
    if(!m_searchstring.empty()) m_buttonReset->enable();
    return 1;
}

long LogViewer::onRightTree(FXObject*, FXSelector, void *ptr)
{
    FXEvent* event = (FXEvent*)ptr;
    if(event->moved) return 1;
    LogItem *item = (LogItem*)m_treeHistory->getItemAt(event->win_x,event->win_y);
    if(item)
    {
        if(item == m_treeHistory->getFirstItem())
            return 1;
        m_itemOnRight = item;
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

long LogViewer::onPack(FXObject*, FXSelector, void*)
{
    if(m_itemOnRight)
    {
        m_treeHistory->collapseTree(m_itemOnRight, TRUE);
    }
    return 1;
}

long LogViewer::onUnpack(FXObject*, FXSelector, void*)
{
    if(m_itemOnRight)
    {
        scan();
        m_treeHistory->expandTree(m_itemOnRight, TRUE);
    }
    return 1;
}

long LogViewer::onDelete(FXObject*, FXSelector, void*)
{
    if(m_itemOnRight)
    {
        if(m_itemOnRight == m_treeHistory->getFirstItem())
            return 1;
        FXString message;
        if(m_itemOnRight->isDirectory())
        {
            if(m_itemOnRight->hasItems())
                message = FXStringFormat(_("Delete %s with all child items?\nThis cann't be UNDONE!"), m_itemOnRight->getText().text());
            else
                message = FXStringFormat(_("Delete %s?\nThis cann't be UNDONE!"), m_itemOnRight->getText().text());
        }
        else if(m_itemOnRight->isFile())
            message = FXStringFormat(_("Delete file %s?\nThis cann't be UNDONE!"), m_itemOnRight->getText().text());
        else
            return 1;
        if(FXMessageBox::question(this, MBOX_YES_NO, _("Question"), message.text()) == 1)
        {
            FXFile::removeFiles(getItemPathname(m_itemOnRight), TRUE);
            scan();
        }
        else
            return 1;
    }
    return 1;
}

long LogViewer::onKeyPress(FXObject *sender, FXSelector sel, void *ptr)
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

long LogViewer::onCmdSearchOptions(FXObject*, FXSelector sel, void*)
{
    switch(FXSELID(sel))
    {
    case ID_ALL:
        m_channel = FALSE;
        m_file = FALSE;
        m_all = TRUE;
        break;
    case ID_CHANNEL:
        m_all = FALSE;
        m_file = FALSE;
        m_channel = TRUE;
        break;
    case ID_FILE:
        m_all = FALSE;
        m_channel = FALSE;
        m_file = TRUE;
        break;
    }
    return 1;
}

// Load file thanks for xfe again
FXbool LogViewer::loadFile(const FXString& file)
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
    m_text->setText(txt,n);
    FXFREE(&txt);

    // Kill wait cursor
    getApp()->endWaitCursor();

    return TRUE;
}

void LogViewer::enableAllItems()
{
    LogItem *item = (LogItem*)m_treeHistory->getFirstItem();
    item = (LogItem*)item->first;
    while(item)
    {
        if(item->isDirectory())
        {
            if(isChannelItem(item))
            {
                LogItem *fitem = (LogItem*)item->first;
                while(fitem)
                {
                    m_treeHistory->enableItem(fitem);
                    fitem = (LogItem*)fitem->next;
                }
            }
            m_treeHistory->enableItem(item);
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

FXString LogViewer::getItemPathname(const FXTreeItem* item)
{
    FXString pathname;
    if (item)
    {
        if(item == m_treeHistory->getFirstItem())
            return m_logPath;
        while (1)
        {
            if(item!=m_treeHistory->getFirstItem()) pathname.prepend(item->getText());
            item = item->getParent();
            if(!item) break;
            if(item->getParent()) pathname.prepend(PATHSEP);
        }
        return pathname.prepend(m_logPath+PATHSEPSTRING);
    }
    return FXString::null;
}

FXbool LogViewer::isChannelItem(const LogItem* item)
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

void LogViewer::loadTree()
{
    LogItem *ritem = new LogItem(_("Logs"), NULL, NULL, NULL);
    ritem->setDraggable(FALSE);
    ritem->state = LogItem::FOLDER|LogItem::HASITEMS;
    m_treeHistory->appendItem(NULL, ritem, TRUE);
    getApp()->beginWaitCursor();
    listChildItems(ritem);
    m_treeHistory->sortChildItems(ritem);
    m_treeHistory->expandTree(ritem, TRUE);
    getApp()->endWaitCursor();
}

// modified FXDirList::listChildItems(FXDirItem *par)
void LogViewer::listChildItems(LogItem *par)
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
    directory = getItemPathname(par);

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
            if (!info.isDirectory() && !isRightFile(pathname,name))continue;

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
            item = new LogItem(name, ICO_OPENFOLDER, ICO_CLOSEFOLDER, NULL);
            m_treeHistory->appendItem(par, item, TRUE);

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
                openicon = ICO_OPENFOLDER;
                closedicon = ICO_CLOSEFOLDER;
            }
            else
            {
                openicon = ICO_FILE;
                closedicon = ICO_FILE;
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
        m_treeHistory->removeItem(item, TRUE);
    }

    // Now we know for sure whether we really have subitems or not
    if (par->first)
        par->state |= FXDirItem::HASITEMS;
    else
        par->state &= ~FXDirItem::HASITEMS;

    // Remember new list
    par->list = newlist;

    // Need to layout
    m_treeHistory->recalc();
}

FXbool LogViewer::isRightFile(const FXString& path, const FXString& name)
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
void LogViewer::scan()
{
    FXString pathname;
    LogItem *item;
    FXStat info;
    if(!m_treeHistory->getFirstItem())
        loadTree();
    item = (LogItem*)m_treeHistory->getFirstItem();
    getApp()->beginWaitCursor();
    while(item)
    {
        if(item->isDirectory() /*&& item->isExpanded()*/)
        {
          // Get the full path of the item
          pathname=getItemPathname(item);
          // Stat this directory; should not fail as parent has been scanned already
          FXStat::statFile(pathname,info);
          // Get the mod date of the item
          FXTime newdate=info.touched();
          // date was changed
          if(item->date!=newdate || (!item->getNumChildren() && item->isExpanded()))
          {
              // And do the refresh
              listChildItems(item);
              m_treeHistory->sortChildItems(item);
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


