//
// Created by Adrian on 11/02/2024.
//

#include "minimapRenderer.h"
#include "../color/ColorHandler.h"
#include "../heightmap/HeightMapCalc.h"
#include <minecraft/src-client/common/client/renderer/helpers/MeshHelpers.h>
#include <minecraft/src/common/world/level/BlockPos.h>
#include <minecraft/src/common/world/level/block/Block.h>
#include <minecraft/src/common/world/level/block/BlockLegacy.h>

int frame_c = 0;

Vec3 vertexes[6] = {
    Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f), Vec3(1.0f, 0.0f, 0.0f),
    Vec3(0.0f, 1.0f, 0.0f), Vec3(1.0f, 1.0f, 0.0f), Vec3(1.0f, 0.0f, 0.0f)
};

void Minimap::OnJoinGame()
{
    this->hasInitializedMinimap = false;
}

void Minimap::OnLeaveGame()
{
    if (this->hasInitializedMinimap) {
        this->region->removeListener(*this);
        this->hasInitializedMinimap = false;
    }
}

void Minimap::Render(ScreenView* screenView, MinecraftUIRenderContext* renderContext)
{
    this->region = renderContext->mClient->getRegion();
    if (this->region == nullptr) return;

    if (!this->hasInitializedMinimap) {
        this->region->addListener(*this);
        this->hasInitializedMinimap = true;
    }

    frame_c++;
    if (frame_c == 1000) {
        for (int x = -1000; x < 1000; ++x) {
            for (int z = -1000; z < 1000; ++z) {
                mce::Color color = ColorHandler::getColor(region, x, z, this->mUseTranslucency);
                this->mColorData[x][z] = color;
            }
        }
    }

    int minimapScreenSize = 135;
    int offset = 5;

    LocalPlayer* player = renderContext->mClient->getLocalPlayer();
    Tessellator* tessellator = &renderContext->mScreenContext->tessellator;

    Vec2 uiScreenSize = renderContext->mClient->guiData->clientUIScreenSize;

    int width_in_blocks = ((this->mRenderDistance + this->mRenderSafeZone) * 16) * 2;
    float pixels_per_block = (float)minimapScreenSize / (float)width_in_blocks;

    // Placement of the minimap on the screen
    float left = uiScreenSize.x - minimapScreenSize - offset * 2;
    float bottom = offset * 2 + minimapScreenSize;

    // Position the player in the center of the map
    Vec3* player_pos = player->getPosition();
    int map_min_x = (int)player_pos->x - width_in_blocks / 2;
    int map_min_z = (int)player_pos->z - width_in_blocks / 2;

    tessellator->begin(mce::TriangleList, width_in_blocks * width_in_blocks);
    tessellator->mNoColor = false;

    mce::Color color(0, 0, 0, 1);

    for (int local_x = 0; local_x < width_in_blocks; ++local_x) {
        for (int local_z = 0; local_z < width_in_blocks; ++local_z) {
            float pixel_left = left + (float)local_x * pixels_per_block;
            float pixel_bottom = bottom - (float)local_z * pixels_per_block;

            color = this->mColorData[map_min_x + local_x][map_min_z + local_z];

            tessellator->color(color.r, color.g, color.b, color.a);

            for (Vec3 vertex : vertexes) {
                float vertex_x = (vertex.x * pixels_per_block) + pixel_left;;
                float vertex_y = (vertex.y * pixels_per_block) + pixel_bottom;;

                tessellator->vertex(vertex_x, vertex_y, 0);
            }
        }
    }

    Log::Info("#{:x}{:x}{:x}", (int) (color.r*255), (int) (color.g*255), (int) (color.b*255));

    // Get material
    //HashedString hashedMaterialName("entity_static");
    //mce::MaterialPtr material(*mce::RenderMaterialGroup::switchable, hashedMaterialName);

    // We need to find another material because this one goes black when it is showing a hover outline
    mce::MaterialPtr* material = *reinterpret_cast<mce::MaterialPtr**>(SlideAddress(0x572A440));

    // Draw everything
    MeshHelpers::renderMeshImmediately(renderContext->mScreenContext, tessellator, material);

}

void Minimap::onBlockChanged(BlockSource& source, const BlockPos& pos, uint32_t layer, const Block& block, const Block& oldBlock, int updateFlags, const ActorBlockSyncMessage* syncMsg, BlockChangedEventTarget eventTarget, Actor* blockChangeSource)
{
    //mce::Color color = ColorHandler::getColor(&source, pos.x, pos.z, this->mUseTranslucency);
    //Log::Info("X: {}, Y: {}, Z: {} | #{:x}{:x}{:x}", pos.x, pos.y, pos.z, (int) (color.r*255), (int) (color.g*255), (int) (color.b*255));
    //this->mColorData[pos.x][pos.z] = color;
}
