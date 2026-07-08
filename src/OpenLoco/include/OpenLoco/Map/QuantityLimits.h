#pragma once
#include "OpenLoco/Objects/Object.h"
#include "OpenLoco/S5/Limits.h"
#include "OpenLoco/Types.hpp"

#include <array>

namespace OpenLoco::Map::Count
{

    constexpr int16_t kObjectCountUnlimited = -1;
    struct ObjectCount
    {
        int16_t limit = kObjectCountUnlimited;
        uint16_t count;
    };

    struct CompanyObjectCount
    {
        std::array<ObjectCount, OpenLoco::S5::Limits::kMaxAirportObjects> _airports;
        std::array<ObjectCount, OpenLoco::S5::Limits::kMaxBridgeObjects> _bridges;
        std::array<ObjectCount, OpenLoco::S5::Limits::kMaxDockObjects> _docks;
        std::array<ObjectCount, OpenLoco::S5::Limits::kMaxRoadObjects> _roads;
        std::array<ObjectCount, OpenLoco::S5::Limits::kMaxRoadExtraObjects> _roadExtras;
        std::array<ObjectCount, OpenLoco::S5::Limits::kMaxRoadStationObjects> _roadStations;
        std::array<ObjectCount, OpenLoco::S5::Limits::kMaxTrackObjects> _tracks;
        std::array<ObjectCount, OpenLoco::S5::Limits::kMaxTrackExtraObjects> _trackExtras;
        std::array<ObjectCount, OpenLoco::S5::Limits::kMaxTrainStationObjects> _trainStations;
        // not implemented because signals do not store their ownership in the element, they infer it from the track element below
        std::array<ObjectCount, OpenLoco::S5::Limits::kMaxTrainSignalObjects> _trainSignals;
        std::array<ObjectCount, OpenLoco::S5::Limits::kMaxVehicleObjects> _vehicles;
        ObjectCount& countm(ObjectType type, uint16_t objectId);

        const ObjectCount& count(ObjectType type, uint16_t objectId) const;
        bool canAdd(ObjectType T, uint16_t objectId) const;
        bool canAddMultiple(ObjectType T, uint16_t objectId, int16_t amount) const;
        bool tryAdd(ObjectType T, uint16_t objectId);
        void add(ObjectType T, uint16_t objectId);
        void subtract(ObjectType T, uint16_t objectId);
        void set(ObjectType T, uint16_t objectId, uint16_t num);
        void reset();
    };

    CompanyObjectCount& getCompanyObjectCount(CompanyId company);

    void resetCompanyCount(CompanyId company);
    void resetAllCount();

}