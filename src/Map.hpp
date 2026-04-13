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
	size_t					_walkTileWidth;
	size_t					_walkTileHeight;
	size_t					_buildTileWidth;
	size_t					_buildTileHeight;
	bvv						_mapData;	            // true if walk tile [x][y] is walkable

	bvv						_unitData;	            // true if unit on build tile [x][y]
	bvv						_buildingData;          // true if building on build tile [x][y]

	const Position getWalkPosition(const Position & pixelPosition) const
	{
		return Position(pixelPosition.x() / 8, pixelPosition.y() / 8);
	}

    void resetVectors()
    {
        _mapData =          bvv(_walkTileWidth,  std::vector<bool>(_walkTileHeight,  true));
		_unitData =         bvv(_buildTileWidth, std::vector<bool>(_buildTileHeight, false));
		_buildingData =     bvv(_buildTileWidth, std::vector<bool>(_buildTileHeight, false));
    }

public:

	Map() 
        : _walkTileWidth(0)
		, _walkTileHeight(0)
		, _buildTileWidth(0)
		, _buildTileHeight(0)
    {
    }

    // constructor which sets a completely walkable map
    Map(const size_t & bottomRightBuildTileX, const size_t & bottomRightBuildTileY)
        : _walkTileWidth(bottomRightBuildTileX * 4)
		, _walkTileHeight(bottomRightBuildTileY * 4)
		, _buildTileWidth(bottomRightBuildTileX)
		, _buildTileHeight(bottomRightBuildTileY)
    {
        resetVectors();
    }

	Map(BWAPI::Game & game) 
        : _walkTileWidth(static_cast<size_t>(game.mapWidth()) * 4)
		, _walkTileHeight(static_cast<size_t>(game.mapHeight()) * 4)
		, _buildTileWidth(static_cast<size_t>(game.mapWidth()))
		, _buildTileHeight(static_cast<size_t>(game.mapHeight()))
	{
		resetVectors();

		for (size_t x(0); x<_walkTileWidth; ++x)
		{
			for (size_t y(0); y<_walkTileHeight; ++y)
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
		return _walkTileWidth;
	}

	const size_t & getWalkTileHeight() const
	{
		return _walkTileHeight;
	}

	const size_t & getBuildTileWidth() const
	{
		return _buildTileWidth;
	}

	const size_t & getBuildTileHeight() const
	{
		return _buildTileHeight;
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
		return _mapData[walkTileX][walkTileY];
	}

	const bool getUnitData(const size_t & buildTileX, const size_t & buildTileY) const
	{
		return _unitData[buildTileX][buildTileY];
	}

	void setMapData(const size_t & walkTileX, const size_t & walkTileY, const bool val)
	{
		_mapData[walkTileX][walkTileY] = val;
	}

	void setUnitData(BWAPI::Game & game)
	{
		_unitData = bvv(getBuildTileWidth(), std::vector<bool>(getBuildTileHeight(), true));

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
		return _unitData[static_cast<size_t>(x)][static_cast<size_t>(y)] && _buildingData[static_cast<size_t>(x)][static_cast<size_t>(y)];
	}

	void setBuildingData(BWAPI::Game & game)
	{
		_buildingData = bvv(getBuildTileWidth(), std::vector<bool>(getBuildTileHeight(), true));

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
					_buildingData[x][y] = false;
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
					_unitData[x][y] = false;
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
		_walkTileWidth = atoi(line.c_str());

		getline(fin, line);
		_walkTileHeight = atoi(line.c_str());

        _buildTileWidth = _walkTileWidth/4;
        _buildTileHeight = _walkTileHeight/4;

		resetVectors();

		for (size_t y(0); y<getWalkTileHeight(); ++y)
		{
			getline(fin, line);

			for (size_t x(0); x<getWalkTileWidth(); ++x)
			{
				_mapData[x][y] = line[x] == '1' ? true : false;
			}
		}

		fin.close();
	}
};
}
