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

#ifndef GUI_GUI_COMMON_HPP
#define GUI_GUI_COMMON_HPP

#include "calibrator.hh"
#include <list>
#include <string>
#include <memory>

class CommonData;
using PtrCommonData = std::shared_ptr<CommonData>;

class CommonData
{
public:
    CommonData(
                const int time_step = 50,
                const int max_time  = 15000,
                const int last_time = 2000,
                const int cross_lines = 40,
                const int cross_circle = 40,
                const int clock_radius = 120,
                const int clock_line_width = 20,
                const int boarderWidth = 3,
                const std::string &lang = "en");

    CommonData(const std::string &lang);

    struct Font
    {
        Font(){}
        Font(std::string n, int s, int i):
            name(n), fontSize(s), interLines(i)
        {}

        std::string name;
        int fontSize;
        int interLines;
    };

    int getTimeStep() const;
    void setTimeStep(int value);    // in milliseconds

    int getMaxTime() const;
    void setMaxTime(int value);     // in milliseconds

    int getLastTime() const;
    void setLastTime(int value);

    int getCrossLines() const;
    void setCrossLines(int value);

    int getCrossCircle() const;
    void setCrossCircle(int value);

    int getClockRadius() const;
    void setClockRadius(int value);

    int getClockLineWidth() const;
    void setClockLineWidth(int value);

    int getDefaultBoarderWidth() const;
    void setDefaultBoarderWidth(int value);


    using PtrVectorString = std::shared_ptr<std::vector<std::string> >;

    void setDisplay_texts(const PtrVectorString& value);

    PtrVectorString getDisplay_texts() const;

    void initDataFromFile(const std::string &lang);

    CommonData::Font getDefaultFont() const;
    void setDefaultFont(const CommonData::Font& value);

    CommonData::Font getSmallFont() const;
    void setSmallFont(const CommonData::Font& value);

private:

    // Timeout parameters
    int timeStep;          // in milliseconds
    int maxTime;           // in milliseconds, 5000 = 5 sec
    int lastTime;          // in milliseconds

    // Clock appereance
    int crossLines;
    int crossCircle;

    int clockRadius;
    int clockLineWidth;

    int defaultBoarderWidth;

    CommonData::Font defaultFont;
    CommonData::Font smallFont;

    std::shared_ptr<std::vector<std::string> > display_texts = nullptr;
};

void get_display_texts_default(std::shared_ptr<std::vector<std::string> >texts, const Lang &lang);

bool get_common_data_json(CommonData& data, const Lang &lang);

#endif
