#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>

class LoginDialog : public QDialog {
    Q_OBJECT
public:
    explicit LoginDialog(QWidget *parent = nullptr);

signals:
    void loginRequested(const QString &username, const QString &password);
    void registerRequested(const QString &username, const QString &password);

private:
    QLineEdit *m_usernameInput;
    QLineEdit *m_passwordInput;
    QPushButton *m_loginBtn;
    QPushButton *m_registerBtn;
    QLabel *m_statusLabel;
};

#endif // LOGINDIALOG_H