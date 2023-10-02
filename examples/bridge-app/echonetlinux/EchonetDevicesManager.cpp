#ifndef ECHONETDEVICES_MANAGER_C
#define ECHONETDEVICES_MANAGER_C
#include"EchonetDevicesManager.h"
#include "Device.h"
#include <string>
#include <json/value.h>
#include <fstream>
#include <json/writer.h>
#include <string>

using namespace chip;
using namespace chip::app;
using namespace chip::Credentials;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::DeviceLayer;
using namespace chip::app::Clusters;
using namespace std;

EchonetDevicesManager::EchonetDevicesManager()
{
    
}

void * FindEchonetDevices_Thread(void * context)
{
    EchonetDevicesManager *manager = (EchonetDevicesManager *)context;
    
    shared_ptr<Echo::EventListener> eventListener(new MyEventListener());
	Echo::addEventListener(eventListener);

	shared_ptr<DefaultNodeProfile> profile(new DefaultNodeProfile());
	vector<shared_ptr<DeviceObject> > devices;
	devices.push_back(shared_ptr<DeviceObject>(new DefaultController()));
    
    TimeManager::GetInstance()->RecordTime(TimeRecordType::APP_BEGIN_SEND_MULTI_CAST_5D);

    //Start openechonet
	Echo::start(profile, devices);

	while(true) {
        //Interval request all avaible echonetLITE endpoints.
		NodeProfile::Getter(NodeProfile::ECHO_CLASS_CODE
				, NodeProfile::INSTANCE_CODE
				, EchoSocket::MULTICAST_ADDRESS).reqGetSelfNodeInstanceListS().send();

        printf("[TOANSTT] Requesting NodeInstanceListS \n\n");
		sleep(10000);
	}
    manager->PrintDevicesSummary();
    
    return NULL;
}
EchonetDevicesManager* EchonetDevicesManager::instance= nullptr;
void EchonetDevicesManager::FindEchonetDevices()
{
   pthread_t poll_thread2; 
    int res = pthread_create(&poll_thread2, nullptr, FindEchonetDevices_Thread, this);
    if (res)
    {
        printf("Error creating polling thread: %d\n", res);
    }
}

void EchonetDevicesManager::PrintDevicesSummary()
{
    printf("\n\n============PrintDevicesSummary============\n");
    
    int countEndpoint=0;
    for(int i =0; i < (int)this->endpoints.size(); i++)
    {
        countEndpoint ++;
    }
    printf("Num devices: %d \t Num endpoints: %d\n", -1, countEndpoint );
    printf("===========================================\n");
}


void MyEventListener::onNewDeviceObject(std::shared_ptr<DeviceObject> device) 
{
    TimeManager::GetInstance()->RecordTime(TimeRecordType::FOUND_AN_ECHONET_ENDPOINT, device->getEchoClassCode(), device->getInstanceCode());

    if(device.get()->getEchoClassCode() == 0x05FF) { // Found a controller
        cout << "Controller found." << endl;
        device.get()->setReceiver(shared_ptr<EchoObject::Receiver>(new MyControllerReceiver()));
        device.get()->get().reqGetOperationStatus().reqGetInstallationLocation().send();
    } 
    else 
    {
        

        printf("NNNN onNewDeviceObject ip=%s instance=0x%04x-%02x \n", device.get()->getNode()->getAddress().c_str(),  device->getEchoClassCode(), device->getInstanceCode());
        pair<string,unsigned int> id = make_pair(device.get()->getNode()->getAddress(),(unsigned int)device.get()->getEchoClassCode()*256+device.get()->getInstanceCode() );
        EchonetDevicesManager::GetInstance()->AddDeviceObject(device, id);

        device.get()->setReceiver(shared_ptr<EchoObject::Receiver>(new EchonetControllerReceiver()));
        // Request to get all SET property Map 
        device.get()->get().reqGetSetPropertyMap().send();
        device.get()->get().reqGetStatusChangeAnnouncementPropertyMap().send();
    } 
}

bool EchonetControllerReceiver::onGetProperty(std::shared_ptr<EchoObject> eoj, unsigned short tid, unsigned char esv, EchoProperty& property, bool success)
{
    //myMutex_.lock();
    EchonetEndpoint* ep = EchonetDevicesManager::GetInstance()->GetEchonetEndpointById(make_pair(eoj->getNode()->getAddress(), (unsigned int)eoj->getEchoClassCode()*256+eoj->getInstanceCode()));
    if(property.epc==0x9f || property.epc==0x9e || property.epc==0x9d)
    {
        TimeManager::GetInstance()->RecordTime(TimeRecordType::GET_A_ENDPOINT_INFO,eoj->getEchoClassCode(),eoj->getInstanceCode(), property.epc  );

        shared_ptr<std::vector<unsigned char>> mSharedPtr = std::make_shared<std::vector<unsigned char> >(property.edt);
        shared_ptr<std::set<unsigned char> > gets = EchoUtils::convertPropertyMapToProperties(mSharedPtr);

        std::shared_ptr<DeviceObject> device = std::static_pointer_cast<DeviceObject>(eoj);
        auto setter = device.get()->get();

        //Process GET properties
        for(unsigned char i :*gets )
        {
            if(property.epc==0x9f) 
            {
                eoj->addGetProperty(i);               
                ep->SET_properties.insert({i,PropertyValues{}});
            }
            else if(property.epc==0x9e)
            {
                eoj->addSetProperty(i);
            } 
            else if(property.epc==0x9d) eoj->addStatusChangeAnnouncementProperty(i);

            //Request all Get property values
            if(property.epc==0x9f)
            {
                if(i!= 0x9f && i != 0x9e && i != 0x9d) 
                {
                    setter.reqGetProperty(i); 
                }
            }
        }
        // If the property is 0x9e then try to get all GET properties
        if(property.epc==0x9e)
        {
            device.get()->get().reqGetGetPropertyMap().send();
            
            //Briefly process the endpoint information
            ep->CalcEndpointType();

            if(ep->type!=MatterDeviceEndpointType::UNKNOW )
            {
                //Process to the echonetLITE endpoint to manager if the endpoint is valid
                EchonetDevicesManager::GetInstance()->onAEchonetEndpointAddedDelegate(ep);
            }
            else 
                printf("[INFO] SKip getting data for device id 0x%04x%02x\n", eoj->getEchoClassCode(), eoj->getInstanceCode());
        }
        if(property.epc==0x9f && ep->type!=MatterDeviceEndpointType::UNKNOW) 
        {
            setter.send();
            
            
            
        }
        //printf("\n");
    } else 
    {
        //myMutex_.lock();
        //TimeManager::GetInstance()->RecordTime(TimeRecordType::RECEIVE_ECHONET_PROPERTY_VALUE,eoj->getEchoClassCode(),eoj->getInstanceCode(), property.epc,0,0, ConvertToUnsignedInt(property.edt) );
        printf("TTTT onGetProperty object=0x%04x-%02x epc=0x%02x len=%d tid=%d esv=0x%02x epc=0x%02x edt=%s\n", eoj->getEchoClassCode(), eoj->getInstanceCode(),
        property.epc, (int)property.edt.size(),tid,esv,property.epc,ConvertEchonetValueToHexString(property.edt).c_str());
        auto id = make_pair( eoj->getNode()->getAddress()  , (unsigned int) eoj.get()->getEchoClassCode()*256+eoj.get()->getInstanceCode());
        EchonetDevicesManager::GetInstance()->AddEchonetGetAttributeValue(id,property);

    } 

    //myMutex_.unlock();
    return success;

}
void MyEventListener::onFoundEchoObject(std::shared_ptr<EchoObject> eoj)
{
    // printf("onFoundEchoObject ClassGroupCode=%02x , ClassCode=%02x , InstanceCode=%02x\n", eoj.get()->getClassGroupCode(), eoj.get()->getClassCode(),
	// 	 eoj.get()->getInstanceCode());

        
    //     EchonetDevicesManager* manager = EchonetDevicesManager::GetInstance();
    //     manager->OnFoundEchoObject(eoj);
}
void EchonetDevicesManager::OnFoundEchoObject(std::shared_ptr<EchoObject> eoj)
{
    //unsigned char ClassGroupCode = eoj.get()->getClassGroupCode();
   // unsigned char ClassCode = eoj.get()->getClassCode();
    unsigned char InstanceCode = eoj.get()->getInstanceCode();
    unsigned short EchoClassCode = eoj.get()->getEchoClassCode();
    pair<string,unsigned int> eoInstanceId = make_pair( eoj->getNode()->getAddress() ,(unsigned int)EchoClassCode*256+InstanceCode);
    if(endpoints.find( eoInstanceId)== endpoints.end() )
    {
        // EchonetEndpoint * echonetEndpoint = new EchonetEndpoint(eoj.get(), eoj_pair_);
        // this->endpoints.insert({eoj_pair_,*echonetEndpoint}); 
    } else 
    {

    }
}
void EchonetDevicesManager::AddDeviceObject(shared_ptr<DeviceObject> deviceObject,pair<string,unsigned int>& id )
{

    EchonetEndpoint* echonetEndpoint = new EchonetEndpoint(deviceObject->getNode()->getAddress() ,id);
    this->endpoints.insert({id,echonetEndpoint });
}
void EchonetDevicesManager::AddDeviceObject(EchonetEndpoint* echonetEndpoint_,pair<string,unsigned int>& id )
{
    //EchonetEndpoint* echonetEndpoint = new EchonetEndpoint(address ,id);
    this->endpoints.insert({id,echonetEndpoint_ });
}
EchonetEndpoint* EchonetDevicesManager::GetEchonetEndpointById(pair<string,unsigned int> id)
{
    if(endpoints.find(id)==endpoints.end()) return NULL;
    return endpoints[id];
}
void EchonetDevicesManager::AddEchonetGetAttributeValue(pair<string,unsigned int>& id, EchoProperty& echoProperty)
{
    if(endpoints.find(id) == endpoints.end())
    {
        printf("\n\n\n\n TOANSTT NOT IMPLEMENTED YET\n\n\n\n");
    }
    else 
    {
        
        endpoints[id]->InsertGETProperty(echoProperty.epc,&echoProperty );
        
    }
}



bool EchonetControllerReceiver::onSetProperty(std::shared_ptr<EchoObject> eoj, unsigned short tid, unsigned char esv, EchoProperty& property, bool success)
{
    printf("UUUU onSetProperty object=0x%04x-%02x epc=0x%02x len=%d tid=%d esv=0x%02x epc=0x%02x edt=%s success=%s\n", eoj->getEchoClassCode(), eoj->getInstanceCode(),
        property.epc, (int)property.edt.size(),tid,esv,property.epc,ConvertEchonetValueToHexString(property.edt).c_str(), success== true?"OK":"FAIL");

    pair<string,unsigned int> id = make_pair(eoj->getNode()->getAddress() ,(unsigned int)eoj.get()->getEchoClassCode()*256+eoj.get()->getInstanceCode());
    EchonetDevicesManager* em = EchonetDevicesManager::GetInstance();

    if(em->endpoints.find(id) != em->endpoints.end())
        em->endpoints[id]->onSetProperty(eoj, tid, esv, property, success);
    
    return success;
}
bool EchonetControllerReceiver::onInformProperty(std::shared_ptr<EchoObject> eoj, unsigned short tid, unsigned char esv, EchoProperty& property)
{
    printf("UUUU onInformProperty object=0x%04x-%02x epc=0x%02x len=%d tid=%d esv=0x%02x epc=0x%02x edt=%s\n", eoj->getEchoClassCode(), eoj->getInstanceCode(),
        property.epc, (int)property.edt.size(),tid,esv,property.epc,ConvertEchonetValueToHexString(property.edt).c_str());
    return true;
}

void EchonetDevicesManager::CheckStartupDone()
{
    map<pair<string,unsigned int>, EchonetEndpoint*>::iterator it;
    int count = 0;
    int countValidEndpoint = 0;
    printf("\ntypes: ");
    for (it = endpoints.begin(); it != endpoints.end(); it++)
    {
       if(it->second->isStartupDone) count++;
       if(it->second->type != MatterDeviceEndpointType::UNKNOW ) countValidEndpoint++; //0x30783009
       //else printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n 88888888888888888888 %d  \n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",(int)it->second->type);

       printf("%d ", it->second->type);
    }
    printf("\nCheckStartupDone: count=%d total=%d countValidEndpoint=%d \n", count, (int)endpoints.size(), countValidEndpoint);

    TimeManager::GetInstance()->RecordTime(TimeRecordType::STARTUP_COUNT_DEVICE, (unsigned short)count, (unsigned char)endpoints.size(),0, 0 );
    
    if(count==countValidEndpoint )
    {
        TimeManager::GetInstance()->RecordTime(TimeRecordType::FINISH_STARTUP, 0, 0,0, 0 );
        WriteAllAdapterInfoToTextFile();
    }
}
void EchonetDevicesManager::RemoveAnEchonetEndpoint(EchonetEndpoint* e)
{
    auto id = e->eoj_pair;
    endpoints.erase(id);
}

void EchonetDevicesManager::WriteAllAdapterInfoToTextFile(string filename)
{
    std::ofstream people_file(filename);
    Json::StyledWriter styledWriter;
    Json::Value json;
    Json::Value listEndpoints;
    //json["aaa"] = "ok 100";
    

    map<pair<string,unsigned int>, EchonetEndpoint*>::iterator it;
    int i=0;
    for (it = endpoints.begin(); it != endpoints.end(); it++)
    {
        if(it->second->type != MatterDeviceEndpointType::UNKNOW)
        {
            listEndpoints[i++] = it->second->ToJson();
        }
    }
    json["endpoints"] = listEndpoints;
    people_file << styledWriter.write(json);
    people_file.close();
}

void EchonetDevicesManager::PrintEchonetDevicesSummary()
{
    map<pair<string,unsigned int>, EchonetEndpoint*>::iterator it;
    char t[128];

    printf("\n\n============Print MyDeviceManager Summary============\n");
    for (it = endpoints.begin(); it != endpoints.end(); it++)
    {
        
        sprintf(t," %s: 0x%06x \n", it->first.first.c_str(),it->first.second );
        printf("%s",t);
    }
    printf("===========================================\n");
}


#endif