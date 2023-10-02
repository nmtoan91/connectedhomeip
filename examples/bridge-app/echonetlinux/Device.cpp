/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
#ifndef DEVICE_C
#define DEVICE_C
#include "Device.h"

#include <cstdio>
#include <platform/CHIPDeviceLayer.h>
#include "EchonetEndpoint.h"
#include <string>
#include "Utils.h"

using namespace std;
using namespace chip::app::Clusters::Actions;

Device::Device(const char * szDeviceName, std::string szLocation)
{
    chip::Platform::CopyString(mName, szDeviceName);
    mLocation   = szLocation;
    mReachable  = false;
    mEndpointId = 0;

    // vendorName = "NOT_ASSIGNED";
    // productName = "NOT_ASSIGNED";
    // softwareVersionString = "NOT_ASSIGNED";
    // manufacturingDate = "NOT_ASSIGNED";
}

bool Device::IsReachable()
{
    return mReachable;
}

void Device::SetReachable(bool aReachable)
{
    bool changed = (mReachable != aReachable);

    mReachable = aReachable;

    if (aReachable)
    {
        ChipLogProgress(DeviceLayer, "Device[%s]: ONLINE", mName);
    }
    else
    {
        ChipLogProgress(DeviceLayer, "Device[%s]: OFFLINE", mName);
    }

    if (changed)
    {
        HandleDeviceChange(this, kChanged_Reachable);
    }
}

void Device::SetName(const char * szName)
{
    bool changed = (strncmp(mName, szName, sizeof(mName)) != 0);

    ChipLogProgress(DeviceLayer, "Device[%s]: New Name=\"%s\"", mName, szName);

    chip::Platform::CopyString(mName, szName);

     if (changed)
    {
        HandleDeviceChange(this, kChanged_Name);
    }
}

void Device::SetLocation(std::string szLocation)
{
    bool changed = (mLocation.compare(szLocation) != 0);

    mLocation = szLocation;

    ChipLogProgress(DeviceLayer, "Device[%s]: Location=\"%s\"", mName, mLocation.c_str());

    if (changed)
    {
        HandleDeviceChange(this, kChanged_Location);
    }
}


DeviceWindowCovering::DeviceWindowCovering(const char * szDeviceName, std::string szLocation) : Device(szDeviceName, szLocation)
{
    
}
void DeviceWindowCovering::HandleDeviceChange(Device * device, Device::Changed_t changeMask)
{

}

EndpointListInfo::EndpointListInfo(uint16_t endpointListId, std::string name, EndpointListTypeEnum type)
{
    mEndpointListId = endpointListId;
    mName           = name;
    mType           = type;
}

EndpointListInfo::EndpointListInfo(uint16_t endpointListId, std::string name, EndpointListTypeEnum type,
                                   chip::EndpointId endpointId)
{
    mEndpointListId = endpointListId;
    mName           = name;
    mType           = type;
    mEndpoints.push_back(endpointId);
}

void EndpointListInfo::AddEndpointId(chip::EndpointId endpointId)
{
    mEndpoints.push_back(endpointId);
}

Room::Room(std::string name, uint16_t endpointListId, EndpointListTypeEnum type, bool isVisible)
{
    mName           = name;
    mEndpointListId = endpointListId;
    mType           = type;
    mIsVisible      = isVisible;
}

Action::Action(uint16_t actionId, std::string name, ActionTypeEnum type, uint16_t endpointListId, uint16_t supportedCommands,
               ActionStateEnum status, bool isVisible)
{
    mActionId          = actionId;
    mName              = name;
    mType              = type;
    mEndpointListId    = endpointListId;
    mSupportedCommands = supportedCommands;
    mStatus            = status;
    mIsVisible         = isVisible;
}







DeviceEchonetAdapter::DeviceEchonetAdapter(const char * szDeviceName, std::string szLocation) : Device(szDeviceName, szLocation)
{
    
}
void DeviceEchonetAdapter::HandleDeviceChange(Device * device, Device::Changed_t changeMask)
{

}

DeviceIHouseEchonetSwitch::DeviceIHouseEchonetSwitch(const char * szDeviceName, std::string szLocation) : Device(szDeviceName, szLocation)
{
    
}
void DeviceIHouseEchonetSwitch::HandleDeviceChange(Device * device, Device::Changed_t changeMask)
{

}



#endif
