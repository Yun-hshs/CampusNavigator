#include "routepanel.h"

RoutePanel::RoutePanel(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 12, 10, 12);
    layout->setSpacing(8);

    // Title
    QLabel *title = new QLabel(QStringLiteral("路径规划"), this);
    QFont titleFont = title->font();
    titleFont.setBold(true);
    titleFont.setPointSize(13);
    title->setFont(titleFont);
    layout->addWidget(title);

    // Strategy selector
    QLabel *strategyLabel = new QLabel(QStringLiteral("规划策略:"), this);
    layout->addWidget(strategyLabel);
    m_strategyCombo = new QComboBox(this);
    m_strategyCombo->addItem(QStringLiteral("最短距离"), 0);
    m_strategyCombo->addItem(QStringLiteral("最短时间"), 1);
    layout->addWidget(m_strategyCombo);

    // Start node
    QLabel *startLabel = new QLabel(QStringLiteral("起点:"), this);
    layout->addWidget(startLabel);
    m_startCombo = new QComboBox(this);
    m_startCombo->setEditable(true);
    m_startCombo->setInsertPolicy(QComboBox::NoInsert);
    m_startCombo->lineEdit()->setPlaceholderText(QStringLiteral("选择或搜索起点..."));
    layout->addWidget(m_startCombo);

    // End node
    QLabel *endLabel = new QLabel(QStringLiteral("终点:"), this);
    layout->addWidget(endLabel);
    m_endCombo = new QComboBox(this);
    m_endCombo->setEditable(true);
    m_endCombo->setInsertPolicy(QComboBox::NoInsert);
    m_endCombo->lineEdit()->setPlaceholderText(QStringLiteral("选择或搜索终点..."));
    layout->addWidget(m_endCombo);

    // Search button
    m_searchBtn = new QPushButton(QStringLiteral("搜索路径"), this);
    m_searchBtn->setObjectName(QStringLiteral("routeSearchBtn"));
    m_searchBtn->setCursor(Qt::PointingHandCursor);
    layout->addWidget(m_searchBtn);

    // Result area
    QLabel *resultLabel = new QLabel(QStringLiteral("路线结果:"), this);
    layout->addWidget(resultLabel);
    m_resultText = new QTextEdit(this);
    m_resultText->setReadOnly(true);
    m_resultText->setMaximumHeight(140);
    m_resultText->setPlaceholderText(QStringLiteral("选择起终点后点击搜索..."));
    layout->addWidget(m_resultText, 1);

    layout->addStretch();

    // Signal
    connect(m_searchBtn, &QPushButton::clicked, this, [this]() {
        int startId = m_startCombo->currentData().toInt();
        int endId = m_endCombo->currentData().toInt();
        int strategy = m_strategyCombo->currentData().toInt();
        if (startId > 0 && endId > 0) {
            emit searchRouteClicked(startId, endId, strategy);
        }
    });

    // Style
    setStyleSheet(QStringLiteral(
        "RoutePanel { background: #FAFBFC; }"
        "QLabel { color: #444; font-size: 12px; }"
        "QComboBox {"
        "  border: 1px solid #D0D3D6; border-radius: 5px;"
        "  padding: 5px 8px; background: #FFFFFF; font-size: 12px;"
        "}"
        "QComboBox:focus { border-color: #7B5EA7; }"
        "QComboBox::drop-down { border: none; width: 20px; }"
        "#routeSearchBtn {"
        "  background: #7B5EA7; color: #FFFFFF;"
        "  border: none; border-radius: 6px;"
        "  padding: 9px 0; font-size: 13px; font-weight: bold;"
        "}"
        "#routeSearchBtn:hover { background: #8E6FBA; }"
        "#routeSearchBtn:pressed { background: #6A4F95; }"
        "QTextEdit {"
        "  border: 1px solid #DDE0E2; border-radius: 5px;"
        "  background: #FFFFFF; font-size: 12px; color: #333;"
        "  padding: 6px;"
        "}"
    ));
}

void RoutePanel::setNodes(const QVector<QPair<int, QString>> &items)
{
    m_startCombo->clear();
    m_endCombo->clear();
    for (const auto &pair : items) {
        m_startCombo->addItem(pair.second, pair.first);
        m_endCombo->addItem(pair.second, pair.first);
    }
}

int RoutePanel::startNodeId() const { return m_startCombo->currentData().toInt(); }
int RoutePanel::endNodeId() const { return m_endCombo->currentData().toInt(); }

void RoutePanel::setResultText(const QString &text)
{
    m_resultText->setHtml(text);
}

void RoutePanel::clearResult()
{
    m_resultText->clear();
}
