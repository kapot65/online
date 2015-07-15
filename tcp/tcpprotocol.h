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
 * \brief The ERROR_TYPE enum
 * \details коды ошибок
 */
enum ERROR_TYPE
{
    CLIENT_NO_ERROR = 0,
    UNKNOWN_ERROR = 1,
    SERVER_INIT_ERROR = 2, //сервер не ицициализирован
    CLIENT_DISCONNECT = 3, //сервер отключиля или не подключен
    PARSE_MESSAGE_ERROR = 4, //ошибка при парсинге сообщения
    TIMEOUT_ERROR = 5, //превышение времени обработки запроса
    ALGORITM_ERROR = 6, //ошибка в алгоритме исполнения команды
    UNKNOWN_MESSAGE_ERROR = 7, //у сервера нету действия для данного сообщения
    SERVER_BUSY_ERROR = 8, //сервер сейчас занят
    INCORRECT_MESSAGE_PARAMS = 9 //неправильные параметры в сообщении
};

enum METATYPE
{
    UNDEFINED_METATYPE = 0,
    JSON_METATYPE = 1
};

enum BINARYTYPE
{
    UNDEFINED_BINARY = 0,
    DIRECT_BINARY = 1,
    QDATASTREAM_BINARY = 7
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
     * \brief readMachineHeader
     * \param message
     * \param ok
     * \return
     */
    static MachineHeader readMachineHeader(QByteArray &message, bool *ok = 0);
    /*!
     * \brief createMessage
     * \details метод создает сообщение в соотвествии с форматом протокола
     * \param meta
     * \param data
     * \return
     */
    static QByteArray createMessage(QVariantMap meta, QByteArray data, unsigned int metaType = 0x01, unsigned int binaryType = 0);
    /*!
     * \brief parceMesssage
     * \details метод пытается считать сообщение в соотвествии с форматом протокола
     * \param message
     * \param meta
     * \param data
     * \return в случае успешного парсинга возвращает 1, в противном случае возвращает 0
     */
    static bool parceMesssage(QByteArray message, QVariantMap &meta, QByteArray &data, bool headerOnly = 0);
    /*!
     * \brief функция создает бинарный хедер в соответсвии с форматом
     * \param header
     * \return
     */
    static QByteArray writeMachineHeader(MachineHeader header);

    /*!
     * \brief wrapErrorInfo
     * \details метод оборертывает описание ошибки, добавляя поля, по которым метод TcpProtocol::checkMessageForError
     * может опознать сообщение как ошибку
     * \param error_info
     * \return
     */
    static QVariantMap wrapErrorInfo(QVariantMap error_info);

    /*!
     * \brief unwrapErrorInfo
     * \details убирает из сообщения поля, созданные TcpProtocol::wrapErrorInfo
     * \param error_info
     * \return
     */
    static QVariantMap unwrapErrorInfo(QVariantMap error_info);


    /*!
     * \brief checkMessageForError
     * \details Проверяет, является ли сообщение, сообщением об ошибке.
     * Чтобы фунция работала, нужно оборачивать описание ошибок методом TcpProtocol::wrapErrorInfo
     * \param message
     * \return
     */
    static bool checkMessageForError(QVariantMap message);

    /*!
     * \brief createMessageWithPoints
     * \details сериализация сообщения содержащего события точек
     * \param meta метаданные
     * \param events вектор событий
     * \param metaType тип метаданных
     * \param binaryType тип бинарных данных
     * \return
     */
    static QByteArray createMessageWithPoints(QVariantMap meta, QVector<Event> events,
                                              unsigned int metaType = JSON_METATYPE, unsigned int binaryType = UNDEFINED_BINARY);

    static bool parceMessageWithPoints(MachineHeader messageHeader, QVariantMap messageMeta, QByteArray messageData, QVector<Event> &events);
    static bool parceMessageWithPoints(QByteArray message, QVariantMap &meta, QVector<Event> &events);
    static QMap<int, unsigned short> getAviableMeasuteTimes();
};

#endif // TCPPROTOCOL_H
