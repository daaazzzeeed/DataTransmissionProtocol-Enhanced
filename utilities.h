//
// Created by dazzzed on 21.02.2021.
//
#pragma once

#include <iostream>
#include <string>
#include <vector>

static std::string vector_to_string(std::vector<std::string> v)
{
    std::string out = "";

    if (!v.empty())
    {
        out += "{ ";

        for (auto item: v)
        {
            out += item;
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

static std::string vector_to_string(std::vector<int> v)
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
