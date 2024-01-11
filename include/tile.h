#ifndef TILE_H
#define TILE_H

#include <vector>
#include <glm/glm.hpp>

using namespace std;
using namespace glm;

class Tile
{
public:
    Tile() { m_character = ' '; }
    Tile( const char c ) { m_character = c; }

    char GetCharacter() const { return m_character; }
    void SetCharacter( const char c ) { m_character = c; }

    ivec2 GetPosition() const { return m_position; }
    void SetPosition( const ivec2 p ) { m_position = p; }

private:
    ivec2 m_position;
    char m_character;
};

#endif // TILE_H
