#include "BuiltinStages.hpp"

#include "Stage.hpp"
#include "../BaseWorld.hpp"

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
                    auto &stash = world->getCreatedTiles();
                    auto *payload = &stash[item]->getTileDescription();

                    auto event = KCore::MapEvent::MakeInFrustumEvent(item, (void *) payload);
                    world->pushToSyncEvents(event);
                }
            }
        });

        return stage;
    }
}