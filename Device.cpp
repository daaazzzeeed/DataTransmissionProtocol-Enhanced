//
// Created by dazzzed on 20.02.2021.
//

#include "Device.h"
#include "Router.h"

Device::Device(int address, int dispatchInterval)
{
    this->address = address;
    name = "device0" + address;
    dataDispatchInterval = dispatchInterval;
    parentRouter = nullptr;
    parentRouterPortIndex = -1;
    timeToNextDispatch = dispatchInterval;
    deltaTime = 1;
    dataToSend = {};
    dataSizeLeftToSend = -1;
    currentByteTransmissionProgress = 1;
    responseData = {};
    statistics = {0, 0, 0};
}

void Device::GenerateData(int payloadSize, int destination)
{
    // generate data and write it to 'dataToSend' property

    // first remove all data
    dataToSend.clear();

    // add header data and payload size
    dataToSend = {
                MESSAGE_START,
                std::to_string(address),
                std::to_string(destination),
                std::to_string(payloadSize)
    };

    // define max and min values in payload
    int max = 1000;
    int min = 1;
    int range = max - min;

    // fill the package with payload
    for (int i = 0; i < payloadSize; i++)
    {
        dataToSend.push_back(std::to_string(rand() % range + min));
    }

    // add ending character to a package
    dataToSend.push_back(MESSAGE_END);
    // update dataSizeLeftToSend
    dataSizeLeftToSend = dataToSend.size();
    currentByteTransmissionProgress = 1; // currently byte is stored fully in device
}
void Device::SendData()
{
    // device has data to send
    if (dataSizeLeftToSend > 0)
    {
        if (currentByteTransmissionProgress == 0)
        {
            // check if current byte is completely transmitted
             currentByteTransmissionProgress = 1;

             std::cout << dataToSend[dataToSend.size() - dataSizeLeftToSend] << std::endl;
             dataSizeLeftToSend--;
        }
        else
        {
            // send a batch of data to a parent router
            currentByteTransmissionProgress -= dataAtOneMicrosecond;
        }
    }
    else
    {
        if (!dataToSend.empty())
        {
            std::cout << "Package was sent" << std::endl;
            statistics[0]++;
            // device has no data to send
            dataToSend.clear();
            currentByteTransmissionProgress = 1.0;
            dataSizeLeftToSend = 0;
        }
    }
}

void Device::Run() {
    if (timeToNextDispatch == 0)
    {
        statistics[2]++; // dispatch cycles incrementation

        if (dataToSend.empty())
        {
            GenerateData(); // initially generate new data
            std::cout << vector_to_string(dataToSend) << std::endl;
        }

        SendData(); // send a batch of data to a parent router
        timeToNextDispatch = dataDispatchInterval-1;
    }
    else
    {
        SendData(); // send a batch of data to a parent router
        timeToNextDispatch -= deltaTime;
    }

    // check the response data
    ReceiveData();
}

std::vector<int> Device::GetStatistics()
{
    return statistics;
}

void Device::ReceiveData()
{
    // receive response data
    if (responseData.back() == MESSAGE_END)
    {
        // handle the response
        // then clear the response data
        responseData.clear();
    }
}

void Device::ConnectToRouter(Router *router, int portIndex)
{
    parentRouter = router;
    parentRouterPortIndex = portIndex;
}

std::string Device::GetName()
{
    return name;
}