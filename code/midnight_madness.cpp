#include "midnight_madness.h"


internal void GameUpdateAndRender(game_offscreen_buffer *Buffer, int XOffset, int YOffset)
{
    int Width = Buffer->Width;
    int Height = Buffer->Height;

    uint8 *Row = (uint8 *)Buffer->Memory;
    for(int Y = 0; Y < Buffer->Height; ++Y)
    {
        uint32 *Pixel = (uint32 *)Row;
        for(int X = 0; X < Buffer->Width; ++X)
        {
            //The (Padding)RGB channels are BGR(Padding) in Windows
            /*
            32 bit Memory: BB GG RR xx
            32 bit Register: xx RR GG BB 
            */

            uint8 Blue= (X + XOffset);
            uint8 Green = (Y + YOffset);
            uint8 Red = 0;
            uint8 Padding = 0;

            *Pixel++ = ((Padding << 24) | (Red << 16) | (Green << 8) | Blue );
        }

        Row += Buffer->Pitch;
    }
}