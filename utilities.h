//
// Created by dazzzed on 21.02.2021.
//
#pragma once

#include <iostream>
#include <string>
#include <vector>
#include "math.h"

template <typename T>
static std::string vector_to_string(std::vector<T> v)
{
    std::string out = "";

    if (!v.empty())
    {
        out += "{ ";

        for (auto item: v)
        {
            out += std::to_string(item);
            out += ", ";
        }
        out = out.substr(0, out.size()-2);
        out += " }";
        return out;
    }
    else {
        return "";
    }
}

static int binaryStringToInt(std::string binaryString)
{
    int result = 0;

    for (int i = 0; i < binaryString.size(); i++)
    {
        int bit = binaryString[binaryString.size() - i - 1] - '0';
        result += bit * pow(2, i);
    }

    return result;
}

static void analyzeStats()
{
    std::ifstream file;
    file.open("stats.txt");

    std::vector<std::string> lines;
    std::string line;

    while (getline(file, line))
    {
        lines.push_back(line);
    }

    for (int i = 0; i < lines.size(); i++)
    {
        double time1 = std::stod(lines[i].substr(0, lines[i].find(':')));
        std::string data1 = lines[i].substr(lines[i].find('{'), lines[i].size());

        for (int j = i+1; j < lines.size(); j++)
        {
            double time2 = std::stod(lines[j].substr(0, lines[j].find(':')));
            std::string data2 = lines[j].substr(lines[j].find('{'), lines[j].size());

            if (data1 == data2)
            {
                double delay = abs(time1 - time2);
                std::cout << "delay = " << delay << " [Data] : " << data1 << std::endl;
            }
        }

    }

}