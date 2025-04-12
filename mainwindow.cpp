#include "mainwindow.h"
#include <thread>
#include <chrono>
#include <QElapsedTimer>

using namespace std::literals;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QFontDatabase::addApplicationFont(":/res/fonts/zxspectr.ttf");
    QFontDatabase::addApplicationFont(":/res/fonts/ZxSpectrumRU.ttf");
    QFontDatabase::addApplicationFont(":/res/fonts/fontawesome-webfont.ttf");
    QFontDatabase::addApplicationFont(":/res/fonts/OldSchoolIcons.ttf");

    QFile f(":/style.css");
    f.open(QIODevice::ReadOnly);
    QByteArray css = f.readAll();
    f.close();

    int dpi = QGuiApplication::primaryScreen()->logicalDotsPerInch();
    int fontSizePx = 8 * ((dpi + 36) / 72);
    css = css.replace("$fontSizePx", QByteArray::number(fontSizePx) + "px");
    qApp->setProperty("fontSizePx", fontSizePx);

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

    joyWidget = new JoystickWidget;
    joyWidget->hide();

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
    act = m_toolbar->addAction("R"/*QChar(0xf021)*/, this, &MainWindow::reset);
    act->setToolTip("Reset");
    act->setShortcut(QKeySequence("F2"));
    act = m_toolbar->addAction("C"/*QChar(0xf2db)*/, this, &MainWindow::setDebugMode);
    act->setToolTip("Debug");
    act->setCheckable(true);
    m_debugAction = act;
    act = m_toolbar->addAction("T"/*QChar(0xf025)*/, tapeWidget, &QWidget::setVisible);
    act->setToolTip("Cassette tape");
    act->setCheckable(true);
    m_tapeAction = act;
    act = m_toolbar->addAction("J"/*QChar(0xf025)*/, joyWidget, &QWidget::setVisible);
    act->setToolTip("Joystick");
    act->setCheckable(true);
    QMenu *joyMenu = new QMenu("Joystick", m_toolbar);
    act->setMenu(joyMenu);
    QActionGroup *joyTypeGroup = new QActionGroup(joyMenu);
    joyTypeGroup->addAction(
        joyMenu->addAction("Kempston", [=](){joyWidget->setJoystickType(ZxJoystick::Kempston);}));
    joyTypeGroup->addAction(
        joyMenu->addAction("Sinclair", [=](){joyWidget->setJoystickType(ZxJoystick::Sinclair);}));
    joyTypeGroup->addAction(
        joyMenu->addAction("Cursor", [=](){joyWidget->setJoystickType(ZxJoystick::Cursor);}));
    joyTypeGroup->addAction(
        joyMenu->addAction("Interface II", [=](){joyWidget->setJoystickType(ZxJoystick::InterfaceII);}));
    for (QAction *a: joyTypeGroup->actions())
        a->setCheckable(true);
    joyTypeGroup->actions()[0]->setChecked(true);
    act = m_toolbar->addAction("K"/*QChar(0xf11c)*/, keybWidget, &QWidget::setVisible);
    act->setToolTip("ZX keyboard");
    act->setCheckable(true);
    m_keybAction = act;
    QToolButton *snapshotBtn = new QToolButton();
    snapshotBtn->setText("P");//QChar(0xf0c7));
    snapshotBtn->setToolTip("Take snapshot");
    snapshotBtn->setCheckable(true);
    m_toolbar->addWidget(snapshotBtn);
//    act = m_toolbar->addAction(QChar(0xf0c7));//, [=](){});
    QMenu *menu = new QMenu("Snapshot", m_toolbar);
    snapshotBtn->setMenu(menu);
    connect(snapshotBtn, &QToolButton::pressed, snapshotBtn, &QToolButton::showMenu);
    connect(menu, &QMenu::aboutToShow, computer, &Computer::pause);
    connect(menu, &QMenu::aboutToHide, computer, &Computer::resume);
    act = menu->addAction("Save state"/*QString(QChar(0xf0c7)) + " save state"*/, computer, &Computer::save);
    act->setToolTip("Save state");
    act->setShortcut(QKeySequence("F8"));
    act = menu->addAction("Restore state"/*QString(QChar(0xf01e/*0xf1da*)) + " restore"*/, computer, &Computer::restore);
    act->setToolTip("Restore state");
    act->setShortcut(QKeySequence("F9"));
    m_turboBtn = new QToolButton();
    m_turboBtn->setText("F"/*QChar(0xf135)*/);
    m_turboBtn->setToolTip("Turbo");
    m_toolbar->addWidget(m_turboBtn);
//    act = m_toolbar->addAction(QChar(0xf135), computer, &Computer::restore);
//    act->setToolTip("Turbo");

    if (QToolButton *button = m_toolbar->findChild<QToolButton *>("qt_toolbar_ext_button"))
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

    status = new QLabel();
    dynamic_cast<QVBoxLayout*>(scrWidget->layout())->addWidget(status, 0, Qt::AlignTop | Qt::AlignRight);
//    status->move(4, 0);
//    status->setMinimumWidth(400);

    setContentsMargins(0, 0, 0, 0);
    m_layout = new QGridLayout;
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    m_layout->addWidget(tapeWidget, 0, 0);
    m_layout->addWidget(cpuWidget, 0, 1);
    m_layout->addWidget(scrWidget, 0, 2);
    m_layout->addWidget(keybWidget, 1, 0, 1, 3);
    m_layout->addWidget(joyWidget, 2, 0, 1, 3);

    setCentralWidget(new QWidget());
    centralWidget()->setLayout(m_layout);

#if defined(Q_OS_ANDROID)
    resize(QGuiApplication::primaryScreen()->geometry().size());
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
    joyWidget->bindJoystick(computer->joystick());
}

void MainWindow::unbindWidgets()
{
    scrWidget->bindScreen(nullptr);
    cpuWidget->bindCpu(nullptr);
    tapeWidget->bindTape(nullptr);
    keybWidget->bindKeyboard(nullptr);
    joyWidget->bindJoystick(nullptr);
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
        m_layout->addWidget(cpuWidget, 0, 2, 3, 1);
        m_layout->addWidget(keybWidget, 1, 0, 1, 2);

        m_layout->removeWidget(joyWidget);
        joyWidget->setParent(this);
        joyWidget->move(0 * m_toolbar->width(), 0);
        QSize sz = e->size();
        sz.rwidth() -= joyWidget->x();
        joyWidget->resize(sz);

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
        m_layout->addWidget(joyWidget, 1, 0, 1, 2);
        m_layout->addWidget(tapeWidget, 2, 0, 1, 2);
        m_layout->addWidget(keybWidget, 3, 0, 1, 2);
//        m_layout->addWidget(joyWidget, 3, 0, 1, 2);

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
