#ifndef CALIBRATIONPANEL_H
#define CALIBRATIONPANEL_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QPointF>

// Two-point calibration panel.
// User provides old JSON coords and clicks the corresponding
// locations on the real map image to get new coords.
// The panel computes scale+translate and emits applyCalibration.
class CalibrationPanel : public QWidget {
    Q_OBJECT
public:
    explicit CalibrationPanel(QWidget *parent = nullptr);

    // Called from outside to fill the "new" coordinate fields
    void setPickResult(int pairIndex, const QPointF &pos);
    // Fill old coordinate fields from building click
    void setOldCoords(int pairIndex, const QPointF &pos);

signals:
    void pickRequested(int pairIndex);
    void applyCalibration(QPointF old1, QPointF new1,
                          QPointF old2, QPointF new2);
    void exportRequested();

private:
    struct PairRow {
        QLabel *label;
        QLineEdit *oldX, *oldY;
        QLineEdit *newX, *newY;
        QPushButton *pickBtn;
    };
    PairRow m_rows[2];
    int m_pickingIndex = -1;
};

#endif
