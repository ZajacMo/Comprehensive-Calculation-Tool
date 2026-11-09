#include "StatisticsWidget.h"

#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

#include "UiCommon.h"
#include "core/InputParsing.h"
#include "core/Numeric.h"
#include "core/Statistics.h"

namespace {

constexpr int kColumnCount = 3;   // 点号、x、y
constexpr int kInitialRows = 5;

}  // namespace

StatisticsWidget::StatisticsWidget(QWidget* parent) : QWidget(parent) {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(24, 24, 24, 24);
    root->setSpacing(16);

    auto* title = new QLabel(QStringLiteral("统计与回归"));
    title->setObjectName(QStringLiteral("pageTitle"));
    root->addWidget(title);

    auto* description = new QLabel(
        QStringLiteral("在表格中录入若干 (x, y) 数据点，程序会给出 x、y 各自的描述统计，"
                       "以及最小二乘拟合直线的斜率 k、截距 b、相关系数 r 与决定系数 R²，"
                       "并可代入任意 x 预测 y。数据点数量不限。"));
    description->setWordWrap(true);
    description->setStyleSheet(QStringLiteral("color: #6b7280;"));
    root->addWidget(description);

    m_table = new QTableWidget(kInitialRows, kColumnCount);
    m_table->setHorizontalHeaderLabels({QStringLiteral("点号"), QStringLiteral("x"), QStringLiteral("y")});
    m_table->verticalHeader()->setVisible(false);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    root->addWidget(m_table, 1);

    auto* tableButtons = new QHBoxLayout;
    tableButtons->setSpacing(8);
    auto* addButton = new QPushButton(QStringLiteral("增加一行"));
    auto* removeButton = new QPushButton(QStringLiteral("删除末行"));
    auto* clearButton = new QPushButton(QStringLiteral("清空"));
    tableButtons->addWidget(addButton);
    tableButtons->addWidget(removeButton);
    tableButtons->addWidget(clearButton);
    tableButtons->addStretch();
    root->addLayout(tableButtons);

    auto* actionRow = new QHBoxLayout;
    actionRow->setSpacing(8);
    auto* calculateButton = new QPushButton(QStringLiteral("计算"));
    calculateButton->setObjectName(QStringLiteral("primaryButton"));
    actionRow->addWidget(calculateButton);

    actionRow->addSpacing(24);
    actionRow->addWidget(new QLabel(QStringLiteral("代入 x =")));
    m_predictX = new QLineEdit;
    m_predictX->setPlaceholderText(QStringLiteral("预测 y"));
    m_predictX->setMaximumWidth(160);
    actionRow->addWidget(m_predictX);
    auto* predictButton = new QPushButton(QStringLiteral("求值"));
    actionRow->addWidget(predictButton);
    actionRow->addStretch();
    root->addLayout(actionRow);

    m_message = ui::makePlaceholder();
    root->addWidget(m_message);

    m_results = new ui::ResultPanel;
    root->addWidget(m_results);

    connect(addButton, &QPushButton::clicked, this, &StatisticsWidget::addPointRow);
    connect(removeButton, &QPushButton::clicked, this, &StatisticsWidget::removePointRow);
    connect(clearButton, &QPushButton::clicked, this, [this] {
        m_table->setRowCount(kInitialRows);
        for (int row = 0; row < m_table->rowCount(); ++row) {
            m_table->setItem(row, 0, new QTableWidgetItem(QString::number(row + 1)));
            m_table->setItem(row, 1, new QTableWidgetItem);
            m_table->setItem(row, 2, new QTableWidgetItem);
        }
        m_results->clear();
        m_message->setText(QStringLiteral("填写左侧数据后点击「计算」"));
    });
    connect(calculateButton, &QPushButton::clicked, this, &StatisticsWidget::calculate);
    connect(predictButton, &QPushButton::clicked, this, &StatisticsWidget::predict);
    connect(m_predictX, &QLineEdit::returnPressed, this, &StatisticsWidget::predict);

    // 初始化点号列。
    for (int row = 0; row < m_table->rowCount(); ++row) {
        m_table->setItem(row, 0, new QTableWidgetItem(QString::number(row + 1)));
        m_table->setItem(row, 1, new QTableWidgetItem);
        m_table->setItem(row, 2, new QTableWidgetItem);
    }
}

void StatisticsWidget::addPointRow() {
    const int row = m_table->rowCount();
    m_table->insertRow(row);
    m_table->setItem(row, 0, new QTableWidgetItem(QString::number(row + 1)));
    m_table->setItem(row, 1, new QTableWidgetItem);
    m_table->setItem(row, 2, new QTableWidgetItem);
}

void StatisticsWidget::removePointRow() {
    if (m_table->rowCount() <= 1) {
        m_message->setText(QStringLiteral("至少需要保留一行。"));
        return;
    }
    m_table->removeRow(m_table->rowCount() - 1);
    for (int row = 0; row < m_table->rowCount(); ++row) {
        m_table->item(row, 0)->setText(QString::number(row + 1));
    }
}

bool StatisticsWidget::collectPoints(std::vector<calc::Point>& points, QString& error) {
    points.clear();
    for (int row = 0; row < m_table->rowCount(); ++row) {
        const auto* xItem = m_table->item(row, 1);
        const auto* yItem = m_table->item(row, 2);
        const QString xText = xItem ? xItem->text().trimmed() : QString();
        const QString yText = yItem ? yItem->text().trimmed() : QString();

        // 两个都为空表示这一行还没填，跳过而不是报错。
        if (xText.isEmpty() && yText.isEmpty()) continue;
        if (xText.isEmpty() || yText.isEmpty()) {
            error = QStringLiteral("第 %1 行只填了部分数据，x 与 y 需要成对给出。").arg(row + 1);
            return false;
        }

        const auto xParsed = calc::parseDouble(xText.toStdString());
        if (!xParsed.ok) {
            error = QStringLiteral("第 %1 行的 x 无效：%2")
                        .arg(row + 1)
                        .arg(QString::fromStdString(xParsed.error));
            return false;
        }
        const auto yParsed = calc::parseDouble(yText.toStdString());
        if (!yParsed.ok) {
            error = QStringLiteral("第 %1 行的 y 无效：%2")
                        .arg(row + 1)
                        .arg(QString::fromStdString(yParsed.error));
            return false;
        }
        points.push_back({xParsed.value, yParsed.value});
    }
    if (points.empty()) {
        error = QStringLiteral("请先输入至少一个数据点。");
        return false;
    }
    return true;
}

void StatisticsWidget::calculate() {
    std::vector<calc::Point> points;
    QString error;
    if (!collectPoints(points, error)) {
        m_message->setText(error);
        m_results->clear();
        return;
    }

    const calc::DescriptiveStats xs = calc::describeX(points);
    const calc::DescriptiveStats ys = calc::describeY(points);
    const calc::LinearRegression fit = calc::fitLinearRegression(points);

    m_results->clear();
    m_results->addRow(QStringLiteral("样本数 n"), QString::number(points.size()));

    m_results->addRow(QStringLiteral("x 的平均值 x̄"), QString::fromStdString(calc::formatReal(xs.mean)));
    m_results->addRow(QStringLiteral("x 的中位数"), QString::fromStdString(calc::formatReal(xs.median)));
    m_results->addRow(QStringLiteral("x 的标准差"), QString::fromStdString(calc::formatReal(xs.stdDev)));
    m_results->addRow(QStringLiteral("x 的极差"), QString::fromStdString(calc::formatReal(xs.range)));

    m_results->addRow(QStringLiteral("y 的平均值 ȳ"), QString::fromStdString(calc::formatReal(ys.mean)));
    m_results->addRow(QStringLiteral("y 的中位数"), QString::fromStdString(calc::formatReal(ys.median)));
    m_results->addRow(QStringLiteral("y 的标准差"), QString::fromStdString(calc::formatReal(ys.stdDev)));
    m_results->addRow(QStringLiteral("y 的极差"), QString::fromStdString(calc::formatReal(ys.range)));

    if (!fit.valid) {
        m_results->addRow(QStringLiteral("回归结果"), QStringLiteral("无法拟合"));
        m_message->setText(QStringLiteral("描述统计已给出，但无法拟合直线：%1")
                               .arg(QString::fromStdString(fit.reason)));
        return;
    }

    m_results->addRow(QStringLiteral("回归斜率 k"), QString::fromStdString(calc::formatReal(fit.slope)));
    m_results->addRow(QStringLiteral("回归截距 b"), QString::fromStdString(calc::formatReal(fit.intercept)));
    m_results->addRow(QStringLiteral("相关系数 r"), QString::fromStdString(calc::formatReal(fit.r)));
    m_results->addRow(QStringLiteral("决定系数 R²"), QString::fromStdString(calc::formatReal(fit.rSquared)));
    if (fit.slopeStdError > 0.0) {
        m_results->addRow(QStringLiteral("k 的标准误"),
                          QString::fromStdString(calc::formatReal(fit.slopeStdError)));
    }
    m_results->addRow(QStringLiteral("回归方程"),
                      QStringLiteral("y = %1x + %2")
                          .arg(QString::fromStdString(calc::formatReal(fit.slope)),
                               QString::fromStdString(calc::formatReal(fit.intercept))));

    m_message->setText(QStringLiteral("计算完成。"));
}

void StatisticsWidget::predict() {
    std::vector<calc::Point> points;
    QString error;
    if (!collectPoints(points, error)) {
        m_message->setText(error);
        return;
    }
    const calc::LinearRegression fit = calc::fitLinearRegression(points);
    if (!fit.valid) {
        m_message->setText(QStringLiteral("无法拟合直线：%1").arg(QString::fromStdString(fit.reason)));
        return;
    }
    const auto parsed = calc::parseDouble(m_predictX->text().toStdString());
    if (!parsed.ok) {
        m_message->setText(QString::fromStdString(parsed.error));
        return;
    }
    const double y = fit.predict(parsed.value);
    m_results->setRow(QStringLiteral("预测值 y"),
                      QStringLiteral("x = %1 时，y = %2")
                          .arg(QString::fromStdString(calc::formatReal(parsed.value)),
                               QString::fromStdString(calc::formatReal(y))));
    m_message->setText(QStringLiteral("预测完成。"));
}
