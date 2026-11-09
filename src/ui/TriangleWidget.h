// 三角形计算页面。
#pragma once

#include <QWidget>

class QLineEdit;
class QLabel;

namespace ui {
class ResultPanel;
}

// 三维三角形：面积、周长、边长与高。
class TriangleWidget : public QWidget {
    Q_OBJECT

public:
    explicit TriangleWidget(QWidget* parent = nullptr);

private:
    void calculate();

    QLineEdit* m_x[3] = {nullptr};
    QLineEdit* m_y[3] = {nullptr};
    QLineEdit* m_z[3] = {nullptr};
    QLabel* m_message = nullptr;
    ui::ResultPanel* m_results = nullptr;
};
