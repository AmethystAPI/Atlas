//
// Created by adria on 12/02/2024.
//

#include "f3DataBuilder.h"
#include "../../recorder/ps_recorder/psRecorder.h"

std::vector<std::string> f3DataBuilder::BuildInfoData(ClientInstance *clientInstance) {
    std::vector<std::string> data;

    Vec3* playerPos = clientInstance->getLocalPlayer()->getPosition();
    Vec2 clientUIScreenSize = clientInstance->guiData->clientUIScreenSize;
    Vec2 clientScreenSize = clientInstance->guiData->clientScreenSize;
    Vec2 totalScreenSize = clientInstance->guiData->totalScreenSize;

    Vec3 tpsFpsUps = psRecorder::getData();

    auto* component = clientInstance->getLocalPlayer()->tryGetComponent<ActorRotationComponent>();
    Vec2 headRot = component->mHeadRot;

    float cardinalRot = headRot.y + 180.0f;
    std::string direction;

    if (cardinalRot >= 315.0f || cardinalRot < 45.0f) direction = "North (Towards negative Z)";
    else if (cardinalRot >= 45.0f && cardinalRot < 135.0f) direction = "East (Towards positive X)";
    else if (cardinalRot >= 135.0f && cardinalRot < 225.0f) direction = "South (Towards positive Z)";
    else direction = "West (Towards negative X)";

    data.emplace_back(std::format("Minecraft Version: 1.20.51.1"));
    data.emplace_back(std::format("Amethyst Version: 1.2.0"));
    data.emplace_back("");
    data.emplace_back(std::format("TPS: {:.2f}", tpsFpsUps.x));
    data.emplace_back(std::format("FPS: {:.2f}", tpsFpsUps.y));
    data.emplace_back(std::format("UPS: {:.2f}", tpsFpsUps.z));
    data.emplace_back("");
    data.emplace_back(std::format("Position: [ x: {:.2f}, y: {:.2f}, z: {:.2f} ]", playerPos->x, playerPos->y, playerPos->z));
    data.emplace_back(std::format("Biome: Savanna"));
    data.emplace_back("");
    data.emplace_back(std::format("Rotation: [ x: {:.2f}, y: {:.2f} ]", headRot.x, headRot.y));
    data.emplace_back(std::format("Facing: {}", direction));
    data.emplace_back("");
    data.emplace_back(std::format("Client UI Screen Size: [ x: {}, y: {} ]", clientUIScreenSize.x, clientUIScreenSize.y));
    data.emplace_back(std::format("Client Screen Size: [ x: {}, y: {} ]", clientScreenSize.x, clientScreenSize.y));
    data.emplace_back(std::format("Total Screen Size: [ x: {}, y: {} ]", totalScreenSize.x, totalScreenSize.y));

    return data;
}

std::vector<std::string> f3DataBuilder::BuildControlData(ClientInstance *clientInstance) {
    std::vector<std::string> data;

    data.emplace_back("Graph: F3 + g");
    data.emplace_back("Chunk Borders: F3 + k");
    data.emplace_back("Current Block: F3 + y");
    data.emplace_back("Current Entity: F3 + y");
    data.emplace_back("Other: F3 + y");

    return data;
}
