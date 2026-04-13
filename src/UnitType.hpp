#pragma once

#include "Location.hpp"

class Unit 
{
	int		m_damage,
			m_maxHP,
			m_currentHP,
			m_range,
			m_moveCooldown,
			m_weaponCooldown,
			m_lastMove,
			m_lastAttack;

public:

	Unit()
		 : m_damage(0)
		 , m_maxHP(0)
		 , m_currentHP(0)
		 , m_range(0)
		 , m_moveCooldown(0)
		 , m_weaponCooldown(0)
		 , m_lastMove(-1)
		 , m_lastAttack(-1)
	{

	}

	Unit(const int & damage, const int & maxHP, const int & currentHP, 
		 const int & range, const int & moveCooldown, const int & weaponCooldown) 
		 : m_damage(damage)
		 , m_maxHP(maxHP)
		 , m_currentHP(currentHP)
		 , m_range(range)
		 , m_moveCooldown(moveCooldown)
		 , m_weaponCooldown(weaponCooldown)
		 , m_lastMove(-1)
		 , m_lastAttack(-1)
	{

	}

	const int damage()			const { return m_damage; }
	const int maxHP()			const { return m_maxHP; }
	const int currentHP()		const { return m_currentHP; }
	const int range()			const { return m_range; }
	const int moveCooldown()	const { return m_moveCooldown; }
	const int weaponCooldown()	const { return m_weaponCooldown; }
	const int lastMove()		const { return m_lastMove; }
	const int lastAttack()		const { return m_lastAttack; }
};

