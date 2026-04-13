#pragma once

#include "Common.h"
#include "Array.hpp"
#include "Unit.h"
#include <algorithm>

namespace SparCraft
{

typedef std::vector< std::vector<bool> > bvv;

class Map
{
	size_t					m_walkTileWidth;
	size_t					m_walkTileHeight;
	size_t					m_buildTileWidth;
	size_t					m_buildTileHeight;
	bvv						m_mapData;	            // true if walk tile [x][y] is walkable

	bvv						m_unitData;	            // true if unit on build tile [x][y]
	bvv						m_buildingData;          // true if building on build tile [x][y]

	const Position getWalkPosition(const Position & pixelPosition) const
	{
		return Position(pixelPosition.x() / 8, pixelPosition.y() / 8);
	}

    void resetVectors()
    {
        m_mapData =          bvv(m_walkTileWidth,  std::vector<bool>(m_walkTileHeight,  true));
		m_unitData =         bvv(m_buildTileWidth, std::vector<bool>(m_buildTileHeight, false));
		m_buildingData =     bvv(m_buildTileWidth, std::vector<bool>(m_buildTileHeight, false));
    }

public:

	Map() 
        : m_walkTileWidth(0)
		, m_walkTileHeight(0)
		, m_buildTileWidth(0)
		, m_buildTileHeight(0)
    {
    }

    // constructor which sets a completely walkable map
    Map(const size_t & bottomRightBuildTileX, const size_t & bottomRightBuildTileY)
        : m_walkTileWidth(bottomRightBuildTileX * 4)
		, m_walkTileHeight(bottomRightBuildTileY * 4)
		, m_buildTileWidth(bottomRightBuildTileX)
		, m_buildTileHeight(bottomRightBuildTileY)
    {
        resetVectors();
    }

	Map(BWAPI::Game & game) 
        : m_walkTileWidth(static_cast<size_t>(game.mapWidth()) * 4)
		, m_walkTileHeight(static_cast<size_t>(game.mapHeight()) * 4)
		, m_buildTileWidth(static_cast<size_t>(game.mapWidth()))
		, m_buildTileHeight(static_cast<size_t>(game.mapHeight()))
	{
		resetVectors();

		for (size_t x(0); x<m_walkTileWidth; ++x)
		{
			for (size_t y(0); y<m_walkTileHeight; ++y)
			{
				setMapData(x, y, game.isWalkable(static_cast<int>(x), static_cast<int>(y)));
			}
		}
	}
	
    const size_t getPixelWidth() const
    {
        return getWalkTileWidth() * 4;
    }

    const size_t getPixelHeight() const
    {
        return getWalkTileHeight() * 4;
    }

	const size_t & getWalkTileWidth() const
	{
		return m_walkTileWidth;
	}

	const size_t & getWalkTileHeight() const
	{
		return m_walkTileHeight;
	}

	const size_t & getBuildTileWidth() const
	{
		return m_buildTileWidth;
	}

	const size_t & getBuildTileHeight() const
	{
		return m_buildTileHeight;
	}

	const bool isWalkable(const SparCraft::Position & pixelPosition) const
	{
		const Position & wp(getWalkPosition(pixelPosition));

		return	isWalkable(wp.x(), wp.y());
	}
    
    const bool isFlyable(const SparCraft::Position & pixelPosition) const
	{
		const Position & wp(getWalkPosition(pixelPosition));

		return isFlyable(wp.x(), wp.y());
	}

	const bool isWalkable(const size_t & walkTileX, const size_t & walkTileY) const
	{
		return	walkTileX >= 0 && walkTileX < (PositionType)getWalkTileWidth() && 
				walkTileY >= 0 && walkTileY < (PositionType)getWalkTileHeight() &&
				getMapData(walkTileX, walkTileY);
	}

    const bool isFlyable(const size_t & walkTileX, const size_t & walkTileY) const
	{
		return	walkTileX >= 0 && walkTileX < (PositionType)getWalkTileWidth() && 
				walkTileY >= 0 && walkTileY < (PositionType)getWalkTileHeight();
	}

	const bool getMapData(const size_t & walkTileX, const size_t & walkTileY) const
	{
		return m_mapData[walkTileX][walkTileY];
	}

	const bool getUnitData(const size_t & buildTileX, const size_t & buildTileY) const
	{
		return m_unitData[buildTileX][buildTileY];
	}

	void setMapData(const size_t & walkTileX, const size_t & walkTileY, const bool val)
	{
		m_mapData[walkTileX][walkTileY] = val;
	}

	void setUnitData(BWAPI::Game & game)
	{
		m_unitData = bvv(getBuildTileWidth(), std::vector<bool>(getBuildTileHeight(), true));

		for (BWAPI::Unit * unit : game.getAllUnits())
		{
			if (unit && !unit->getType().isBuilding())
			{
				addUnit(unit);
			}
		}
	}

	const bool canBuildHere(BWAPI::TilePosition pos)
	{
        const int x = pos.x();
        const int y = pos.y();
        if (x < 0 || y < 0 || x >= static_cast<int>(getBuildTileWidth()) || y >= static_cast<int>(getBuildTileHeight()))
        {
            return false;
        }
		return m_unitData[static_cast<size_t>(x)][static_cast<size_t>(y)] && m_buildingData[static_cast<size_t>(x)][static_cast<size_t>(y)];
	}

	void setBuildingData(BWAPI::Game & game)
	{
		m_buildingData = bvv(getBuildTileWidth(), std::vector<bool>(getBuildTileHeight(), true));

		for (BWAPI::Unit * unit : game.getAllUnits())
		{
			if (unit && unit->getType().isBuilding())
			{
				addUnit(unit);
			}
		}
	}

	void addUnit(const BWAPI::Unit * unit)
	{
        if (!unit)
        {
            return;
        }

        const BWAPI::UnitType unitType = unit->getType();
        const BWAPI::Position unitPosition = unit->getPosition();
        
		if (unitType.isBuilding())
		{
			int tx = unitPosition.x() / TILE_SIZE;
			int ty = unitPosition.y() / TILE_SIZE;
			int sx = unitType.tileWidth(); 
			int sy = unitType.tileHeight();
            int startX = std::max(0, tx);
            int endX = std::min(tx + sx, static_cast<int>(getBuildTileWidth()));
            int startY = std::max(0, ty);
            int endY = std::min(ty + sy, static_cast<int>(getBuildTileHeight()));
			for(int x = startX; x < endX; ++x)
			{
				for(int y = startY; y < endY; ++y)
				{
					m_buildingData[x][y] = false;
				}
			}
		}
		else
		{
			int startX = (unitPosition.x() - unitType.dimensionLeft()) / TILE_SIZE;
			int endX   = (unitPosition.x() + unitType.dimensionRight() + TILE_SIZE - 1) / TILE_SIZE; // Division - round up
			int startY = (unitPosition.y() - unitType.dimensionUp()) / TILE_SIZE;
			int endY   = (unitPosition.y() + unitType.dimensionDown() + TILE_SIZE - 1) / TILE_SIZE;
            startX = std::max(0, startX);
            endX = std::min(endX, static_cast<int>(getBuildTileWidth()));
            startY = std::max(0, startY);
            endY = std::min(endY, static_cast<int>(getBuildTileHeight()));
			for (int x = startX; x < endX; ++x)
			{
				for (int y = startY; y < endY; ++y)
				{
					m_unitData[x][y] = false;
				}
			}
		}
	}

	unsigned int * getRGBATexture()
	{
		unsigned int * data = new unsigned int[getWalkTileWidth() * getWalkTileHeight()];
		for (size_t x(0); x<getWalkTileWidth(); ++x)
		{
			for (size_t y(0); y<getWalkTileHeight(); ++y)
			{
				if (!isWalkable(x, y))
				{
					data[y*getWalkTileWidth() + x] = 0xffffffff;
				}
				else
				{
					data[y*getWalkTileWidth() + x] = 0x00000000;
				}
			}
		}

		return data;
	}

	void write(const std::string & filename)
	{
		std::ofstream fout(filename.c_str());
		fout << getWalkTileWidth() << "\n" << getWalkTileHeight() << "\n";

		for (size_t y(0); y<getWalkTileHeight(); ++y)
		{
			for (size_t x(0); x<getWalkTileWidth(); ++x)
			{
				fout << (isWalkable(x, y) ? 1 : 0);
			}

			fout << "\n";
		}

		fout.close();
	}

	void load(const std::string & filename)
	{
		std::ifstream fin(filename.c_str());
		std::string line;
		
		getline(fin, line);
		m_walkTileWidth = atoi(line.c_str());

		getline(fin, line);
		m_walkTileHeight = atoi(line.c_str());

        m_buildTileWidth = m_walkTileWidth/4;
        m_buildTileHeight = m_walkTileHeight/4;

		resetVectors();

		for (size_t y(0); y<getWalkTileHeight(); ++y)
		{
			getline(fin, line);

			for (size_t x(0); x<getWalkTileWidth(); ++x)
			{
				m_mapData[x][y] = line[x] == '1' ? true : false;
			}
		}

		fin.close();
	}
};
}

