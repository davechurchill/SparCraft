#include "AnimationFrameData.h"

using namespace SparCraft;

std::vector<AttackFrameData> AnimationFrameData::attackFrameData;

void AnimationFrameData::init()
{
    int maxTypeId = -1;
    for (const auto & unitType : BWAPI::UnitTypes::allUnitTypes())
    {
        const int unitTypeId = unitType.getID();
        if (unitTypeId > maxTypeId)
        {
            maxTypeId = unitTypeId;
        }
    }

    if (maxTypeId < 0)
    {
        attackFrameData.clear();
        return;
    }

    attackFrameData.assign(static_cast<size_t>(maxTypeId) + 1, AttackFrameData(0, 0));

    auto setAttackFrames = [&](const BWAPI::UnitType & unitType, const TimeType firstAttackFrame, const TimeType repeatAttackFrame)
    {
        const int unitTypeId = unitType.getID();
        if (unitTypeId < 0)
        {
            return;
        }

        const size_t unitTypeIndex = static_cast<size_t>(unitTypeId);
        if (unitTypeIndex >= attackFrameData.size())
        {
            return;
        }

        attackFrameData[unitTypeIndex] = AttackFrameData(firstAttackFrame, repeatAttackFrame);
    };

    // Protoss Units
    setAttackFrames(BWAPI::UnitTypes::Protoss_Probe, 2, 2);
    setAttackFrames(BWAPI::UnitTypes::Protoss_Zealot, 8, 7);
    setAttackFrames(BWAPI::UnitTypes::Protoss_Dragoon, 7, 3);
    setAttackFrames(BWAPI::UnitTypes::Protoss_Dark_Templar, 9, 9);
    setAttackFrames(BWAPI::UnitTypes::Protoss_Scout, 2, 2);
    setAttackFrames(BWAPI::UnitTypes::Protoss_Corsair, 8, 8);
    setAttackFrames(BWAPI::UnitTypes::Protoss_Arbiter, 2, 2);
    setAttackFrames(BWAPI::UnitTypes::Protoss_Archon, 1, 1);
    setAttackFrames(BWAPI::UnitTypes::Protoss_Photon_Cannon, 1, 1);

    // Terran Units
    setAttackFrames(BWAPI::UnitTypes::Terran_SCV, 2, 2);
    setAttackFrames(BWAPI::UnitTypes::Terran_Marine, 8, 6);
    setAttackFrames(BWAPI::UnitTypes::Terran_Firebat, 8, 8);
    setAttackFrames(BWAPI::UnitTypes::Terran_Ghost, 3, 2);
    setAttackFrames(BWAPI::UnitTypes::Terran_Vulture, 1, 1);
    setAttackFrames(BWAPI::UnitTypes::Terran_Goliath, 1, 1);
    setAttackFrames(BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode, 1, 1);
    setAttackFrames(BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode, 1, 1);
    setAttackFrames(BWAPI::UnitTypes::Terran_Wraith, 2, 2);
    setAttackFrames(BWAPI::UnitTypes::Terran_Battlecruiser, 2, 2);
    setAttackFrames(BWAPI::UnitTypes::Terran_Valkyrie, 40, 40);
    setAttackFrames(BWAPI::UnitTypes::Terran_Missile_Turret, 1, 1);

    // Zerg Units
    setAttackFrames(BWAPI::UnitTypes::Zerg_Drone, 2, 2);
    setAttackFrames(BWAPI::UnitTypes::Zerg_Zergling, 5, 5);
    setAttackFrames(BWAPI::UnitTypes::Zerg_Hydralisk, 3, 2);
    setAttackFrames(BWAPI::UnitTypes::Zerg_Lurker, 2, 2);
    setAttackFrames(BWAPI::UnitTypes::Zerg_Ultralisk, 14, 14);
    setAttackFrames(BWAPI::UnitTypes::Zerg_Mutalisk, 1, 1);
    setAttackFrames(BWAPI::UnitTypes::Zerg_Devourer, 9, 9);
    setAttackFrames(BWAPI::UnitTypes::Zerg_Sunken_Colony, 1, 1);
    setAttackFrames(BWAPI::UnitTypes::Zerg_Spore_Colony, 1, 1);
}

const AttackFrameData & AnimationFrameData::getAttackFrames(const BWAPI::UnitType & type)
{
    static const AttackFrameData defaultAttackFrames(0, 0);

    const int unitTypeId = type.getID();
    if (unitTypeId < 0)
    {
        return defaultAttackFrames;
    }

    const size_t unitTypeIndex = static_cast<size_t>(unitTypeId);
    if (unitTypeIndex >= attackFrameData.size())
    {
        return defaultAttackFrames;
    }

    return attackFrameData[unitTypeIndex];
}
