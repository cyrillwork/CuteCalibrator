/*
 * Copyright (c) 2013 Andreas Müller
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "gui/gui_common.hpp"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/istreamwrapper.h"

#include <iostream>
#include <fstream>


void get_display_texts_default(std::vector<std::string> *texts, const Lang& lang)
{
	switch(lang.getLang())
	{
		case LangKind::fr:
		{
			texts->push_back("Touchez la cible pour continuer");
			texts->push_back("ou attendez pour annuler");
			texts->push_back("Intéraction tactile incorrecte. Redémarrage en cours…");
			texts->push_back("La calibration est terminée");
		}
		break;

		case LangKind::ru:
		{
			texts->push_back("Нажмите на цель, чтобы продолжить, ");
			texts->push_back("или ожидайте, чтобы отменить калибровку");
			texts->push_back("Некорректное нажатие. Перезапуск...");
			texts->push_back("Калибровка завершена");
		}
		break;

		default:
		{
			texts->push_back("Touch the target to continue ");
			texts->push_back("or wait to cancel");
			texts->push_back("Mistouch detected, restarting...");
			texts->push_back("Calibration completed");
		}

	}
}

void get_display_texts_testmode(std::list<std::string> *texts, PtrCalibrator /*calibrator*/)
{
    std::string str;
    /* 1st line */
    str = "Touchscreen test mode";
    /*
    const char* sysfs_name = calibrator->get_sysfs_name();
    if(sysfs_name != NULL) {
        str += " for '";
        str += sysfs_name;
        str += "'";
    }
    */
    texts->push_back(str);
    /* 2nd line */
    str = "Press screen for testing...";
    texts->push_back(str);
    /* 3rd line */
    str = "";
    texts->push_back(str);
    /* 4th line */
    texts->push_back("(To abort, press any key)");
}

using namespace rapidjson;

bool get_display_texts_json(std::vector<std::string>* texts, const Lang& lang)
{
    bool result = false;

    std::ifstream ifs(Calibrator::getPathResource() + "lang.json");

    if(ifs.is_open())
    {
        Document document;
        IStreamWrapper wp(ifs);
        document.ParseStream(wp);
        for(auto it=document.MemberBegin(); it<document.MemberEnd(); ++it)
        {
            //std::cout << "Element name=" << it->name.GetString() << std::endl;
            std::string node(it->name.GetString());
            if(node == lang.toString())
            {
                //std::cout << "node=" << node << std::endl;
                const auto &doc = it->value;
                if(doc.IsObject())
                {
                    texts->push_back(doc["0"].GetString());
                    texts->push_back(doc["1"].GetString());
                    texts->push_back(doc["2"].GetString());
                    texts->push_back(doc["3"].GetString());
                    result = true;
                }
            }
        }
    }

    return result;
}
