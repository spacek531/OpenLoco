#pragma once
#include "OpenLoco/Graphics/Colour.h"
#include "OpenLoco/Objects/ObjectManager.h"
#include "OpenLoco/Utility/LookupTable.hpp"
#include "OpenLoco/Types.hpp"

namespace OpenLoco
{

    static auto kDefaultColours = Utility::buildLookupTable<std::string, ColourScheme>({
        { "AF26CATT", ColourScheme(Colour::mutedDarkYellow, Colour::mutedYellow) },
        { "AF26P013", ColourScheme(Colour::darkBlue, Colour::black) },
        { "AF26GUKA", ColourScheme(Colour::black, Colour(25)) },
        { "AF26GKP1", ColourScheme(Colour(3), Colour(25)) },
    });

    inline const ColourScheme getSpecialColourScheme(const ObjectHeader& header, ColourScheme colourScheme)
    {
        char namebuff[9];
        strncpy(namebuff,header.name,8);
        if (kDefaultColours.find(namebuff) != kDefaultColours.end())
        {
            return kDefaultColours.find(namebuff)->second;
        }
        return colourScheme;
    }

    inline const ColourScheme getSpecialColourScheme( ObjectType objectType, uint16_t vehicleObjectNum, ColourScheme colourScheme)
    {
        return getSpecialColourScheme(ObjectManager::getHeader({objectType, vehicleObjectNum}), colourScheme);
    }
}