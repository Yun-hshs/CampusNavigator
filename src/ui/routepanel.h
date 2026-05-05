#ifndef ROUTEPANEL_H
#define ROUTEPANEL_H

#include <QWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QLabel>

class RoutePanel : public QWidget {
    Q_OBJECT
public:
    explicit RoutePanel(QWidget *parent = nullptr);

    void setNodes(const QVector<QPair<int, QString>> &items);
    int startNodeId() const;
    int endNodeId() const;
    void setResultText(const QString &text);
    void clearResult();

signals:
    void searchRouteClicked(int startNodeId, int endNodeId, int strategy);

private:
    QComboBox *m_startCombo;
    QComboBox *m_endCombo;
    QComboBox *m_strategyCombo;
    QPushButton *m_searchBtn;
    QTextEdit *m_resultText;
};

#endif // ROUTEPANEL_H
