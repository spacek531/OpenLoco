#include "Graphics/Colour.h"
#include "Graphics/Gfx.h"
#include "Graphics/ImageIds.h"
#include "Graphics/RenderTarget.h"
#include "Graphics/TextRenderer.h"
#include "Input.h"
#include "Localisation/FormatArguments.hpp"
#include "Localisation/Formatting.h"
#include "Localisation/StringIds.h"
#include "Map/BuildingElement.h"
#include "Map/IndustryElement.h"
#include "Map/MapSelection.h"
#include "Map/RoadElement.h"
#include "Map/SignalElement.h"
#include "Map/StationElement.h"
#include "Map/SurfaceElement.h"
#include "Map/TileManager.h"
#include "Map/TrackElement.h"
#include "Map/TreeElement.h"
#include "Map/WallElement.h"
#include "Objects/AirportObject.h"
#include "Objects/BuildingObject.h"
#include "Objects/DockObject.h"
#include "Objects/IndustryObject.h"
#include "Objects/InterfaceSkinObject.h"
#include "Objects/LandObject.h"
#include "Objects/ObjectManager.h"
#include "Objects/RoadObject.h"
#include "Objects/RoadStationObject.h"
#include "Objects/TrackObject.h"
#include "Objects/TrainSignalObject.h"
#include "Objects/TrainStationObject.h"
#include "Objects/TreeObject.h"
#include "Objects/WallObject.h"
#include "Ui/ToolManager.h"
#include "Ui/ViewportInteraction.h"
#include "Ui/Widget.h"
#include "Ui/Widgets/CaptionWidget.h"
#include "Ui/Widgets/CheckboxWidget.h"
#include "Ui/Widgets/FrameWidget.h"
#include "Ui/Widgets/GroupBoxWidget.h"
#include "Ui/Widgets/ImageButtonWidget.h"
#include "Ui/Widgets/PanelWidget.h"
#include "Ui/Widgets/ScrollViewWidget.h"
#include "Ui/Widgets/StepperWidget.h"
#include "Ui/Widgets/TableHeaderWidget.h"
#include "Ui/WindowManager.h"

#include "World/CompanyManager.h"
#include "World/Industry.h"
#include "World/IndustryManager.h"
#include "World/Station.h"

#include <OpenLoco/Utility/LookupTable.hpp>
#include <map>

using namespace OpenLoco::World;

namespace OpenLoco::Ui::Windows::TileInspector
{

    StringId getElementTypeName(const TileElementEntry& element);
    StringId getObjectName(const TileElementEntry& element);
    std::tuple<StringId, CompanyId> getOwnerName(const TileElementEntry& element);

    static TilePos2 _currentPosition{};
    static int _selectedTileType = -1; // int instead of ElementType because ElementType has no null/sentinel value

    namespace widx
    {
        enum
        {
            frame,
            title,
            close,
            panel,
            xPos,
            xPosDecrease,
            xPosIncrease,
            yPos,
            yPosDecrease,
            yPosIncrease,
            select,
            nameTypeHeader,
            baseHeightHeader,
            clearHeightHeader,
            directionHeader,
            ghostHeader,
            scrollview,
            detailsGroup,

            // below: widgets which change position and visibility based on type of selected tile element
            primaryColour,
            secondaryColour,
            tertiaryColour,

            checkbox1,
            industryConstructionCompleteCheckbox = checkbox1,

            checkbox2,
            industryRandomAnimationQueuedCheckbox = checkbox2,

            checkbox3,
            industryRandomAnimationPlayingCheckbox = checkbox3,
        };
    }

    constexpr int16_t kDataGroupBoxTop = 165;
    constexpr int32_t kDataColumnSpacing[4][4] = {
        { 10, 0, 0, 0 },
        { 10, 180, 0, 0 },
        { 10, 125, 250, 0 },
    };
    constexpr int32_t kDataRowHeight = 14;
    constexpr int32_t kDataRows = 9;

    constexpr int32_t dataY(int32_t row)
    {
        return kDataGroupBoxTop + kDataRowHeight * row;
    }

    constexpr int32_t dataX(uint8_t columns, uint8_t column)
    {
        return kDataColumnSpacing[columns - 1][column - 1];
    }

    constexpr Point dataPosition(int32_t row, uint8_t columns, uint8_t column, Point windowPos)
    {
        return Point(windowPos.x + dataX(columns, column), windowPos.y + dataY(row));
    }

    static constexpr Ui::Size kWindowSize = { 350, kDataGroupBoxTop + kDataRowHeight* kDataRows + 4 };

    static constexpr auto _widgets = makeWidgets(
        Widgets::Frame({ 0, 0 }, kWindowSize, WindowColour::primary),
        Widgets::Caption({ 1, 1 }, { kWindowSize.width - 2, 13 }, Widgets::Caption::Style::whiteText, WindowColour::primary, StringIds::tile_inspector),
        Widgets::ImageButton({ kWindowSize.width - 15, 2 }, { 13, 13 }, WindowColour::primary, ImageIds::close_button, StringIds::tooltip_close_window),
        Widgets::Panel({ 0, 15 }, { kWindowSize.width, kWindowSize.height - 15 }, WindowColour::secondary),
        Widgets::stepperWidgets({ 19, 24 }, { 55, 12 }, WindowColour::secondary),
        Widgets::stepperWidgets({ 92, 24 }, { 55, 12 }, WindowColour::secondary),
        Widgets::ImageButton({ kWindowSize.width - 26, 18 }, { 24, 24 }, WindowColour::secondary, ImageIds::construction_new_position, StringIds::tile_inspector_select_btn_tooltip),
        Widgets::TableHeader({ 4, 46 }, { kWindowSize.width - 98, 12 }, WindowColour::secondary, StringIds::tileInspectorHeaderNameType, StringIds::tileInspectorHeaderNameTypeTip), // name
        Widgets::TableHeader({ kWindowSize.width - 109, 46 }, { 30, 12 }, WindowColour::secondary, StringIds::tileInspectorHeaderBaseHeight, StringIds::tileInspectorHeaderBaseHeightTip),
        Widgets::TableHeader({ kWindowSize.width - 79, 46 }, { 30, 12 }, WindowColour::secondary, StringIds::tileInspectorHeaderClearHeight, StringIds::tileInspectorHeaderClearHeightTip),
        Widgets::TableHeader({ kWindowSize.width - 49, 46 }, { 15, 12 }, WindowColour::secondary, StringIds::tileInspectorHeaderDirection, StringIds::tileInspectorHeaderDirectionTip),
        Widgets::TableHeader({ kWindowSize.width - 34, 46 }, { 30, 12 }, WindowColour::secondary, StringIds::tileInspectorHeaderGhost, StringIds::tileInspectorHeaderGhostTip),
        Widgets::ScrollView({ 4, 60 }, { kWindowSize.width - 8, 103 }, WindowColour::secondary, Ui::Scrollbars::vertical),
        Widgets::GroupBox({ 4, kDataGroupBoxTop }, { kWindowSize.width - 8, kWindowSize.height - kDataGroupBoxTop - 4 }, WindowColour::secondary, StringIds::tile_element_data),
        Widgets::ImageButton({ 80, 210 }, { 16, 16 }, WindowColour::secondary, Widget::kContentNull, StringIds::empty),
        Widgets::ImageButton({ 80, 210 }, { 16, 16 }, WindowColour::secondary, Widget::kContentNull, StringIds::empty),
        Widgets::ImageButton({ 80, 210 }, { 16, 16 }, WindowColour::secondary, Widget::kContentNull, StringIds::empty),
        Widgets::Checkbox({ 15, 80 }, { 140, 12 }, WindowColour::secondary, StringIds::empty, StringIds::empty),
        Widgets::Checkbox({ 15, 80 }, { 140, 12 }, WindowColour::secondary, StringIds::empty, StringIds::empty),
        Widgets::Checkbox({ 15, 80 }, { 140, 12 }, WindowColour::secondary, StringIds::empty, StringIds::empty));

    static constexpr Point kPrimaryColourPositions[] = {
        Point(),
        Point(),
        Point(),
        Point(),
        Point(),
        Point(),
        Point(),
        Point(),
        Point(dataX(3, 1), dataY(4) - 3),
    };

    static constexpr Point kSecondaryColourPositions[] = {
        Point(),
        Point(),
        Point(),
        Point(),
        Point(),
        Point(),
        Point(),
        Point(),
        Point(),
    };

    static constexpr Point kTertiaryColourPositions[] = {
        Point(),
        Point(),
        Point(),
        Point(),
        Point(),
        Point(),
        Point(),
        Point(),
        Point(),
    };

    static constexpr Point kCheckbox1Positions[] = {
        Point(),
        Point(),
        Point(),
        Point(),
        Point(),
        Point(),
        Point(),
        Point(),
        Point(dataX(3, 1), dataY(6)),
    };

    static constexpr Point kCheckbox2Positions[] = {
        Point(),
        Point(),
        Point(),
        Point(),
        Point(),
        Point(),
        Point(),
        Point(),
        Point(dataX(3, 2), dataY(6)),
    };

    static constexpr Point kCheckbox3Positions[] = {
        Point(),
        Point(),
        Point(),
        Point(),
        Point(),
        Point(),
        Point(),
        Point(),
        Point(dataX(3, 2), dataY(7)),
    };

    static constexpr StringId kCheckbox1Contents[] = {
        StringIds::empty,
        StringIds::empty,
        StringIds::empty,
        StringIds::empty,
        StringIds::empty,
        StringIds::empty,
        StringIds::empty,
        StringIds::empty,
        StringIds::tile_inspector_industry_element_construction_complete,
    };

    static constexpr StringId kCheckbox2Contents[] = {
        StringIds::empty,
        StringIds::empty,
        StringIds::empty,
        StringIds::empty,
        StringIds::empty,
        StringIds::empty,
        StringIds::empty,
        StringIds::empty,
        StringIds::tile_inspector_industry_element_random_animation_playing,
    };

    static constexpr StringId kCheckbox3Contents[] = {
        StringIds::empty,
        StringIds::empty,
        StringIds::empty,
        StringIds::empty,
        StringIds::empty,
        StringIds::empty,
        StringIds::empty,
        StringIds::empty,
        StringIds::tile_inspector_industry_element_random_animation_available,
    };

    static void repositionWidget(Widget& widget, const Point* positions)
    {
        widget.hidden = _selectedTileType == -1;
        if (_selectedTileType == -1)
        {
            return;
        }
        Point widgetSize = Point(widget.right - widget.left, widget.bottom - widget.top);
        Point position = positions[_selectedTileType];
        if (position == Point())
        {
            widget.hidden = true;
            return;
        }
        widget.left = position.x;
        widget.top = position.y;
        widget.right = widget.left + widgetSize.x;
        widget.bottom = widget.top + widgetSize.y;
    }

    static void activateMapSelectionTool(const Window& self)
    {
        ToolManager::toolSet(self, widx::panel, CursorId::crosshair);
        Input::setFlag(Input::Flags::flag6);
    }

    static const WindowEventList& getEvents();

    Window* open()
    {
        auto window = WindowManager::bringToFront(WindowType::tileInspector);
        if (window != nullptr)
        {
            return window;
        }

        window = WindowManager::createWindow(
            WindowType::tileInspector,
            kWindowSize,
            WindowFlags::none,
            getEvents());

        window->setWidgets(_widgets);
        window->rowCount = 0;
        window->rowHeight = 10;
        window->selectedTileIndex = -1;
        window->initScrollWidgets();

        auto skin = ObjectManager::get<InterfaceSkinObject>();
        window->setColour(WindowColour::primary, skin->windowTitlebarColour);
        window->setColour(WindowColour::secondary, skin->windowColour);

        _selectedTileType = -1;
        activateMapSelectionTool(*window);

        return window;
    }

    static TileElementEntry* getSelectedTile(int16_t selectedTileIndex)
    {
        if (_currentPosition == TilePos2(0, 0))
        {
            return nullptr;
        }
        auto tile = TileManager::get(_currentPosition);
        if (selectedTileIndex >= tile.size())
        {
            return nullptr;
        }
        return tile[selectedTileIndex];
    }
    static std::tuple<Colour, Colour, Colour> getElementColour(const TileElementEntry& element)
    {
        switch (element.type())
        {
            case ElementType::building:
                return { element.get<BuildingElement>().colour(), Colour::black, Colour::black };
            case ElementType::industry:
                return { element.get<IndustryElement>().colour(), Colour::black, Colour::black };
            case ElementType::tree:
                return { element.get<TreeElement>().colour(), Colour::black, Colour::black };
            case ElementType::wall:
                return { element.get<WallElement>().getPrimaryColour(), element.get<WallElement>().getSecondaryColour(), element.get<WallElement>().getTertiaryColour() };
            default:
                return { Colour::black, Colour::black, Colour::black };
        }
    }

    static void prepareDraw(Window& self)
    {
        if (ToolManager::isToolActive(WindowType::tileInspector))
        {
            self.activatedWidgets |= (1 << widx::select);
        }
        else
        {
            self.activatedWidgets &= ~(1 << widx::select);
        }
        auto element = getSelectedTile(self.selectedTileIndex);
        if (element == nullptr)
        {
            _selectedTileType = -1;
            for (int i = widx::primaryColour; i <= widx::checkbox3; i++)
            {
                self.widgets[i].hidden = true;
            }
            return;
        }

        _selectedTileType = enumValue(element->type());
        for (int i = widx::primaryColour; i <= widx::checkbox3; i++)
        {
            self.widgets[i].hidden = false;
        }

        auto colours = getElementColour(*element);
        self.widgets[widx::primaryColour].image = Widget::kImageIdColourSet | Gfx::recolour(ImageIds::colour_swatch_recolourable, std::get<0>(colours));
        self.widgets[widx::secondaryColour].image = Widget::kImageIdColourSet | Gfx::recolour(ImageIds::colour_swatch_recolourable, std::get<1>(colours));
        self.widgets[widx::tertiaryColour].image = Widget::kImageIdColourSet | Gfx::recolour(ImageIds::colour_swatch_recolourable, std::get<2>(colours));

        repositionWidget(self.widgets[widx::primaryColour], kPrimaryColourPositions);
        repositionWidget(self.widgets[widx::secondaryColour], kSecondaryColourPositions);
        repositionWidget(self.widgets[widx::tertiaryColour], kTertiaryColourPositions);

        repositionWidget(self.widgets[widx::checkbox1], kCheckbox1Positions);
        self.widgets[widx::checkbox1].text = kCheckbox1Contents[_selectedTileType];

        repositionWidget(self.widgets[widx::checkbox2], kCheckbox2Positions);
        self.widgets[widx::checkbox2].text = kCheckbox2Contents[_selectedTileType];

        repositionWidget(self.widgets[widx::checkbox3], kCheckbox3Positions);
        self.widgets[widx::checkbox3].text = kCheckbox3Contents[_selectedTileType];
    }

    static void drawNoTileSelected(Ui::Window& self, Gfx::TextRenderer& tr, Point point)
    {
        point += Point(7, 14);
        tr.drawStringLeft(point, Colour::black, StringIds::tile_inspector_no_tile_selected);
    }

    static void drawIndustryTileData(Ui::Window& self, Gfx::TextRenderer& tr, const TileElementEntry& element)
    {
        const IndustryElement& tileElement = element.get<IndustryElement>();
        const auto& industry = *tileElement.industry();

        auto wpos = Point(self.x, self.y);

        AdvancedColour completedColour = AdvancedColour(Colour::grey);
        AdvancedColour constructionColour = AdvancedColour(Colour::grey);
        if (tileElement.isConstructed())
        {
            constructionColour = constructionColour.inset();
        }
        else
        {
            completedColour = completedColour.inset();
        }

        // colour
        {
            FormatArguments args{};
            args.push<uint16_t>(enumValue(tileElement.colour()));
            tr.drawStringLeft(dataPosition(4, 3, 1, wpos) + Point(20, 0), Colour::black, StringIds::tile_inspector_industry_element_colour, args);
        }
        // associated industry
        {
            FormatArguments args{};
            args.push(industry.name);
            args.push(StringIds::empty);
            args.push<uint16_t>(enumValue(industry.id()));
            tr.drawStringLeft(dataPosition(4, 3, 2, wpos), Colour::black, StringIds::tile_inspector_industry_element_industry, args);
        }

        // building type
        {
            FormatArguments args{};
            args.push<uint16_t>(tileElement.buildingType());
            tr.drawStringLeft(dataPosition(5, 3, 1, wpos), Colour::black, StringIds::tile_inspector_industry_element_building_type, args);
        }

        // sequence number
        {
            FormatArguments args{};
            args.push<uint16_t>(tileElement.sequenceIndex());
            tr.drawStringLeft(dataPosition(5, 3, 2, wpos), Colour::black, StringIds::tile_inspector_industry_element_sequence_index, args);
        }

        self.widgets[widx::industryConstructionCompleteCheckbox].activated = tileElement.isConstructed();

        self.widgets[widx::industryRandomAnimationQueuedCheckbox].disabled = !tileElement.isConstructed();
        self.widgets[widx::industryRandomAnimationQueuedCheckbox].activated = tileElement.randomAnimationPlaying();

        // construction progress
        {
            FormatArguments args{};
            args.push<int16_t>(tileElement.sectionsCompleted());
            tr.drawStringLeft(dataPosition(7, 3, 1, wpos), constructionColour, StringIds::tile_inspector_industry_element_completed_sections, args);
        }

        self.widgets[widx::industryRandomAnimationPlayingCheckbox].disabled = !tileElement.isConstructed();
        self.widgets[widx::industryRandomAnimationPlayingCheckbox].activated = tileElement.randomAnimationAvailable();

        // construction progress
        {
            FormatArguments args{};
            args.push<int16_t>(tileElement.sectionProgress());
            tr.drawStringLeft(dataPosition(8, 3, 1, wpos), constructionColour, StringIds::tile_inspector_industry_element_section_progress, args);
        }
        // animation type
        {
            FormatArguments args{};
            args.push<int16_t>(tileElement.randomAnimationType());
            tr.drawStringLeft(dataPosition(8, 3, 2, wpos), completedColour, StringIds::tile_inspector_industry_element_random_animation_type, args);
        }
    }

    static void draw(Ui::Window& self, Gfx::DrawingContext& drawingCtx)
    {
        auto tr = Gfx::TextRenderer(drawingCtx);

        // Draw widgets.
        self.draw(drawingCtx);
        // Coord X/Y labels
        {
            FormatArguments args{};
            args.push(StringIds::tile_inspector_x_coord);
            auto& widget = self.widgets[widx::xPos];
            auto point = Point(self.x + widget.left - 15, self.y + widget.top + 1);
            tr.drawStringLeft(point, Colour::black, StringIds::wcolour2_stringid, args);
        }
        {
            FormatArguments args{};
            args.push(StringIds::tile_inspector_y_coord);
            auto& widget = self.widgets[widx::yPos];
            auto point = Point(self.x + widget.left - 15, self.y + widget.top + 1);
            tr.drawStringLeft(point, Colour::black, StringIds::wcolour2_stringid, args);
        }

        // Coord X/Y values
        {
            FormatArguments args{};
            args.push<int16_t>(_currentPosition.x);
            auto& widget = self.widgets[widx::xPos];
            auto point = Point(self.x + widget.left + 2, self.y + widget.top + 1);
            tr.drawStringLeft(point, Colour::black, StringIds::tile_inspector_coord, args);
        }
        {
            FormatArguments args{};
            args.push<int16_t>(_currentPosition.y);
            auto& widget = self.widgets[widx::yPos];
            auto point = Point(self.x + widget.left + 2, self.y + widget.top + 1);
            tr.drawStringLeft(point, Colour::black, StringIds::tile_inspector_coord, args);
        }

        // Selected element details
        auto widget = self.widgets[widx::detailsGroup];
        auto point = Point(self.x + widget.left, self.y + widget.top);

        auto element = getSelectedTile(self.selectedTileIndex);
        if (element == nullptr)
        {
            drawNoTileSelected(self, tr, point);
            return;
        }

        Point wpos = Point(self.x, self.y);
        // base height
        {
            FormatArguments args{};
            args.push<int16_t>(element->baseZ());
            tr.drawStringLeft(dataPosition(1, 3, 1, wpos), Colour::black, StringIds::tile_inspector_tile_element_base_height, args);
        }
        // tile type
        {
            FormatArguments args{};
            args.push(getElementTypeName(*element));
            args.push(static_cast<uint16_t>(element->type()));
            tr.drawStringLeft(dataPosition(1, 3, 2, wpos), Colour::black, StringIds::tile_inspector_tile_element_type, args);
        }

        // clearance
        {
            FormatArguments args{};
            args.push<int16_t>(element->clearZ());
            tr.drawStringLeft(dataPosition(2, 3, 1, wpos), Colour::black, StringIds::tile_inspector_tile_element_clearance_height, args);
        }
        // object
        {
            FormatArguments args{};
            args.push(getObjectName(*element));
            tr.drawStringLeft(dataPosition(2, 3, 2, wpos), Colour::black, StringIds::tile_inspector_tile_element_object, args);
        }

        // rotation
        {
            FormatArguments args{};
            args.push<int16_t>(element->data()[0] & 0x3);
            tr.drawStringLeft(dataPosition(3, 3, 1, wpos), Colour::black, StringIds::tile_inspector_tile_element_rotation, args);
        }
        // owner
        {
            FormatArguments args{};
            auto ownerInfo = getOwnerName(*element);
            args.push(std::get<0>(ownerInfo));
            args.push(StringIds::empty);
            args.push<int16_t>(enumValue(std::get<1>(ownerInfo)));
            tr.drawStringLeft(dataPosition(3, 3, 2, wpos), Colour::black, StringIds::tile_inspector_tile_element_owner, args);
        }

        switch (element->type())
        {
            case ElementType::industry:
                drawIndustryTileData(self, tr, *element);
                break;
            default:
                break;
        }
    }

    StringId getElementTypeName(const TileElementEntry& element)
    {
        static constexpr auto kTypeToString = Utility::buildLookupTable<ElementType, StringId>({
            { ElementType::surface, StringIds::tile_inspector_element_type_surface },
            { ElementType::track, StringIds::tile_inspector_element_type_track },
            { ElementType::station, StringIds::tile_inspector_element_type_station },
            { ElementType::signal, StringIds::tile_inspector_element_type_signal },
            { ElementType::building, StringIds::tile_inspector_element_type_building },
            { ElementType::tree, StringIds::tile_inspector_element_type_tree },
            { ElementType::wall, StringIds::tile_inspector_element_type_wall },
            { ElementType::road, StringIds::tile_inspector_element_type_road },
            { ElementType::industry, StringIds::tile_inspector_element_type_industry },
        });

        return kTypeToString.at(element.type());
    }

    StringId getObjectName(const TileElementEntry& element)
    {
        switch (element.type())
        {
            case ElementType::surface:
            {
                auto& surface = element.get<SurfaceElement>();
                auto terrainId = surface.terrain();
                auto object = ObjectManager::get<LandObject>(terrainId);
                return object->name;
            }
            case ElementType::track:
            {
                auto& track = element.get<TrackElement>();
                auto objectId = track.trackObjectId();
                auto object = ObjectManager::get<TrackObject>(objectId);
                return object->name;
            }
            case ElementType::station:
            {
                auto& station = element.get<StationElement>();
                auto objectId = station.objectId();
                auto stationType = station.stationType();
                switch (stationType)
                {
                    case StationType::trainStation:
                        return ObjectManager::get<TrainStationObject>(objectId)->name;
                    case StationType::roadStation:
                        return ObjectManager::get<RoadStationObject>(objectId)->name;
                    case StationType::airport:
                        return ObjectManager::get<AirportObject>(objectId)->name;
                    case StationType::docks:
                        return ObjectManager::get<DockObject>(objectId)->name;
                }
                break;
            }
            case ElementType::signal:
            {
                auto& signal = element.get<SignalElement>();

                const TrainSignalObject* object = nullptr;
                if (signal.getLeft().hasSignal())
                {
                    object = ObjectManager::get<TrainSignalObject>(signal.getLeft().signalObjectId());
                }
                else if (signal.getRight().hasSignal())
                {
                    object = ObjectManager::get<TrainSignalObject>(signal.getRight().signalObjectId());
                }

                if (object != nullptr)
                {
                    return object->name;
                }

                break;
            }
            case ElementType::building:
            {
                auto& building = element.get<BuildingElement>();
                auto objectId = building.objectId();
                auto object = ObjectManager::get<BuildingObject>(objectId);
                return object->name;
            }
            case ElementType::tree:
            {
                auto& tree = element.get<TreeElement>();
                auto objectId = tree.treeObjectId();
                auto object = ObjectManager::get<TreeObject>(objectId);
                return object->name;
            }
            case ElementType::wall:
            {
                auto wall = element.get<WallElement>();
                auto objectId = wall.wallObjectId();
                auto object = ObjectManager::get<WallObject>(objectId);
                return object->name;
            }
            case ElementType::road:
            {
                auto& road = element.get<RoadElement>();
                auto objectId = road.roadObjectId();
                auto object = ObjectManager::get<RoadObject>(objectId);
                return object->name;
            }
            case ElementType::industry:
            {
                auto& industry = element.get<IndustryElement>();
                auto object = ObjectManager::get<IndustryObject>(industry.industry()->objectId);
                return object->name;
            }
        }
        return StringIds::empty;
    }

    std::tuple<StringId, CompanyId> getOwnerName(const TileElementEntry& element)
    {
        if (element.type() == ElementType::road)
        {
            auto& road = element.get<RoadElement>();
            auto ownerId = road.owner();
            if (ownerId != CompanyId::neutral)
            {
                auto company = CompanyManager::get(ownerId);
                return { company->name, ownerId };
            }
        }
        else if (element.type() == ElementType::track)
        {
            auto& track = element.get<TrackElement>();
            auto ownerId = track.owner();
            if (ownerId != CompanyId::neutral)
            {
                auto company = CompanyManager::get(ownerId);
                return { company->name, ownerId };
            }
        }
        else if (element.type() == ElementType::industry)
        {
            auto& industryElement = element.get<IndustryElement>();
            auto industry = IndustryManager::get(industryElement.industryId());
            if (industry->owner != CompanyId::neutral)
            {
                auto company = CompanyManager::get(industry->owner);
                return { company->name, industry->owner };
            }
        }
        return { StringIds::publicly_owned, CompanyId::neutral };
    }

    static void drawScroll(Ui::Window& self, Gfx::DrawingContext& drawingCtx, const uint32_t)
    {
        if (_currentPosition == TilePos2(0, 0))
        {
            return;
        }

        const auto& rt = drawingCtx.currentRenderTarget();
        auto tr = Gfx::TextRenderer(drawingCtx);

        auto tile = TileManager::get(_currentPosition);
        auto yPos = 0;
        auto rowNum = 0;
        for (auto& element : tile)
        {
            if (yPos + self.rowHeight < rt.y)
            {
                yPos += self.rowHeight;
                continue;
            }
            else if (yPos > rt.y + rt.height)
            {
                break;
            }

            StringId formatString;
            if (self.selectedTileIndex == rowNum)
            {
                drawingCtx.fillRect(0, yPos, self.width, yPos + self.rowHeight, PaletteIndex::black0, Gfx::RectFlags::none);
                formatString = StringIds::white_stringid;
            }
            else if (self.rowHover == rowNum)
            {
                drawingCtx.fillRect(0, yPos, self.width, yPos + self.rowHeight, enumValue(ExtColour::unk30), Gfx::RectFlags::transparent);
                formatString = StringIds::wcolour2_stringid;
            }
            else
            {
                formatString = StringIds::wcolour2_stringid;
            }

            FormatArguments args = {};

            StringId elementName = getElementTypeName(element);
            StringId objectName = getObjectName(element);
            auto ownerInfo = getOwnerName(element);
            StringId ownerName = std::get<0>(ownerInfo);

            if (std::get<1>(ownerInfo) != CompanyId::neutral)
            {
                args.push(StringIds::tile_inspector_entry_three_pos);
                args.push(objectName);
                args.push(ownerName);
                args.push(StringIds::empty);
                args.push(elementName);
            }
            else
            {
                args.push(StringIds::tile_inspector_entry_two_pos);
                args.push(objectName);
                args.push(elementName);
            }

            // Draw name and type
            auto* widget = &self.widgets[widx::nameTypeHeader];
            auto point = Point(0, yPos);
            tr.drawStringLeftClipped(point, widget->width(), Colour::black, formatString, args);

            // Draw base height
            widget = &self.widgets[widx::baseHeightHeader];
            args.rewind();
            args.push(StringIds::uint16_raw);
            args.push<uint16_t>(element.baseZ());
            point = Point(widget->left - 4, yPos);
            tr.drawStringLeftClipped(point, widget->width(), Colour::black, formatString, args);

            // Draw clear height
            widget = &self.widgets[widx::clearHeightHeader];
            args.rewind();
            args.push(StringIds::uint16_raw);
            args.push<uint16_t>(element.clearZ());
            point = Point(widget->left - 4, yPos);
            tr.drawStringLeftClipped(point, widget->width(), Colour::black, formatString, args);

            // Draw direction
            widget = &self.widgets[widx::directionHeader];
            args.rewind();
            args.push(StringIds::uint16_raw);
            args.push<uint16_t>(element.data()[0] & 0x03);
            point = Point(widget->left - 4, yPos);
            tr.drawStringLeftClipped(point, widget->width(), Colour::black, formatString, args);

            // Draw ghost flag
            widget = &self.widgets[widx::ghostHeader];
            if (element.isGhost())
            {
                static constexpr char strCheckmark[] = "\xAC";
                point = Point(widget->left - 4, yPos);
                tr.drawString(point, Colour::white, strCheckmark);
            }

            rowNum++;
            yPos += self.rowHeight;
        }
    }

    static void scrollMouseDown(Window& self, [[maybe_unused]] const int16_t x, const int16_t y, [[maybe_unused]] const uint8_t scrollIndex)
    {
        auto index = y / self.rowHeight;
        if (index >= self.rowCount)
        {
            return;
        }

        if (self.selectedTileIndex != index)
        {
            self.selectedTileIndex = index;
            self.invalidate();
            return;
        }
    }

    static void scrollMouseOver(Window& self, [[maybe_unused]] const int16_t x, const int16_t y, [[maybe_unused]] const uint8_t scrollIndex)
    {
        auto index = y / self.rowHeight;
        if (index >= self.rowCount)
        {
            return;
        }

        if (self.rowHover != index)
        {
            self.rowHover = index;
            self.invalidate();
        }
    }

    static void onMouseUp(Ui::Window& self, WidgetIndex_t widgetIndex, [[maybe_unused]] const WidgetId id)
    {
        switch (widgetIndex)
        {
            case widx::close:
                WindowManager::close(self.type);
                break;

            case widx::select:
                activateMapSelectionTool(self);
                break;
        }
    }

    static void onMouseDown(Ui::Window& self, const WidgetIndex_t widgetIndex, [[maybe_unused]] const WidgetId id)
    {
        switch (widgetIndex)
        {
            case widx::xPosDecrease:
                _currentPosition.x = std::clamp<coord_t>(_currentPosition.x - 1, 1, World::kMapColumns);
                self.invalidate();
                break;

            case widx::xPosIncrease:
                _currentPosition.x = std::clamp<coord_t>(_currentPosition.x + 1, 1, World::kMapColumns);
                self.invalidate();
                break;

            case widx::yPosDecrease:
                _currentPosition.y = std::clamp<coord_t>(_currentPosition.y - 1, 1, World::kMapRows);
                self.invalidate();
                break;

            case widx::yPosIncrease:
                _currentPosition.y = std::clamp<coord_t>(_currentPosition.y + 1, 1, World::kMapRows);
                self.invalidate();
                break;
        }
    }

    static void getScrollSize(Ui::Window& self, uint32_t, [[maybe_unused]] int32_t& scrollWidth, int32_t& scrollHeight)
    {
        if (_currentPosition == TilePos2(0, 0))
        {
            scrollHeight = 0;
            return;
        }

        scrollHeight = self.rowCount * self.rowHeight;
    }

    static void onToolUpdate([[maybe_unused]] Window& self, const WidgetIndex_t widgetIndex, [[maybe_unused]] const WidgetId id, const int16_t x, const int16_t y)
    {
        if (widgetIndex != widx::panel)
        {
            return;
        }

        World::mapInvalidateSelectionRect();
        World::resetMapSelectionFlag(World::MapSelectionFlags::enable);
        auto res = Ui::ViewportInteraction::getSurfaceLocFromUi({ x, y });
        if (res)
        {
            World::setMapSelectionSingleTile(res->first);
        }
    }

    static void onToolDown(Window& self, const WidgetIndex_t widgetIndex, [[maybe_unused]] const WidgetId id, const int16_t x, const int16_t y)
    {
        if (widgetIndex != widx::panel || !World::hasMapSelectionFlag(World::MapSelectionFlags::enable))
        {
            return;
        }

        auto res = Ui::ViewportInteraction::getSurfaceLocFromUi({ x, y });
        if (!res)
        {
            return;
        }
        _currentPosition = World::toTileSpace(res->first);
        auto tile = TileManager::get(_currentPosition);

        self.rowCount = static_cast<uint16_t>(tile.size());
        self.rowHover = -1;
        self.selectedTileIndex = 0;
        self.invalidate();
    }

    static void onClose([[maybe_unused]] Window& self)
    {
        ToolManager::toolCancel();
    }

    static void onUpdate([[maybe_unused]] Window& self)
    {
        WindowManager::invalidate(WindowType::tileInspector);
    }

    static constexpr WindowEventList kEvents = {
        .onClose = onClose,
        .onMouseUp = onMouseUp,
        .onMouseDown = onMouseDown,
        .onUpdate = onUpdate,
        .onToolUpdate = onToolUpdate,
        .onToolDown = onToolDown,
        .getScrollSize = getScrollSize,
        .scrollMouseDown = scrollMouseDown,
        .scrollMouseOver = scrollMouseOver,
        .prepareDraw = prepareDraw,
        .draw = draw,
        .drawScroll = drawScroll,
    };

    static const WindowEventList& getEvents()
    {
        return kEvents;
    }
}
