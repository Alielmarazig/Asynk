#pragma once
#include <QWidget>
#include <QLabel>
#include <QColor>

namespace asynk {

class StatCard : public QFrame {
    Q_OBJECT
public:
    explicit StatCard(const QString& title, const QString& value,
                      const QColor& accentColor = QColor("#e8e8f0"),
                      QWidget* parent = nullptr);

    void setValue(const QString& value);

private:
    QLabel* m_titleLabel;
    QLabel* m_valueLabel;
    QColor  m_accent;
};

class StatCardRow : public QWidget {
    Q_OBJECT
public:
    explicit StatCardRow(QWidget* parent = nullptr);

    void updateStats(int clipCount, int syncedCount,
                     double avgConfidence, double totalDurationSec);
    void reset();

private:
    StatCard* m_clipsCard;
    StatCard* m_syncedCard;
    StatCard* m_confidenceCard;
    StatCard* m_durationCard;
};

} // namespace asynk
