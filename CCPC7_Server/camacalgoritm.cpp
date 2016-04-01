#include "camacalgoritm.h"

#if defined(TEST_MODE) || defined(VIRTUAL_MODE)
    #include <QDebug>
#endif

CamacAlgoritm::CamacAlgoritm(QObject *parent) : CCPCCommands(), QObject(parent)
{
    breakFlag = 0;

    //connect(camac, SIGNAL(have_error(QString)), this, SLOT(process_error(QString)));
    aviableMeasureTimes = TcpProtocol::getAviableMeasuteTimes();
}

void CamacAlgoritm::on_breakAcquisition()
{
    breakFlag = 1;
#ifdef VIRTUAL_MODE
    emit breakAcquisition();
#endif
}

void CamacAlgoritm::resetCounters()
{
    unsigned short none = 0;

    //обнуление всех счетчиков
#ifdef TEST_MODE
    qDebug() << "Reseting counters";
#endif

    none  = 0;
    for(int j = 0; j < 7; j++)
    {
        NAF(settings->getCOUNTER1(), j, 2,none);
        NAF(settings->getCOUNTER2(), j, 2,none);
    }
}

unsigned int CamacAlgoritm::getCounterValue(int counterNum, int channelNum, bool withReset)
{
    int counter;
    switch (counterNum)
    {
    case 1:
        counter = settings->getCOUNTER1();
        break;
    case 2:
        counter = settings->getCOUNTER2();
        break;
    default:
        return (-1);
    }

    //проверка правильности параметров
    if(channelNum < 0 || channelNum > 3)
        return (-1);

    int f = 0;

    if(withReset)
        f = 2;

    long fullData = 0;

#ifndef VIRTUAL_MODE
    unsigned short data = 0;
    NAF(counter, channelNum, f, data);
#ifdef TEST_MODE
    qDebug() << QString("Counter: %1, A: %2, data: %3").arg(counter)
                .arg(channelNum).arg(data);
#endif

    data = 0;
    NAF(counter, channelNum + 4, f, data);

#ifdef TEST_MODE
    qDebug() << QString("Counter: %1, A: %2, data: %3").arg(counter)
                .arg(channelNum).arg(data);
#endif

    fullData += 65536 * data;
#else
    fullData = qrand() % 4096;
#endif

    return fullData;
}

QVector<Event> CamacAlgoritm::acquirePoint(int measureTime, bool *manuallyBreak)
{
    breakFlag = 0;

    unsigned short none = 0;

    //установка TLL_NIM  в состояние
#ifdef TEST_MODE
    qDebug() << "Disabling TTL";
#endif

    unsigned short data = 0xFFFF;
#ifndef VIRTUAL_MODE
    NAF(settings->getTTL_NIM(), 0, 17, data);

    //отключение измерений
    disableMeasurement();
#endif

    //установка MADC в режим измерения
#ifdef TEST_MODE
    qDebug() << "Setting MADS to measuring mode";
#endif

    long MADCaddr = 0;
#ifndef VIRTUAL_MODE
    setMADCAddr(MADCaddr, measureTime);
#endif

    //обнуление всех счетчиков
#ifdef TEST_MODE
    qDebug() << "Reseting counters";
#endif

    none  = 0;

#ifndef VIRTUAL_MODE
    for(int j = 0; j < 7; j++)
    {
        NAF(settings->getCOUNTER1(), j, 2,none);
        NAF(settings->getCOUNTER2(), j, 2,none);
    }
    //включение измерений
    enableMeasurement();
#endif

    //задержка 10 мс (нужна?)
    waitMSec(10);

#ifdef TEST_MODE
    qDebug() << "Enable all outputs in OV1, Up Channel";
#endif

#ifndef VIRTUAL_MODE
    data = 0x1F;
    NAF(settings->getOV1(), 0, 16, data);
#endif

#ifdef TEST_MODE
    qDebug() << "Enable all outputs in OV1, Down Channel";
#endif

#ifndef VIRTUAL_MODE
    data = 0x0F;
    NAF(settings->getOV1(), 1, 16, data);
#endif

#ifdef TEST_MODE
    qDebug() << "\"Start\" pulse";
#endif

#ifndef VIRTUAL_MODE
    NAF(settings->getOV1(), 0, 25, none);
#endif

    //цикл сбора
#ifndef VIRTUAL_MODE
    long addr;
    bool addrOverflow;
    bool endOfMeasurement;
    long total_events = 0;
    do
    {
        //задержка 1 сек
        waitMSec(1000);
        getMADCAddr(addr, addrOverflow, endOfMeasurement);
        //количество зафиксированных частиц на текущий момент
        total_events = addr;
        LOG(INFO) << tr("Total events = %1, aovfl = %2, EoM = %3")
                     .arg(total_events).arg(addrOverflow)
                     .arg(endOfMeasurement).toStdString();

        emit currentEventCount(total_events);
    }
    while(!addrOverflow && !endOfMeasurement && !breakFlag);
#else
    int eventsInSecond = 20; //количество генерируемых событий в секунду
    //Ожидание времени сбора или флага остановки.
    for(int i = 0; i < measureTime; i++)
    {
        qDebug() << tr("Simulate acquisition: %1 total events").arg(eventsInSecond * i);
        emit currentEventCount(eventsInSecond * i);
        waitMSec(1000);
    }
#endif

    if(manuallyBreak)
        if(breakFlag)
            manuallyBreak[0] = 1;
        else
            manuallyBreak[0] = 0;

    breakFlag = 0;

#ifdef VIRTUAL_MODE
    //Имитация набранных событий
    QVector<Event> events;
    //получение коэффициента перевода из сырого времени MADS в наносекунды
    double coeff = TcpProtocol::madsTimeToNSecCoeff(measureTime);

    for(int i = 0; i < measureTime * eventsInSecond; i++)
    {
        Event e;
        e.data = 500 + qrand() % 2000;
        e.time = ((double)i / (double)eventsInSecond) * (qPow(10, 9) / coeff);
        e.valid = true;

        events.push_back(e);
    }
#else
    //считывание данных
    //отключение измерения
    disableMeasurement();
    getMADCAddr(addr, addrOverflow, endOfMeasurement);
    //полное количество зафиксированных частиц
    total_events = addr;
    emit currentEventCount(total_events);
#ifdef TEST_MODE
    qDebug() << tr("Acquisition of point done. Total events = %1").arg(total_events);
#endif

    //установка времени сбора (?) 2115
#ifdef TEST_MODE
    qDebug() << "Setting measurement time (?)";
#endif

    long madcAddr = 0;
    setMADCAddr(madcAddr, measureTime);


    //очистка прошлых данных
    QVector<Event> events;
#ifdef TEST_MODE
    qDebug() << "Reading acquired data from CamacAlgoritm";
#endif

    for(int j =0; j < addr; j++)
    {

        unsigned short data;
        long time;
        bool valid;
        readMADC(data, time, valid);

        events.push_back(Event(data, time, valid));

        /*
        if(valid)   //{ ADC & Time, True Codes }
        {

            Save(Chr($10 OR BYTE(DATA DIV 256)));
            Save(Chr(BYTE(DATA MOD 256)));
            Inc(D[DATA]);
            Inc(T[Time DIV $80000]);
            FOR i := 1 TO 4 DO
                BEGIN
                    Save(Chr(BYTE(TIME MOD 256)));
                    TIME := TIME DIV 256;
                END;
            Inc(Ev_Dig);
            IF (DATA < First_Channel) THEN Inc(Ev_Low)
            ELSE
                BEGIN
                    IF (DATA > Last_Channel) THEN Inc(Ev_Up)
                    ELSE Inc(Ev_Gate)
                END;
        }
        else //{ Time Only, False ADC Code }
        {
            Save(#$2F);
            Inc(T[Time DIV $80000]);
            FOR i := 1 TO 4 DO
                BEGIN
                    Save(Chr(BYTE(TIME MOD 256)));
                    TIME := TIME DIV 256;
                END;
            Inc(Ev_Skip);

        }
        */

    }

/*
        FOR li := 1 TO ADDR DO
            BEGIN
                READ_MADC_DATA(DATA, VALID);
                IF VALID THEN                  { ADC Only, True Code }
                    BEGIN
                        Save(Chr($40 OR BYTE(DATA DIV 256)));
                        Save(Chr(BYTE(DATA MOD 256)));
                        Inc(D[DATA]);
                        Inc(Ev_Dig);
                        IF (DATA < First_Channel) THEN Inc(Ev_Low)
                        ELSE
                            BEGIN
                                IF (DATA > Last_Channel) THEN Inc(Ev_Up)
                                ELSE Inc(Ev_Gate)
                            END;
                    END
                ELSE                           { ADC Only, False Code }
                    BEGIN
                        Save(#$8F);
                        Inc(Ev_Skip);
                    END;
            END
*/

        //обработка overflowAddress
        //IF OVFL_ADDR THEN WriteLn('* Out Of Memory *');


        //считывание каунтеров
/*
        for(int j =0; j < 3; j++)
        {
            unsigned short data = 0;
            NAF(settings->getCOUNTER1(), j, 2, data);

//            Save(Chr(DATA MOD 256));
//            Save(Chr(DATA DIV 256));
//            Cn[1, i+1] := 1.0*(DATA);


            data = 0;
            NAF(settings->getCOUNTER1(), j + 4, 2, data);

//            Save(Chr(DATA MOD 256));
//            Save(Chr(DATA DIV 256));
//            Cn[1, i+1] := Cn[1, i+1] + 65536.0 * DATA;

        }
        for(int j =0; j < 3; j++)
        {
            unsigned short data = 0;
            NAF(settings->getCOUNTER2(), j, 2, data);

//            Save(Chr(DATA MOD 256));
//            Save(Chr(DATA DIV 256));
//            Cn[2, i+1] := 1.0*DATA;


            data = 0;
            NAF(settings->getCOUNTER2(), j + 4, 2, data);
//            Save(Chr(DATA MOD 256));
//            Save(Chr(DATA DIV 256));
//            Cn[2, i+1] := Cn[2, i+1] + 65536.0 * DATA;

        }
*/
#endif
    return events;
}

void CamacAlgoritm::disableMeasurement()
{
#ifdef TEST_MODE
    qDebug() << "Disabling measurement";
#endif
    unsigned short data;
    NAF(settings->getMADC(), 0, 12, data);
}

void CamacAlgoritm::enableMeasurement()
{
#ifdef TEST_MODE
    qDebug() << "Enabling measurement";
#endif
    unsigned short data;
    NAF(settings->getMADC(), 0, 11, data);
}

void CamacAlgoritm::writeMADC(unsigned short &data, long &time)
{
    unsigned short tw = time%65636;
    NAF(settings->getMADC(), 0, 16, tw);
    tw = (time & 0x0FFFFFFF)%65536;
    NAF(settings->getMADC(), 1, 16, tw);
    tw = data & 0x0FFF;
    NAF(settings->getMADC(), 2, 16, tw);
}

void CamacAlgoritm::readMADC(unsigned short &data, long &time, bool &valid)
{
    /*
    //чтение времени
    time = 0;
    unsigned short tw = 0;
    NAF(settings->getMADC(), 0, 0, tw);
    //qDebug() << QString("%1").arg(tw, 16, 2, QChar('0'));
    time = tw;
    tw = 0;
    NAF(settings->getMADC(), 1, 0, tw);
    //qDebug() << QString("%1").arg(tw, 16, 2, QChar('0'));
    time += (((long)tw) << 16);
    //qDebug() << QString("%1").arg(time, 32, 2, QChar('0'));

    //чтение данных
    tw = 0;
    NAF(settings->getMADC(), 2, 0, tw);
    data = tw;
    //qDebug() << QString("%1").arg(tw, 16, 2, QChar('0'));
    valid  = checkBit(tw, 15);
    replaceBit(data, 15, 0);
    //обнуление 12 13 14 бит на всякий случай
    replaceBit(data, 14, 0);
    replaceBit(data, 13, 0);
    replaceBit(data, 12, 0);
    /*/
    unsigned short tw;
    long tl;

    NAF(settings->getMADC(), 0, 0, tw);
    time = tw;

    NAF(settings->getMADC(), 1, 0, tw);
    tl = tw & 0x0FFF;
    time = tl * 65536 + time;

    NAF(settings->getMADC(), 2, 0, data);

    if ((data & 0x8000) == 0)
        valid = true;
    else
        valid = false;

    data = data & 0x0FFF;
    //*/

}

void CamacAlgoritm::readMADCData(unsigned short &data, long &time, bool &valid)
{
    NAF(settings->getMADC(), 2, 0, data);

    if ((data & 0x8000) == 0)
        valid = true;
    else
        valid = false;

    data = data & 0x0FFF;
}

void CamacAlgoritm::setMADCAddr(long &addr, int measureTime)
{
    QMap<int, unsigned short>::iterator it = aviableMeasureTimes.lowerBound(measureTime);

    if(measureTime != it.key())
        LOG(WARNING) << "Measure time " << measureTime << " is not correct, changing to " << it.key();

    unsigned short tw;

    tw = it.value();

    tw = tw + (unsigned short)((addr & 0x30000) / 65536);

    //tw = qToBigEndian(tw);

    NAF(settings->getMADC(), 1, 17, tw);
#if QT_VERSION >= 0x050300
    QThread::msleep(10);
#elif QT_VERSION >= 0x040800
    timer->start(10);
    eventLoop->exec();
#endif
    tw = addr % 65536;
    NAF(settings->getMADC(), 0, 17, tw);
}

void CamacAlgoritm::getMADCAddr(long &addr, bool &addrOverflow, bool &endOfMeasurement)
{
    /*
    //проверка адреса
    unsigned short finalAdress = 0;
    NAF(settings->getMADC(), 1, 1, finalAdress);

    if(checkBit(finalAdress, 2))
        addrOverflow = true;
    else
        addrOverflow = false;

    if (checkBit(finalAdress, 3))
        endOfMeasurement = true;
    else
        endOfMeasurement = false;

    unsigned short finalAdress2 = 0;
    NAF(settings->getMADC(), 0, 1, finalAdress2);


    //склеивание адресса
    addr = 0;
    addr = finalAdress2;
    replaceBit(addr, 16, checkBit(finalAdress, 0));
    replaceBit(addr, 17, checkBit(finalAdress, 1));

    /*/
    unsigned short tw;

    NAF(settings->getMADC(), 1, 1, tw);

#ifdef TEST_MODE
    qDebug() << QString("%1").arg(tw, 16, 2, QChar('0'));
#endif

    addr = tw & 0x0003;

    if ((tw & 0x0004) != 0)
         addrOverflow = true;
    else
         addrOverflow = false;

    if ((tw & 0x0008) != 0)
        endOfMeasurement = true;
    else
        endOfMeasurement = false;

    NAF(settings->getMADC(), 0, 1, tw);
#ifdef TEST_MODE
    qDebug() << QString("%1").arg(tw, 16, 2, QChar('0'));
#endif

    addr = addr * 65536 + tw;
    //*/
}

void CamacAlgoritm::testMADC()
{
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));

    unsigned short none = 0;

    //установка TLL_NIM  в состояние
#ifdef TEST_MODE
    qDebug() << "Disabling TTL";
#endif

    unsigned short data = 0xFFFF;
    NAF(settings->getTTL_NIM(), 0, 17, data);

    //установка в режим TBIP
    NAF(settings->getMADC(), 0, 12, none);
    none = 0;

    int count = 1000;
    //установка памяти на ноль
    data = 0;
    NAF(settings->getMADC(), 0, 17, data);
    data = 0x4; //00100; //время измерения 5 сек
    NAF(settings->getMADC(), 1, 17, data);

    QVector<unsigned short> values1;
    QVector<unsigned short> values2;
    QVector<unsigned short> values3;

    for(int i = 0; i < count; i++)
    {
        values1.push_back((unsigned short)qrand());
        values2.push_back((unsigned short)qrand());
        values3.push_back((unsigned short)qrand());
    }

    //ручная запись слов
    for(int i = 0; i < count; i++)
    {
        NAF(settings->getMADC(), 0, 16, values1[i]);
        NAF(settings->getMADC(), 1, 16, values2[i]);
        NAF(settings->getMADC(), 2, 16, values3[i]);
    }

    //установка памяти на ноль для считывания
    data = 0;
    NAF(settings->getMADC(), 0, 17, data);
    data = 0x4; //00100; //время измерения 5 сек
    NAF(settings->getMADC(), 1, 17, data);
    //проверка записанных слов
    for(int i = 0; i < count; i++)
    {
        unsigned short F0A0 = 0;
        unsigned short F0A1 = 0;
        unsigned short F0A2 = 0;

        NAF(settings->getMADC(), 0, 0, F0A0);
        NAF(settings->getMADC(), 1, 0, F0A1);
        NAF(settings->getMADC(), 2, 0, F0A2);

        if((F0A0 != values1[i])&&
           (F0A1 != values2[i])&&
           (F0A2 != values3[i]))
            int a = 0;
   }

    //проверка адреса
    unsigned short finalAdress = 0;
    NAF(settings->getMADC(), 1, 1, finalAdress);
    unsigned short finalAdress2 = 0;
    NAF(settings->getMADC(), 0, 1, finalAdress2);
}

