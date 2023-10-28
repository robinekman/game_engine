/*

    - hInstance is the handle to an instance or handle to a module. The operating system uses this value to identify the executable or EXE when it's loaded in memory. Certain Windows functions need the instance handle, for example to load icons or bitmaps.
    - hPrevInstance has no meaning. It was used in 16-bit Windows, but is now always zero.
    - pCmdLine contains the command-line arguments as a Unicode string.
    - nCmdShow is a flag that indicates whether the main application window is minimized, maximized, or shown normally.

*/

#include <windows.h>
//In windows, function signatures only depend on the types you are trying to pass, not the actual name of the parameter. You can rename it however you like.
//This is a function that will be called when windows needs to send something to it to have it do something
LRESULT MainWindowCallback(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
    LRESULT Result = 0;
    switch(Message)
    {
        case WM_SIZE:
        {
            OutputDebugStringA("WM_SIZE\n");
        } break;
    
        case WM_DESTROY:
        {
            OutputDebugStringA("WM_DESTROY\n");
        } break;

        case WM_CLOSE:
        {
            OutputDebugStringA("WM_CLOSE\n");
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
            PatBlt(DeviceContext, X, Y, Width, Height, WHITENESS);

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
        WindowClass.lpfnWndProc = MainWindowCallback;
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
                for(;;)
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