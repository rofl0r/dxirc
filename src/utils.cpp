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
#ifdef HAVE_ENCHANT
#include <enchant++.h>
#endif

#ifdef HAVE_ENCHANT
typedef std::pair<Checkers*,dxStringArray*> CheckerPair;
void enumerateDicts (const char * const lang_tag,
         const char * const /*provider_name*/,
         const char * const /*provider_desc*/,
         const char * const /*provider_file*/,
         void * user_data)
{
    CheckerPair *chpair = (CheckerPair*)user_data;
    const Checkers &checkers = *chpair->first;
    dxStringArray &spellLang = *(chpair->second);
    if(checkers.find(lang_tag) == checkers.end())
        spellLang.append(lang_tag);
}
#endif

//This's from Xfe, thanks
int streq(const FXchar *a, const FXchar *b)
{
    if (a == NULL || b == NULL)
        return 0;
    return (strcmp(a, b) == 0);
}

utils::utils()
{
    m_lcodec = NULL;
    m_iniFile = FXString::null;
    m_locale = FXString::null;
}

utils::~utils()
{
#ifdef HAVE_ENCHANT
    Checkers::const_iterator it = m_checkers.begin();
    for(; it != m_checkers.end(); it++)
    {
        delete (*it).second;
    }
#endif
}

utils &utils::instance()
{
    static utils iutils;
    return iutils;
}

void utils::fillCommands()
{
    m_commands.clear();
    m_commands.append("ADMIN");
    m_commands.append("AWAY");
    m_commands.append("BANLIST");
    m_commands.append("CONNECT");
    m_commands.append("COMMANDS");
    m_commands.append("CTCP");
    m_commands.append("CYCLE");
    m_commands.append("DCC");
    m_commands.append("DEOP");
    m_commands.append("DEVOICE");
    m_commands.append("DISCONNECT");
#ifndef WIN32
    m_commands.append("EXEC");
#endif
    m_commands.append("HELP");
    m_commands.append("IGNORE");
    m_commands.append("INVITE");
    m_commands.append("JOIN");
    m_commands.append("KICK");
    m_commands.append("KILL");
    m_commands.append("LIST");
#ifdef HAVE_LUA
    m_commands.append("LUA");
#endif
    m_commands.append("ME");
    m_commands.append("MODE");
    m_commands.append("MSG");
    m_commands.append("NAMES");
    m_commands.append("NICK");
    m_commands.append("NOTICE");
    m_commands.append("OP");
    m_commands.append("OPER");
    m_commands.append("PART");
    m_commands.append("QUERY");
    m_commands.append("QUIT");
    m_commands.append("QUOTE");
    m_commands.append("SAY");
    m_commands.append("STATS");
    m_commands.append("TIME");
    m_commands.append("TOPIC");
    m_commands.append("VOICE");
    m_commands.append("WALLOPS");
    m_commands.append("WHO");
    m_commands.append("WHOAMI");
    m_commands.append("WHOIS");
    m_commands.append("WHOWAS");
    StringIt it;
    for(it=m_aliases.begin(); it!=m_aliases.end(); it++)
    {
        m_commands.append((*it).first.after('/'));
    }
    for(FXint i=0; i<m_scriptCommands.no(); i++)
    {
        m_commands.append(m_scriptCommands[i].name);
    }
    FXString v;
    register FXint i,j,h;
    for(h=1; h<=m_commands.no()/9; h=3*h+1);
    for(; h > 0; h /= 3)
    {
        for(i = h + 1; i <= m_commands.no(); i++)
        {
            v = m_commands[i - 1];
            j = i;
            while (j > h && comparecase(m_commands[j - h - 1], v) > 0)
            {
                m_commands[j - 1] = m_commands[j - h - 1];
                j -= h;
            }
            m_commands[j - 1] = v;
        }
    }
}
    
FXTextCodec* utils::getCodec()
{
    if(m_lcodec != NULL)
        return m_lcodec;
    else
    {
#ifdef WIN32
        UINT codepage = ::GetACP();
        switch(codepage)
        {
            case 1250:
            {
                m_lcodec = new FXCP1250Codec();
                return m_lcodec;
            }
            case 1251:
            {
                m_lcodec = new FXCP1251Codec();
                return m_lcodec;
            }
            case 1252:
            {
                m_lcodec = new FXCP1252Codec();
                return m_lcodec;
            }
            case 1253:
            {
                m_lcodec = new FXCP1253Codec();
                return m_lcodec;
            }
            case 1254:
            {
                m_lcodec = new FXCP1254Codec();
                return m_lcodec;
            }
            case 1255:
            {
                m_lcodec = new FXCP1255Codec();
                return m_lcodec;
            }
            case 1256:
            {
                m_lcodec = new FXCP1256Codec();
                return m_lcodec;
            }
            case 1257:
            {
                m_lcodec = new FXCP1257Codec();
                return m_lcodec;
            }
            case 1258:
            {
                m_lcodec = new FXCP1258Codec();
                return m_lcodec;
            }
            case 437:
            {
                m_lcodec = new FXCP437Codec();
                return m_lcodec;
            }
            case 850:
            {
                m_lcodec = new FXCP850Codec();
                return m_lcodec;
            }
            case 852:
            {
                m_lcodec = new FXCP852Codec();
                return m_lcodec;
            }
            case 855:
            {
                m_lcodec = new FXCP855Codec();
                return m_lcodec;
            }
            case 856:
            {
                m_lcodec = new FXCP856Codec();
                return m_lcodec;
            }
            case 857:
            {
                m_lcodec = new FXCP857Codec();
                return m_lcodec;
            }
            case 860:
            {
                m_lcodec = new FXCP860Codec();
                return m_lcodec;
            }
            case 861:
            {
                m_lcodec = new FXCP861Codec();
                return m_lcodec;
            }
            case 862:
            {
                m_lcodec = new FXCP862Codec();
                return m_lcodec;
            }
            case 863:
            {
                m_lcodec = new FXCP863Codec();
                return m_lcodec;
            }
            case 864:
            {
                m_lcodec = new FXCP864Codec();
                return m_lcodec;
            }
            case 865:
            {
                m_lcodec = new FXCP865Codec();
                return m_lcodec;
            }
            case 866:
            {
                m_lcodec = new FXCP866Codec();
                return m_lcodec;
            }
            case 869:
            {
                m_lcodec = new FXCP869Codec();
                return m_lcodec;
            }
            case 874:
            {
                m_lcodec = new FXCP874Codec();
                return m_lcodec;
            }
            case 20866:
            {
                m_lcodec = new FXKOI8RCodec();
                return m_lcodec;
            }
            default:
            {
                m_lcodec = new FXCP1252Codec();
                return m_lcodec;
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
            m_lcodec = new FX88591Codec();
            return m_lcodec;
        }
        if(language == "ar")
        {
            m_lcodec = new FX88596Codec();
            return m_lcodec;
        }
        if(language == "az")
        {
            m_lcodec = new FX88599Codec();
            return m_lcodec;
        }
        if(language == "be")
        {
            m_lcodec = new FXCP1251Codec();
            return m_lcodec;
        }
        if(language == "bg")
        {
            if(codeset=="iso88595") m_lcodec = new FX88595Codec();
            else if(codeset=="koi8r") m_lcodec = new FXKOI8RCodec();
            else m_lcodec = new FXCP1251Codec();
            return m_lcodec;
        }
        if(language == "br")
        {
            if(modifier=="euro" || codeset=="iso885915") m_lcodec = new FX885915Codec();
            else if(codeset=="iso885914") m_lcodec = new FX885914Codec();
            else m_lcodec = new FX88591Codec();
            return m_lcodec;
        }
        if(language == "ca")
        {
            if(modifier=="euro" || codeset=="iso885915") m_lcodec = new FX885915Codec();
            else m_lcodec = new FX88591Codec();
            return m_lcodec;
        }
        if(language == "cs")
        {
            m_lcodec = new FX88592Codec();
            return m_lcodec;
        }
        if(language == "cy")
        {
            if(modifier=="euro" || codeset=="iso885915") m_lcodec = new FX885915Codec();
            else if(codeset=="iso885914") m_lcodec = new FX885914Codec();
            else m_lcodec = new FX88591Codec();
            return m_lcodec;
        }
        if(language == "da")
        {
            if(modifier=="euro" || codeset=="iso885915") m_lcodec = new FX885915Codec();
            else m_lcodec = new FX88591Codec();
            return m_lcodec;
        }
        if(language == "de")
        {
            if(modifier=="euro" || codeset=="iso885915") m_lcodec = new FX885915Codec();
            else m_lcodec = new FX88591Codec();
            return m_lcodec;
        }
        if(language == "ee")
        {
            m_lcodec = new FX88594Codec();
            return m_lcodec;
        }
        if(language == "el")
        {
            if(modifier=="euro") m_lcodec = new FX885915Codec();
            else m_lcodec = new FX88597Codec();
            return m_lcodec;
        }
        if(language == "en")
        {
            if(modifier=="euro" || codeset=="iso885915") m_lcodec = new FX885915Codec();
            else m_lcodec = new FX88591Codec();
            return m_lcodec;
        }
        if(language == "eo")
        {
            m_lcodec = new FX88593Codec();
            return m_lcodec;
        }
        if(language == "es")
        {
            if(modifier=="euro" || codeset=="iso885915") m_lcodec = new FX885915Codec();
            else m_lcodec = new FX88591Codec();
            return m_lcodec;
        }
        if(language == "et")
        {
            if(codeset=="iso88591") m_lcodec = new FX88591Codec();
            else if(codeset=="iso885913") m_lcodec = new FX885913Codec();
            else if(codeset=="iso88594") m_lcodec = new FX88594Codec();
            else m_lcodec = new FX885915Codec();
            return m_lcodec;
        }
        if(language == "eu")
        {
            if(modifier=="euro" || codeset=="iso885915") m_lcodec = new FX885915Codec();
            else m_lcodec = new FX88591Codec();
            return m_lcodec;
        }
        if(language == "fi")
        {
            if(codeset=="88591" || codeset=="iso88591") m_lcodec = new FX88591Codec();
            else m_lcodec = new FX885915Codec();
            return m_lcodec;
        }
        if(language == "fo")
        {
            if(modifier=="euro" || codeset=="iso885915") m_lcodec = new FX885915Codec();
            else m_lcodec = new FX88591Codec();
            return m_lcodec;
        }
        if(language == "fr")
        {
            if(modifier=="euro" || codeset=="iso885915") m_lcodec = new FX885915Codec();
            else m_lcodec = new FX88591Codec();
            return m_lcodec;
        }
        if(language == "fre")
        {
            m_lcodec = new FX88591Codec();
            return m_lcodec;
        }
        if(language == "ga")
        {
            if(modifier=="euro" || codeset=="iso885915") m_lcodec = new FX885915Codec();
            else if(codeset=="iso885914") m_lcodec = new FX885914Codec();
            else m_lcodec = new FX88591Codec();
            return m_lcodec;
        }
        if(language == "gd")
        {
            if(modifier=="euro" || codeset=="iso885915") m_lcodec = new FX885915Codec();
            else if(codeset=="iso885914") m_lcodec = new FX885914Codec();
            else m_lcodec = new FX88591Codec();
            return m_lcodec;
        }
        if(language == "ger")
        {
            m_lcodec = new FX88591Codec();
            return m_lcodec;
        }
        if(language == "gl")
        {
            if(modifier=="euro" || codeset=="iso885915") m_lcodec = new FX885915Codec();
            else m_lcodec = new FX88591Codec();
            return m_lcodec;
        }
        if(language == "gv")
        {
            if(modifier=="euro" || codeset=="iso885915") m_lcodec = new FX885915Codec();
            else if(codeset=="iso885914") m_lcodec = new FX885914Codec();
            else m_lcodec = new FX88591Codec();
            return m_lcodec;
        }
        if(language == "he")
        {
            if(codeset=="cp1255" || codeset=="microsoftcp1255") m_lcodec = new FXCP1255Codec();
            else m_lcodec = new FX88598Codec();
            return m_lcodec;
        }
        if(language == "hr")
        {
            m_lcodec = new FX88592Codec();
            return m_lcodec;
        }
        if(language == "hu")
        {
            m_lcodec = new FX88592Codec();
            return m_lcodec;
        }
        if(language == "id")
        {
            m_lcodec = new FX88591Codec();
            return m_lcodec;
        }
        if(language == "in")
        {
            m_lcodec = new FX88591Codec();
            return m_lcodec;
        }
        if(language == "is")
        {
            if(modifier=="euro" || codeset=="iso885915") m_lcodec = new FX885915Codec();
            else m_lcodec = new FX88591Codec();
            return m_lcodec;
        }
        if(language == "it")
        {
            if(modifier=="euro" || codeset=="iso885915") m_lcodec = new FX885915Codec();
            else m_lcodec = new FX88591Codec();
            return m_lcodec;
        }
        if(language == "iw")
        {
            m_lcodec = new FX88598Codec();
            return m_lcodec;
        }
        if(language == "kl")
        {
            if(modifier=="euro" || codeset=="iso885915") m_lcodec = new FX885915Codec();
            else m_lcodec = new FX88591Codec();
            return m_lcodec;
        }
        if(language == "kw")
        {
            if(modifier=="euro" || codeset=="iso885915") m_lcodec = new FX885915Codec();
            else if(codeset=="iso885914") m_lcodec = new FX885914Codec();
            else m_lcodec = new FX88591Codec();
            return m_lcodec;
        }
        if(language == "lt")
        {
            if(codeset=="iso88594") m_lcodec = new FX88594Codec();
            else m_lcodec = new FX885913Codec();
            return m_lcodec;
        }
        if(language == "lv")
        {
            if(codeset=="iso88594") m_lcodec = new FX88594Codec();
            else m_lcodec = new FX885913Codec();
            return m_lcodec;
        }
        if(language == "mi")
        {
            m_lcodec = new FX88591Codec();
            return m_lcodec;
        }
        if(language == "mk")
        {
            if(codeset=="cp1251" || codeset=="microsoftc1251") m_lcodec = new FXCP1251Codec();
            else m_lcodec = new FX88595Codec();
            return m_lcodec;
        }
        if(language == "ms")
        {
            m_lcodec = new FX88591Codec();
            return m_lcodec;
        }
        if(language == "mt")
        {
            m_lcodec = new FX88593Codec();
            return m_lcodec;
        }
        if(language == "nb")
        {
            if(modifier=="euro" || codeset=="iso885915") m_lcodec = new FX885915Codec();
            else m_lcodec = new FX88591Codec();
            return m_lcodec;
        }
        if(language == "nl")
        {
            if(modifier=="euro" || codeset=="iso885915") m_lcodec = new FX885915Codec();
            else m_lcodec = new FX88591Codec();
            return m_lcodec;
        }
        if(language == "nn")
        {
            if(modifier=="euro" || codeset=="iso885915") m_lcodec = new FX885915Codec();
            else m_lcodec = new FX88591Codec();
            return m_lcodec;
        }
        if(language == "no")
        {
            if(modifier=="euro" || codeset=="iso885915") m_lcodec = new FX885915Codec();
            else m_lcodec = new FX88591Codec();
            return m_lcodec;
        }
        if(language == "ny")
        {
            if(modifier=="euro" || codeset=="iso885915") m_lcodec = new FX885915Codec();
            else m_lcodec = new FX88591Codec();
            return m_lcodec;
        }
        if(language == "oc")
        {
            if(modifier=="euro" || codeset=="iso885915") m_lcodec = new FX885915Codec();
            else m_lcodec = new FX88591Codec();
            return m_lcodec;
        }
        if(language == "pd")
        {
            if(modifier=="euro" || codeset=="iso885915") m_lcodec = new FX885915Codec();
            else m_lcodec = new FX88591Codec();
            return m_lcodec;
        }
        if(language == "ph")
        {
            m_lcodec = new FX88591Codec();
            return m_lcodec;
        }
        if(language == "pl")
        {
            m_lcodec = new FX88592Codec();
            return m_lcodec;
        }
        if(language == "pp")
        {
            m_lcodec = new FX88591Codec;
            return m_lcodec;
        }
        if(language == "pt")
        {
            if(modifier=="euro" || codeset=="iso885915") m_lcodec = new FX885915Codec();
            else m_lcodec = new FX88591Codec();
            return m_lcodec;
        }
        if(language == "ro")
        {
            m_lcodec = new FX88592Codec();
            return m_lcodec;
        }
        if(language == "ru")
        {
            if(codeset=="koi8r" || territory=="ua") m_lcodec = new FXKOI8RCodec();
            else if(codeset=="cp1251" || codeset=="microsoftcp1251") m_lcodec = new FXCP1251Codec();
            else m_lcodec = new FX88595Codec();
            return m_lcodec;
        }
        if(language == "sh")
        {
            m_lcodec = new FX88592Codec();
            return m_lcodec;
        }
        if(language == "sk")
        {
            m_lcodec = new FX88592Codec();
            return m_lcodec;
        }
        if(language == "sl")
        {
            m_lcodec = new FX88592Codec();
            return m_lcodec;
        }
        if(language == "sp")
        {
            m_lcodec = new FX88595Codec();
            return m_lcodec;
        }
        if(language == "sq")
        {
            m_lcodec = new FX88592Codec();
            return m_lcodec;
        }
        if(language == "sr")
        {
            if(codeset=="iso88592" || territory=="sp") m_lcodec = new FX88592Codec();
            else if(codeset=="cp1251" || codeset=="microsoftcp1251") m_lcodec = new FXCP1251Codec();
            else m_lcodec = new FX88595Codec();
            return m_lcodec;
        }
        if(language == "sv")
        {
            if(modifier=="euro" || codeset=="iso885915") m_lcodec = new FX885915Codec();
            else m_lcodec = new FX88591Codec();
            return m_lcodec;
        }
        if(language == "th")
        {
            m_lcodec = new FX885911Codec();
            return m_lcodec;
        }
        if(language == "tl")
        {
            m_lcodec = new FX88591Codec();
            return m_lcodec;
        }
        if(language == "tr")
        {
            m_lcodec = new FX88599Codec();
            return m_lcodec;
        }
        if(language == "tt")
        {
            m_lcodec = new FXKOI8RCodec();
            return m_lcodec;
        }
        if(language == "uk")
        {
            if(codeset=="cp1251" || codeset=="microsoftcp1251") m_lcodec = new FXCP1251Codec();
            else if(codeset=="koi8u") new FX885915Codec(); // FXKOI8UCodec doesn't exist
            else m_lcodec = new FXKOI8RCodec();
            return m_lcodec;
        }
        if(language == "ur")
        {
            m_lcodec = new FXCP1256Codec();
            return m_lcodec;
        }
        if(language == "wa")
        {
            if(modifier=="euro" || codeset=="iso885915") m_lcodec = new FX885915Codec();
            else m_lcodec = new FX88591Codec();
            return m_lcodec;
        }
        if(language == "yi")
        {
            m_lcodec = new FXCP1255Codec();
            return m_lcodec;
        }
        m_lcodec = new FX885915Codec();
        return m_lcodec;
#endif
    }
    return m_lcodec;
}

FXString utils::localeToUtf8(const FXchar *buffer)
{
    return getCodec()->mb2utf(buffer);
}

FXString utils::localeToUtf8(const FXString& text)
{
    return getCodec()->mb2utf(text);
}

void utils::setIniFile(const FXString &file)
{
    m_iniFile = file;
}

FXString utils::getIniFile()
{
    FXString pathname;
    if(!m_iniFile.empty() && !FXStat::isExecutable(m_iniFile))
    {
        pathname = FXPath::directory(m_iniFile);
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
        return m_iniFile;
    }
    else
    {
#ifdef WIN32
        pathname = localeToUtf8(FXSystem::getEnvironment("AppData")+PATHSEPSTRING+"dxirc");
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
        m_iniFile = pathname.append(PATHSEPSTRING "dxirc.ini");
        return m_iniFile;
#else
        pathname = FXSystem::getHomeDirectory()+PATHSEPSTRING+".config"+PATHSEPSTRING+"dxirc";
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
        m_iniFile = pathname.append(PATHSEPSTRING "dxirc");
        return m_iniFile;
#endif
    }
    return FXString::null;
}

FXString utils::getParam(FXString toParse, FXint n, FXbool toEnd)
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

FXString utils::getParam(FXString toParse, FXint n, FXbool toEnd, const FXchar &separator)
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

FXString utils::removeSpaces(const FXString &text)
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

FXString utils::removeNonalphanumeric(const FXString &text)
{
    if(FXRex("^[a-zA-Z0-9]+$").match(text))
        return text;
    else
    {
        FXString rettxt;
        for(FXint i=0; i<text.length(); i++)
        {
            if((47<(FXint)text[i]&&(FXint)text[i]<58) ||
                    (64<(FXint)text[i]&&(FXint)text[i]<91) ||
                    (96<(FXint)text[i]&&text[i]<123))
                rettxt += text[i];
            else
                rettxt += '0';
        }
        return rettxt;
    }
}

FXString utils::createModes(FXchar sign, FXchar mode, FXString nicks)
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

void utils::setAlias()
{
    FXSettings set;
    set.parseFile(getIniFile(), TRUE);
    FXint no = set.readIntEntry("ALIASES", "number", 0);
    for(FXint i=0; i<no; i++)
    {
        FXString key, value;
        key = set.readStringEntry("ALIASES", FXStringFormat("key%d", i).text());
        value = set.readStringEntry("ALIASES", FXStringFormat("value%d", i).text());
        if(!key.empty() && !value.empty() && key[0]=='/' && !key.contains(' '))
            m_aliases.insert(StringPair(key, value));
    }
    fillCommands();

}

void utils::setAliases(dxStringMap a)
{
    m_aliases = a;
    fillCommands();
}

FXString utils::getAlias(FXString key)
{
    if(m_aliases.count(key.lower())>0)
        return m_aliases.find(key)->second;
    return FXString::null;
}

dxStringMap utils::getAliases()
{
    return m_aliases;
}

FXint utils::commandsNo()
{
    return m_commands.no();
}

FXString utils::commandsAt(FXint i)
{
    return m_commands.at(i);
}

FXbool utils::isCommand(const FXString &command)
{
    if(!m_commands.no()) return FALSE;
    for(FXint i=0; i<m_commands.no(); i++)
    {
        if(comparecase(command, m_commands.at(i)) == 0) return TRUE;
    }
    return FALSE;
}

//This's from Xfe, thanks
FXbool utils::isUtf8(const FXchar* string, FXuint length)
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
            if (streq(s,BOM))
                return TRUE;

            // String don't contain BOM
            else
                return FALSE;
        }
    }
}

void utils::addScriptCommand(LuaScriptCommand command)
{
    m_scriptCommands.append(command);
    fillCommands();
}

//Remove one command for lua script
FXbool utils::removeScriptCommand(const FXString &command)
{
    for(FXint i=m_scriptCommands.no()-1; i>-1; i--)
    {
        if(comparecase(command, m_scriptCommands[i].name) == 0)
        {
            m_scriptCommands.erase(i);
            fillCommands();
            return TRUE;
        }
    }
    return FALSE;
}

//Remove all commands for lua script
FXbool utils::removeScriptCommands(const FXString& script)
{
    FXbool result = FALSE;
    for(FXint i=m_scriptCommands.no()-1; i>-1; i--)
    {
        if(comparecase(script, m_scriptCommands[i].script) == 0)
        {
            m_scriptCommands.erase(i);
            result = TRUE;
        }
    }
    fillCommands();
    return result;
}

FXbool utils::isScriptCommand(const FXString &command)
{
    for(FXint i=0; i<m_scriptCommands.no(); i++)
    {
        if(comparecase(command, m_scriptCommands[i].name) == 0) return TRUE;
    }
    return FALSE;
}

FXString utils::availableCommands()
{
    FXString commandstr = _("Available commnads: ");
    for(FXint i=0; i < m_commands.no(); i++)
    {
        if(m_commands[i] != "commands") commandstr += m_commands[i].upper()+(i != m_commands.no() - 1? ", " : "");
    }
    return commandstr;
}

FXString utils::availableScriptCommands()
{
    FXString commandstr = _("Available commnads: ");
    for(FXint i=0; i < m_scriptCommands.no(); i++)
    {
        commandstr += m_scriptCommands[i].name.upper()+(i != m_scriptCommands.no() - 1? ", " : "");
    }
    return commandstr;
}

FXString utils::getHelpText(const FXString &command)
{
    for(FXint i=0; i<m_scriptCommands.no(); i++)
    {
        if(comparecase(command, m_scriptCommands[i].name) == 0) return m_scriptCommands[i].helptext;
    }
    return FXStringFormat(_("Command %s doesn't exists"), command.text());
}

FXString utils::getFuncname(const FXString &command)
{
    for(FXint i=0; i<m_scriptCommands.no(); i++)
    {
        if(comparecase(command, m_scriptCommands[i].name) == 0) return m_scriptCommands[i].funcname;
    }
    return FXStringFormat(_("Command %s doesn't exists"), command.text());
}

FXString utils::getScriptName(const FXString &command)
{
    for(FXint i=0; i<m_scriptCommands.no(); i++)
    {
        if(comparecase(command, m_scriptCommands[i].name) == 0) return m_scriptCommands[i].script;
    }
    return FXStringFormat(_("Command %s doesn't exists"), command.text());
}

FXString utils::checkThemePath(const FXString &path)
{
    if(path == "internal") return path;
    else
    {
        const char *themeDefaultPath = DXIRC_DATADIR PATHSEPSTRING "icons" PATHSEPSTRING "default";
        if(FXStat::exists(path)) return path;
        return FXString(themeDefaultPath);
    }
}

FXString utils::checkThemesList(const FXString &list)
{
    const char *themeDefaultPath = DXIRC_DATADIR PATHSEPSTRING "icons" PATHSEPSTRING "default;";
    FXString themes;
    for(FXint i=0; i<list.contains(';'); i++)
    {
        if(list.before(';', i+1).rafter(';') == "internal") themes.append("internal;");
        if(FXStat::exists(list.before(';', i+1).rafter(';'))) themes.append(list.before(';', i+1).rafter(';')+";");
    }
    if(!themes.empty()) return themes;
    return FXString("internal;")+FXString(themeDefaultPath);
}

FXString utils::encrypt(const FXString &text)
{
    FXString result = "";
    for(FXint i=0; i<text.count(); i++)
    {
        result += text[i];
        FXint r = rand()%127;
        result += FXchar(r<33 ? r+33 : r);
    }
    return result;
}

FXString utils::decrypt(const FXString &text)
{
    FXString result = "";
    for(FXint i=0; i<text.count(); i++)
    {
        if((i+1)%2) result += text[i];
    }
    return result;
}

FXString utils::getStringIniEntry(const FXchar *section,const FXchar *key,const FXchar *def)
{
    FXSettings set;
    set.parseFile(m_iniFile, TRUE);
    return set.readStringEntry(section, key, def);
}

FXint utils::getIntIniEntry(const FXchar* section, const FXchar* key, FXint def)
{
    FXSettings set;
    set.parseFile(m_iniFile, TRUE);
    return set.readIntEntry(section, key, def);
}

//return bool entry in inifile
FXbool utils::getBoolIniEntry(const FXchar* section, const FXchar* key, FXbool def)
{
    FXSettings set;
    set.parseFile(m_iniFile, TRUE);
    return set.readBoolEntry(section, key, def);
}

//Return file size in human readable form
FXString utils::getFileSize(FXlong size)
{
    FXfloat fsize = 0.0;
    if(size > 1000000000)
    {
        fsize = size/1073741824.0;
        if(fsize == (FXint)fsize)
            return FXStringFormat("%.0f %s", fsize, _("GB"));
        else
            return FXStringFormat("%.2f %s", fsize, _("GB"));
    }
    if(size > 100000)
    {
        fsize = size/1048576.0;
        if(fsize == (FXint)fsize)
            return FXStringFormat("%.0f %s", fsize, _("MB"));
        else
            return FXStringFormat("%.2f %s", fsize, _("MB"));
    }
    if(size > 1000)
    {
        fsize = size/1024.0;
        if(fsize == (FXint)fsize)
            return FXStringFormat("%.0f %s", fsize, _("KB"));
        else
            return FXStringFormat("%.2f %s", fsize, _("KB"));
    }
    return FXStringFormat("%s %s", FXStringVal(size).text(), _("bytes"));
}

//Return file size in human readable form
FXString utils::getFileSize(const FXString &ssize)
{
    FXlong size = FXLongVal(ssize);
    FXfloat fsize = 0.0;
    if(size > 1000000000)
    {
        fsize = size/1073741824.0;
        if(fsize == (FXuint)fsize)
            return FXStringFormat("%.0f %s", fsize, _("GB"));
        else
            return FXStringFormat("%.2f %s", fsize, _("GB"));
    }
    if(size > 100000)
    {
        fsize = size/1048576.0;
        if(fsize == (FXuint)fsize)
            return FXStringFormat("%.0f %s", fsize, _("MB"));
        else
            return FXStringFormat("%.2f %s", fsize, _("MB"));
    }
    if(size > 1000)
    {
        fsize = size/1024.0;
        if(fsize == (FXuint)fsize)
            return FXStringFormat("%.0f %s", fsize, _("KB"));
        else
            return FXStringFormat("%.2f %s", fsize, _("KB"));
    }
    return FXStringFormat("%s %s", ssize.text(), _("bytes"));
}

/*Return download/send speed in human readable form
 * speed = difference between two position during 1 second
 */
FXString utils::getSpeed(FXlong speed)
{
    FXfloat fspeed = 0.0;
    if(speed > 100000)
    {
        fspeed = speed/(1048576.0);
        if(fspeed == (FXuint)fspeed)
            return FXStringFormat("%.0f %s", fspeed, _("MB/s"));
        else
            return FXStringFormat("%.2f %s", fspeed, _("MB/s"));
    }
    fspeed = speed/(1024.0);
    if(fspeed == (FXuint)fspeed)
        return FXStringFormat("%.0f %s", fspeed, _("KB/s"));
    else
        return FXStringFormat("%.2f %s", fspeed, _("KB/s"));
}

//Return remaining time of download/send
FXString utils::getRemaining(FXlong size, FXlong speed)
{
    FXfloat fsize = 0.0;
    FXfloat fspeed = 0.0;
    FXfloat fremain = 0.0;
    fspeed = speed/1024.0;
    fsize = size/1024.0;
    fremain = fsize/fspeed;
    if(fremain <= 0 || fremain > 86400.0) return "?";
    else
    {
        FXint remainingTime = (FXint)fremain;
        FXint secs = remainingTime;
        FXint hours = remainingTime/3600;
        secs -= hours*3600;
        FXint mins = remainingTime/60;
        secs -= mins*60;
        return FXStringFormat("%02d:%02d:%02d", hours, mins, secs);
    }
}

//play event sound
void utils::playFile(const FXString &file)
{
    if(!FXStat::exists(file))
        return;
#ifdef WIN32
    PlaySoundA((LPCSTR)file.text(), NULL, SND_FILENAME | SND_ASYNC);
#else
    static const char * players[]={"aplay","play","esdplay","artsplay","ossplay",NULL};
    FXString path = FXSystem::getExecPath();
    FXString exec;
    for(int i=0; players[i]!=NULL; i++)
    {
        exec = FXPath::search(path, players[i]);
        if(!exec.empty()) break;
    }
    if(exec.empty()) return;
    if(exec.contains("aplay")) exec += " -q";
    exec += " "+FXPath::enquote(file)+" &";
    system(exec.text());
#endif
}

//write debugline
void utils::debugLine(const FXString &line)
{
#ifdef DEBUG
    fxmessage("[%s] %s\n", FXSystem::time("%H:%M:%S", FXSystem::now()).text(), line.text());
#endif
}

//true if word right
#ifdef HAVE_ENCHANT
FXbool utils::checkWord(FXString word, FXString lang)
{
    Checkers::const_iterator it = m_checkers.find(lang);
    if(it != m_checkers.end())
        return (*it).second->check(word.text());
    return TRUE;
}
#else
FXbool utils::checkWord(FXString, FXString)
{
    return TRUE;
}
#endif

//fill available langs
#ifdef HAVE_ENCHANT
void utils::setLangs()
{
    CheckerPair chpair(&m_checkers, &m_spellLang);
    enchant::Broker::instance()->list_dicts(enumerateDicts, &chpair);
    if(m_spellLang.no())
    {
        FXString v;
        register FXint i,j,h;
        for(h=1; h<=m_spellLang.no()/9; h=3*h+1);
        for(; h > 0; h /= 3)
        {
            for(i = h + 1; i <= m_spellLang.no(); i++)
            {
                v = m_spellLang[i - 1];
                j = i;
                while (j > h && comparecase(m_spellLang[j - h - 1], v) > 0)
                {
                    m_spellLang[j - 1] = m_spellLang[j - h - 1];
                    j -= h;
                }
                m_spellLang[j - 1] = v;
            }
        }
    }
    for(FXint i=0; i<m_spellLang.no(); i++)
    {
        if(m_checkers.find(m_spellLang[i]) != m_checkers.end())
            return;
        try
        {
            m_checkers[m_spellLang[i]] = enchant::Broker::instance()->request_dict(m_spellLang[i].text());
        }
        catch(enchant::Exception &e)
        {
        }
    }
}
#else
void utils::setLangs()
{
    m_spellLang.no(0);
}
#endif

//get number of spellchecking language
#ifdef HAVE_ENCHANT
FXint utils::getLangsNum()
{
    return m_spellLang.no();
}
#else
FXint utils::getLangsNum()
{
    return 0;
}
#endif

//return array of spellchecking language
dxStringArray utils::getLangs()
{
    return m_spellLang;
}

//get default spellchecking language
#ifdef HAVE_ENCHANT
FXString utils::getDefaultLang()
{
    if(m_locale.empty()) getLocale();
    if(m_checkers.find(m_locale) != m_checkers.end())
        return m_locale;
    Checkers::const_iterator it = m_checkers.begin();
    FXString language = m_locale.before('_');
    for(; it != m_checkers.end(); it++)
    {
        if(comparecase((*it).first.before('_'),language)==0) return language;
    }
    if(m_spellLang.no()) return m_spellLang[0];
    else return FXString::null;
}
#else
FXString utils::getDefaultLang()
{
    return FXString::null;
}
#endif

//return locale, e.g cs_CZ etc.
FXString utils::getLocale()
{
    if(!m_locale.empty())
        return m_locale;
#ifdef WIN32
    LCID lcid = GetUserDefaultLCID();
    if(lcid!=0)
    {
        FXuint localeid = LANGIDFROMLCID(lcid);
        /* locale code from this page
         * http://msdn.microsoft.com/en-us/library/dd318693%28v=VS.85%29.aspx
         * not fully implemented
         */
        switch(localeid){
            case 0x0436: m_locale = "af_ZA";break; // Afrikaans
            case 0x041c: m_locale = "sq_AL";break; // Albanian
            case 0x0401: m_locale = "ar_SA";break; // Arabic - Saudi Arabia
            case 0x0801: m_locale = "ar_IQ";break; // Arabic - Iraq
            case 0x0c01: m_locale = "ar_EG";break; // Arabic - Egypt
            case 0x1001: m_locale = "ar_LY";break; // Arabic - Libya
            case 0x1401: m_locale = "ar_DZ";break; // Arabic - Algeria
            case 0x1801: m_locale = "ar_MA";break; // Arabic - Morocco
            case 0x1c01: m_locale = "ar_TN";break; // Arabic - Tunisia
            case 0x2001: m_locale = "ar_OM";break; // Arabic - Oman
            case 0x2401: m_locale = "ar_YE";break; // Arabic - Yemen
            case 0x2801: m_locale = "ar_SY";break; // Arabic - Syria
            case 0x2c01: m_locale = "ar_JO";break; // Arabic - Jordan
            case 0x3001: m_locale = "ar_LB";break; // Arabic - Lebanon
            case 0x3401: m_locale = "ar_KW";break; // Arabic - Kuwait
            case 0x3801: m_locale = "ar_AE";break; // Arabic - United Arab Emirates
            case 0x3c01: m_locale = "ar_BH";break; // Arabic - Bahrain
            case 0x4001: m_locale = "ar_QA";break; // Arabic - Qatar
            case 0x042b: m_locale = "hy_AM";break; // Armenian
            case 0x042c: m_locale = "az_AZ";break; // Azeri Latin
            case 0x082c: m_locale = "az_AZ";break; // Azeri - Cyrillic
            case 0x042d: m_locale = "eu_ES";break; // Basque
            case 0x0423: m_locale = "be_BY";break; // Belarusian
            case 0x0445: m_locale = "bn_IN";break; // Begali
            case 0x201a: m_locale = "bs_BA";break; // Bosnian
            case 0x141a: m_locale = "bs_BA";break; // Bosnian - Cyrillic
            case 0x047e: m_locale = "br_FR";break; // Breton - France
            case 0x0402: m_locale = "bg_BG";break; // Bulgarian
            case 0x0403: m_locale = "ca_ES";break; // Catalan
            case 0x0004: m_locale = "zh_CHS";break; // Chinese - Simplified
            case 0x0404: m_locale = "zh_TW";break; // Chinese - Taiwan
            case 0x0804: m_locale = "zh_CN";break; // Chinese - PRC
            case 0x0c04: m_locale = "zh_HK";break; // Chinese - Hong Kong S.A.R.
            case 0x1004: m_locale = "zh_SG";break; // Chinese - Singapore
            case 0x1404: m_locale = "zh_MO";break; // Chinese - Macao S.A.R.
            case 0x7c04: m_locale = "zh_CHT";break; // Chinese - Traditional
            case 0x041a: m_locale = "hr_HR";break; // Croatian
            case 0x101a: m_locale = "hr_BA";break; // Croatian - Bosnia
            case 0x0405: m_locale = "cs_CZ";break; // Czech
            case 0x0406: m_locale = "da_DK";break; // Danish
            case 0x048c: m_locale = "gbz_AF";break; // Dari - Afghanistan
            case 0x0465: m_locale = "div_MV";break; // Divehi - Maldives
            case 0x0413: m_locale = "nl_NL";break; // Dutch - The Netherlands
            case 0x0813: m_locale = "nl_BE";break; // Dutch - Belgium
            case 0x0409: m_locale = "en_US";break; // English - United States
            case 0x0809: m_locale = "en_GB";break; // English - United Kingdom
            case 0x0c09: m_locale = "en_AU";break; // English - Australia
            case 0x1009: m_locale = "en_CA";break; // English - Canada
            case 0x1409: m_locale = "en_NZ";break; // English - New Zealand
            case 0x1809: m_locale = "en_IE";break; // English - Ireland
            case 0x1c09: m_locale = "en_ZA";break; // English - South Africa
            case 0x2009: m_locale = "en_JA";break; // English - Jamaica
            case 0x2409: m_locale = "en_CB";break; // English - Carribbean
            case 0x2809: m_locale = "en_BZ";break; // English - Belize
            case 0x2c09: m_locale = "en_TT";break; // English - Trinidad
            case 0x3009: m_locale = "en_ZW";break; // English - Zimbabwe
            case 0x3409: m_locale = "en_PH";break; // English - Phillippines
            case 0x0425: m_locale = "et_EE";break; // Estonian
            case 0x0438: m_locale = "fo_FO";break; // Faroese
            case 0x0464: m_locale = "fil_PH";break; // Filipino
            case 0x040b: m_locale = "fi_FI";break; // Finnish
            case 0x040c: m_locale = "fr_FR";break; // French - France
            case 0x080c: m_locale = "fr_BE";break; // French - Belgium
            case 0x0c0c: m_locale = "fr_CA";break; // French - Canada
            case 0x100c: m_locale = "fr_CH";break; // French - Switzerland
            case 0x140c: m_locale = "fr_LU";break; // French - Luxembourg
            case 0x180c: m_locale = "fr_MC";break; // French - Monaco
            case 0x0462: m_locale = "fy_NL";break; // Frisian - Netherlands
            case 0x0456: m_locale = "gl_ES";break; // Galician
            case 0x0437: m_locale = "ka_GE";break; // Georgian
            case 0x0407: m_locale = "de_DE";break; // German - Germany
            case 0x0807: m_locale = "de_CH";break; // German - Switzerland
            case 0x0c07: m_locale = "de_AT";break; // German - Austria
            case 0x1007: m_locale = "de_LU";break; // German - Luxembourg
            case 0x1407: m_locale = "de_LI";break; // German - Liechtenstein
            case 0x0408: m_locale = "el_GR";break; // Greek
            case 0x0447: m_locale = "gu_IN";break; // Gujarati
            case 0x040d: m_locale = "he_IL";break; // Hebrew
            case 0x0439: m_locale = "hi_IN";break; // Hindi
            case 0x040e: m_locale = "hu_HU";break; // Hungarian
            case 0x040f: m_locale = "is_IS";break; // Icelandic
            case 0x0421: m_locale = "id_ID";break; // Indonesian
            case 0x045d: m_locale = "iu_CA";break; // Inuktitut
            case 0x085d: m_locale = "iu_CA";break; // Inuktitut - Latin
            case 0x083c: m_locale = "ga_IE";break; // Irish - Ireland
            case 0x0434: m_locale = "xh_ZA";break; // Xhosa - South Africa
            case 0x0435: m_locale = "zu_ZA";break; // Zulu
            case 0x0410: m_locale = "it_IT";break; // Italian - Italy
            case 0x0810: m_locale = "it_CH";break; // Italian - Switzerland
            case 0x0411: m_locale = "ja_JP";break; // Japanese
            case 0x044b: m_locale = "kn_IN";break; // Kannada - India
            case 0x043f: m_locale = "kk_KZ";break; // Kazakh
            case 0x0457: m_locale = "kok_IN";break; // Konkani
            case 0x0412: m_locale = "ko_KR";break; // Korean
            case 0x0440: m_locale = "ky_KG";break; // Kyrgyz
            case 0x0426: m_locale = "lv_LV";break; // Latvian
            case 0x0427: m_locale = "lt_LT";break; // Lithuanian
            case 0x046e: m_locale = "lb_LU";break; // Luxembourgish
            case 0x042f: m_locale = "mk_MK";break; // FYRO Macedonian
            case 0x043e: m_locale = "ms_MY";break; // Malay - Malaysia
            case 0x083e: m_locale = "ms_BN";break; // Malay - Brunei
            case 0x044c: m_locale = "ml_IN";break; // Malayalam - India
            case 0x043a: m_locale = "mt_MT";break; // Maltese
            case 0x0481: m_locale = "mi_NZ";break; // Maori
            case 0x047a: m_locale = "arn_CL";break; // Mapudungun
            case 0x044e: m_locale = "mr_IN";break; // Marathi
            case 0x047c: m_locale = "moh_CA";break; // Mohawk - Canada
            case 0x0450: m_locale = "mn_MN";break; // Mongolian
            case 0x0461: m_locale = "ne_NP";break; // Nepali
            case 0x0414: m_locale = "nb_NO";break; // Norwegian - Bokmal
            case 0x0814: m_locale = "nn_NO";break; // Norwegian - Nynorsk
            case 0x0482: m_locale = "oc_FR";break; // Occitan - France
            case 0x0448: m_locale = "or_IN";break; // Oriya - India
            case 0x0463: m_locale = "ps_AF";break; // Pashto - Afghanistan
            case 0x0429: m_locale = "fa_IR";break; // Persian
            case 0x0415: m_locale = "pl_PL";break; // Polish
            case 0x0416: m_locale = "pt_BR";break; // Portuguese - Brazil
            case 0x0816: m_locale = "pt_PT";break; // Portuguese - Portugal
            case 0x0446: m_locale = "pa_IN";break; // Punjabi
            case 0x046b: m_locale = "quz_BO";break; // Quechua (Bolivia)
            case 0x086b: m_locale = "quz_EC";break; // Quechua (Ecuador)
            case 0x0c6b: m_locale = "quz_PE";break; // Quechua (Peru)
            case 0x0418: m_locale = "ro_RO";break; // Romanian - Romania
            case 0x0417: m_locale = "rm_CH";break; // Raeto-Romanese
            case 0x0419: m_locale = "ru_RU";break; // Russian
            case 0x243b: m_locale = "smn_FI";break; // Sami Finland
            case 0x103b: m_locale = "smj_NO";break; // Sami Norway
            case 0x143b: m_locale = "smj_SE";break; // Sami Sweden
            case 0x043b: m_locale = "se_NO";break; // Sami Northern Norway
            case 0x083b: m_locale = "se_SE";break; // Sami Northern Sweden
            case 0x0c3b: m_locale = "se_FI";break; // Sami Northern Finland
            case 0x203b: m_locale = "sms_FI";break; // Sami Skolt
            case 0x183b: m_locale = "sma_NO";break; // Sami Southern Norway
            case 0x1c3b: m_locale = "sma_SE";break; // Sami Southern Sweden
            case 0x044f: m_locale = "sa_IN";break; // Sanskrit
            case 0x0c1a: m_locale = "sr_SP";break; // Serbian - Cyrillic
            case 0x1c1a: m_locale = "sr_BA";break; // Serbian - Bosnia Cyrillic
            case 0x081a: m_locale = "sr_SP";break; // Serbian - Latin
            case 0x181a: m_locale = "sr_BA";break; // Serbian - Bosnia Latin
            case 0x046c: m_locale = "ns_ZA";break; // Northern Sotho
            case 0x0432: m_locale = "tn_ZA";break; // Setswana - Southern Africa
            case 0x041b: m_locale = "sk_SK";break; // Slovak
            case 0x0424: m_locale = "sl_SI";break; // Slovenian
            case 0x040a: m_locale = "es_ES";break; // Spanish - Spain
            case 0x080a: m_locale = "es_MX";break; // Spanish - Mexico
            case 0x0c0a: m_locale = "es_ES";break; // Spanish - Spain (Modern)
            case 0x100a: m_locale = "es_GT";break; // Spanish - Guatemala
            case 0x140a: m_locale = "es_CR";break; // Spanish - Costa Rica
            case 0x180a: m_locale = "es_PA";break; // Spanish - Panama
            case 0x1c0a: m_locale = "es_DO";break; // Spanish - Dominican Republic
            case 0x200a: m_locale = "es_VE";break; // Spanish - Venezuela
            case 0x240a: m_locale = "es_CO";break; // Spanish - Colombia
            case 0x280a: m_locale = "es_PE";break; // Spanish - Peru
            case 0x2c0a: m_locale = "es_AR";break; // Spanish - Argentina
            case 0x300a: m_locale = "es_EC";break; // Spanish - Ecuador
            case 0x340a: m_locale = "es_CL";break; // Spanish - Chile
            case 0x380a: m_locale = "es_UR";break; // Spanish - Uruguay
            case 0x3c0a: m_locale = "es_PY";break; // Spanish - Paraguay
            case 0x400a: m_locale = "es_BO";break; // Spanish - Bolivia
            case 0x440a: m_locale = "es_SV";break; // Spanish - El Salvador
            case 0x480a: m_locale = "es_HN";break; // Spanish - Honduras
            case 0x4c0a: m_locale = "es_NI";break; // Spanish - Nicaragua
            case 0x500a: m_locale = "es_PR";break; // Spanish - Puerto Rico
            case 0x0441: m_locale = "sw_KE";break; // Swahili
            case 0x041d: m_locale = "sv_SE";break; // Swedish - Sweden
            case 0x081d: m_locale = "sv_FI";break; // Swedish - Finland
            case 0x045a: m_locale = "syr_SY";break; // Syriac
            case 0x0449: m_locale = "ta_IN";break; // Tamil
            case 0x0444: m_locale = "tt_RU";break; // Tatar
            case 0x044a: m_locale = "te_IN";break; // Telugu
            case 0x041e: m_locale = "th_TH";break; // Thai
            case 0x041f: m_locale = "tr_TR";break; // Turkish
            case 0x0422: m_locale = "uk_UA";break; // Ukrainian
            case 0x0420: m_locale = "ur_PK";break; // Urdu
            case 0x0820: m_locale = "ur_IN";break; // Urdu - India
            case 0x0443: m_locale = "uz_UZ";break; // Uzbek - Latin
            case 0x0843: m_locale = "uz_UZ";break; // Uzbek - Cyrillic
            case 0x042a: m_locale = "vi_VN";break; // Vietnamese
            case 0x0452: m_locale = "cy_GB";break; // Welsh
            default: m_locale = "en_US";
        }
    }
    else m_locale = "en_US";
#else
    if(!(FXSystem::getEnvironment("LANG")).empty()) m_locale = FXSystem::getEnvironment("LANG");
    else if(!(FXSystem::getEnvironment("LC_ALL")).empty()) m_locale = FXSystem::getEnvironment("LC_ALL");
    else if(!(FXSystem::getEnvironment("LC_MESSAGES")).empty()) m_locale = FXSystem::getEnvironment("LC_MESSAGES");
    else m_locale = "en_US";
    if(m_locale == "C" || m_locale == "POSIX")
    {
        m_locale = "en_US";
    }
    m_locale = m_locale.before('.').before('@');
#endif
    return m_locale;
}

//get spellchecking language for channel
#if 1
FXString utils::getChannelLang(FXString channel)
{
    FXSettings set;
    debugLine(FXStringFormat("Lang file:%s",FXPath::directory(m_iniFile).append(PATHSEPSTRING "langs").text()));
    set.parseFile(FXPath::directory(m_iniFile).append(PATHSEPSTRING "langs"), TRUE);
    FXString lang = set.readStringEntry("LANGS", channel.prepend('_').text(), getDefaultLang().text());
    if(m_checkers.find(lang) != m_checkers.end())
        return lang;
    else return getDefaultLang();
}
#else
FXString utils::getChannelLang(FXString)
{
    return FXString::null;
}
#endif

