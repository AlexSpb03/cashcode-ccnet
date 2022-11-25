# cashcode-ccnet

Класс С++ для работы с устройством CashCode по протоколу CCNET.

Пример использования:
```c++
#include <iostream>

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
```
