#include "cpuwidget.h"
#include <QFormLayout>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QScreen>
#include <QDebug>
#include <QPushButton>
#include <QLabel>

CpuWidget::CpuWidget(QWidget *parent) : QGroupBox("CPU", parent)
{
//    setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    int fontSizePx = qApp->property("fontSizePx").toInt();

    QFormLayout *reglay = new QFormLayout;

    QPushButton *stepBtn = new QPushButton(QChar(0xf051));
    stepBtn->setShortcut(QKeySequence("F10"));
    stepBtn->setToolTip("Step into");
    connect(stepBtn, &QPushButton::clicked, this, [this]()
    {
        emit step();
        updateRegs();
    });

    QPushButton *runBtn = new QPushButton(QChar(0xf144));
    runBtn->setShortcut(QKeySequence("F5"));
    runBtn->setToolTip("Run");
    connect(runBtn, &QPushButton::clicked, this, &CpuWidget::run);

    reglay->addRow(stepBtn, runBtn);

    QStringList regNames = {"T", "FLAGS", "AF", "BC", "DE", "HL", "AF'", "BC'", "DE'", "HL'",
                            "PC", "SP", "IX", "IY", "I", "R", "IM", "IFF1", "IFF2"};
    for (const QString &name: regNames)
    {
        QLineEdit *edit = new QLineEdit;
//        edit->setFixedWidth(fontSizePx * 4 + 8);
        edit->setAlignment(Qt::AlignRight);
        edit->setReadOnly(true);
        edit->setObjectName(name);
        m_regEdits[name] = edit;
        if (name == "T")
        {
            reglay->addRow(new QLabel("CYCLES:"));
            reglay->addRow(edit);
//            edit->setFixedWidth(fontSizePx * 8 + 8);
        }
        else if (name == "FLAGS")
        {
            reglay->addRow(new QLabel("FLAGS:"));
            reglay->addRow(edit);
//            edit->setFixedWidth(fontSizePx * 8 + 8);
        }
        else
        {
            edit->setFixedWidth(fontSizePx * 4 + 8);
            reglay->addRow(name, edit);
        }
    }

    m_bkptEdit = new QLineEdit("0000");
    m_bkptEdit->setAlignment(Qt::AlignRight);
    connect(m_bkptEdit, &QLineEdit::textChanged, [this]()
    {
        m_bkptEdit->setStyleSheet("background-color: yellow;");
    });
    connect(m_bkptEdit, &QLineEdit::returnPressed, [this]()
    {
        bool ok;
        uint16_t v = m_bkptEdit->text().toInt(&ok, 16);
        if (ok)
        {
            m_bkptEdit->setStyleSheet("");
            if (m_cpu)
                m_cpu->bkpt  = v;
        }
    });
    reglay->addRow("BKPT", m_bkptEdit);

    setLayout(reglay);
}


void CpuWidget::updateRegs()
{
    if (!m_cpu || !isVisible())
        return;

    m_regEdits["AF" ]->setText(QString().asprintf("%04X", m_cpu->AF));
    m_regEdits["BC" ]->setText(QString().asprintf("%04X", m_cpu->BC));
    m_regEdits["DE" ]->setText(QString().asprintf("%04X", m_cpu->DE));
    m_regEdits["HL" ]->setText(QString().asprintf("%04X", m_cpu->HL));
    m_regEdits["AF'"]->setText(QString().asprintf("%04X", m_cpu->A_F_));
    m_regEdits["BC'"]->setText(QString().asprintf("%04X", m_cpu->B_C_));
    m_regEdits["DE'"]->setText(QString().asprintf("%04X", m_cpu->D_E_));
    m_regEdits["HL'"]->setText(QString().asprintf("%04X", m_cpu->H_L_));
    m_regEdits["PC" ]->setText(QString().asprintf("%04X", m_cpu->PC));
    m_regEdits["SP" ]->setText(QString().asprintf("%04X", m_cpu->SP));
    m_regEdits["IX" ]->setText(QString().asprintf("%04X", m_cpu->IX));
    m_regEdits["IY" ]->setText(QString().asprintf("%04X", m_cpu->IY));
    m_regEdits["I"  ]->setText(QString().asprintf("%02X", m_cpu->I));
    m_regEdits["R"  ]->setText(QString().asprintf("%02X", m_cpu->R));
    m_regEdits["IM" ]->setText(QString("%1").arg(m_cpu->IM));
    m_regEdits["IFF1"]->setText(QString("%1").arg(m_cpu->IFF1));
    m_regEdits["IFF2"]->setText(QString("%1").arg(m_cpu->IFF2));
    m_regEdits["FLAGS"]->setText(m_cpu->flagString());
    m_regEdits["T"]->setText(QString::number(m_cpu->T));
}
