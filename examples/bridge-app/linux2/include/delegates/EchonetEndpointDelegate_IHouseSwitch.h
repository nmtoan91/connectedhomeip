#pragma once
#ifndef EchonetEndpointDelegate_IHouseSwitch_H
#define EchonetEndpointDelegate_IHouseSwitch_H
#include <app/util/af-types.h>
#include "EchonetEndpointDelegate.h"
class EchonetEndpointDelegate_IHouseSwitch : public EchonetEndpointDelegate {
public:

    //EchonetEndpoint* ep;
    EchonetEndpointDelegate_IHouseSwitch(void* ep_):EchonetEndpointDelegate(ep_)
    {

    };
    int ReadProperty(chip::ClusterId clusterId,chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength);
    Protocols::InteractionModel::Status WriteProperty(chip::AttributeId attributeId, ClusterId clusterId, const EmberAfAttributeMetadata * attributeMetadata,uint8_t * buffer); 
    Protocols::InteractionModel::Status SetGET(unsigned char epc, vector<unsigned char> value);
};

#endif