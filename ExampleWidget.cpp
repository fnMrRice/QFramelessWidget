#include "ExampleWidget.h"
#include "ui_ExampleWidget.h"
#include <QMessageBox>

ExampleWidget::ExampleWidget(QWidget *parent) : QFramelessWidget(parent), ui(new Ui::ExamplwWidget) {
    ui->setupUi(this);

    this->setTitleBarWidget(ui->title_bar);
    this->setMinimalButton(ui->winbutton_min);
    this->setToggleButton(ui->winbutton_toggle);
    this->setCloseButton(ui->winbutton_close);
}

ExampleWidget::~ExampleWidget() {
    delete ui;
}

bool ExampleWidget::confirmClose() {
    auto result = QMessageBox::question(this, tr("Confirm"), tr("Are you sure to quit this application?"));
    if (result == QMessageBox::StandardButton::Yes) { return true; }
    else { return false; }
}

bool ExampleWidget::shouldPerformBorderDrag(const QPoint &pos) const {
    auto widget = QApplication::widgetAt(QCursor::pos());
    if (dynamic_cast<QPushButton*>(widget)) {
        return false;
    }
    return QFramelessWidget::shouldPerformWindowDrag(pos);
}

bool ExampleWidget::shouldPerformWindowDrag(const QPoint &pos) const {
    auto widget = QApplication::widgetAt(QCursor::pos());
    if (dynamic_cast<QPushButton*>(widget)) {
        return false;
    }
    return QFramelessWidget::shouldPerformWindowDrag(pos);
}
