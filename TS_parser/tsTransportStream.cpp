#include "tsTransportStream.h"
#include <iostream>
#include <iomanip>
#define YELLOW "\033[33m"
#define WHITE "\033[37m"

using namespace std;

//=============================================================================================================================================================================
// xTS_PacketHeader
//=============================================================================================================================================================================
void xTS_PacketHeader::Reset() {
  //write me
}

int32_t xTS_PacketHeader::Parse(const uint8_t* Input) {
  //std::cout << "Before casting: " << Input << std::endl;
  uint32_t temp=*((uint32_t*)Input);
  //std::cout << "After casting:  " << temp << std::endl;
  uint32_t pakiet=xSwapBytes32(temp);

  uint32_t masks[8] = {
      0b11111111000000000000000000000000, 
      0b00000000100000000000000000000000,
      0b00000000010000000000000000000000, 
      0b00000000001000000000000000000000,
      0b00000000000111111111111100000000, 
      0b00000000000000000000000011000000,
      0b00000000000000000000000000110000, 
      0b00000000000000000000000000001111
  };

  SB=pakiet & masks[0];
  SB = SB >> 24;

  E=pakiet & masks[1];
  E = E >> 23;

  S=pakiet & masks[2];
  S = S >> 22;

  T=pakiet & masks[3];
  T = T >> 21;

  PID=pakiet & masks[4];
  PID = PID >> 8;

  TSC=pakiet & masks[5];
  TSC = TSC >> 6;

  AFC=pakiet & masks[6];
  AFC = AFC >> 4;

  CC=pakiet & masks[7];
  return 0;
}

void xTS_PacketHeader::Print() const {
  //print sth
    cout << "TS: " << "SB=" << SB << " E=" << E << " S=" << S << " P=" << T << " PID="
        << PID << " TSC=" << TSC << " AF=" << AFC << " CC=" << CC;
}

//=============================================================================================================================================================================
int32_t xTS_AdaptationField::Parse(const uint8_t* Input, uint8_t AdaptationFieldControl) {
    uint32_t temp = *((uint32_t*)(Input));
    temp = xSwapBytes32(temp);

    uint32_t mask[9] = {
        0b11111111000000000000000000000000,
        0b00000000100000000000000000000000,
        0b00000000010000000000000000000000,
        0b00000000001000000000000000000000,
        0b00000000000100000000000000000000,
        0b00000000000010000000000000000000,
        0b00000000000001000000000000000000,
        0b00000000000000100000000000000000,
        0b00000000000000010000000000000000
    };

    AFL = temp & mask[0];
    AFL >>= 24;

    DC = temp & mask[1];
    DC >>= 23;

    RA = temp & mask[2];
    RA >>= 22;

    SP = temp & mask[3];
    SP >>= 21;

    PR = temp & mask[4];
    PR >>= 20;

    OR = temp & mask[5];
    OR >>= 19;

    SF = temp & mask[6];
    SF >>= 18;

    TP = temp & mask[7];
    TP >>= 17;

    EX = temp & mask[6];
    EX >>= 16;

    return 0;
}

void xTS_AdaptationField::Print() const{
    //YELLOW - colouring text in CMD...
    cout << YELLOW << "\tAF: L=" << setw(3) << AFL << " DC=" << DC << " RA=" << RA << " SP=" << SP
        << " PR=" << PR << " OR=" << OR << " SF=" << SF << " TP=" << TP << " EX=" << EX << WHITE;
}

//int32_t xPES_PacketHeader::Parse(const uint8_t* Input) {
//    cout << (*Input) << endl;
//    return 0;
//}

int32_t xPES_PacketHeader::Parse(const uint8_t* Input, const uint8_t* AF) {
    uint32_t Tmp = *((uint32_t*)Input);
    Tmp = xSwapBytes32(Tmp);
    m_StreamId = Input[3];
    m_PacketStartCodePrefix = Tmp >> 8;
    uint16_t Tmp2 = *((uint16_t*)AF);
    Tmp2 = xSwapBytes16(Tmp2);
    m_PacketLength = Tmp2;
    return 0;
}

void xPES_PacketHeader::Print()const {
    cout << "PSCP: " << (int)m_PacketStartCodePrefix << " ";
    cout << "SI: " << (int)m_StreamId << " ";
    cout << "PL: " << (int)m_PacketLength << " ";
}