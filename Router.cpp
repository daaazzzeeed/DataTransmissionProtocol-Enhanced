//
// Created by Dmitryi Petukhov on 20.02.2021.
//

#include "Router.h"
#include "Device.h"

Router::Router(std::string name, int portCount, int maxBufSize)
{
    this->name = name;
    portsCount = portCount;
    maximumBufferSize = maxBufSize;
    ConnectedDevices = {};
   // Connections = {};
    currentTime = 0;
    deltaTime = 1;
    responseResult = RESPONSE_BAD;

    // init ports data and buffers
    for (int i = 0; i < portsCount; i++)
    {
        int initialState = States::Idle;
        portStates[i] = initialState;

        std::vector<int> initialRawData = {};
        portsRawData[i] = initialRawData;

        std::vector<double> initialProcessedData = {};
        portsProcessedData[i] = initialProcessedData;
    }
}

void Router::Run()
{
    std::cout << "\n---------------" << name << "---------------" << std::endl;

    for (int i = 0; i < portsCount; i++)
    {
        int currentState = portStates[i];

        std::cout << "---------------PORT [" << i << "]---------------" << std::endl;
        std::cout << "State: " << stateNames[currentState] << std::endl;

        /**
         * ---------------------------------------------------
         *                  ***States***
         * ---------------------------------------------------
         * Possible conversions:
         * ---------------------------------------------------
         * 1.
         *       Idle -> Receiving -> Responding -> Idle
         *
         * In this case port is receiving data and then
         * optionally redirects data to another port of
         * the router for a further handling.
         * ---------------------------------------------------
         * 2.
         *       Idle -> Receiving -> Idle
         *
         * In this case port is receiving data and then
         * redirects data to another port of
         * the router for a further handling.
         * ---------------------------------------------------
         * 3.
         *          Idle -> Sending -> Idle
         *
         * In this case port is receiving data from another
         * port and then sends data to a connected
         * device for a further handling.
         * ---------------------------------------------------
         *
         */

        std::string binary = "";

        switch (currentState)
        {
            case States::Idle:
                if (portsRawData[i].size() > 0) // some data was already received
                {
                    portStates[i] = States::Receiving; // achieve state 'Receiving'
                }
                break;
            case States::Sending:
                if (portsRawData[i].size() > 0)
                {
                    if (!PackageJustDelivered[i])
                    {
                        int data = portsRawData[i][0];
                        auto first = portsRawData[i].begin() + 1;
                        auto last = portsRawData[i].end();
                        portsRawData[i] = std::vector(first, last);
                        ConnectedDevices[i]->Receive(data);
                    }
                    else
                    {
                        PackageJustDelivered[i] = false;
                    }
                }
                else
                {
                    // free current port
                    portStates[i] = States::Idle;

                    std::cout << "---Package was sent from a port" << i << "---" << std::endl;

                }
                break;
            case States::Receiving:
                responseResult = RESPONSE_BAD;
                // port has 1 byte of raw data in it
                if (portsRawData[i].size() % 8 == 0 && portsRawData[i].size() > 0)
                {
                    // convert 8 bits to 1 byte
                    std::string bits = "";

                    // copy contents of byte to string
                    for (int j = 0; j < 8; j++)
                    {
                        bits += std::to_string(portsRawData[i][j]);
                    }

                    // clear raw data or take first byte
                    if (portsRawData[i].size() > 8)
                    {
                        portsRawData[i] = std::vector(portsRawData[i].begin() + 8, portsRawData[i].end());
                    }
                    else
                    {
                        portsRawData[i].clear();
                    }

                    portsProcessedData[i].push_back(binaryStringToInt(bits));

                    // check that byte if it's end char
                    // if so then package is completely received
                    if (portsProcessedData[i].back() == MESSAGE_END)
                    {
                        bool isRequest = false;

                        for (auto byte : portsProcessedData[i])
                        {
                            if (byte == REQUEST)
                            {
                                isRequest = true;
                                break;
                            }
                        }

                        if (!isRequest)
                        {
                            // got accepted message from device

                            std::cout << "---Package " << vector_to_string(portsProcessedData[i])
                                      << " was received by port " << i << " of " << name << std::endl;
                            // now package is completely received and can be inspected
                            // now check if transmission is allowed
                            // transmission is allowed if there is a record with current port index
                            // and target port in the schedule

                            // fetch target port from a commutation table
                            int targetPort = commutationTable[portsProcessedData[i][2]];

                            // check if target port is free (state == Idle)
                            if (portStates[targetPort] == States::Idle) {
                                // target port is free
                                portStates[i] = States::Idle;
                                portStates[targetPort] = States::Sending;
                                PackageJustDelivered[targetPort] = true;
                                portsProcessedData[targetPort] = portsProcessedData[i];
                                portsProcessedData[i].clear();

                                // fill raw data with bits of processed data
                                // reverse transformation of processed to raw data
                                for (auto dataItem : portsProcessedData[targetPort]) {
                                    std::bitset<8> processedDataAsBits(dataItem);
                                    std::string processedDataAsBitsToString = processedDataAsBits.to_string();

                                    for (auto bitChar : processedDataAsBitsToString) {
                                        int bit = bitChar - '0';
                                        portsRawData[targetPort].push_back(bit);
                                    }
                                }

                                std::cout << "---Package was sent to a port " << targetPort << "---" << std::endl;

                            } else {
                                // target port is busy
                                // decline package
                                // set state to Idle and free port
                                portStates[i] = States::Idle;
                                // clear buffer
                                portsProcessedData[i] = {};
                                // reset currentByteProgress
                                portsRawData[i] = {};

                                std::cout << "---Schedule Error!" << std::endl;
                                std::cout << "Package will be declined!" << std::endl;
                                std::cout << "Target port is busy---" << std::endl;
                            }
                        }
                        else
                        {
                            // Request
                            portStates[i] = States::Waiting;
                        }
                    }

                }
                break;
            case Waiting:
                // find out if there is any schedule slot
                // traverse schedule
                for (auto item : schedule[i])
                {
                    // check if selected time slot is ok
                    if (item[0] < currentTime && currentTime < item[1] )
                    {
                        responseResult = RESPONSE_OK;
                        // escape from the loop
                        break;
                    }
                }

                if (responseResult == RESPONSE_OK)
                {
                    portStates[i] = States::Responding;
                    // clear processed data
                    portsProcessedData[i].clear();
                    std::cout << "approved"<< std::endl;
                }
                else
                {
                    std::cout << "denied"<< std::endl;
                }
                break;
            case States::Responding:
                if (responseRaw[i].empty())
                {
                    generateResponseForPort(i);
                }

                if (!sendResponse(i))
                {
                    portStates[i] = States::Idle;
                    responseRaw[i].clear();
                }
                break;
            default:
                std::cout << "---State Error---\n---Wrong state was set!---" << std::endl;
                break;
        }

    }

    currentTime += deltaTime;
}


void Router::AddCommutationTable(std::map<int, int> commutationTable)
{
    this->commutationTable = commutationTable;
}

void Router::AddSchedule(int port, std::vector<std::vector<int>> schedule)
{
    this->schedule[port] = schedule;
}

void Router::Receive(int data, int port)
{
    // pass raw data to a buffer
    portsRawData[port].push_back(data);
}

void Router::Connect(IConnectable* device, int port)
{
    ConnectedDevices[port] = device;
}

std::map<int, IConnectable*> Router::GetConnectedDevices()
{
    return ConnectedDevices;
}

double Router::GetAddress()
{
    return address;
}


bool Router::sendResponse(int port)
{
    // device has data to send
    if (responsePrepared[port].size() > 0)
    {
        // fetch data
        int data = responsePrepared[port][0];
        // crop vector
        auto first = responsePrepared[port].begin() + 1;
        auto last = responsePrepared[port].end();

        responsePrepared[port] = std::vector(first, last);

        // fetch connected device
        auto connected = GetConnectedDevices()[port];

        // finally send data to target port
        connected->Receive(data);
        return true;
    }
    else
    {
        return false;
    }
}

void Router::generateResponseForPort(int port)
{
    // first remove all data
    responseRaw[port].clear();

    // add header data and payload size

    responseRaw[port] = {
            MESSAGE_START,
            responseResult,
            MESSAGE_END
    };

    // convert bytes to bits
    for (auto item : responseRaw[port])
    {
        std::bitset<8> binaryItem(item);

        for (auto bitChar : binaryItem.to_string())
        {
            int bit = bitChar - '0';
            responsePrepared[port].push_back(bit);
        }
    }
}
