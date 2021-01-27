#ifndef COMMON_H
#define COMMON_H

const int port = 12345;

enum MessageType: unsigned char
{
    Login = 0,
    Online,      // response to Login
    RoomInfo,       // info of chatrooms, including name, owner, etc
    RoomNameInfo,
    RoomOwnerInfo,
    NumOfPeopleInfo,

    NewPerson,
    PeopleInfo,

    Quit,
    Enter,
    ComeIn,
    Leave,
    MaxSize = 255
};

#endif // COMMON_H
