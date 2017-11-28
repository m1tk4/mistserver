#include "rtp.h"
#include "defines.h"
#include "encode.h"
#include "timing.h"
#include "bitfields.h"
#include "mpeg.h"
#include "sdp.h"
#include <arpa/inet.h>

namespace RTP{
  double Packet::startRTCP = 0;
  unsigned int MAX_SEND = 1500 - 28;

  unsigned int Packet::getHsize() const{return 12 + 4 * getContribCount();}

  unsigned int Packet::getPayloadSize() const{return datalen - getHsize();}

  char *Packet::getPayload() const{return data + getHsize();}

  unsigned int Packet::getVersion() const{return (data[0] >> 6) & 0x3;}

  unsigned int Packet::getPadding() const{return (data[0] >> 5) & 0x1;}

  unsigned int Packet::getExtension() const{return (data[0] >> 4) & 0x1;}

  unsigned int Packet::getContribCount() const{return (data[0]) & 0xE;}

  unsigned int Packet::getMarker() const{return (data[1] >> 7) & 0x1;}

  unsigned int Packet::getPayloadType() const{return (data[1]) & 0x7F;}

  unsigned int Packet::getSequence() const{return (((((unsigned int)data[2]) << 8) + data[3]));}

  unsigned int Packet::getTimeStamp() const{return ntohl(*((unsigned int *)(data + 4)));}

  unsigned int Packet::getSSRC() const{return ntohl(*((unsigned int *)(data + 8)));}

  char *Packet::getData(){return data + 8 + 4 * getContribCount() + getExtension();}

  void Packet::setTimestamp(unsigned int t){*((unsigned int *)(data + 4)) = htonl(t);}

  void Packet::setSequence(unsigned int seq){*((short *)(data + 2)) = htons(seq);}

  void Packet::setSSRC(unsigned long ssrc){*((int *)(data + 8)) = htonl(ssrc);}

  void Packet::increaseSequence(){*((short *)(data + 2)) = htons(getSequence() + 1);}

  void Packet::sendH264(void *socket, void callBack(void *, char *, unsigned int, unsigned int),
                        const char *payload, unsigned int payloadlen, unsigned int channel){
    /// \todo This function probably belongs in DMS somewhere.
    if (payloadlen + getHsize() + 2 <= maxDataLen){
      data[1] |= 0x80; // setting the RTP marker bit to 1
      memcpy(data + getHsize(), payload, payloadlen);
      callBack(socket, data, getHsize() + payloadlen, channel);
      sentPackets++;
      sentBytes += payloadlen + getHsize();
      increaseSequence();
    }else{
      data[1] &= 0x7F; // setting the RTP marker bit to 0
      unsigned int sent = 0;
      unsigned int sending =
          maxDataLen - getHsize() - 2; // packages are of size MAX_SEND, except for the final one
      char initByte = (payload[0] & 0xE0) | 0x1C;
      char serByte = payload[0] & 0x1F; // ser is now 000
      data[getHsize()] = initByte;
      while (sent < payloadlen){
        if (sent == 0){
          serByte |= 0x80; // set first bit to 1
        }else{
          serByte &= 0x7F; // set first bit to 0
        }
        if (sent + sending >= payloadlen){
          // last package
          serByte |= 0x40;
          sending = payloadlen - sent;
          data[1] |= 0x80; // setting the RTP marker bit to 1
        }
        data[getHsize() + 1] = serByte;
        memcpy(data + getHsize() + 2, payload + 1 + sent, sending);
        callBack(socket, data, getHsize() + 2 + sending, channel);
        sentPackets++;
        sentBytes += sending + getHsize() + 2;
        sent += sending;
        increaseSequence();
      }
    }
  }

  void Packet::sendH265(void *socket, void callBack(void *, char *, unsigned int, unsigned int),
                        const char *payload, unsigned int payloadlen, unsigned int channel){
    /// \todo This function probably belongs in DMS somewhere.
    if (payloadlen + getHsize() + 3 <= maxDataLen){
      data[1] |= 0x80; // setting the RTP marker bit to 1
      memcpy(data + getHsize(), payload, payloadlen);
      callBack(socket, data, getHsize() + payloadlen, channel);
      sentPackets++;
      sentBytes += payloadlen + getHsize();
      increaseSequence();
    }else{
      data[1] &= 0x7F; // setting the RTP marker bit to 0
      unsigned int sent = 0;
      unsigned int sending =
          maxDataLen - getHsize() - 3; // packages are of size MAX_SEND, except for the final one
      char initByteA = (payload[0] & 0x81) | 0x62;
      char initByteB = payload[1];
      char serByte = (payload[0] & 0x7E) >> 1; // SE is now 00
      data[getHsize()] = initByteA;
      data[getHsize()+1] = initByteB;
      while (sent < payloadlen){
        if (sent == 0){
          serByte |= 0x80; // set first bit to 1
        }else{
          serByte &= 0x7F; // set first bit to 0
        }
        if (sent + sending >= payloadlen){
          // last package
          serByte |= 0x40;
          sending = payloadlen - sent;
          data[1] |= 0x80; // setting the RTP marker bit to 1
        }
        data[getHsize() + 2] = serByte;
        memcpy(data + getHsize() + 3, payload + 2 + sent, sending);
        callBack(socket, data, getHsize() + 3 + sending, channel);
        sentPackets++;
        sentBytes += sending + getHsize() + 3;
        sent += sending;
        increaseSequence();
      }
    }
  }

  void Packet::sendMPEG2(void *socket, void callBack(void *, char *, unsigned int, unsigned int),
                        const char *payload, unsigned int payloadlen, unsigned int channel){
    /// \todo This function probably belongs in DMS somewhere.
    if (payloadlen + getHsize() + 4 <= maxDataLen){
      data[1] |= 0x80; // setting the RTP marker bit to 1
      Mpeg::MPEG2Info mInfo = Mpeg::parseMPEG2Headers(payload, payloadlen);
      MPEGVideoHeader mHead(data+getHsize());
      mHead.clear();
      mHead.setTempRef(mInfo.tempSeq);
      mHead.setPictureType(mInfo.frameType);
      if (mInfo.isHeader){
        mHead.setSequence();
      }
      mHead.setBegin();
      mHead.setEnd();
      memcpy(data + getHsize() + 4, payload, payloadlen);
      callBack(socket, data, getHsize() + payloadlen + 4, channel);
      sentPackets++;
      sentBytes += payloadlen + getHsize() + 4;
      increaseSequence();
    }else{
      data[1] &= 0x7F; // setting the RTP marker bit to 0
      unsigned int sent = 0;
      unsigned int sending =
          maxDataLen - getHsize() - 4; // packages are of size MAX_SEND, except for the final one
      Mpeg::MPEG2Info mInfo;
      MPEGVideoHeader mHead(data+getHsize());
      while (sent < payloadlen){
        mHead.clear();
        if (sent + sending >= payloadlen){
          mHead.setEnd();
          sending = payloadlen - sent;
          data[1] |= 0x80; // setting the RTP marker bit to 1
        }
        Mpeg::parseMPEG2Headers(payload, sent+sending, mInfo);
        mHead.setTempRef(mInfo.tempSeq);
        mHead.setPictureType(mInfo.frameType);
        if (sent == 0){
          if (mInfo.isHeader){
            mHead.setSequence();
          }
          mHead.setBegin();
        }
        memcpy(data + getHsize() + 4, payload + sent, sending);
        callBack(socket, data, getHsize() + 4 + sending, channel);
        sentPackets++;
        sentBytes += sending + getHsize() + 4;
        sent += sending;
        increaseSequence();
      }
    }
  }

  void Packet::sendData(void *socket, void callBack(void *, char *, unsigned int, unsigned int),
                        const char *payload, unsigned int payloadlen, unsigned int channel,
                        std::string codec){
    if (codec == "H264"){
      unsigned long sent = 0;
      while (sent < payloadlen){
        unsigned long nalSize = ntohl(*((unsigned long *)(payload + sent)));
        sendH264(socket, callBack, payload + sent + 4, nalSize, channel);
        sent += nalSize + 4;
      }
      return;
    }
    if (codec == "HEVC"){
      unsigned long sent = 0;
      while (sent < payloadlen){
        unsigned long nalSize = ntohl(*((unsigned long *)(payload + sent)));
        sendH265(socket, callBack, payload + sent + 4, nalSize, channel);
        sent += nalSize + 4;
      }
      return;
    }
    if (codec == "MPEG2"){
      sendMPEG2(socket, callBack, payload, payloadlen, channel);
      return;
    }
    /// \todo This function probably belongs in DMS somewhere.
    data[1] |= 0x80; // setting the RTP marker bit to 1
    long offsetLen = 0;
    if (codec == "AAC"){
      *((long *)(data + getHsize())) = htonl(((payloadlen << 3) & 0x0010fff8) | 0x00100000);
      offsetLen = 4;
    }else if (codec == "MP3" || codec == "MP2"){
      //See RFC 2250, "MPEG Audio-specific header"
      *((long *)(data + getHsize())) = 0; // this is MBZ and Frag_Offset, which are always 0
      if (payload[0] != 0xFF){
        FAIL_MSG("MP2/MP3 data does not start with header?");
      }
      offsetLen = 4;
    }else if (codec == "AC3"){
      *((short *)(data + getHsize())) = htons(0x0001); // this is 6 bits MBZ, 2 bits FT = 0 = full
                                                       // frames and 8 bits saying we send 1 frame
      offsetLen = 2;
    }
    if (maxDataLen < getHsize() + offsetLen + payloadlen){
      if (!managed){
        FAIL_MSG("RTP data too big for packet, not sending!");
        return;
      }
      uint32_t newMaxLen = getHsize() + offsetLen + payloadlen;
      char *newData = new char[newMaxLen];
      if (newData){
        memcpy(newData, data, maxDataLen);
        delete[] data;
        data = newData;
        maxDataLen = newMaxLen;
      }
    }
    memcpy(data + getHsize() + offsetLen, payload, payloadlen);
    callBack(socket, data, getHsize() + offsetLen + payloadlen, channel);
    sentPackets++;
    sentBytes += payloadlen + offsetLen + getHsize();
    increaseSequence();
  }

  void Packet::sendRTCP_SR(long long &connectedAt, void *socket, unsigned int tid,
                        DTSC::Meta &metadata,
                        void callBack(void *, char *, unsigned int, unsigned int)){
    char *rtcpData = (char*)malloc(32);
    if (!rtcpData){
      FAIL_MSG("Could not allocate 32 bytes. Something is seriously messed up.");
      return;
    }
    rtcpData[0] = 0x80;//version 2, no padding, zero receiver reports
    rtcpData[1] = 200;//sender report
    Bit::htobs(rtcpData+2, 6);//6 4-byte words follow the header
    Bit::htobl(rtcpData+4, getSSRC());//set source identifier
   // timestamp in ms
    double ntpTime = 2208988800UL + Util::epoch() + (Util::getMS() % 1000) / 1000.0;
    if (startRTCP < 1 && startRTCP > -1){startRTCP = ntpTime;}
    ntpTime -= startRTCP;

    ((int *)rtcpData)[2] = htonl(2208988800UL + Util::epoch()); // epoch is in seconds
    ((int *)rtcpData)[3] = htonl((Util::getMS() % 1000) * 4294967.295);
    if (metadata.tracks[tid].codec == "H264"){
      ((int *)rtcpData)[4] = htonl((ntpTime - 0) * 90000); // rtpts
    }else{
      ((int *)rtcpData)[4] = htonl((ntpTime - 0) * metadata.tracks[tid].rate); // rtpts
    }
    // it should be the time packet was sent maybe, after all?
    //*((int *)(rtcpData+16) ) = htonl(getTimeStamp());//rtpts
    ((int *)rtcpData)[5] = htonl(sentPackets); // packet
    ((int *)rtcpData)[6] = htonl(sentBytes);   // octet
    callBack(socket, (char *)rtcpData, 28, 0);
    free(rtcpData);
  }

  void Packet::sendRTCP_RR(long long &connectedAt, SDP::Track & sTrk, unsigned int tid,
                        DTSC::Meta &metadata,
                        void callBack(void *, char *, unsigned int, unsigned int)){
    char *rtcpData = (char*)malloc(32);
    if (!rtcpData){
      FAIL_MSG("Could not allocate 32 bytes. Something is seriously messed up.");
      return;
    }
    if (!(sTrk.lostCurrent + sTrk.packCurrent)){sTrk.packCurrent++;}
    rtcpData[0] = 0x81;//version 2, no padding, one receiver report
    rtcpData[1] = 201;//receiver report
    Bit::htobs(rtcpData+2, 7);//7 4-byte words follow the header
    Bit::htobl(rtcpData+4, sTrk.mySSRC);//set receiver identifier
    Bit::htobl(rtcpData+8, sTrk.theirSSRC);//set source identifier
    rtcpData[12] = (sTrk.lostCurrent * 255) / (sTrk.lostCurrent + sTrk.packCurrent); //fraction lost since prev RR
    Bit::htob24(rtcpData+13, sTrk.lostTotal); //cumulative packets lost since start
    Bit::htobl(rtcpData+16, sTrk.rtpSeq | (sTrk.packTotal & 0xFFFF0000ul)); //highest sequence received
    Bit::htobl(rtcpData+20, 0); /// \TODO jitter (diff in timestamp vs packet arrival)
    Bit::htobl(rtcpData+24, 0); /// \TODO last SR (middle 32 bits of last SR or zero)
    Bit::htobl(rtcpData+28, 0); /// \TODO delay since last SR in 2b seconds + 2b fraction
    callBack(&(sTrk.rtcp), (char *)rtcpData, 32, 0);
    sTrk.lostCurrent = 0;
    sTrk.packCurrent = 0;
    free(rtcpData);
  }

  Packet::Packet(){
    managed = false;
    data = 0;
    maxDataLen = 0;
    sentBytes = 0;
    sentPackets = 0;
  }

  Packet::Packet(unsigned int payloadType, unsigned int sequence, unsigned int timestamp,
                 unsigned int ssrc, unsigned int csrcCount){
    managed = true;
    data = new char[12 + 4 * csrcCount + 2 +
                    MAX_SEND]; // headerSize, 2 for FU-A, MAX_SEND for maximum sent size
    if (data){
      maxDataLen = 12 + 4 * csrcCount + 2 + MAX_SEND;
      data[0] = ((2) << 6) | ((0 & 1) << 5) | ((0 & 1) << 4) |
                (csrcCount & 15);   // version, padding, extension, csrc count
      data[1] = payloadType & 0x7F; // marker and payload type
    }else{
      maxDataLen = 0;
    }
    setSequence(sequence - 1); // we automatically increase the sequence each time when p
    setTimestamp(timestamp);
    setSSRC(ssrc);
    sentBytes = 0;
    sentPackets = 0;
  }

  Packet::Packet(const Packet &o){
    managed = true;
    maxDataLen = 0;
    if (o.data && o.maxDataLen){
      data = new char[o.maxDataLen]; // headerSize, 2 for FU-A, MAX_SEND for maximum sent size
      if (data){
        maxDataLen = o.maxDataLen;
        memcpy(data, o.data, o.maxDataLen);
      }
    }else{
      data = new char[14 + MAX_SEND]; // headerSize, 2 for FU-A, MAX_SEND for maximum sent size
      if (data){
        maxDataLen = 14 + MAX_SEND;
        memset(data, 0, maxDataLen);
      }
    }
    sentBytes = o.sentBytes;
    sentPackets = o.sentPackets;
  }

  void Packet::operator=(const Packet &o){
    if (data && managed){delete[] data;}
    managed = true;
    maxDataLen = 0;
    data = 0;

    if (o.data && o.maxDataLen){
      data = new char[o.maxDataLen]; // headerSize, 2 for FU-A, MAX_SEND for maximum sent size
      if (data){
        maxDataLen = o.maxDataLen;
        memcpy(data, o.data, o.maxDataLen);
      }
    }else{
      data = new char[14 + MAX_SEND]; // headerSize, 2 for FU-A, MAX_SEND for maximum sent size
      if (data){
        maxDataLen = 14 + MAX_SEND;
        memset(data, 0, maxDataLen);
      }
    }
    sentBytes = o.sentBytes;
    sentPackets = o.sentPackets;
  }

  Packet::~Packet(){
    if (managed){delete[] data;}
  }
  Packet::Packet(const char *dat, unsigned int len){
    managed = false;
    datalen = len;
    maxDataLen = len;
    sentBytes = 0;
    sentPackets = 0;
    data = (char *)dat;
  }

  MPEGVideoHeader::MPEGVideoHeader(char *d){
    data = d;
  }

  uint16_t MPEGVideoHeader::getTotalLen() const{
    uint16_t ret = 4;
    if (data[0] & 0x08){
      ret += 4;
      if (data[4] & 0x40){
        ret += data[8];
      }
    }
    return ret;
  }

  std::string MPEGVideoHeader::toString() const{
    std::stringstream ret;
    uint32_t firstHead = Bit::btohl(data);
    ret << "TR=" << ((firstHead & 0x3FF0000) >> 16);
    if (firstHead & 0x4000000){ret << " Ext";}
    if (firstHead & 0x2000){ret << " SeqHead";}
    if (firstHead & 0x1000){ret << " SliceBegin";}
    if (firstHead & 0x800){ret << " SliceEnd";}
    ret << " PicType=" << ((firstHead & 0x700) >> 8);
    if (firstHead & 0x80){ret << " FBV";}
    ret << " BFC=" << ((firstHead & 0x70) >> 4);
    if (firstHead & 0x8){ret << " FFV";}
    ret << " FFC=" << (firstHead & 0x7);
    return ret.str();
  }

  void MPEGVideoHeader::clear(){
    ((uint32_t*)data)[0] = 0;
  }

  void MPEGVideoHeader::setTempRef(uint16_t ref){
    data[0] |= (ref >> 8) & 0x03;
    data[1] = ref & 0xff;
  }

  void MPEGVideoHeader::setPictureType(uint8_t pType){
    data[2] |= pType & 0x7;
  }

  void MPEGVideoHeader::setSequence(){
    data[2] |= 0x20;
  }
  void MPEGVideoHeader::setBegin(){
    data[2] |= 0x10;
  }
  void MPEGVideoHeader::setEnd(){
    data[2] |= 0x8;
  }

}
