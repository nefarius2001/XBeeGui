#include "Flashbuffer.h"
#include <QFile>
#include <QDebug>

Flashbuffer::Flashbuffer(int iOffset, int iSize)
{
    this->offset=iOffset;
    this->size=iSize;
    this->buffer=QByteArray(iSize,0xFF);
    qDebug() << "creating flashbuffer, emptycrc = " << QString().sprintf("0x%08X",GetCrc());
}
uint32_t Flashbuffer::Crc32(uint32_t Crc, uint32_t Data)
{
  int i;

  Crc = Crc ^ Data;

  for(i=0; i<32; i++)
    if (Crc & 0x80000000)
      Crc = (Crc << 1) ^ 0x04C11DB7; // Polynomial used in STM32
    else
      Crc = (Crc << 1);

  return(Crc);
}


quint32 Flashbuffer::GetCrc(){
    quint32 crc;
    const char* p8=buffer.constData();
    const quint32* p32;
    int iFor;

    crc=0;
    for(iFor=0;iFor<this->size;iFor+=4){
        p32=(const quint32*) &p8[iFor];
        crc=Crc32Fast(crc,*p32);
    }
    return crc;
}

uint32_t Flashbuffer::Crc32Fast(uint32_t Crc, uint32_t Data)
{
  static const uint32_t CrcTable[16] = { // Nibble lookup table for 0x04C11DB7 polynomial
    0x00000000,0x04C11DB7,0x09823B6E,0x0D4326D9,0x130476DC,0x17C56B6B,0x1A864DB2,0x1E475005,
    0x2608EDB8,0x22C9F00F,0x2F8AD6D6,0x2B4BCB61,0x350C9B64,0x31CD86D3,0x3C8EA00A,0x384FBDBD };

  Crc = Crc ^ Data; // Apply all 32-bits

  // Process 32-bits, 4 at a time, or 8 rounds

  Crc = (Crc << 4) ^ CrcTable[Crc >> 28]; // Assumes 32-bit reg, masking index to 4-bits
  Crc = (Crc << 4) ^ CrcTable[Crc >> 28]; //  0x04C11DB7 Polynomial used in STM32
  Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
  Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
  Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
  Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
  Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
  Crc = (Crc << 4) ^ CrcTable[Crc >> 28];

  return(Crc);
}

int Flashbuffer::readIntelHex(QString sFile){
    QFile f(sFile);
    QString l;
    uint8_t tmp;
    uint8_t crc;
    uint32_t flashextend=0;
    uint32_t flashaddr;
    uint32_t bufferaddr; //temp
    int linenr=0;
    int iFor;
    bool useline;
    int ret=0;
    if(!f.exists()){
        qDebug() << "file does not exist "<<sFile;
        return -1;
    }
    if(!f.open(QIODevice::ReadOnly| QIODevice::Text)){
        qDebug() << "cannot open file"<<sFile;
        return -2;
    }
    while(1){
        l=f.readLine(); linenr++; useline=true;
        l.replace("\n","");
        //qDebug() << "line" << linenr <<": " << l << "  " << l.length();
        if(l.length()==0){
            qDebug() << "IntelHex unexpected EOF @" << linenr;
            ret=-5;
            break;
        }
        if(l.compare(":00000001FF")==0){
            qDebug() << "IntelHex EOF-line @" << linenr;
            break;
        }
        if(l.left(1).compare(":")!=0){
            qDebug() << "IntelHex ignoring non-':'-line @" << linenr;
            useline=false; ret=-6;
        }
        l=l.mid(1); // cut away ':'
        if((l.length()%2)!=0){
            qDebug() << "IntelHex ignoring line with uneven characters" << l.length() << "@"  << linenr;
            useline=false; ret=-7;
        }
        crc=0;
        for(iFor=0;iFor<l.length();iFor+=2){
            tmp=l.mid(iFor,2).toInt(0,16);
            crc+=tmp;
        }
        if(crc!=0){
            qDebug() << "IntelHex ignoring line with wrong checksum @" << linenr;
            useline=false; ret=-8;
        }
        l=l.left(l.length()-2); // cut away CRC

        uint16_t LineLen     = l.mid(0,2).toInt(0,16);
        uint16_t LineAddress = l.mid(2,4).toInt(0,16);
        uint8_t  LineTyp     = l.mid(6,2).toInt(0,16);
        QByteArray LineData=QByteArray().fromHex(l.mid(8).toLatin1());

        //qDebug() << "IntelHex LineData " << LineData.toHex();
        if(LineLen!=LineData.length()){
            qDebug() << "IntelHex ignoring line with wrong length @" << linenr;
            useline=false; ret=-9;
        }
        if(useline){
            switch(LineTyp){
            case 0x00: //data
                flashaddr=flashextend+LineAddress;
                //qDebug() << "IntelHex dataline  @" << linenr << "  " <<LineLen<<" bytes @ " << flashaddr;
                if(     (flashaddr < this->offset)
                        ||(flashaddr+LineLen)>(offset+size))
                {
                    qDebug() << "data out of range ("<<flashaddr<<"-"<<(flashaddr+LineLen)<<" vs "<<offset<<"-"<<(offset+size)<<")";
                    qDebug() << QString().sprintf("data out of range (0x%lX-0x%lX vs 0x%lX-0x%lX)",
                                                  (long unsigned int)flashaddr,
                                                  (long unsigned int)(flashaddr+LineLen),
                                                  (long unsigned int)offset,
                                                  (long unsigned int)(offset+size));
                    ret=-11;
                    break;
                }
                bufferaddr=flashaddr - this->offset;
                int i;
                char *pBuf,*pLineData;
                pBuf=this->buffer.data();
                pLineData=LineData.data();
                for(i=0;i<LineLen;i++){
                    pBuf[bufferaddr+i]=pLineData[i];
                }

                break;
            case 0x04: //Extended Linear Address Record
                //qDebug() << "IntelHex extaddr " << LineData.toHex();
                flashextend = LineData.toHex().toInt(0,16);
                //qDebug() << "IntelHex extaddr " << flashextend;
                flashextend<<= 16;
                //qDebug() << "IntelHex extaddr @" << linenr << "  :" << flashextend;
                break;
            case 0x05: //Extended Linear Address Record
                qDebug() << "IntelHex ignore linear address " << LineData.toHex();;
                break;
            default:
                qDebug() << "forgotten LineTyp " << LineTyp << " @" << linenr;
                ret=-10;
                break;
            }
        }


    }

    f.close();
    return ret;
}
