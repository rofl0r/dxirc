/*
 *      utils.cpp
 *
 *      Copyright 2008 David Vachulka <david@konstrukce-cad.com>
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

#include "utils.h"

FXTextCodec *lcodec = NULL;
FXString iniFile = FXString::null;
dxStringMap aliases;
dxStringArray commands;



namespace utils
{

    void FillCommands()
    {
        commands.clear();        
        commands.append("ADMIN");
        commands.append("AWAY");
        commands.append("BANLIST");
        commands.append("CONNECT");
        commands.append("COMMANDS");
        commands.append("CTCP");
        commands.append("DEOP");
        commands.append("DEVOICE");
#ifndef WIN32
        commands.append("EXEC");
#endif
        commands.append("INVITE");
        commands.append("JOIN");        
        commands.append("KICK");
        commands.append("KILL");
        commands.append("LIST");
        commands.append("ME");
        commands.append("MODE");
        commands.append("MSG");
        commands.append("NAMES");
        commands.append("NICK");
        commands.append("NOTICE");
        commands.append("OP");
        commands.append("OPER");
        commands.append("PART");
        commands.append("QUERY");
        commands.append("QUIT");
        commands.append("QUOTE");
        commands.append("SAY");
        commands.append("TOPIC");
        commands.append("VOICE");
        commands.append("WALLOPS");
        commands.append("WHO");
        commands.append("WHOIS");
        commands.append("WHOWAS");
        StringIt it;
        for(it=aliases.begin(); it!=aliases.end(); it++)
        {
            commands.append((*it).first.after('/'));
        }
        register FXString v;
        register FXint i,j,h;
        for(h=1; h<=commands.no()/9; h=3*h+1);
        for (; h > 0; h /= 3)
        {
            for (i = h + 1; i <= commands.no(); i++)
            {
                v = commands[i - 1];
                j = i;
                while (j > h && comparecase(commands[j - h - 1], v) > 0)
                {
                    commands[j - 1] = commands[j - h - 1];
                    j -= h;
                }
                commands[j - 1] = v;
            }
        }
    }
    
    FXTextCodec* GetCodec()
    {
        if(lcodec != NULL)
            return lcodec;
        else
        {
#ifdef WIN32
            UINT codepage = ::GetACP();
            switch(codepage)
            {
                case 1250:
                {
                    lcodec = new FXCP1250Codec();
                    return lcodec;
                }
                case 1251:
                {
                    lcodec = new FXCP1251Codec();
                    return lcodec;
                }
                case 1252:
                {
                    lcodec = new FXCP1252Codec();
                    return lcodec;
                }
                case 1253:
                {
                    lcodec = new FXCP1253Codec();
                    return lcodec;
                }
                case 1254:
                {
                    lcodec = new FXCP1254Codec();
                    return lcodec;
                }
                case 1255:
                {
                    lcodec = new FXCP1255Codec();
                    return lcodec;
                }
                case 1256:
                {
                    lcodec = new FXCP1256Codec();
                    return lcodec;
                }
                case 1257:
                {
                    lcodec = new FXCP1257Codec();
                    return lcodec;
                }
                case 1258:
                {
                    lcodec = new FXCP1258Codec();
                    return lcodec;
                }
                case 437:
                {
                    lcodec = new FXCP437Codec();
                    return lcodec;
                }
                case 850:
                {
                    lcodec = new FXCP850Codec();
                    return lcodec;
                }
                case 852:
                {
                    lcodec = new FXCP852Codec();
                    return lcodec;
                }
                case 855:
                {
                    lcodec = new FXCP855Codec();
                    return lcodec;
                }
                case 856:
                {
                    lcodec = new FXCP856Codec();
                    return lcodec;
                }
                case 857:
                {
                    lcodec = new FXCP857Codec();
                    return lcodec;
                }
                case 860:
                {
                    lcodec = new FXCP860Codec();
                    return lcodec;
                }
                case 861:
                {
                    lcodec = new FXCP861Codec();
                    return lcodec;
                }
                case 862:
                {
                    lcodec = new FXCP862Codec();
                    return lcodec;
                }
                case 863:
                {
                    lcodec = new FXCP863Codec();
                    return lcodec;
                }
                case 864:
                {
                    lcodec = new FXCP864Codec();
                    return lcodec;
                }
                case 865:
                {
                    lcodec = new FXCP865Codec();
                    return lcodec;
                }
                case 866:
                {
                    lcodec = new FXCP866Codec();
                    return lcodec;
                }
                case 869:
                {
                    lcodec = new FXCP869Codec();
                    return lcodec;
                }
                case 874:
                {
                    lcodec = new FXCP874Codec();
                    return lcodec;
                }
                case 20866:
                {
                    lcodec = new FXKOI8RCodec();
                    return lcodec;
                }
                default:
                {
                    lcodec = new FXCP1252Codec();
                    return lcodec;
                }
            }
#else
            FXString locale, language, territory, codeset, modifier;
            if(!(FXSystem::getEnvironment("LANG")).empty()) locale = FXSystem::getEnvironment("LANG");
            else if(!(FXSystem::getEnvironment("LC_ALL")).empty()) locale = FXSystem::getEnvironment("LC_ALL");
            else if(!(FXSystem::getEnvironment("LC_MESSAGES")).empty()) locale = FXSystem::getEnvironment("LC_MESSAGES");
            else locale = "en_US";
            if(locale == "C" || locale == "POSIX")
            {
                locale = "en_US";
            }
            locale.lower();
            if(locale.contains("utf")) locale = locale.before('.');
            language = locale.before('_');
            territory = locale.after('_').before('.').before('@');
            codeset = locale.after('.');
            modifier = locale.after('@');

            if(language == "af")
            {
                lcodec = new FX88591Codec();
                return lcodec;
            }
            if(language == "ar")
            {
                lcodec = new FX88596Codec();
                return lcodec;
            }
            if(language == "az")
            {
                lcodec = new FX88599Codec();
                return lcodec;
            }
            if(language == "be")
            {
                lcodec = new FXCP1251Codec();
                return lcodec;
            }
            if(language == "bg")
            {
                if(codeset=="iso88595") lcodec = new FX88595Codec();
                else if(codeset=="koi8r") lcodec = new FXKOI8RCodec();
                else lcodec = new FXCP1251Codec();
                return lcodec;
            }
            if(language == "br")
            {
                if(modifier=="euro" || codeset=="iso885915") lcodec = new FX885915Codec();
                else if(codeset=="iso885914") lcodec = new FX885914Codec();
                else lcodec = new FX88591Codec();
                return lcodec;
            }
            if(language == "ca")
            {
                if(modifier=="euro" || codeset=="iso885915") lcodec = new FX885915Codec();
                else lcodec = new FX88591Codec();
                return lcodec;
            }
            if(language == "cs")
            {
                lcodec = new FX88592Codec();
                return lcodec;
            }
            if(language == "cy")
            {
                if(modifier=="euro" || codeset=="iso885915") lcodec = new FX885915Codec();
                else if(codeset=="iso885914") lcodec = new FX885914Codec();
                else lcodec = new FX88591Codec();
                return lcodec;
            }
            if(language == "da")
            {
                if(modifier=="euro" || codeset=="iso885915") lcodec = new FX885915Codec();
                else lcodec = new FX88591Codec();
                return lcodec;
            }
            if(language == "de")
            {
                if(modifier=="euro" || codeset=="iso885915") lcodec = new FX885915Codec();
                else lcodec = new FX88591Codec();
                return lcodec;
            }
            if(language == "ee")
            {
                lcodec = new FX88594Codec();
                return lcodec;
            }
            if(language == "el")
            {
                if(modifier=="euro") lcodec = new FX885915Codec();
                else lcodec = new FX88597Codec();
                return lcodec;
            }
            if(language == "en")
            {
                if(modifier=="euro" || codeset=="iso885915") lcodec = new FX885915Codec();
                else lcodec = new FX88591Codec();
                return lcodec;
            }
            if(language == "eo")
            {
                lcodec = new FX88593Codec();
                return lcodec;
            }
            if(language == "es")
            {
                if(modifier=="euro" || codeset=="iso885915") lcodec = new FX885915Codec();
                else lcodec = new FX88591Codec();
                return lcodec;
            }
            if(language == "et")
            {
                if(codeset=="iso88591") lcodec = new FX88591Codec();
                else if(codeset=="iso885913") lcodec = new FX885913Codec();
                else if(codeset=="iso88594") lcodec = new FX88594Codec();
                else lcodec = new FX885915Codec();
                return lcodec;
            }
            if(language == "eu")
            {
                if(modifier=="euro" || codeset=="iso885915") lcodec = new FX885915Codec();
                else lcodec = new FX88591Codec();
                return lcodec;
            }
            if(language == "fi")
            {
                if(codeset=="88591" || codeset=="iso88591") lcodec = new FX88591Codec();
                else lcodec = new FX885915Codec();
                return lcodec;
            }
            if(language == "fo")
            {
                if(modifier=="euro" || codeset=="iso885915") lcodec = new FX885915Codec();
                else lcodec = new FX88591Codec();
                return lcodec;
            }
            if(language == "fr")
            {
                if(modifier=="euro" || codeset=="iso885915") lcodec = new FX885915Codec();
                else lcodec = new FX88591Codec();
                return lcodec;
            }
            if(language == "fre")
            {
                lcodec = new FX88591Codec();
                return lcodec;
            }
            if(language == "ga")
            {
                if(modifier=="euro" || codeset=="iso885915") lcodec = new FX885915Codec();
                else if(codeset=="iso885914") lcodec = new FX885914Codec();
                else lcodec = new FX88591Codec();
                return lcodec;
            }
            if(language == "gd")
            {
                if(modifier=="euro" || codeset=="iso885915") lcodec = new FX885915Codec();
                else if(codeset=="iso885914") lcodec = new FX885914Codec();
                else lcodec = new FX88591Codec();
                return lcodec;
            }
            if(language == "ger")
            {
                lcodec = new FX88591Codec();
                return lcodec;
            }
            if(language == "gl")
            {
                if(modifier=="euro" || codeset=="iso885915") lcodec = new FX885915Codec();
                else lcodec = new FX88591Codec();
                return lcodec;
            }
            if(language == "gv")
            {
                if(modifier=="euro" || codeset=="iso885915") lcodec = new FX885915Codec();
                else if(codeset=="iso885914") lcodec = new FX885914Codec();
                else lcodec = new FX88591Codec();
                return lcodec;
            }
            if(language == "he")
            {
                if(codeset=="cp1255" || codeset=="microsoftcp1255") lcodec = new FXCP1255Codec();
                else lcodec = new FX88598Codec();
                return lcodec;
            }
            if(language == "hr")
            {
                lcodec = new FX88592Codec();
                return lcodec;
            }
            if(language == "hu")
            {
                lcodec = new FX88592Codec();
                return lcodec;
            }
            if(language == "id")
            {
                lcodec = new FX88591Codec();
                return lcodec;
            }
            if(language == "in")
            {
                lcodec = new FX88591Codec();
                return lcodec;
            }
            if(language == "is")
            {
                if(modifier=="euro" || codeset=="iso885915") lcodec = new FX885915Codec();
                else lcodec = new FX88591Codec();
                return lcodec;
            }
            if(language == "it")
            {
                if(modifier=="euro" || codeset=="iso885915") lcodec = new FX885915Codec();
                else lcodec = new FX88591Codec();
                return lcodec;
            }
            if(language == "iw")
            {
                lcodec = new FX88598Codec();
                return lcodec;
            }
            if(language == "kl")
            {
                if(modifier=="euro" || codeset=="iso885915") lcodec = new FX885915Codec();
                else lcodec = new FX88591Codec();
                return lcodec;
            }
            if(language == "kw")
            {
                if(modifier=="euro" || codeset=="iso885915") lcodec = new FX885915Codec();
                else if(codeset=="iso885914") lcodec = new FX885914Codec();
                else lcodec = new FX88591Codec();
                return lcodec;
            }
            if(language == "lt")
            {
                if(codeset=="iso88594") lcodec = new FX88594Codec();
                else lcodec = new FX885913Codec();
                return lcodec;
            }
            if(language == "lv")
            {
                if(codeset=="iso88594") lcodec = new FX88594Codec();
                else lcodec = new FX885913Codec();
                return lcodec;
            }
            if(language == "mi")
            {
                lcodec = new FX88591Codec();
                return lcodec;
            }
            if(language == "mk")
            {
                if(codeset=="cp1251" || codeset=="microsoftc1251") lcodec = new FXCP1251Codec();
                else lcodec = new FX88595Codec();
                return lcodec;
            }
            if(language == "ms")
            {
                lcodec = new FX88591Codec();
                return lcodec;
            }
            if(language == "mt")
            {
                lcodec = new FX88593Codec();
                return lcodec;
            }
            if(language == "nb")
            {
                if(modifier=="euro" || codeset=="iso885915") lcodec = new FX885915Codec();
                else lcodec = new FX88591Codec();
                return lcodec;
            }
            if(language == "nl")
            {
                if(modifier=="euro" || codeset=="iso885915") lcodec = new FX885915Codec();
                else lcodec = new FX88591Codec();
                return lcodec;
            }
            if(language == "nn")
            {
                if(modifier=="euro" || codeset=="iso885915") lcodec = new FX885915Codec();
                else lcodec = new FX88591Codec();
                return lcodec;
            }
            if(language == "no")
            {
                if(modifier=="euro" || codeset=="iso885915") lcodec = new FX885915Codec();
                else lcodec = new FX88591Codec();
                return lcodec;
            }
            if(language == "ny")
            {
                if(modifier=="euro" || codeset=="iso885915") lcodec = new FX885915Codec();
                else lcodec = new FX88591Codec();
                return lcodec;
            }
            if(language == "oc")
            {
                if(modifier=="euro" || codeset=="iso885915") lcodec = new FX885915Codec();
                else lcodec = new FX88591Codec();
                return lcodec;
            }
            if(language == "pd")
            {
                if(modifier=="euro" || codeset=="iso885915") lcodec = new FX885915Codec();
                else lcodec = new FX88591Codec();
                return lcodec;
            }
            if(language == "ph")
            {
                lcodec = new FX88591Codec();
                return lcodec;
            }
            if(language == "pl")
            {
                lcodec = new FX88592Codec();
                return lcodec;
            }
            if(language == "pp")
            {
                lcodec = new FX88591Codec;
                return lcodec;
            }
            if(language == "pt")
            {
                if(modifier=="euro" || codeset=="iso885915") lcodec = new FX885915Codec();
                else lcodec = new FX88591Codec();
                return lcodec;
            }
            if(language == "ro")
            {
                lcodec = new FX88592Codec();
                return lcodec;
            }
            if(language == "ru")
            {
                if(codeset=="koi8r" || territory=="ua") lcodec = new FXKOI8RCodec();
                else if(codeset=="cp1251" || codeset=="microsoftcp1251") lcodec = new FXCP1251Codec();
                else lcodec = new FX88595Codec();
                return lcodec;
            }
            if(language == "sh")
            {
                lcodec = new FX88592Codec();
                return lcodec;
            }
            if(language == "sk")
            {
                lcodec = new FX88592Codec();
                return lcodec;
            }
            if(language == "sl")
            {
                lcodec = new FX88592Codec();
                return lcodec;
            }
            if(language == "sp")
            {
                lcodec = new FX88595Codec();
                return lcodec;
            }
            if(language == "sq")
            {
                lcodec = new FX88592Codec();
                return lcodec;
            }
            if(language == "sr")
            {
                if(codeset=="iso88592" || territory=="sp") lcodec = new FX88592Codec();
                else if(codeset=="cp1251" || codeset=="microsoftcp1251") lcodec = new FXCP1251Codec();
                else lcodec = new FX88595Codec();
                return lcodec;
            }
            if(language == "sv")
            {
                if(modifier=="euro" || codeset=="iso885915") lcodec = new FX885915Codec();
                else lcodec = new FX88591Codec();
                return lcodec;
            }
            if(language == "th")
            {
                lcodec = new FX885911Codec();
                return lcodec;
            }
            if(language == "tl")
            {
                lcodec = new FX88591Codec();
                return lcodec;
            }
            if(language == "tr")
            {
                lcodec = new FX88599Codec();
                return lcodec;
            }
            if(language == "tt")
            {
                lcodec = new FXKOI8RCodec();
                return lcodec;
            }
            if(language == "uk")
            {
                if(codeset=="cp1251" || codeset=="microsoftcp1251") lcodec = new FXCP1251Codec();
                else if(codeset=="koi8u") new FX885915Codec(); // FXKOI8UCodec doesn't exist
                else lcodec = new FXKOI8RCodec();
                return lcodec;
            }
            if(language == "ur")
            {
                lcodec = new FXCP1256Codec();
                return lcodec;
            }
            if(language == "wa")
            {
                if(modifier=="euro" || codeset=="iso885915") lcodec = new FX885915Codec();
                else lcodec = new FX88591Codec();
                return lcodec;
            }
            if(language == "yi")
            {
                lcodec = new FXCP1255Codec();
                return lcodec;
            }
            lcodec = new FX885915Codec();
            return lcodec;
#endif
        }
        return lcodec;
    }

    FXString LocaleToUtf8(const FXchar *buffer)
    {
        return GetCodec()->mb2utf(buffer);
    }

    FXString LocaleToUtf8(const FXString& text)
    {
        return GetCodec()->mb2utf(text);
    }

    void SetIniFile(const FXString &file)
    {
        iniFile = file;
    }

    FXString GetIniFile()
    {
        FXString pathname;
        if(!iniFile.empty() && !FXStat::isExecutable(iniFile))
        {
            pathname = FXPath::directory(iniFile);
            if(pathname.empty()) pathname = FXSystem::getCurrentDirectory();
            if(!FXStat::exists(pathname))
            {
                if(!FXDir::create(pathname))
                {
                    fxwarning("%s: unable to create directory.\n",pathname.text());
                }
            }
            else
            {
                if(!FXStat::isDirectory(pathname))
                {
                    fxwarning("%s: is not a directory.\n",pathname.text());
                }
            }
            return iniFile;
        }
        else
        {
#ifdef WIN32
            pathname = LocaleToUtf8(FXSystem::getEnvironment("AppData")+PATHSEPSTRING+"dxirc");
            if(!FXStat::exists(pathname))
            {
                if(!FXDir::create(pathname))
                {
                    fxwarning("%s: unable to create directory.\n",pathname.text());
                }
            }
            else
            {
                if(!FXStat::isDirectory(pathname))
                {
                    fxwarning("%s: is not a directory.\n",pathname.text());
                }
            }
            iniFile = pathname.append(PATHSEPSTRING "dxirc.ini");
            return iniFile;
#else
            pathname = FXSystem::getHomeDirectory()+PATHSEPSTRING+".dxirc";
            if(!FXStat::exists(pathname))
            {
                if(!FXDir::create(pathname))
                {
                    fxwarning("%s: unable to create directory.\n",pathname.text());
                }
            }
            else
            {
                if(!FXStat::isDirectory(pathname))
                {
                    fxwarning("%s: is not a directory.\n",pathname.text());
                }
            }
            iniFile = pathname.append(PATHSEPSTRING "dxirc");
            return iniFile;
#endif
        }
        return FXString::null;
    }

    FXString GetParam(FXString toParse, FXint n, FXbool toEnd)
    {
        if (toEnd)
        {
            return toParse.after(' ', n-1);
        }
        else
        {
            return toParse.before(' ', n).rafter(' ');
        }
    }

    FXString GetParam(FXString toParse, FXint n, FXbool toEnd, const FXchar &separator)
    {
        if (toEnd)
        {
            return toParse.after(separator, n-1);
        }
        else
        {
            return toParse.before(separator, n).rafter(separator);
        }
    }

    FXString RemoveSpaces(const FXString &text)
    {
        FXint num = text.contains(' ');
        FXString removed;
        if(num)
        {
            for(FXint i=0; i<=num; i++)
            {
                removed.append(text.section(' ',i));
            }
            return removed;
        }
        else
            return text;
    }

    FXString CreateModes(FXchar sign, FXchar mode, FXString nicks)
    {
        FXString modes;
        modes += sign;
        FXString tomode;
        if(!nicks.contains(' ')) return modes+mode+" "+nicks;
        if(nicks.right(1) != " ") nicks.append(" ");
        while(nicks.contains(' '))
        {
            modes += mode;
            tomode += nicks.before(' ')+" ";
            nicks = nicks.after(' ');
        }
        return modes+" "+tomode;
    }

    void SetAlias()
    {
        FXSettings set;
        set.parseFile(GetIniFile(), true);
        FXint no = set.readIntEntry("ALIASES", "number", 0);
        for(FXint i=0; i<no; i++)
        {
            FXString key, value;
            key = set.readStringEntry("ALIASES", FXStringFormat("key%d", i).text());
            value = set.readStringEntry("ALIASES", FXStringFormat("value%d", i).text());
            if(!key.empty() && !value.empty() && key[0]=='/' && !key.contains(' '))
                aliases.insert(StringPair(key, value));
        }
        FillCommands();

    }

    void SetAliases(dxStringMap a)
    {
        aliases = a;
        FillCommands();
    }

    FXString GetAlias(FXString key)
    {
        if(aliases.count(key.lower())>0)
            return aliases.find(key)->second;
        return FXString::null;
    }

    dxStringMap GetAliases()
    {
        return aliases;
    }

    FXint CommandsNo()
    {
        return commands.no();
    }

    FXString CommandsAt(FXint i)
    {
        return commands.at(i);
    }

    FXbool IsCommand(const FXString &command)
    {
        for(FXint i=0; i<commands.no(); i++)
        {
            if(comparecase(command, commands.at(i)) == 0) return true;
        }
        return false;
    }

    //This's from Xfe, thanks
    int Streq(const FXchar *a, const FXchar *b)
    {
        if (a == NULL || b == NULL)
            return 0;
        return (strcmp(a, b) == 0);
    }

    //This's from Xfe, thanks
    FXbool IsUtf8(const FXchar* string, FXuint length)
    {
        FXchar s[4];
        const FXchar BOM[] = { 0xEF, 0xBB, 0xBF, '\0' };

        // Keep only length left bytes
        FXString str=string;
        str=str.left(length);

        // Convert forth and back to UTF8
        FXUTF8Codec utf8;
        FXString utf8str=utf8.mb2utf(utf8.utf2mb(str));

        // Strings are equal => UTF8
        if (str==utf8str)
            return TRUE;

        // Strings not equal => test if BOM is present
        else
        {
            // String too small to contain BOM
            if (length<=2)
                return FALSE;

            // Test if string contains BOM
            else
            {
                s[0]=string[0];
                s[1]=string[1];
                s[2]=string[2];
                s[3]='\0';

                // String contains BOM => UTF8
                if (Streq(s,BOM))
                    return TRUE;

                // String don't contain BOM
                else
                    return FALSE;
            }
        }
    }
}
