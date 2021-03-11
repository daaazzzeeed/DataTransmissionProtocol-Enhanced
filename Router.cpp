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
    Connections = {};
    currentTime = 0;
    deltaTime = 1;

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
         *       Idle -> Receiving -> Processing -> Idle
         *
         * In this case port is receiving data and then
         * optionally redirects data to another port of
         * the router for a further handling.
         * ---------------------------------------------------
         * 2.
         *          Idle -> Sending -> Idle
         *
         * In this case port is receiving data from another
         * port and then optionally sends data to a connected
         * device for a further handling.
         * ---------------------------------------------------
         */
        //std::cout << "State: " << currentState << std::endl;
       /* for (int k = 0; k < portsCount; k++)
        {
            std::cout << "Raw data [" << k << "] :" << vector_to_string(portsRawData[k]) << std::endl;
        }*/

        std::string binary = "";

        switch (currentState)
        {
            case States::Idle:
                if (portsRawData[i].size() > 0) // some data was already received
                {
                    portStates[i]= States::Receiving; // achieve state 'Receiving'
                }
                break;
            case States::Sending:
                if (portsRawData[i].size() > 0)
                {
                    if (!PackageJustDelivered[i])
                    {
                        //std::cout << portsRawData[i].size() << std::endl;
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

                    // free connected port
                    portStates[Connections[i]] = States::Idle;

                    std::cout << "---Package was sent from a port" << i << "---" << std::endl;

                }
                break;
            case States::Receiving:
                // port has 1 byte of raw data in it
                if (portsRawData[i].size() % 8 == 0)
                {
                   // std::cout << vector_to_string(portsRawData[i]) << std::endl;
                    // convert 8 bits to 1 byte
                    std::string bits = "";

                    // copy contents of byte to string
                    for (int j = 0; j < 8; j++)
                    {
                        bits += std::to_string(portsRawData[i][j]);
                    }

                   // std::cout << "bits : " << bits << std::endl;

                    // clear raw data
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
                  //  std::cout << "--------------" << portsProcessedData[i].back() << "--------------" << std::endl;
                    if (portsProcessedData[i].back() == MESSAGE_END)
                    {
                        std::cout << "---Package " << vector_to_string(portsProcessedData[i]) << " was received by port " << i << " of " << name << std::endl;
                        // now package is completely received and can be inspected
                        // now check if transmission is allowed
                        // transmission is allowed if there is a record with current port index
                        // and target port in the schedule

                        // fetch target port from a commutation table
                        int targetPort = commutationTable[portsProcessedData[i][2]];

                        // add new connection
                        Connections[targetPort] = i;

                       // std::cout << "Target port is " << targetPort  << std::endl;

                        // find out if there is any schedule slot
                        bool found = false;

                        // traverse schedule
                        for (auto item : schedule)
                        {
                            // check if selected time slot is ok
                           // std::cout << item[0] << ", " << currentTime << ", " << item[1] << std::endl;
                            if (item[0] < (int)currentTime && (int)currentTime < item[1] )
                            {
                                // update flag
                                found = true;
                                // escape from the loop
                                break;
                            }
                        }

                        // flag for a package cancellation
                        bool hasToBeDeclined = false;

                        // if schedule slot is found
                        if (found)
                        {
                            // check if target port is free (state == Idle)
                            if (portStates[targetPort] == States::Idle)
                            {
                                // target port is free
                                portStates[i] = States::Waiting;
                                portStates[targetPort] = States::Sending;
                                PackageJustDelivered[targetPort] = true;
                                portsProcessedData[targetPort] = portsProcessedData[i];
                                portsProcessedData[i].clear();

                                // fill raw data with bits of processed data
                                // reverse transformation of processed to raw data
                                for (auto dataItem : portsProcessedData[targetPort])
                                {
                                    std::bitset<8> processedDataAsBits(dataItem);
                                    std::string processedDataAsBitsToString = processedDataAsBits.to_string();

                                    for (auto bitChar : processedDataAsBitsToString)
                                    {
                                        int bit = bitChar - '0';
                                        portsRawData[targetPort].push_back(bit);
                                    }
                                }

                                std::cout << "---Package was sent to a port " << targetPort << "---" << std::endl;

                            }
                            else
                            {
                                // target port is busy
                                // decline package
                                hasToBeDeclined = true;
                                std::cout << "---Package will be declined!---" << std::endl;
                                std::cout << "---Target port is busy---" << std::endl;
                            }
                        }
                        else
                        {
                            // schedule slot not found
                            // decline package
                            hasToBeDeclined = true;
                            std::cout << "---Package will be declined!---" << std::endl;
                            std::cout << "---Schedule not found---" << std::endl;
                        }

                        // if package must be declined for one of the 2 mentioned above reasons
                        if (hasToBeDeclined)
                        {
                            // set state to Idle and free port
                            portStates[i] = States::Idle;
                            // clear buffer
                            portsProcessedData[i] = {};
                            // reset currentByteProgress
                            portsRawData[i] = {};
                        }
                    }
                }

                break;
            case States::Waiting:
                for (int pos = portsRawData[i].size()-1; pos > portsRawData[i].size()-1-8; pos--)
                {
                    binary += std::to_string(portsRawData[i][pos]);
                }

                if (binaryStringToInt(binary) == MESSAGE_END)
                {
                    std::cout << "---Package was received but can't be processed---\n---Port is busy---\n---Package will be declined---" << std::endl;
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

void Router::AddSchedule(std::vector<std::vector<int>> schedule)
{
    this->schedule = schedule;
}

void Router::Receive(int data, int port)
{
    // pass raw data to a buffer
    portsRawData[port].push_back(data);
}

void Router::ConnectTo(IConnectable* device, int port)
{
    // FIXME: NO two-sided connection
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
