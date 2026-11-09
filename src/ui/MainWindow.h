// 主窗口：左侧功能导航 + 右侧堆叠页面。
#pragma once

#include <QMainWindow>

class QStackedWidget;
class QListWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private:
    void buildUi();

    QListWidget* m_navigation = nullptr;
    QStackedWidget* m_pages = nullptr;
};
