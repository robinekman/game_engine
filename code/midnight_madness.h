#if !defined(MIDNIGHT_MADNESS_H)

#define Pi32 3.14159265359f

typedef short int16;
typedef int int32;
typedef long long int64;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;
typedef int bool32;

typedef float float32;
typedef double float64;



#define local_persist static
#define internal static
#define global_variable static

//Services that the platform provides to the game

//Services that the game provides to the platform
//FOUR THINGS - timing, keyboard input, bitmap buffer to use, sound buffer to use
struct game_offscreen_buffer
{
    void *Memory;
    int Width;
    int Height;
    int Pitch;
};

struct game_sound_output_buffer
{
    int SamplesPerSecond;
    int SampleCount;
    int16 *Samples;
};

internal void GameUpdateAndRender(game_offscreen_buffer *Buffer, int XOffset, int YOffset, game_sound_output_buffer *SoundBuffer, int ToneHz);

#define MIDNIGHT_MADNESS_H
#endif