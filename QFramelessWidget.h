#ifndef QFRAMELESSWIDGET_H
#define QFRAMELESSWIDGET_H

#include <QWidget>
class QPushButton;

enum BORDER_AREA {
    BORDER_LEFT = 0b0000'0001,
    BORDER_RIGHT = 0b0000'0010,
    BORDER_TOP = 0b0000'0100,
    BORDER_BOTTOM = 0b0000'1000,
    CORNOR_TOP_LEFT = 0b0001'0000,
    CORNOR_TOP_RIGHT = 0b0010'0000,
    CORNOR_BOTTOM_LEFT = 0b0100'0000,
    CORNOR_BOTTOM_RIGHT = 0b1000'0000,
};

class QFramelessWidget : public QWidget {
    Q_OBJECT
private:
    enum class CURSOR_POSITION_TYPE {
        POS_LEFT, POS_TOP, POS_RIGHT, POS_BOTTOM,
        POS_TOP_LEFT, POS_TOP_RIGHT, POS_BOTTOM_LEFT, POS_BOTTOM_RIGHT,
        POS_TITLE, POS_NONE
    };

public:
    QFramelessWidget(QWidget *parent = nullptr);
    virtual ~QFramelessWidget() = default;

    void setBorderTriggerWidth(const size_t &value) { m_border_width = value; }
    size_t borderTriggerWidth() const { return m_border_width; }
    void setMinimalButton(QPushButton *button);
    QPushButton* minimalButton() const { return m_minimal_button; }
    void setToggleButton(QPushButton *button);
    QPushButton* toggleButton() const { return m_toggle_button; }
    void setCloseButton(QPushButton *button);
    QPushButton* closeButton() const { return m_close_button; }
    void setTitleBarWidget(QWidget *widget) { m_header_area = widget; }
    QWidget* titleBarWidget() const { return m_header_area; }
    void setResizeBorder(const int &options) { m_enabled_drag_borders = options; }
    int resizeBorder() const { return m_enabled_drag_borders; }

protected:
    virtual bool confirmClose() { return true; }
    virtual bool shouldPerformBorderDrag(const QPoint&) const { return true; }
    virtual bool shouldPerformWindowDrag(const QPoint&) const { return true; }

protected:
    void changeEvent(QEvent *event) override;
    // only supports windows now, not support qt4 or earlier
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    bool nativeEvent(const QByteArray &eventType, void *msg, long *result) override;
#else
    bool nativeEvent(const QByteArray &eventType, void *msg, qintptr *result) override;
#endif

private:
    CURSOR_POSITION_TYPE GetCursorPositionType(const int &x, const int &y) const;

private slots:
    void toggleWindow(bool checked);
    void close();

private:
    QPushButton *m_minimal_button = nullptr, *m_toggle_button = nullptr, *m_close_button = nullptr;
    QWidget *m_header_area = nullptr;
    size_t m_border_width = 6;
    int m_enabled_drag_borders = 0b1111'1111;
};
#endif // QFRAMELESSWIDGET_H
