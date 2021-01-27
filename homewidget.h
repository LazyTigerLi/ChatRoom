#ifndef HOMEWIDGET_H
#define HOMEWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QListWidget>
#include <QListWidgetItem>
#include <QUdpSocket>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QVector>
#include <QNetworkDatagram>
#include <QVector>
#include <QPair>
#include <QInputDialog>
#include <QNetworkInterface>
#include <QMap>
#include <QCloseEvent>
#include <string>
#include <ctime>
#include "roomwidget.h"
#include "common.h"

class HomeWidget : public QWidget
{
    Q_OBJECT

public:
    HomeWidget(QWidget *parent = nullptr);
    ~HomeWidget();

private:
    QPushButton *createRoomButton = nullptr;
    QPushButton *quitButton = nullptr;
    QLabel *peopleOnlineLabel = nullptr;
    QLabel *numOfPeopleOnlineLabel = nullptr;
    QLabel *userNameLabel = nullptr;
    QLabel *userName = nullptr;
    QTimer *timer = nullptr;
    QListWidget *roomsListWidget = nullptr;
    QUdpSocket *udpSocket = nullptr;
    QMap<QString, QListWidgetItem*> roomItems;
    QMap<QString, RoomWidget*> roomsEntered;
    QMap<QString, RoomWidget*> roomsOwned;
    QHostAddress localAddr;

    int numOfPeopleOnline = -1;
    int delay = 200;

private slots:
    void createRoom();
    void process();
    void setNumOfPeopleOnline();
    void enterRoom(QListWidgetItem *item);

protected:
    virtual void closeEvent(QCloseEvent *event);

};
#endif // HOMEWIDGET_H
