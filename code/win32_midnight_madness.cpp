/*

    - hInstance is the handle to an instance or handle to a module. The operating system uses this value to identify the executable or EXE when it's loaded in memory. Certain Windows functions need the instance handle, for example to load icons or bitmaps.
    - hPrevInstance has no meaning. It was used in 16-bit Windows, but is now always zero.
    - pCmdLine contains the command-line arguments as a Unicode string.
    - nCmdShow is a flag that indicates whether the main application window is minimized, maximized, or shown normally.

*/
#include <windows.h>
#include <stdio.h>
#include <dsound.h>
#include <math.h>

#include "midnight_madness.cpp"



//Local persists should only be used for debugging
//Internal = only this source file (translation unit) can modify the value

/*
Partial list of stuff to do for the platform layer:
- Saved game location
- Getting handle to our own exe file
- Asset loading path
- Multithreading
- Raw Input (support for multiple keyboards)
- Sleep/timeBeginPeriod
- ClipCursor() (for multimonitor support)
- Fullscreen support - 
- WM-SETCURSOR (control cursir visibility)
- QueryCancelAutoplay
- WM_ACTIVATEAPP (for when we are not the active app)
- Blit speed improvement (BitBlt)
- Hardware acceleration (OpenGL, Direct3D, or both?)
- GetKeyboardLayout (different keyboard layouts)
*/

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID lpGUID, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter);
typedef DIRECT_SOUND_CREATE(direct_sound_create);



struct win32_offscreen_buffer
{
    BITMAPINFO Info;
    void *Memory;
    int Width;
    int Height;
    int Pitch;
};

global_variable bool GlobalRunning;
global_variable win32_offscreen_buffer GlobalBackbuffer;
global_variable LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer;


struct win32_window_dimension
{
    int Width;
    int Height;
};

struct win32_sound_output
{
    int SamplesPerSecond;
    int ToneHz;
    uint32 RunningSampleIndex;
    int WavePeriod;
    //int HalfWavePeriod = WavePeriod/2;
    int BytesPerSample;
    int SecondaryBufferSize;
    int ToneVolume;
};

internal void Win32FillSoundBuffer(win32_sound_output *SoundOutput, DWORD ByteToLock, DWORD BytesToWrite, game_sound_output_buffer *SourceBuffer)
{
    VOID *Region1;
    DWORD Region1Size;
    VOID *Region2;
    DWORD Region2Size;

    //Goes on forever

    if(SUCCEEDED(GlobalSecondaryBuffer->Lock(ByteToLock, BytesToWrite, &Region1, &Region1Size, &Region2, &Region2Size, 0)))
    {
    //TODO(Robin) assert that Region1Size and Region2Size are valid

        DWORD Region1SampleCount = Region1Size/SoundOutput->BytesPerSample;
        int16 *DestSample = (int16 *)Region1;
        int16 *SourceSample = SourceBuffer -> Samples;
        for(DWORD i = 0; i < Region1SampleCount; ++i)
        {
            *DestSample++ = *SourceSample++;
            *DestSample++ = *SourceSample++;
            ++SoundOutput->RunningSampleIndex;
        }

        DWORD Region2SampleCount = Region2Size/SoundOutput->BytesPerSample;
        DestSample = (int16 *)Region2;
        for(DWORD i = 0; i < Region2SampleCount; ++i)
        {
            *DestSample++ = *SourceSample++;
            *DestSample++ = *SourceSample++;
            ++SoundOutput->RunningSampleIndex;
        }

        GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
    }
}



internal void Win32InitDSound(HWND Window, int32 SamplesPerSecond, int32 BufferSize)
{
    HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");

    if (DSoundLibrary)
    {
        direct_sound_create *DirectSoundCreate = (direct_sound_create *)GetProcAddress(DSoundLibrary, "DirectSoundCreate");

        LPDIRECTSOUND DirectSound;

        if (DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0)))
        {
            tWAVEFORMATEX WaveFormat = {};
            WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
            WaveFormat.nChannels = 2;
            WaveFormat.nSamplesPerSec = SamplesPerSecond;
            WaveFormat.wBitsPerSample = 16;
            WaveFormat.nBlockAlign = (WaveFormat.nChannels*WaveFormat.wBitsPerSample) / 8;
            WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec*WaveFormat.nBlockAlign;
            WaveFormat.cbSize = 0;

            if(SUCCEEDED(DirectSound -> SetCooperativeLevel(Window, DSSCL_PRIORITY)))
            {
                //Initializes the size of the BufferDescription to zero
                //The primary buffer acts as a handle to the primary sound device, so we can set its format, allowing the sound card to play in the format we want
                DSBUFFERDESC BufferDescription = {};
                BufferDescription.dwSize = sizeof(BufferDescription);
                BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

                //Create a primary buffer
                LPDIRECTSOUNDBUFFER PrimaryBuffer;
                if SUCCEEDED((DirectSound -> CreateSoundBuffer(&BufferDescription, &PrimaryBuffer, 0)))
                {
                    //What should the graph of the sound look like
                    
                   if(SUCCEEDED(PrimaryBuffer->SetFormat(&WaveFormat)))
                   {
                   }
                   else
                   {
                    //TODO(Robin) Diagnostics
                   }
                }
                else
                {
                }
            }
            else
            {
                //TODO(Robin) Diagnostic
            }
        
            //This is the sound
            DSBUFFERDESC BufferDescription = {};
            BufferDescription.dwSize = sizeof(BufferDescription);
            BufferDescription.dwFlags = 0;
            BufferDescription.dwBufferBytes = BufferSize;
            BufferDescription.lpwfxFormat = &WaveFormat;

            if SUCCEEDED((DirectSound -> CreateSoundBuffer(&BufferDescription, &GlobalSecondaryBuffer, 0)))
                {
                    //What should the graph of the sound look like
                    
                   if(SUCCEEDED(GlobalSecondaryBuffer->SetFormat(&WaveFormat)))
                   {

                   }
                   else
                   {
                    //TODO(Robin) Diagnostics
                   }
            }
            else
            {
                //TODO(Robin) Diagntostic

            }
        }
    }
}

internal win32_window_dimension GetWindowDimension(HWND Window)
{
    win32_window_dimension Result;

    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Height = ClientRect.bottom - ClientRect.top;
    Result.Width = ClientRect.right - ClientRect.left;

    return Result;
};

  
//Create backbuffer
internal void Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height)
{
    if (Buffer->Memory)
    {
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }

    Buffer->Width = Width;
    Buffer->Height = Height;
    int BytesPerPixel = 4;

    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    //Since memory is 1D, and we need to render the pixels in 2D, we need to decide how to change rows.
    //We set BitmapHeight to negative, allowing us to top-down DIB with origin at upper-left corner.
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
    Buffer->Info.bmiHeader.biPlanes = 1;
    //biBitCount will be set to 32 due to alignment reasons, even though we only need 1 byte each for RGB, thus we include an extra byte of padding
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;

    
    int BitmapMemorySize = (Buffer->Width*Buffer->Height)*BytesPerPixel;
    //Parameters of VirtualAlloc(1, 2, 3 ,4)
    //1) The starting address of the region to allocate as virtual memory. Can be anywhere.
    //2) Rounds up allocated memory to match page size.
    //3) MEM_COMMIT = Start using the memory right away, MEM_RESERVE = We're going to reserve memory to be used in the future.
    //4) PAGE_READWRITE = We're both reading from and writing to the memory
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

    Buffer->Pitch = Width*BytesPerPixel;
    //TODO(Robin) probably clear this to black

}

//Rectangle to rectangle copy, scaling due to differing sizes
internal void Win32DisplayBufferInWindow(win32_offscreen_buffer *Buffer, HDC DeviceContext, int WindowWidth, int WindowHeight)
{
    //TODO(Robin) Aspect ratio correction
    //TODO(Robin) Play with stretch modes
    StretchDIBits(DeviceContext, 
                /*
                X, Y, Width, Height,
                X, Y, Width, Height,
                */
               0, 0, WindowWidth, WindowHeight,
               0, 0, Buffer->Width, Buffer->Height,
                Buffer->Memory,
                &Buffer->Info,
                DIB_RGB_COLORS,
                SRCCOPY);
}
                                
                
//In windows, function signatures only depend on the types you are trying to pass, not the actual name of the parameter. You can rename it however you like.
//This is a function that will be called when windows needs to send us a message
//WParam and LParam are anonymous parameters in different functions, their datatypes change based on the function
//Message is used to decrypt what WParam and LParam are
internal LRESULT Win32MainWindowCallback(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
    LRESULT Result = 0;
    switch(Message)
    {
        case WM_SIZE:
        {

        } break;
    
        case WM_DESTROY:
        {
            //TODO(Robin) Handle this a error - recreate window?
            GlobalRunning = false;
        } break;

        case WM_CLOSE:
        {
            //TODO(Robin) Handle this with a message to the user?
            GlobalRunning = false;
        } break;

        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        } break;

        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            uint32 VKCode = WParam;
            bool WasDown = ((LParam & (1 << 30)) != 0);
            bool IsDown = ((LParam & (1 << 31)) == 0);
            if(WasDown != IsDown)
            {
                if(VKCode == 'W')
                {
                    if(WasDown)
                    {
                        printf("Was down\n");
                    }
                    if(IsDown)
                    {
                        printf("Is down\n");
                    }
                }
                else if(VKCode == 'A')
                {

                }
                else if(VKCode == 'S')
                {
                    
                }
                else if(VKCode == 'D')
                {
                    
                }
                else if(VKCode == 'Q')
                {
                    
                }
                else if(VKCode == 'E')
                {
                    
                }
                else if(VKCode == VK_UP)
                {
                    
                }
                else if(VKCode == VK_LEFT)
                {
                    
                }
                else if(VKCode == VK_DOWN)
                {
                    
                }
                else if(VKCode == VK_RIGHT)
                {
                    
                }
                else if(VKCode == VK_ESCAPE)
                {
                    
                }
                else if(VKCode == VK_SPACE)
                {
                }
            } 

            bool32 AltKeyWasDown = (LParam & (1 << 29));
            if(VKCode == VK_F4 && AltKeyWasDown)
            {
                GlobalRunning = false;
            }
        } break;

        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;
            int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
            int Width = Paint.rcPaint.right - Paint.rcPaint.left;

            win32_window_dimension Dimension = GetWindowDimension(Window);

            Win32DisplayBufferInWindow(&GlobalBackbuffer, DeviceContext, Dimension.Width, Dimension.Height);
            EndPaint(Window, &Paint);
        } break;

        default:
        {
            OutputDebugStringA("default\n");
            Result = DefWindowProc(Window, Message, WParam, LParam);
        } break;
    } 
    return(Result);
}

int WINAPI WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, PSTR CommandLine, int ShowCode)
{
    LARGE_INTEGER PerfCounterFrequencyResult;
    QueryPerformanceFrequency(&PerfCounterFrequencyResult);
    int64 PerfCountFrequency = PerfCounterFrequencyResult.QuadPart;

    WNDCLASSA WindowClass = {};

    Win32ResizeDIBSection(&GlobalBackbuffer, 1280, 720);

    WindowClass.style = CS_HREDRAW|CS_VREDRAW;
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = Instance;
    //HICON     hIcon;
    WindowClass.lpszClassName = "Midnight Madness";

    if(RegisterClassA(&WindowClass))
    {
        //We open a window
        HWND Window = CreateWindowExA(
            0,
            WindowClass.lpszClassName,
            "Midnight Madness",
            WS_OVERLAPPEDWINDOW|WS_VISIBLE,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            0,
            0,
            Instance,
            0);

        //If windows returns control to us..
        if(Window)
        {
            HDC DeviceContext = GetDC(Window);

            //Graphics test
            int XOffset = 0;
            int YOffset = 0;

            win32_sound_output SoundOutput = {};
            
            SoundOutput.SamplesPerSecond = 48000;
            SoundOutput.ToneHz = 256;
            SoundOutput.RunningSampleIndex = 0;
            SoundOutput.WavePeriod = SoundOutput.SamplesPerSecond/SoundOutput.ToneHz;
            SoundOutput.BytesPerSample = sizeof(int16)*2;
            SoundOutput.SecondaryBufferSize = SoundOutput.SamplesPerSecond*SoundOutput.BytesPerSample;
            SoundOutput.ToneVolume = 1000;
            Win32InitDSound(Window, SoundOutput.SamplesPerSecond, SoundOutput.SecondaryBufferSize);
            GlobalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);

            int16 *Samples = (int16 *) VirtualAlloc(0, SoundOutput.SecondaryBufferSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

            GlobalRunning = true;
            LARGE_INTEGER LastCounter;
            QueryPerformanceCounter(&LastCounter);
            int64 LastCycleCount = __rdtsc();

            //We enter an infinite loop
            while(GlobalRunning)
            {
                

                //We process messages as long as there are messages in the queue
                MSG Message;
                while(PeekMessageA(&Message, 0, 0, 0, PM_REMOVE))
                {
                    if(Message.message == WM_QUIT)
                    {
                        GlobalRunning = false;
                    }

                //If we get a new message (no WM_QUIT or error, for example)..
                
                    //We tell windows to process the message
                    TranslateMessage(&Message);
                    //We tell windows to dispatch the message to MainWindowCallback, windows wants to be the one who dispatches
                    DispatchMessage(&Message);
                }

                

                DWORD ByteToLock;
                DWORD BytesToWrite;
                DWORD PlayCursor;
                DWORD WriteCursor;
                bool32 SoundIsValid = false;


                if(SUCCEEDED(GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor)))
                {
                    ByteToLock = (SoundOutput.RunningSampleIndex*SoundOutput.BytesPerSample) % SoundOutput.SecondaryBufferSize;

                    if(ByteToLock > PlayCursor)
                    {
                        BytesToWrite = SoundOutput.SecondaryBufferSize-ByteToLock;
                        BytesToWrite += PlayCursor;
                    }
                    else
                    {
                        BytesToWrite = PlayCursor - ByteToLock;
                    }

                    SoundIsValid = true;

                }

                game_sound_output_buffer SoundBuffer = {};
                SoundBuffer.SamplesPerSecond = SoundOutput.SamplesPerSecond;
                SoundBuffer.SamplesPerSecond = SoundOutput.SamplesPerSecond;
                SoundBuffer.SampleCount = BytesToWrite / SoundOutput.BytesPerSample;
                SoundBuffer.Samples = Samples;

                game_offscreen_buffer Buffer = {};
                Buffer.Memory = GlobalBackbuffer.Memory;
                Buffer.Width = GlobalBackbuffer.Width;
                Buffer.Height = GlobalBackbuffer.Height;
                Buffer.Pitch = GlobalBackbuffer.Pitch;
                GameUpdateAndRender(&Buffer, XOffset, YOffset, &SoundBuffer, SoundOutput.ToneHz);

                if(SoundIsValid)
                {
                    Win32FillSoundBuffer(&SoundOutput, ByteToLock, BytesToWrite, &SoundBuffer);
                }



                win32_window_dimension Dimension = GetWindowDimension(Window);
                Win32DisplayBufferInWindow(&GlobalBackbuffer, DeviceContext, Dimension.Width, Dimension.Height);

                ++XOffset;

                LARGE_INTEGER EndCounter;
                QueryPerformanceCounter(&EndCounter);

                int64 EndCycleCount = __rdtsc();

                int64 CyclesElapsed = EndCycleCount - LastCycleCount;
                int64 CounterElapsed = EndCounter.QuadPart - LastCounter.QuadPart;
                int32 microsecPerFrame = (1000'000*CounterElapsed) / PerfCountFrequency;
                uint16 FramesPerSecond = 1000'000/microsecPerFrame;
                int32 MegaCyclesPerFrame = CyclesElapsed/1000'000;

                char StringBuffer[256];
                wsprintfA(StringBuffer, "FPS: %d   |   Megacycles per frame: %d\n ", FramesPerSecond, MegaCyclesPerFrame);
                OutputDebugStringA(StringBuffer);
                
                LastCounter = EndCounter;
                LastCycleCount = EndCycleCount;
            }
                ReleaseDC(Window, DeviceContext);
        }

        else
        {
                    //TODO(Robin): Logging
        }
    }

    return 0;
}