#include "PrimeFactorWidget.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include "UiCommon.h"
#include "core/InputParsing.h"
#include "core/NumberTheory.h"
#include "core/Numeric.h"

PrimeFactorWidget::PrimeFactorWidget(QWidget* parent) : QWidget(parent) {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(24, 24, 24, 24);
    root->setSpacing(16);

    auto* title = new QLabel(QStringLiteral("质因数分解"));
    title->setObjectName(QStringLiteral("pageTitle"));
    root->addWidget(title);

    auto* description = new QLabel(
        QStringLiteral("输入一个不小于 2 的整数，程序会给出质因数分解，并一并计算欧拉函数 φ、"
                       "莫比乌斯函数 μ、约数个数与约数和。分解采用试除法（只试到 √n），"
                       "10⁹ 级别的输入也能即时返回。"));
    description->setWordWrap(true);
    description->setStyleSheet(QStringLiteral("color: #6b7280;"));
    root->addWidget(description);

    auto* form = new QFormLayout;
    form->setSpacing(12);

    m_input = new QLineEdit;
    m_input->setPlaceholderText(QStringLiteral("例如 1000000000"));
    m_input->setClearButtonEnabled(true);
    m_input->setMaximumWidth(280);
    form->addRow(QStringLiteral("待分解的整数："), m_input);

    auto* buttonRow = new QHBoxLayout;
    buttonRow->setSpacing(8);
    auto* calculateButton = new QPushButton(QStringLiteral("计算"));
    calculateButton->setObjectName(QStringLiteral("primaryButton"));
    calculateButton->setDefault(true);
    buttonRow->addWidget(calculateButton);
    buttonRow->addStretch();
    form->addRow(QString(), buttonRow);
    root->addLayout(form);

    m_message = ui::makePlaceholder();
    root->addWidget(m_message);

    m_results = new ui::ResultPanel;
    root->addWidget(m_results, 1);

    connect(calculateButton, &QPushButton::clicked, this, &PrimeFactorWidget::calculate);
    connect(m_input, &QLineEdit::returnPressed, this, &PrimeFactorWidget::calculate);
}

void PrimeFactorWidget::calculate() {
    const auto parsed = calc::parseInteger(m_input->text().toStdString());
    if (!parsed.ok) {
        m_message->setText(QString::fromStdString(parsed.error));
        m_results->clear();
        return;
    }
    const calc::int64 n = parsed.value;
    if (n < 2) {
        m_message->setText(QStringLiteral("请输入不小于 2 的整数。"));
        m_results->clear();
        return;
    }

    const calc::Factorization factorization = calc::factorize(n);
    const bool prime = calc::isPrime(static_cast<calc::uint64>(n));

    m_message->setText(QStringLiteral("计算完成。"));
    m_results->clear();
    m_results->addRow(QStringLiteral("输入值 n"), QString::number(n));
    m_results->addRow(QStringLiteral("质因数分解"), QString::fromStdString(factorization.describe()));
    m_results->addRow(QStringLiteral("是否素数"), prime ? QStringLiteral("是") : QStringLiteral("否"));
    m_results->addRow(QStringLiteral("欧拉函数 φ(n)"), QString::number(calc::eulerPhi(n)));
    m_results->addRow(QStringLiteral("莫比乌斯函数 μ(n)"), QString::number(calc::mobius(n)));
    m_results->addRow(QStringLiteral("正约数个数"), QString::number(calc::divisorCount(n)));
    m_results->addRow(QStringLiteral("正约数之和"), QString::number(calc::divisorSum(n)));

    // 校验分解自洽：把质因数乘回去应当等于原数。
    if (factorization.reconstruct() != n) {
        m_message->setText(QStringLiteral("警告：分解结果与输入不一致，请反馈。"));
    }
}
