/*
 *      aboutdialog.h
 *
 *      Copyright 2010 David Vachulka <david@konstrukce-cad.com>
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

#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

class AboutDialog: public FXDialogBox
{
    FXDECLARE(AboutDialog)
public:
    AboutDialog(FXMainWindow *owner);
    virtual ~AboutDialog();

    virtual void create();

    long onKeyPress(FXObject*,FXSelector,void*);

private:
    AboutDialog() {}
    AboutDialog(const AboutDialog& orig);

    FXVerticalFrame *m_contents;
    FXHorizontalFrame *m_buttonframe;
    FXButton *m_buttonClose;
    FXFont *m_boldfont, *m_smallfont;
};

#endif  /* ABOUTDIALOG_H */

