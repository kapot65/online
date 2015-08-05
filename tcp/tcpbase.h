#ifndef TCPBASE_H
#define TCPBASE_H

#include <tcpprotocol.h>

/*!
 * \brief В этом классе находятся реализации методов, общих для
 * TcpClient и TcpServer
 */
class TcpBase
{
public:
    TcpBase();
protected:
    void readMessageFromStream(QIODevice *dev,
                               MachineHeader &header,
                               QVariantMap &meta,
                               QByteArray &data,
                               bool &ok,
                               bool &hasMore);

    /*!
     * \brief Текущее собираемое сообщение.
     */
    QByteArray fullMessage;

    /*!
     * \brief Флаг продолжения чтения сообщения.
     */
    bool continue_message;

    /*!
     * \brief Машиночитаемый заголовок текущего сообщения.
     */
    MachineHeader header;
};

#endif // TCPBASE_H
