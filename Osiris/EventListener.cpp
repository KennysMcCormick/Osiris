#include <cassert>

#include "EventListener.h"
#include "fnv.h"
#include "GameData.h"
#include "Hacks/Misc.h"
#include "Hacks/SkinChanger.h"
#include "Hacks/Visuals.h"
#include "Interfaces.h"
#include "Memory.h"

EventListener::EventListener() noexcept
{
    assert(interfaces);

    interfaces->gameEventManager->addListener(this, "item_purchase");
    interfaces->gameEventManager->addListener(this, "round_start");
    interfaces->gameEventManager->addListener(this, "round_freeze_end");
    interfaces->gameEventManager->addListener(this, "player_hurt");

    interfaces->gameEventManager->addListener(this, "player_death");

#ifdef _WIN32
    if (const auto desc = memory->getEventDescriptor(interfaces->gameEventManager, "player_death", nullptr))
        std::swap(desc->listeners[0], desc->listeners[desc->listeners.size - 1]);
    else
        assert(false);
#endif
}

void EventListener::remove() noexcept
{
    assert(interfaces);

    interfaces->gameEventManager->removeListener(this);
}

void EventListener::fireGameEvent(GameEvent* event)
{
    switch (fnv::hashRuntime(event->getName())) {
    case fnv::hash("round_start"):
        GameData::clearProjectileList();
        Misc::preserveKillfeed(true);
        [[fallthrough]];
    case fnv::hash("item_purchase"):
    case fnv::hash("round_freeze_end"):
        Misc::purchaseList(event);
        break;
    case fnv::hash("player_death"):
        SkinChanger::updateStatTrak(*event);
        SkinChanger::overrideHudIcon(*event);
        Misc::killMessage(*event);
        Misc::killSound(*event);
        break;
    case fnv::hash("player_hurt"):
        Misc::playHitSound(*event);
        Visuals::hitEffect(event);
        Visuals::hitMarker(event);
        break;
    }
}
