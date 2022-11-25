#ifndef CCNET_H
#define CCNET_H

#include <iostream>
#include <vector>
#include <iomanip>
#include <string.h>
#include <chrono>
#include <thread>
#include <stdio.h>   /* Стандартные объявления ввода/вывода */
#include <string.h>  /* Объявления строковых функций */
#include <unistd.h>  /* Объявления стандартных функций UNIX */
#include <fcntl.h>   /* Объявления управления файлами */
#include <errno.h>   /* Объявления кодов ошибок */
#include <termios.h> /* Объявления управления POSIX-терминалом */
#include "ccnet_types.hpp"
#include "ccnetException.hpp"
#include "ValidatorCommands.hpp"
#include "Poll.hpp"
#include "ccnet.hpp"

class ccnet
{
    int fd;
    int POLYNOMIAL = 0x08408;
    const cc_byte SYNC = 0x02;
    cc_byte ADDR = 0x03;
    int error_code = 0x00;
    int m_CashReceived;

    vec_bytes SECURITY_CODE = {0x00, 0x00, 0x00};
    vec_bytes ENABLE_BILL_TYPES_WITH_ESCROW = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    vec_bytes DISABLE_BILL_TYPES_WITH_ESCROW = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    std::vector<float> denomination;
    std::vector<int> payment;
    std::vector<std::string> country_code;

public:
    ccnet();
    ~ccnet();

    void connect(std::string com_port, int baudrate = 9600);

    cc_short GetCRC16(vec_bytes bufData, size_t size);
    bool checkErrors(vec_bytes &result);
    vec_bytes compileCommand(ValidatorCommands cmd, vec_bytes data = {});
    vec_bytes sendCommand(ValidatorCommands cmd, vec_bytes data = {}, int _delay_ms = 10);
    vec_bytes getResponse();
    bool testCRC(vec_bytes bufData);
    void print(std::string msg, vec_bytes data);

    void reset();
    int powerUp();
    void enableSequence();
    void disableSequence();
    bool getPayment(int summ);
    void getBillTable();

    inline int getCashRecv() noexcept { return m_CashReceived; }
};

#endif // CCNET_H
