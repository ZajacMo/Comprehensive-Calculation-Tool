// 界面复用的小工具：统一的表单构建、结果展示与样式。
#pragma once

#include <QFormLayout>
#include <QLabel>
#include <QString>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QWidget>

namespace ui {

// 结果区：左侧是键值表，右侧可放图示或补充说明。
class ResultPanel : public QWidget {
public:
    explicit ResultPanel(QWidget* parent = nullptr);

    // 追加一行结果。value 会被自动加粗显示。
    void addRow(const QString& key, const QString& value);

    // 覆盖式设置一行（键相同则替换，否则追加）。
    void setRow(const QString& key, const QString& value);

    // 清空所有结果行。
    void clear();

    // 结果区是否为空。
    bool isEmpty() const;

    // 用纯文本形式返回全部结果，便于复制。
    QString toPlainText() const;

private:
    QTableWidget* m_table = nullptr;
};

// 构建一个只读、可选中的结果标签。
QLabel* makeValueLabel();

// 给标签应用统一的样式：主色 + 半透明背景。
void styleValueLabel(QLabel* label);

// 结果为空时的占位提示。
QLabel* makePlaceholder(const QString& text = "填写左侧数据后点击「计算」");

}  // namespace ui
