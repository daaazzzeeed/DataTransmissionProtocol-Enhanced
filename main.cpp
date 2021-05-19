#include <iostream>
#include <vector>
#include "Device.h"
#include "Router.h"
#include "utilities.h"

int main() {

    redirectOutputToFile(false, "out.txt");

    // auto schedule parameters
    std::map<int, int> routersInfo = {{1, 4}, {2, 4}};
    std::map<int, std::map<int, int>> routeSpecs =
            {
                    {15, std::map<int, int>{{1, 816}}},
                    {30, std::map<int, int>{{2, 816}}},
                    {45, std::map<int, int>{{3, 1584}}}
            };
    std::map<int, std::vector<std::pair<int, int>>> routes =
            {
                    {1, {{1, 3}, {2, 0, }, {2, 3}}},
                    {2, {{1, 2}, {2, 0, }, {2, 2}}},
                    {3, {{1, 0}, {2, 0, }, {2, 1}}}
            };


    // clear stats file
    std::ofstream file;
    file.open("stats.txt", std::ios::trunc);
    file.close();

		const int SYNC = 0;
		const int ASYNC = 1;
    int currentTime = 0;
    int MSec2McSec = 1000;
    double durationInMSecs = 100;
    int simulationTime = durationInMSecs*MSec2McSec; // microseconds

    // create networking entities
    auto device2 = new Device(2, 45*MSec2McSec); // microseconds
    auto device3 = new Device(3, -1); // no dispatches
    auto device4 = new Device(4, 30*MSec2McSec); // microseconds
    auto device5 = new Device(5, -1); // no dispatches
    auto device6 = new Device(6, 15*MSec2McSec); // microseconds
    auto device7 = new Device(7, -1); // no dispatches

    auto router1 = new Router("router1", 4, 1024);
    auto router2 = new Router("router2", 4, 1024);

    std::vector<Router*> routers = {router1, router2};

    // setup
    // all packages with dest addr 3 will be forwarded to a port 1
    for (auto router : routers)
    {
        router->AddCommutationTable(3, 1);
    }

    // set commutation tables
    routers[0]->AddCommutationTable(5, 1);
    routers[0]->AddCommutationTable(7, 1);
    routers[1]->AddCommutationTable(5, 2);
    routers[1]->AddCommutationTable(7, 3);

    // create schedules
    auto schedule = calculateSchedules(routersInfo, routeSpecs, routes, durationInMSecs);

    // show schedules
    showSchedule(schedule);

    // set schedules

    for (auto routerItem : schedule)
    {
        for (auto portItem : routerItem.second)
        {
            routers[routerItem.first-1]->AddSchedule(portItem.first, portItem.second);
        }
    }

    //router1->AddSchedule(3, {{8024, 8030}});
    // router2->AddSchedule(0, {{8600,8800}});

    // build network
    Connect(router1, 1, router2, 0);
    Connect(router1, 0, device2, 0);
    Connect(router1, 2, device4, 0);
    Connect(router1, 3, device6, 0);
    Connect(router2, 2, device5, 0);
    Connect(router2, 3, device7, 0);
    Connect(router2, 1, device3, 0);

    // set destinations for source devices
    device2->SetDestination(3);
    device4->SetDestination(5);
    device6->SetDestination(7);

    // set payload sizes for source devices
    double packageSize = 64;
    double systemData = 5;
    double payloadSize = packageSize - systemData;
    device2->SetPayloadSize(payloadSize); // 64 bytes
    device4->SetPayloadSize(((int)packageSize/2) - systemData); // 32 bytes
    device6->SetPayloadSize(((int)packageSize/2) - systemData); // 32 bytes

    // add devices to a vector for a group actions
    auto devices = {device2, device3, device4, device5, device6, device7};

    //std::cout.setstate(std::ios_base::failbit);
    while (currentTime < simulationTime)
    {
        std::cout << "---------------------------TIME: " << currentTime << "---------------------------" << std::endl;

        for (auto device : devices)
        {
            device->Run();
        }

        for (auto router : routers)
        {
            router->Run(SYNC);
        }

        currentTime++;
    }

    std::cout.clear();

    auto delays = analyzeStats();

    std::map<std::string, std::vector<int>> delaysForDataMap;

    for (auto &item : delays)
    {
        auto routeData = getRouteDataFromMessage(item.first);
        auto routeDataBaked = std::to_string(routeData.first) + "-" + std::to_string(routeData.second);
        delaysForDataMap[routeDataBaked].push_back(item.second);
    }

    for (auto &item : delaysForDataMap)
    {
        std::cout << item.first << " : " << vector_to_string(item.second) << std::endl;
    }

    return 0;
}
