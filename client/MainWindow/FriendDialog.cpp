#include "FriendDialog.h"
#include "./ui_FriendDialog.h"
#include <QDebug>

FriendDialog::FriendDialog(const QString &username, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FriendDialog)
    , username(username)
{
    ui->setupUi(this);

    connect(ui->friendList, &QListWidget::itemClicked,
            this, [this](QListWidgetItem *item)
            {
                emit friendSelected(item->text());
            });
    connect(ui->searchButton, &QPushButton::clicked,
            this, [this]()
            {
                QString keyword = ui->searchInput->text();

                if (keyword.trimmed().isEmpty())
                {
                    return;
                }

                emit searchRequested(keyword);
            });
}

FriendDialog::~FriendDialog()
{
    delete ui;
}

void FriendDialog::setFriends(const QStringList &friends)
{
    ui->friendList->clear();

    for (const QString &friendName : friends)
    {
        ui->friendList->addItem(friendName);
    }
}
void FriendDialog::setSearchResults(const QStringList &results)
{
    ui->friendList->clear();

    for (const QString &username : results)
    {
        ui->friendList->addItem(username);
    }
}