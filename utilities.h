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


    for (auto timeMoment : allRequestTimeMoments)
    {
        // get all requests for a current time
        auto requestsForCurrentTimeMoment = requestsByTime.at(timeMoment);

        // cumulative delay
        int totalDelay = 0;

        // iterate through priorities from highest to lowest

        for (auto &priority : routePrioritiesMap)
        {
            // if there is a request with a specific priority for a current time moment
            if (std::find(requestsForCurrentTimeMoment.begin(),
                          requestsForCurrentTimeMoment.end(),
                          priority.second) != requestsForCurrentTimeMoment.end())
            {
                // fetch route's own delay
                int ownDelay = routeSpecs.at(priority.second).begin()->second;

                // accumulate own delays to form total delay
                totalDelay += ownDelay;

                // if max delays map contains key (route period)
                if (maxRouteDelays.find(priority.second) != maxRouteDelays.end())
                {
                    if (totalDelay > maxRouteDelays[priority.second])
                    {
                        maxRouteDelays[priority.second] = totalDelay;
                    }
                }
                else
                {
                    maxRouteDelays[priority.second] = totalDelay;
                }
            }
        }
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

    for (auto requestMoment : allRequestTimeMoments)
    {
        auto requestingRoutesForCurrentTime = mapRequestToTime[requestMoment];
        for (auto requestingRoute : requestingRoutesForCurrentTime)
        {
            int maxDelay = maxRouteDelays[requestingRoute];
            int delta = maxDelay - routeSpecs.at(requestingRoute).begin()->second;
            std::cout << "delta : " << delta << std::endl;
            int requestSendTime = 24; // microseconds
            int ms2mcs = 1000;
            int permittedIntervalStart = requestMoment * ms2mcs + requestSendTime + delta;
            int permittedIntervalEnd = permittedIntervalStart + 100;
            int routeID = routeSpecs.at(requestingRoute).begin()->first;
            auto routePart = routes.at(routeID)[0];
            int routerIndex = routePart.first;
            int portIndex = routePart.second;
            std::cout << "router : " << routerIndex << " port : " << portIndex << std::endl;

            std::vector<int> scheduleItem = {permittedIntervalStart, permittedIntervalEnd};
            result[routerIndex][portIndex].push_back(scheduleItem);
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