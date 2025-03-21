#include "cpuwidget.h"
#include <QFormLayout>
#include <QFontDatabase>

CpuWidget::CpuWidget(QWidget *parent) : QGroupBox("CPU", parent)
{
    setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    setStyleSheet("font-family: 'Consolas';");

    QFormLayout *reglay = new QFormLayout;
    QStringList regNames = {"FLAGS", "AF", "BC", "DE", "HL", "AF'", "BC'", "DE'", "HL'",
                            "PC", "SP", "IX", "IY", "I", "R", "IM", "IFF1", "IFF2", "T"};
    for (const QString &name: regNames)
    {
        QLineEdit *edit = new QLineEdit;
        edit->setAlignment(Qt::AlignRight);
        edit->setFixedWidth(80);
        edit->setReadOnly(true);
        m_regEdits[name] = edit;
        reglay->addRow(name, edit);
    }

    m_bkptEdit = new QLineEdit("0000");
    m_bkptEdit->setAlignment(Qt::AlignRight);
    m_bkptEdit->setFixedWidth(48);
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
