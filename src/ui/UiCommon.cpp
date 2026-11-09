#include "UiCommon.h"

#include <QHeaderView>
#include <QTableWidgetItem>

namespace ui {

ResultPanel::ResultPanel(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(2);
    m_table->setHorizontalHeaderLabels({QStringLiteral("项目"), QStringLiteral("结果")});
    m_table->verticalHeader()->setVisible(false);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setAlternatingRowColors(true);
    layout->addWidget(m_table);
}

void ResultPanel::addRow(const QString& key, const QString& value) {
    const int row = m_table->rowCount();
    m_table->insertRow(row);

    auto* keyItem = new QTableWidgetItem(key);
    keyItem->setForeground(QColor(120, 120, 120));
    m_table->setItem(row, 0, keyItem);

    auto* valueItem = new QTableWidgetItem(value);
    QFont valueFont(QStringLiteral("Consolas"));
    valueFont.setPointSize(10);
    valueFont.setBold(true);
    valueItem->setFont(valueFont);
    valueItem->setToolTip(value);
    m_table->setItem(row, 1, valueItem);
}

void ResultPanel::setRow(const QString& key, const QString& value) {
    for (int row = 0; row < m_table->rowCount(); ++row) {
        if (m_table->item(row, 0) && m_table->item(row, 0)->text() == key) {
            m_table->item(row, 1)->setText(value);
            m_table->item(row, 1)->setToolTip(value);
            return;
        }
    }
    addRow(key, value);
}

void ResultPanel::clear() {
    m_table->setRowCount(0);
}

bool ResultPanel::isEmpty() const {
    return m_table->rowCount() == 0;
}

QString ResultPanel::toPlainText() const {
    QStringList lines;
    for (int row = 0; row < m_table->rowCount(); ++row) {
        const auto* keyItem = m_table->item(row, 0);
        const auto* valueItem = m_table->item(row, 1);
        if (keyItem && valueItem) {
            lines << QStringLiteral("%1: %2").arg(keyItem->text(), valueItem->text());
        }
    }
    return lines.join('\n');
}

QLabel* makeValueLabel() {
    auto* label = new QLabel;
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    label->setWordWrap(true);
    return label;
}

void styleValueLabel(QLabel* label) {
    label->setStyleSheet(QStringLiteral(
        "QLabel {"
        "  background: rgba(37, 99, 235, 0.08);"
        "  border: 1px solid rgba(37, 99, 235, 0.25);"
        "  border-radius: 4px;"
        "  padding: 6px 8px;"
        "  color: #1e3a8a;"
        "}"));
}

QLabel* makePlaceholder(const QString& text) {
    auto* label = new QLabel(text);
    label->setStyleSheet(QStringLiteral("color: #9ca3af; font-style: italic;"));
    label->setWordWrap(true);
    return label;
}

}  // namespace ui
