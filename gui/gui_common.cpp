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

using namespace rapidjson;

void get_display_texts_default(std::shared_ptr<std::vector<std::string>> texts, const Lang& lang)
{
	switch(lang.getLang())
	{
		case LangKind::fr:
		{
			texts->push_back("Touchez la cible pour continuer");
			texts->push_back("ou attendez pour annuler");
			texts->push_back("Intéraction tactile incorrecte. Redémarrage en cours…");
			texts->push_back("La calibration est terminée");
			texts->push_back("Test mode. Check calibration.");
		}
		break;

		case LangKind::ru:
		{
			texts->push_back("Нажмите на цель, чтобы продолжить, ");
			texts->push_back("или ожидайте, чтобы отменить калибровку");
			texts->push_back("Некорректное нажатие. Перезапуск...");
			texts->push_back("Калибровка завершена");
			texts->push_back("Test mode. Check calibration.");
		}
		break;

		default:
		{
			texts->push_back("Touch the target to continue ");
			texts->push_back("or wait to cancel");
			texts->push_back("Mistouch detected, restarting...");
			texts->push_back("Calibration completed");
			texts->push_back("Test mode. Check calibration.");
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

bool get_common_data_json(CommonData &data, const Lang& lang)
{
    bool result = false;

    std::string fileName = Calibrator::getPathResource() + "options.json";

    std::ifstream ifs(fileName);

    if(ifs.is_open())
    {

        data.getDisplay_texts()->clear();

        Document document;
        IStreamWrapper wp(ifs);
        document.ParseStream(wp);
        for(auto it=document.MemberBegin(); it<document.MemberEnd(); ++it)
        {
            //std::cout << "Element name=" << it->name.GetString() << std::endl;
            std::string node(it->name.GetString());

            if(node == "main")
            {
                const auto &doc = it->value;
                if(doc.IsObject())
                {
                    data.setTimeStep( doc["time_step"].GetInt() );
                    data.setMaxTime( doc["max_time"].GetInt() );
                    data.setLastTime( doc["last_time"].GetInt() );
                    data.setCrossLines( doc["cross_lines"].GetInt() );
                    data.setCrossCircle( doc["cross_circle"].GetInt() );
                    data.setClockRadius( doc["clock_radius"].GetInt() );
                    data.setClockLineWidth( doc["clock_line_width"].GetInt() );
                    data.setDefaultBoarderWidth( doc["boarderWidth"].GetInt() );
                }
            }
            else
            if(node == lang.toString())
            {
                //std::cout << "node=" << node << std::endl;
                const auto &doc = it->value;
                if(doc.IsArray())
                {
                    for(auto iii = doc.Begin(); iii < doc.End(); ++iii)
                    {
                        data.getDisplay_texts()->push_back(iii->GetString());
                    }
                    result = true;
                }
                else
                {
                    std::cout << "Error json, lang strings aren't array !!!" << std::endl;
                }
            }
        }
    }
    else
    {
        std::cout << "File " << fileName << " not open !!!" << std::endl;
    }

    return result;
}

CommonData::CommonData(const int time_step,
                            const int max_time,
                            const int last_time,
                            const int cross_lines,
                            const int cross_circle,
                            const int clock_radius,
                            const int clock_line_width,
                            const int boarderWidth,
                            const std::string&):
    timeStep(time_step),
    maxTime(max_time),
    lastTime(last_time),
    crossLines(cross_lines),
    crossCircle(cross_circle),
    clockRadius(clock_radius),
    clockLineWidth(clock_line_width),
    defaultBoarderWidth(boarderWidth)
{
    display_texts = std::make_shared<std::vector<std::string> >();
}

int CommonData::getTimeStep() const
{
    return timeStep;
}

void CommonData::setTimeStep(int value)
{
    timeStep = value;
}

int CommonData::getMaxTime() const
{
    return maxTime;
}

void CommonData::setMaxTime(int value)
{
    maxTime = value;
}

int CommonData::getLastTime() const
{
    return lastTime;
}

void CommonData::setLastTime(int value)
{
    lastTime = value;
}

int CommonData::getCrossLines() const
{
    return crossLines;
}

void CommonData::setCrossLines(int value)
{
    crossLines = value;
}

int CommonData::getCrossCircle() const
{
    return crossCircle;
}

void CommonData::setCrossCircle(int value)
{
    crossCircle = value;
}

int CommonData::getClockRadius() const
{
    return clockRadius;
}

void CommonData::setClockRadius(int value)
{
    clockRadius = value;
}

int CommonData::getClockLineWidth() const
{
    return clockLineWidth;
}

void CommonData::setClockLineWidth(int value)
{
    clockLineWidth = value;
}

int CommonData::getDefaultBoarderWidth() const
{
    return defaultBoarderWidth;
}

void CommonData::setDefaultBoarderWidth(int value)
{
    defaultBoarderWidth = value;
}

void CommonData::setDisplay_texts(const PtrVectorString&value)
{
    display_texts = value;
}

CommonData::PtrVectorString CommonData::getDisplay_texts() const
{
    return display_texts;
}

void CommonData::initDataFromFile(const std::string &lang)
{
    if(!get_common_data_json(*this,  Lang(lang)))
    {
        get_display_texts_default(display_texts, Lang(lang));
    }

}
