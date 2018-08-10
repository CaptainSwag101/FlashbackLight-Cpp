#ifndef SPCENTRY_H
#define SPCENTRY_H

#include <QDataStream>
#include <QString>
#include <QVector>

class SPCEntry
{
public:
    static SPCEntry fromBytes(QDataStream &stream);
    static QByteArray toBytes(SPCEntry &entry);

    QString filename;
    QByteArray data;
    ushort cmpFlag;
    ushort unkFlag;

private:
    static QByteArray decompress(QDataStream &cmpData, int cmpSize, int decSize);
    static QByteArray compress(QByteArray &decData);
    static inline uchar bit_reverse(uchar b)
    {
        return (uchar)((b * 0x0202020202 & 0x010884422010) % 1023);
    }
};

#endif // SPCENTRY_H
