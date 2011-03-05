/*
 *      aboutdialog.cpp
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

#include <fx.h>
#include <fxkeys.h>
#include "aboutdialog.h"
#include "config.h"
#include "i18n.h"
#include "icons.h"
#ifdef HAVE_LUA
extern "C" {
#include <lua.h>
}
#endif
#ifdef HAVE_OPENSSL
#include <openssl/ssl.h>
#endif

FXDEFMAP(AboutDialog) AboutDialogMap[] = {
    FXMAPFUNC(SEL_KEYPRESS, 0, AboutDialog::onKeyPress)
};

FXIMPLEMENT(AboutDialog, FXDialogBox, AboutDialogMap, ARRAYNUMBER(AboutDialogMap))

AboutDialog::AboutDialog(FXMainWindow *owner)
        : FXDialogBox(owner, FXStringFormat(_("About %s"), PACKAGE), DECOR_TITLE|DECOR_BORDER|DECOR_RESIZE, 0,0,0,0, 0,0,0,0, 0,0)
{
    m_contents = new FXVerticalFrame(this, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    FXFontDesc fontdescription;
    getApp()->getNormalFont()->getFontDesc(fontdescription);
    fontdescription.size  += 10;
    fontdescription.weight = FXFont::Bold;
    m_boldfont = new FXFont(getApp(),fontdescription);
    m_boldfont->create();
    getApp()->getNormalFont()->getFontDesc(fontdescription);
    fontdescription.size  -= 10;
    m_smallfont = new FXFont(getApp(),fontdescription);
    m_smallfont->create();
    new FXLabel(m_contents, "", ICO_BIG, ICON_BELOW_TEXT|LAYOUT_CENTER_X|LAYOUT_FILL_Y);
    FXLabel *label = new FXLabel(m_contents, (FXString)"\n"+PACKAGE+" "+VERSION, 0, LAYOUT_CENTER_X|LAYOUT_FILL_Y);
    label->setFont(m_boldfont);
    new FXLabel(m_contents, (FXString)"\n"+_("Copyright (C) 2008~ David Vachulka (david@konstrukce-cad.com)\nand others")+"\n", 0, JUSTIFY_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    label = new FXLabel(m_contents, FXStringFormat(_("Built with the FOX Toolkit Library version %d.%d.%d (http://www.fox-toolkit.org)"),FOX_MAJOR,FOX_MINOR,FOX_LEVEL), 0, JUSTIFY_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    label->setFont(m_smallfont);
#ifdef HAVE_OPENSSL
    label = new FXLabel(m_contents, FXStringFormat(_("Built with %s"), OPENSSL_VERSION_TEXT), 0, JUSTIFY_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    label->setFont(m_smallfont);
#endif
#ifdef HAVE_LUA
    label = new FXLabel(m_contents, FXStringFormat(_("Built with %s"), LUA_RELEASE), 0, JUSTIFY_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    label->setFont(m_smallfont);
#endif
    label = new FXLabel(m_contents, _("Uses http://www.famfamfam.com/lab/icons/"), 0, JUSTIFY_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    label->setFont(m_smallfont);
    m_buttonframe = new FXHorizontalFrame(m_contents, LAYOUT_FILL_X);
    m_buttonClose  = new dxEXButton(m_contents, _("C&lose"), 0, this, FXDialogBox::ID_ACCEPT, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X, 0,0,0,0, 10,10,2,5);
}

AboutDialog::~AboutDialog()
{
    delete m_boldfont;
    delete m_smallfont;
}

void AboutDialog::create()
{
    FXDialogBox::create();
}

long AboutDialog::onKeyPress(FXObject *sender, FXSelector sel, void *ptr)
{
    if(FXTopWindow::onKeyPress(sender,sel,ptr)) return 1;
    if(((FXEvent*)ptr)->code == KEY_Escape)
    {
        handle(this,FXSEL(SEL_COMMAND,ID_CANCEL),NULL);
        return 1;
    }
    return 0;
}

