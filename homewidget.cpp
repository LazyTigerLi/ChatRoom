#include "homewidget.h"

HomeWidget::HomeWidget(QWidget *parent)
    : QWidget(parent)
{
    roomsListWidget = new QListWidget(this);
    createRoomButton = new QPushButton(tr("create"), this);
    quitButton = new QPushButton(tr("quit"), this);
    peopleOnlineLabel = new QLabel(tr("people online"), this);
    numOfPeopleOnlineLabel = new QLabel(this);
    userNameLabel = new QLabel(tr("username"), this);
    userName = new QLabel(this);
    connect(createRoomButton, &QPushButton::clicked, this, &HomeWidget::createRoom);
    connect(quitButton, &QPushButton::clicked, this, &HomeWidget::close);
    connect(roomsListWidget, &QListWidget::itemDoubleClicked, this, &HomeWidget::enterRoom);

    QGridLayout *upperLayout = new QGridLayout;
    upperLayout->addWidget(peopleOnlineLabel, 0, 0);
    upperLayout->addWidget(numOfPeopleOnlineLabel, 0, 1);
    upperLayout->addWidget(userNameLabel, 1, 0);
    upperLayout->addWidget(userName, 1, 1);
    QHBoxLayout *lowerLayout = new QHBoxLayout;
    lowerLayout->addWidget(createRoomButton);
    lowerLayout->addWidget(quitButton);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(upperLayout);
    mainLayout->addWidget(roomsListWidget);
    mainLayout->addLayout(lowerLayout);

    foreach (const QHostAddress &address, QNetworkInterface::allAddresses())
    {
        if (address.protocol() == QUdpSocket::IPv4Protocol
                && address != QHostAddress::LocalHost)
        {
            localAddr = address;
            break;
        }
    }
    timer = new QTimer;
    timer->setInterval(delay);
    connect(timer, &QTimer::timeout, this, &HomeWidget::setNumOfPeopleOnline);
    udpSocket = new QUdpSocket(this);
    udpSocket->bind(port, QUdpSocket::ShareAddress);
    connect(udpSocket, &QUdpSocket::readyRead, this, &HomeWidget::process);

    MessageType msgType = MessageType::Login;
    udpSocket->writeDatagram(reinterpret_cast<const char*>(&msgType), sizeof(MessageType),
                             QHostAddress::Broadcast, port);
    timer->start();
}

HomeWidget::~HomeWidget()
{
}

void HomeWidget::process()
{
    while (udpSocket->hasPendingDatagrams())
    {
        QNetworkDatagram datagram = udpSocket->receiveDatagram();
        QByteArray recvData = datagram.data();
        QHostAddress addr = datagram.senderAddress();
        MessageType msgType;
        memcpy(&msgType, recvData.data(), sizeof(MessageType));
        switch (msgType)
        {
        case Login:     // the sender will receive Login and Online, because they are broadcast
        {
            msgType = MessageType::Online;
            udpSocket->writeDatagram(reinterpret_cast<const char*>(&msgType),
                                     sizeof(MessageType), addr, port);
            ++numOfPeopleOnline;
            numOfPeopleOnlineLabel->setText(QString::number(numOfPeopleOnline));

            msgType = MessageType::RoomInfo;
            QByteArray sendData(reinterpret_cast<const char*>(&msgType), sizeof(MessageType));
            foreach (const RoomWidget *room, roomsOwned)
            {
                sendData.append(room->roomName.toUtf8());
                // appending char* will not append the last '\0'
                sendData.append('\0');
                sendData.append(localAddr.toString().toUtf8());
                sendData.append('\0');
                sendData.append(room->roomID.toUtf8());
                sendData.append('\0');
                sendData.append(reinterpret_cast<const char*>(&room->numOfPeople),
                                sizeof(unsigned int));
            }
            udpSocket->writeDatagram(sendData, addr, port);
            break;
        }
        case Online:
        {
            ++numOfPeopleOnline;
            if (!timer->isActive())
                setNumOfPeopleOnline();
            break;
        }
        case RoomInfo:
        {
            int pos = sizeof(MessageType);
            while (pos < recvData.size())
            {
                QString roomName(&recvData.data()[pos]);
                pos += roomName.size() + 1;         // '\0' at the end of roomName
                QString roomAddr(&recvData.data()[pos]);
                pos += roomAddr.size() + 1;
                QString roomID(&recvData.data()[pos]);
                pos += roomID.size() + 1;
                int numOfPeople;
                memcpy(&numOfPeople, &recvData.data()[pos], sizeof(int));
                pos += sizeof(int);
                QListWidgetItem *roomItem = new QListWidgetItem(roomName, roomsListWidget);
                roomItem->setData(Qt::UserRole, roomID);
                roomItem->setData(Qt::UserRole + 1, roomAddr);
                roomItem->setData(Qt::UserRole + 2, numOfPeople);
                roomItems[roomID] = roomItem;
            }
            break;
        }
        case Quit:
        {
            --numOfPeopleOnline;
            numOfPeopleOnlineLabel->setText(QString::number(numOfPeopleOnline));
            break;
        }
        case Enter:
        {
            int pos = sizeof(MessageType);
            QString name(&recvData.data()[pos]);
            pos += name.size() + 1;
            QString addr(&recvData.data()[pos]);
            pos += addr.size() + 1;
            QString roomID(&recvData.data()[pos]);
            MessageType msgType = MessageType::ComeIn;
            udpSocket->writeDatagram(QByteArray(reinterpret_cast<const char*>(&msgType)).append(
                                         roomID.toUtf8()), QHostAddress(addr), port);
            break;
        }
        case ComeIn:
        {
            QString roomID(&recvData.data()[sizeof (MessageType)]);
            QListWidgetItem *roomItem = roomItems[roomID];
            RoomWidget *room = new RoomWidget(roomItem->text(), roomID, userName->text(),
                                              QHostAddress(roomItem->data(Qt::UserRole + 1).toString()),
                                              roomsListWidget);
            roomsEntered[roomID] = room;
            room->show();
            break;
        }
        default:
            break;
        }
    }
}

void HomeWidget::setNumOfPeopleOnline()
{
    timer->stop();
    numOfPeopleOnlineLabel->setText(QString::number(numOfPeopleOnline));
    userName->setText(QString("#%1").arg(numOfPeopleOnline));
}

void HomeWidget::createRoom()
{
    bool ok = false;
    QString roomName = QInputDialog::getText(this, tr("create room"), tr("room name"),
                                             QLineEdit::Normal, "", &ok);
    if (ok)
    {
        time_t currentTime = time(nullptr);
        QString roomID = QString::number(currentTime) + localAddr.toString();
        RoomWidget *room = new RoomWidget(roomName, roomID, userName->text(), localAddr);
        roomsOwned[roomID] = room;
        room->show();

        MessageType msgType = MessageType::RoomInfo;
        int numOfPeople = 1;
        QByteArray sendData(reinterpret_cast<const char*>(&msgType), sizeof(MessageType));
        sendData.append(room->roomName.toUtf8()).append('\0').append(
                    localAddr.toString().toUtf8()).append(
                    '\0').append(room->roomID.toUtf8()).append(
                    '\0').append(reinterpret_cast<const char*>(&numOfPeople),
                                 sizeof(unsigned int));
        udpSocket->writeDatagram(sendData, QHostAddress::Broadcast, port);
    }
}

void HomeWidget::enterRoom(QListWidgetItem *item)
{
    QString roomID = item->data(Qt::UserRole).toString();
    if (roomsEntered.find(roomID) != roomsEntered.end())
        roomsEntered[roomID]->show();
    else if (roomsOwned.find(roomID) != roomsOwned.end())
        roomsOwned[roomID]->show();
    else
    {
        MessageType msgType = MessageType::Enter;
        udpSocket->writeDatagram(QByteArray(reinterpret_cast<const char*>(&msgType)).append(
                                     userName->text().toUtf8()).append('\0').append(
                                     localAddr.toString().toUtf8()).append(
                                     '\0').append(item->data(Qt::UserRole).toString().toUtf8()),
                                 QHostAddress(item->data(Qt::UserRole + 1).toString()), port);
    }
}


void HomeWidget::closeEvent(QCloseEvent* event)
{
    MessageType msgType = MessageType::Quit;
    udpSocket->writeDatagram(reinterpret_cast<const char*>(&msgType), sizeof(MessageType),
                             QHostAddress::Broadcast, port);
    foreach (RoomWidget *room, roomsEntered)
        room->close();
    foreach (RoomWidget *room, roomsOwned)
        room->close();
    event->accept();
}

