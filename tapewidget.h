#ifndef TAPEWIDGET_H
#define TAPEWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QListWidget>
#include "zxtape.h"

class TapeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TapeWidget(ZxTape *tape, QWidget *parent = nullptr);

    void open(QString filename);

    void updateState();

protected:
    virtual void showEvent(QShowEvent *e) override;

signals:

private:
    ZxTape *m_tape;
    QPushButton *m_openBtn;
    QPushButton *m_copyBtn;
    QLabel *m_tapeLabel;
    QPushButton *m_playBtn;
    QPushButton *m_stopBtn;
    QPushButton *m_recBtn;
    QPushButton *m_upBtn;
    QPushButton *m_downBtn;
    QPushButton *m_delBtn;
    QLabel *m_label;
    QProgressBar *m_progress;
    QListWidget *m_list;
    int m_oldState = 0;

    int m_curBlockOffset = 0;
    int m_curBlockLength = 0;

    struct TapHeader
    {
        uint8_t blockType; // 0x00 - header, 0xff - data
        uint8_t fileType; // 0x00 - Program, 0x01 - Numeric Array, 0x02 - Symbol Array, 0x03 - Bytes
        char m_name[10];
        uint16_t dataLength;
        uint16_t param; // autorun line or start address
        uint16_t programLength;
        //    uint8_t checksum;

        QString type() const
        {
            switch (fileType)
            {
            case 0x00: return "Program";
            case 0x01: return "NumArray";
            case 0x02: return "SymArray";
            case 0x03: return "Bytes";
            default: "Unknown";
            }
        }

        QString name() const
        {
            return QString::fromLatin1(m_name, 10);
        }

        QString toString() const
        {
            return QString("%1: %2 (len=%3)").arg(type()).arg(name()).arg(dataLength);
        }
    };

    void openTap(QString filename);
    void updateBlocks();
    void onStateChange();

    void activateBlock(int idx);
    void activateCurrentBlock();

    void swapBlocks(QListWidgetItem *item1, QListWidgetItem *item2);
    void raiseCurrentBlock();
    void lowerCurrentBlock();
    void deleteCurrentBlock();

};

#endif // TAPEWIDGET_H
