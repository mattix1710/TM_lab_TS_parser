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

void xTS_AdaptationField::Reset() {

}

int32_t xPES_PacketHeader::Parse(const uint8_t* Input)      //TODO: Parse but change from BIT_STREAM to MASKS...
{
    uint32_t tempStr = *((uint32_t*)Input);
    uint32_t pakiet = xSwapBytes32(tempStr);

    uint32_t maskCodePrefix = 0x00000100;                   //32bit mask

    m_PacketStartCodePrefix = pakiet & maskCodePrefix;
    m_PacketStartCodePrefix >>= 8;

    m_StreamId = Input[3];

    /* // OLD version - d³uuugie
    uint64_t lengthTEMP = *((uint64_t*)Input);
    lengthTEMP = xSwapBytes64(lengthTEMP);
    uint64_t maskLength = 0x00000000FFFF0000;
    m_PacketLength = (uint16_t)((lengthTEMP & maskLength) >> 16);*/

    //NEW - skrócone do 1 linijki, bez dodatkowych zmiennych
    m_PacketLength = xSwapBytes32(*((uint16_t*)((uint8_t*)Input + 3)));


    //TEMP: dalej jak w projekcie z GITHUBa
    if (m_StreamId != eStreamId::eStreamId_program_stream_map
        && m_StreamId != eStreamId::eStreamId_padding_stream
        && m_StreamId != eStreamId::eStreamId_private_stream_2
        && m_StreamId != eStreamId::eStreamId_ECM
        && m_StreamId != eStreamId::eStreamId_EMM
        && m_StreamId != eStreamId::eStreamId_program_stream_directory
        && m_StreamId != eStreamId::eStreamId_DSMCC_stream
        && m_StreamId != eStreamId::eStreamId_ITUT_H222_1_type_E)
    {
        //header length is not known, read 3 bytes for flags
        uint32_t next3bytes = *((uint32_t*)((uint8_t*)Input + xTS::PES_HeaderLength));

        uint32_t reversedBytes = xSwapBytes32(next3bytes);

        uint32_t masks[14] = {
            0b11000000000000000000000000000000,
            0b00110000000000000000000000000000,
            0b00001000000000000000000000000000,
            0b00000100000000000000000000000000,
            0b00000010000000000000000000000000,
            0b00000001000000000000000000000000,
            0b00000000110000000000000000000000,
            0b00000000001000000000000000000000,
            0b00000000000100000000000000000000,
            0b00000000000010000000000000000000,
            0b00000000000001000000000000000000,
            0b00000000000000100000000000000000,
            0b00000000000000010000000000000000,
            0b00000000000000001111111100000000
        };

        m_Marker_bits = (uint8_t)((reversedBytes & masks[0]) >> 30);
        m_PES_scrambling_control = (uint8_t)((reversedBytes & masks[1]) >> 28);
        m_PES_priority = (uint8_t)((reversedBytes & masks[2]) >> 27);
        m_Data_alignment_indicator = (uint8_t)((reversedBytes & masks[3]) >> 26);
        m_Copyright = (uint8_t)((reversedBytes & masks[4]) >> 25);
        m_Original_or_copy = (uint8_t)((reversedBytes & masks[5]) >> 24);

        m_PTS_DTS_flags = (uint8_t)((reversedBytes & masks[6]) >> 22);
        m_ESCR_flag = (uint8_t)((reversedBytes & masks[7]) >> 21);
        m_ES_rate_flag = (uint8_t)((reversedBytes & masks[8]) >> 20);
        m_DSM_trick_mode_flag = (uint8_t)((reversedBytes & masks[9]) >> 19);
        m_Additional_copy_info_flag = (uint8_t)((reversedBytes & masks[10]) >> 18);
        m_PES_CRC_flag = (uint8_t)((reversedBytes & masks[11]) >> 17);
        m_PES_extension_flag = (uint8_t)((reversedBytes & masks[12]) >> 16);
        m_PES_header_data_length = (uint8_t)((reversedBytes & masks[13]) >> 8);

        cout << "HEADER LENGTH: " <<(int)m_PES_header_data_length;

        //TEMP: coœ tam dalej pisz¹, ale chyba nie potrzebne
    }
    //else if (m_StreamId == eStreamId::eStreamId_program_stream_map
    //    || m_StreamId == eStreamId::eStreamId_padding_stream
    //    || m_StreamId == eStreamId::eStreamId_private_stream_2
    //    || m_StreamId == eStreamId::eStreamId_ECM
    //    || m_StreamId == eStreamId::eStreamId_EMM
    //    || m_StreamId == eStreamId::eStreamId_program_stream_directory
    //    || m_StreamId == eStreamId::eStreamId_DSMCC_stream
    //    || m_StreamId == eStreamId::eStreamId_ITUT_H222_1_type_E)
    //{
    //    //TEMP: doslownie
    //    //"to powinno byc jako parsowanie calego pakietu a nie naglowka"
    //}

    return 0;
}

void xPES_PacketHeader::Print()const {
    cout << "\t";
    cout << "PSCP: " << (int)m_PacketStartCodePrefix << " ";
    cout << "SID: " << (int)m_StreamId << " ";
    cout << "PL: " << (int)m_PacketLength << " ";
}

void xPES_Assembler::parsePESheader(const uint8_t* buff)
{
    uint32_t temp = *((uint32_t*)buff);
    uint32_t pakiet = xSwapBytes32(temp);

    uint32_t maskPrefix = 0x00000100;

    uint32_t SCprefix = pakiet & maskPrefix;
    SCprefix >>= 8;
    uint8_t sID = buff[3];


    uint64_t lengthTEMP = *((uint64_t*)buff);
    lengthTEMP = xSwapBytes64(lengthTEMP);
    uint64_t maskLength = 0x00000000FFFF0000;

    uint64_t PL = lengthTEMP & maskLength;
    PL >>= 16;
    PL == PL & 0x00FF;

    cout << "SCprefix: " << SCprefix << "; sID: " << (int)sID << "; PL:" << PL;
    
}

xPES_Assembler::xPES_Assembler()
{
    m_BufferSize = 0;
    m_Buffer = nullptr;
    m_Started = false;
    m_PID = -1;
    m_LastContinuityCounter = -1;
    m_DataOffset = xTS::PES_HeaderLength;
    m_File = nullptr;
}

xPES_Assembler::~xPES_Assembler()
{
    fclose(m_File);
    delete[] m_Buffer;
}

void xPES_Assembler::InitMusic(std::string filename, int32_t PID)                                      //DONE
{
    m_PID = PID;
    std::string fileInit = filename + ".mp2";
    fopen_s(&m_File, fileInit.c_str(), "wb");
}

void xPES_Assembler::InitVideo(std::string filename, int32_t PID)                                      //DONE
{
    m_PID = PID;
    std::string fileInit = filename + ".264";
    fopen_s(&m_File, fileInit.c_str(), "wb");
}

void xPES_Assembler::xBufferReset()                                         //DONE
{
    m_BufferSize = 0;
    m_DataOffset = 0;

    delete[] m_Buffer;
    m_Buffer = nullptr;
}

void xPES_Assembler::xBufferAppend(const uint8_t* Data, int32_t Size)       //DONE
{
    m_BufferSize += Size;
    m_DataOffset += Size;

    if (m_Buffer == nullptr) //after reset
    {
        m_Buffer = new uint8_t[m_BufferSize];
        std::memcpy(m_Buffer, Data, m_BufferSize);
        return;
    }

    uint8_t* temp_buffer = new uint8_t[m_BufferSize];

    int32_t new_data_start_byte = m_BufferSize - Size;

    std::memcpy(temp_buffer, m_Buffer, new_data_start_byte);
    std::memcpy(&temp_buffer[new_data_start_byte], Data, Size);     //append new data

    delete[] m_Buffer;
    m_Buffer = new uint8_t[m_BufferSize];

    std::memcpy(m_Buffer, temp_buffer, m_BufferSize);               //fill buffer

    delete[] temp_buffer;
}

xPES_Assembler::eResult xPES_Assembler::AbsorbPacket(const uint8_t* TransportStreamPacket, const xTS_PacketHeader* PacketHeader, const xTS_AdaptationField* AdaptationField) {
    int32_t start_byte = 0;
    int32_t payload_size = 0;

    if (m_LastContinuityCounter != -1 && (PacketHeader->getCC() - m_LastContinuityCounter) != 1 && ((int32_t)PacketHeader->getCC() - m_LastContinuityCounter) != -15) {
        xBufferReset();
        return eResult::StreamPacketLost;
    }

    if (PacketHeader->getPayloadSI() == 1) {
        if (m_Started && m_PESH.getPacketLength() == 0) {
            m_Started = false;                                  //TEMP: bez sensu, bo zaraz jest = true
            size_t written = fwrite(&m_Buffer[this->getHeaderLength()], sizeof(char), this->getDataLength(), m_File);
            xBufferReset();
        }

        if (m_Buffer != nullptr) xBufferReset();

        m_Started = true;
        m_LastContinuityCounter = PacketHeader->getCC();

        start_byte = xTS::TS_HeaderLength + AdaptationField->getAFL() + 1;
        payload_size = xTS::TS_PacketLength - xTS::TS_HeaderLength - (AdaptationField->getAFL() + 1);

        if (AdaptationField->getAFL() == 0) {
            start_byte--;
            payload_size++;
        }

        m_PESH.Parse(&TransportStreamPacket[start_byte]);

        xBufferAppend(&TransportStreamPacket[start_byte], payload_size);
        return eResult::AssemblingStarted;
    }

    //still assembling
    if (m_Started) {
        m_LastContinuityCounter = PacketHeader->getCC();

        if (PacketHeader->hasAdaptationField())          //if there is last PES packet
        {
            start_byte = xTS::TS_HeaderLength + AdaptationField->getAFL() + 1;
            payload_size = xTS::TS_PacketLength - xTS::TS_HeaderLength - (AdaptationField->getAFL() + 1);
        }
        else
        {
            start_byte = xTS::TS_HeaderLength;
            payload_size = xTS::TS_PacketLength - xTS::TS_HeaderLength;
        }

        xBufferAppend(&TransportStreamPacket[start_byte], payload_size);

        //TEMP INFO: last PES packet part, length without header
        if (m_PESH.getPacketLength() == m_BufferSize - xTS::PES_HeaderLength) {
            m_Started = false;
            return eResult::AssemblingFinished;
        }

        return eResult::AssemblingContinue;
    }
}