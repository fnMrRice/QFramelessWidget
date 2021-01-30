#ifndef EXAMPLEWIDGET_H
#define EXAMPLEWIDGET_H

#include <QWidget>
#include "QFramelessWidget.h"

namespace Ui {
class ExamplwWidget;
}

class ExampleWidget : public QFramelessWidget {
    Q_OBJECT

public:
    explicit ExampleWidget(QWidget *parent = nullptr);
    ~ExampleWidget();

protected:
    bool confirmClose() override;
    bool shouldPerformBorderDrag(const QPoint&) const override;
    bool shouldPerformWindowDrag(const QPoint &pos) const override;

private:
    Ui::ExamplwWidget *ui;
};

#endif // EXAMPLEWIDGET_H
