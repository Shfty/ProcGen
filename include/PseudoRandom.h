#ifndef PSEUDORANDOM_H
#define PSEUDORANDOM_H

#include <iostream>

using namespace std;

static uint16_t m_seed = 34083;
inline void advanceSeed()
{
    m_seed = ( m_seed * std::log( m_seed ) ) / atan2( m_seed, std::cos( m_seed ) );
}

namespace PseudoRandom {
    inline void Seed( uint16_t seed ) { m_seed = seed; }

    inline uint16_t PRInteger()
    {
        advanceSeed();
        return m_seed;
    }

    inline float PRScalar()
    {
        static int interval = 0;

        float out;
        switch( interval % 4 )
        {
            case 0:
                out = std::sin( m_seed );
                break;
            case 1:
                out = std::cos( m_seed * m_seed );
                break;
            case 2:
                out = std::sin( m_seed * m_seed * m_seed );
                break;
            case 3:
                out = std::cos( m_seed * m_seed * m_seed * m_seed );
                break;
            default:
                break;
        }

        advanceSeed();
        ++interval;

        return out;
    }

    inline float PRScalarAbs()
    {
        return abs( PRScalar() );
    }
}

#endif // PSEUDORANDOM_H
