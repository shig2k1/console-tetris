// ConsoleTetris.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
using namespace std;

#include <thread>
#include <Windows.h>
#include <vector>

wstring tetromino[7];
int nFieldWidth = 12;
int nFieldHeight = 18;

int nScreenWidth = 80;                // Console screen size x (columns)
int nScreenHeight = 30;               // Console screen size y (rows)

unsigned char *pField = nullptr;

/*
transforms positons in congiuous array to reposition items in rotated configuration

0 degrees     90 degrees    180 degrees   270 degrees

01|02|03|04   13|09|05|01   16|15|14|13   04|08|12|16
-----------   -----------   -----------   -----------
05|06|07|08   14|10|06|02   12|11|10|09   03|07|11|15
-----------   -----------   -----------   -----------
09|10|11|12   15|11|07|03   08|07|06|05   02|06|10|14
-----------   -----------   -----------   -----------
13|14|15|16   16|12|08|04   04|03|02|01   01|05|09|13
*/
int Rotate(int px, int py, int r) {
  switch (r % 4) 
  {
    case 0: return py * 4 + px;         // 0 degrees
    case 1: return 12 + py - (px * 4);  // 90 degrees
    case 2: return 15 - (py * 4) - px;  // 180 degrees
    case 3: return 3 - py + (px * 4);   // 270 degrees
  }
  return 0;
}

bool DoesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY)
{
  for (int px = 0; px < 4; px++)
    for (int py = 0; py < 4; py++)
    {
      // get index into piece
      int pi = Rotate(px, py, nRotation);
      
      // get index into field
      int fi = (nPosY + py) * nFieldWidth + (nPosX + px);

      // check bounds
      if (nPosX + px >= 0 && nPosX + px < nFieldWidth)
      {
        if (nPosY + py >= 0 && nPosY + py < nFieldHeight)
        {
          if (tetromino[nTetromino][pi] == L'X' && pField[fi] != 0)
            return false; // fail - something is in the way
        }
      }
    }
  return true;
}

int main()
{
  // create assets
  tetromino[0].append(L"..X.");
  tetromino[0].append(L"..X.");
  tetromino[0].append(L"..X.");
  tetromino[0].append(L"..X.");

  tetromino[1].append(L"..X.");
  tetromino[1].append(L".XX.");
  tetromino[1].append(L".X..");
  tetromino[1].append(L"....");

  tetromino[2].append(L".X..");
  tetromino[2].append(L".XX.");
  tetromino[2].append(L"..X.");
  tetromino[2].append(L"....");

  tetromino[3].append(L"....");
  tetromino[3].append(L".XX.");
  tetromino[3].append(L".XX.");
  tetromino[3].append(L"....");

  tetromino[4].append(L".XX.");
  tetromino[4].append(L".X..");
  tetromino[4].append(L".X..");
  tetromino[4].append(L"....");

  tetromino[5].append(L".XX.");
  tetromino[5].append(L"..X.");
  tetromino[5].append(L"..X.");
  tetromino[5].append(L"....");

  tetromino[6].append(L"..X.");
  tetromino[6].append(L".XX.");
  tetromino[6].append(L"..X.");
  tetromino[6].append(L"....");

  // define playfield
  pField = new unsigned char[nFieldWidth * nFieldHeight]; // create playfield buffer
  for (int x = 0; x < nFieldWidth; x++) // Board boundary
    for (int y = 0; y < nFieldHeight; y++)
      pField[y * nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0;

  // buffer the output to the screen
  wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
  for (int i = 0; i < nScreenWidth * nScreenHeight; i++) screen[i] = L' ';
  HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
  SetConsoleActiveScreenBuffer(hConsole);
  DWORD dwBytesWritten = 0;

  bool bGameOver = false;

  int nCurrentPiece = 0;
  int nCurrentRotation = 0;
  int nCurrentX = nFieldWidth / 2;
  int nCurrentY = 0;

  int nSpeed = 20; // game speed
  int nSpeedCounter = 0;
  bool bForceDown = false;

  bool bKey[4]; // store key states
  bool bRotationHold = false; // latch rotation state

  vector<int> vLines; // store lines for visual disappear effect

  int nPieceCount = 1;
  int nScore = 0;

  while (!bGameOver)
  {
    // GAME TIMING ====================================================================
    this_thread::sleep_for(50ms);
    nSpeedCounter++;
    bForceDown = (nSpeedCounter == nSpeed);

    // INPUT ==========================================================================
    for (int k = 0; k < 4; k++) // create an array of the states for the pressed keys
    {                                                      // R  L   D  Z
      bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0;
    }

    // LOGIC ==========================================================================
    nCurrentX += (bKey[0] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)) ? 1 : 0;  // left pressed
    nCurrentX -= (bKey[1] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY)) ? 1 : 0;  // right pressed
    nCurrentY += (bKey[2] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) ? 1 : 0;  // down pressed

    if (bKey[3]) {
      nCurrentRotation += (!bRotationHold && DoesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY)) ? 1 : 0; // z pressed
      bRotationHold = true;
    }
    else {
      bRotationHold = false;
    }

    if (bForceDown)
    {
      if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) 
        nCurrentY++; // Piece can fit on next line, so move it down
      else {
        // lock the current piece into the field
        for (int px = 0; px < 4; px++)
          for (int py = 0; py < 4; py++)
            // if the current position inside the shape rotated shape is an X...
            if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X')
              // write data into the playfield at that position
              pField[(nCurrentY + py) * nFieldWidth + (nCurrentX + px)] = nCurrentPiece + 1;

        // check have created any full horizonal lines?
        for (int py = 0; py < 4; py++)
          if (nCurrentY + py < nFieldHeight - 1) {
            bool bLine = true;
            for (int px = 1; px < nFieldWidth - 1; px++)
              bLine &= (pField[(nCurrentY + py) * nFieldWidth + px]) != 0;

            if (bLine) 
            {
              for (int px = 1; px < nFieldWidth - 1; px++)
                bLine &= (pField[(nCurrentY + py) * nFieldWidth + px]) = 8;

              // store current line for removal
              vLines.push_back(nCurrentY + py);
            }
          }

        // every placed piece scores 25 points
        nScore += 25;
        // lines score exponentially larger points!
        if (!vLines.empty()) nScore += (1 << vLines.size()) * 100;

        // choose next piece
        nCurrentX = nFieldWidth / 2;
        nCurrentY = 0;
        nCurrentRotation = 0;
        nCurrentPiece = rand() % 7;

        // increment
        nPieceCount++;
        if (nPieceCount % 10 == 0) nSpeed--;

        // if piece does not fit
        bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
      }
      nSpeedCounter = 0;
    }




    // RENDER OUTPUT ==================================================================


    // draw playfield
    for (int x = 0; x < nFieldWidth; x++)
      for (int y = 0; y < nFieldHeight; y++)
        screen[(y + 2) * nScreenWidth + (x + 2)] = L" ABCDEFG=#"[pField[y * nFieldWidth + x]];

    // draw current piece
    for (int px = 0; px < 4; px++)
      for (int py = 0; py < 4; py++)
        if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X')
          // Sets character to use for piece : 0 + 65 is A | 1 + 65 = B ... etc
          screen[(nCurrentY + py + 2) * nScreenWidth + (nCurrentX + px + 2)] = nCurrentPiece + 65;

    // draw score
    swprintf_s(&screen[2 * nScreenWidth + nFieldWidth + 6], 16, L"SCORE: %8d", nScore);

    // clean up lines
    if (!vLines.empty())
    {
      // display frame (cheekily to draw lines)
      WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0, 0 }, &dwBytesWritten);
      this_thread::sleep_for(400ms); // delay a bit

      // remove the lines and move all the pieces down
      for (auto& v : vLines) {
        for (int px = 1; px < nFieldWidth - 1; px++) 
        {
          for (int py = v; py > 0; py--) 
          {
            pField[py * nFieldWidth + px] = pField[(py - 1) * nFieldWidth + px];
            pField[px] = 0;
          }
        }
      }

      vLines.clear();
    }

    // display frame
    WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0, 0 }, &dwBytesWritten);
  }

  // oh dear
  CloseHandle(hConsole);
  cout << "Game Over!! Score: " << nScore << endl;
  system("pause");
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
