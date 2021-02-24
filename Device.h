//
// Created by Dmitryi Petukhov on 20.02.2021.
//

#ifndef DATATRANSMISSIONPROTOCOL_ENHANCED_DEVICE_H
#define DATATRANSMISSIONPROTOCOL_ENHANCED_DEVICE_H

#include <iostream>
#include <vector>
#include <string>
#include "utilities.h"

/**
 * This is an implementation of a device.
 * Device can only actively send data with a specific frequency and receive dispatch status from a parent router.
 **/

class Router;
class Device {
private:
    std::string name; // device name
    int address; // device physical address
    int dataDispatchInterval; // device sends data once in this interval
    int timeToNextDispatch; // time left until next dispatch
    int deltaTime; // time interval between iterations
    Router* parentRouter; // router to which the device is connected to
    int parentRouterPortIndex; // parent router's port index device is streaming data to
    std::vector<std::string> dataToSend; // current data transmitting to a router in bytes
    int dataSizeLeftToSend; // current size of data transmitting to a router in bytes
    double currentByteTransmissionProgress; // indicates current byte transmission progress percentage
    const std::string MESSAGE_START = "START"; // indicates start of packet
    const std::string MESSAGE_END = "END"; // indicates end of packet
    std::vector<std::string> responseData; // buffer for a router's responses about dispatch statuses
    const int linkSpeed = 1000000; // 1 MBit/s
    const double dataAtOneMicrosecond = 0.125; // 0.125 Byte or 1 Bit
    const int byteStreamingTime = 8; // 8 microseconds
    std::vector<int> statistics; // stores stats data about packages [sent, declined, dispatch cycles]
    void ConnectToRouter(Router* router, int portIndex); // adds connection to a parent router
    void ReceiveData(); // receives response data
    void SendData(); // sends data to a parent router
    void GenerateData(int payloadSize=1, int destination=0); // generates new data and writes it to a dataToSend property

public:
    Device(int address, int dispatchInterval); // constructor
    std::string GetName(); // returns device name
    void Run(); // main loop with an event handling
    std::vector<int> GetStatistics(); // returns vector of statistics
};


#endif //DATATRANSMISSIONPROTOCOL_ENHANCED_DEVICE_H
