//
// Created by Adrian on 11/02/2024.
//

#include "f3Renderer.h"
#include "f3DataBuilder.h"
#include "../common/text_vector_renderer/TextVectorRenderer.h"
#include "../common/api_extension_renderer/SimpleLineRenderer.h"
#include "../../recorder/ps_recorder/psRecorder.h"
#include <vector>

std::vector<std::string> info_data;
std::vector<std::string> control_data;

int cycle = 0;

void f3Renderer::Renderer(ScreenView* screenView, MinecraftUIRenderContext* uiRenderContext, ClientInstance* clientInstance) {
    cycle++;

    if (cycle > 15) {
        cycle = 0;

        info_data.clear();
        info_data = f3DataBuilder::BuildInfoData(clientInstance);

        control_data.clear();
        control_data = f3DataBuilder::BuildControlData(clientInstance);
    }

    std::vector<std::string> extra_data = {
            "GPU: Nvidia 4090 TI Super",
            "CPU: Intel i9 15700K",
            "RAM: 67/128GB (52%)"
    };

    std::vector<std::string> tool_tips = {
            "§bIf you press alt+F4",
            "§byou get free diamonds."
    };

    Vec2 uiScreenSize = clientInstance->guiData->clientUIScreenSize;

    TextVectorRenderer::TextVectorTopLeftRenderer(screenView, uiRenderContext, info_data, uiScreenSize);
    TextVectorRenderer::TextVectorTopRightRenderer(screenView, uiRenderContext, control_data, uiScreenSize);

    TextVectorRenderer::TextVectorBottomLeftRenderer(screenView, uiRenderContext, extra_data, uiScreenSize);
    TextVectorRenderer::TextVectorBottomRightRenderer(screenView, uiRenderContext, tool_tips, uiScreenSize);
}
