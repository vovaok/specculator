#include "tapewidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QFileDialog>

TapeWidget::TapeWidget(QWidget *parent)
    : QWidget{parent}
{
    setStyleSheet("QListWidget {min-width: 16em;} QPushButton {max-width: 2em; max-height: 2em;}");

    m_openBtn = new QPushButton("Open");
    m_copyBtn = new QPushButton("Copy");
    m_tapeLabel = new QLabel();

    m_list = new QListWidget;
    m_label = new QLabel;
    m_progress = new QProgressBar;

    m_playBtn = new QPushButton(">");
    m_stopBtn = new QPushButton("[]");
    m_recBtn = new QPushButton("o");
    m_upBtn = new QPushButton("^");
    m_downBtn = new QPushButton("v");
    m_delBtn = new QPushButton("x");

    QHBoxLayout *tapelay = new QHBoxLayout;
    tapelay->addWidget(m_tapeLabel);
    tapelay->addWidget(m_openBtn);
    tapelay->addWidget(m_copyBtn);

    QHBoxLayout *btnlay = new QHBoxLayout;
    btnlay->addWidget(m_stopBtn);
    btnlay->addWidget(m_playBtn);
    btnlay->addWidget(m_recBtn);
    btnlay->addWidget(m_upBtn);
    btnlay->addWidget(m_downBtn);
    btnlay->addWidget(m_delBtn);

    QVBoxLayout *lay = new QVBoxLayout;
    setLayout(lay);
    lay->addLayout(tapelay);
    lay->addLayout(btnlay);
    lay->addWidget(m_label);
    lay->addWidget(m_progress);
    lay->addWidget(m_list);

    connect(m_openBtn, &QPushButton::clicked, this, [this](){
        QString filename = QFileDialog::getOpenFileName(nullptr, "Open tape", QString(), "Tape (*.tap, *.TAP)");
        if (QFile::exists(filename))
            openTap(filename);
    });

    connect(m_copyBtn, &QPushButton::clicked, this, [this](){
        if (!m_tape)
            return;
        QString filename = QFileDialog::getSaveFileName(nullptr, "Copy tape", QString(), "Tape (*.tap, *.TAP)");
        m_tape->m_filename = filename;
        m_tape->saveTap();
        openTap(filename);
    });

    connect(m_stopBtn, &QPushButton::clicked, this, [this](){if (m_tape) m_tape->stop();});
    connect(m_playBtn, &QPushButton::clicked, this, [this](){if (m_tape) m_tape->play();});
    connect(m_recBtn, &QPushButton::clicked, this, [this](){if (m_tape) m_tape->rec();});
    connect(m_upBtn, &QPushButton::clicked, this, &TapeWidget::raiseCurrentBlock);
    connect(m_downBtn, &QPushButton::clicked, this, &TapeWidget::lowerCurrentBlock);
    connect(m_delBtn, &QPushButton::clicked, this, &TapeWidget::deleteCurrentBlock);

    connect(m_list, &QListWidget::currentRowChanged, this, &TapeWidget::activateCurrentBlock);
}

void TapeWidget::bindTape(ZxTape *tape)
{
    m_tape = tape;

    QSettings sets;
    open(sets.value("tapeFile", "cassette.TAP").toString());
}

void TapeWidget::open(QString filename)
{
    if (filename.endsWith(".TAP", Qt::CaseInsensitive))
        openTap(filename);

//    activateBlock(0);
}

void TapeWidget::openTap(QString filename)
{
    if (!m_tape)
        return;
    m_tape->openTap(filename);
    updateBlocks();
    m_tapeLabel->setText("Tape: " + QFileInfo(filename).baseName());
    QSettings sets;
    sets.setValue("tapeFile", filename);
}

void TapeWidget::updateState()
{
    if (!m_tape)
        return;
    if (!isVisible())
        return;

    if (m_tape->isChanged())
        updateBlocks();

    int state = m_tape->isPlaying()? 1: m_tape->isRecording()? 2: 0;
    if (state != m_oldState)
    {
        onStateChange();
        m_oldState = state;
    }

    if (m_tape->isPlaying())
    {
        m_progress->setValue(m_tape->curOffset() - m_curBlockOffset);
    }
    else if (m_tape->isRecording())
    {
        m_progress->setValue(m_tape->curOffset() - m_curBlockOffset);
    }
}

void TapeWidget::showEvent(QShowEvent *)
{
//    updateState();
    updateBlocks();
}

void TapeWidget::updateBlocks()
{
    if (!m_tape)
        return;

    int tapeOffset = m_tape->m_ptr? m_tape->curOffset(): 0;

    m_list->clear();
    if (m_tape->m_buffer.isEmpty())
        return;

    QListWidgetItem *curItem = nullptr;

    const uint8_t *begin = m_tape->begin();
    const uint8_t *end = begin + m_tape->m_buffer.size();
    const uint8_t *ptr = begin;
    while (ptr < end)
    {
        int blockOffset = ptr - begin;
        uint16_t len = *reinterpret_cast<const uint16_t *>(ptr);
        ptr += 2;
        QString text;
        if (*ptr == 0x00 && len == 19)
        {
            const TapHeader *hdr = reinterpret_cast<const TapHeader*>(ptr);
            ptr += len;
            len = *reinterpret_cast<const uint16_t *>(ptr);
            if (len == hdr->dataLength + 2) // eat next block if we're parsing its header now
                ptr += len + 2;
            text = hdr->toString();
        }
        else
        {
            ptr += len;
            text = QString("Block (len=%1)").arg(len);
        }
        int blockLen = (ptr - begin) - blockOffset;

        QListWidgetItem *item = new QListWidgetItem(/*icon,*/ text, m_list);
        item->setData(Qt::UserRole, blockOffset);
        item->setData(Qt::UserRole + 1, blockLen);
        if (tapeOffset >= blockOffset && tapeOffset < blockOffset + blockLen)
            curItem = item;
    }

    if (curItem)
        m_list->setCurrentItem(curItem);
    else
        m_list->setCurrentRow(0);

    if (m_tape->isRecording())
    {
        QListWidgetItem *item = m_list->item(m_list->count() - 1);
        if (item)
        {
            int off = item->data(Qt::UserRole).toInt();
            int len = item->data(Qt::UserRole + 1).toInt();
            const TapHeader *hdr = reinterpret_cast<const TapHeader *>(m_tape->begin() + off + 2);
            if (hdr->blockType == 0x00 && len == 21)
            {
                m_label->setText(item->text());
                m_curBlockOffset = off;
                m_curBlockLength = hdr->dataLength + 21;
                m_progress->setMaximum(m_curBlockLength);
            }
            else
            {
                m_label->setText("saving...");
            }
        }
        else
        {
            m_label->setText("saving...");
            m_progress->setValue(0);
        }
    }
}

void TapeWidget::onStateChange()
{
    if (!m_tape)
        return;

    bool stopped = m_tape->isStopped();
    m_stopBtn->setDown(stopped);
    m_playBtn->setDown(m_tape->isPlaying());
    m_recBtn->setDown(m_tape->isRecording());

    m_playBtn->setDisabled(m_tape->isRecording());
    m_recBtn->setDisabled(m_tape->isPlaying());
    m_upBtn->setEnabled(stopped);
    m_downBtn->setEnabled(stopped);
    m_delBtn->setEnabled(stopped);
    m_list->setEnabled(stopped);
    updateBlocks();
}

void TapeWidget::activateBlock(int idx)
{
    m_list->setCurrentRow(idx);
    activateCurrentBlock();
}

void TapeWidget::activateCurrentBlock()
{
    if (!m_tape)
        return;

    QListWidgetItem *item = m_list->currentItem();
    if (item)
    {
        m_curBlockOffset = item->data(Qt::UserRole).toInt();
        m_curBlockLength = item->data(Qt::UserRole + 1).toInt();
        if (m_tape->isStopped())
            m_tape->m_ptr = m_tape->begin() + m_curBlockOffset;
        m_progress->setMaximum(m_curBlockLength);
        m_progress->setValue(0);
        m_label->setText(item->text());
    }
    else
    {
        m_curBlockOffset = 0;
        m_curBlockLength = 0;
        if (m_tape->isStopped())
            m_tape->m_ptr = nullptr;
        m_progress->setValue(0);
        m_label->setText("");
    }
}

void TapeWidget::swapBlocks(QListWidgetItem *item1, QListWidgetItem *item2)
{
    if (!m_tape)
        return;

    int off1 = item1->data(Qt::UserRole).toInt();
    int len1 = item1->data(Qt::UserRole + 1).toInt();
    int off2 = item2->data(Qt::UserRole).toInt();
    int len2 = item2->data(Qt::UserRole + 1).toInt();

    if (off1 > off2)
    {
        std::swap(off1, off2);
        std::swap(len1, len2);
    }

    QByteArray buf1 = m_tape->m_buffer.mid(off1, len1);
    QByteArray buf2 = m_tape->m_buffer.mid(off2, len2);
    m_tape->m_buffer.remove(off1, len1 + len2);
    m_tape->m_buffer.insert(off1, buf1);
    m_tape->m_buffer.insert(off1, buf2);

    updateBlocks();
}

void TapeWidget::raiseCurrentBlock()
{
    if (!m_tape)
        return;

    if (!m_tape->isStopped())
        return;
    int idx = m_list->currentRow();
    if (idx < 1)
        return;
    QListWidgetItem *item1 = m_list->item(idx - 1);
    QListWidgetItem *item2 = m_list->item(idx);
    swapBlocks(item1, item2);
    activateBlock(idx - 1);
    m_tape->saveTap();
}

void TapeWidget::lowerCurrentBlock()
{
    if (!m_tape)
        return;

    if (!m_tape->isStopped())
        return;
    int idx = m_list->currentRow();
    if (idx < 0 || idx > m_list->count() - 2)
        return;
    QListWidgetItem *item1 = m_list->item(idx);
    QListWidgetItem *item2 = m_list->item(idx + 1);
    swapBlocks(item1, item2);
    activateBlock(idx + 1);
    m_tape->saveTap();
}

void TapeWidget::deleteCurrentBlock()
{
    if (!m_tape)
        return;

    if (!m_tape->isStopped())
        return;
    QListWidgetItem *item = m_list->currentItem();
    if (!item)
        return;
    if (QMessageBox::Yes != QMessageBox::question(0L, "Delete block", QString("Delete %1?").arg(item->text())))
        return;

    int blockOffset = item->data(Qt::UserRole).toInt();
    int blockLen = item->data(Qt::UserRole + 1).toInt();
    m_tape->m_buffer.remove(blockOffset, blockLen);
    m_tape->saveTap();
    updateBlocks();
}

