#ifndef ROUTEHISTORYDIALOG_H
#define ROUTEHISTORYDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QVector>
#include "userdao.h"

class RouteHistoryDialog : public QDialog {
    Q_OBJECT
public:
    explicit RouteHistoryDialog(QWidget *parent = nullptr);

    void loadRecords(const QVector<RouteHistoryRecord> &records);

signals:
    void routeSelected(int startNode, int endNode, const QString &routeJson);

private:
    QTableWidget *m_table;
    QPushButton *m_loadBtn;
    QPushButton *m_closeBtn;
    QVector<RouteHistoryRecord> m_records;
};

#endif // ROUTEHISTORYDIALOG_H
