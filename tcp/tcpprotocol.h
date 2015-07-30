#ifndef TCPPROTOCOL_H
#define TCPPROTOCOL_H

#include <QByteArray>
#include <QVariantMap>
#ifdef USE_QTJSON
#include <QJsonObject>
#include <QJsonDocument>
#else
#include <QJson/Parser>
#include <QJson/Serializer>
#endif

#include <QVector>
#include "event.h"

/*!
 * \brief Коды ошибок серверов.
 */
enum ERROR_TYPE
{
    CLIENT_NO_ERROR = 0,
    UNKNOWN_ERROR = 1,
    SERVER_INIT_ERROR = 2, ///сервер не ицициализирован
    CLIENT_DISCONNECT = 3, ///сервер отключиля или не подключен
    PARSE_MESSAGE_ERROR = 4, ///ошибка при парсинге сообщения
    TIMEOUT_ERROR = 5, ///превышение времени обработки запроса
    ALGORITM_ERROR = 6, ///ошибка в алгоритме исполнения команды
    UNKNOWN_MESSAGE_ERROR = 7, ///у сервера нету действия для данного сообщения
    SERVER_BUSY_ERROR = 8, ///сервер сейчас занят
    INCORRECT_MESSAGE_PARAMS = 9, ///неправильные параметры в сообщении
    MULTIPLE_CONNECTION = 10 ///попытка подключится к серверу, у которого уже есть активное соединение
};

/*!
 * \brief Форматы метаданных.
 */
enum METATYPE
{
    UNDEFINED_METATYPE = 0x00000000, ///Неизвестный формат метаданных
    JSON_METATYPE = 0x00010000 ///Формат Json
};

/*!
 * \brief Формат записи бинарных данных.
 */
enum BINARYTYPE
{
    UNDEFINED_BINARY = 0x00000000, ///Неопределенный формат
    POINT_DIRECT_BINARY = 0x00000100, ///Точка в обычной сериализации
    POINT_QDATASTREAM_BINARY = 0x00000107, ///Точка в сериализации через QDataStream
    HV_BINARY = 0x00000200, ///Бинарный формат HV
    HV_TEXT_BINARY = 0x00000201, ///тексовый формат HV
};

/*!
 * \brief
 * Внутренний формат бинарного заголовка.
 * */
struct MachineHeader
{
    MachineHeader();
    unsigned int time;
    unsigned int type;
    unsigned int metaType;
    unsigned int metaLength;
    unsigned int dataType;
    unsigned int dataLenght;
};
/*!
 * \brief The TcpProtocol class
 * \details В этом классе инкапсулированны все методы, которые относятся к протоколу общения между TcpClient
 * и ТcpServer
 */
class TcpProtocol
{
public:
    /*!
     * \brief Метод считывает бинарный хедер.
     * \param message
     * \param ok
     * \return
     */
    static MachineHeader readMachineHeader(QByteArray &message, bool *ok = 0);

    /*!
     * \brief Метод создает сообщение в соотвествии с форматом протокола
     * http://elog.mass.inr.ru/online/1 .
     * \param meta
     * \param data
     * \return
     */
    static QByteArray createMessage(QVariantMap meta, QByteArray data = QByteArray(),
                                    unsigned int metaType = JSON_METATYPE,
                                    unsigned int binaryType = UNDEFINED_BINARY);

    /*!
     * \brief Метод пытается считать сообщение в соотвествии с форматом протокола
     * \param message
     * \param meta
     * \param data
     * \return в случае успешного парсинга возвращает 1, в противном случае возвращает 0
     */
    static bool parceMessage(QByteArray message, QVariantMap &meta, QByteArray &data, bool headerOnly = 0);

    /*!
     * \brief Функция создает бинарный хедер в соответсвии с форматом
     * \param header
     * \return
     */
    static QByteArray writeMachineHeader(MachineHeader header);

    /*!
     * \brief Метод оборертывает описание ошибки, добавляя поля, по которым метод TcpProtocol::checkMessageForError
     * может опознать сообщение как ошибку
     * \param error_info
     * \return
     */
    static QVariantMap wrapErrorInfo(QVariantMap error_info);

    /*!
     * \brief Убирает из сообщения поля, созданные TcpProtocol::wrapErrorInfo
     * \param error_info
     * \return
     */
    static QVariantMap unwrapErrorInfo(QVariantMap error_info);


    /*!
     * \brief Проверяет, является ли сообщение, сообщением об ошибке.
     * Чтобы фунция работала, нужно оборачивать описание ошибок методом TcpProtocol::wrapErrorInfo
     * \param message
     * \return
     */
    static bool checkMessageForError(QVariantMap message);

    /*!
     * \brief Сериализация сообщения содержащего события точек
     * \param meta метаданные
     * \param events вектор событий
     * \param metaType тип метаданных
     * \param binaryType тип бинарных данных
     * \return
     */
    static QByteArray createMessageWithPoints(QVariantMap meta, QVector<Event> events,
                                              unsigned int metaType = JSON_METATYPE, unsigned int binaryType = UNDEFINED_BINARY);

    /*!
     * \brief Метод парсит сообщение с бинарными данными точки, создаваемое методом
     * TcpProtocol::createMessageWithPoints. На вход принимается уже разобранное сообщение.
     * \param messageHeader
     * \param messageMeta
     * \param messageData
     * \param events Вектор выходных точек.
     * \return
     */
    static bool parceMessageWithPoints(MachineHeader messageHeader, QVariantMap messageMeta, QByteArray messageData, QVector<Event> &events);
    /*!
     * \brief Метод парсит сообщение с бинарными данными точки, создаваемое методом
     * TcpProtocol::createMessageWithPoints. На вход принимается неразобранное сообщение.
     * \param message
     * \param meta
     * \param events Вектор выходных точек.
     * \return
     */
    static bool parceMessageWithPoints(QByteArray message, QVariantMap &meta, QVector<Event> &events);

    /*!
     * \brief Метод выдает список доступных времен сбора для CAMAC.
     * \return
     */
    static QMap<int, unsigned short> getAviableMeasuteTimes();

    /*!
     * \brief Возвращает коэффициент перевода сырого времени с MADS
     * во время в наносекундах
     * \param measureTime Время измерения. Подгоняется под доступные времена
     * сбора из TcpProtocol::getAviableMeasuteTimes.
     * \return Коэффициент перевода.
     */
    static double madsTimeToNSecCoeff(int measureTime);
};

#endif // TCPPROTOCOL_H
