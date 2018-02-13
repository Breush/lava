#include "./window-impl.hpp"

#include <lava/chamber/logger.hpp>

namespace {
    LRESULT CALLBACK onWindowMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        auto* window = reinterpret_cast<lava::crater::Window::Impl*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        return window->processEvent(uMsg, wParam, lParam) ? 0 : DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

using namespace lava::chamber;
using namespace lava::crater;

Window::Impl::Impl(VideoMode mode, const std::string& /* title */)
    : IWindowImpl(mode)
{
    DWORD style = WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    const wchar_t* className = L"lava::crater::Window";
    const wchar_t* title = L"Some not custom title";
    m_hinstance = GetModuleHandle(nullptr);

    // @todo To be done once only
    WNDCLASS wc = {};
    wc.lpfnWndProc = onWindowMessage;
    wc.hInstance = m_hinstance;
    wc.lpszClassName = className;
    RegisterClass(&wc);

    m_hwnd = CreateWindowEx(0, className, title, style, CW_USEDEFAULT, CW_USEDEFAULT, mode.width, mode.height, nullptr, nullptr,
                            m_hinstance, nullptr);

    SetWindowLongPtrW(m_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    if (m_hwnd == nullptr) {
        logger.error("magma.vulkan.window") << "Unable to create window handle: " << GetLastError() << "." << std::endl;
    }

    ShowWindow(m_hwnd, SW_SHOWDEFAULT);
    UpdateWindow(m_hwnd);
}

WindowHandle Window::Impl::windowHandle() const
{
    WindowHandle windowHandle;
    windowHandle.dwm.hwnd = m_hwnd;
    windowHandle.dwm.hinstance = m_hinstance;
    return windowHandle;
}

void Window::Impl::processEvents()
{
    MSG msg;
    while (PeekMessage(&msg, m_hwnd, 0, 0, PM_REMOVE) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

bool Window::Impl::processEvent(UINT uMsg, WPARAM /*wParam*/, LPARAM lParam)
{
    switch (uMsg) {
    case WM_DESTROY: {
        PostQuitMessage(0);
        return true;
    }

    case WM_CLOSE: {
        Event event;
        event.type = Event::WindowClosed;
        pushEvent(event);
        return true;
    }

    case WM_SETCURSOR: {
        // @todo We should probably take the one in params
        SetCursor(LoadCursor(nullptr, IDC_ARROW));
        return true;
    }

    case WM_LBUTTONDOWN: {
        Event event;
        event.type = Event::MouseButtonPressed;
        event.mouseButton.which = MouseButton::Left;
        event.mouseButton.x = static_cast<int16_t>(LOWORD(lParam));
        event.mouseButton.y = static_cast<int16_t>(HIWORD(lParam));
        pushEvent(event);
        break;
    }

    // @todo Fill other event

    case WM_MOUSEMOVE: {
        // Extract the mouse local coordinates
        int x = static_cast<int16_t>(LOWORD(lParam));
        int y = static_cast<int16_t>(HIWORD(lParam));

        Event event;
        event.type = Event::MouseMoved;
        event.mouseMove.x = x;
        event.mouseMove.y = y;
        pushEvent(event);
        break;
    }
    }

    return false;
}
