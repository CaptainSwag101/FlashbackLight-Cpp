#ifndef STX_H
#define STX_H

#include <QDataStream>
#include <QVector>

class STX
{
public:
    static STX fromBytes(QDataStream &stream);
    static QByteArray toBytes(STX &stx);
};

#endif // STX_H
