/*
 * A Guided Tutorial of recreating Tetris in C++
 * -----
 * Credit to javidx9 for producing the base code and tutorial. Check out his great work at https://www.onelonecoder.com/, he's a swell guy.
 */

#include <iostream>
#include <thread>
#include <vector>
using namespace std;

#include <stdio.h>
#include <Windows.h>

wstring tetrominoArray[7];

int screenWidth = 80;
int screenHeight = 30;

int playingFieldWidth = 12;
int playingFieldHeight = 18;

unsigned char *ptrField = nullptr;

int Rotate(int px, int py, int r)
{
    int rotation = 0;
    switch (r % 4)
    {
    // 0 degrees
    case 0:
        rotation = (py * 4) + px;
        break;

    // 90 degrees
    case 1:
        rotation = 12 + py - (px * 4);
        break;

    // 180 degrees
    case 2:
        rotation = 15 - (py * 4) - px;
        break;

    // 270 degrees
    case 3:
        rotation = 3 - py + (px * 4);
        break;
    }
    return rotation;
}

bool DoesPieceFit(int tetromino, int rotation, int posX, int posY)
{
    for (int x = 0; x < 4; x++)
    {
        for (int y = 0; y < 4; y++)
        {
            // Get index of piece
            int pieceIndex = Rotate(x, y, rotation);

            // Get index of playingField
            int fieldIndex = (posY + y) * playingFieldWidth + (posX + x);

            // Check for memory out of bounds
            if (posX + x >= 0 && posX + x < playingFieldWidth)
            {
                if (posY + y >= 0 && posY + y < playingFieldHeight)
                {
                    if (tetrominoArray[tetromino][pieceIndex] != L'.' && ptrField[fieldIndex] != 0)
                    {
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

int main()
{
    // Screen Buffer
    wchar_t *screen = new wchar_t[screenWidth * screenHeight];
    for (int i = 0; i < screenWidth * screenHeight; i++)
    {
        screen[i] = L' ';
    }

    HANDLE console = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(console);
    DWORD bytesWritten = 0;

    //Create game assets
    tetrominoArray[0].append(L"..X...X...X...X.");
    tetrominoArray[1].append(L"..X..XX...X.....");
    tetrominoArray[2].append(L".....XX..XX.....");
    tetrominoArray[3].append(L"..X..XX..X......");
    tetrominoArray[4].append(L".X...XX...X.....");
    tetrominoArray[5].append(L".X...X...XX.....");
    tetrominoArray[6].append(L"..X...X..XX.....");

    // Create the playing field
    ptrField = new unsigned char[playingFieldWidth * playingFieldHeight];

    // Board boundary
    for (int x = 0; x < playingFieldWidth; x++)
    {
        for (int y = 0; y < playingFieldHeight; y++)
        {
            ptrField[y * playingFieldWidth + x] = (x == 0 || x == playingFieldWidth - 1 || y == playingFieldHeight - 1) ? 9 : 0;
        }
    }

    //Game Logic
    bool Keys[4];
    //  tetromino variables
    int currentPiece = 0;
    int currentRotation = 0;
    int currentX = playingFieldWidth / 2;
    int currentY = 0;

    // Game Speed
    int gameSpeed = 20;
    int gameSpeedCounter = 0;

    // latch for rotate 'Z' key
    bool forcePieceDown = false;
    bool rotateHold = true;
    int pieceCount = 0;
    int gameScore = 0;
    // vector to store horizontal lines
    vector<int> linesVector;
    bool isGameOver = false;

    while (!isGameOver)
    {
        // Game Time

        // One game tick
        this_thread::sleep_for(50ms);
        gameSpeedCounter++;
        forcePieceDown = (gameSpeedCounter == gameSpeed);

        // Input
        for (int i = 0; i < 4; i++)
        {
            Keys[i] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[i]))) != 0;
        }

        //  Game Logic

        // Player Movement
        currentX += (Keys[0]) && (DoesPieceFit(currentPiece, currentRotation, currentX + 1, currentY)) ? 1 : 0;
        currentX -= (Keys[1]) && (DoesPieceFit(currentPiece, currentRotation, currentX - 1, currentY)) ? 1 : 0;
        currentY += (Keys[2]) && (DoesPieceFit(currentPiece, currentRotation, currentX, currentY + 1)) ? 1 : 0;

        // Rotate, but latch to stop odd spinning behavior
        if (Keys[3])
        {
            currentRotation += (rotateHold && DoesPieceFit(currentPiece, currentRotation + 1, currentX, currentY)) ? 1 : 0;
            rotateHold = false;
        }
        else
        {
            rotateHold = true;
        }

        if (forcePieceDown)
        {
            // increase game speed
            gameSpeedCounter = 0;
            pieceCount++;
            if (pieceCount % 10 == 0)
            {
                if (gameSpeed >= 10)
                {
                    gameSpeed--;
                }
            }
            //If it can fit, place it in that position
            if (DoesPieceFit(currentPiece, currentRotation, currentX, currentY + 1))
            {
                currentY++;
            }
            else
            {
                // If it can't fit, lock into the field
                for (int x = 0; x < 4; x++)
                {
                    for (int y = 0; y < 4; y++)
                    {
                        if (tetrominoArray[currentPiece][Rotate(x, y, currentRotation)] != L'.')
                        {
                            ptrField[(currentY + y) * playingFieldWidth + (currentX + x)] = currentPiece + 1;
                        }
                    }
                }

                // Check have for a full horizontal line
                for (int y = 0; y < 4; y++)
                {
                    if (currentY + y < playingFieldHeight - 1)
                    {
                        bool isFullLine = true;
                        for (int x = 1; x < playingFieldWidth - 1; x++)
                        {
                            isFullLine &= (ptrField[(currentY + y) * playingFieldWidth + x]) != 0;
                        }
                        if (isFullLine)
                        {
                            // Remove line set all elements to '='
                            for (int x = 1; x < playingFieldWidth - 1; x++)
                            {
                                ptrField[(currentY + y) * playingFieldWidth + x] = 8;
                            }
                            linesVector.push_back(currentY + y);
                        }
                    }
                }

                gameScore += 25;
                if (!linesVector.empty())
                {
                    gameScore += (1 << linesVector.size()) * 100;
                }

                // Choose next piece
                currentX = playingFieldWidth / 2;
                currentY = 0;
                currentRotation = 0;
                currentPiece = rand() % 7;

                // The piece doesn't fit
                isGameOver = !DoesPieceFit(currentPiece, currentRotation, currentX, currentY);
            }
        }

        // Display Output

        // Draw Game Field
        for (int x = 0; x < playingFieldWidth; x++)
        {
            for (int y = 0; y < playingFieldHeight; y++)
            {
                screen[(y + 2) * screenWidth + (x + 2)] = L" ABCDEFG=#"[ptrField[y * playingFieldWidth + x]];
            }
        }

        // Draw Current Piece
        for (int x = 0; x < 4; x++)
        {
            for (int y = 0; y < 4; y++)
            {
                if (tetrominoArray[currentPiece][Rotate(x, y, currentRotation)] != L'.')
                {
                    screen[(currentY + y + 2) * screenWidth + (currentX + x + 2)] = currentPiece + 65;
                }
            }
        }

        // Draw Score
        swprintf_s(&screen[2 * screenWidth + playingFieldWidth + 6], 16, L"SCORE: %8d", gameScore);
        // If I get a line clear it out.
        if (!linesVector.empty())
        {
            // Display Frame
            WriteConsoleOutputCharacter(console, screen, playingFieldWidth * playingFieldHeight, {0, 0}, &bytesWritten);

            // Delay for a bit to simulate the arcade feeling
            this_thread::sleep_for(400ms);

            for (auto &v : linesVector)
            {
                for (int x = 1; x < playingFieldWidth - 1; x++)
                {
                    for (int y = v; y > 0; y--)
                    {
                        ptrField[y * playingFieldWidth + x] = ptrField[(y - 1) * playingFieldWidth + x];
                    }
                    ptrField[x] = 0;
                }
            }
            linesVector.clear();
        }

        //Display Frame
        WriteConsoleOutputCharacter(console, screen, screenWidth * screenHeight, {0, 0}, &bytesWritten);
    }

    // Game Over Message
    CloseHandle(console);
    cout << "GAME OVER!!! Score: " << gameScore << endl;
    system("pause");

    return 0;
}