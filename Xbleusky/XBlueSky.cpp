
//************************
//NuGet\Install-Package Microsoft.Web.WebView2 -Version 1.0.2739.15
//***************************

//vcpkg install wil
//vcpkg integrate install

// WebRed.cpp : Defines the entry point for the application.



// WebRed.cpp : Defines the entry point for the application.

#include <windows.h>
#include <wrl.h>
#include <wil/com.h>
#include <webview2.h>
#include <string>
#include <sstream>

using namespace Microsoft::WRL;

// Global WebView2 controllers and webview instances
wil::com_ptr<ICoreWebView2Controller> webViewController1;
wil::com_ptr<ICoreWebView2> webView1;

wil::com_ptr<ICoreWebView2Controller> webViewController2;
wil::com_ptr<ICoreWebView2> webView2;

// Search input buffer
wchar_t searchQuery[256] = L"";

// Function to initialize WebView2
void InitWebView2(HWND hwnd, RECT bounds, LPCWSTR url, wil::com_ptr<ICoreWebView2Controller>& controller, wil::com_ptr<ICoreWebView2>& webView) {
    CreateCoreWebView2EnvironmentWithOptions(nullptr, nullptr, nullptr,
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [hwnd, bounds, url, &controller, &webView](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
                env->CreateCoreWebView2Controller(hwnd, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                    [hwnd, bounds, url, &controller, &webView](HRESULT result, ICoreWebView2Controller* newController) -> HRESULT {
                        if (newController != nullptr) {
                            controller = newController;
                            controller->get_CoreWebView2(&webView);

                            // Set bounds for the WebView
                            controller->put_Bounds(bounds);

                            // Navigate to the given URL
                            webView->Navigate(url);
                        }
                        return S_OK;
                    }).Get());
                return S_OK;
            }).Get());
}

// Function to handle search
void PerformSearch() {
    // Check if the search query is empty
    if (wcslen(searchQuery) == 0) {
        MessageBox(NULL, L"Search query is empty! Please enter a search term.", L"Error", MB_OK | MB_ICONERROR);
        return;  // Don't perform search if the query is empty
    }

    if (webView1 && webView2) {
        // Construct the search URLs
        std::wstringstream searchUrl1, searchUrl2;

        // For x.com, append the query with the correct format
        searchUrl1 << L"https://x.com/search?q=" << searchQuery << L"&src=typed_query";

        // For bsky.app, append the query with the correct format
        searchUrl2 << L"https://bsky.app/search?q=" << searchQuery;

        // Debug: Print the URLs to check their format
        wprintf(L"Search URL 1: %s\n", searchUrl1.str().c_str());
        wprintf(L"Search URL 2: %s\n", searchUrl2.str().c_str());

        // Navigate both WebView2 instances to the search URLs
        webView1->Navigate(searchUrl1.str().c_str());
        webView2->Navigate(searchUrl2.str().c_str());
    }
}

// Window procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND inputBox, searchButton;

    switch (uMsg) {
    case WM_SIZE: {
        if (webViewController1 && webViewController2) {
            RECT bounds;
            GetClientRect(hwnd, &bounds);

            // Divide the window into a top bar for search and two equal parts for WebView2
            int barHeight = 40;
            RECT leftBounds = { bounds.left, bounds.top + barHeight, bounds.left + (bounds.right - bounds.left) / 2, bounds.bottom };
            RECT rightBounds = { leftBounds.right, bounds.top + barHeight, bounds.right, bounds.bottom };

            webViewController1->put_Bounds(leftBounds);
            webViewController2->put_Bounds(rightBounds);

            // Center the search tool
            int inputWidth = 400, buttonWidth = 80, elementHeight = 20;
            int totalWidth = inputWidth + buttonWidth + 10;
            int centerX = (bounds.right - bounds.left) / 2 - totalWidth / 2;

            MoveWindow(inputBox, centerX, 10, inputWidth, elementHeight, TRUE);
            MoveWindow(searchButton, centerX + inputWidth + 10, 10, buttonWidth, elementHeight, TRUE);
        }
        return 0;
    }

    case WM_COMMAND:
        if (LOWORD(wParam) == 1) { // Search button clicked
            GetWindowText(inputBox, searchQuery, 256);
            PerformSearch();
        }
        return 0;

    case WM_CREATE: {
        // Create input box
        inputBox = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
            0, 0, 0, 0, hwnd, (HMENU)2, GetModuleHandle(NULL), NULL);

        // Create search button
        searchButton = CreateWindow(L"BUTTON", L"Search", WS_CHILD | WS_VISIBLE,
            0, 0, 0, 0, hwnd, (HMENU)1, GetModuleHandle(NULL), NULL);
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    default:
        if (uMsg == WM_COMMAND && HIWORD(wParam) == EN_CHANGE && LOWORD(wParam) == 2) {
            // Update the search query as the user types
            GetWindowText(inputBox, searchQuery, 256);
        }
        break;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Entry point for the application
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"DualWebView2Window";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles
        CLASS_NAME,                     // Window class
        L"Dual WebView2 with Search",   // Window text
        WS_OVERLAPPEDWINDOW,            // Window style
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, // Size and position
        NULL,                           // Parent window
        NULL,                           // Menu
        hInstance,                      // Instance handle
        NULL                            // Additional application data
    );

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    // Get the client area bounds
    RECT bounds;
    GetClientRect(hwnd, &bounds);

    // Define bounds for the left and right WebView2 instances
    int barHeight = 40;
    RECT leftBounds = { bounds.left, bounds.top + barHeight, bounds.left + (bounds.right - bounds.left) / 2, bounds.bottom };
    RECT rightBounds = { leftBounds.right, bounds.top + barHeight, bounds.right, bounds.bottom };

    // Initialize the two WebViews with default URLs
    InitWebView2(hwnd, leftBounds, L"https://x.com", webViewController1, webView1);
    InitWebView2(hwnd, rightBounds, L"https://bsky.app", webViewController2, webView2);

    // Run the message loop
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}



/*
#include <windows.h>
#include <wrl.h>
#include <wil/com.h>
#include <webview2.h>
#include <string>
#include <sstream>

using namespace Microsoft::WRL;

// Global WebView2 controllers and webview instances
wil::com_ptr<ICoreWebView2Controller> webViewController1;
wil::com_ptr<ICoreWebView2> webView1;

wil::com_ptr<ICoreWebView2Controller> webViewController2;
wil::com_ptr<ICoreWebView2> webView2;

// Search input buffer
wchar_t searchQuery[256] = L"";

// Function to initialize WebView2
void InitWebView2(HWND hwnd, RECT bounds, LPCWSTR url, wil::com_ptr<ICoreWebView2Controller>& controller, wil::com_ptr<ICoreWebView2>& webView) {
    CreateCoreWebView2EnvironmentWithOptions(nullptr, nullptr, nullptr,
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [hwnd, bounds, url, &controller, &webView](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
                env->CreateCoreWebView2Controller(hwnd, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                    [hwnd, bounds, url, &controller, &webView](HRESULT result, ICoreWebView2Controller* newController) -> HRESULT {
                        if (newController != nullptr) {
                            controller = newController;
                            controller->get_CoreWebView2(&webView);

                            // Set bounds for the WebView
                            controller->put_Bounds(bounds);

                            // Navigate to the given URL
                            webView->Navigate(url);
                        }
                        return S_OK;
                    }).Get());
                return S_OK;
            }).Get());
}

// Function to handle search
void PerformSearch() {
    if (webView1 && webView2) {
        // Construct the search URLs
        std::wstringstream searchUrl1, searchUrl2;

        // For x.com, append the query with the correct format
        searchUrl1 << L"https://x.com/search?q=" << searchQuery << L"&src=typed_query";

        // For bsky.app, append the query with the correct format
        searchUrl2 << L"https://bsky.app/search?q=" << searchQuery;

        // Navigate both WebView2 instances to the search URLs
        webView1->Navigate(searchUrl1.str().c_str());
        webView2->Navigate(searchUrl2.str().c_str());
    }
}




// Window procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND inputBox, searchButton;

    switch (uMsg) {
    case WM_SIZE: {
        if (webViewController1 && webViewController2) {
            RECT bounds;
            GetClientRect(hwnd, &bounds);

            // Divide the window into a top bar for search and two equal parts for WebView2
            int barHeight = 40;
            RECT leftBounds = { bounds.left, bounds.top + barHeight, bounds.left + (bounds.right - bounds.left) / 2, bounds.bottom };
            RECT rightBounds = { leftBounds.right, bounds.top + barHeight, bounds.right, bounds.bottom };

            webViewController1->put_Bounds(leftBounds);
            webViewController2->put_Bounds(rightBounds);

            // Center the search tool
            int inputWidth = 400, buttonWidth = 80, elementHeight = 20;
            int totalWidth = inputWidth + buttonWidth + 10;
            int centerX = (bounds.right - bounds.left) / 2 - totalWidth / 2;

            MoveWindow(inputBox, centerX, 10, inputWidth, elementHeight, TRUE);
            MoveWindow(searchButton, centerX + inputWidth + 10, 10, buttonWidth, elementHeight, TRUE);
        }
        return 0;
    }

    case WM_COMMAND:
        if (LOWORD(wParam) == 1) { // Search button clicked
            PerformSearch();
        }
        return 0;

    case WM_CREATE: {
        // Create input box
        inputBox = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
            0, 0, 0, 0, hwnd, (HMENU)2, GetModuleHandle(NULL), NULL);

        // Create search button
        searchButton = CreateWindow(L"BUTTON", L"Search", WS_CHILD | WS_VISIBLE,
            0, 0, 0, 0, hwnd, (HMENU)1, GetModuleHandle(NULL), NULL);
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    default:
        if (uMsg == WM_COMMAND && HIWORD(wParam) == EN_CHANGE && LOWORD(wParam) == 2) {
            // Update the search query as the user types
            GetWindowText(inputBox, searchQuery, 256);
        }
        break;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Entry point for the application
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"DualWebView2Window";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles
        CLASS_NAME,                     // Window class
        L"Dual WebView2 with Search",   // Window text
        WS_OVERLAPPEDWINDOW,            // Window style
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, // Size and position
        NULL,                           // Parent window
        NULL,                           // Menu
        hInstance,                      // Instance handle
        NULL                            // Additional application data
    );

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    // Get the client area bounds
    RECT bounds;
    GetClientRect(hwnd, &bounds);

    // Define bounds for the left and right WebView2 instances
    int barHeight = 40;
    RECT leftBounds = { bounds.left, bounds.top + barHeight, bounds.left + (bounds.right - bounds.left) / 2, bounds.bottom };
    RECT rightBounds = { leftBounds.right, bounds.top + barHeight, bounds.right, bounds.bottom };

    // Initialize the two WebViews with default URLs
    InitWebView2(hwnd, leftBounds, L"https://x.com", webViewController1, webView1);
    InitWebView2(hwnd, rightBounds, L"https://bsky.app", webViewController2, webView2);

    // Run the message loop
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
*/