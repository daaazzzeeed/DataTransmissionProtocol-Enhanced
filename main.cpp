#include <iostream>
#include "device.h"
#include "utilities.h"

int main() {

    int currentTime = 0;
    int simulationTime = 200;

    Device device(0, 50);

    while (currentTime < simulationTime)
    {
        std::cout << "TIME: " << currentTime << "---------------------------" << std::endl;
        device.Run();
        currentTime++;
    }

    auto stats = device.GetStatistics();

    std::cout <<  "___________________________" << std::endl;
    std::cout << "Stats for " << device.GetName() << std::endl;
    std::cout <<  "–––––––––––––––––––––––––––" << std::endl;
    std::cout << "Sent: " << stats[0] << std::endl;
    std::cout << "Declined: " << stats[1] << std::endl;
    std::cout << "Dispatch cycles: " << stats[2] << std::endl;
    std::cout <<  "---------------------------" << std::endl;

    return 0;
}
