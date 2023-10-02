#ifndef EchonetEndpointDelegate_IHouseWindowCovering_CPP
#define EchonetEndpointDelegate_IHouseWindowCovering_CPP
#include "EchonetEndpointDelegate_IHouseWindowCovering.h"
#include "../../EchonetEndpoint.h"

EmberAfStatus EchonetEndpointDelegate_IHouseWindowCovering::WriteProperty(chip::AttributeId attributeId, ClusterId clusterId, const EmberAfAttributeMetadata * attributeMetadata,uint8_t * buffer)
{
    //EchonetEndpoint*ep = (EchonetEndpoint*)myEchonetEndpointpointer;
    printf("\n\n\n\n\n\n\n\n\n\n WriteProperty_iHouseSwitch clusterId=0x%08x, attributeId=0x%08x \n\n\n\n\n\n\n",clusterId, attributeId );
    
    

    return EMBER_ZCL_STATUS_SUCCESS;
}
int EchonetEndpointDelegate_IHouseWindowCovering::ReadProperty(chip::ClusterId clusterId,chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength)
{
    EchonetEndpoint*ep = (EchonetEndpoint*)myEchonetEndpointpointer;
    printf("\n\n\n\n\n\n\n\n\n\n ReadProperty_iHouseSwitch clusterId=0x%08x, attributeId=0x%08x \n\n\n\n\n\n\n",clusterId, attributeId );

    ep->currentWaitingPropertyId = 0x80;
    shared_ptr<DeviceObject> device_ = ep->GetDeviceObject(0x05fd, 01);
    if(device_ == nullptr) return 1;

    device_->get().reqGetProperty(ep->currentWaitingPropertyId).send();
    for(int i =0; i < 500; i++)
    {
        if(i%50==0)
        printf("[ReadProperty ihouse] Waiting for response 0x%02x (sleeping 50ms)\n",ep->currentWaitingPropertyId);
        if(ep->currentWaitingPropertyId  == 0xFF) 
        {
            break;
        }
        usleep(10 * 1000); 
    }
    vector<unsigned char> echonetValue = ep->GET_properties[0x80].echonetValue;
    if(echonetValue.size()>0)
    {
    if(echonetValue[0] == 0x30)//open
        buffer[0] = 100;
    else if(echonetValue[0] == 0x31) //close
        buffer[0] = 0;
    else buffer[0] = 50;
    } else buffer[0] = 50;

    return 0;
}
EmberAfStatus EchonetEndpointDelegate_IHouseWindowCovering::SetGET(unsigned char epc, vector<unsigned char> value)
{
    EchonetEndpoint*ep = (EchonetEndpoint*)myEchonetEndpointpointer;

    printf("\n\n\n AAAAAAAAAAAAAAAAAAAAAA 0x%02x 0x%02x \n\n\n",  epc, value[0]);
    EmberAfStatus errorCode = EMBER_ZCL_STATUS_SUCCESS;
    
    shared_ptr<DeviceObject> device_;
    if (value[0] == 0x30 ||value[0] == 0x42) //close = 0x42
    {
        device_ = ep->GetDeviceObject(0x05fd,2 );
    }
    else if (value[0] == 0x31 || value[0] == 0x41) // open = 0x41
    {
        device_ = ep->GetDeviceObject(0x05fd,1 );
    }
    else 
    {
        printf("\n\n\n [ERROR][IHOUSE] Cannot find the case for epc=0x%02x val=0x%02x \n\n\n",  epc, value[0]);
        return EMBER_ZCL_STATUS_DEPRECATED83;
    }
    if(device_ == nullptr)
    {
        printf("\n\n\n\n\n[ERROR] Cannot find echo device with address %s  \n\n\nn\n\n", ep->address.c_str());
    }
    value = {0x30};
    epc = 0x80;
    EchoFrame eFrame = device_.get()->set(true).reqSetProperty(epc,value).send();
    printf("____ EchonetEndpoint::SetGET epc=0x%02x  value=%s tid=%d\n",epc, ConvertEchonetValueToHexString(value).c_str(), eFrame.getTID());
    //test only

    ep->currentWaitingTID = eFrame.getTID();
    TimeManager::GetInstance()->RecordTime(TimeRecordType::START_SEND_WRITE_COMMAND_TO_ECHONET_DEVICE, ep->echoClassCode,ep->instanceCode,epc ,ep->eoj_pair.second>>8,ep->eoj_pair.second%256, ConvertToUnsignedInt(value) );

    if (STATIC_CONFIG_IS_FAST_COMISSION_WRITE== false)
    {
        for(int i =0; i < 500; i++)
        {
            if(ep->currentWaitingTID  == 0xFFFF) 
            {
                TimeManager::GetInstance()->RecordTime(TimeRecordType::END_SEND_WRITE_COMMAND_TO_ECHONET_DEVICE, ep->echoClassCode,ep->instanceCode,epc ,ep->eoj_pair.second>>8,ep->eoj_pair.second%256, ConvertToUnsignedInt(value) );
                break;
            }
            usleep(10 * 1000); //sleep 10 miliseconds
        }
        //if(ep->currentWaitingTID != 0xFFFF) errorCode = EMBER_ZCL_STATUS_DEPRECATED83; //for test only
    }
    
    ep->GET_properties[epc].echonetValue = value;
    return errorCode;
}
#endif