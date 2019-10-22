#ifndef _lang_hh
#define _lang_hh

#include <iostream>

enum class LangKind
{
    en = 0,
    fr,
    ru,
    de
};

class Lang
{
public:
    Lang();

    Lang(const Lang &lang);

    Lang(const std::string &str_lang);


    void setLang(const LangKind& value);
    void setLang(std::string str1);

    Lang& operator =(const std::string &str_lang);

    friend std::ostream& operator << (std::ostream &sss, Lang lang);

    LangKind getLang() const;

    std::string toString();

private:
    LangKind lang;

};

#endif
