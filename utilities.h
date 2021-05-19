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
#include <algorithm>
#include "IConnectable.h"

static std::string map_to_string(const std::map<int, int>& map)
{
    std::string out = "{ ";

    if (!map.empty())
    {
        for (auto & it : map)
        {
            out += std::to_string(it.first) + " : " + std::to_string(it.second) + ", ";
        }
    }

    out = out.substr(0, out.size()-2);
    out += " }";

    return out;
}

static std::string map_to_string(const std::map<std::string, int>& map)
{
    std::string out = "{ ";

    if (!map.empty())
    {
        for (auto & it : map)
        {
            out += it.first + " : " + std::to_string(it.second) + ", ";
        }
    }

    out = out.substr(0, out.size()-2);
    out += " }";

    return out;
}

template <typename T>
static std::string vector_to_string(std::vector<T> v)
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

static std::map<std::string, int> analyzeStats()
{
    std::map<std::string, int> resultingDelays;

    std::map<std::string, int> stats =
        {
            {"created", 0},
            {"approved", 0},
            {"denied", 0},
            {"received", 0} //,
          //  {"total", 0}
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
                    resultingDelays[data1] = delay;
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

              file.close();
       // stats["total"]++;

    }

    std::cout << map_to_string(stats) << std::endl;
    double dataLoss = (1 - (double)stats["received"] / (double)stats["created"]) * 100;
    std::cout << "data loss: " << dataLoss << "%" << std::endl;

    return resultingDelays;
}

static auto createEmptySchedule(const std::map<int, int>& routersInfo)
{
    // routersInfo: map { routerIndex : portCount}
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

    return result;
}

static auto getRequestPeriods(const std::map<int, std::map<int, int>>& routeSpecs)
{
    // routeSpecs: map {period : map {routeID, routeTime} }

    // fetch request periods
    // period for each route
    std::vector<int> requestPeriods;

    for (auto &it : routeSpecs)
    {
        requestPeriods.push_back(it.first);
    }

    return requestPeriods;
}

static auto getAllRequestMoments(const std::map<int, std::map<int, int>>& routeSpecs, int simulationEndTime)
{
    // routeSpecs: map {period : map {routeID, routeTime} }

    // get all request moments
    std::set<int> allRequestTimeMoments;

    for (auto & it : routeSpecs)
    {
        int step = it.first;
        int requestTimeMoment = it.first;
        while (requestTimeMoment < simulationEndTime)
        {
            allRequestTimeMoments.insert(requestTimeMoment);
            requestTimeMoment += step;
        }
    }

    return allRequestTimeMoments;
}

static auto mapRequestsToTime(std::vector<int> requestPeriods, int simulationEndTime)
{
    // map simulation time to a send requests
    // time - requested route period (works as an ID)
    std::map<int, std::vector<int>> requestsByTime;

    int currentTime = 1;

    while (currentTime < simulationEndTime)
    {
        for (auto period : requestPeriods)
        {
            if (currentTime % period == 0)
            {
                requestsByTime[currentTime].push_back(period);
            }
        }
        currentTime++;
    }

    return requestsByTime;
}

static auto getRoutePrioritiesMap(const std::map<int, std::map<int, int>>& routeSpecs)
{
    // priority : lower -> better
    std::map<int, int> routePrioritiesMap;
    int priority = routeSpecs.size();

    for (const auto& route : routeSpecs)
    {
        routePrioritiesMap[priority] = route.first;
        priority--;
    }

    std::cout << map_to_string(routePrioritiesMap) << std::endl;
    return routePrioritiesMap;
}

static auto getMaxRouteDelays(const std::set<int>& allRequestTimeMoments,
                       const std::map<int, std::vector<int>>& requestsByTime,
                       const std::map<int, int>& routePrioritiesMap,
                       const std::map<int, std::map<int, int>>& routeSpecs)
{
    // calculate delays
    // min delay = request time + response time + buffering time + sending time
    // response time is regulated by schedule, further route has no delays
    // fetch maximum delay for each route
    std::map<int, int> maxRouteDelays;

        int delay = 0;

        for (auto priority : routePrioritiesMap)
        {
            maxRouteDelays[priority.second] = delay + routeSpecs.at(priority.second).begin()->second;
            delay += routeSpecs.at(priority.second).begin()->second;
        }

        return maxRouteDelays;

}

static auto calculateSchedules(const std::map<int, int>& routersInfo,
                        const std::map<int, std::map<int, int>>& routeSpecs,
                        const std::map<int, std::vector<std::pair<int, int>>>& routes,
                        int simulationEndTime)
{
    // routersInfo: map { routerIndex : portCount}
    // routeSpecs: map {period : map {routeID, routeTime} }
    // routes: map {routeID: vector { pair {router1, port1},..., pair {routerN, portM}}}

    std::map<int, std::map<int, std::vector<std::vector<int>>>> result;

    auto initialSchedule = createEmptySchedule(routersInfo);
    auto requestPeriods = getRequestPeriods(routeSpecs);
    auto allRequestTimeMoments = getAllRequestMoments(routeSpecs, simulationEndTime);
    auto mapRequestToTime = mapRequestsToTime(requestPeriods, simulationEndTime);
    auto routePrioritiesMap = getRoutePrioritiesMap(routeSpecs);
    auto maxRouteDelays = getMaxRouteDelays(allRequestTimeMoments, mapRequestToTime, routePrioritiesMap, routeSpecs);

    std::cout << map_to_string(maxRouteDelays) << std::endl;
        //exit(0);

    std::cout << allRequestTimeMoments.size() << std::endl;
    for (auto requestMoment : allRequestTimeMoments)
    {
        auto requestingRoutesForCurrentTime = mapRequestToTime[requestMoment];
        std::cout << vector_to_string(requestingRoutesForCurrentTime) << std::endl;
        for (auto priority : routePrioritiesMap)
        {
            // find top priority route
            // mark it not found
            int requestingRoute = -1;

            // if routes for current time include route with the top priority
            if (std::find(requestingRoutesForCurrentTime.begin(),
                          requestingRoutesForCurrentTime.end(),
                          priority.second) != requestingRoutesForCurrentTime.end())
            {
                // take this route
                requestingRoute = priority.second;
            }
            else
            {
                // continue search
                continue;
            }

            int maxDelay = maxRouteDelays[requestingRoute];
            int delta = maxDelay - routeSpecs.at(requestingRoute).begin()->second;
            //std::cout << "delta : " << delta << std::endl;
            int requestSendTime = 24; // microseconds
            int ms2mcs = 1000;
            int permittedIntervalStart = requestMoment*ms2mcs  + requestSendTime + delta;
            int permittedIntervalEnd = permittedIntervalStart + 100;
            int routeID = routeSpecs.at(requestingRoute).begin()->first;
            auto routePart = routes.at(routeID)[0];
            int routerIndex = routePart.first;
            int portIndex = routePart.second;
            //std::cout << "router : " << routerIndex << " port : " << portIndex << std::endl;

            std::vector<int> scheduleItem = {permittedIntervalStart, permittedIntervalEnd};
            result[routerIndex][portIndex].push_back(scheduleItem);
        }
    }

    return result;
}

static void showSchedule(std::map<int, std::map<int, std::vector<std::vector<int>>>> schedule)
{
    for (auto routerItem : schedule)
    {
        std::cout << "router" << routerItem.first << " : {";

        for (auto portItem : routerItem.second)
        {
            std::cout << " { port" << portItem.first << " : { ";

            for (auto scheduleItem : portItem.second)
            {
                std::cout << "{ " << scheduleItem[0] << "-" << scheduleItem[1] << " }, ";
            }

            std::cout << " } " << std::endl;
        }

        std::cout << " } \n" << std::endl;
    }
}

static void Connect(IConnectable* A, int portA, IConnectable* B, int portB)
{
    // Connects entity A via portA to an entityB portB
    A->Connect(B, portA);
    B->Connect(A, portB);
}

static void redirectOutputToFile(bool activate, const char* filename)
{
    if (activate)
    {
        freopen(filename,"w",stdout);
    }
}

static std::pair<int, int> getRouteDataFromMessage(std::string message)
{
    message = message.substr(message.find(", ")+2, message.size()-message.find(", ")+2);
    int senderAddress = std::stoi(message.substr(0, message.find(", ")));
    message = message.substr(message.find(", ")+2, message.size()-message.find(", ")+2);
    int receiverAddress = std::stoi(message.substr(0, message.find(", ")));
    return std::pair<int, int>(senderAddress, receiverAddress);
}
