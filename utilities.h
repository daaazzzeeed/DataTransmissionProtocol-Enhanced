//
// Created by dazzzed on 21.02.2021.
//
#pragma once

#include <iostream>
#include <cmath>
#include <string>
#include <vector>
#include <map>
#include <set>
#include "IConnectable.h"

template <typename T1, typename T2>
static std::string map_to_string(std::map<T1, T2> map)
{
    std::string out = "{ ";

    if (!map.empty())
    {
        for (auto it = map.begin(); it != map.end(); it++)
        {
            out += it->first + " : " + std::to_string(it->second) + ", ";
        }
    }

    out = out.substr(0, out.size()-2);
    out += " }";

    return out;
}

template <typename T>
static std::string iterable_to_string(std::vector<T> v)
{
    std::string out;

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
    // TODO : replace current logic with a dictionary based one

    std::map<std::string, int> stats =
        {
            {"created", 0},
            {"approved", 0},
            {"denied", 0},
            {"received", 0},
            {"total", 0}
        };

    std::string TYPE_CREATED = " created ";
    std::string TYPE_RECEIVED = " received ";
    std::string TYPE_APPROVED = " approved ";
    std::string TYPE_DENIED = " denied ";

    std::ifstream file;
    file.open("stats.txt");

    std::vector<std::string> lines;
    std::string item;

    while (getline(file, item))
    {
        lines.push_back(item);
    }

    for (int i = 0; i < lines.size(); i++)
    {
        double time1 = std::stod(lines[i].substr(0, lines[i].find(':')));

        std::string type = lines[i].substr(lines[i].find(' '), lines[i].find('{')-lines[i].find(' '));

        if (type == TYPE_CREATED)
        {
            stats["created"]++;
            std::string data1 = lines[i].substr(lines[i].find('{'), lines[i].find('}')-lines[i].find('{') + 1);

            for (auto & line : lines)
            {
                std::string type2 = line.substr(line.find(' '), line.find('{')-line.find(' '));
                std::string data2 = line.substr(line.find('{'), line.find('}')-line.find('{')+1);

                if (data1 == data2 && type2 == TYPE_RECEIVED)
                {
                    double time2 = std::stod(line.substr(0, line.find(':')));
                    double delay = abs(time1 - time2);
                    std::cout << "delay = " << delay << " [Data] : " << data1 << std::endl;
                }
            }
        }

        if (type == TYPE_APPROVED)
        {
            stats["approved"]++;
        }

        if (type == TYPE_DENIED)
        {
            stats["denied"]++;
        }

        if (type == TYPE_RECEIVED)
        {
            stats["received"]++;
        }

        stats["total"]++;
    }

    std::cout << map_to_string(stats) << std::endl;
    double dataLoss = (1 - (double)stats["received"] / (double)stats["total"]) * 100;
    std::cout << "data loss: " << dataLoss << "%" << std::endl;
}

auto calculateSchedules(const std::map<int, int>& routersInfo, std::map<int, std::map<int, int>> routeSpecs,
                        const std::map<int, std::vector<std::pair<int, int>>>& routes,
                        int simulationEndTime)
{
    // routersInfo: map { routerIndex : portCount}
    // routeSpecs: map {period : map {routeID, routeTime} }
    // routes: map {routeID: vector {{router1, port1},..., {routerN, portM}}}

    // create resulting data structure
    std::map<int, std::map<int, std::vector<std::vector<int>>>> result = {};

    // fill resulting data structure with the initial values
    for (auto & it : routersInfo)
    {
        int portCount = it.second;
        std::map<int, std::vector<std::vector<int>>> routerData = {};

        for (int i = 0; i < portCount; i++)
        {
            routerData[i] = {};
        }

        result[it.first] = routerData;
    }

    // sort routes by transmission periods
    std::sort(routeSpecs.begin(), routeSpecs.end());

    std::map<int, std::set<int>> requestTimeMomentsMap;

    for (auto & it : routeSpecs)
    {
        int step = it.first;
        int requestTimeMoment = it.first;
        std::set<int> requestTimeMoments;
        while (requestTimeMoment < simulationEndTime)
        {
            requestTimeMoments.insert(requestTimeMoment);
            requestTimeMoment += step;
        }
        requestTimeMomentsMap[it.first] = requestTimeMoments;
    }

    std::map<int, std::vector<int>> requestTimeMomentsIntersections;

    for (auto it = requestTimeMomentsMap.begin(); it != requestTimeMomentsMap.end(); it++)
    {
        for (auto innerIt = it; innerIt != requestTimeMomentsMap.end(); innerIt++)
        {
            std::vector<int> intersectionElements;
            std::set_intersection(it->second.begin(), it->second.end(),
                                  innerIt->second.begin(), innerIt->second.end(),
                                  intersectionElements);

            if (!intersectionElements.empty())
            {
                requestTimeMomentsIntersections[it->first].push_back(innerIt->first);
            }
        }
    }


    return result;
}

static void Connect(IConnectable* A, int portA, IConnectable* B, int portB)
{
    // Connects entity A via portA to an entityB portB
    A->Connect(B, portA);
    B->Connect(A, portB);
}