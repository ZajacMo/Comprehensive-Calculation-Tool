// 统计与回归页面。
#pragma once

#include <QWidget>

#include <vector>

#include "core/Statistics.h"

class QTableWidget;
class QLineEdit;
class QLabel;

namespace ui {
class ResultPanel;
}

// 描述统计与最小二乘线性回归。
class StatisticsWidget : public QWidget {
    Q_OBJECT

public:
    explicit StatisticsWidget(QWidget* parent = nullptr);

private:
    void addPointRow();
    void removePointRow();
    void calculate();
    void predict();

    // 读取表格中的数据点，跳过空行。
    bool collectPoints(std::vector<calc::Point>& points, QString& error);

    QTableWidget* m_table = nullptr;
    QLineEdit* m_predictX = nullptr;
    QLabel* m_message = nullptr;
    ui::ResultPanel* m_results = nullptr;
};
