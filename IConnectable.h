//
// Created by dazzzed on 09.03.2021.
//

#ifndef DATATRANSMISSIONPROTOCOL_ENHANCED_ICONNECTABLE_H
#define DATATRANSMISSIONPROTOCOL_ENHANCED_ICONNECTABLE_H

#include <iostream>
#include <vector>

class IConnectable
{
public:
    virtual double GetAddress() = 0;
    virtual void Receive(int data, int port) = 0;
    virtual void Connect(IConnectable* device, int port) = 0;
    virtual std::map<int, IConnectable*> GetConnectedDevices() = 0;
};

#endif //DATATRANSMISSIONPROTOCOL_ENHANCED_ICONNECTABLE_H
