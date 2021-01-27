#ifndef ROOMWIDGET_H
#define ROOMWIDGET_H

#include <QWidget>
#include <QString>
#include <QTextBrowser>
#include <QListWidget>
#include <QListWidgetItem>
#include <QVector>
#include <tuple>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHostAddress>
#include <QGridLayout>
#include <QUdpSocket>
#include <QNetworkInterface>
#include <QCloseEvent>
#include <QNetworkDatagram>
#include <QUdpSocket>
#include <ctime>
#include <common.h>

class RoomWidget : public QWidget
{
    Q_OBJECT
//    friend class HomeWidget;
public:
    RoomWidget(QString roomName, QString roomID, QString userName,
               QHostAddress owner, QWidget *parent = nullptr);
    ~RoomWidget();

public:
    QString roomName;
    QString roomID;
    unsigned int numOfPeople = 0;

private:
    QString userName;
    QHostAddress owner;
    QHostAddress localAddr;
    QListWidget *peopleWidget = nullptr;
    QVector<QListWidgetItem*> personItems;
    QUdpSocket *udpSocket = nullptr;

    QTextBrowser *chatHistoryWidget = nullptr;
    QTextBrowser *inputWidget = nullptr;
    bool leave = false;

    void addPerson(QString name, QHostAddress addr);

protected:
    virtual void closeEvent(QCloseEvent *event);

private slots:
    void process();

};

#endif // ROOMWIDGET_H
