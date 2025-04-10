#include "mainwindow.h"
#include <thread>
#include <chrono>
#include <QElapsedTimer>

using namespace std::literals;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QFontDatabase::addApplicationFont(":/res/fonts/zxspectr.ttf");
    QFontDatabase::addApplicationFont(":/res/fonts/ZXDTPSi.ttf");
    QFontDatabase::addApplicationFont(":/res/fonts/fontawesome-webfont.ttf");

    QFile f(":/style.css");
    f.open(QIODevice::ReadOnly);
    QByteArray css = f.readAll();
    f.close();

    int dpi = QGuiApplication::primaryScreen()->logicalDotsPerInch();
    int fontSizePx = 23 * dpi / 144;
    css = css.replace("$fontSizePx", QByteArray::number(fontSizePx) + "px");

    setStyleSheet(css);

    scrWidget = new ScreenWidget();
    scrWidget->setFocusPolicy(Qt::StrongFocus);
    scrWidget->setFocus();

    cpuWidget = new CpuWidget;
    cpuWidget->hide();

    tapeWidget = new TapeWidget;
    tapeWidget->hide();

    keybWidget = new KeyboardWidget;
    keybWidget->hide();

    computer = new Computer();
    connect(computer, &Computer::powerOn, this, &MainWindow::bindWidgets);
    connect(computer, &Computer::powerOff, this, &MainWindow::unbindWidgets, Qt::DirectConnection);
    connect(computer, &Computer::vsync, this, &MainWindow::updateScreen);
    computer->start();
//    connect(computer, &Computer::finished, computer, &Computer::deleteLater);
    computer->setPriority(QThread::TimeCriticalPriority);

    m_toolbar = addToolBar("main");
//    m_toolbar->setFloatable(false);
    m_toolbar->setMovable(false);
    QAction *act;
    act = m_toolbar->addAction(QChar(0xf021), this, &MainWindow::reset);
    act->setToolTip("Reset");
    act->setShortcut(QKeySequence("F2"));
    act = m_toolbar->addAction(QChar(0xf2db/*0xf051*//*0xf188*/), this, &MainWindow::setDebugMode);
    act->setToolTip("Debug");
    act->setCheckable(true);
    m_debugAction = act;
    act = m_toolbar->addAction(QChar(0xf025), tapeWidget, &QWidget::setVisible);
    act->setToolTip("Cassette tape");
    act->setCheckable(true);
    m_tapeAction = act;
    act = m_toolbar->addAction(QChar(0xf11c), keybWidget, &QWidget::setVisible);
    act->setToolTip("ZX keyboard");
    act->setCheckable(true);
    m_keybAction = act;
    act = m_toolbar->addAction(QChar(0xf0c7), computer, &Computer::save);
    act->setToolTip("Save state");
    act->setShortcut(QKeySequence("F8"));
    act = m_toolbar->addAction(QChar(0xf01e/*0xf1da*/), computer, &Computer::restore);
    act->setToolTip("Restore state");
    act->setShortcut(QKeySequence("F9"));
    m_turboBtn = new QToolButton();
    m_turboBtn->setText(QChar(0xf135));
    m_turboBtn->setToolTip("Turbo");
    m_toolbar->addWidget(m_turboBtn);
//    act = m_toolbar->addAction(QChar(0xf135), computer, &Computer::restore);
//    act->setToolTip("Turbo");

    if(QToolButton *button = m_toolbar->findChild<QToolButton *>("qt_toolbar_ext_button"))
    {
        button->setToolButtonStyle(Qt::ToolButtonTextOnly);
        button->setText(QChar(0xf141));
    }

    connect(cpuWidget, &CpuWidget::step, computer, &Computer::step);
    connect(cpuWidget, &CpuWidget::run, this, [this]()
    {
        setDebugMode(false);
        m_debugAction->setChecked(false);
    });

    status = new QLabel(scrWidget);
    status->move(4, 0);
    status->setMinimumWidth(400);

    setContentsMargins(0, 0, 0, 0);
    m_layout = new QGridLayout;
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    m_layout->addWidget(tapeWidget, 0, 0);
    m_layout->addWidget(cpuWidget, 0, 1);
    m_layout->addWidget(scrWidget, 0, 2);
    m_layout->addWidget(keybWidget, 1, 0, 1, 3);

    setCentralWidget(new QWidget());
    centralWidget()->setLayout(m_layout);

#if defined(Q_OS_ANDROID)
    resize(QGuiApplication::primaryScreen()->geometry().size());// QApplication::desktop()->screenGeometry().size());
#endif

//    setProperty("orient", "landscape");

//    QTimer *timer = new QTimer(this);
//    connect(timer, &QTimer::timeout, this, &MainWindow::updateScreen);
//    timer->start(20);
}

MainWindow::~MainWindow()
{
    delete computer;
}

void MainWindow::reset()
{
    computer->reset();
    updateScreen();
}

void MainWindow::setDebugMode(bool enabled)
{
    if (enabled)
    {
        computer->pause();
        cpuWidget->show();
        cpuWidget->updateRegs();
    }
    else
    {
        cpuWidget->hide();
        computer->resume();
    }
}

//void MainWindow::step()
//{
//    computer->step();
//    cpuWidget->show();
//    cpuWidget->updateRegs();
//}

//void MainWindow::run()
//{
//    cpuWidget->hide();
//    computer->resume();
//}

void MainWindow::bindWidgets()
{
    scrWidget->bindScreen(computer->screen());
    cpuWidget->bindCpu(computer->cpu());
    tapeWidget->bindTape(computer->tape());
    keybWidget->bindKeyboard(computer->keyboard());
}

void MainWindow::unbindWidgets()
{
    scrWidget->bindScreen(nullptr);
    cpuWidget->bindCpu(nullptr);
    tapeWidget->bindTape(nullptr);
    keybWidget->bindKeyboard(nullptr);
}

void MainWindow::updateScreen()
{
    computer->turbo = m_turboBtn->isDown();

    if (scrWidget)
        scrWidget->update();

//    if (cpuWidget)
//        cpuWidget->updateRegs();

    if (tapeWidget)
        tapeWidget->updateState();

    if (keybWidget && keybWidget->isVisible())
        keybWidget->updateState();

//    status->setText(QString("%1").arg(cpu->cyclesCount()));
    status->setText(QString("CPU:%1%").arg(computer->cpuUsage(), 3));
//    status->setText(QString("%1/%2").arg(run_us).arg(frame_us));
    //    qDebug() << QString("%1/%2 - %3").arg(run_us).arg(frame_us).arg(NN);
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    computer->requestInterruption();
    computer->wait(1000);
    //    e->ignore();
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
    bool album = e->size().width() >= e->size().height();
    if (album)
    {
        setProperty("orient", "landscape");
//        keybWidget->hide();
        m_layout->addWidget(tapeWidget, 0, 0);
        m_layout->addWidget(scrWidget, 0, 1);
        m_layout->addWidget(cpuWidget, 0, 2, 2, 1);
        m_layout->addWidget(keybWidget, 1, 0, 1, 2);
        m_layout->setColumnStretch(1, 1);
        m_layout->setColumnStretch(0, 0);
        m_layout->setRowStretch(0, 0);
        removeToolBar(m_toolbar);
        addToolBar(Qt::LeftToolBarArea, m_toolbar);
        m_toolbar->show();
#ifdef Q_OS_ANDROID
        showFullScreen();
#endif
    }
    else
    {
        setProperty("orient", "portrait");
        m_layout->addWidget(scrWidget, 0, 0);
        m_layout->addWidget(cpuWidget, 0, 1);
        m_layout->addWidget(tapeWidget, 1, 0, 1, 2);
        m_layout->addWidget(keybWidget, 2, 0, 1, 2);
        m_layout->setColumnStretch(1, 0);
        m_layout->setColumnStretch(0, 1);
        m_layout->setRowStretch(0, 1);
        m_keybAction->setChecked(true);
        keybWidget->show();
        removeToolBar(m_toolbar);
        addToolBar(Qt::TopToolBarArea, m_toolbar);
        m_toolbar->show();
#ifdef Q_OS_ANDROID
        showMaximized();
#endif
    }
    style()->polish(m_toolbar);
}

void MainWindow::keyPressEvent(QKeyEvent *e)
{
    if (e->isAutoRepeat())
        return;
    int key = e->key();
    if (key == Qt::Key_Escape)
        computer->turbo = true;
    if (computer->keyboard())
        computer->keyboard()->setKeyState(key, true);
    if (e->key() == Qt::Key_Back)
        close();
}

void MainWindow::keyReleaseEvent(QKeyEvent *e)
{
    if (e->isAutoRepeat())
        return;
    int key = e->key();
    if (key == Qt::Key_Escape)
        computer->turbo = false;
    if (computer->keyboard())
        computer->keyboard()->setKeyState(key, false);
}
