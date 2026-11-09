#include "GeometryWidget.h"

#include <QComboBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

#include "UiCommon.h"
#include "core/Geometry.h"
#include "core/InputParsing.h"
#include "core/Numeric.h"

GeometryWidget::GeometryWidget(QWidget* parent) : QWidget(parent) {
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);

    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    outer->addWidget(scroll);

    auto* content = new QWidget;
    auto* root = new QVBoxLayout(content);
    root->setContentsMargins(24, 24, 24, 24);
    root->setSpacing(16);
    scroll->setWidget(content);

    auto* title = new QLabel(QStringLiteral("空间几何"));
    title->setObjectName(QStringLiteral("pageTitle"));
    root->addWidget(title);

    auto* description = new QLabel(
        QStringLiteral("计算四面体的体积、表面积、重心与内外接球，"
                       "以及球、圆柱、圆锥、正棱柱、正棱锥的表面积与体积。"));
    description->setWordWrap(true);
    description->setStyleSheet(QStringLiteral("color: #6b7280;"));
    root->addWidget(description);

    // ---- 四面体 ----
    auto* tetraBox = new QGroupBox(QStringLiteral("四面体（输入四个顶点的坐标）"));
    auto* tetraLayout = new QVBoxLayout(tetraBox);

    auto* grid = new QGridLayout;
    grid->setSpacing(8);
    grid->addWidget(new QLabel(QStringLiteral("顶点")), 0, 0);
    grid->addWidget(new QLabel(QStringLiteral("x")), 0, 1);
    grid->addWidget(new QLabel(QStringLiteral("y")), 0, 2);
    grid->addWidget(new QLabel(QStringLiteral("z")), 0, 3);

    for (int vertex = 0; vertex < 4; ++vertex) {
        grid->addWidget(new QLabel(QStringLiteral("P%1").arg(vertex + 1)), vertex + 1, 0);
        for (int axis = 0; axis < 3; ++axis) {
            auto* edit = new QLineEdit;
            edit->setAlignment(Qt::AlignRight);
            edit->setPlaceholderText(QStringLiteral("0"));
            grid->addWidget(edit, vertex + 1, axis + 1);
            m_tetra[vertex * 3 + axis] = edit;
        }
    }
    tetraLayout->addLayout(grid);

    auto* tetraButton = new QPushButton(QStringLiteral("计算四面体"));
    tetraButton->setObjectName(QStringLiteral("primaryButton"));
    tetraLayout->addWidget(tetraButton);
    root->addWidget(tetraBox);

    // ---- 常见几何体 ----
    auto* solidBox = new QGroupBox(QStringLiteral("常见几何体"));
    auto* solidForm = new QFormLayout(solidBox);
    solidForm->setSpacing(10);

    m_solidType = new QComboBox;
    m_solidType->addItems({QStringLiteral("球"), QStringLiteral("圆柱"), QStringLiteral("圆锥"),
                           QStringLiteral("正棱柱"), QStringLiteral("正棱锥")});
    solidForm->addRow(QStringLiteral("类型："), m_solidType);

    m_solidA = new QLineEdit;
    m_solidA->setPlaceholderText(QStringLiteral("半径"));
    m_solidA->setMaximumWidth(180);
    solidForm->addRow(QStringLiteral("半径 r ="), m_solidA);

    m_solidB = new QLineEdit;
    m_solidB->setPlaceholderText(QStringLiteral("高"));
    m_solidB->setMaximumWidth(180);
    solidForm->addRow(QStringLiteral("高 h ="), m_solidB);

    m_solidC = new QLineEdit;
    m_solidC->setPlaceholderText(QStringLiteral("边数，如 6"));
    m_solidC->setMaximumWidth(180);
    solidForm->addRow(QStringLiteral("底面边数 n ="), m_solidC);

    auto* solidButton = new QPushButton(QStringLiteral("计算"));
    solidButton->setObjectName(QStringLiteral("primaryButton"));
    solidForm->addRow(solidButton);
    root->addWidget(solidBox);

    m_message = ui::makePlaceholder();
    root->addWidget(m_message);

    m_results = new ui::ResultPanel;
    root->addWidget(m_results);

    connect(tetraButton, &QPushButton::clicked, this, &GeometryWidget::analyzeTetrahedron);
    connect(solidButton, &QPushButton::clicked, this, &GeometryWidget::analyzeSolid);

    // 随类型切换需要填写的参数。
    connect(m_solidType, &QComboBox::currentIndexChanged, this, [this](int index) {
        m_solidB->setVisible(index != 0);
        m_solidC->setVisible(index >= 3);
        if (index == 3) {
            m_solidA->setPlaceholderText(QStringLiteral("底面外接圆半径"));
        } else if (index == 4) {
            m_solidA->setPlaceholderText(QStringLiteral("底面外接圆半径"));
        } else {
            m_solidA->setPlaceholderText(QStringLiteral("半径"));
        }
    });
}

void GeometryWidget::analyzeTetrahedron() {
    double values[12];
    for (int i = 0; i < 12; ++i) {
        const auto parsed = calc::parseDouble(m_tetra[i]->text().toStdString());
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
    const calc::Vec3 d{values[9], values[10], values[11]};
    const calc::TetrahedronResult result = calc::analyzeTetrahedron(a, b, c, d);

    m_results->clear();
    if (result.degenerate) {
        m_message->setText(QString::fromStdString(result.reason));
        return;
    }
    m_results->addRow(QStringLiteral("体积"), QString::fromStdString(calc::formatReal(result.volume)));
    m_results->addRow(QStringLiteral("表面积"), QString::fromStdString(calc::formatReal(result.surfaceArea)));
    m_results->addRow(QStringLiteral("重心"),
                      QStringLiteral("(%1, %2, %3)")
                          .arg(QString::fromStdString(calc::formatReal(result.centroid.x)),
                               QString::fromStdString(calc::formatReal(result.centroid.y)),
                               QString::fromStdString(calc::formatReal(result.centroid.z))));
    m_results->addRow(QStringLiteral("外接球球心"),
                      QStringLiteral("(%1, %2, %3)")
                          .arg(QString::fromStdString(calc::formatReal(result.circumcenter.x)),
                               QString::fromStdString(calc::formatReal(result.circumcenter.y)),
                               QString::fromStdString(calc::formatReal(result.circumcenter.z))));
    m_results->addRow(QStringLiteral("外接球半径"),
                      QString::fromStdString(calc::formatReal(result.circumradius)));
    m_results->addRow(QStringLiteral("内切球半径"),
                      QString::fromStdString(calc::formatReal(result.inradius)));
    m_message->setText(QStringLiteral("计算完成。"));
}

void GeometryWidget::analyzeSolid() {
    const auto parseA = calc::parseDouble(m_solidA->text().toStdString());
    if (!parseA.ok) {
        m_message->setText(QStringLiteral("半径无效：%1").arg(QString::fromStdString(parseA.error)));
        m_results->clear();
        return;
    }

    calc::SolidResult solid;
    const int type = m_solidType->currentIndex();
    switch (type) {
        case 0: {
            solid = calc::analyzeSphere(parseA.value);
            break;
        }
        case 1:
        case 2: {
            const auto parseB = calc::parseDouble(m_solidB->text().toStdString());
            if (!parseB.ok) {
                m_message->setText(QStringLiteral("高无效：%1").arg(QString::fromStdString(parseB.error)));
                m_results->clear();
                return;
            }
            solid = (type == 1) ? calc::analyzeCylinder(parseA.value, parseB.value)
                                : calc::analyzeCone(parseA.value, parseB.value);
            break;
        }
        default: {
            const auto parseB = calc::parseDouble(m_solidB->text().toStdString());
            const auto parseC = calc::parseDouble(m_solidC->text().toStdString());
            if (!parseB.ok) {
                m_message->setText(QStringLiteral("高无效：%1").arg(QString::fromStdString(parseB.error)));
                m_results->clear();
                return;
            }
            if (!parseC.ok) {
                m_message->setText(QStringLiteral("边数无效：%1").arg(QString::fromStdString(parseC.error)));
                m_results->clear();
                return;
            }
            const int sides = static_cast<int>(parseC.value);
            solid = (type == 3) ? calc::analyzePrism(parseA.value, parseB.value, sides)
                                : calc::analyzePyramid(parseA.value, parseB.value, sides);
            break;
        }
    }

    m_results->clear();
    if (!solid.valid) {
        m_message->setText(QString::fromStdString(solid.reason));
        return;
    }
    m_results->addRow(QStringLiteral("表面积"), QString::fromStdString(calc::formatReal(solid.surfaceArea)));
    m_results->addRow(QStringLiteral("体积"), QString::fromStdString(calc::formatReal(solid.volume)));
    m_message->setText(QStringLiteral("计算完成。"));
}
