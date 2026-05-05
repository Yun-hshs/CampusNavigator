#include "calibrationpanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QMessageBox>

CalibrationPanel::CalibrationPanel(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 12, 10, 12);
    mainLayout->setSpacing(10);

    QLabel *title = new QLabel(QStringLiteral("坐标校准"), this);
    QFont f = title->font();
    f.setBold(true);
    f.setPointSize(13);
    title->setFont(f);
    mainLayout->addWidget(title);

    QLabel *hint = new QLabel(
        QStringLiteral("输入2个参考点的旧坐标(JSON)和新坐标(地图):\n"
                       "点击「拾取」后在地图上左键点击目标位置。"),
        this);
    hint->setWordWrap(true);
    hint->setStyleSheet("color:#666; font-size:11px;");
    mainLayout->addWidget(hint);

    // Pre-fill old coords with two far-apart known buildings
    const char *labels[2] = {"参考点 A (如西大门)", "参考点 B (如东大门)"};
    const double oldDefaults[2][2] = {{50, 550}, {950, 500}};

    for (int i = 0; i < 2; ++i) {
        QGroupBox *gb = new QGroupBox(QString::fromUtf8(labels[i]), this);
        QFormLayout *fl = new QFormLayout(gb);

        QHBoxLayout *oldRow = new QHBoxLayout;
        m_rows[i].oldX = new QLineEdit(this);
        m_rows[i].oldX->setPlaceholderText("old X");
        m_rows[i].oldX->setText(QString::number(oldDefaults[i][0], 'f', 0));
        m_rows[i].oldX->setFixedWidth(70);
        m_rows[i].oldY = new QLineEdit(this);
        m_rows[i].oldY->setPlaceholderText("old Y");
        m_rows[i].oldY->setText(QString::number(oldDefaults[i][1], 'f', 0));
        m_rows[i].oldY->setFixedWidth(70);
        oldRow->addWidget(new QLabel("旧:", this));
        oldRow->addWidget(m_rows[i].oldX);
        oldRow->addWidget(m_rows[i].oldY);
        oldRow->addStretch();
        fl->addRow(oldRow);

        QHBoxLayout *newRow = new QHBoxLayout;
        m_rows[i].newX = new QLineEdit(this);
        m_rows[i].newX->setPlaceholderText("new X");
        m_rows[i].newX->setFixedWidth(70);
        m_rows[i].newY = new QLineEdit(this);
        m_rows[i].newY->setPlaceholderText("new Y");
        m_rows[i].newY->setFixedWidth(70);
        m_rows[i].pickBtn = new QPushButton(QStringLiteral("拾取"), this);
        m_rows[i].pickBtn->setFixedWidth(55);
        m_rows[i].pickBtn->setCursor(Qt::PointingHandCursor);
        m_rows[i].pickBtn->setStyleSheet(
            "QPushButton { background:#7B5EA7; color:#FFF; border:none;"
            "border-radius:4px; padding:4px 0; }"
            "QPushButton:hover { background:#8E6FBA; }");

        int idx = i;
        connect(m_rows[i].pickBtn, &QPushButton::clicked, this, [this, idx]() {
            m_pickingIndex = idx;
            m_rows[idx].pickBtn->setText(QStringLiteral("拾取中..."));
            m_rows[idx].pickBtn->setStyleSheet(
                "QPushButton { background:#E88040; color:#FFF; border:none;"
                "border-radius:4px; padding:4px 0; }");
            emit pickRequested(idx);
        });

        newRow->addWidget(new QLabel("新:", this));
        newRow->addWidget(m_rows[i].newX);
        newRow->addWidget(m_rows[i].newY);
        newRow->addWidget(m_rows[i].pickBtn);
        newRow->addStretch();
        fl->addRow(newRow);

        mainLayout->addWidget(gb);
    }

    // Buttons
    QHBoxLayout *btnRow = new QHBoxLayout;
    QPushButton *applyBtn = new QPushButton(QStringLiteral("应用变换"), this);
    applyBtn->setCursor(Qt::PointingHandCursor);
    applyBtn->setStyleSheet(
        "QPushButton { background:#5B9A41; color:#FFF; border:none;"
        "border-radius:6px; padding:10px 0; font-size:13px; font-weight:bold; }"
        "QPushButton:hover { background:#6BAD4E; }");

    QPushButton *exportBtn = new QPushButton(QStringLiteral("导出 JSON"), this);
    exportBtn->setCursor(Qt::PointingHandCursor);
    exportBtn->setStyleSheet(
        "QPushButton { background:#4A6FA5; color:#FFF; border:none;"
        "border-radius:6px; padding:10px 0; font-size:13px; }"
        "QPushButton:hover { background:#5A80B5; }");

    btnRow->addWidget(applyBtn);
    btnRow->addWidget(exportBtn);
    mainLayout->addLayout(btnRow);
    mainLayout->addStretch();

    // Connect apply
    connect(applyBtn, &QPushButton::clicked, this, [this]() {
        bool ok1, ok2, ok3, ok4;
        QPointF old1(m_rows[0].oldX->text().toDouble(&ok1),
                     m_rows[0].oldY->text().toDouble(&ok2));
        QPointF new1(m_rows[0].newX->text().toDouble(&ok3),
                     m_rows[0].newY->text().toDouble(&ok4));
        if (!ok1 || !ok2 || !ok3 || !ok4) {
            QMessageBox::warning(this, "输入错误", "参考点 A 坐标无效");
            return;
        }
        QPointF old2(m_rows[1].oldX->text().toDouble(&ok1),
                     m_rows[1].oldY->text().toDouble(&ok2));
        QPointF new2(m_rows[1].newX->text().toDouble(&ok3),
                     m_rows[1].newY->text().toDouble(&ok4));
        if (!ok1 || !ok2 || !ok3 || !ok4) {
            QMessageBox::warning(this, "输入错误", "参考点 B 坐标无效");
            return;
        }
        emit applyCalibration(old1, new1, old2, new2);
    });

    connect(exportBtn, &QPushButton::clicked, this, [this]() {
        emit exportRequested();
    });

    setMinimumWidth(250);
}

void CalibrationPanel::setPickResult(int pairIndex, const QPointF &pos)
{
    if (pairIndex < 0 || pairIndex > 1) return;
    m_rows[pairIndex].newX->setText(QString::number(int(pos.x())));
    m_rows[pairIndex].newY->setText(QString::number(int(pos.y())));
    m_rows[pairIndex].pickBtn->setText(QStringLiteral("拾取"));
    m_rows[pairIndex].pickBtn->setStyleSheet(
        "QPushButton { background:#7B5EA7; color:#FFF; border:none;"
        "border-radius:4px; padding:4px 0; }"
        "QPushButton:hover { background:#8E6FBA; }");
    m_pickingIndex = -1;
}

void CalibrationPanel::setOldCoords(int pairIndex, const QPointF &pos)
{
    if (pairIndex < 0 || pairIndex > 1) return;
    m_rows[pairIndex].oldX->setText(QString::number(int(pos.x())));
    m_rows[pairIndex].oldY->setText(QString::number(int(pos.y())));
}
