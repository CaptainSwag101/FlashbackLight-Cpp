#include "spcentry.h"

SPCEntry SPCEntry::fromBytes(QDataStream &stream)
{
    SPCEntry result;

    stream >> result.cmpFlag;
    stream >> result.unkFlag;

    int cmpSize = 0;
    stream >> cmpSize;
    int decSize = 0;
    stream >> decSize;
    int nameLen = 0;
    stream >> nameLen;
    stream.skipRawData(0x10);

    // Everything's aligned to multiples of 0x10.
    const int namePadding = (0x10 - (nameLen + 1) % 0x10) % 0x10;
    const int dataPadding = (0x10 - cmpSize % 0x10) % 0x10;

    result.filename = QString::fromUtf8(stream.device()->read(nameLen));
    stream.skipRawData(namePadding + 1);    // nameLen doesn't include the null terminator, so we skip one extra byte here.

    switch (result.cmpFlag)
    {
    case 1: // Uncompressed
        result.data = stream.device()->read(cmpSize);
        break;
    case 2: // Compressed
        result.data = decompress(stream, cmpSize, decSize);
        break;
    case 3: // Data stored externally (is this ever actually used?)
        throw "Not Implemented.";
        break;
    }
    stream.skipRawData(dataPadding);

    return result;
}

QByteArray SPCEntry::toBytes(SPCEntry &entry)
{
    QByteArray result;
    QDataStream stream(&result, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);

    stream << entry.cmpFlag;
    stream << entry.unkFlag;

    int cmpSize = 0;
    int decSize = entry.data.size();
    QByteArray dataToWrite;

    switch (entry.cmpFlag)
    {
    case 1: // Uncompressed
        dataToWrite = entry.data;
        break;
    case 2: // Compressed
        dataToWrite = compress(entry.data);
        break;
    case 3: // Data stored externally (is this ever actually used?)
        throw "Not Implemented.";
        break;
    }
    cmpSize = dataToWrite.size();

    stream << cmpSize;
    stream << decSize;
    QByteArray filename = entry.filename.toUtf8();
    stream << (int)filename.size();
    stream << QByteArray(0x10, 0x00);
    stream << filename;

    // Everything's aligned to multiples of 0x10.
    const int namePadding = (0x10 - (filename.size() + 1) % 0x10) % 0x10;
    const int dataPadding = (0x10 - cmpSize % 0x10) % 0x10;

    stream << QByteArray(namePadding + 1, 0x00);
    stream << dataToWrite;
    stream << QByteArray(dataPadding, 0x00);

    stream.setDevice(0);
    return result;
}

QByteArray SPCEntry::decompress(QDataStream &cmpData, int cmpSize, int decSize)
{
    QByteArray decData;
    decData.reserve(decSize);

    const int endPos = cmpData.device()->pos() + cmpSize;
    int flag = 1;

    while (cmpData.device()->pos() < endPos)
    {
        // We use an 8-bit flag to determine whether something is raw data,
        // or if we need to pull from the buffer, going from most to least significant bit.
        // We reverse the bit order to make it easier to work with.

        if (flag == 1)
        {
            uchar c;
            cmpData >> c;
            // Add an extra "1" bit so our last flag value will always cause us to read new flag data.
            flag = 0x100 | bit_reverse(c);
        }

        if (cmpData.device()->pos() >= endPos)
            break;

        if (flag & 1)
        {
            // Raw byte
            uchar c;
            cmpData >> c;
            decData.append(c);
        }
        else
        {
            // Pull from the buffer
            // xxxxxxyy yyyyyyyy
            // Count  -> x + 2 (max length of 65 bytes)
            // Offset -> y (from the beginning of a 1023-byte sliding window)
            ushort b;
            cmpData >> b;
            const char count = (b >> 10) + 2;
            const short offset = b & 1023;

            for (char i = 0; i < count; ++i)
            {
                const int reverse_index = decData.size() - 1024 + offset;
                decData.append(decData.at(reverse_index));
            }
        }

        flag >>= 1;
    }

    return decData;
}

// First, read from the readahead area into the sequence one byte at a time.
// Then, see if the sequence already exists in the previous 1023 bytes.
// If it does, note its position. Once we encounter a sequence that
// is not duplicated, take the last found duplicate and compress it.
// If we haven't found any duplicate sequences, add the first byte as raw data.
// If we did find a duplicate sequence, and it is adjacent to the readahead area,
// see how many bytes of that sequence can be repeated until we encounter
// a non-duplicate byte or reach the end of the readahead area.
QByteArray SPCEntry::compress(QByteArray &decData)
{
    const int decSize = decData.size();

    QByteArray cmpData;
    cmpData.reserve(decSize);
    QDataStream stream(&cmpData, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    QByteArray block;
    block.reserve(16);

    int pos = 0;
    uchar flag = 0;
    uchar cur_flag_bit = 0;

    // This repeats until we've stored the final compressed block,
    // after we reach the end of the uncompressed data.
    while (true)
    {
        // At the end of each 8-byte block (or the end of the uncompressed data),
        // append the flag and compressed block to the compressed data.
        if (cur_flag_bit == 8 || pos >= decSize)
        {

            flag = bit_reverse(flag);
            stream << flag;
            stream << block;

            block.clear();
            block.reserve(16);

            flag = 0;
            cur_flag_bit = 0;
        }

        if (pos >= decSize)
            break;



        const int lookahead_len = std::min(decSize - pos, 65);
        const QByteArray lookahead = decData.mid(pos, lookahead_len);
        const int searchback_len = std::min(pos, 1024);
        const QByteArray window = decData.mid(pos - searchback_len, searchback_len + (lookahead_len - 1));

        // Find the largest matching sequence in the window.
        int s = -1;
        int l = 1;
        QByteArray seq;
        seq.reserve(65);
        seq.append(lookahead.at(0));
        for ( ; l <= lookahead_len; ++l)
        {
            const int last_s = s;
            if (searchback_len < 1)
                break;

            s = window.lastIndexOf(seq, searchback_len - 1);

            if (s == -1)
            {
                if (l > 1)
                {
                    --l;
                    seq.chop(1);
                }
                s = last_s;
                break;
            }

            if (l == lookahead_len)
                break;

            seq.append(lookahead.at(l));
        }

        // if (seq.size() >= 2)
        if (l >= 2 && s != -1)
        {
            // We found a duplicate sequence
            ushort repeat_data = 0;
            repeat_data |= 1024 - searchback_len + s;
            repeat_data |= (l - 2) << 10;
            QByteArray raw(0x02, 0x00);
            raw[0] = (char)(repeat_data);
            raw[1] = (char)(repeat_data >> 8);
            block.append(raw);
        }
        else
        {
            // We found a new raw byte
            flag |= (1 << cur_flag_bit);
            block.append(seq);
        }


        ++cur_flag_bit;
        // Seek forward to the end of the duplicated sequence,
        // in case it continued into the lookahead buffer.
        pos += l;
    }

    stream.setDevice(0);
    return cmpData;
}
