#pragma once

enum class MessageType {
    REQUEST,
    DATA,
    AGGREGATE
};

struct Message {
    MessageType type;
    int senderId;
    int receiverId;
    double consumptionValue;

    Message(MessageType t, int sender, int receiver, double value)
        : type(t), senderId(sender), receiverId(receiver), consumptionValue(value) {}
};