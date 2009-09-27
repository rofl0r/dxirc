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
    o Log viewer - Ctrl-g\n \
    o Close current tab - Ctrl-w\n \
    o Clear window - Ctrl-l\n \
    o Clear all windows - Ctrl-shift-l\n \
    o Show/Hide users list - Ctrl-u\n \
    o Switch between tabs - Alt-1~9 or Ctrl-tab\n \
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

#define LUA_TEXT _("\
Those irc event are sent to Lua script:\n\
- PRIVMSG, which you can handle by OnMessage(msg)\n\
  msg is table with fields:\n\
  from ... sender of message\n\
  target ... target of message\n\
  text ... text of message\n\
  nick ... your nick, for identify server\n\
  server ... server name, for identify server\n\
- JOIN, which you can handle by OnJoin(msg)\n\
  msg is table with fields:\n\
  jnick ... who is joined\n\
  target ... where is joined\n\
  nick ... your nick, for identify server\n\
  server ... server name, for identify server\n\
Those functions dxirc hadle:\n\
- ProcessCommand(command, text, target, nick, server)\n\
  command can be empty or one from /commands\n\
  target ... target of command (channel or query), if target is EMPTY, command is processed by all tabs\n\
  nick ... for identify server\n\
  server ... for identify server\n\
- GetInfo(request)\n\
  request:\n\
  - server ... return info about current server\n\
  answer can handle by OnInfo(info)\n\
  info is table with fields:\n\
  - for server: name, port, nick\
")
;



#endif
