#include "searchbar.h"
#include "searchcontroller.h"
#include <QTimer>
#include <QDebug>

SearchBar::SearchBar(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(2);

    m_input = new QLineEdit(this);
    m_input->setPlaceholderText(QStringLiteral("搜索建筑、地点..."));
    layout->addWidget(m_input);

    m_list = new QListWidget(this);
    m_list->setMaximumHeight(200);
    m_list->hide();
    layout->addWidget(m_list);

    // Auto-search with debounce
    QTimer *debounceTimer = new QTimer(this);
    debounceTimer->setSingleShot(true);
    debounceTimer->setInterval(300);
    connect(m_input, &QLineEdit::textChanged, this, [debounceTimer](const QString &) {
        debounceTimer->start();
    });
    connect(debounceTimer, &QTimer::timeout, this, [this]() {
        if (!m_input->text().isEmpty()) {
            emit searchRequested(m_input->text());
        } else {
            clearResults();
        }
    });

    connect(m_list, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        int id = item->data(Qt::UserRole).toInt();
        emit resultSelected(id);
        m_list->hide();
    });
}

void SearchBar::setResults(const QVector<SearchResult> &results)
{
    m_list->clear();
    for (const SearchResult &r : results) {
        QString text = r.name + " (" + r.type + ")";
        QListWidgetItem *item = new QListWidgetItem(text, m_list);
        item->setData(Qt::UserRole, r.id);
    }
    m_list->setVisible(!results.isEmpty());
}

void SearchBar::clearResults()
{
    m_list->clear();
    m_list->hide();
}