#include "roomwidget.h"

RoomWidget::RoomWidget(QString roomName, QString roomID, QString userName,
                       QHostAddress owner, QWidget *parent)
    : QWidget(parent), roomName(roomName), roomID(roomID),
      userName(userName), owner(owner)
{
    chatHistoryWidget = new QTextBrowser(this);
    inputWidget = new QTextBrowser(this);
    peopleWidget = new QListWidget(this);
    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->addWidget(chatHistoryWidget);
    leftLayout->addWidget(inputWidget);
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addLayout(leftLayout);
    mainLayout->addWidget(peopleWidget);
    setLayout(mainLayout);

    udpSocket = new QUdpSocket(this);
    udpSocket->bind(port, QUdpSocket::ShareAddress);
    connect(udpSocket, &QUdpSocket::readyRead, this, &RoomWidget::process);
    foreach (const QHostAddress &address, QNetworkInterface::allAddresses())
    {
        if (address.protocol() == QUdpSocket::IPv4Protocol
                && address != QHostAddress::LocalHost)
        {
            localAddr = address;
            break;
        }
    }
//    QListWidgetItem *personItem = new QListWidgetItem(userName, peopleWidget);
//    personItem->setData(Qt::UserRole, localAddr.toString());
//    peopleItem.push_back(personItem);
    MessageType msgType = MessageType::NewPerson;
    udpSocket->writeDatagram(QByteArray(reinterpret_cast<const char*>(&msgType),
                                        sizeof(MessageType)).append(userName.toUtf8()).append(
                                 '\0').append(localAddr.toString().toUtf8()),
                             owner, port);
}

RoomWidget::~RoomWidget()
{
}

void RoomWidget::process()
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
        case PeopleInfo:
        {
            QString name(&recvData.data()[sizeof(MessageType)]);
            addPerson(name, addr);
        }
        case NewPerson:
        {
            int pos = sizeof(MessageType);
            QString name(&recvData.data()[pos]);
            pos += name.size() + 1;
            QString addr(&recvData.data()[pos]);
            if (owner == localAddr)
            {
                foreach (QListWidgetItem *person, personItems)
                    udpSocket->writeDatagram(recvData,
                                             QHostAddress(person->data(Qt::UserRole).toString()), port);
            }
            else
            {
                MessageType msgType = MessageType::PeopleInfo;
                udpSocket->writeDatagram(QByteArray(reinterpret_cast<const char*>(&msgType)).append(
                                             userName.toUtf8()),
                                         QHostAddress(addr), port);
            }
            addPerson(name, QHostAddress(addr));
        }
        default:
            break;
        }
    }
}

void RoomWidget::addPerson(QString name, QHostAddress addr)
{
    QListWidgetItem *personItem = new QListWidgetItem(name, peopleWidget);
    personItem->setData(Qt::UserRole, addr.toString());
    personItems.push_back(personItem);
    ++numOfPeople;
}

void RoomWidget::closeEvent(QCloseEvent *event)
{
    if (leave)
        event->accept();
    else
        hide();
}
