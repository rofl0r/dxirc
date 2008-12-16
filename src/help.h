#ifndef HELP_H
#define HELP_H

#include "config.h"
#include "i18n.h"

#define HELP_TEXT _("\n \
\n \
  dxirc, another IRC client\n \
\n \
  Copyright (C) 2008~ David Vachulka\n \
\n \
  <http://dxirc.org>\n \
\n \
\n \
\n \
\n \
This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.\n \
\n \
\n \
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.\n \
\n \
\n \
  Description\n \
  =-=-=-=-=-=\n \
\n \
dxirc is a simple IRC client for various os (linux, *bsd, windows), written using the FOX toolkit.\n \
\n \
\n \
  Key bindings:\n \
  =-=-=-=-=-=\n \
\n \
    o Help - F1\n \
    o Server list - F2\n \
    o Quick connect - Ctrl-k\n \
    o Disconnect - Ctrl-d\n \
    o Close current tab - Ctrl-w\n \
    o Clear window - Ctrl-l\n \
    o Clear all windows - Ctrl-shift-l\n \
    o Show/Hide users list - Ctrl-u\n \
    o Switch between tabs - Alt-0~9 or Ctrl-tab\n \
    o Switch between unreaded tabs - Ctrl-n\n\
    o Quit - Alt-F4\n \
\n \
\n \
  Configuration\n \
  =-=-=-=-=-=-=\n \
\n \
You can perform any dxirc customization (ignoring, colors, ...) without editing any file by hand. However, you may want to understand the configuration principles, because some customizations can easily be done by editing the configurations files.\n \
Be careful to quit dxirc before hand editing any configuration file, otherwise changes could not be taken into account.\n \
\n \
\n \
  Bugs\n \
  =-=-=\n \
\n \
Please report bugs to David Vachulka <david@konstrukce-cad.com>.\n \
\n \
\n \
\
  Translations\n \
  =-=-=-=-=-=-=\n \
\n \
dxirc is now available in several languages. To translate dxirc to your language, open with a software like poedit, kbabel or gtranslator the dxirc.pot file in the po directory of the source tree and fill it with your translated strings, and then send it back to me.\n \
\n \
\n \
\
  Patches\n \
  =-=-=-=\n \
\n \
If you have done some interesting patch, please send it to me, I will try to include it in the next release...\n \
\n \
[Many thanks to Jeroen van der Zijp for his excellent FOX-tookit and to all people that have provided useful patches, \
translations, tests, advices and also all users.]\n \
\n \
")
;


#endif
