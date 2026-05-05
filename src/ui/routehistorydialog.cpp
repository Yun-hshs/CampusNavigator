#include "routehistorydialog.h"
#include "userdao.h"

#include <QHeaderView>
#include <QJsonDocument>
#include <QJsonObject>
#include <QHBoxLayout>

RouteHistoryDialog::RouteHistoryDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("路线历史"));
    resize(650, 400);

    QVBoxLayout *layout = new QVBoxLayout(this);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(5);
    m_table->setHorizontalHeaderLabels({
        QStringLiteral("时间"),
        QStringLiteral("起点"),
        QStringLiteral("终点"),
        QStringLiteral("距离"),
        QStringLiteral("详情")
    });
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layout->addWidget(m_table);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    m_loadBtn = new QPushButton(QStringLiteral("加载选中路线"), this);
    m_closeBtn = new QPushButton(QStringLiteral("关闭"), this);
    btnLayout->addStretch();
    btnLayout->addWidget(m_loadBtn);
    btnLayout->addWidget(m_closeBtn);
    layout->addLayout(btnLayout);

    connect(m_closeBtn, &QPushButton::clicked, this, &QDialog::close);
    connect(m_loadBtn, &QPushButton::clicked, this, [this]() {
        int row = m_table->currentRow();
        if (row < 0 || row >= m_records.size()) return;
        const auto &rec = m_records[row];
        emit routeSelected(rec.fromNode, rec.toNode, rec.routeJson);
        accept();
    });
}

void RouteHistoryDialog::loadRecords(const QVector<RouteHistoryRecord> &records)
{
    m_records = records;
    m_table->setRowCount(records.size());

    for (int i = 0; i < records.size(); ++i) {
        const auto &rec = records[i];
        m_table->setItem(i, 0, new QTableWidgetItem(rec.timestamp));
        m_table->setItem(i, 1, new QTableWidgetItem(
            QStringLiteral("节点 %1").arg(rec.fromNode)));
        m_table->setItem(i, 2, new QTableWidgetItem(
            QStringLiteral("节点 %1").arg(rec.toNode)));

        QJsonDocument doc = QJsonDocument::fromJson(rec.routeJson.toUtf8());
        if (doc.isObject()) {
            QJsonObject json = doc.object();
            double dist = json["totalDistance"].toDouble();
            m_table->setItem(i, 3, new QTableWidgetItem(
                QStringLiteral("%1 米").arg(int(dist))));
            m_table->setItem(i, 4, new QTableWidgetItem(
                json["description"].toString()));
        } else {
            m_table->setItem(i, 3, new QTableWidgetItem("-"));
            m_table->setItem(i, 4, new QTableWidgetItem(rec.routeJson));
        }
    }

    m_table->resizeColumnsToContents();
}
