#include "MainWindow.h"

#include <QListWidget>
#include <QStackedWidget>
#include <QSplitter>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QWidget>

#include "EquationWidget.h"
#include "GeometryWidget.h"
#include "PrimeFactorWidget.h"
#include "StatisticsWidget.h"
#include "TriangleWidget.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    buildUi();
}

MainWindow::~MainWindow() = default;

void MainWindow::buildUi() {
    setWindowTitle(QStringLiteral("综合计算工具"));
    resize(1080, 720);

    auto* central = new QWidget;
    auto* layout = new QVBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_navigation = new QListWidget;
    m_navigation->setFixedWidth(190);
    m_navigation->setSpacing(2);
    m_navigation->setFrameShape(QFrame::NoFrame);
    m_navigation->setStyleSheet(QStringLiteral(
        "QListWidget { background: #f8fafc; border-right: 1px solid #e2e8f0; padding: 12px 8px; }"
        "QListWidget::item { padding: 10px 12px; border-radius: 6px; color: #334155; }"
        "QListWidget::item:hover { background: #eef2ff; }"
        "QListWidget::item:selected { background: #dbeafe; color: #1d4ed8; font-weight: 600; }"));

    m_navigation->addItem(QStringLiteral("质因数分解"));
    m_navigation->addItem(QStringLiteral("统计与回归"));
    m_navigation->addItem(QStringLiteral("方程求解"));
    m_navigation->addItem(QStringLiteral("空间几何"));
    m_navigation->addItem(QStringLiteral("三角形计算"));

    m_pages = new QStackedWidget;
    m_pages->addWidget(new PrimeFactorWidget);
    m_pages->addWidget(new StatisticsWidget);
    m_pages->addWidget(new EquationWidget);
    m_pages->addWidget(new GeometryWidget);
    m_pages->addWidget(new TriangleWidget);

    auto* splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(m_navigation);
    splitter->addWidget(m_pages);
    splitter->setStretchFactor(1, 1);
    splitter->setCollapsible(0, false);

    layout->addWidget(splitter);
    setCentralWidget(central);

    statusBar()->showMessage(QStringLiteral("就绪 · 所有计算均由 src/core 的算法内核完成"));

    connect(m_navigation, &QListWidget::currentRowChanged, m_pages, &QStackedWidget::setCurrentIndex);
    m_navigation->setCurrentRow(0);
}
