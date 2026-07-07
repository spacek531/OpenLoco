#include "Objects/ScaffoldingObject.h"
#include "Graphics/Colour.h"
#include "Graphics/DrawingContext.h"
#include "Graphics/Gfx.h"
#include "Objects/ObjectImageTable.h"
#include "Objects/ObjectManager.h"
#include "Objects/ObjectStringTable.h"

#include <cassert>

namespace OpenLoco
{
    // 0x0042DF15
    void ScaffoldingObject::drawPreviewImage(Gfx::DrawingContext& drawingCtx, const int16_t x, const int16_t y, [[maybe_unused]] ColourScheme colourScheme) const
    {
        auto colourImage = Gfx::recolour(image, colourScheme.primary);

        drawingCtx.drawImage(ZoomLevel::full, x, y + 23, colourImage + Scaffolding::ImageIds::type21x1SegmentBack);
        drawingCtx.drawImage(ZoomLevel::full, x, y + 23, colourImage + Scaffolding::ImageIds::type21x1SegmentFront);
        drawingCtx.drawImage(ZoomLevel::full, x, y + 23 - segmentHeights[2], colourImage + Scaffolding::ImageIds::type21x1RoofSE);
    }

    // 0x0042DED8
    void ScaffoldingObject::load(const LoadedObjectHandle& handle, std::span<const std::byte> data, ObjectManager::DependentObjects*)
    {
        auto remainingData = data.subspan(sizeof(ScaffoldingObject));

        // Load object name string
        auto strRes = ObjectManager::loadStringTable(remainingData, handle, 0);
        name = strRes.str;
        remainingData = remainingData.subspan(strRes.tableLength);

        // Load images
        auto imageRes = ObjectManager::loadImageTable(remainingData);
        image = imageRes.imageOffset;

        // Ensure we've loaded the entire object
        assert(remainingData.size() == imageRes.tableLength);
    }

    // 0x0042DEFE
    void ScaffoldingObject::unload()
    {
        name = 0;
        image = 0;
    }
}
