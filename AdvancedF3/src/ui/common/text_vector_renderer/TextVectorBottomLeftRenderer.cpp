//
// Created by adria on 14/02/2024.
//

#include "TextVectorRenderer.h"
#include "../../config/UiConfig.h"

void TextVectorRenderer::TextVectorBottomLeftRenderer(ScreenView *screenView, MinecraftUIRenderContext *uiRenderContext, std::vector<std::string> data, Vec2 uiScreenSize){
    int offset = UiConfig::offset;
    mce::Color color = UiConfig::background_color;

    float targetStringSize = 0;

    for (const auto & i : data) {
        if (static_cast<float>(i.length()) > targetStringSize ){
            targetStringSize = static_cast<float>(i.length());
        }
    }

    float height = static_cast<float>(data.size()) * 10;

    auto textMeasureData = TextMeasureData(
            1.0f,
            1,
            false,
            false,
            false
    );

    auto caretMeasureData = CaretMeasureData(
            1,
            false
    );

    auto bg_area = RectangleArea(
            static_cast<float>(offset),
            targetStringSize * 5 + static_cast<float>(offset) * 3,
            uiScreenSize.y - height - (static_cast<float>(offset) * 3),
            uiScreenSize.y - static_cast<float>(offset)
    );

    uiRenderContext->drawRectangle(
            &bg_area,
            &color,
            UiConfig::background_color_alpha,
            0
    );

    for (int i = 0; i < data.size(); ++i) {
        if (data[i].empty()){
            continue;
        }

        auto area = RectangleArea(
                static_cast<float>(offset) * 2,
                1000.0f,
                uiScreenSize.y - height - (static_cast<float>(offset) * 2) + (static_cast<float>(i * 10)),
                1000.0f
        );

        uiRenderContext->drawDebugText(
                &area,
                &data[i],
                &mce::Color::WHITE,
                0.75f,
                ui::TextAlignment::Left,
                &textMeasureData,
                &caretMeasureData
        );
    }
}