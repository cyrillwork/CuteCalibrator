#include "lang.hh"

Lang::Lang():
    lang{ LangKind::en }
{
}

Lang::Lang(const Lang&lang)
{
    this->lang = lang.getLang();
}

Lang::Lang(const std::string& str_lang)
{
    this->setLang(str_lang);
}

void Lang::setLang(const LangKind& value)
{
    lang = value;
}

void Lang::setLang(std::string str1)
{
    if(str1 == "en")
    {
        this->lang = LangKind::en;
    } else
    if(str1 == "fr")
    {
        this->lang = LangKind::fr;
    } else
    if(str1 == "ru")
    {
        this->lang = LangKind::ru;
    } else
    if(str1 == "de")
    {
        this->lang = LangKind::de;
    }
}

Lang& Lang::operator =(const std::string &str_lang)
{
    this->setLang(str_lang);
    return *this;
}

LangKind Lang::getLang() const
{
    return lang;
}

std::string Lang::toString()
{
    switch (lang) {
        case LangKind::en:
            return "en";
        break;
        case LangKind::ru:
            return "ru";
        break;
        case LangKind::de:
            return "de";
        break;
        case LangKind::fr:
            return "fr";
        break;
    }
    return "null";
}

std::ostream& operator <<(std::ostream& sss, Lang lang)
{
    switch (lang.lang)
    {
        case LangKind::fr:
        {
            sss << "French";
        }
        break;
        case LangKind::en:
        {
            sss << "English";
        }
        break;
        default:
            sss << "Unknown language";
    }
    return sss;
}
