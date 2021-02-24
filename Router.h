//
// Created by Dmitryi Petukhov on 20.02.2021.
//

#ifndef DATATRANSMISSIONPROTOCOL_ENHANCED_ROUTER_H
#define DATATRANSMISSIONPROTOCOL_ENHANCED_ROUTER_H

#include <iostream>
#include <string>
#include <vector>
#include <map>

/**
 * This is an implementation of a router.
 * It has two modes: async and sync.
 * Second mode implements schedule-wise routing.
 **/
class Device;
class Router {
private:
    std::string name; // router name
    int portsCount; // number of ports
    int maximumBufferSize; // maximum buffer size in bytes
    const double dataAtOneMicrosecond = 0.125;
    double currentTime; // current time to work with the schedule
    const std::string MESSAGE_START = "START"; // indicates start of packet
    const std::string MESSAGE_END = "END"; // indicates end of packet

    /**
     * data for each port
     * --------------------------------------------------------------------------
     * including:
     * - state
     * - data size left
     * - current byte progress
     */
    std::vector<std::map<std::string, double>> portsData;


    /**
     * buffer for each port
     * --------------------------------------------------------------------------
     * containing package data
     */

    std::vector<std::vector<std::string>> portBuffers;

    /**
     *  commutation table
     *  --------------------------------------------------------------------------
     *  each key specifies device-receiver name
     *  each value specifies port on which data will be sent
     */
    std::map<std::string, int> commutationTable;

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
        ReceivingInternal = 2,
        SendingInternal = 3,
        Sending = 4
    };

public:
    Router(std::string name, int portCount, int maxBufSize);
    void Run();
    void ConnectDevice(int portIndex, Device* device);
    void ConnectRouter(int portIndex, Router* router);
    void AddCommutationTable(std::map<std::string, int> commutationTable);
    void AddSchedule(std::vector<std::vector<int>> schedule);
};


#endif //DATATRANSMISSIONPROTOCOL_ENHANCED_ROUTER_H
