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
    int simulationTime = 1200; // microseconds

    // create networking entities

    Device* device1 = new Device(2, 50); // microseconds
    Device* device2 = new Device(3, -1); // no dispatches
   // Device* device3 = new Device(4, 1112); // microseconds


    std::vector<Device*> devices = {device1, device2};//, device3};

    Router* router1 = new Router("router1", 2, 1024);
    Router* router2 = new Router("router2", 2, 1024);
    Router* router3 = new Router("router3", 2, 1024);

    std::vector<Router*> routers = {router1, router2, router3};

    // setup
    // all package with dest addr 3 will be forwarded to a port 1
    for (auto router : routers)
    {
        router->AddCommutationTable({{3, 1}});
    }

    routers[0]->AddSchedule(0, {{0, 100}, {900, 1000}});
    routers[1]->AddSchedule(0, {{100, 200}});
    routers[2]->AddSchedule(0, {{100, 300}});

    // build network
    routers[0]->ConnectTo(device1, 0);
    device1->ConnectTo(routers[0], 0);

    routers[2]->ConnectTo(device2, 1);
    device2->ConnectTo(routers[2], 1);

   // device3->ConnectTo(routers[1], 2);
   // routers[1]->ConnectTo(device2, 0);

    routers[0]->ConnectTo(routers[1], 1);
    routers[1]->ConnectTo(routers[0], 0);

    routers[1]->ConnectTo(routers[2], 1);
    routers[2]->ConnectTo(routers[1], 0);

    device1->SetDestination(3);
    //device3->SetDestination(3);

    double packageSize = 6;
    double systemData = 5;
    double payloadSize = packageSize - systemData;
    device1->SetPayloadSize(payloadSize); // 64 bytes
   // device3->SetPayloadSize(payloadSize); // 64 bytes

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
