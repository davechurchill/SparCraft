#include "GameState.h"
#include "Player.h"
#include "Game.h"

using namespace SparCraft;

#define TABS(N) for (int i(0); i<N; ++i) { fprintf(stderr, "\t"); }

class UnitIndexCompare
{
    const GameState & state;
    int player;

public:

    UnitIndexCompare(const GameState & s, const int p)
        : state(s)
        , player(p)
    {

    }

	const bool operator() (const int u1, const int u2) const
	{
        return state.getUnitDirect(player, u1) < state.getUnitDirect(player, u2);
    }
};

// default constructor
GameState::GameState()
	: m_map(NULL)
	, m_currentTime(0)
	, m_maxUnits(Constants::Max_Units)
    , m_turnsWithNoHPChange(0)
{
	m_numUnits.fill(0);
	m_prevNumUnits.fill(0);
	m_numMovements.fill(0);
    m_prevHPSum.fill(0);

    m_units[0] = std::vector<Unit>(Constants::Max_Units, Unit());
    m_units[1] = std::vector<Unit>(Constants::Max_Units, Unit());
    m_unitIndex[0] = std::vector<int>(Constants::Max_Units, 0);
    m_unitIndex[1] = std::vector<int>(Constants::Max_Units, 0);

	for (size_t u(0); u<m_maxUnits; ++u)
	{
        m_unitIndex[0][u] = (int)u;
		m_unitIndex[1][u] = (int)u;
	}
}

// call this whenever we are done with moves
void GameState::finishedMoving()
{
	// sort the unit vector based on time left to move
	sortUnits();

	// update the current time of the state
	updateGameTime();

    // calculate the hp sum of each player
    int hpSum[2];
    for (size_t p(0); p<Constants::Num_Players; ++p)
	{
		hpSum[p] = 0;

		for (size_t u(0); u<numUnits(p); ++u)
		{ 
            hpSum[p] += getUnit(p, u).currentHP();
        }
    }

    // if the hp sums match the last hp sum
    if (hpSum[0] == m_prevHPSum[0] && hpSum[1] == m_prevHPSum[1])
    {
        m_turnsWithNoHPChange++;
    }
    else
    {
        m_turnsWithNoHPChange = 0;
    }

    for (size_t p(0); p<Constants::Num_Players; ++p)
	{
        m_prevHPSum[p] = hpSum[p];
    }
}

const HashType GameState::calculateHash(const size_t hashNum) const
{
	HashType hash(0);

	for (size_t p(0); p < Constants::Num_Players; ++p)
	{
		for (size_t u(0); u < m_numUnits[p]; ++u)
		{
			hash ^= Hash::magicHash(getUnit(p,u).calculateHash(hashNum, m_currentTime), p, u);
		}
	}

	return hash;
}

void GameState::generateMoves(MoveArray & moves, const size_t playerIndex) const
{
	moves.clear();

    // which is the enemy player
	size_t enemyPlayer  = getEnemy(playerIndex);

    // make sure this player can move right now
    const size_t canMove(whoCanMove());
    if (canMove == enemyPlayer)
    {
        System::FatalError("GameState Error - Called generateMoves() for a player that cannot currently move");
    }

	// we are interested in all simultaneous moves
	// so return all units which can move at the same time as the first
	TimeType firstUnitMoveTime = getUnit(playerIndex, 0).firstTimeFree();
		
	for (size_t unitIndex(0); unitIndex < m_numUnits[playerIndex]; ++unitIndex)
	{
		// unit reference
		const Unit & unit(getUnit(playerIndex,unitIndex));
			
		// if this unit can't move at the same time as the first
		if (unit.firstTimeFree() != firstUnitMoveTime)
		{
			// stop checking
			break;
		}

		if (unit.previousActionTime() == m_currentTime && m_currentTime != 0)
		{
            System::FatalError("Previous Move Took 0 Time: " + unit.previousAction().moveString());
		}

		moves.addUnit();

		// generate attack moves
		if (unit.canAttackNow())
		{
			for (size_t u(0); u<m_numUnits[enemyPlayer]; ++u)
			{
				const Unit & enemyUnit(getUnit(enemyPlayer, u));
				bool invisible = false;
				if (enemyUnit.type().hasPermanentCloak())
				{
					invisible = true;
					for (size_t detectorIndex(0); detectorIndex < m_numUnits[playerIndex]; ++detectorIndex)
					{
						// unit reference
						const Unit & detector(getUnit(playerIndex, detectorIndex));
						if (detector.type().isDetector() && detector.canSeeTarget(enemyUnit, m_currentTime))
						{
							invisible = false;
							break;
						}
					}
				}
				if (!invisible && unit.canAttackTarget(enemyUnit, m_currentTime) && enemyUnit.isAlive())
				{
					moves.add(Action(unitIndex, playerIndex, ActionTypes::ATTACK, u));
                    //moves.add(Action(unitIndex, playerIndex, ActionTypes::ATTACK, unit.ID()));
				}
			}
		}
		else if (unit.canHealNow())
		{
			for (size_t u(0); u<m_numUnits[playerIndex]; ++u)
			{
				// units cannot heal themselves in broodwar
				if (u == unitIndex)
				{
					continue;
				}

				const Unit & ourUnit(getUnit(playerIndex, u));
				if (unit.canHealTarget(ourUnit, m_currentTime) && ourUnit.isAlive())
				{
					moves.add(Action(unitIndex, playerIndex, ActionTypes::HEAL, u));
                    //moves.add(Action(unitIndex, playerIndex, ActionTypes::HEAL, unit.ID()));
				}
			}
		}
		// generate the wait move if it can't attack yet
		else
		{
			if (!unit.canHeal())
			{
				moves.add(Action(unitIndex, playerIndex, ActionTypes::RELOAD, 0));
			}
		}
		
		// generate movement moves
		if (unit.isMobile())
		{
            // In order to not move when we could be shooting, we want to move for the minimum of:
            // 1) default move distance move time
            // 2) time until unit can attack, or if it can attack, the next cooldown
            double timeUntilAttack          = unit.nextAttackActionTime() - getTime();
            timeUntilAttack                 = timeUntilAttack == 0 ? unit.attackCooldown() : timeUntilAttack;

            // the default move duration
            double defaultMoveDuration      = (double)Constants::Move_Distance / unit.speed();

            // if we can currently attack
			double chosenTime = timeUntilAttack != 0 ? std::min(timeUntilAttack, defaultMoveDuration) : defaultMoveDuration;

            // the chosen movement distance
            PositionType moveDistance       = (PositionType)(chosenTime * unit.speed());

            // DEBUG: If chosen move distance is ever 0, something is wrong
            if (moveDistance == 0)
            {
                System::FatalError("Move Action with distance 0 generated. timeUntilAttack:"+
					std::to_string(timeUntilAttack)+", speed:"+std::to_string(unit.speed()));
            }

            // we are only generating moves in the cardinal direction specified in common.h
			for (size_t d(0); d<Constants::Num_Directions; ++d)
			{
                // the direction of this movement
              	Position dir(Constants::Move_Dir[d][0], Constants::Move_Dir[d][1]);

                // the final destination position of the unit
                Position dest = unit.pos() + Position(moveDistance*dir.x(), moveDistance*dir.y());

                // if that poisition on the map is walkable
                if (isWalkable(dest) || (unit.type().isFlyer() && isFlyable(dest)))
				{
                    // add the move to the MoveArray
					moves.add(Action(unitIndex, playerIndex, ActionTypes::MOVE, d, dest));
				}
			}
		}

		// if no moves were generated for this unit, it must be issued a 'PASS' move
		if (moves.numMoves(unitIndex) == 0)
		{
			moves.add(Action(unitIndex, playerIndex, ActionTypes::PASS, 0));
		}
	}
}


void GameState::makeMoves(const std::vector<Action> & moves, const bool ignoreCanMoveCheck)
{    
    if (!ignoreCanMoveCheck && moves.size() > 0)
    {
        const size_t canMove(whoCanMove());
        const size_t playerToMove(moves[0].player());
        if (canMove == getEnemy(playerToMove))
        {
            System::FatalError("GameState Error - Called makeMove() for a player that cannot currently move");
        }
    }
    
    for (size_t m(0); m<moves.size(); ++m)
    {
        performAction(moves[m]);
    }
}

void GameState::performAction(const Action & move)
{
	Unit & ourUnit		= getUnit(move.player(), move.unit());
	size_t player		= ourUnit.player();
	size_t enemyPlayer  = getEnemy(player);

	if (move.type() == ActionTypes::ATTACK)
	{
		Unit & enemyUnit(getUnit(enemyPlayer,move.index()));

		// if the target was already killed by a previous action this round, treat as a pass
		if (!enemyUnit.isAlive())
		{
			ourUnit.pass(move, m_currentTime);
		}
		else
		{
			// attack the unit and apply damage
			ourUnit.attack(move, enemyUnit, m_currentTime);
			enemyUnit.takeAttack(ourUnit);

			// check to see if enemy unit died
			if (!enemyUnit.isAlive())
			{
				m_numUnits[enemyPlayer]--;
			}
		}
	}
	else if (move.type() == ActionTypes::MOVE)
	{
		m_numMovements[player]++;

		ourUnit.move(move, m_currentTime);
	}
	else if (move.type() == ActionTypes::HEAL)
	{
		Unit & ourOtherUnit(getUnit(player,move.index()));
			
		// attack the unit
		ourUnit.heal(move, ourOtherUnit, m_currentTime);
			
		if (ourOtherUnit.isAlive())
		{
			ourOtherUnit.takeHeal(ourUnit);
		}
	}
	else if (move.type() == ActionTypes::RELOAD)
	{
		ourUnit.waitUntilAttack(move, m_currentTime);
	}
	else if (move.type() == ActionTypes::PASS)
	{
		ourUnit.pass(move, m_currentTime);
	}
}

const Unit & GameState::getUnitByID(const size_t unitID) const
{
	for (size_t p(0); p<Constants::Num_Players; ++p)
	{
		for (size_t u(0); u<numUnits(p); ++u)
		{
			if (getUnit(p, u).ID() == unitID)
			{
				return getUnit(p, u);
			}
		}
	}

	System::FatalError("GameState Error: getUnitByID() Unit not found, id:" + std::to_string(unitID));
	return getUnit(0,0);
}

const Unit & GameState::getUnitByID(const size_t player, const size_t unitID) const
{
	for (size_t u(0); u<numUnits(player); ++u)
	{
		if (getUnit(player, u).ID() == unitID)
		{
			return getUnit(player, u);
		}
	}

	System::FatalError("GameState Error: getUnitByID() Unit not found, player:"+std::to_string(player)+" id:" + std::to_string(unitID));
	return getUnit(0,0);
}

Unit & GameState::getUnitByID(const size_t player, const size_t unitID) 
{
	for (size_t u(0); u<numUnits(player); ++u)
	{
		if (getUnit(player, u).ID() == unitID)
		{
			return getUnit(player, u);
		}
	}

	System::FatalError("GameState Error: getUnitByID() Unit not found, player:" + std::to_string(player) + " id:" + std::to_string(unitID));
	return getUnit(0,0);
}

const bool GameState::isWalkable(const Position & pos) const
{
	if (m_map)
	{
		return m_map->isWalkable(pos);
	}

	// if there is no map, then return true
	return true;
}

const bool GameState::isFlyable(const Position & pos) const
{
	if (m_map)
	{
		return m_map->isFlyable(pos);
	}

	// if there is no map, then return true
	return true;
}

const size_t GameState::getEnemy(const size_t player) const
{
	return (player + 1) % 2;
}

const Unit & GameState::getClosestOurUnit(const size_t player, const size_t unitIndex)
{
	const Unit & myUnit(getUnit(player,unitIndex));

	size_t minDist(1000000);
	size_t minUnitInd(unitIndex);
    bool foundUnit(false);

	Position currentPos = myUnit.currentPosition(m_currentTime);

	for (size_t u(0); u<m_numUnits[player]; ++u)
	{
		if (u == unitIndex || getUnit(player, u).canHeal())
		{
			continue;
		}

		//size_t distSq(myUnit.distSq(getUnit(enemyPlayer,u)));
		size_t distSq(currentPos.getDistanceSq(getUnit(player, u).currentPosition(m_currentTime)));

		if (distSq < minDist)
		{
			minDist = distSq;
			minUnitInd = u;
            foundUnit = true;
		}
	}

	return foundUnit ? getUnit(player, minUnitInd) : myUnit;
}

const Unit & GameState::getClosestEnemyUnit(const size_t player, const size_t unitIndex, bool checkCloaked)
{
	const size_t enemyPlayer(getEnemy(player));
	const Unit & myUnit(getUnit(player,unitIndex));

    if (m_numUnits[enemyPlayer] == 0)
    {
        return myUnit;
    }

	PositionType minDist(1000000);
	size_t minUnitInd(0);
    size_t minUnitID(std::numeric_limits<size_t>::max());
    bool foundUnit(false);

	Position currentPos = myUnit.currentPosition(m_currentTime);

	for (size_t u(0); u<m_numUnits[enemyPlayer]; ++u)
	{
        Unit & enemyUnit(getUnit(enemyPlayer, u));
		if (checkCloaked&& enemyUnit.type().hasPermanentCloak())
		{
			bool invisible = true;
			for (size_t detectorIndex(0); detectorIndex < m_numUnits[player]; ++detectorIndex)
			{
				// unit reference
				const Unit & detector(getUnit(player, detectorIndex));
				if (detector.type().isDetector() && detector.canSeeTarget(enemyUnit, m_currentTime))
				{
					invisible = false;
					break;
				}
			}
			if (invisible)
			{
				continue;
			}
		}
        PositionType distSq = myUnit.getDistanceSqToUnit(enemyUnit, m_currentTime);

		if ((distSq < minDist))// || ((distSq == minDist) && (enemyUnit.ID() < minUnitID)))
		{
			minDist = distSq;
			minUnitInd = u;
            minUnitID = enemyUnit.ID();
            foundUnit = true;
		}
        else if ((distSq == minDist) && (enemyUnit.ID() < minUnitID))
        {
            minDist = distSq;
			minUnitInd = u;
            minUnitID = enemyUnit.ID();
            foundUnit = true;
        }
	}

	return foundUnit ? getUnit(enemyPlayer, minUnitInd) : getUnit(enemyPlayer, 0);
}

void GameState::checkFull(const size_t player) const
{
    if (numUnits(player) >= Constants::Max_Units)
    {
        std::stringstream ss;
        ss << "GameState has too many units. Constants::Max_Units = " << Constants::Max_Units;
        System::FatalError(ss.str());
    }
}

// Add a given unit to the state
// This function will give the unit a unique unitID
void GameState::addUnit(const Unit & u)
{
    checkFull(u.player());
    System::checkSupportedUnitType(u.type());

    // Calculate the unitID for this unit
    // This will just be the current total number of units in the state
    size_t unitID = m_numUnits[Players::Player_One] + m_numUnits[Players::Player_Two];

    // Set the unit and it's unitID
		m_units[u.player()][m_numUnits[u.player()]] = u;
	    m_units[u.player()][m_numUnits[u.player()]].setUnitID(unitID);

    // Increment the number of units this player has
	m_numUnits[u.player()]++;
	m_prevNumUnits[u.player()]++;

    // And do the clean-up
	finishedMoving();
	calculateStartingHealth();

    if (!checkUniqueUnitIDs())
    {
        System::FatalError("GameState has non-unique Unit ID values");
    }
}

// Add a unit with given parameters to the state
// This function will give the unit a unique unitID
void GameState::addUnit(const BWAPI::UnitType type, const size_t playerID, const Position & pos)
{
    checkFull(playerID);
    System::checkSupportedUnitType(type);

    // Calculate the unitID for this unit
    // This will just be the current total number of units in the state
    size_t unitID = m_numUnits[Players::Player_One] + m_numUnits[Players::Player_Two];

    // Set the unit and it's unitID
		m_units[playerID][m_numUnits[playerID]] = Unit(type, playerID, pos);
	    m_units[playerID][m_numUnits[playerID]].setUnitID(unitID);

    // Increment the number of units this player has
	m_numUnits[playerID]++;
	m_prevNumUnits[playerID]++;

    // And do the clean-up
	finishedMoving();
	calculateStartingHealth();

    if (!checkUniqueUnitIDs())
    {
        System::FatalError("GameState has non-unique Unit ID values");
    }
}

// Add a given unit to the state
// This function will keep the unit ID assigned by player. Only use this for advanced / BWAPI states
void GameState::addUnitWithID(const Unit & u)
{
    checkFull(u.player());
    System::checkSupportedUnitType(u.type());

    // Simply add the unit to the array
		m_units[u.player()][m_numUnits[u.player()]] = u;

    // Increment the number of units this player has
	m_numUnits[u.player()]++;
	m_prevNumUnits[u.player()]++;

    // And do the clean-up
	finishedMoving();
	calculateStartingHealth();

    if (!checkUniqueUnitIDs())
    {
        System::FatalError("GameState has non-unique Unit ID values");
    }
}

void GameState::sortUnits()
{
	// sort the units based on time free
	for (size_t p(0); p<Constants::Num_Players; ++p)
	{
		if (m_prevNumUnits[p] <= 1)
		{
			m_prevNumUnits[p] = m_numUnits[p];
			continue;
		}
		else
		{
            std::sort(&m_unitIndex[p][0], &m_unitIndex[p][0] + m_prevNumUnits[p], UnitIndexCompare(*this, (int)p));
			m_prevNumUnits[p] = m_numUnits[p];
		}
	}	
}

Unit & GameState::getUnit(const size_t player, const size_t unitIndex)
{
    if (player >= Constants::Num_Players)
    {
        System::FatalError("GameState::getUnit() invalid player index: " + std::to_string(player));
    }

    const size_t accessibleUnits = std::max(m_numUnits[player], m_prevNumUnits[player]);
    if (unitIndex >= accessibleUnits)
    {
        System::FatalError(
            "GameState::getUnit() unit index out of range. player=" + std::to_string(player) +
            " unitIndex=" + std::to_string(unitIndex) +
            " accessibleUnits=" + std::to_string(accessibleUnits));
    }

    const int storageIndex = m_unitIndex[player][unitIndex];
    if (storageIndex < 0 || static_cast<size_t>(storageIndex) >= m_units[player].size())
    {
        System::FatalError(
            "GameState::getUnit() storage index out of range. player=" + std::to_string(player) +
            " unitIndex=" + std::to_string(unitIndex) +
            " storageIndex=" + std::to_string(storageIndex) +
            " unitStorageSize=" + std::to_string(m_units[player].size()));
    }

    return m_units[player][static_cast<size_t>(storageIndex)];
}

const Unit & GameState::getUnit(const size_t player, const size_t unitIndex) const
{
    if (player >= Constants::Num_Players)
    {
        System::FatalError("GameState::getUnit() const invalid player index: " + std::to_string(player));
    }

    const size_t accessibleUnits = std::max(m_numUnits[player], m_prevNumUnits[player]);
    if (unitIndex >= accessibleUnits)
    {
        System::FatalError(
            "GameState::getUnit() const unit index out of range. player=" + std::to_string(player) +
            " unitIndex=" + std::to_string(unitIndex) +
            " accessibleUnits=" + std::to_string(accessibleUnits));
    }

    const int storageIndex = m_unitIndex[player][unitIndex];
    if (storageIndex < 0 || static_cast<size_t>(storageIndex) >= m_units[player].size())
    {
        System::FatalError(
            "GameState::getUnit() const storage index out of range. player=" + std::to_string(player) +
            " unitIndex=" + std::to_string(unitIndex) +
            " storageIndex=" + std::to_string(storageIndex) +
            " unitStorageSize=" + std::to_string(m_units[player].size()));
    }

    return m_units[player][static_cast<size_t>(storageIndex)];
}

const size_t GameState::closestEnemyUnitDistance(const Unit & unit) const
{
	size_t enemyPlayer(getEnemy(unit.player()));

	size_t closestDist(std::numeric_limits<size_t>::max());

	for (size_t u(0); u<numUnits(enemyPlayer); ++u)
	{
        size_t dist(unit.getDistanceSqToUnit(getUnit(enemyPlayer, u), m_currentTime));

		if (dist < closestDist)
		{
			closestDist = dist;
		}
	}

	return closestDist;
}

const bool GameState::playerDead(const size_t player) const
{
	if (numUnits(player) <= 0)
	{
		return true;
	}

	for (size_t u(0); u<numUnits(player); ++u)
	{
		if (getUnit(player, u).damage() > 0)
		{
			return false;
		}
	}

	return true;
}

const size_t GameState::whoCanMove() const
{
    if (m_numUnits[0] == 0 && m_numUnits[1] == 0) { return Players::Player_Both; }
	if (m_numUnits[0] == 0) { return Players::Player_Two; }
	if (m_numUnits[1] == 0) { return Players::Player_One; }

	TimeType p1Time(getUnit(0,0).firstTimeFree());
	TimeType p2Time(getUnit(1,0).firstTimeFree());

	// if player one is to move first
	if (p1Time < p2Time)
	{
		return Players::Player_One;
	}
	// if player two is to move first
	else if (p1Time > p2Time)
	{
		return Players::Player_Two;
	}
	else
	{
		return Players::Player_Both;
	}
}

const bool GameState::checkUniqueUnitIDs() const
{
    std::set<size_t> unitIDs;

    for (size_t p(0); p<Constants::Num_Players; ++p)
    {
        for (size_t u(0); u<numUnits(p); ++u)
        {
            size_t unitID(getUnit(p, u).ID());
            if (unitIDs.find(unitID) != unitIDs.end())
            {
                return false;
            }
            else
            {
                unitIDs.insert(unitID);
            }
        }
    }

    return true;
}

void GameState::updateGameTime()
{
    if (m_numUnits[Players::Player_One] == 0 && m_numUnits[Players::Player_Two] == 0)
    {
        return;
    }

	const size_t who(whoCanMove());

	if (who == Players::Player_One)
	{
		m_currentTime = getUnit(Players::Player_One, 0).firstTimeFree();
	}
	else if (who == Players::Player_Two)
	{
		m_currentTime = getUnit(Players::Player_Two, 0).firstTimeFree();
	}
	else // Player_Both: both players have units with the same time
	{
		m_currentTime = getUnit(Players::Player_Two, 0).firstTimeFree();
	}
}

const StateEvalScore GameState::eval(const size_t player, const size_t evalMethod, const size_t p1Script, const size_t p2Script) const
{
	StateEvalScore score;
	const size_t enemyPlayer(getEnemy(player));

	// if both players are dead, return 0
	if (playerDead(enemyPlayer) && playerDead(player))
	{
		return StateEvalScore(0, 0);
	}

	StateEvalScore simEval;

	if (evalMethod == SparCraft::EvaluationMethods::LTD)
	{
		score = StateEvalScore(evalLTD(player), 0);
	}
	else if (evalMethod == SparCraft::EvaluationMethods::LTD2)
	{
		score = StateEvalScore(evalLTD2(player), 0);
	}
	else if (evalMethod == SparCraft::EvaluationMethods::Playout)
	{
		score = evalSim(player, p1Script, p2Script);
	}

	ScoreType winBonus(0);

	if (playerDead(enemyPlayer) && !playerDead(player))
	{
		winBonus = 100000;
	}
	else if (playerDead(player) && !playerDead(enemyPlayer))
	{
		winBonus = -100000;
	}

	return StateEvalScore(score.val() + winBonus, score.numMoves());
}

// evaluate the state for m_playerToMove
const ScoreType GameState::evalLTD(const size_t player) const
{
	const size_t enemyPlayer(getEnemy(player));
	
	return LTD(player) - LTD(enemyPlayer);
}

// evaluate the state for m_playerToMove
const ScoreType GameState::evalLTD2(const size_t player) const
{
	const size_t enemyPlayer(getEnemy(player));

	return LTD2(player) - LTD2(enemyPlayer);
}

const StateEvalScore GameState::evalSim(const size_t player, const size_t p1Script, const size_t p2Script) const
{
	PlayerPtr p1(AllPlayers::getPlayerPtr(Players::Player_One, p1Script));
	PlayerPtr p2(AllPlayers::getPlayerPtr(Players::Player_Two, p2Script));

	Game game(*this, p1, p2, 200);

	game.play();

	ScoreType evalReturn = game.getState().evalLTD2(player);

	return StateEvalScore(evalReturn, game.getState().getNumMovements(player));
}

void GameState::calculateStartingHealth()
{
	for (size_t p(0); p<Constants::Num_Players; ++p)
	{
		float totalHP(0);
		float totalSQRT(0);

		for (size_t u(0); u<m_numUnits[p]; ++u)
		{
			totalHP += getUnit(p, u).maxHP() * getUnit(p, u).dpf();
			totalSQRT += sqrtf(getUnit(p,u).maxHP()) * getUnit(p, u).dpf();;
		}

		m_totalLTD[p] = totalHP;
		m_totalSumSQRT[p] = totalSQRT;
	}
}

const ScoreType	GameState::LTD2(const size_t player) const
{
	if (numUnits(player) == 0 || m_totalSumSQRT[player] == 0)
	{
		return 0;
	}

	float sum(0);

	for (size_t u(0); u<numUnits(player); ++u)
	{
		const Unit & unit(getUnit(player, u));

		sum += sqrtf(unit.currentHP()) * unit.dpf();
	}

	ScoreType ret = (ScoreType)(1000 * sum / m_totalSumSQRT[player]);

	return ret;
}

const ScoreType GameState::LTD(const size_t player) const
{
	if (numUnits(player) == 0 || m_totalLTD[player] == 0)
	{
		return 0;
	}

	float sum(0);

	for (size_t u(0); u<numUnits(player); ++u)
	{
		const Unit & unit(getUnit(player, u));

		sum += unit.currentHP() * unit.dpf();
	}

	return (ScoreType)(1000 * sum / m_totalLTD[player]);
}

void GameState::setMap(Map * map)
{
	m_map = map;

    // check to see if all units are on walkable tiles
    for (size_t p(0); p<Constants::Num_Players; ++p)
    {
        for (size_t u(0); u<numUnits(p); ++u)
        {
            const Position & pos(getUnit(p, u).pos());

            if (!isWalkable(pos))
            {
                std::stringstream ss;
                ss << "Unit initial position on non-walkable map tile: " << getUnit(p, u).name() << " (" << pos.x() << "," << pos.y() << ")";
                System::FatalError(ss.str());
            }
        }
    }
}

const size_t GameState::numUnits(const size_t player) const
{
	return m_numUnits[player];
}

const size_t GameState::prevNumUnits(const size_t player) const
{
	return m_prevNumUnits[player];
}

const Unit & GameState::getUnitDirect(const size_t player, const size_t unit) const
{
    if (player >= Constants::Num_Players)
    {
        System::FatalError("GameState::getUnitDirect() invalid player index: " + std::to_string(player));
    }

    if (unit >= m_units[player].size())
    {
        System::FatalError(
            "GameState::getUnitDirect() unit index out of range. player=" + std::to_string(player) +
            " unit=" + std::to_string(unit) +
            " unitStorageSize=" + std::to_string(m_units[player].size()));
    }

	return m_units[player][unit];
}

const bool GameState::bothCanMove() const
{
	return whoCanMove() == Players::Player_Both;
}

void GameState::setTime(const TimeType time)
{
	m_currentTime = time;
}

const int GameState::getNumMovements(const size_t player) const
{
	return m_numMovements[player];
}

const TimeType GameState::getTime() const
{
	return m_currentTime;
}

const float GameState::getTotalLTD(const size_t player) const
{
	return m_totalLTD[player];
}

const float GameState::getTotalLTD2(const size_t player)	const
{
	return m_totalSumSQRT[player];
}

void GameState::setTotalLTD(const float p1, const float p2)
{
	m_totalLTD[Players::Player_One] = p1;
	m_totalLTD[Players::Player_Two] = p2;
}

// detect if there is a deadlock, such that no team can possibly win
const bool GameState::isTerminal() const
{
    // if someone is dead, then nobody can move
    if (playerDead(Players::Player_One) || playerDead(Players::Player_Two))
    {
        return true;
    }

    if (m_turnsWithNoHPChange > Constants::Stalemate_Turn_Limit)
    {
        return true;
    }

	for (size_t p(0); p<Constants::Num_Players; ++p)
	{
		for (size_t u(0); u<numUnits(p); ++u)
		{
			// if any unit on any team is a mobile attacker
			if (getUnit(p, u).isMobile() && !getUnit(p, u).canHeal())
			{
				// there is no deadlock, so return false
				return false;
			}
		}
	}

	// at this point we know everyone must be immobile, so check for attack deadlock
	for (size_t u1(0); u1<numUnits(Players::Player_One); ++u1)
	{
		const Unit & unit1(getUnit(Players::Player_One, u1));

		for (size_t u2(0); u2<numUnits(Players::Player_Two); ++u2)
		{
			const Unit & unit2(getUnit(Players::Player_Two, u2));

			// if anyone can attack anyone else
			if (unit1.canAttackTarget(unit2, m_currentTime) || unit2.canAttackTarget(unit1, m_currentTime))
			{
				// then there is no deadlock
				return false;
			}
		}
	}
	
	// if everyone is immobile and nobody can attack, then there is a deadlock
	return true;
}

void GameState::setTotalLTD2(const float p1, const float p2)
{
	m_totalSumSQRT[Players::Player_One] = p1;
	m_totalSumSQRT[Players::Player_Two] = p2;
}

Map * GameState::getMap() const
{
	return m_map;
}

const size_t GameState::numNeutralUnits() const
{
	return m_neutralUnits.size();
}

const Unit & GameState::getNeutralUnit(const size_t u) const
{
	return m_neutralUnits[u];
}

void GameState::addNeutralUnit(const Unit & unit)
{
	m_neutralUnits.add(unit);
}

// print the state in a neat way
void GameState::print(int indent) const
{
	TABS(indent);
	std::cout << calculateHash(0) << "\n";
	fprintf(stderr, "State - Time: %d\n", m_currentTime);

	for (size_t p(0); p<Constants::Num_Players; ++p)
	{
		for (size_t u(0); u<m_numUnits[p]; ++u)
		{
			const Unit & unit(getUnit(p, u));

			TABS(indent);
			fprintf(stderr, "  P%d %5d %5d    (%3d, %3d)     %s\n", static_cast<int>(unit.player()), unit.currentHP(), unit.firstTimeFree(), unit.x(), unit.y(), unit.name().c_str());
		}
	}
	fprintf(stderr, "\n\n");
}

std::string GameState::toString() const
{

	std::stringstream ss;

	ss << calculateHash(0) << "\n";
	ss << "Time: " << m_currentTime << std::endl;

	for (size_t p(0); p<Constants::Num_Players; ++p)
	{
		for (size_t u(0); u<m_numUnits[p]; ++u)
		{
			const Unit & unit(getUnit(p, u));

			ss << "  P" << (int)unit.player() << " " << unit.currentHP() << " (" << unit.x() << ", " << unit.y() << ") " << unit.name() << std::endl;
		}
	}
	ss << std::endl;

	return ss.str();
}

std::string GameState::toStringCompact() const
{
	std::stringstream ss;

	for (size_t p(0); p<Constants::Num_Players; ++p)
	{
        std::map<BWAPI::UnitType, size_t> typeCount;

		for (size_t u(0); u<m_numUnits[p]; ++u)
		{
			const Unit & unit(getUnit(p, u));

            if (typeCount.find(unit.type()) != std::end(typeCount))
            {
                typeCount[unit.type()]++;
            }
            else
            {
                typeCount[unit.type()] = 1;
            }
		}

        for (auto & kv : typeCount)
        {
            const BWAPI::UnitType & type = kv.first;
            const size_t count = kv.second;

            ss << "P" << (int)p << " " << count << " " << type.getName() << "\n";
        }
	}

	return ss.str();
}

