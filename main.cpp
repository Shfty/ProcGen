#include <iostream>
#include <cstdlib>
#include <cstring>
#include <bitset>
#include <glm/glm.hpp>

#ifdef _WIN32
#include <windows.h>
#endif

#include "tile.h"
#include "PseudoRandom.h"
#include "WorleyNoise.h"

using namespace std;
using namespace glm;

const ivec2 BASE_GRID_SIZE = ivec2( 15, 15 );
const ivec2 MIN_GRID_SIZE = ivec2( 10, 10 );
const float TILE_INTENSITY_THRESHOLD = 0.25f;
const float TILE_RANDOM_THRESHOLD = 0.9f;

int m_gridDimX = 20;
int m_gridDimY = 20;
#define GRID_SIZE m_gridDimX * m_gridDimY
vector<Tile*> m_tiles;
WorleyNoise* noise;

// Outputs grid to stdout
void drawGrid( const std::vector<Tile*>& tiles )
{
    cout << "Drawing grid of size " << tiles.size() << endl << endl;

#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO sBInfo;
    GetConsoleScreenBufferInfo( hConsole, &sBInfo );

    COORD offset = sBInfo.dwCursorPosition;

    for( uint16_t i = 0; i < tiles.size(); ++i )
    {
        COORD coord;
        coord.X = ( tiles[ i ]->GetPosition().x + offset.X ) * 2;
        coord.Y = tiles[ i ]->GetPosition().y + offset.Y;
        SetConsoleCursorPosition( hConsole, coord );
        cout << tiles[ i ]->GetCharacter();
    }

    COORD outPos;
    outPos.X = 0;
    outPos.Y = offset.Y + m_gridDimY;
    SetConsoleCursorPosition( hConsole, outPos );
#else
    char cells[m_gridDimY][m_gridDimX * 2];
    fill(&cells[0][0], &cells[m_gridDimX][0], ' ');

    for( uint16_t i = 0; i < tiles.size(); i++ )
    {
        auto pos = tiles[ i ]->GetPosition();
        int x = pos.x;
        int y = pos.y;
        cells[y][x] = tiles[ i ]->GetCharacter();
    }

    for (int y = 0; y < m_gridDimX; y++) {
        for (int x = 0; x < m_gridDimX; x++) {
            cout << cells[y][x] << " ";
        }
        cout << endl;
    }
#endif

    cout << endl;
    cout << "Done." << endl;
}

// Returns true if str is composed of 0-9, false otherwise
bool stringIsNumber( const char* str )
{
    for( uint16_t i = 0; i < strlen( str ); ++i )
    {
        switch( str[ i ] )
        {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            break;
        default:
            return false;
        }
    }

    return true;
}

// Bitset Parameter Helper Functions
int TWO_BIT( int a, int b ) { return a + 2 * b; }
int THREE_BIT( int a, int b, int c ) { return a + 2 * b + 4 * c; }
int MAP_SIZE_FACTOR( const bitset< 16 >& params ) { return TWO_BIT( params[ 0 ], params[ 1 ] ) + 1; }
int NOISE_DISTANCE_METRIC( const bitset< 16 >& params ) { return std::min( THREE_BIT( params[ 2 ], params[ 3 ], params[ 4 ] ), 5 ); }
int NOISE_MULTISAMPLE_FACTOR( const bitset< 16 >& params ) { return TWO_BIT( params[ 5 ], params[ 6 ] ); }
int MIN_TILE_DISTANCE( const bitset< 16 >& params ) { return TWO_BIT( params[ 7 ], params[ 8 ] ); }
int TERRAIN_DEFORM_FACTOR( const bitset< 16 >& params ) { return TWO_BIT( params[ 9 ], params[ 10 ] ); }
bool START_CELL_CENTRAL( const bitset< 16 >& params ) { return params[ 11 ]; }

// Generate cell tiles from a noise map
const int MULTISAMPLE_FACTOR = 8;
void genCells( int mapSizeFactor, int noiseDistanceMetric, int noiseMultisampleFactor )
{
    // Determine map size
    m_gridDimX = round( clamp( PseudoRandom::PRScalarAbs(), 0.5f, 1.0f ) * BASE_GRID_SIZE.x * mapSizeFactor );
    m_gridDimX = std::max( m_gridDimX, MIN_GRID_SIZE.x );
    m_gridDimY = round( clamp( PseudoRandom::PRScalarAbs(), 0.5f, 1.0f ) * BASE_GRID_SIZE.y * mapSizeFactor );
    m_gridDimY = std::max( m_gridDimY, MIN_GRID_SIZE.y );
    cout << "Map Dimensions: " << m_gridDimX << ", " << m_gridDimY << endl;
    cout << endl;
    cout << "Generating tiles..." << endl << endl;

    // Setup noise
    int msFactor;
    if( noiseMultisampleFactor == 0 ) msFactor = 1;
    if( noiseMultisampleFactor > 0 ) msFactor = pow( 2.0f, float( noiseMultisampleFactor ) );

    noise = new WorleyNoise( m_gridDimX * msFactor, m_gridDimY * msFactor );
    noise->SetDistanceMetric( noiseDistanceMetric );

    // Iterate over grid cells
    for( int x = 0; x < m_gridDimX; ++x )
    {
        for( int y = 0; y < m_gridDimY; ++y )
        {
            // Multisampling
            float intensity = 0.0f;
            int idx;
            for( idx = 0; idx < msFactor * msFactor; ++idx )
            {
                int ox = idx % msFactor;
                int oy = idx / msFactor;
                int xpos = msFactor * x + ox;
                int ypos = msFactor * y + oy;
                intensity += noise->Noise2D( vec2( xpos, ypos ) );
            }
            intensity /= idx + 1;

            // Add tiles if the intensity and random thresholds are exceeded
            if( intensity > TILE_INTENSITY_THRESHOLD )
            {
                if( PseudoRandom::PRScalarAbs() > TILE_RANDOM_THRESHOLD )
                {
                    float normInt = std::max( intensity - TILE_INTENSITY_THRESHOLD, 0.0f );
                    // Determine type by noise intensity
                    char type;
                    if( normInt < 0.25f ) type = '3';
                    if( normInt >= 0.25f && normInt < 0.5f ) type = '4';
                    if( normInt >= 0.5f && normInt < 0.75f ) type = '5';
                    if( normInt >= 0.75f ) type = '6';

                    Tile* tile = new Tile( type );
                    tile->SetPosition( ivec2( x, y ) );
                    m_tiles.push_back( tile );
                }
            }
        }
    }
}

// Prune cells
void pruneCells( int minTileDistance )
{
    // Iterate over the tiles
        // Check neighbouring n cells, where n = minTileDistance
            // If the target tile's importance matches the current tile, delete it
}

int main()
{
    cout << "Procedural Tile World Generator" << endl;

    cout << "Enter Random Seed: ";
    char input[ 16 ];
    cin >> input;

    if( !stringIsNumber( input ) )
    {
        cout << "Invalid Seed" << endl;
        return -1;
    }

    // Extract integer from seed string, use it to seed the random generator
    uint16_t seed = atoi( input );
    PseudoRandom::Seed( seed );
    cout << endl;
    cout << "Initial Random Seed: " << seed << endl;

    // Initialise bitset from seed
    bitset< 16 > params( seed );
    cout << "Binary Parameters: ";
    for( int i = 0; i < 16; ++i ) cout << params[ i ];
    cout << endl << endl;

    // Interpret bitset and print readable parameters
    cout << "Terrain Deformation Factor: " << TERRAIN_DEFORM_FACTOR( params ) << endl;
    cout << "Start Position Central? " << START_CELL_CENTRAL( params ) << endl;
    cout << endl;

    // Procedurally generate tile grid
    // First pass - Generate importance tiles from noise map
    int mapSizeFactor = MAP_SIZE_FACTOR( params );
    int noiseDistanceMetric = NOISE_DISTANCE_METRIC( params );
    int noiseMultisampleFactor = NOISE_MULTISAMPLE_FACTOR( params );
    cout << "Map Size Factor: " << mapSizeFactor << endl;
    cout << "Noise Distance Metric: " << noiseDistanceMetric << endl;
    cout << "Noise Multisampling Factor: " << noiseMultisampleFactor << endl;
    genCells( mapSizeFactor, noiseDistanceMetric, noiseMultisampleFactor );

    // Second pass - Pruning
    int minTileDistance = MIN_TILE_DISTANCE( params );
    cout << "Min Tile Distance: " << minTileDistance << endl;
    pruneCells( minTileDistance );

    // Third pass - Fulfil essential criteria
    /* Criteria:
       At least one tile of each importance level should be present
       ( Start/End tiles each have a unique importance )
     */

    // Fourth pass - Assign events based on tile importance
    /* Notes:
       Events are stored in separate lists based on importance and base world
       Tiles are assigned by choosing an event at random from the appropriate list
     */
    int terrainDeformFactor = TERRAIN_DEFORM_FACTOR( params );

    // Draw tiles
    cout << endl;
    drawGrid( m_tiles );

    // Clean up and return
    for( uint16_t i = 0; i < m_tiles.size(); ++i )
    {
        delete m_tiles[ i ];
    }
    return 0;
}
