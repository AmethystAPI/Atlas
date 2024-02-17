//
// Created by adria on 15/02/2024.
//

#include "SimpleLineRenderer.h"
#include <vector>

void SimpleLineRenderer(MinecraftUIRenderContext* ctx, Vec2 point_1, Vec2 point_2) {
    Tessellator* tessellator = &ctx->mScreenContext->tessellator;

    // found in ScreenRenderer::loadMaterials
    mce::MaterialPtr* mat = *reinterpret_cast<mce::MaterialPtr**>(SlideAddress(0x572A440));

    tessellator->begin(mce::PrimitiveMode::LineList, 10);
    tessellator->vertex(point_1.x, point_1.y, 0);
    tessellator->vertex(point_2.x, point_2.y, 0);

    //tessellator->end(mesh, 0, "Test Mesh", 0);
    MeshHelpers::renderMeshImmediately(ctx->mScreenContext, tessellator, mat);
}

void VectorLineRenderer(MinecraftUIRenderContext* ctx, const std::vector<Vec2>& points){
    size_t max_size = points.size() - 1;

    for (int i = 0; i < points.size(); ++i) {
        if (i >= max_size){
            break;
        }

        //Log::Info("POINT: {}; MAX: {}; NX: {}", i, max_size, i+1);
        //Log::Info("DATA: {} {}", points[i].x, points[i].y);
        //Log::Info("NX DATA: {} {}", points[i+1].x, points[i+1].y);

        SimpleLineRenderer(ctx, points[i], points[i+1]);
    }
}