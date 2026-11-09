// 应用程序入口。
#include <QApplication>
#include <QFont>

#include "MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    QApplication::setApplicationName(QStringLiteral("综合计算工具"));
    QApplication::setApplicationVersion(QStringLiteral("2.0.0"));
    QApplication::setOrganizationName(QStringLiteral("ZajacMo"));

    // 界面统一使用支持中文的字体。
    QFont font(QStringLiteral("Microsoft YaHei UI"), 9);
    app.setFont(font);

    MainWindow window;
    window.show();
    return app.exec();
}
