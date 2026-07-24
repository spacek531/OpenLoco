#pragma once
#include "GameCommands/GameCommands.h"
#include "GameCommands/Vehicles/RenameVehicle.h"
#include "OpenLoco/Graphics/Colour.h"
#include "OpenLoco/Objects/ObjectManager.h"
#include "OpenLoco/Types.hpp"
#include "OpenLoco/Utility/LookupTable.hpp"
#include "VehicleHead.h"

namespace OpenLoco
{

    static auto kDefaultColours = Utility::buildLookupTable<std::string, ColourScheme>({
        { "AF26CAT1", ColourScheme(Colour::mutedDarkYellow, Colour::mutedYellow) },
        { "AF26CAT2", ColourScheme(Colour::mutedDarkYellow, Colour::mutedYellow) },
        { "AF26P013", ColourScheme(Colour::darkBlue, Colour::black) },
        { "AF26GUKA", ColourScheme(Colour::black, Colour(25)) },
        { "AF26GKP1", ColourScheme(Colour(3), Colour(25)) },
        { "AF26BUFF", ColourScheme(Colour::red, Colour::black) },
        { "AF26EDMU", ColourScheme(Colour::orange, Colour::white) },
        { "AF26DRL1", ColourScheme(Colour::red, Colour::white) },
        { "AF26DRL2", ColourScheme(Colour::red, Colour::white) },
        { "AF26DRL3", ColourScheme(Colour::red, Colour::white) },
        { "AF26SLYM", ColourScheme(Colour::yellow, Colour(25)) },
        { "AF26EMAN", ColourScheme(Colour::mutedPurple, Colour(1)) },
        { "AF26BUG1", ColourScheme(Colour::yellow, Colour(1)) },
        { "AF26RWH1", ColourScheme(Colour::mutedPurple, Colour::pink) },
        { "AF26RWH2", ColourScheme(Colour::mutedPurple, Colour::pink) },
        { "AF26RWH3", ColourScheme(Colour::mutedPurple, Colour::pink) },
        { "AF26ROS1", ColourScheme(Colour(17), Colour(3)) },
        { "AF26ROS2", ColourScheme(Colour(17), Colour(3)) },
        { "AF26RAI1", ColourScheme(Colour(3), Colour(2)) },
        { "AF26RAI2", ColourScheme(Colour(3), Colour(2)) },
        { "AF26BLDD", ColourScheme(Colour::green, Colour::yellow) },
        { "AF26DORO", ColourScheme(Colour::mutedTeal, Colour::white) },
        { "AF26TOTO", ColourScheme(Colour::darkRed, Colour::black) },
        { "AF26ALYS", ColourScheme(Colour::green, Colour::red) },
        { "AF26NATH", ColourScheme(Colour::yellow, Colour::blue) },
    });

    inline const ColourScheme getSpecialColourScheme(const ObjectHeader& header, ColourScheme colourScheme)
    {
        char namebuff[9];
        strncpy(namebuff, header.name, 8);
        if (kDefaultColours.find(namebuff) != kDefaultColours.end())
        {
            return kDefaultColours.find(namebuff)->second;
        }
        return colourScheme;
    }

    inline const ColourScheme getSpecialColourScheme(ObjectType objectType, uint16_t vehicleObjectNum, ColourScheme colourScheme)
    {
        return getSpecialColourScheme(ObjectManager::getHeader({ objectType, vehicleObjectNum }), colourScheme);
    }

    static auto kDefaultVehicleNames = Utility::buildLookupTable<std::string, std::string>({
        { "AF26CAT1", "Cat Train" },
        { "AF26CAT2", "Cat Train" },
        { "AF26P013", "Projectz_013" },
        { "AF26GUKA", "Gulaabee Kamal" },
        { "AF26GKP1", "Gulaabee Kamal" },
        { "AF26BUFF", "Buffey" },
        { "AF26EDMU", "Edmundo" },
        { "AF26DRL1", "Daryll" },
        { "AF26DRL2", "Daryll" },
        { "AF26DRL3", "Daryll" },
        { "AF26SLYM", "SLYM-11513 ASTE" },
        { "AF26EMAN", "Emanuel" },
        { "AF26BUG1", "Bug" },
        { "AF26RWH1", "Row Whoosh" },
        { "AF26RWH2", "Row Whoosh" },
        { "AF26RWH3", "Row Whoosh" },
        { "AF26ROS1", "Rosie the Time Train" },
        { "AF26ROS2", "Rosie the Time Train" },
        { "AF26RAI1", "Raine" },
        { "AF26RAI2", "Raine" },
        { "AF26BLDD", "Blaidd" },
        { "AF26DORO", "Dorothy and Toto" },
        { "AF26TOTO", "Dorothy and Toto" },
        { "AF26ALYS", "Alyssa" },
        { "AF26NATH", "Nathan" },
    });

    static void setSpecialVehicleName(const ObjectHeader& header, const Vehicles::VehicleHead& head)
    {
        char namebuff[9];
        strncpy(namebuff, header.name, 8);
        auto specialName = kDefaultVehicleNames.find(namebuff);
        if (specialName == kDefaultVehicleNames.end())
        {
            return;
        }

        GameCommands::setErrorTitle(StringIds::cant_rename_this_vehicle);
        GameCommands::VehicleRenameArgs args{};
        args.head = head.id;
        std::memcpy(args.buffer, specialName->second.c_str(), 36);
        args.i = 1;
        GameCommands::doCommand(args, GameCommands::Flags::apply);
        args.head = EntityId(0);
        args.i = 2;
        GameCommands::doCommand(args, GameCommands::Flags::apply);
        args.i = 0;
        GameCommands::doCommand(args, GameCommands::Flags::apply);
    }

    inline const void setSpecialVehicleName(ObjectType objectType, uint16_t vehicleObjectNum, const Vehicles::VehicleHead& head)
    {
        setSpecialVehicleName(ObjectManager::getHeader({ objectType, vehicleObjectNum }), head);
    }
}