#ifndef POLL_H

#include <iostream>
#include <vector>
#include "ccnet_types.hpp"

class Poll
{
public:
    const static cc_byte ACK = 0x00;
    const static cc_byte INITIALIZE = 0x13;
    const static cc_byte IDLING = 0x14;
    const static cc_byte ACCEPTING = 0x15;
    const static cc_byte STACKING = 0x17;
    const static cc_byte RETURNING = 0x18;
    const static cc_byte DISABLED = 0x19;
    const static cc_byte REJECTING = 0x1C;
    const static cc_byte DROP_CASSETTE_FULL = 0x41;
    const static cc_byte DROP_CASSETTE_OUT_OF_POSITION = 0x42;
    const static cc_byte VALIDATOR_JAMMED = 0x43;
    const static cc_byte DROP_CASSETTE_JAMMED = 0x44;
    const static cc_byte ESCROW_POSITION = 0x80;
    const static cc_byte BILL_STACKED = 0x81;
    const static cc_byte BILL_RETURNED = 0x82;
};

#endif // POLL_H