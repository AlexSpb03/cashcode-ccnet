#include "ccnet.hpp"

ccnet::ccnet() : m_CashReceived(0)
{
}

ccnet::~ccnet()
{
}

void ccnet::connect(std::string com_port, int baudrate)
{
    fd = open(com_port.c_str(), O_RDWR | O_NOCTTY);
    if (fd < 0)
    {
        printf("Error open com port\n");
    }
    printf("Open\n");

    struct termios tty;

    if (tcgetattr(fd, &tty) != 0)
    {
        std::cerr << "Error from tcgetattr: " << strerror(errno) << std::endl;
        return;
    }

    cfsetispeed(&tty, baudrate);
    cfsetospeed(&tty, baudrate);

    tty.c_cflag &= ~PARENB; // Make 8n1
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~CRTSCTS; // no flow control
    tty.c_lflag = 0;         // no signaling chars, no echo, no canonical processing
    tty.c_oflag = 0;         // no remapping, no delays
    tty.c_cc[VMIN] = 1;      // read doesn't block
    tty.c_cc[VTIME] = 1;     // 0.1 seconds read timeout

    tty.c_cflag |= CREAD | CLOCAL;                  // turn on READ & ignore ctrl lines
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);         // turn off s/w flow ctrl
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // make raw
    tty.c_oflag &= ~OPOST;                          // make raw

    if (tcsetattr(fd, TCSANOW, &tty) != 0)
    {
        std::cerr << "Error from tcsetattr: " << strerror(errno) << std::endl;
        return;
    }
}

// Вычисление контрольной суммы для сообщения
cc_short ccnet::GetCRC16(vec_bytes bufData, size_t size)
{
    cc_short CRC = 0;
    for (int i = 0; i < size; i++)
    {
        CRC = CRC ^ bufData[i];
        for (int j = 0; j < 8; j++)
        {
            if (CRC & 0x0001)
            {
                CRC >>= 1;
                CRC ^= POLYNOMIAL;
            }
            else
                CRC >>= 1;
        }
    }
    return CRC;
};

bool ccnet::checkErrors(vec_bytes &response)
{
    if (!response.size())
    {
        return false;
    }
    bool is_error = false;
    error_code = response[3];

    switch (response[3])
    {
    case 0x30:
        error_code = 0x30;
        is_error = true;
        break;
    case 0x40:
        error_code = 0x40;
        is_error = true;
        break;
    case 0x45:
        error_code = 0x45;
        is_error = true;
        break;
    case 0x46:
        error_code = 0x46;
        is_error = true;
        break;
    case 0x47:
        switch (response[4])
        {
        case 0x50:
            error_code = 0x50;
            break;
        case 0x51:
            error_code = 0x51;
            break;
        case 0x52:
            error_code = 0x52;
            break;
        case 0x53:
            error_code = 0x53;
            break;
        case 0x54:
            error_code = 0x54;
            break;
        case 0x55:
            error_code = 0x55;
            break;
        case 0x56:
            error_code = 0x56;
            break;
        case 0x5F:
            error_code = 0x5F;
            break;
        default:
            error_code = 0x47;
        }
        is_error = true;
        break;
    case 0x1C:
        switch (response[4])
        {
        case 0x60:
            error_code = 0x60;
            break;
        case 0x61:
            error_code = 0x61;
            break;
        case 0x62:
            error_code = 0x62;
            break;
        case 0x63:
            error_code = 0x63;
            break;
        case 0x64:
            error_code = 0x64;
            break;
        case 0x65:
            error_code = 0x65;
            break;
        case 0x66:
            error_code = 0x66;
            break;
        case 0x67:
            error_code = 0x67;
            break;
        case 0x68:
            error_code = 0x68;
            break;
        case 0x69:
            error_code = 0x69;
            break;
        case 0x6A:
            error_code = 0x6A;
            break;
        case 0x6C:
            error_code = 0x6C;
            break;
        case 0x6D:
            error_code = 0x6D;
            break;
        case 0x92:
            error_code = 0x92;
            break;
        case 0x93:
            error_code = 0x93;
            break;
        case 0x94:
            error_code = 0x94;
            break;
        case 0x95:
            error_code = 0x95;
            break;
        default:
            error_code = 0x1C;
            break;
        }
        is_error = true;
        break;
    }

    if (is_error)
        std::cerr << "ERROR: " << std::setw(2) << std::setfill('0') << std::hex << error_code << std::endl;
    return is_error;
}

// Формирование сообщения для отправки
std::vector<cc_byte> ccnet::compileCommand(ValidatorCommands cmd, vec_bytes data)
{

    vec_bytes packet;

    packet.push_back(SYNC); // байт синхронизации SYNC 0x02
    packet.push_back(ADDR); // адрес устройства

    // Высчитываем длинну сообщения
    size_t length = data.size() + 6;

    // Если длина больше 250 символов
    if (length > 250)
    {
        packet.push_back(0x00);
    }
    else
    {
        packet.push_back(static_cast<cc_byte>(length & 0xFF)); // LNG
    }

    packet.push_back(static_cast<cc_byte>(cmd)); // CMD

    if (length > 250)
    {
        packet.push_back(static_cast<cc_byte>((length >> 8) & 0xFF));
        packet.push_back(static_cast<cc_byte>(length & 0xFF));
    }

    for (int i = 0; i < data.size(); i++)
    {
        packet.push_back(data[i]);
    }
    cc_short crc = GetCRC16(packet, packet.size());

    packet.push_back(static_cast<cc_byte>(crc & 0xFF));
    packet.push_back(static_cast<cc_byte>((crc >> 8) & 0xFF));

    return packet;
}

// Получение ответа от устройства
vec_bytes ccnet::getResponse()
{
    vec_bytes buf;

    size_t size_buf = 1000;
    char buf_read[size_buf];
    memset(buf_read, 0, size_buf);

    size_t len;

    try
    {
        len = read(fd, &buf_read, size_buf);
    }
    catch (std::exception &e)
    {
        throw new ccnetException(e.what(), 0x11);
    }

    for (int i = 0; i < len; i++)
    {
        buf.push_back(static_cast<cc_byte>(buf_read[i]));
    }

    if (!testCRC(buf))
    {
        throw new ccnetException("Bad CRC response", EXCEPTION_BAD_CRC_IN_RESPONSE);
    }

    return buf;
}

/**
 * @brief Отправка команды на устройство
 *
 * @param cmd - команда
 * @param data - данные
 * @param _delay_ms - задержка в милисекундах, перед получением ответа.
 * Если задержка есть, то будем получать ответ.
 * Если 0, то ответа на команду не ждем
 * @return vec_bytes
 */
vec_bytes ccnet::sendCommand(ValidatorCommands cmd, vec_bytes data, int _delay_ms)
{
    try
    {
        auto packet = compileCommand(cmd, data);
        char *buf = new char[packet.size()];
        for (int i = 0; i < packet.size(); i++)
        {
            buf[i] = packet[i];
        }
        write(fd, buf, packet.size());
    }
    catch (std::exception &e)
    {
        throw e;
    }
    if (_delay_ms > 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(_delay_ms));
        try
        {
            return getResponse();
        }
        catch (const ccnetException &e)
        {
            throw e;
        }
    }
    vec_bytes ret{};
    return ret;
}

// Команда RESET
void ccnet::reset()
{
    vec_bytes response;

    try
    {
        response = sendCommand(ValidatorCommands::RESET, {}, 300);
    }
    catch (ccnetException &e)
    {

        switch (e.getCode())
        {
        case EXCEPTION_BAD_CRC_IN_RESPONSE:
            try
            {
                sendCommand(ValidatorCommands::NAK, {}, -1);
            }
            catch (const std::exception &e)
            {
                throw e;
            }
            break;
        default:
            throw e;
        }
    }

    // Если купюроприемник не ответил сигналом ACK
    if (response[3] != 0x00)
    {
        throw new ccnetException("Validator don't response ACK", response[3]);
    }
}

int ccnet::powerUp()
{
    vec_bytes response;
    try
    {
        response = sendCommand(ValidatorCommands::POLL, {}, 200);
        print("POWER UP: ", response);

        if (checkErrors(response))
        {
            sendCommand(ValidatorCommands::NAK);
            return error_code;
        }

        // Если CashCode вернул в 4 байте 0х19 значит он уже включен
        if (response[3] == 0x19)
        {
            std::cout << "Validator ready to work!" << std::endl;
            return error_code;
        }

        // std::this_thread::sleep_for(std::chrono::milliseconds(20));

        // Если все хорошо, отправляет команду подтверждения
        sendCommand(ValidatorCommands::ACK, {}, -1);

        std::this_thread::sleep_for(std::chrono::milliseconds(20));

        // Команда reset
        reset();

        // Опрос купюроприемника процедура инициализации
        response = sendCommand(ValidatorCommands::POLL, {}, 200);
        print("POLL: ", response);

        if (checkErrors(response))
        {
            sendCommand(ValidatorCommands::NAK);
            return error_code;
        }

        sendCommand(ValidatorCommands::ACK, {}, -1);

        response = sendCommand(ValidatorCommands::GET_STATUS, {}, 200);
        print("GET_STATUS: ", response);

        // Команда GET STATUS возвращает 6 байт ответа. Если все равны 0, то статус ok и можно работать дальше, иначе ошибка
        if (response[3] != 0x00 || response[4] != 0x00 || response[5] != 0x00 ||
            response[6] != 0x00 || response[7] != 0x00 || response[8] != 0x00)
        {
            throw new ccnetException("Error get status", 0x70);
        }

        // Подтверждает если все хорошо
        sendCommand(ValidatorCommands::ACK, {}, -1);

        response = sendCommand(ValidatorCommands::SET_SECURITY, SECURITY_CODE, 200);
        print("SET_SECURITY: ", response);

        // Если не получили от купюроприемника сигнал ACK
        if (response[3] != Poll::ACK)
        {
            throw new ccnetException("Validator don't response ACK", 0x00);
        }

        // IDENTIFICATION
        response = sendCommand(ValidatorCommands::IDENTIFICATION, {}, 200);

        sendCommand(ValidatorCommands::ACK, {}, -1);
        print("IDENTIFICATION: ", response);

        // Опрашиваем купюроприемник должны получить команду INITIALIZE
        response = sendCommand(ValidatorCommands::POLL, {}, 200);
        print("INITIALIZE: ", response);

        if (checkErrors(response))
        {
            sendCommand(ValidatorCommands::NAK);
            return error_code;
        }

        sendCommand(ValidatorCommands::ACK, {}, -1);

        // POLL Должны получить команду UNIT DISABLE
        response = sendCommand(ValidatorCommands::POLL, {}, 200);
        print("UNIT DISABLE: ", response);
        if (checkErrors(response))
        {
            sendCommand(ValidatorCommands::NAK);
            return error_code;
        }

        sendCommand(ValidatorCommands::ACK, {}, -1);
    }
    catch (ccnetException &e)
    {
        throw e;
    }
    return 0;
}

/**
 * @brief 
 * Включение купюроприёмника на приём денежных средств. Загорается ззелёным
 */
void ccnet::enableSequence()
{
    vec_bytes response;
    try
    {
        //--- Отправляем команду активации всех купюр
        response = sendCommand(ValidatorCommands::ENABLE_BILL_TYPES, ENABLE_BILL_TYPES_WITH_ESCROW, 200);
        if (checkErrors(response))
        {
            sendCommand(ValidatorCommands::NAK);
            throw new ccnetException("Error enable bill types");
        }

        //--- Отправляем команду poll согласно документации
        response = sendCommand(ValidatorCommands::POLL, {}, 200);
        if (checkErrors(response))
        {
            sendCommand(ValidatorCommands::NAK);
            throw new ccnetException("Error poll cmd");
        }

        //--- если ответ получен и соответствует условию, то подтверждаем приём
        if (response[3] == Poll::IDLING || response[3] == Poll::INITIALIZE)
        {
            response = sendCommand(ValidatorCommands::ACK, {}, -1);
        }
        else
        {
            sendCommand(ValidatorCommands::NAK, {}, -1);
            throw new ccnetException("[enableSequence] Recv not IDLING");
        }
    }
    catch (std::exception &e)
    {
        throw e;
    }
}

/**
 * @brief 
 * Отключение приёма денежных средств
 */
void ccnet::disableSequence()
{

    vec_bytes response;
    try
    {
        //--- отправляем команду на отключение приёма купюр
        response = sendCommand(ValidatorCommands::ENABLE_BILL_TYPES, DISABLE_BILL_TYPES_WITH_ESCROW, 200);
        if (checkErrors(response))
        {
            sendCommand(ValidatorCommands::NAK);
            throw new ccnetException("Error disable bill types");
        }
        //--- Отправляем команду poll
        response = sendCommand(ValidatorCommands::POLL, {}, 200);
        if (checkErrors(response))
        {
            sendCommand(ValidatorCommands::NAK);
            throw new ccnetException("Error poll cmd");
        }

        //--- Если выполняется условие, то подтверждаем приём сообщения 
        if (response[3] == Poll::DISABLED)
        {
            response = sendCommand(ValidatorCommands::ACK, {}, -1);
        }
    }
    catch (std::exception &e)
    {
        throw e;
    }
}

/**
 * @brief Приём денежных средств
 *
 * @param summ - сумма в рублях сколько надо принять денег
 * @return true
 * @return false
 */
bool ccnet::getPayment(int summ)
{
    vec_bytes response;
    bool isCancel = false;
    payment.assign(24, 0);
    m_CashReceived = 0;

    enableSequence();

    int timer = 0;
    try
    {
        //--- Цикл приёма денег
        for (;;)
        {
            //--- Если активности нет N итераций, то выходим
            if (timer == 40)
            {
                break;
            }

            if (isCancel)
            {
                break;
            }

            // POLL - команда для получения текущего статуса купюроприёмника
            response = sendCommand(ValidatorCommands::POLL, {}, 200);

            // Если пакет получен не полностью, то сообщаем об этом
            if (response.size() < 3)
            {
                sendCommand(ValidatorCommands::NAK, {}, -1);
                continue;
            }

            // Проверка на ошибки, если были обнаружены ошибки посылаем команду NAK
            if (checkErrors(response))
            {
                sendCommand(ValidatorCommands::NAK, {}, -1);
                continue;
            }

            //--- Проверяем статус
            switch (response[3])
            {

            case Poll::DROP_CASSETTE_FULL:
                std::cout << "Poll::DROP_CASSETTE_FULL" << std::endl;
                break;

            case Poll::DROP_CASSETTE_OUT_OF_POSITION:
                std::cout << "Poll::DROP_CASSETTE_OUT_OF_POSITION" << std::endl;
                break;

            case Poll::VALIDATOR_JAMMED:
                std::cout << "Poll::VALIDATOR_JAMMED" << std::endl;
                break;

            case Poll::DROP_CASSETTE_JAMMED:
                std::cout << "Poll::DROP_CASSETTE_JAMMED" << std::endl;
                break;

            case Poll::IDLING:
                std::cout << "Poll::IDLING" << std::endl;
                sendCommand(ValidatorCommands::ACK, {}, -1);
                break;

            case Poll::ACCEPTING:
                timer = 0;
                std::cout << "Poll::ACCEPTING" << std::endl;
                sendCommand(ValidatorCommands::ACK, {}, -1);
                break;

            case Poll::REJECTING:
                timer = 0;
                // не распознана купюра
                std::cout << "Poll::REJECTING" << std::endl;
                sendCommand(ValidatorCommands::ACK, {}, -1);
                break;

            case Poll::ESCROW_POSITION:
                timer = 0;
                // Купюра распознана и находится в отсеке хранения
                std::cout << "Poll::ESCROW_POSITION" << std::endl;
                sendCommand(ValidatorCommands::ACK, {}, -1);

                std::this_thread::sleep_for(std::chrono::milliseconds(20));

                // Отправляем купюру с стек
                response = sendCommand(ValidatorCommands::STACK, {}, 200);
                break;

            case Poll::STACKING:
                timer = 0;
                std::cout << "Poll::STACKING" << std::endl;
                sendCommand(ValidatorCommands::ACK, {}, -1);
                break;

            case Poll::BILL_STACKED:
                timer = 0;
                std::cout << "Poll::BILL_STACKED" << std::endl;
                sendCommand(ValidatorCommands::ACK, {}, -1);

                m_CashReceived += denomination[response[4]];

                payment[response[4]]++;
                std::cout << "\nCASH: " << std::dec << denomination[response[4]] << std::endl;
                print("STACKED: ", response);

                if (m_CashReceived >= summ)
                {
                    isCancel = true;
                }

                break;

            case Poll::RETURNING:
                timer = 0;
                std::cout << "Poll::RETURNING" << std::endl;
                sendCommand(ValidatorCommands::ACK, {}, -1);
                break;

            case Poll::BILL_RETURNED:
                timer = 0;
                std::cout << "Poll::BILL_RETURNED" << std::endl;
                sendCommand(ValidatorCommands::ACK, {}, -1);
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            timer++;
        }
    }
    catch (std::exception &e)
    {
        std::cout << e.what() << std::endl;
    }

    disableSequence();

    if (m_CashReceived >= summ)
    {
        return true;
    }
    return false;
}

/**
 * @brief Получение таблицы соответствия номера и номинала купюры.
 *
 */
void ccnet::getBillTable()
{
    auto response = sendCommand(ValidatorCommands::GET_BILL_TABLE, {}, 200);
    if (checkErrors(response))
    {
        return;
    }

    for (int i = 3; i < response.size() - 2;)
    {
        //--- Массив строки страны купюры
        char country[4]{0};
        //--- Номинал купюры
        float d = 0;

        if ((response[i + 4] & 0x80) == 0 && response[i])
        {

            d = 1;

            country[0] = response[i + 1];
            country[1] = response[i + 2];
            country[2] = response[i + 3];

            //--- Получаем количество нулей в номинале купюры
            int c = response[i + 4];
            c &= 0x7F;

            //--- Получаем первую цифру номинала купюры
            d = response[i];

            //--- Формируем номинал купюры исходя из первой цифры и количества нулей
            for (int j = 0; j < c; j++)
            {
                d *= 10;
            }
        }

        //--- Если пустая запись, то пустое значение названия страны
        if (!d)
        {
            country_code.push_back("");
        }
        else
        {
            country_code.push_back(country);
        }

        denomination.push_back(d);

        i += 5;
    }
}

/**
 * @brief Проверка контрольной суммы для ответа
 * 
 * @param bufData 
 * @return true 
 * @return false 
 */
bool ccnet::testCRC(vec_bytes bufData)
{
    if (bufData.size() == 0)
    {
        return true;
    }
    cc_short crc = GetCRC16(bufData, bufData.size() - 2);
    cc_short crc_packet = 0;
    crc_packet = bufData[bufData.size() - 1] << 8;
    crc_packet += bufData[bufData.size() - 2];

    return crc == crc_packet;
}

/**
 * @brief Вывод полученных данныхы
 * 
 * @param msg 
 * @param data 
 */
void ccnet::print(std::string msg, vec_bytes data)
{
    std::cout << msg;
    for (auto byte : data)
        std::cout << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << (int)byte;
    std::cout << std::endl;
}
