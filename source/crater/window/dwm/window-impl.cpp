#include "./window-impl.hpp"

#include <windowsx.h>

using namespace lava;

namespace {
    LRESULT CALLBACK onWindowMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        auto* window = reinterpret_cast<lava::crater::Window::Impl*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        return window->processEvent(uMsg, wParam, lParam) ? 0 : DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    // Got list from https://msdn.microsoft.com/en-us/library/windows/desktop/dd375731(v=vs.85).aspx
    Key virtualKeyToKey(WPARAM wParam)
    {
        switch (wParam) {
        case 'A': return Key::A;
        case 'B': return Key::B;
        case 'C': return Key::C;
        case 'D': return Key::D;
        case 'E': return Key::E;
        case 'F': return Key::F;
        case 'G': return Key::G;
        case 'H': return Key::H;
        case 'I': return Key::I;
        case 'J': return Key::J;
        case 'K': return Key::K;
        case 'L': return Key::L;
        case 'M': return Key::M;
        case 'N': return Key::N;
        case 'O': return Key::O;
        case 'P': return Key::P;
        case 'Q': return Key::Q;
        case 'R': return Key::R;
        case 'S': return Key::S;
        case 'T': return Key::T;
        case 'U': return Key::U;
        case 'V': return Key::V;
        case 'W': return Key::W;
        case 'X': return Key::X;
        case 'Y': return Key::Y;
        case 'Z': return Key::Z;
        case VK_ESCAPE: return Key::Escape;
        case VK_F1: return Key::F1;
        case VK_F2: return Key::F2;
        case VK_F3: return Key::F3;
        case VK_F4: return Key::F4;
        case VK_F5: return Key::F5;
        case VK_F6: return Key::F6;
        case VK_F7: return Key::F7;
        case VK_F8: return Key::F8;
        case VK_F9: return Key::F9;
        case VK_F10: return Key::F10;
        case VK_F11: return Key::F11;
        case VK_F12: return Key::F12;
        case VK_LEFT: return Key::Left;
        case VK_UP: return Key::Up;
        case VK_RIGHT: return Key::Right;
        case VK_DOWN: return Key::Down;
        case VK_LMENU: return Key::LeftAlt;
        case VK_RMENU: return Key::RightAlt;
        case VK_DELETE: return Key::Delete;
        }

        return Key::Unknown;
    }
}

using namespace lava::chamber;
using namespace lava::crater;

Window::Impl::Impl(VideoMode mode, const std::string& /* title */)
    : IWindowImpl(mode)
{
    DWORD style = WS_OVERLAPPEDWINDOW;
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
        logger.error("crater.window") << "Unable to create window handle: " << GetLastError() << "." << std::endl;
    }

    ShowWindow(m_hwnd, SW_SHOWDEFAULT);
    UpdateWindow(m_hwnd);
}

lava::WsHandle Window::Impl::handle() const
{
    WsHandle handle;
    handle.dwm.hwnd = m_hwnd;
    handle.dwm.hinstance = m_hinstance;
    return handle;
}

void Window::Impl::processEvents()
{
    MSG msg;
    while (PeekMessage(&msg, m_hwnd, 0, 0, PM_REMOVE) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

bool Window::Impl::processEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_DESTROY: {
        PostQuitMessage(0);
        return true;
    }

    case WM_CLOSE: {
        WsEvent event;
        event.type = WsEventType::WindowClosed;
        pushEvent(event);
        return true;
    }

    case WM_SETCURSOR: {
        // @todo We should probably take the one in params
        SetCursor(LoadCursor(nullptr, IDC_ARROW));
        return true;
    }

    case WM_SIZE: {
        WsEvent event;
        event.type = WsEventType::WindowResized;
        event.windowSize.width = GET_X_LPARAM(lParam);
        event.windowSize.height = GET_Y_LPARAM(lParam);
        pushEvent(event);
        break;
    }

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN: {
        WsEvent event;
        event.type = WsEventType::KeyPressed;
        event.key.which = virtualKeyToKey(wParam);
        pushEvent(event);
        break;
    }

    case WM_KEYUP:
    case WM_SYSKEYUP: {
        WsEvent event;
        event.type = WsEventType::KeyReleased;
        event.key.which = virtualKeyToKey(wParam);
        pushEvent(event);
        break;
    }

    case WM_LBUTTONDOWN: {
        WsEvent event;
        event.type = WsEventType::MouseButtonPressed;
        event.mouseButton.which = MouseButton::Left;
        event.mouseButton.x = GET_X_LPARAM(lParam);
        event.mouseButton.y = GET_Y_LPARAM(lParam);
        pushEvent(event);
        break;
    }

    case WM_RBUTTONDOWN: {
        WsEvent event;
        event.type = WsEventType::MouseButtonPressed;
        event.mouseButton.which = MouseButton::Right;
        event.mouseButton.x = GET_X_LPARAM(lParam);
        event.mouseButton.y = GET_Y_LPARAM(lParam);
        pushEvent(event);
        break;
    }

    case WM_LBUTTONUP: {
        WsEvent event;
        event.type = WsEventType::MouseButtonReleased;
        event.mouseButton.which = MouseButton::Left;
        event.mouseButton.x = GET_X_LPARAM(lParam);
        event.mouseButton.y = GET_Y_LPARAM(lParam);
        pushEvent(event);
        break;
    }

    case WM_RBUTTONUP: {
        WsEvent event;
        event.type = WsEventType::MouseButtonReleased;
        event.mouseButton.which = MouseButton::Right;
        event.mouseButton.x = GET_X_LPARAM(lParam);
        event.mouseButton.y = GET_Y_LPARAM(lParam);
        pushEvent(event);
        break;
    }

    case WM_MOUSEMOVE: {
        WsEvent event;
        event.type = WsEventType::MouseMoved;
        event.mouseMove.x = GET_X_LPARAM(lParam);
        event.mouseMove.y = GET_Y_LPARAM(lParam);
        pushEvent(event);
        break;
    }

    case WM_MOUSEWHEEL: {
        WsEvent event;
        event.type = WsEventType::MouseWheelScrolled;
        event.mouseWheel.which = MouseWheel::Vertical;
        event.mouseWheel.x = GET_X_LPARAM(lParam);
        event.mouseWheel.y = GET_Y_LPARAM(lParam);
        event.mouseWheel.delta = static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / static_cast<float>(WHEEL_DELTA);
        pushEvent(event);
        break;
    }

    default: {
        break;
    }
    }

    return false;
}
