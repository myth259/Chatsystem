#ifndef FRIENDDIALOG_H
#define FRIENDDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
class FriendDialog;
}
QT_END_NAMESPACE

class FriendDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FriendDialog(const QString &username, QWidget *parent = nullptr);
    ~FriendDialog() override;

    void setFriends(const QStringList &friends);
    void setSearchResults(const QStringList &results);

signals:
    void friendSelected(const QString &friendName);
    void searchRequested(const QString &keyword);
    void addFriendRequested(const QString &friendName);

private:
    Ui::FriendDialog *ui;
    QString username;
};

#endif // FRIENDDIALOG_H