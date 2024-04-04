#include "framework.h"
#include "UU.h"
#include <thread>
#include <chrono>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <ctime> // Include ctime for time_t
#include <cmath>


HBRUSH hRedBrush;
HWND hWnd;
POINT cursorPosition = { 0, 0 };
std::vector<POINT> figurePositions;
const int numFigures = 1; // Initial number of figures
const int releaseInterval = 5000; // Interval in milliseconds to release a new figure (5 seconds)
bool gameEnded = false;
int countBalls = 1;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

void UpdateFigurePositions();
void ReleaseNewFigure();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_UU, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_UU));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_UU));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = hRedBrush = CreateSolidBrush(RGB(0, 0, 0));
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_UU);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Store instance handle in our global variable

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, // Use WS_OVERLAPPEDWINDOW style
        0, 0, screenWidth, screenHeight, // Set width and height to screen size
        nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // Initialize figure positions
    srand(static_cast<unsigned int>(time(NULL)));
    for (int i = 0; i < numFigures; ++i)
    {
        POINT figurePos = { rand() % screenWidth, -20 }; // Initial position is above the window
        figurePositions.push_back(figurePos);
    }

    // Start figure position update thread
    std::thread figureUpdateThread(UpdateFigurePositions);
    figureUpdateThread.detach();

    // Start releasing new figures thread
    std::thread releaseFigureThread(ReleaseNewFigure);
    releaseFigureThread.detach();

    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_MOUSEMOVE:
        cursorPosition.x = LOWORD(lParam);
        cursorPosition.y = HIWORD(lParam);
        break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        // Draw figures
        HBRUSH hWhiteBrush = CreateSolidBrush(RGB(255, 255, 255));
        SelectObject(hdc, hWhiteBrush);
        for (const auto& figurePos : figurePositions)
        {
            Ellipse(hdc, figurePos.x - 20, figurePos.y - 20, figurePos.x + 20, figurePos.y + 20);
        }
        DeleteObject(hWhiteBrush);

        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void UpdateFigurePositions()
{
    while (!gameEnded)
    {
        // Move figures towards cursor
        for (size_t i = 0; i < figurePositions.size(); ++i)
        {
            POINT& figurePos = figurePositions[i];
            POINT direction = { cursorPosition.x - figurePos.x, cursorPosition.y - figurePos.y };
            double distance = sqrt(static_cast<double>(direction.x * direction.x + direction.y * direction.y));
            double speed = 7.0; // Constant speed
            if (distance > speed)
            {
                direction.x = static_cast<LONG>(speed * direction.x / distance);
                direction.y = static_cast<LONG>(speed * direction.y / distance);
            }
            figurePos.x += static_cast<LONG>(direction.x);
            figurePos.y += static_cast<LONG>(direction.y);

            // Check collision with cursor
            double distanceToCursor = sqrt(pow(cursorPosition.x - figurePos.x, 2) + pow(cursorPosition.y - figurePos.y, 2));
            if (distanceToCursor <= 20) // If the distance between figure and cursor is less than or equal to 20 (radius of figure)
            {
                gameEnded = true;
                std::wstring countBallsStr = std::to_wstring(countBalls);
                std::wstring message = L"You've lost! Your score is " + countBallsStr;
                MessageBox(hWnd, message.c_str(), L"Game Over", MB_OK | MB_ICONERROR);
                return;
            }

            // Check collision with other figures
            for (size_t j = i + 1; j < figurePositions.size(); ++j)
            {
                POINT& otherFigurePos = figurePositions[j];
                double distanceBetweenFigures = sqrt(pow(otherFigurePos.x - figurePos.x, 2) + pow(otherFigurePos.y - figurePos.y, 2));
                if (distanceBetweenFigures <= 40) // If the distance between two figures is less than or equal to 40 (2 * radius of figure)
                {
                    POINT normal = { otherFigurePos.x - figurePos.x, otherFigurePos.y - figurePos.y };
                    double normalLength = sqrt(static_cast<double>(normal.x * normal.x + normal.y * normal.y));
                    normal.x /= static_cast<LONG>(normalLength);
                    normal.y /= static_cast<LONG>(normalLength);

                    POINT relativeVelocity = { direction.x, direction.y };

                    double dotProduct = relativeVelocity.x * normal.x + relativeVelocity.y * normal.y;

                    double impulseScalar = 5.0 * dotProduct / (1 + 1); // Mass of figures is assumed to be 1

                    relativeVelocity.x -= impulseScalar * normal.x;
                    relativeVelocity.y -= impulseScalar * normal.y;

                    direction.x -= relativeVelocity.x;
                    direction.y -= relativeVelocity.y;
                    figurePos.x += static_cast<LONG>(direction.x);
                    figurePos.y += static_cast<LONG>(direction.y);

                    otherFigurePos.x += static_cast<LONG>(relativeVelocity.x);
                    otherFigurePos.y += static_cast<LONG>(relativeVelocity.y);
                }
            }

            // Bounce off the walls
            if (figurePos.x <= 20 || figurePos.x >= GetSystemMetrics(SM_CXSCREEN) - 20) // If the figure touches the left or right wall
                direction.x = -direction.x;
            if (figurePos.y <= 20 || figurePos.y >= GetSystemMetrics(SM_CYSCREEN) - 20) // If the figure touches the top or bottom wall
                direction.y = -direction.y;
        }

        // Redraw window
        InvalidateRect(hWnd, NULL, TRUE);
        UpdateWindow(hWnd);

        // Sleep for 10ms 
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void ReleaseNewFigure()
{
    while (!gameEnded) {
        // Wait for release interval
        std::this_thread::sleep_for(std::chrono::milliseconds(releaseInterval));

        // Add a new figure from the top
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        POINT figurePos = { rand() % screenWidth, 0 }; // Initial position is above the window
        figurePositions.push_back(figurePos);
        countBalls++;
    }
}

