// BL99X - библиотека отправки данных на часы/метеостанции, имитируя датчики BL990 и BL999
// Автор Александр Вашестюк Май, 2025г.

#pragma once
#include <Arduino.h>

#define BL990 0
#define BL999 1

class BL99 {
   public:
    // pin(пин с передатчиком), type(0-BL990, 1-BL999), channel(канал 1-3), id(0-255(BL990) 0-63(BL999)))
    BL99(uint8_t pin, uint8_t type, uint8_t channel = 1, uint8_t id = 1);
    // отправка только температуры
    void send(float temp);
    // отправка температуры с влажностью
    void send(float temp, uint8_t hum);
    // отправка температуры с статусом батареи и сигналом при приеме
    void send(float temp, bool bat, bool bip);
    // отправка температуры с влажностью, статусом батареи и сигналом при приеме
    void send(float temp, uint8_t hum, bool bat, bool bip);
    // установка интервала отправки в милисекундах (у BL990 по умолчанию 35)
    void setInterval(int16_t interval);
    // автоматическая отправка по таймеру
    void loop();
    // установка параметров для автоматической отправки
    void set(float temp, uint8_t humi, bool bat = false, bool bip = false);
    // установка температуры
    void setTemp(float temp);
    // установка влажности для автоматической отправки
    void setHumi(uint8_t humi);
    // установка статуса батареи для автоматической отправки
    void setBat(bool bat);
    // установка сигнала при получении для автоматической отправки
    void setBip(bool bip);

   private:
    void _bl990(int16_t temp, bool bat, bool bip);
    void _bl999(int16_t temp, uint8_t humi, bool bat, bool bip);
    void _bl990_send(int8_t bit);
    void _bl999_send(int8_t bit);
    static inline uint8_t _bl99_count{};
    uint8_t _bl99_pin;
    uint8_t _bl99_type;
    uint8_t _bl99_channel;
    uint8_t _bl99_id;
    float _bl99_temp;
    uint8_t _bl99_humi;
    bool _bl99_bat;
    bool _bl99_bip;
    uint8_t _bl99_oldtemp_counter{1};
    int16_t _bl99_oldtemp[10];
    uint32_t _bl99_timer;
    uint16_t _bl99_interval;
};