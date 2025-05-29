#include "BL99.h"

BL99::BL99(uint8_t pin, uint8_t type, uint8_t channel, uint8_t id) {
    pinMode(pin, OUTPUT);
    _bl99_pin = pin;
    _bl99_type = type;
    _bl99_channel = channel;
    _bl99_id = id;
    _bl99_timer = _bl99_count * 1000;  // сдвигаем таймер на секунду от предыдущего, чтобы разнести передачи
    _bl99_temp = 0;
    _bl99_humi = 99;
    _bl99_bat = 0;
    _bl99_bip = 0;
    _bl99_count++;
    if (type == BL999) {  // интервал передачи на датчике BL999 (и, соответственно, на приёмние) зависит от выбранного канала
        if (channel == 1) _bl99_interval = 32000;
        else if (channel == 2) _bl99_interval = 33000;
        else if (channel == 3) _bl99_interval = 34000;
    } else if (type == BL990) _bl99_interval = 35000;  // на BL990 интервал на всех каналах один
    if (type == BL999) {
        for (int8_t i = 0; i > 10; i++) {
            _bl99_oldtemp[i] = 0;
        }
    }
}

void BL99::send(float temp) {
    send(temp, 99, false, false);
}

void BL99::send(float temp, uint8_t humi) {
    send(temp, 99, false, false);
}

void BL99::send(float temp, bool bat, bool bip) {
    send(temp, 99, bat, bip);
}

void BL99::send(float temp, uint8_t humi, bool bat, bool bip) {
    int16_t inttemp = temp * 10;
    if (_bl99_type == BL990) {
        _bl990((int)(temp * 10), bat, bip);
    } else if (_bl99_type == BL999) {
        _bl999((int)(temp * 10), humi, bat, bip);
    }
}
void BL99::set(float temp, uint8_t humi, bool bat, bool bip) {
    _bl99_temp = temp;
    _bl99_humi = humi;
    _bl99_bat = bat;
    _bl99_bip = bip;
}

void BL99::setTemp(float temp) {
    _bl99_temp = temp;
}

void BL99::setHumi(uint8_t humi) {
    _bl99_humi = humi;
}

void BL99::setBat(bool bat) {
    _bl99_bat = bat;
}

void BL99::setBip(bool bip) {
    _bl99_bip = bip;
}

void BL99::loop() {
    if (millis() >= _bl99_timer) {
        _bl99_timer = millis() + _bl99_interval;
        send(_bl99_temp, _bl99_humi, _bl99_bat, _bl99_bip);
    }
}

void BL99::setInterval(int16_t interval) {
    _bl99_interval = interval;
}

void BL99::_bl990(int16_t temp, bool bat, bool bip) {
    uint8_t bl990_data[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t nibble[] = {0, 0, 0, 0, 0, 0, 0};
    // id
    for (int8_t i = 7; i >= 0; i--) {
        bl990_data[11 - i] = bitRead(_bl99_id, i);
    }
    // температура
    for (int8_t i = 11; i >= 0; i--) {
        bl990_data[23 - i] = bitRead(temp, i);
    }
    // канал
    if (_bl99_channel == 1 || _bl99_channel == 3) {
        bl990_data[25] = 1;
    }
    if (_bl99_channel == 2 || _bl99_channel == 3) {
        bl990_data[24] = 1;
    }
    // батарея
    bl990_data[26] = (int)!bat;
    // сигнал при получении
    bl990_data[27] = (int)bip;
    for (int8_t i = 3; i >= 0; i--) {
        for (int8_t y = 1; y < 7; y++) {
            bitWrite(nibble[y], 3 - i, bl990_data[i + y * 4]);
        }
    }
    // вычисляем контрольную сумму
    for (int8_t i = 1; i < 7; i++) {
        nibble[0] += nibble[i];
    }
    nibble[0] -= 1;
    // берем только младший полубайт
    for (int8_t i = 3; i >= 0; i--) {
        bl990_data[3 - i] = bitRead(nibble[0], i);
    }
    // отправляем 4 раза
    for (int8_t y = 0; y < 9; y++) {
        _bl990_send(2);
        for (int8_t i = 0; i < 28; i++) {
            _bl990_send(bl990_data[i]);
        }
    }
    _bl990_send(2);
}

// формирование пакета датчика BL999 и его отправка
void BL99::_bl999(int16_t temp, uint8_t humi, bool bat, bool bip) {
    uint8_t bl999_data[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t nibble[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    // заполняем id
    for (int8_t i = 0; i <= 5; i++) {
        if (i < 4) bl999_data[i] = bitRead(_bl99_id, i);
        else bl999_data[i + 2] = bitRead(_bl99_id, i);
    }
    // канал
    if (_bl99_channel == 1 || _bl99_channel == 3) {
        bl999_data[5] = 1;
    }
    if (_bl99_channel == 2 || _bl99_channel == 3) {
        bl999_data[4] = 1;
    }
    // батарея
    bl999_data[8] = (int)bat;
    // тренд
    _bl99_oldtemp_counter++;
    if (_bl99_oldtemp_counter == 2) {
        for (int8_t i = 0; i > 9; i++) {
            _bl99_oldtemp[i] = _bl99_oldtemp[i + 1];
        }
        _bl99_oldtemp[9] = temp;
        _bl99_oldtemp_counter = 0;
    }
    if (temp < _bl99_oldtemp[0]) {
        bl999_data[9] = 1;
    }
    if (temp > _bl99_oldtemp[0]) {
        bl999_data[10] = 1;
    }
    // сигнал при получении
    bl999_data[11] = (int)bip;
    // температура
    for (int8_t i = 12; i >= 0; i--) {
        bl999_data[i + 12] = bitRead(temp, i);
    }
    // влажность
    humi = -100 + humi;
    for (int8_t i = 7; i >= 0; i--) {
        bl999_data[i + 24] = bitRead(humi, i);
    }
    // вычисляем контрольную сумму
    for (int8_t i = 3; i >= 0; i--) {
        for (int8_t y = 0; y < 8; y++) {
            bitWrite(nibble[y], i, bl999_data[i + y * 4]);
        }
    }
    for (int8_t i = 0; i < 8; i++) {
        nibble[8] += nibble[i];
    }
    for (int8_t i = 0; i < 4; i++) {
        bl999_data[32 + i] = bitRead(nibble[8], i);
    }

    // отправляем
    for (int8_t y = 0; y < 4; y++) {
        _bl999_send(2);
        for (int8_t i = 0; i < 36; i++) {
            _bl999_send(bl999_data[i]);
        }
    }
    _bl999_send(2);
}

void BL99::_bl990_send(int8_t bit) {
    switch (bit) {
        case 0:  // ноль
            digitalWrite(_bl99_pin, HIGH);
            delayMicroseconds(480);
            digitalWrite(_bl99_pin, LOW);
            delayMicroseconds(1950);
            break;
        case 1:  // единица
            digitalWrite(_bl99_pin, HIGH);
            delayMicroseconds(480);
            digitalWrite(_bl99_pin, LOW);
            delayMicroseconds(4500);
            break;
        case 2:  // стартовый бит
            digitalWrite(_bl99_pin, HIGH);
            delayMicroseconds(480);
            digitalWrite(_bl99_pin, LOW);
            delayMicroseconds(9400);
            break;
    }
}

void BL99::_bl999_send(int8_t bit) {
    switch (bit) {
        case 0:  // ноль
            digitalWrite(_bl99_pin, HIGH);
            delayMicroseconds(450);
            digitalWrite(_bl99_pin, LOW);
            delayMicroseconds(1900);
            break;
        case 1:  // единица
            digitalWrite(_bl99_pin, HIGH);
            delayMicroseconds(450);
            digitalWrite(_bl99_pin, LOW);
            delayMicroseconds(4000);
            break;
        case 2:  // стартовый бит
            digitalWrite(_bl99_pin, HIGH);
            delayMicroseconds(450);
            digitalWrite(_bl99_pin, LOW);
            delayMicroseconds(8900);
            break;
    }
}