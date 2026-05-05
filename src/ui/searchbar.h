#ifndef SEARCHBAR_H
#define SEARCHBAR_H

#include <QWidget>
#include <QLineEdit>
#include <QListWidget>
#include <QVBoxLayout>

struct SearchResult;

class SearchBar : public QWidget {
    Q_OBJECT
public:
    explicit SearchBar(QWidget *parent = nullptr);

    void setResults(const QVector<SearchResult> &results);
    void clearResults();

signals:
    void searchRequested(const QString &keyword);
    void resultSelected(int id);

private:
    QLineEdit *m_input;
    QListWidget *m_list;
};

#endif // SEARCHBAR_H