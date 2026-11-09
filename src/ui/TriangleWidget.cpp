#include "TriangleWidget.h"

#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include "UiCommon.h"
#include "core/Geometry.h"
#include "core/InputParsing.h"
#include "core/Numeric.h"

TriangleWidget::TriangleWidget(QWidget* parent) : QWidget(parent) {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(24, 24, 24, 24);
    root->setSpacing(16);

    auto* title = new QLabel(QStringLiteral("三角形计算"));
    title->setObjectName(QStringLiteral("pageTitle"));
    root->addWidget(title);

    auto* description = new QLabel(
        QStringLiteral("输入空间中三角形的三个顶点坐标，程序会计算其面积与周长。"
                       "面积由叉积的模求得，三点共线或重合时会提示三角形退化。"));
    description->setWordWrap(true);
    description->setStyleSheet(QStringLiteral("color: #6b7280;"));
    root->addWidget(description);

    auto* box = new QGroupBox(QStringLiteral("顶点坐标"));
    auto* grid = new QGridLayout(box);
    grid->setSpacing(8);
    grid->addWidget(new QLabel(QStringLiteral("顶点")), 0, 0);
    grid->addWidget(new QLabel(QStringLiteral("x")), 0, 1);
    grid->addWidget(new QLabel(QStringLiteral("y")), 0, 2);
    grid->addWidget(new QLabel(QStringLiteral("z")), 0, 3);

    for (int vertex = 0; vertex < 3; ++vertex) {
        grid->addWidget(new QLabel(QStringLiteral("P%1").arg(vertex + 1)), vertex + 1, 0);
        for (int axis = 0; axis < 3; ++axis) {
            auto* edit = new QLineEdit;
            edit->setAlignment(Qt::AlignRight);
            edit->setPlaceholderText(QStringLiteral("0"));
            grid->addWidget(edit, vertex + 1, axis + 1);
            if (axis == 0) m_x[vertex] = edit;
            if (axis == 1) m_y[vertex] = edit;
            if (axis == 2) m_z[vertex] = edit;
        }
    }
    root->addWidget(box);

    auto* actionRow = new QHBoxLayout;
    actionRow->setSpacing(8);
    auto* calculateButton = new QPushButton(QStringLiteral("计算"));
    calculateButton->setObjectName(QStringLiteral("primaryButton"));
    actionRow->addWidget(calculateButton);
    actionRow->addStretch();
    root->addLayout(actionRow);

    m_message = ui::makePlaceholder();
    root->addWidget(m_message);

    m_results = new ui::ResultPanel;
    root->addWidget(m_results, 1);

    connect(calculateButton, &QPushButton::clicked, this, &TriangleWidget::calculate);
}

void TriangleWidget::calculate() {
    double values[9];
    QLineEdit* edits[9] = {m_x[0], m_y[0], m_z[0], m_x[1], m_y[1], m_z[1],
                            m_x[2], m_y[2], m_z[2]};
    for (int i = 0; i < 9; ++i) {
        const auto parsed = calc::parseDouble(edits[i]->text().toStdString());
        if (!parsed.ok) {
            m_message->setText(QStringLiteral("第 %1 个坐标无效：%2")
                                   .arg(i + 1)
                                   .arg(QString::fromStdString(parsed.error)));
            m_results->clear();
            return;
        }
        values[i] = parsed.value;
    }

    const calc::Vec3 a{values[0], values[1], values[2]};
    const calc::Vec3 b{values[3], values[4], values[5]};
    const calc::Vec3 c{values[6], values[7], values[8]};
    const calc::TriangleResult result = calc::analyzeTriangle(a, b, c);

    m_results->clear();
    if (result.degenerate) {
        m_results->addRow(QStringLiteral("退化三角形"), QStringLiteral("三点共线或重合，面积为 0"));
        m_results->addRow(QStringLiteral("周长"), QString::fromStdString(calc::formatReal(result.perimeter)));
        m_message->setText(QString::fromStdString(result.reason));
        return;
    }

    m_results->addRow(QStringLiteral("面积"), QString::fromStdString(calc::formatReal(result.area)));
    m_results->addRow(QStringLiteral("周长"), QString::fromStdString(calc::formatReal(result.perimeter)));
    m_results->addRow(QStringLiteral("边长 |P2P3|"), QString::fromStdString(calc::formatReal(result.sideA)));
    m_results->addRow(QStringLiteral("边长 |P1P3|"), QString::fromStdString(calc::formatReal(result.sideB)));
    m_results->addRow(QStringLiteral("边长 |P1P2|"), QString::fromStdString(calc::formatReal(result.sideC)));
    m_results->addRow(QStringLiteral("最大高"), QString::fromStdString(calc::formatReal(result.altitude)));
    m_message->setText(QStringLiteral("计算完成。"));
}
