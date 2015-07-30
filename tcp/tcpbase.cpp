#include "tcpbase.h"
#include <easylogging++.h>

TcpBase::TcpBase()
{
    continue_message = 0;
}

void TcpBase::readMessageFromStream(QIODevice *dev,
                                    MachineHeader &header,
                                    QVariantMap &meta,
                                    QByteArray &data,
                                    bool &ok,
                                    bool &hasMore)
{
    //чтение посылки
    QByteArray message;
    if(!continue_message)
    {
        message += dev->read(30);
        //попытка считать бинарный заголовок
        //попытка пробуется на каждом пакете, чтобы избежать поломки сервера
        //в случае когда в бинарном хедере и в фактическом сообщении различаются длины
        this->header = TcpProtocol::readMachineHeader(message, &ok);
        if(ok)
        {
            fullMessage.clear();
            continue_message = 1;
        }
        else
            if(!continue_message)
            {
                //поток чем-то забит
                //очистка всего потока
                LOG(ERROR) << "Error parcing tcp stream: can not parse binary header. Clearing all stream.";
                dev->readAll();
                hasMore = false;
                return;
            }
    }

    if(continue_message)
    {
        message += dev->read((this->header.metaLength + this->header.dataLenght + 30) -
                                   (fullMessage.size() + message.size()));
        fullMessage.push_back(message);

        if(fullMessage.size() >= this->header.metaLength + this->header.dataLenght + 30)
        {
            //Сообщение собрано. Попытка распарсить его.
            continue_message = 0;

            //попытка распарсить сообщение
            if(!(TcpProtocol::parceMessage(fullMessage, meta, data)))
                ok = false;
            else
                ok = true;

            if(dev->size())
                hasMore = true;
            else
                hasMore = false;

            header = this->header;
        }
    }
}
