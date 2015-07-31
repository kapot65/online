/*! \mainpage Программный комплекс Online
 *
 * \section Введение
 * Программный комплекс Online предназначен для набора данных на установке Троицк ню масс. Комплекс написан на Qt 5.4 (4.8 для CCPC7_Server)
 * с использованием сторонних библиотек QJson, QCustomPlot и easylogging++. Комплекс состоит из 4 программ:
 * CCPC7_Server - программа для взаимодействия с CCPC. Устанавливается на CCPC. Программа осуществляет возможность управления CCPC
 * через сеть и реализует функции, необходимые для сбора, такие как:
 * - Инициализация CCPC.
 * - Сбор точки.
 * - Остановка сбора точки.
 * - Выполнение команды NAF (в работе не используется, необходимо для тестирования).
 * - Внутреннее управление CCPC происходит через API, поставляемое с машиной.
 *
 * HV_Server - программа для управления высоким напряжением. Ставится на компьютер, контролирующий высокое напряжение. Так же как и
 * CCPC7_Server программа осуществляет возможность управления высоким напряжением через сеть. Программа позволяет:
 * - Устанавливать напряжение на блоке.
 * - Считывать напряжение с блока.
 * - Внутреннее управление высоким напряжением осуществляется через общение с блоком через Com порты.
 *
 * CamacClient - Основная программа, через нее проводится управление сбором. Программа подключается к CCPC7_Server и HV_Server
 * и с помощью них проводит управление набором. CamacClient общается с CCPC7_Server и HV_Server по средствам TCP в формате
 * http://elog.mass.inr.ru/online/1. Программа позволяет:
 * - Установить сценарий сбора.
 * - Провести одно или несколько повторений сбора по сценарию со сменой или без смены направления прохода.
 * - Провести предварительную визуализацию собираемых данных.
 * - Выводить текущую информацию о сборе.
 * - Контролировать процесс сбора (ставить на паузу, останавливать сбор. Делать текущее повторение последним).
 *
 * DataVisualizer. Необязательная программа. Позволяет визуализировать уже набранные данные сценарии.
 *
 * \todo Дополнить титульный лист.
 * \todo Поменять название CamacClient.
 * \todo Поменять название CamacServer.
 * \todo Добавить описание алгоритма сбора точки.
 */


#include <QApplication>
#include <QDir>
#include <QDateTime>
#include <datavisualizerform.h>
#include "camacclientform.h"

// настройки логгера
INITIALIZE_EASYLOGGINGPP
#define LOG_DIRECTORY "D:\\Logs\\CamacClient\\"

int main(int argc, char *argv[])
{
    START_EASYLOGGINGPP(argc, argv);

    QDateTime curr_datetime = QDateTime::currentDateTime();

    el::Loggers::reconfigureAllLoggers(el::ConfigurationType::Filename,
                                         (LOG_DIRECTORY +
                                          curr_datetime.toString("yyyyMMdd-hhmmss.zzz")).toStdString());
    LOG(INFO) << "Programm started";

    QApplication a(argc, argv);
    /*
    MainWindow w;
    w.show();
    /*/

    CamacClientForm cF;
    cF.showMaximized();
    //SeverTester st;
    //st.show();
    //AlgoritmForm af;
    //af.show();
    //*/

    
    return a.exec();
}
