/* ASCII-style DOOM-based game
*/


// Importing libraries
#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <stdio.h>
#include <windows.h>
using namespace std;





// X and Y console screen size
int nScreenWidth = 120;     // Columns
int nScreenHeight = 40;     // Lines

// Constantes para o mapa
int nMapHeight = 19;
int nMapWidth = 19;

// Player's starting x and y rotation position
float fPlayerX = 14.7f;
float fPlayerY = 5.09f;

float fPlayerA = 0.0f;      // Angle the player looks at

float fFOV = 3.14159 / 4.0; // Player's field of view (narrow)
float fDepth = 19.0f;       // Maximum rendering distance


int main()
{
    // Pedaço de código para criar um buffer de tela;
    wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwBytesWritten = 0;

    wstring map;    // Create world map

    // Bound the maze with '#' wall blocks
    map += L"###################";
    map += L"#...#.............#";
    map += L"#...##............#";
    map += L"#.................#";
    map += L"#.................#";
    map += L"#.................#";
    map += L"#.................#";
    map += L"#.................#";
    map += L"######..........###";
    map += L"#.................#";
    map += L"#.....#...........#";
    map += L"#.................#";
    map += L"###...............#";
    map += L"#..........########";
    map += L"#.................#";
    map += L"#.................#";
    map += L"#.........#.......#";
    map += L"#.........#.......#";
    map += L"###################";

    // Controls the speed of a frame for motion variables
    auto tp1 = chrono::system_clock::now();
    auto tp2 = chrono::system_clock::now();

    // Game loop
    while (1)
    {
        // Gets the current system time;
        // Calculates the duration between the current system time and the previous one;
        // Update the old time point;
        // Get the elapsed time.
        tp2 = chrono::system_clock::now();
        chrono::duration<float> elapsedTime = tp2 - tp1;
        tp1 = tp2;
        float fElapsedTime = elapsedTime.count();


        // Controls
        // Progress consistently
        // CW Rotation
        if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
            fPlayerA -= (0.9f) * fElapsedTime;                          // Anticlockwise
        if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
            fPlayerA += (0.9f) * fElapsedTime;                          // Clockwise

        // Frontal movement to collision
        if (GetAsyncKeyState((unsigned short)'W') & 0x8000)
        {
            fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
            fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;

            // Crash test
            if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#')
            {
                fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
                fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;
            }
        }

        // Consistently pulls back
        if (GetAsyncKeyState((unsigned short)'S') & 0x8000)
        {
            fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
            fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;
            if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#')
            {
                fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
                fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;
            }
        }


        for (int x = 0; x < nScreenWidth; x++)
        {
            // For each column, the angle of the ray projected into world space is calculated.
            float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) * fFOV;

            // Tracking function;
            // Calculates the distance from the player to the wall
            float fDistanceToWall = 0.0;
            bool bHitWall = false;                  // Impact flag
            bool bBoundary = false;

            float fEyeX = sinf(fRayAngle);          // Unit vector for radius in player space
            float fEyeY = cosf(fRayAngle);

            // Without hitting the impact wall, continue to increase the distance to the wall in small steps.
            while (!bHitWall && fDistanceToWall < fDepth)
            {
                fDistanceToWall += 0.1f;

                int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
                int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

                // Test if radius is out of bounds
                if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight)
                {
                    bHitWall = true;                                // Sets the distance to the maximum depth
                    fDistanceToWall = fDepth;
                }
                else
                {
                    // The ray is coming in. Then see if the radius cell is a wall block
                    if (map[nTestY * nMapWidth + nTestX] == '#')
                    {
                        bHitWall = true;

                        vector<pair<float, float>> p; // distance, dot

                        for (int tx = 0; tx < 2; tx++)
                            for (int ty = 0; ty < 2; ty++)
                            {
                                float vy = (float)nTestY + ty - fPlayerY;
                                float vx = (float)nTestX + tx - fPlayerX;
                                float d = sqrt(vx * vx + vy * vy);
                                float dot = (fEyeX * vx / d) + (fEyeY * vy / d);
                                p.push_back(make_pair(d, dot));
                            }

                        // Sort pairs from closest to farthest
                        sort(p.begin(), p.end(), [](const pair<float, float>& left, const pair<float, float>& right) {return left.first < right.first; });

                        float fBound = 0.01;
                        if (acos(p.at(0).second) < fBound) bBoundary = true;
                        if (acos(p.at(1).second) < fBound) bBoundary = true;
                        //if (acos(p.at(2).second) < fBound) bBoundary = true;
                    }
                }
            }


            // Caculate distance to ceiling and floor
            int nCelling = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)fDistanceToWall);
            int nFloor = nScreenHeight - nCelling;

            short nShade = ' ';

            if (fDistanceToWall <= fDepth / 4.0f)           nShade = 0x2588;      // Very close
            else if (fDistanceToWall < fDepth / 3.0f)       nShade = 0x2593;
            else if (fDistanceToWall < fDepth / 2.0f)       nShade = 0x2592;
            else if (fDistanceToWall < fDepth)              nShade = 0x2591;
            else                                            nShade = ' ';        // Too far away

            if (bBoundary)      nShade = 'I';                                   // Black it out


            // Column drawing
            for (int y = 0; y < nScreenHeight; y++)
            {
                if (y <= nCelling)
                    screen[y * nScreenWidth + x] = ' ';
                else if (y > nCelling && y <= nFloor)
                    screen[y * nScreenWidth + x] = nShade;
                else
                {
                    // Shade floor based on distance
                    float b = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));
                    if (b < 0.25)       nShade = '#';
                    else if (b < 0.5)   nShade = 'x';
                    else if (b < 0.75)  nShade = '.';
                    else if (b < 0.9)   nShade = '-';
                    else                        nShade = ' ';
                    screen[y * nScreenWidth + x] = nShade;
                }
            }
        }

        // Display stats
        swprintf_s(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f FPS=%3.2f ", fPlayerX, fPlayerY, fPlayerA, 1.0f / fElapsedTime);

        // Display Map
        for (int nx = 0; nx < nMapWidth; nx++)
            for (int ny = 0; ny < nMapWidth; ny++)
            {
                screen[(ny + 1) * nScreenWidth + nx] = map[ny * nMapWidth + nx];
            }

        // Player locator
        screen[((int)fPlayerY + 1) * nScreenHeight + (int)fPlayerX] = 'P1';

        // Function that specifies on the left side the coordinates
        screen[nScreenWidth * nScreenHeight - 1] = '\0';
        WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
    }

    return 0;
}