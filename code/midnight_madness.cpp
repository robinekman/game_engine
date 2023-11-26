#include "midnight_madness.h"

internal void GameOutputSound(game_sound_output_buffer *SoundBuffer, int ToneHz)
{
    local_persist float32 tSine;
    int16 ToneVolume = 3000;
    int WavePeriod = SoundBuffer->SamplesPerSecond/ToneHz;

    int16 *SampleOut = SoundBuffer->Samples;

    for(int i = 0; i < SoundBuffer->SampleCount; ++i)
    {
        float32 SineValue = sinf(tSine);
        int16 SampleValue = (int16)(SineValue * ToneVolume);
        *SampleOut++ = SampleValue;
        *SampleOut++ = SampleValue;

        tSine += 2.0f * Pi32 * 1.0f / (float32)WavePeriod;
    }
}
internal void RenderWeirdGradient(game_offscreen_buffer *Buffer, int XOffset, int YOffset)
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

internal void GameUpdateAndRender(game_offscreen_buffer *Buffer, int XOffset, int YOffset, game_sound_output_buffer *SoundBuffer, int ToneHz)
{
    GameOutputSound(SoundBuffer, ToneHz);
    RenderWeirdGradient(Buffer, XOffset, YOffset);
}
