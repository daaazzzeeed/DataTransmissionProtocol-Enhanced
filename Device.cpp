//
// Created by dazzzed on 20.02.2021.
//

#include "Device.h"
#include "Router.h"

Device::Device(int address, int dispatchInterval)
{
    this->address = address;
    name = "device at 0x" + std::to_string(address);
    dataDispatchInterval = dispatchInterval;
    parentRouter = nullptr;
    timeToNextDispatch = dataDispatchInterval;
    deltaTime = 1;
    dataToSendRaw = {};
    dataToSendPrepared= {};
    inDataRaw = {};
    inDataProcessed = {};
    statistics = {0, 0, 0};
    currentTime = 0;
}

void Device::GenerateData()
{
    // generate data and write it to 'dataToSend' property

    // first remove all data
    dataToSendRaw.clear();

    // add header data and payload size
    std::cout << "*****" << payloadSize << "*****" << std::endl;
    dataToSendRaw = {
                MESSAGE_START,
                address,
                destAddress,
                payloadSize
    };

    // define max and min values in payload
    int max = 254;
    int min = 2;
    int range = max - min;

    // fill the package with payload
    for (int i = 0; i < payloadSize; i++)
    {
        dataToSendRaw.push_back(rand() % range + min);
    }

    // add ending character to a package
    dataToSendRaw.push_back(MESSAGE_END);

    // convert bytes to bits
    for (auto item : dataToSendRaw)
    {
        std::bitset<8> binaryItem(item);

        for (auto bitChar : binaryItem.to_string())
        {
            int bit = bitChar - '0';
            dataToSendPrepared.push_back(bit);
        }
    }

    //std::cout << vector_to_string(dataToSendPrepared) << std::endl;
}
void Device::SendData()
{
    // device has data to send
    if (dataToSendPrepared.size() > 0)
    {
        // fetch data
        int data = dataToSendPrepared[0];
        //std::cout << data << std::endl;
        // crop vector
        auto first = dataToSendPrepared.begin() + 1;
        auto last = dataToSendPrepared.end();

        dataToSendPrepared = std::vector(first, last);

        // find router's target port
        double targetPort = -1;
        // fetch connected devices of a parent router
        auto connected = parentRouter->GetConnectedDevices();

        // compare each device address to a current address

        for (auto it = connected.begin(); it != connected.end(); it++)
        {
            if (it->second->GetAddress() == address)
            {
                targetPort = it->first;
                break;
            }
        }

        // finally send data to target port
        parentRouter->Receive(data, targetPort);
    }
    else
    {
        std::cout << "Package " << vector_to_string(dataToSendRaw) << " was sent by " << name << std::endl;
        statistics[0]++;
        // device has no data to send
        dataToSendRaw.clear();
    }
}

void Device::Run() {
    //std::cout << name + " : TTND: " << timeToNextDispatch << std::endl;
    //std::cout << name + " : Data to send prepared left: " << dataToSendPrepared.size() << std::endl;

    if (dataDispatchInterval != -1)
    {
        if (timeToNextDispatch == 0 && dataToSendPrepared.empty())
        {
            statistics[2]++; // dispatch cycles incrementation

            if (dataToSendPrepared.empty())
            {
                GenerateData(); // initially generate new data
                std::cout << "Package " << vector_to_string(dataToSendRaw) << " was generated by " << name << std::endl;
                std::ofstream file;
                file.open("stats.txt", std::ios::app);
                file << currentTime << ": created " << vector_to_string(dataToSendRaw) << "\n";
                file.close();
            }

            SendData(); // send a batch of data to a parent router
            timeToNextDispatch = dataDispatchInterval-1;
        }
        else if (timeToNextDispatch != 0 && !dataToSendPrepared.empty() ||
                timeToNextDispatch == 0 && !dataToSendPrepared.empty())
        {
            SendData(); // send a batch of data to a parent router
            timeToNextDispatch -= deltaTime;
        }
        else
        {
            timeToNextDispatch -= deltaTime;
        }
    }

    // check the response data
    HandleInData();

    currentTime += deltaTime;
}

std::vector<int> Device::GetStatistics()
{
    return statistics;
}

void Device::Receive(int data, int port)
{
    inDataRaw.push_back(data);
}

void Device::ConnectTo(IConnectable* device, int port)
{
    parentRouter = device;
}

std::string Device::GetName()
{
    return name;
}

void Device::HandleInData()
{
    if (inDataRaw.size() == 8)
    {
        std::string binary = "";

        for (auto bit : inDataRaw)
        {
            binary += std::to_string(bit);
        }

        int processedByte = binaryStringToInt(binary);
        inDataProcessed.push_back(processedByte);
        inDataRaw.clear();

        if (inDataProcessed.back() == MESSAGE_END)
        {
            std::cout << "Package " << vector_to_string(inDataProcessed) << " was received " << " by " << name << std::endl;
            std::ofstream file;
            file.open("stats.txt", std::ios::app);
            file << currentTime << ": reached destination " << vector_to_string(inDataProcessed) << "\n";
            file.close();
            inDataProcessed.clear();
            statistics[1]++;
        }
    }
}

double Device::GetAddress()
{
    return address;
}


std::map<int, IConnectable*> Device::GetConnectedDevices()
{
    return {{0, parentRouter}};
}

void Device::SetDestination(double dest)
{
    destAddress = dest;
}


void Device::SetPayloadSize(double size)
{
    payloadSize = size;
}



