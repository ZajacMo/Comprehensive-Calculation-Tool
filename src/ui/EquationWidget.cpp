#include "EquationWidget.h"

#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

#include "UiCommon.h"
#include "core/Algebra.h"
#include "core/InputParsing.h"
#include "core/Numeric.h"

namespace {

QLineEdit* makeCoefficientInput(const QString& placeholder = QStringLiteral("0")) {
    auto* edit = new QLineEdit;
    edit->setPlaceholderText(placeholder);
    edit->setMaximumWidth(180);
    edit->setAlignment(Qt::AlignRight);
    return edit;
}

// 一次性创建 a、b、c 三个系数输入框。
void buildCoefficientRow(QFormLayout* form, QLineEdit*& a, QLineEdit*& b, QLineEdit*& c) {
    a = makeCoefficientInput();
    b = makeCoefficientInput();
    c = makeCoefficientInput();

    auto* row = new QHBoxLayout;
    row->setSpacing(6);
    row->addWidget(new QLabel(QStringLiteral("a =")));
    row->addWidget(a);
    row->addWidget(new QLabel(QStringLiteral("b =")));
    row->addWidget(b);
    row->addWidget(new QLabel(QStringLiteral("c =")));
    row->addWidget(c);
    row->addStretch();
    form->addRow(row);
}

}  // namespace

EquationWidget::EquationWidget(QWidget* parent) : QWidget(parent) {
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

    auto* title = new QLabel(QStringLiteral("方程求解"));
    title->setObjectName(QStringLiteral("pageTitle"));
    root->addWidget(title);

    auto* description = new QLabel(
        QStringLiteral("求解一次方程、二次方程与多项式方程。二次方程支持标准式 ax² + bx + c = 0 "
                       "与顶点式 a(x − h)² + k = 0 两种输入；判别式为负时给出共轭复根。"));
    description->setWordWrap(true);
    description->setStyleSheet(QStringLiteral("color: #6b7280;"));
    root->addWidget(description);

    // ---- 一次方程 ----
    auto* linearBox = new QGroupBox(QStringLiteral("一次方程 ax + b = 0"));
    auto* linearForm = new QFormLayout(linearBox);
    linearForm->setSpacing(10);
    m_linearA = makeCoefficientInput();
    m_linearB = makeCoefficientInput();
    auto* linearRow = new QHBoxLayout;
    linearRow->setSpacing(6);
    linearRow->addWidget(new QLabel(QStringLiteral("a =")));
    linearRow->addWidget(m_linearA);
    linearRow->addWidget(new QLabel(QStringLiteral("b =")));
    linearRow->addWidget(m_linearB);
    linearRow->addStretch();
    linearForm->addRow(linearRow);
    auto* linearButton = new QPushButton(QStringLiteral("求解"));
    linearButton->setObjectName(QStringLiteral("primaryButton"));
    linearForm->addRow(linearButton);
    root->addWidget(linearBox);

    // ---- 二次方程（标准式） ----
    auto* quadBox = new QGroupBox(QStringLiteral("二次方程 ax² + bx + c = 0"));
    auto* quadForm = new QFormLayout(quadBox);
    quadForm->setSpacing(10);
    buildCoefficientRow(quadForm, m_quadA, m_quadB, m_quadC);
    auto* quadButton = new QPushButton(QStringLiteral("求解"));
    quadButton->setObjectName(QStringLiteral("primaryButton"));
    quadForm->addRow(quadButton);
    root->addWidget(quadBox);

    // ---- 二次方程（顶点式） ----
    auto* vertexBox = new QGroupBox(QStringLiteral("二次方程 a(x − h)² + k = 0"));
    auto* vertexForm = new QFormLayout(vertexBox);
    vertexForm->setSpacing(10);
    m_vertexA = makeCoefficientInput();
    m_vertexH = makeCoefficientInput();
    m_vertexK = makeCoefficientInput();
    auto* vertexRow = new QHBoxLayout;
    vertexRow->setSpacing(6);
    vertexRow->addWidget(new QLabel(QStringLiteral("a =")));
    vertexRow->addWidget(m_vertexA);
    vertexRow->addWidget(new QLabel(QStringLiteral("h =")));
    vertexRow->addWidget(m_vertexH);
    vertexRow->addWidget(new QLabel(QStringLiteral("k =")));
    vertexRow->addWidget(m_vertexK);
    vertexRow->addStretch();
    vertexForm->addRow(vertexRow);
    auto* vertexButton = new QPushButton(QStringLiteral("求解"));
    vertexButton->setObjectName(QStringLiteral("primaryButton"));
    vertexForm->addRow(vertexButton);
    root->addWidget(vertexBox);

    // ---- 多项式 ----
    auto* polyBox = new QGroupBox(QStringLiteral("多项式求根（系数按降幂排列）"));
    auto* polyForm = new QFormLayout(polyBox);
    polyForm->setSpacing(10);
    m_polyInput = new QLineEdit;
    m_polyInput->setPlaceholderText(QStringLiteral("例如 1 -6 11 -6  表示 x³ − 6x² + 11x − 6"));
    polyForm->addRow(QStringLiteral("系数（空格分隔）："), m_polyInput);
    auto* polyButton = new QPushButton(QStringLiteral("求根"));
    polyButton->setObjectName(QStringLiteral("primaryButton"));
    polyForm->addRow(polyButton);
    root->addWidget(polyBox);

    m_message = ui::makePlaceholder();
    root->addWidget(m_message);

    m_results = new ui::ResultPanel;
    root->addWidget(m_results);

    connect(linearButton, &QPushButton::clicked, this, &EquationWidget::solveLinear);
    connect(quadButton, &QPushButton::clicked, this, &EquationWidget::solveQuadratic);
    connect(vertexButton, &QPushButton::clicked, this, &EquationWidget::solveVertex);
    connect(polyButton, &QPushButton::clicked, this, &EquationWidget::solvePolynomial);
    for (auto* edit : {m_linearA, m_linearB, m_quadA, m_quadB, m_quadC,
                       m_vertexA, m_vertexH, m_vertexK, m_polyInput}) {
        connect(edit, &QLineEdit::returnPressed, this, [this] {
            // 回车时默认走二次方程求解（最常用）。
            solveQuadratic();
        });
    }
}

bool EquationWidget::readCoefficients(QLineEdit* a, QLineEdit* b, QLineEdit* c,
                                       double& va, double& vb, double& vc, QString& error) {
    const auto parseA = calc::parseDouble(a->text().toStdString());
    const auto parseB = calc::parseDouble(b->text().toStdString());
    const auto parseC = calc::parseDouble(c->text().toStdString());
    if (!parseA.ok) { error = QStringLiteral("a 无效：%1").arg(QString::fromStdString(parseA.error)); return false; }
    if (!parseB.ok) { error = QStringLiteral("b 无效：%1").arg(QString::fromStdString(parseB.error)); return false; }
    if (!parseC.ok) { error = QStringLiteral("c 无效：%1").arg(QString::fromStdString(parseC.error)); return false; }
    va = parseA.value;
    vb = parseB.value;
    vc = parseC.value;
    return true;
}

void EquationWidget::solveLinear() {
    const auto parseA = calc::parseDouble(m_linearA->text().toStdString());
    const auto parseB = calc::parseDouble(m_linearB->text().toStdString());
    if (!parseA.ok || !parseB.ok) {
        m_message->setText(QStringLiteral("请输入合法的 a 与 b。"));
        m_results->clear();
        return;
    }
    const calc::EquationSolution solution = calc::solveLinear(parseA.value, parseB.value);
    m_results->clear();
    if (!solution.valid) {
        m_message->setText(QString::fromStdString(solution.reason));
        return;
    }
    m_results->addRow(QStringLiteral("解"), QString::fromStdString(solution.describe()));
    m_message->setText(QStringLiteral("求解完成。"));
}

void EquationWidget::solveQuadratic() {
    double a = 0.0, b = 0.0, c = 0.0;
    QString error;
    if (!readCoefficients(m_quadA, m_quadB, m_quadC, a, b, c, error)) {
        m_message->setText(error);
        m_results->clear();
        return;
    }
    const calc::EquationSolution solution = calc::solveQuadratic(a, b, c);
    m_results->clear();
    if (!solution.valid) {
        m_message->setText(QString::fromStdString(solution.reason));
        return;
    }
    m_results->addRow(QStringLiteral("判别式 Δ"), QString::fromStdString(calc::formatReal(solution.discriminant)));
    m_results->addRow(QStringLiteral("解"), QString::fromStdString(solution.describe()));
    if (!solution.hasRealRoots) {
        m_results->addRow(QStringLiteral("说明"), QStringLiteral("该方程无实根，上面给出的是共轭复根"));
    }
    m_message->setText(QStringLiteral("求解完成。"));
}

void EquationWidget::solveVertex() {
    double a = 0.0, h = 0.0, k = 0.0;
    QString error;
    if (!readCoefficients(m_vertexA, m_vertexH, m_vertexK, a, h, k, error)) {
        m_message->setText(error);
        m_results->clear();
        return;
    }
    if (a == 0.0) {
        m_message->setText(QStringLiteral("a = 0 时不是二次方程，请改用一次方程求解。"));
        m_results->clear();
        return;
    }
    const calc::EquationSolution solution = calc::solveQuadraticVertex(a, h, k);
    m_results->clear();
    if (!solution.valid) {
        m_message->setText(QString::fromStdString(solution.reason));
        return;
    }
    m_results->addRow(QStringLiteral("展开式"),
                      QStringLiteral("%1x² %2x %3 = 0")
                          .arg(QString::fromStdString(calc::formatReal(a)),
                               QString::fromStdString(calc::formatReal(-2.0 * a * h)),
                               QString::fromStdString(calc::formatReal(a * h * h + k))));
    m_results->addRow(QStringLiteral("判别式 Δ"), QString::fromStdString(calc::formatReal(solution.discriminant)));
    m_results->addRow(QStringLiteral("解"), QString::fromStdString(solution.describe()));
    m_message->setText(QStringLiteral("求解完成。"));
}

void EquationWidget::solvePolynomial() {
    const QStringList tokens =
        m_polyInput->text().split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
    if (tokens.isEmpty()) {
        m_message->setText(QStringLiteral("请输入多项式系数。"));
        m_results->clear();
        return;
    }
    std::vector<double> coefficients;
    for (const QString& token : tokens) {
        const auto parsed = calc::parseDouble(token.toStdString());
        if (!parsed.ok) {
            m_message->setText(QStringLiteral("系数 %1 无效：%2")
                                   .arg(token, QString::fromStdString(parsed.error)));
            m_results->clear();
            return;
        }
        coefficients.push_back(parsed.value);
    }
    if (coefficients.size() < 2) {
        m_message->setText(QStringLiteral("至少需要两个系数。"));
        m_results->clear();
        return;
    }

    const calc::PolynomialRoots roots = calc::solvePolynomial(coefficients);
    m_results->clear();
    if (!roots.valid) {
        m_message->setText(QString::fromStdString(roots.reason));
        return;
    }
    for (size_t i = 0; i < roots.roots.size(); ++i) {
        m_results->addRow(QStringLiteral("根 x%1").arg(i + 1),
                          QString::fromStdString(calc::formatComplex(roots.roots[i])));
    }
    m_message->setText(QStringLiteral("求根完成，共 %1 个根。").arg(roots.roots.size()));
}
