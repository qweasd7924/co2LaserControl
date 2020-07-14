#include <Arduino.h>  // Лобавляем необходимую библиотеку
#include <LiquidCrystal.h>  // Лобавляем необходимую библиотеку
#include <OneWire.h>
#include <DallasTemperature.h>


const int watermeterPin = 2;

volatile int  pulse_frequency;
unsigned int  literpermin;
unsigned long currentTime, loopTime;
byte sensorInterrupt = 0;


void getFlow ()
{
    pulse_frequency++;
}


LiquidCrystal lcd(7, 6, 5, 4, 3, 8); // (RS, E, DB4, DB5, DB6, DB7)

// сигнальный провод датчика
#define ONE_WIRE_BUS 9

// создаём объект для работы с библиотекой OneWire
OneWire oneWire(ONE_WIRE_BUS);
// создадим объект для работы с библиотекой DallasTemperature
DallasTemperature sensors(&oneWire);
// создаём указатель массив для хранения адресов датчиков
DeviceAddress *sensorsUnique;
// количество датчиков на шине
int countSensors;

// функция вывода адреса датчика
void printAddress(DeviceAddress deviceAddress){
    for (uint8_t i = 0; i < 8; i++){
        if (deviceAddress[i] < 16) Serial.print("0");
        Serial.print(deviceAddress[i], HEX);
    }
}

void setup(){
    pinMode(watermeterPin, INPUT);
    attachInterrupt(sensorInterrupt, getFlow, FALLING);


    lcd.begin(16, 2);                  // Задаем размерность экрана

    lcd.flush();
    lcd.setCursor(0, 0);              // Устанавливаем курсор в начало 1 строки
    lcd.print("Start");       // Выводим текст
    lcd.setCursor(0, 1);              // Устанавливаем курсор в начало 2 строки
    // lcd.print("(8===0)");         // Выводим текст
    // инициализируем работу Serial-порта
    Serial.begin(9600);
    // ожидаем открытия Serial-порта
    while(!Serial);
    // начинаем работу с датчиком
    sensors.begin();
    // выполняем поиск устройств на шине
    countSensors = sensors.getDeviceCount();
    Serial.print("Found sensors: ");
    Serial.println(countSensors);
    // выделяем память в динамическом массиве под количество обнаруженных сенсоров
    sensorsUnique = new DeviceAddress[countSensors];

    // определяем в каком режиме питания подключены сенсоры
    if (sensors.isParasitePowerMode()) {
        Serial.println("Mode power is Parasite");
    } else {
        Serial.println("Mode power is Normal");
    }

    // делаем запрос на получение адресов датчиков
    for (int i = 0; i < countSensors; i++) {
        sensors.getAddress(sensorsUnique[i], i);
    }
    // выводим полученные адреса
    for (int i = 0; i < countSensors; i++) {
        Serial.print("Device ");
        Serial.print(i);
        Serial.print(" Address: ");
        printAddress(sensorsUnique[i]);
        Serial.println();
    }
    Serial.println();
    // устанавливаем разрешение всех датчиков в 12 бит
    for (int i = 0; i < countSensors; i++) {
        sensors.setResolution(sensorsUnique[i], 12);
    }

    currentTime = millis();
    loopTime = currentTime;

}

void loop(){
    // переменная для хранения температуры
    float temperature[10];
    // отправляем запрос на измерение температуры всех сенсоров
    sensors.requestTemperatures();
    // считываем данные из регистра каждого датчика по очереди
    for (int i = 0; i < countSensors; i++) {
        temperature[i] = sensors.getTempCByIndex(i);
    }


    // Serial.print("Temp C: ");
    // Serial.println(temperature);
    // ждём одну секунду
    // выводим температуру в Serial-порт по каждому датчику
    for (int i = 0; i < countSensors; i++) {
        Serial.print("Device ");
        Serial.print(i);
        Serial.print(" Temp C: ");
        Serial.print(temperature[i]);
        Serial.println();
    }
    Serial.println();
    currentTime = millis();
    if(currentTime >= (loopTime + 1000))
    {


        loopTime = currentTime;
        literpermin = (pulse_frequency / 7.5);
        pulse_frequency = 0;
        // Serial.print(literperhour, DEC);
        // Serial.println(" Liter/hour");
        lcd.setCursor(0, 0);              // Устанавливаем курсор в начало 1 строки
        lcd.print("In:  ");
        lcd.print(temperature[0]);       // Выводим текст
        lcd.print(" C ");
        lcd.rightToLeft();
        Serial.print(literpermin);
        if (literpermin <10)
        {
            lcd.setCursor(14,0);
            lcd.print(" ");
        }else if (literpermin<100)
        {
            lcd.setCursor(13,0);
            lcd.print(" ");
        }
        lcd.setCursor(15, 0);
        lcd.print(literpermin, DEC);
        lcd.leftToRight();

        lcd.setCursor(0, 1);              // Устанавливаем курсор в начало 2 строки
        lcd.print("Out: ");
        lcd.print(temperature[1]);         //
        lcd.print(" C");

    }
    // ждём одну секунду
    delay(1000);
}