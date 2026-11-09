// 离屏渲染冒烟测试：实例化每个页面、驱动一次计算、并把窗口渲染成 PNG。
// 用 QT_QPA_PLATFORM=offscreen 运行，无需真实显示器即可验证界面能正常构建与绘制。
#include <QApplication>
#include <QDir>
#include <QGroupBox>
#include <QLineEdit>
#include <QPixmap>
#include <QPushButton>
#include <QTableWidget>
#include <QTextStream>
#include <QWidget>

#include <iostream>

#include "EquationWidget.h"
#include "GeometryWidget.h"
#include "MainWindow.h"
#include "PrimeFactorWidget.h"
#include "StatisticsWidget.h"
#include "TriangleWidget.h"

namespace {

QTextStream& out() {
    static QTextStream stream(stdout);
    return stream;
}

// 按文字找到按钮并点击。
bool clickButton(QWidget* page, const QString& text) {
    const auto buttons = page->findChildren<QPushButton*>();
    for (auto* button : buttons) {
        if (button->text() == text) {
            button->click();
            return true;
        }
    }
    return false;
}

// 把控件渲染成 PNG，便于人工核对布局。
bool saveWidget(QWidget* widget, const QString& fileName) {
    widget->resize(1000, 640);
    widget->show();
    QCoreApplication::processEvents();
    const QPixmap pixmap = widget->grab();
    return pixmap.save(fileName, "PNG");
}

}  // namespace

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    const QString outputDir = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QStringLiteral(".");
    QDir().mkpath(outputDir);

    int failures = 0;

    // 1. 主窗口整体
    {
        MainWindow window;
        window.resize(1080, 720);
        window.show();
        QCoreApplication::processEvents();
        const bool ok = window.grab().save(QDir(outputDir).filePath("main-window.png"), "PNG");
        out() << (ok ? "ok   " : "FAIL ") << "主窗口渲染\n";
        if (!ok) ++failures;
    }

    // 2. 质因数分解：输入 1000000000 后计算
    {
        PrimeFactorWidget page;
        page.findChild<QLineEdit*>()->setText(QStringLiteral("1000000000"));
        clickButton(&page, QStringLiteral("计算"));
        QCoreApplication::processEvents();
        const bool ok = saveWidget(&page, QDir(outputDir).filePath("page-factorize.png"));
        out() << (ok ? "ok   " : "FAIL ") << "质因数分解页面\n";
        if (!ok) ++failures;
    }

    // 3. 方程求解：x² - 5x + 6 = 0
    {
        EquationWidget page;
        const auto edits = page.findChildren<QLineEdit*>();
        // 输入框顺序：一次方程 a, b；二次方程 a, b, c；顶点式 a, h, k；多项式
        if (edits.size() >= 5) {
            edits[2]->setText(QStringLiteral("1"));
            edits[3]->setText(QStringLiteral("-5"));
            edits[4]->setText(QStringLiteral("6"));
        }
        // 二次方程分组内的「求解」按钮
        const auto boxes = page.findChildren<QGroupBox*>();
        for (auto* box : boxes) {
            if (!box->title().contains(QStringLiteral("ax²"))) continue;
            const auto buttons = box->findChildren<QPushButton*>();
            if (!buttons.isEmpty()) {
                buttons.first()->click();
            }
        }
        QCoreApplication::processEvents();
        const bool ok = saveWidget(&page, QDir(outputDir).filePath("page-equation.png"));
        out() << (ok ? "ok   " : "FAIL ") << "方程求解页面\n";
        if (!ok) ++failures;
    }

    // 4. 三角形：3-4-5 直角三角形
    {
        TriangleWidget page;
        const auto edits = page.findChildren<QLineEdit*>();
        const QStringList values = {QStringLiteral("0"), QStringLiteral("0"), QStringLiteral("0"),
                                    QStringLiteral("3"), QStringLiteral("0"), QStringLiteral("0"),
                                    QStringLiteral("0"), QStringLiteral("4"), QStringLiteral("0")};
        for (int i = 0; i < qMin(edits.size(), values.size()); ++i) {
            edits[i]->setText(values[i]);
        }
        clickButton(&page, QStringLiteral("计算"));
        QCoreApplication::processEvents();
        const bool ok = saveWidget(&page, QDir(outputDir).filePath("page-triangle.png"));
        out() << (ok ? "ok   " : "FAIL ") << "三角形页面\n";
        if (!ok) ++failures;
    }

    // 5. 统计与回归：填入 4 个点
    {
        StatisticsWidget page;
        auto* table = page.findChild<QTableWidget*>();
        if (table) {
            const double xs[] = {1, 2, 3, 4};
            const double ys[] = {2.1, 3.9, 6.2, 7.8};
            for (int row = 0; row < 4; ++row) {
                table->item(row, 1)->setText(QString::number(xs[row]));
                table->item(row, 2)->setText(QString::number(ys[row]));
            }
        }
        clickButton(&page, QStringLiteral("计算"));
        QCoreApplication::processEvents();
        const bool ok = saveWidget(&page, QDir(outputDir).filePath("page-statistics.png"));
        out() << (ok ? "ok   " : "FAIL ") << "统计与回归页面\n";
        if (!ok) ++failures;
    }

    // 6. 空间几何：四面体
    {
        GeometryWidget page;
        const auto edits = page.findChildren<QLineEdit*>();
        const QStringList values = {QStringLiteral("0"), QStringLiteral("0"), QStringLiteral("0"),
                                    QStringLiteral("1"), QStringLiteral("0"), QStringLiteral("0"),
                                    QStringLiteral("0"), QStringLiteral("1"), QStringLiteral("0"),
                                    QStringLiteral("0"), QStringLiteral("0"), QStringLiteral("1")};
        for (int i = 0; i < qMin(edits.size(), values.size()); ++i) {
            edits[i]->setText(values[i]);
        }
        clickButton(&page, QStringLiteral("计算四面体"));
        QCoreApplication::processEvents();
        const bool ok = saveWidget(&page, QDir(outputDir).filePath("page-geometry.png"));
        out() << (ok ? "ok   " : "FAIL ") << "空间几何页面\n";
        if (!ok) ++failures;
    }

    out() << "\n离屏渲染完成，失败 " << failures << " 项。\n";
    out().flush();
    return failures == 0 ? 0 : 1;
}
