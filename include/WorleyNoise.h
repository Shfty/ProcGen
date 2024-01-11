#ifndef NOISEGEN_H
#define NOISEGEN_H

#include <vector>
#include <glm/glm.hpp>

#include "PseudoRandom.h"

using namespace std;
using namespace glm;

class WorleyNoise
{
public:
    WorleyNoise( const float xSize, const float ySize );

    float GetMinkowskiNumber() const;
    void SetMinkowskiNumber( const float n );

    ivec2 GetGridDivisions() const { return m_gridDivisions; }
    void SetGridDivisions( const ivec2& d );

    uint16_t GetMaxPointsPerCell() const { return m_maxPointsPerCell; }
    void SetMaxPointsPerCell( const uint16_t p );

    uint16_t GetFValue() const { return m_fValue; }
    void SetFValue( const uint16_t v ) { m_fValue = v; }

    uint16_t GetDistanceMetric() const { return m_distanceMetric; }
    void SetDistanceMetric( const uint16_t m ) { m_distanceMetric = m; }

    float Noise2D( const vec2& pt );

private:
    void setupCells();

    ivec2 m_bounds = ivec2( 320, 320 );
    ivec2 m_gridDivisions = ivec2( 3.5, 3.5 );
    uint16_t m_maxPointsPerCell = 5;
    uint16_t m_fValue = 1;
    uint16_t m_distanceMetric = 0;
    ivec2 m_cellSize;

    vector< vector< vec2 > > m_pointGrid;
};

#endif // NOISEGEN_H
