/**
 * stat_cards.cpp
 *
 * Four metric cards: clips loaded, synced count, avg confidence, total duration.
 */

#include "stat_cards.h"
#include "theme.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

namespace asynk {

StatCard::StatCard(const QString& title, const QString& value,
                   const QColor& accentColor, QWidget* parent)
    : QFrame(parent), m_accent(accentColor)
{
    setObjectName("card");
    setFixedHeight(72);
    setMinimumWidth(130);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 10, 12, 10);
    layout->setSpacing(4);

    m_titleLabel = new QLabel(title, this);
    m_titleLabel->setObjectName("dimLabel");
    QFont tf = theme::bodyFont();
    tf.setPointSize(8);
    tf.setWeight(QFont::Medium);
    m_titleLabel->setFont(tf);

    m_valueLabel = new QLabel(value, this);
    QFont vf = theme::headerFont();
    vf.setPointSize(16);
    vf.setWeight(QFont::Bold);
    m_valueLabel->setFont(vf);
    m_valueLabel->setStyleSheet(
        QString("color: %1; background: transparent;").arg(accentColor.name()));

    layout->addWidget(m_titleLabel);
    layout->addWidget(m_valueLabel);
}

void StatCard::setValue(const QString& value) {
    m_valueLabel->setText(value);
}

// ── StatCardRow ──

StatCardRow::StatCardRow(QWidget* parent) : QWidget(parent) {
    setStyleSheet("background: transparent;");

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);

    m_clipsCard      = new StatCard("Clips loaded",   "0");
    m_syncedCard     = new StatCard("Synced",          "0", theme::GREEN);
    m_confidenceCard = new StatCard("Avg confidence",  "--", theme::PURPLE);
    m_durationCard   = new StatCard("Total duration",  "00:00");

    layout->addWidget(m_clipsCard);
    layout->addWidget(m_syncedCard);
    layout->addWidget(m_confidenceCard);
    layout->addWidget(m_durationCard);
}

void StatCardRow::updateStats(int clipCount, int syncedCount,
                              double avgConfidence, double totalDurationSec)
{
    m_clipsCard->setValue(QString::number(clipCount));
    m_syncedCard->setValue(QString::number(syncedCount));

    if (avgConfidence > 0)
        m_confidenceCard->setValue(QString("%1%").arg(avgConfidence, 0, 'f', 1));
    else
        m_confidenceCard->setValue("--");

    int totalSec = static_cast<int>(totalDurationSec);
    int h = totalSec / 3600;
    int m = (totalSec % 3600) / 60;
    int s = totalSec % 60;

    if (h > 0)
        m_durationCard->setValue(QString("%1:%2:%3")
            .arg(h).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0')));
    else
        m_durationCard->setValue(QString("%1:%2")
            .arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0')));
}

void StatCardRow::reset() {
    updateStats(0, 0, 0.0, 0.0);
}

} // namespace asynk
