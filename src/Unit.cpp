#include "Unit.h"
#include <algorithm>

using namespace SparCraft;

Unit::Unit()
    : m_unitType             (BWAPI::UnitTypes::None)
    , m_range                (0)
    , m_unitID               (255)
    , m_playerID             (255)
    , m_currentHP            (0)
    , m_currentEnergy        (0)
    , m_timeCanMove          (0)
    , m_timeCanAttack        (0)
    , m_previousActionTime   (0)
    , m_prevCurrentPosTime   (0)
{
    
}

// test constructor for setting all variables of a unit
Unit::Unit(const BWAPI::UnitType unitType, const Position & pos, const size_t & unitID, const size_t & playerID, 
           const HealthType & hp, const HealthType & energy, const TimeType & tm, const TimeType & ta) 
    : m_unitType             (unitType)
    , m_range                (PlayerWeapon(&PlayerProperties::Get(playerID), unitType.groundWeapon()).GetMaxRange() + Constants::Range_Addition)
    , m_position             (pos)
    , m_unitID               (unitID)
    , m_playerID             (playerID)
    , m_currentHP            (hp)
    , m_currentEnergy        (energy)
    , m_timeCanMove          (tm)
    , m_timeCanAttack        (ta)
    , m_previousActionTime   (0)
    , m_prevCurrentPosTime   (0)
    , m_previousPosition     (pos)
    , m_prevCurrentPos       (pos)
{
    System::checkSupportedUnitType(unitType);
}

// constructor for units to construct basic units, sets some things automatically
Unit::Unit(const BWAPI::UnitType unitType, const size_t & playerID, const Position & pos) 
    : m_unitType             (unitType)
    , m_range                (PlayerWeapon(&PlayerProperties::Get(playerID), unitType.groundWeapon()).GetMaxRange() + Constants::Range_Addition)
    , m_position             (pos)
    , m_unitID               (0)
    , m_playerID             (playerID)
    , m_currentHP            (maxHP())
    , m_currentEnergy        (unitType == BWAPI::UnitTypes::Terran_Medic ? Constants::Starting_Energy : 0)
    , m_timeCanMove          (0)
    , m_timeCanAttack        (0)
    , m_previousActionTime   (0)
    , m_prevCurrentPosTime   (0)
    , m_previousPosition     (pos)
    , m_prevCurrentPos       (pos)
{
    System::checkSupportedUnitType(unitType);
}

// Less than operator, used for sorting the GameState unit array.
// Units are sorted in this order:
//		1) alive < dead
//		2) firstTimeFree()
//		3) currentHP()
//		4) pos()
const bool Unit::operator < (const Unit & rhs) const
{
    if (!isAlive())
    {
        return false;
    }
    else if (!rhs.isAlive())
    {
        return true;
    }

    if (firstTimeFree() == rhs.firstTimeFree())
    {
        return ID() < rhs.ID();
    }
    else
    {
        return firstTimeFree() < rhs.firstTimeFree();
    }

    /*if (firstTimeFree() == rhs.firstTimeFree())
    {
        if (currentHP() == rhs.currentHP())
        {
            return pos() < rhs.pos();
        }
        else
        {
            return currentHP() < rhs.currentHP();
        }
    }
    else
    {
        return firstTimeFree() < rhs.firstTimeFree();
    }*/
}

// compares a unit based on unit id
const bool Unit::equalsID(const Unit & rhs) const
{ 
    return m_unitID == rhs.m_unitID; 
}
// returns whether or not this unit can see a given unit at a given time
bool Unit::canSeeTarget(const Unit & unit, const TimeType & gameTime) const
{

	// range of this unit attacking
	PositionType r = type().sightRange();

	// return whether the target unit is in range
	return (r * r) >= getDistanceSqToUnit(unit, gameTime);
}

// returns whether or not this unit can attack a given unit at a given time
const bool Unit::canAttackTarget(const Unit & unit, const TimeType & gameTime) const
{
    BWAPI::WeaponType weapon = unit.type().isFlyer() ? type().airWeapon() : type().groundWeapon();

    if (weapon.damageAmount() == 0)
    {
        return false;
    }

    // range of this unit attacking
    PositionType r = range();

    // return whether the target unit is in range
    return (r * r) >= getDistanceSqToUnit(unit, gameTime);
}

const bool Unit::canHealTarget(const Unit & unit, const TimeType & gameTime) const
{
    // if the unit can't heal or the target unit is not on the same team
    if (!canHeal() || !unit.isOrganic() || !(unit.player() == player()) || (unit.currentHP() == unit.maxHP()))
    {
        // then it can't heal the target
        return false;
    }

    // range of this unit attacking
    PositionType r = healRange();

    // return whether the target unit is in range
    return (r * r) >= getDistanceSqToUnit(unit, gameTime);
}

const Position & Unit::position() const
{
    return m_position;
}

// take an attack, subtract the hp
void Unit::takeAttack(const Unit & attacker)
{
    PlayerWeapon    weapon(attacker.getWeapon(*this));
    HealthType      damage(weapon.GetDamageBase());

    // calculate the damage based on armor and damage types
    damage = std::max((int)((damage-getArmor()) * weapon.GetDamageMultiplier(getSize())), 2);
    
    // special case where units attack multiple times
    if (attacker.type() == BWAPI::UnitTypes::Protoss_Zealot || attacker.type() == BWAPI::UnitTypes::Terran_Firebat)
    {
        damage *= 2;
    }

    //std::cout << type().getName() << " took " << (int)attacker.player() << " " << damage << "\n";

    updateCurrentHP(m_currentHP - damage);
}

void Unit::takeHeal(const Unit & healer)
{
    updateCurrentHP(m_currentHP + healer.healAmount());
}

// returns whether or not this unit is alive
const bool Unit::isAlive() const
{
    return m_currentHP > 0;
}

// attack a unit, set the times accordingly
void Unit::attack(const Action & move, const Unit & target, const TimeType & gameTime)
{
    // if this is a repeat attack
    if (m_previousAction.type() == ActionTypes::ATTACK || m_previousAction.type() == ActionTypes::RELOAD)
    {
        // add the repeat attack animation duration
        // can't attack again until attack cooldown is up
        updateMoveActionTime      (gameTime + attackRepeatFrameTime());
        updateAttackActionTime    (gameTime + attackCooldown());
    }
    // if there previous action was a MOVE action, add the move penalty
    else if (m_previousAction.type() == ActionTypes::MOVE)
    {
        updateMoveActionTime      (gameTime + attackInitFrameTime() + 2);
        updateAttackActionTime    (gameTime + attackCooldown() + Constants::Move_Penalty);
    }
    else
    {
        // add the initial attack animation duration
        updateMoveActionTime      (gameTime + attackInitFrameTime() + 2);
        updateAttackActionTime    (gameTime + attackCooldown());
    }

    // if the unit is not mobile, set its next move time to its next attack time
    if (!isMobile())
    {
        updateMoveActionTime(m_timeCanAttack);
    }

    setPreviousAction(move, gameTime);
}

// attack a unit, set the times accordingly
void Unit::heal(const Action & move, const Unit & target, const TimeType & gameTime)
{
    m_currentEnergy -= healCost();

    // can't attack again until attack cooldown is up
    updateAttackActionTime        (gameTime + healCooldown());
    updateMoveActionTime          (gameTime + healCooldown());

    if (currentEnergy() < healCost())
    {
        updateAttackActionTime(1000000);
    }

    setPreviousAction(move, gameTime);
}

// unit update for moving based on a given Move
void Unit::move(const Action & move, const TimeType & gameTime) 
{
    m_previousPosition = pos();

    // get the distance to the move action destination
    PositionType dist = move.pos().getDistance(pos());
    
    // how long will this move take?
    TimeType moveDuration = (TimeType)((double)dist / speed());

    // update the next time we can move, make sure a move always takes 1 time step
    updateMoveActionTime(gameTime + std::max(moveDuration, 1));

    // assume we need 4 frames to turn around after moving
    updateAttackActionTime(std::max(nextAttackActionTime(), nextMoveActionTime()));

    // update the position
    //m_position.addPosition(dist * dir.x(), dist * dir.y());
    m_position.moveTo(move.pos());

    setPreviousAction(move, gameTime);
}

// unit is commanded to wait until his attack cooldown is up
void Unit::waitUntilAttack(const Action & move, const TimeType & gameTime)
{
    // do nothing until we can attack again
    updateMoveActionTime(m_timeCanAttack);
    setPreviousAction(move, gameTime);
}

void Unit::pass(const Action & move, const TimeType & gameTime)
{
    updateMoveActionTime(gameTime + Constants::Pass_Move_Duration);
    updateAttackActionTime(gameTime + Constants::Pass_Move_Duration);
    setPreviousAction(move, gameTime);
}

const PositionType Unit::getDistanceSqToUnit(const Unit & u, const TimeType & gameTime) const 
{ 
    return getDistanceSqToPosition(u.currentPosition(gameTime), gameTime); 
}

const PositionType Unit::getDistanceSqToPosition(const Position & p, const TimeType & gameTime) const	
{ 
    return currentPosition(gameTime).getDistanceSq(p);
}

// returns current position based on game time
const Position & Unit::currentPosition(const TimeType & gameTime) const
{
    // if the previous move was MOVE, then we need to calculate where the unit is now
    if (m_previousAction.type() == ActionTypes::MOVE)
    {
        // if gameTime is equal to previous move time then we haven't moved yet
        if (gameTime == m_previousActionTime)
        {
            return m_previousPosition;
        }
        // else if game time is >= time we can move, then we have arrived at the destination
        else if (gameTime >= m_timeCanMove)
        {
            return m_position;
        }
        // otherwise we are still moving, so calculate the current position
        else if (gameTime == m_prevCurrentPosTime)
        {
            return m_prevCurrentPos;
        }
        else
        {
            TimeType moveDuration = m_timeCanMove - m_previousActionTime;
            float moveTimeRatio = (float)(gameTime - m_previousActionTime) / moveDuration;
            m_prevCurrentPosTime = gameTime;

            // calculate the new current position
            m_prevCurrentPos = m_position;
            m_prevCurrentPos.subtractPosition(m_previousPosition);
            m_prevCurrentPos.scalePosition(moveTimeRatio);
            m_prevCurrentPos.addPosition(m_previousPosition);

            //m_prevCurrentPos = m_previousPosition + (m_position - m_previousPosition).scale(moveTimeRatio);
            return m_prevCurrentPos;
        }
    }
    // if it wasn't a MOVE, then we just return the Unit position
    else
    {
        return m_position;
    }
}

void Unit::setPreviousPosition(const TimeType & gameTime)
{
    TimeType moveDuration = m_timeCanMove - m_previousActionTime;
    float moveTimeRatio = (float)(gameTime - m_previousActionTime) / moveDuration;
    m_prevCurrentPosTime = gameTime;
    m_prevCurrentPos = m_previousPosition + (m_position - m_previousPosition).scale(moveTimeRatio);
}

// returns the damage a unit does
const HealthType Unit::damage() const	
{ 
    return m_unitType == BWAPI::UnitTypes::Protoss_Zealot ? 
        2 * (HealthType)m_unitType.groundWeapon().damageAmount() : 
    (HealthType)m_unitType.groundWeapon().damageAmount(); 
}

const HealthType Unit::healAmount() const
{
    return canHeal() ? 6 : 0;
}

void Unit::print() const 
{ 
    printf("%s %5d [%5d %5d] (%5d, %5d)\n", m_unitType.getName().c_str(), currentHP(), nextAttackActionTime(), nextMoveActionTime(), x(), y()); 
}

void Unit::updateCurrentHP(const HealthType & newHP) 
{ 
    m_currentHP = std::min(maxHP(), newHP); 
}

void Unit::updateAttackActionTime(const TimeType & newTime)
{ 
    m_timeCanAttack = newTime; 
}

void Unit::updateMoveActionTime(const TimeType & newTime)
{ 
    m_timeCanMove = newTime; 
} 

void Unit::setCooldown(TimeType attack, TimeType move)
{ 
    m_timeCanAttack = attack; m_timeCanMove = move; 
}

void Unit::setUnitID(const size_t & id)
{ 
    m_unitID = id; 
}

void Unit::setPreviousAction(const Action & m, const TimeType & previousMoveTime) 
{	
    // if it was an attack move, store the unitID of the opponent unit
    m_previousAction = m;
    m_previousActionTime = previousMoveTime; 
}

const bool Unit::canAttackNow() const
{ 
    return !canHeal() && m_timeCanAttack <= m_timeCanMove; 
}

const bool Unit::canMoveNow() const
{ 
    return isMobile() && m_timeCanMove <= m_timeCanAttack; 
}

const bool Unit::canHealNow() const
{ 
    return canHeal() && (currentEnergy() >= healCost()) && (m_timeCanAttack <= m_timeCanMove); 
}

const bool Unit::canKite() const
{ 
    return m_timeCanMove < m_timeCanAttack; 
}

const bool Unit::isMobile() const
{ 
    return m_unitType.canMove(); 
}

const bool Unit::canHeal() const
{ 
    return m_unitType == BWAPI::UnitTypes::Terran_Medic; 
}

const bool Unit::isOrganic() const
{ 
    return m_unitType.isOrganic(); 
}

const size_t Unit::ID() const	
{ 
    return m_unitID; 
}

const size_t Unit::player() const
{ 
    return m_playerID; 
}

const Position & Unit::pos() const
{ 
    return m_position; 
}

const PositionType Unit::x() const 
{ 
    return m_position.x(); 
}

const PositionType Unit::y() const 
{ 
    return m_position.y(); 
}

const PositionType Unit::range() const 
{ 
    return m_range; 
}

const PositionType Unit::healRange() const
{ 
    return canHeal() ? 96 : 0; 
}

const HealthType Unit::maxHP() const 
{ 
    return (HealthType)m_unitType.maxHitPoints() + (HealthType)m_unitType.maxShields(); 
}

const HealthType Unit::currentHP() const 
{ 
    return (HealthType)m_currentHP; 
}

const HealthType Unit::currentEnergy() const 
{ 
    return (HealthType)m_currentEnergy; 
}

const HealthType Unit::maxEnergy() const
{ 
    return (HealthType)m_unitType.maxEnergy(); 
}

const HealthType Unit::healCost() const	
{ 
    return 3; 
}

const float Unit::dpf() const 
{ 
    return (float)std::max(Constants::Min_Unit_DPF, (float)damage() / ((float)attackCooldown() + 1)); 
}

const TimeType Unit::moveCooldown() const 
{ 
    return (TimeType)((double)Constants::Move_Distance / m_unitType.topSpeed()); 
}

const TimeType Unit::attackCooldown() const 
{ 
    return (TimeType)m_unitType.groundWeapon().damageCooldown(); 
}

const TimeType Unit::healCooldown() const 
{ 
    return (TimeType)8; 
}

const TimeType Unit::nextAttackActionTime() const 
{ 
    return m_timeCanAttack; 
}

const TimeType Unit::nextMoveActionTime() const	
{ 
    return m_timeCanMove; 
}

const TimeType Unit::previousActionTime() const	
{ 
    return m_previousActionTime; 
}

const TimeType Unit::firstTimeFree() const	
{ 
    return m_timeCanAttack <= m_timeCanMove ? m_timeCanAttack : m_timeCanMove; 
}

const TimeType Unit::attackInitFrameTime() const	
{ 
    return AnimationFrameData::getAttackFrames(m_unitType).first; 
}

const TimeType Unit::attackRepeatFrameTime() const	
{
    return AnimationFrameData::getAttackFrames(m_unitType).second; 
}

const int Unit::typeID() const	
{ 
    return m_unitType.getID(); 
}

const double Unit::speed() const 
{ 
    return m_unitType.topSpeed(); 
}

const BWAPI::UnitType Unit::type() const 
{ 
    return m_unitType; 
}

const Action & Unit::previousAction() const 
{ 
    return m_previousAction; 
}

const BWAPI::UnitSizeType Unit::getSize() const
{
    return m_unitType.size();
}

const PlayerWeapon Unit::getWeapon(const Unit & target) const
{
    return PlayerWeapon(&PlayerProperties::Get(player()), target.type().isFlyer() ? m_unitType.airWeapon() : m_unitType.groundWeapon());
}

const HealthType Unit::getArmor() const
{
    return UnitProperties::Get(type()).GetArmor(PlayerProperties::Get(player())); 
}

const BWAPI::WeaponType Unit::getWeapon(BWAPI::UnitType target) const
{
    return target.isFlyer() ? m_unitType.airWeapon() : m_unitType.groundWeapon();
}

const std::string Unit::name() const 
{ 
    std::string n(m_unitType.getName());
    std::replace(n.begin(), n.end(), ' ', '_');
    return n;
}

// calculates the hash of this unit based on a given game time
const HashType Unit::calculateHash(const size_t & hashNum, const TimeType & gameTime) const
{
    Position currentPos = currentPosition(gameTime);

    return	  Hash::values[hashNum].positionHash(m_playerID, currentPos.x(), currentPos.y()) 
            ^ Hash::values[hashNum].getAttackHash(m_playerID, nextAttackActionTime() - gameTime) 
            ^ Hash::values[hashNum].getMoveHash(m_playerID, nextMoveActionTime() - gameTime)
            ^ Hash::values[hashNum].getCurrentHPHash(m_playerID, currentHP())
            ^ Hash::values[hashNum].getUnitTypeHash(m_playerID, typeID());
}

// calculates the hash of this unit based on a given game time, and prints debug info
void Unit::debugHash(const size_t & hashNum, const TimeType & gameTime) const
{
    std::cout << " Pos   " << Hash::values[hashNum].positionHash(m_playerID, position().x(), position().y());
    std::cout << " Att   " << Hash::values[hashNum].getAttackHash(m_playerID, nextAttackActionTime() - gameTime);
    std::cout << " Mov   " << Hash::values[hashNum].getMoveHash(m_playerID, nextMoveActionTime() - gameTime);
    std::cout << " HP    " << Hash::values[hashNum].getCurrentHPHash(m_playerID, currentHP());
    std::cout << " Typ   " << Hash::values[hashNum].getUnitTypeHash(m_playerID, typeID()) << "\n";;

    HashType hash = Hash::values[hashNum].positionHash(m_playerID, position().x(), position().y()); std::cout << hash << "\n";
    hash ^= Hash::values[hashNum].getAttackHash(m_playerID, nextAttackActionTime() - gameTime) ; std::cout << hash << "\n";
    hash ^= Hash::values[hashNum].getMoveHash(m_playerID, nextMoveActionTime() - gameTime); std::cout << hash << "\n";
    hash ^= Hash::values[hashNum].getCurrentHPHash(m_playerID, currentHP()); std::cout << hash << "\n";
    hash ^= Hash::values[hashNum].getUnitTypeHash(m_playerID, typeID()); std::cout << hash << "\n";
}

const std::string Unit::debugString() const
{
    std::stringstream ss;

    ss << "Unit Type:           " << type().getName()                               << "\n";
    ss << "Unit ID:             " << (int)ID()                                      << "\n";
    ss << "Player:              " << (int)player()                                  << "\n";
    ss << "Range:               " << range()                                        << "\n";
    ss << "Position:            " << "(" << m_position.x() << "," << m_position.y()   << ")\n";
    ss << "Current HP:          " << currentHP()                                    << "\n";
    ss << "Next Move Time:      " << nextMoveActionTime()                           << "\n";
    ss << "Next Attack Time:    " << nextAttackActionTime()                         << "\n";
    ss << "Previous Action:     " << previousAction().debugString()                 << "\n";
    ss << "Previous Pos Time:   " << m_prevCurrentPosTime                            << "\n";
    ss << "Previous Pos:        " << "(" << m_prevCurrentPos.x() << "," << m_prevCurrentPos.y()   << ")\n";

    return ss.str();
}

