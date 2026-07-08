#include "OpenLoco/Map/QuantityLimits.h"
#include "OpenLoco/S5/Limits.h"

namespace OpenLoco::Map::Count
{
    using namespace OpenLoco::S5::Limits;

    static std::array<CompanyObjectCount, kMaxCompanies + 1> _companies;
    static ObjectCount _nullObjectCount{};

    ObjectCount& CompanyObjectCount::countm(ObjectType type, uint16_t objectId)
    {
        switch (type)
        {
            case ObjectType::airport:
                if (objectId >= kMaxAirportObjects)
                {
                    return _nullObjectCount;
                }
                return _airports.at(objectId);
            case ObjectType::bridge:
                if (objectId >= kMaxBridgeObjects)
                {
                    return _nullObjectCount;
                }
                return _bridges.at(objectId);
            case ObjectType::dock:
                if (objectId >= kMaxDockObjects)
                {
                    return _nullObjectCount;
                }
                return _docks.at(objectId);
            case ObjectType::road:
                if (objectId >= kMaxRoadObjects)
                {
                    return _nullObjectCount;
                }
                return _roads.at(objectId);
            case ObjectType::roadExtra:
                if (objectId >= kMaxRoadExtraObjects)
                {
                    return _nullObjectCount;
                }
                return _roadExtras.at(objectId);
            case ObjectType::roadStation:
                if (objectId >= kMaxRoadStationObjects)
                {
                    return _nullObjectCount;
                }
                return _roadStations.at(objectId);
            case ObjectType::track:
                if (objectId >= kMaxTrackObjects)
                {
                    return _nullObjectCount;
                }
                return _tracks.at(objectId);
            case ObjectType::trackExtra:
                if (objectId >= kMaxTrackExtraObjects)
                {
                    return _nullObjectCount;
                }
                return _trackExtras.at(objectId);
            case ObjectType::trainStation:
                if (objectId >= kMaxTrainStationObjects)
                {
                    return _nullObjectCount;
                }
                return _trainStations.at(objectId);
            case ObjectType::trackSignal:
                if (objectId >= kMaxTrainSignalObjects)
                {
                    return _nullObjectCount;
                }
                return _trainSignals.at(objectId);
            case ObjectType::vehicle:
                if (objectId >= kMaxVehicleObjects)
                {
                    return _nullObjectCount;
                }
                return _vehicles.at(objectId);
            default:
                return _nullObjectCount;
        }
    }

    const ObjectCount& CompanyObjectCount::count(ObjectType type, uint16_t objectId) const
    {
        switch (type)
        {
            case ObjectType::airport:
                if (objectId >= kMaxAirportObjects)
                {
                    return _nullObjectCount;
                }
                return _airports.at(objectId);
            case ObjectType::bridge:
                if (objectId >= kMaxBridgeObjects)
                {
                    return _nullObjectCount;
                }
                return _bridges.at(objectId);
            case ObjectType::dock:
                if (objectId >= kMaxDockObjects)
                {
                    return _nullObjectCount;
                }
                return _docks.at(objectId);
            case ObjectType::road:
                if (objectId >= kMaxRoadObjects)
                {
                    return _nullObjectCount;
                }
                return _roads.at(objectId);
            case ObjectType::roadExtra:
                if (objectId >= kMaxRoadExtraObjects)
                {
                    return _nullObjectCount;
                }
                return _roadExtras.at(objectId);
            case ObjectType::roadStation:
                if (objectId >= kMaxRoadStationObjects)
                {
                    return _nullObjectCount;
                }
                return _roadStations.at(objectId);
            case ObjectType::track:
                if (objectId >= kMaxTrackObjects)
                {
                    return _nullObjectCount;
                }
                return _tracks.at(objectId);
            case ObjectType::trackExtra:
                if (objectId >= kMaxTrackExtraObjects)
                {
                    return _nullObjectCount;
                }
                return _trackExtras.at(objectId);
            case ObjectType::trainStation:
                if (objectId >= kMaxTrainStationObjects)
                {
                    return _nullObjectCount;
                }
                return _trainStations.at(objectId);
            case ObjectType::trackSignal:
                if (objectId >= kMaxTrainSignalObjects)
                {
                    return _nullObjectCount;
                }
                return _trainSignals.at(objectId);
            case ObjectType::vehicle:
                if (objectId >= kMaxVehicleObjects)
                {
                    return _nullObjectCount;
                }
                return _vehicles.at(objectId);
            default:
                return _nullObjectCount;
        }
    }

    bool CompanyObjectCount::canAdd(ObjectType T, uint16_t objectId) const
    {
        auto ocount = count(T, objectId);
        return (ocount.limit == kObjectCountUnlimited) || (ocount.count < ocount.limit);
    }

    bool CompanyObjectCount::canAddMultiple(ObjectType T, uint16_t objectId, int16_t amount) const
    {
        auto ocount = count(T, objectId);
        return (ocount.limit == kObjectCountUnlimited) || (ocount.count + amount <= ocount.limit && ocount.count + amount <= std::numeric_limits<int16_t>::max());
    }

    bool CompanyObjectCount::tryAdd(ObjectType T, uint16_t objectId)
    {
        auto ocount = countm(T, objectId);
        if ((ocount.limit == kObjectCountUnlimited) || (ocount.count < ocount.limit))
        {
            ocount.count++;
            return true;
        }
        return false;
    }

    void CompanyObjectCount::add(ObjectType T, uint16_t objectId)
    {
        countm(T, objectId).count++;
    }

    void CompanyObjectCount::subtract(ObjectType T, uint16_t objectId)
    {
        auto& ocount = countm(T, objectId);
        if (ocount.count <= 0)
        {
            return;
        }
        countm(T, objectId).count--;
    }

    void CompanyObjectCount::set(ObjectType T, uint16_t objectId, uint16_t num)
    {
        countm(T, objectId).count = num;
    }

    CompanyObjectCount& getCompanyObjectCount(CompanyId company)
    {
        return _companies.at(enumValue(company));
    }

    void CompanyObjectCount::reset()
    {
        _airports = {};
        _bridges = {};
        _docks = {};
        _roads = {};
        _roadExtras = {};
        _roadStations = {};
        _tracks = {};
        _trackExtras = {};
        _trainSignals = {};
        _trainStations = {};
        _vehicles = {};
    }

    void resetCompanyCount(CompanyId company)
    {
        auto& c = getCompanyObjectCount(company);
        c.reset();
    }
    void resetAllCount()
    {
        for (auto& c : _companies)
        {
            c.reset();
        }
    }
}