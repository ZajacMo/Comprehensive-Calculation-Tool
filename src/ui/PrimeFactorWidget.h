// 质因数分解器页面。
#pragma once

#include <QWidget>

class QLineEdit;
class QLabel;

namespace ui {
class ResultPanel;
}

// 数论工具：质因数分解、素性判定、欧拉函数、莫比乌斯函数、约数统计。
class PrimeFactorWidget : public QWidget {
    Q_OBJECT

public:
    explicit PrimeFactorWidget(QWidget* parent = nullptr);

private:
    void calculate();

    QLineEdit* m_input = nullptr;
    QLabel* m_message = nullptr;
    ui::ResultPanel* m_results = nullptr;
};
