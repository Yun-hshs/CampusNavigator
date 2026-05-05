#include "logindialog.h"

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("登录"));
    setMinimumWidth(300);

    QVBoxLayout *layout = new QVBoxLayout(this);

    layout->addWidget(new QLabel(QStringLiteral("用户名:")));
    m_usernameInput = new QLineEdit(this);
    layout->addWidget(m_usernameInput);

    layout->addWidget(new QLabel(QStringLiteral("密码:")));
    m_passwordInput = new QLineEdit(this);
    m_passwordInput->setEchoMode(QLineEdit::Password);
    layout->addWidget(m_passwordInput);

    m_statusLabel = new QLabel(this);
    m_statusLabel->setStyleSheet("color: red;");
    layout->addWidget(m_statusLabel);

    m_loginBtn = new QPushButton(QStringLiteral("登录"), this);
    m_registerBtn = new QPushButton(QStringLiteral("注册"), this);
    layout->addWidget(m_loginBtn);
    layout->addWidget(m_registerBtn);

    connect(m_loginBtn, &QPushButton::clicked, this, [this]() {
        if (m_usernameInput->text().isEmpty() || m_passwordInput->text().isEmpty()) {
            m_statusLabel->setText(QStringLiteral("请输入用户名和密码"));
            return;
        }
        emit loginRequested(m_usernameInput->text(), m_passwordInput->text());
    });

    connect(m_registerBtn, &QPushButton::clicked, this, [this]() {
        if (m_usernameInput->text().isEmpty() || m_passwordInput->text().isEmpty()) {
            m_statusLabel->setText(QStringLiteral("请输入用户名和密码"));
            return;
        }
        emit registerRequested(m_usernameInput->text(), m_passwordInput->text());
    });
}