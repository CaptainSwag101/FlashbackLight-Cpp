#include "spc.h"

const QString MAGIC = "CPS.";
const QString TABLE_MAGIC = "Root";

SPC SPC::fromBytes(QDataStream &stream, int len)
{
    SPC result;

    QString magic = QString::fromUtf8(stream.device()->read(4));

    if (magic == "$CMP")
    {
        stream.device()->seek(stream.device()->pos() - 4);
        QByteArray origData;

        if (len == -1)
            origData = stream.device()->readAll();
        else
            origData = stream.device()->read(len);

        // TODO: Do SRD decompression on the data and then try again
        throw "Not Implented.";
    }

    if (magic != MAGIC)
    {
        throw "Invalid SPC file.";
    }

    result.unk1 = stream.device()->read(0x24);
    uint fileCount = 0;
    stream >> fileCount;
    stream >> result.unk2;
    stream.skipRawData(0x10);

    QString tableMagic = QString::fromUtf8(stream.device()->read(4));
    if (tableMagic != TABLE_MAGIC)
    {
        throw "Invalid SPC file.";
    }
    stream.skipRawData(0x0C);

    for (uint i = 0; i < fileCount; ++i)
    {
        result.files.append(SPCEntry::fromBytes(stream));   // This automatically decompresses the data if needed.
    }

    return result;
}

QByteArray SPC::toBytes(SPC &spc)
{
    QByteArray result;
    QDataStream stream(&result, QIODevice::WriteOnly);

    stream << MAGIC.toUtf8();
    stream << spc.unk1;
    stream << spc.files.count();
    stream << spc.unk2;
    stream << QByteArray(0x10, 0x00);
    stream << TABLE_MAGIC.toUtf8();
    stream << QByteArray(0x0C, 0x00);

    for (SPCEntry &entry : spc.files)
    {
        stream << SPCEntry::toBytes(entry);
    }

    stream.setDevice(0);
    return result;
}
