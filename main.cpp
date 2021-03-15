#include <iostream>
#include <vector>
#include "device.h"
#include "router.h"
#include "utilities.h"
#include <fstream>

int main() {

    // clear stats file
    std::ofstream file;
    file.open("stats.txt", std::ios::trunc);
    file.close();

    int currentTime = 0;
    int MSec2McSec = 1000;
    int durationInMSecs = 50;
    int simulationTime = durationInMSecs*MSec2McSec; // microseconds

    // create networking entities

    Device* device2 = new Device(2, 32*MSec2McSec); // microseconds
    Device* device3 = new Device(3, -1); // no dispatches
    Device* device4 = new Device(4, 12*MSec2McSec); // microseconds
    Device* device5 = new Device(5, -1); // no dispatches
    Device* device6 = new Device(6, 8*MSec2McSec); // microseconds
    Device* device7 = new Device(7, -1); // no dispatches

    std::vector<Device*> devices = {device2, device3, device4, device5, device6, device7};

    Router* router1 = new Router("router1", 4, 1024);
    Router* router2 = new Router("router2", 4, 1024);

    std::vector<Router*> routers = {router1, router2};

    // setup
    // all packages with dest addr 3 will be forwarded to a port 1
    for (auto router : routers)
    {
        router->AddCommutationTable({{3, 1}});
    }

    // set commutation tables
    routers[0]->AddCommutationTable({{4, 2}, {6, 3}});
    routers[1]->AddCommutationTable({{5, 2}, {7, 3}});

    // set schedules
    routers[0]->AddSchedule(0, {{0, 100}, {900, 1000}});
    routers[1]->AddSchedule(0, {{100, 200}});
    routers[2]->AddSchedule(0, {{100, 300}});

    // build network
    routers[0]->ConnectTo(devices[0], 0);
    devices[1]->ConnectTo(routers[0], 0);

    routers[0]->ConnectTo(devices[2], 2);
    devices[2]->ConnectTo(routers[0], 0);

    routers[0]->ConnectTo(devices[4], 3);
    devices[4]->ConnectTo(routers[0], 0);

    routers[1]->ConnectTo(devices[3], 2);
    devices[3]->ConnectTo(routers[1], 0);

    routers[1]->ConnectTo(devices[5], 3);
    devices[5]->ConnectTo(routers[1], 0);

    routers[1]->ConnectTo(devices[1], 1);
    devices[1]->ConnectTo(routers[1], 0);


    device2->SetDestination(3);
    device4->SetDestination(5);
    device6->SetDestination(7);

    double packageSize = 64;
    double systemData = 5;
    double payloadSize = packageSize - systemData;
    device2->SetPayloadSize(payloadSize); // 64 bytes
    device4->SetPayloadSize(payloadSize/2); // 32 bytes
    device6->SetPayloadSize(payloadSize/2); // 32 bytes

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
            router->Run();
        }

        currentTime++;
    }

    std::cout.clear();

    analyzeStats();

    return 0;
}
