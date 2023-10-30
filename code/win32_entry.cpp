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

#define local_persist static
#define internal static
#define global_variable static

//Statics are initialized to zero by default
global_variable bool Running;

global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;
global_variable int BitmapWidth;
global_variable int BitmapHeight;
  
//Create backbuffer
internal void Win32ResizeDIBSection(int Width, int Height)
{
    if (BitmapMemory)
    {
        VirtualFree(BitmapMemory, 0, MEM_RELEASE);
    }

    BitmapWidth = Width;
    BitmapHeight = Height;

    BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
    BitmapInfo.bmiHeader.biWidth = BitmapWidth;
    //Since memory is 1D, and we need to render the pixels in 2D, we need to decide how to change rows.
    //We set BitmapHeight to negative, allowing us to top-down DIB with origin at upper-left corner.
    BitmapInfo.bmiHeader.biHeight = -BitmapHeight;
    BitmapInfo.bmiHeader.biPlanes = 1;
    //biBitCount will be set to 32 due to alignment reasons, even though we only need 1 byte each for RGB, thus we include an extra byte of padding
    BitmapInfo.bmiHeader.biBitCount = 32;
    BitmapInfo.bmiHeader.biCompression = BI_RGB;

    
    int BytesPerPixel = 4;
    int BitmapMemorySize = (BitmapWidth*BitmapHeight)*BytesPerPixel;
    //Parameters of VirtualAlloc(1, 2, 3 ,4)
    //1) The starting address of the region to allocate as virtual memory. Can be anywhere.
    //2) Rounds up allocated memory to match page size.
    //3) MEM_COMMIT = Start using the memory right away, MEM_RESERVE = We're going to reserve memory to be used in the future.
    //4) PAGE_READWRITE = We're both reading from and writing to the memory
    BitmapMemory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

    int Pitch = Width*BytesPerPixel;
    uint8 *Row = (uint8 *)BitmapMemory;
    for(int Y = 0; Y < BitmapHeight; ++Y)
    {
        uint8 *Pixel = (uint8 *)Row;
        for(int X = 0; X < BitmapWidth; ++X)
        {
            *Pixel = 255; 
            ++Pixel;
            *Pixel = 0;
            ++Pixel;
            *Pixel = 0;
            ++Pixel;
            *Pixel = 0;
            ++Pixel;
        }

        Row += Pitch;
    }
}

//Rectangle to rectangle copy, scaling due to differing sizes
internal void Win32UpdateWindow(HDC DeviceContext, RECT *WindowRect, int X, int Y, int Width, int Height)
{
    int WindowWidth = WindowRect->right - WindowRect->left;
    int WindowHeight = WindowRect->bottom - WindowRect->top;
    StretchDIBits(DeviceContext, 
                /*
                X, Y, Width, Height,
                X, Y, Width, Height,
                */
               0, 0, BitmapWidth, BitmapHeight,
               0, 0, WindowWidth, WindowHeight,
                BitmapMemory,
                &BitmapInfo,
                DIB_RGB_COLORS,
                SRCCOPY);
}
                                
                
//In windows, function signatures only depend on the types you are trying to pass, not the actual name of the parameter. You can rename it however you like.
//This is a function that will be called when windows needs to send us a message
LRESULT Win32MainWindowCallback(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
    LRESULT Result = 0;
    switch(Message)
    {
        case WM_SIZE:
        {
            RECT ClientRect;
            GetClientRect(Window, &ClientRect);
            int Height = ClientRect.bottom - ClientRect.top;
            int Width = ClientRect.right - ClientRect.left;
            Win32ResizeDIBSection(Width, Height);
        } break;
    
        case WM_DESTROY:
        {
            //TODO(Robin) Handle this a error - recreate window?
            Running = false;
        } break;

        case WM_CLOSE:
        {
            //TODO(Robin) Handle this with a message to the user?
            Running = false;
        } break;

        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        } break;

        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;
            int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
            int Width = Paint.rcPaint.right - Paint.rcPaint.left;
            RECT ClientRect;
            GetClientRect(Window, &ClientRect);
            Win32UpdateWindow(DeviceContext, &ClientRect,X, Y, Width, Height);
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
    
        WindowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
        WindowClass.lpfnWndProc = Win32MainWindowCallback;
        WindowClass.hInstance = Instance;
        //HICON     hIcon;
        WindowClass.lpszClassName = "Midnight Madness";
    
        if(RegisterClassA(&WindowClass))
        {
            //We open a window
            HWND WindowHandle = CreateWindowExA(
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
            if(WindowHandle)
            {
                //We enter an infinite loop
                Running = true;
                while(Running)
                {
                    //We ask windows to give us the next message in the message queue
                    MSG Message;
                    BOOL MessageResult = GetMessage(&Message, 0, 0, 0);
                    //If we get a new message (no WM_QUIT or error, for example)..
                    if(MessageResult > 0)
                    {
                        //We tell windows to process the message
                        TranslateMessage(&Message);
                        //We tell windows to dispatch the message to MainWindowCallback, windows wants to be the one who dispatches
                        DispatchMessage(&Message);
                    }
                    else
                    {
                        break;
                    }
                }
            }

            else
            {
                        //TODO(Robin): Logging
            }
        }

        return 0;
    }