//
// Created by Dmitryi Petukhov on 20.02.2021.
//

#ifndef DATATRANSMISSIONPROTOCOL_ENHANCED_ROUTER_H
#define DATATRANSMISSIONPROTOCOL_ENHANCED_ROUTER_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "IConnectable.h"
#include <cassert>

/**
 * This is an implementation of a router.
 * --------------------------------------------------------------------------
 * It has two modes: async and sync.
 * Second mode implements schedule-wise routing.
 **/
class Device;
class Router : public IConnectable{
private:
    std::string name; // router name
    double address;
    int portsCount; // number of ports
    int maximumBufferSize; // maximum buffer size in bytes
    const double dataAtOneMicrosecond = 0.125; // bytes
    double currentTime; // current time to work with the schedule
    double deltaTime;
    const double MESSAGE_END = 0xFF; // indicates end of packet
    std::map<int, IConnectable*> ConnectedDevices;
    std::map<int, bool> PackageJustDelivered;

    /**
     * raw data for each port
     * --------------------------------------------------------------------------
     */
    std::map<int, std::vector<int>> portsRawData;

    /**
     * processed data for each port
     * --------------------------------------------------------------------------
     */
    std::map<int, std::vector<double>> portsProcessedData;

    /**
     * each port state
     * --------------------------------------------------------------------------
     */
    std::map<int, int> portStates;

    /**
     *  commutation table
     *  --------------------------------------------------------------------------
     *  each key specifies device-receiver address
     *  each value specifies port on which data will be sent
     */
    std::map<int, int> commutationTable;

    /**
     *  schedule
     *  --------------------------------------------------------------------------
     *  each item indicates that transmission is allowed in a given time interval
     *  each item has following structure:
     *  - time, start of the permitted interval
     *  - time, end of the permitted interval
     */
    std::vector<std::vector<int>> schedule;

    /**
     *  States
     *  --------------------------------------------------------------------------
     *  state of each port for a state-machine implementation
     */
    enum States
    {
        Idle = 0,
        Receiving = 1,
        Sending = 2,
        Waiting = 3
    };


    /**
     *  State names
     *  --------------------------------------------------------------------------
     *  state name of each state digital code
     */
    std::map<int, std::string> stateNames =
            {
                {0, "Idle"},
                {1, "Receiving"},
                {2, "Sending"},
                {3, "Waiting"}
            };

    /**
     *  Current connections
     *  --------------------------------------------------------------------------
     *  Contains pairs of ports linked to each other
     */

    std::map<int, int> Connections;


public:
    Router(std::string name, int portCount, int maxBufSize);
    void Receive(int data, int port=0);
    void ConnectTo(IConnectable* device, int port=0);
    void Run();
    void AddCommutationTable(std::map<int, int> commutationTable);
    void AddSchedule(std::vector<std::vector<int>> schedule);
    std::map<int, IConnectable*> GetConnectedDevices();
    double GetAddress();
};


#endif //DATATRANSMISSIONPROTOCOL_ENHANCED_ROUTER_H
