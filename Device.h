//
// Created by Dmitryi Petukhov on 20.02.2021.
//

#ifndef DATATRANSMISSIONPROTOCOL_ENHANCED_DEVICE_H
#define DATATRANSMISSIONPROTOCOL_ENHANCED_DEVICE_H

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include "utilities.h"
#include "IConnectable.h"
#include <cassert>
/**
 * This is an implementation of a device.
 * Device can only actively send data with a specific frequency and receive dispatch status from a parent router.
 **/

class Router;

class Device : public IConnectable {
private:
    std::string name; // device name
    int state;
    double address; // device physical address
    double destAddress; // target device's physical address
    double payloadSize; // size of each message in bytes
    int dataDispatchInterval; // device sends data once in this interval
    int timeToNextDispatch; // time left until next dispatch
    int deltaTime; // time interval between iterations
    double currentTime;
    IConnectable* parentRouter; // router to which the device is connected to
    std::vector<double> dataToSendRaw; // current data transmitting to a router in bytes
    std::vector<double> requestRaw; // current request transmitting to a router in bytes
    std::vector<int> dataToSendPrepared; // current data transmitting to a router in bits
    std::vector<int> requestPrepared; // current request transmitting to a router in bits
    const double MESSAGE_START = 0x1; // indicates start of packet
    const double MESSAGE_END = 0xFF; // indicates end of packet
    const double REQUEST = 0xFE; // indicates request
    const double RESPONSE_OK = 0xFD; // indicates granted request
    const double RESPONSE_BAD = 0xFC; // indicates denied request

    std::vector<int> inDataRaw; // buffer for a router's responses about dispatch statuses
    std::vector<double> inDataProcessed; // buffer for a router's responses about dispatch statuses
    std::vector<int> statistics; // stores stats data about packages [sent, received, dispatch cycles]
    bool SendData(); // sends data to a parent router
    int CheckInData(); // checks in data
    void GenerateData(); // generates new data and writes it to a dataToSendRaw and dataToSendPrepared properties
    void GenerateRequest(); // generates new request and writes it to a requestRaw and requestPrepared properties
    bool SendRequest(); // sends request, true - data sent, false - no data to send
    void PrepareRawData();

    enum states
    {
        Idle = 0,
        SendingRequest = 1,
        AwaitingResponse = 2,
        SendingData = 3,
        Receiving = 4
    };

public:
    Device(int address, int dispatchInterval); // constructor
    std::string GetName(); // returns device name
    void Run(); // main loop with an event handling
    std::vector<int> GetStatistics(); // returns vector of statistics
    void Receive(int data, int port) override;
    void Connect(IConnectable* device, int port) override;
    double GetAddress() override;
    std::map<int, IConnectable*> GetConnectedDevices() override;
    void SetDestination(double dest);
    void SetPayloadSize(double size);
};


#endif //DATATRANSMISSIONPROTOCOL_ENHANCED_DEVICE_H
