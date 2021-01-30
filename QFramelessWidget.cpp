#include "QFramelessWidget.h"
#ifdef Q_OS_WINDOWS
#include <Windows.h>
#include <WinUser.h>
#include <windowsx.h>
#endif
#include <QEvent>
#include <QPushButton>
#include <QApplication>
#include <stdexcept>

// some code from the following pages
// https://github.com/Ochrazy/Qt-FramelessNativeWindow
// https://github.com/dfct/TrueFramelessWindow

template <class _Half, class _Full>
static inline constexpr _Half GetLowHalf(const _Full &value) {
    static_assert (std::is_arithmetic_v<_Half> &&std::is_arithmetic_v<_Full>);
    auto constexpr and_val = ~0;
    return static_cast<_Half>(value & and_val);
}

template <class _Half, class _Full>
static inline constexpr _Half GetHighHalf(const _Full &value) {
    static_assert (std::is_arithmetic_v<_Half> &&std::is_arithmetic_v<_Full>);
    auto constexpr bit_half = sizeof(_Full) * 4;
    auto constexpr and_val = ~0;
    return static_cast<_Half>((value >> bit_half) & and_val);
}

QFramelessWidget::QFramelessWidget(QWidget *parent): QWidget(parent) {
    this->setWindowFlags(Qt::FramelessWindowHint |
                         Qt::WindowMinimizeButtonHint |
                         Qt::WindowMaximizeButtonHint |
                         Qt::Window);
    this->setAttribute(Qt::WidgetAttribute::WA_QuitOnClose, false);
}

void QFramelessWidget::setMinimalButton(QPushButton *button) {
    if (m_minimal_button) { m_minimal_button->disconnect(this); }
    connect(button, &QPushButton::clicked, this, &QFramelessWidget::showMinimized);
    m_minimal_button = button;
}

void QFramelessWidget::setToggleButton(QPushButton *button) {
    if (m_toggle_button) { m_toggle_button->disconnect(this); }
    if (!button->isCheckable()) { throw std::runtime_error("cannot use button not checkable for toggle window maxmize/restore"); }
    connect(button, &QPushButton::clicked, this, &QFramelessWidget::toggleWindow, Qt::ConnectionType::DirectConnection);
    m_toggle_button = button;
}

void QFramelessWidget::setCloseButton(QPushButton *button) {
    if (m_close_button) { m_close_button->disconnect(this); }
    connect(button, &QPushButton::clicked, this, &QFramelessWidget::close);
    m_close_button = button;
}

void QFramelessWidget::changeEvent(QEvent *event) {
    if (event->type() == QEvent::WindowStateChange) {
        if (m_toggle_button) {
            if (this->windowState() & Qt::WindowState::WindowMaximized) { m_toggle_button->setChecked(true); }
            else if (this->windowState() == Qt::WindowState::WindowNoState) { m_toggle_button->setChecked(false); }
        }
    }
}
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
bool QFramelessWidget::nativeEvent([[maybe_unused]] const QByteArray &eventType,
                                   void *msg, long *result) {
#else
bool QFramelessWidget::nativeEvent([[maybe_unused]] const QByteArray &nativeType,
                                   void *msg, qintptr *result) {
#endif
#if defined(Q_OS_WINDOWS)
    auto message = static_cast<MSG*>(msg);
    switch (message->message) {
    case WM_CLOSE: {
        if (confirmClose()) { QApplication::quit(); }
        else { *result = 0; }
        return true;
    }
    case WM_NCCALCSIZE: {
        //this kills the window frame and title bar we added with
        //WS_THICKFRAME and WS_CAPTION
        *result = 0;
        return true;
    }
    case WM_SHOWWINDOW: {
        if (message->wParam) {
            auto handle = reinterpret_cast<HWND>(this->winId());
            SetWindowLongPtr(handle, GWL_STYLE, WS_THICKFRAME | WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
        }
        break;
    }
    case WM_NCHITTEST: {
        auto xPos = GET_X_LPARAM(message->lParam);
        auto yPos = GET_Y_LPARAM(message->lParam);

#define DO_SOMETHING(TYPE) *result=TYPE; return true
        switch (this->GetCursorPositionType(xPos, yPos)) {
        case CURSOR_POSITION_TYPE::POS_TOP_LEFT: DO_SOMETHING(HTTOPLEFT);
        case CURSOR_POSITION_TYPE::POS_TOP_RIGHT: DO_SOMETHING(HTTOPRIGHT);
        case CURSOR_POSITION_TYPE::POS_BOTTOM_LEFT: DO_SOMETHING(HTBOTTOMLEFT);
        case CURSOR_POSITION_TYPE::POS_BOTTOM_RIGHT: DO_SOMETHING(HTBOTTOMRIGHT);
        case CURSOR_POSITION_TYPE::POS_TOP: DO_SOMETHING(HTTOP);
        case CURSOR_POSITION_TYPE::POS_BOTTOM: DO_SOMETHING(HTBOTTOM);
        case CURSOR_POSITION_TYPE::POS_LEFT: DO_SOMETHING(HTLEFT);
        case CURSOR_POSITION_TYPE::POS_RIGHT: DO_SOMETHING(HTRIGHT);
        case CURSOR_POSITION_TYPE::POS_TITLE: DO_SOMETHING(HTCAPTION);
        default: *result = 0; break;
        }
#undef DO_SOMETHING

        break;
    }
    case WM_GETMINMAXINFO: { // not considered multi-screen situation
        auto handle = reinterpret_cast<HWND>(this->winId());
        MINMAXINFO* minMaxInfo = reinterpret_cast<MINMAXINFO*>(message->lParam);

        // Get Monitor Info
        auto monitor = ::MonitorFromWindow(handle, MONITOR_DEFAULTTONEAREST);
        MONITORINFO monitor_info{};
        monitor_info.cbSize = sizeof(monitor_info);
        GetMonitorInfoW(monitor, &monitor_info);

        // Set position and size of maximized window
        minMaxInfo->ptMaxPosition.x = monitor_info.rcWork.left;
        minMaxInfo->ptMaxPosition.y = monitor_info.rcWork.top;

        minMaxInfo->ptMaxSize.x = monitor_info.rcWork.right - monitor_info.rcWork.left;
        minMaxInfo->ptMaxSize.y = monitor_info.rcWork.bottom - monitor_info.rcWork.top;

        // Set limits of the size of the window
        minMaxInfo->ptMinTrackSize.x = this->minimumWidth();
        minMaxInfo->ptMinTrackSize.y = this->minimumHeight();
        minMaxInfo->ptMaxTrackSize.x = this->maximumWidth();
        minMaxInfo->ptMaxTrackSize.y =  this->maximumHeight();

        *result = 0;
        return true;
    }
    }
#elif defined(Q_OS_LINUX)

#elif defined(Q_OS_OSX)

#endif
    return false;
}

QFramelessWidget::CURSOR_POSITION_TYPE QFramelessWidget::GetCursorPositionType(const int &x, const int &y) const {
    auto const &xPos = x;
    auto const &yPos = y;
    auto WinLeft = this->geometry().left();
    auto WinRight = this->geometry().right();
    auto WinTop = this->geometry().top();
    auto WinBottom = this->geometry().bottom();
    size_t Left = abs(xPos - WinLeft);
    size_t Right = abs(xPos - WinRight);
    size_t Top = abs(yPos - WinTop);
    size_t Bottom = abs(yPos - WinBottom);
    auto const &area = m_enabled_drag_borders;
    auto const &size = m_border_width;

    if (!(this->windowState()&Qt::WindowState::WindowMaximized)) {
        if (Top <= size && Left <= size) {
            if (shouldPerformBorderDrag({(int)Left, (int)Top}) && ((area & CORNOR_TOP_LEFT) || ((area & BORDER_TOP) && (area & BORDER_LEFT)))) { return CURSOR_POSITION_TYPE::POS_TOP_LEFT; }
        } else if (Top <= size && Right <= size) {
            if (shouldPerformBorderDrag({(int)Left, (int)Top}) && ((area & CORNOR_TOP_RIGHT) || ((area & BORDER_TOP) && (area & BORDER_RIGHT)))) { return CURSOR_POSITION_TYPE::POS_TOP_RIGHT; }
        } else if (Bottom <= size && Left <= size) {
            if (shouldPerformBorderDrag({(int)Left, (int)Top}) && ((area & CORNOR_BOTTOM_LEFT) || ((area & BORDER_BOTTOM) && (area & BORDER_LEFT)))) { return CURSOR_POSITION_TYPE::POS_BOTTOM_LEFT; }
        } else if (Bottom <= size && Right <= size) {
            if (shouldPerformBorderDrag({(int)Left, (int)Top}) && ((area & CORNOR_BOTTOM_RIGHT) || ((area & BORDER_BOTTOM) && (area & BORDER_RIGHT)))) { return CURSOR_POSITION_TYPE::POS_BOTTOM_RIGHT; }
        } else if (Top <= size) {
            if (shouldPerformBorderDrag({(int)Left, (int)Top}) && (area & BORDER_TOP)) { return CURSOR_POSITION_TYPE::POS_TOP; }
        } else if (Bottom <= size) {
            if (shouldPerformBorderDrag({(int)Left, (int)Top}) && (area & BORDER_BOTTOM)) { return CURSOR_POSITION_TYPE::POS_BOTTOM; }
        } else if (Left <= size) {
            if (shouldPerformBorderDrag({(int)Left, (int)Top}) && (area & BORDER_LEFT)) { return CURSOR_POSITION_TYPE::POS_LEFT; }
        } else if (Right <= size) {
            if (shouldPerformBorderDrag({(int)Left, (int)Top}) && (area & BORDER_RIGHT)) { return CURSOR_POSITION_TYPE::POS_RIGHT; }
        }
    }
    if (m_header_area && this->shouldPerformWindowDrag({(int)Left, (int)Top})) {
        auto const &g = m_header_area->geometry();
        auto const l = (size_t)g.left(), r = (size_t)g.right(), t = (size_t)g.top(), b = (size_t)g.bottom();
        auto const x = Left, y = Top;
        if (l <= x && x <= r && t <= y && y <= b) { return CURSOR_POSITION_TYPE::POS_TITLE; }
    }
    return CURSOR_POSITION_TYPE::POS_NONE;
}

void QFramelessWidget::toggleWindow(bool checked) {
    if (checked) { this->showMaximized(); }
    else { this->showNormal(); }
}

void QFramelessWidget::close() {
    if (confirmClose()) { QWidget::close(); }
}

