#include "routedialog.h"

RouteDialog::RouteDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("路径规划"));
    setMinimumWidth(350);

    QVBoxLayout *layout = new QVBoxLayout(this);

    layout->addWidget(new QLabel(QStringLiteral("起点:")));
    m_startCombo = new QComboBox(this);
    layout->addWidget(m_startCombo);

    layout->addWidget(new QLabel(QStringLiteral("终点:")));
    m_endCombo = new QComboBox(this);
    layout->addWidget(m_endCombo);

    layout->addWidget(new QLabel(QStringLiteral("策略:")));
    m_strategyCombo = new QComboBox(this);
    m_strategyCombo->addItem(QStringLiteral("最短距离 (Dijkstra)"));
    m_strategyCombo->addItem(QStringLiteral("最短时间 (A*)"));
    layout->addWidget(m_strategyCombo);

    m_infoLabel = new QLabel(this);
    m_infoLabel->setWordWrap(true);
    layout->addWidget(m_infoLabel);

    m_planBtn = new QPushButton(QStringLiteral("规划路线"), this);
    layout->addWidget(m_planBtn);

    connect(m_planBtn, &QPushButton::clicked, this, [this]() {
        if (m_startCombo->currentData().isValid() && m_endCombo->currentData().isValid()) {
            emit planRouteRequested(m_startCombo->currentData().toInt(),
                                     m_endCombo->currentData().toInt(),
                                     m_strategyCombo->currentIndex());
        }
    });
}

void RouteDialog::setStartPoint(const QString &name, int nodeId)
{
    int idx = m_startCombo->findData(nodeId);
    if (idx >= 0) m_startCombo->setCurrentIndex(idx);
}

void RouteDialog::setEndPoint(const QString &name, int nodeId)
{
    int idx = m_endCombo->findData(nodeId);
    if (idx >= 0) m_endCombo->setCurrentIndex(idx);
}

void RouteDialog::setRouteInfo(const QString &description)
{
    m_infoLabel->setText(description);
}

int RouteDialog::startNodeId() const { return m_startCombo->currentData().toInt(); }
int RouteDialog::endNodeId() const { return m_endCombo->currentData().toInt(); }
int RouteDialog::strategyIndex() const { return m_strategyCombo->currentIndex(); }

void RouteDialog::addNodeItem(const QString &name, int nodeId)
{
    m_startCombo->addItem(name, nodeId);
    m_endCombo->addItem(name, nodeId);
}

void RouteDialog::clearNodeItems()
{
    m_startCombo->clear();
    m_endCombo->clear();
}