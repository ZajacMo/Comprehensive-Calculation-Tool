// 方程求解页面。
#pragma once

#include <QWidget>

class QLineEdit;
class QLabel;
class QComboBox;

namespace ui {
class ResultPanel;
}

// 一次方程、二次方程（标准式与顶点式）、多项式数值求根。
class EquationWidget : public QWidget {
    Q_OBJECT

public:
    explicit EquationWidget(QWidget* parent = nullptr);

private:
    void solveLinear();
    void solveQuadratic();
    void solveVertex();
    void solvePolynomial();

    // 读取并校验三个系数输入。
    bool readCoefficients(QLineEdit* a, QLineEdit* b, QLineEdit* c,
                          double& va, double& vb, double& vc, QString& error);

    QLineEdit* m_linearA = nullptr;
    QLineEdit* m_linearB = nullptr;

    QLineEdit* m_quadA = nullptr;
    QLineEdit* m_quadB = nullptr;
    QLineEdit* m_quadC = nullptr;

    QLineEdit* m_vertexA = nullptr;
    QLineEdit* m_vertexH = nullptr;
    QLineEdit* m_vertexK = nullptr;

    QLineEdit* m_polyInput = nullptr;

    QLabel* m_message = nullptr;
    ui::ResultPanel* m_results = nullptr;
};
