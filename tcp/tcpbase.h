#ifndef TCPBASE_H
#define TCPBASE_H

#include <tcpprotocol.h>

/*!
 * \brief The TcpBase class
 * В этом классе находятся реализации методов, общих для
 * TcpClient и TcpServer
 */
class TcpBase
{
public:
    TcpBase();
protected:
    void readMessageFromStream(QIODevice *dev);

    QByteArray fullMessage;
    bool continue_message;
    MachineHeader header;
};

#endif // TCPBASE_H
