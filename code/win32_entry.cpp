/*

    - hInstance is the handle to an instance or handle to a module. The operating system uses this value to identify the executable or EXE when it's loaded in memory. Certain Windows functions need the instance handle, for example to load icons or bitmaps.
    - hPrevInstance has no meaning. It was used in 16-bit Windows, but is now always zero.
    - pCmdLine contains the command-line arguments as a Unicode string.
    - nCmdShow is a flag that indicates whether the main application window is minimized, maximized, or shown normally.

*/

#include <windows.h>
#include <stdio.h>

//Local persists should only be used for debugging
//Internal = only this source file (translation unit) can modify the value

typedef unsigned char uint8;
typedef unsigned int uint32;
typedef int bool32;

#define local_persist static
#define internal static
#define global_variable static



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

struct win32_window_dimension
{
    int Width;
    int Height;
};

internal win32_window_dimension GetWindowDimension(HWND Window)
{
    win32_window_dimension Result;

    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Height = ClientRect.bottom - ClientRect.top;
    Result.Width = ClientRect.right - ClientRect.left;

    return Result;
};


internal void RenderWeirdGradient(win32_offscreen_buffer *Buffer, int XOffset, int YOffset)
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
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

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
                int XOffset = 0;
                int YOffset = 0;
                //We enter an infinite loop
                GlobalRunning = true;
                while(GlobalRunning)
                {
                    HDC DeviceContext = GetDC(Window);
                    
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
                    RenderWeirdGradient(&GlobalBackbuffer, XOffset, YOffset);

                    win32_window_dimension Dimension = GetWindowDimension(Window);
                    Win32DisplayBufferInWindow(&GlobalBackbuffer, DeviceContext, Dimension.Width, Dimension.Height);

                    ReleaseDC(Window, DeviceContext);
                    ++XOffset;
                }
            }

            else
            {
                        //TODO(Robin): Logging
            }
        }

        return 0;
    }