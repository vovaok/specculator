#ifndef CPUWIDGET_H
#define CPUWIDGET_H

#include <QGroupBox>
#include <QLineEdit>
#include <QMap>
#include "z80.h"

class CpuWidget : public QGroupBox
{
    Q_OBJECT
public:
    explicit CpuWidget(QWidget *parent = nullptr);
    void bindCpu(Z80 *cpu) {m_cpu = cpu;}

    void updateRegs();

signals:
    void step();
    void run();

private:
    Z80 *m_cpu = nullptr;
    QMap<QString, QLineEdit *> m_regEdits;
    QLineEdit *m_bkptEdit;
};

#endif // CPUWIDGET_H
