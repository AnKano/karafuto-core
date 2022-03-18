#include "BuiltinStages.hpp"

#include "Stage.hpp"
#include "../BaseWorld.hpp"

#include "../../misc/Utils.hpp"

namespace KCore {
    KCore::Stage *BuiltInStages::CommonCalculate() {
        auto stage = new KCore::Stage([](KCore::BaseWorld *world, KCore::Stage *nextStage) {
            auto currTiles = world->getCurrentBaseTiles();
            auto prevTiles = world->getPreviousBaseTiles();

            auto diff = mapKeysDifference<std::string>(currTiles, prevTiles);
            auto inter = mapKeysIntersection<std::string>(currTiles, prevTiles);

            for (auto &item: diff) {
                bool inPrev = prevTiles.count(item) > 0;
                bool inNew = currTiles.count(item) > 0;

                if (inPrev) {
                    auto event = KCore::MapEvent::MakeNotInFrustumEvent(item);
                    world->pushToSyncEvents(event);
                }

                if (inNew) {
                    auto &stash = world->getCreatedBaseTiles();
                    auto *payload = &stash[item]->getTileDescription();

                    auto event = KCore::MapEvent::MakeInFrustumEvent(item, (void *) payload);
                    world->pushToSyncEvents(event);
                }
            }
        });

        return stage;
    }

    KCore::Stage *BuiltInStages::MetaCalculate() {
        auto stage = new KCore::Stage([](KCore::BaseWorld *world, KCore::Stage *nextStage) {
            auto currTiles = world->getCurrentMetaTiles();
            auto prevTiles = world->getPreviousMetaTiles();

            auto diff = mapKeysDifference<std::string>(currTiles, prevTiles);
            auto inter = mapKeysIntersection<std::string>(currTiles, prevTiles);

            for (auto &item: diff) {
                bool inPrev = prevTiles.count(item) > 0;
                bool inNew = currTiles.count(item) > 0;

                if (inPrev) {
                    auto event = KCore::MapEvent::MakeNotInFrustumEvent(item);
                    world->pushToSyncEvents(event);
//                    std::cout << "Meta tile not in frustum!" << std::endl;
                }

                if (inNew) {
                    auto &stash = world->getCreatedMetaTiles();
                    auto *payload = &stash[item]->getTileDescription();

                    auto event = KCore::MapEvent::MakeInFrustumEvent(item, (void *) payload);
                    world->pushToSyncEvents(event);
//                    std::cout << "Meta tile in frustum!" << std::endl;
                }
            }
        });

        return stage;
    }
}