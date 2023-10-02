/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#pragma once

#include <app/clusters/mode-select-server/supported-modes-manager.h>
#include <app/util/af.h>
#include <app/util/config.h>
#include <cstring>
#include <vector>
#include "../Utils.h"

using namespace std;
namespace chip {
namespace app {
namespace Clusters {
namespace ModeSelect {

/**
 * This implementation statically defines the options.
 */

class StaticSupportedModesManager : public chip::app::Clusters::ModeSelect::SupportedModesManager
{
    using ModeOptionStructType = Structs::ModeOptionStruct::Type;
    //using storage_value_type   = const ModeOptionStructType;

    struct EndpointSpanPair
    {
        const EndpointId mEndpointId;
        const Span<const ModeOptionStructType> mSpan;

        EndpointSpanPair(const EndpointId aEndpointId, const Span<const ModeOptionStructType> && aSpan) :
            mEndpointId(aEndpointId), mSpan(aSpan)
        {}
        // EndpointSpanPair(const EndpointId aEndpointId, const Span<const ModeOptionStructType>  aSpan) :
        //     mEndpointId(aEndpointId), mSpan(aSpan)
        // {}
        EndpointSpanPair() : mEndpointId(0), mSpan(Span<const ModeOptionStructType>()) {}
    };

    static const ModeOptionStructType echonetOpenCloseStopOptions[];
    static const ModeOptionStructType echonetLightColorOptions[];
    static const ModeOptionStructType echonetPowerSavingOptions[];
    //static const EndpointSpanPair supportedOptionsByEndpoints[EMBER_AF_MODE_SELECT_CLUSTER_SERVER_ENDPOINT_COUNT];
    
public:
//toanstt
    vector<EndpointSpanPair> echonetSupportedOptionsByEndpoint;
    static StaticSupportedModesManager instance;
    
    SupportedModesManager::ModeOptionsProvider getModeOptionsProvider(EndpointId endpointId) const override;

    Protocols::InteractionModel::Status getModeOptionByMode(EndpointId endpointId, uint8_t mode,
                                                            const ModeOptionStructType ** dataPtr) const override;

    ~StaticSupportedModesManager(){};

    StaticSupportedModesManager();

    static inline const StaticSupportedModesManager & getStaticSupportedModesManagerInstance() { return instance; }
    //toanstt
    void AddEchonetSupportedOptionsByEndpoint(EndpointId endpointId,EchonetOptionType echonetOptopnType);
    //void AddEchonetSupportedOptionsByEndpoint(EndpointId endpointId,const vector<ModeSelect::Structs::ModeOptionStruct::Type> selectModeOptions);
    

};

const SupportedModesManager * getSupportedModesManager();

} // namespace ModeSelect
} // namespace Clusters
} // namespace app
} // namespace chip
