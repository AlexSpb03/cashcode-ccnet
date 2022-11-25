#ifndef VALIDATORCOMMANDS_H

#include <iostream>
#include <vector>
#include "ccnet_types.hpp"

enum class ValidatorCommands : cc_byte
{
    ACK = 0x00,
    NAK = 0xFF,
    RESET = 0x30,
    GET_STATUS = 0x31,
    GET_BILL_TABLE = 0x41,
    SET_SECURITY = 0x32,
    IDENTIFICATION = 0x37,
    ENABLE_BILL_TYPES = 0x34,
    STACK = 0x35,
    RETURN = 0x36,
    HOLD = 0x38,
    POLL = 0x33,
    REQUEST_STATISTICS = 0x60,
};

#endif // VALIDATORCOMMANDS_H