#include <stdio.h>   /* Стандартные объявления ввода/вывода */
#include <string.h>  /* Объявления строковых функций */
#include <unistd.h>  /* Объявления стандартных функций UNIX */
#include <fcntl.h>   /* Объявления управления файлами */
#include <errno.h>   /* Объявления кодов ошибок */
#include <termios.h> /* Объявления управления POSIX-терминалом */
#include <pthread.h>
#include <iostream>
#include <chrono>
#include <thread>

#include "ccnet.hpp"


int main()
{
    ccnet cashcode;
    cashcode.connect("/dev/ttyUSB1", B9600);
    cashcode.powerUp();
    cashcode.getBillTable();
    cashcode.getPayment(150);
 

    return 0;
}