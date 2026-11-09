// 空间几何页面：四面体与常见几何体。
#pragma once

#include <QWidget>

class QLineEdit;
class QLabel;
class QComboBox;

namespace ui {
class ResultPanel;
}

// 四面体分析、球/圆柱/圆锥/棱柱/棱锥的表面积与体积。
class GeometryWidget : public QWidget {
    Q_OBJECT

public:
    explicit GeometryWidget(QWidget* parent = nullptr);

private:
    void analyzeTetrahedron();
    void analyzeSolid();

    QLineEdit* m_tetra[12] = {nullptr};
    QComboBox* m_solidType = nullptr;
    QLineEdit* m_solidA = nullptr;
    QLineEdit* m_solidB = nullptr;
    QLineEdit* m_solidC = nullptr;
    QLabel* m_message = nullptr;
    ui::ResultPanel* m_results = nullptr;
};
