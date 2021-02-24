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

    // init ports data and buffers
    for (int i = 0; i < portsCount; i++)
    {
        std::map<std::string, double> item;
        item["state"] = States::Idle;
        item["dataSizeLeft"] = 0;
        item["currentByteProgress"] = 1.0;
        portsData.push_back(item);

        std::vector<std::string> portBuffer = {};
        portBuffers.push_back(portBuffer);

        currentTime = 0;
    }
}

void Router::Run()
{
    for (int i = 0; i < portsData.size(); i++)
    {
        int currentState = portsData[i]["state"];

        /**
         * ---------------------------------------------------
         *                  ***States***
         * ---------------------------------------------------
         * Possible conversions:
         * ---------------------------------------------------
         * 1.
         *  Idle -> Receiving -> Sending Internal -> Idle
         *                         (Optional)
         *
         * In this case port is receiving data and then
         * optionally redirects data to another port of
         * the router for a further handling.
         * ---------------------------------------------------
         * 2.
         *  Idle -> Receiving Internal -> Sending -> Idle
         *                              (Optional)
         *
         * In this case port is receiving data from another
         * port and then optionally sends data to a connected
         * device for a further handling.
         * ---------------------------------------------------
         */
        switch (currentState)
        {
            case States::Idle:
                if (portsData[i]["currentByteProgress"] < 1.0) // some data was already sent
                {
                    portsData[i]["state"] = States::Receiving; // achieve state 'Receiving'
                }
                break;
            case States::Sending:
                break;
            case States::SendingInternal:
                if (portsData[i]["dataSizeLeft"] > 0) // if package is not transmitted in full yet
                {
                    // at this time target port is considered to be free (as it is occupied by current transmission)
                    // fetch target port from a commutation table
                    int targetPort = commutationTable[portBuffers[i][1]];

                    // if current byte is not transmitted fully yet
                    if (portsData[i]["currentByteProgress"] > 0.0)
                    {
                        // transmit a batch of data
                        portsData[i]["currentByteProgress"] -= dataAtOneMicrosecond;
                    }
                    else
                    {
                        // update current byte progress
                        portsData[i]["currentByteProgress"] = 1.0;
                        // send current byte to a target port
                        int dataSizeLeft = portsData[i]["dataSizeLeft"];
                        int currentBufferSize = portBuffers[i].size();
                        portBuffers[targetPort].push_back(portBuffers[i][currentBufferSize - dataSizeLeft]);
                        portsData[i]["dataSizeLeft"]--;
                    }
                }
                else
                {
                    // package was fully transmitted
                    // clear current buffer
                    portBuffers[i] = {};
                    // clear current byte progress
                    portsData["currentByteProgress"] = 1.0;
                    // clear data size left
                    portsData["dataSizeLeft"] = 0.0;
                }

                break;
            case States::Receiving:
                if (portsData[i]["currentByteProgress"] == 1.0) // if byte was completely received
                {
                    // check that byte if it's end char
                    // if so then package is completely received
                    if (portBuffers[i].back() == MESSAGE_END)
                    {
                        // now package is completely received and can be inspected
                        // now check if transmission is allowed
                        // transmission is allowed if there is a record with current port index
                        // and target port in the schedule

                        // fetch target port from a commutation table
                        int targetPort = commutationTable[portBuffers[i][1]];

                        // find out if there is any schedule slot
                        bool found = false;

                        // traverse schedule
                        for (auto item : schedule)
                        {
                            // check if selected time slot is ok
                            if (item[0] > currentTime && currentTime > item[1] )
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
                            if (portsData[targetPort]["state"] == States::Idle)
                            {
                                // target port is free
                                portsData[i]["state"] = States::SendingInternal;
                            }
                            else
                            {
                                // target port is busy
                                // decline package
                                hasToBeDeclined = true;
                            }
                        }
                        else
                        {
                            // schedule slot not found
                            // decline package
                            hasToBeDeclined = true;
                        }

                        // if package must be declined for one of the 2 mentioned above reasons
                        if (hasToBeDeclined)
                        {
                            // set state to Idle and free port
                            portsData[i]["state"] = States::Idle;
                            // clear buffer
                            portBuffers[i] = {};
                            // reset currentByteProgress
                            portsData[i]["currentByteProgress"] = 1.0;
                        }
                    }
                }
                break;
            case ReceivingInternal:
                break;
            default:
                break;
        }
    }
}

void Router::ConnectDevice(int portIndex, Device* device)
{

}

void Router::ConnectRouter(int portIndex, Router* router)
{

}

void Router::AddCommutationTable(std::map<std::string, int> commutationTable)
{
    this->commutationTable = commutationTable;
}

void Router::AddSchedule(std::vector<std::vector<int>> schedule)
{
    this->schedule = schedule;
}