#include "tsCommon.h"
#include "tsTransportStream.h"
#include <iostream>
#include <errno.h>
#include <string.h>
#include <stdio.h>


using namespace std;

int main(int argc, char *argv[ ], char *envp[ ])
{
  FILE * file;
  errno_t err;

  //FILE IS TOO LARGE FOR GITHUB (need to be unzipped before using THIS program)
  const char* fileName = "example_new.ts";

  //file = fopen_s("example_new.ts", "rb");

  if ((err = fopen_s(&file, fileName, "rb")) != 0) {
      // File could not be opened. filepoint was set to NULL
      // error code is returned in err.
      // error message can be retrieved with strerror(err);
      //fprintf(stderr, "cannot open file '%s': %s\n",
      //    fileName, strerror(err));
      // If your environment insists on using so called secure
      // functions, use this instead:

      //char buf[std::to_string(err) + 1];
      //strerror_s(buf, sizeof buf, err);
      //fprintf_s(stderr, "cannot open file '%s': %s\n", fileName, buf);
  }
  else {
      // File was opened, filepoint can be used to read the stream.

      if (file == NULL) {
          cout << "success" << endl;
      }

      xTS_PacketHeader      TS_PacketHeader;
      xTS_AdaptationField   TS_PacketAF;
      xPES_Assembler        PES_Assembler;                      //HACK: gives errors - commentIT!
      xPES_PacketHeader     PES_Header;

      //create buffer for header (188 bytes)
      uint8_t* buffer;
      buffer = (uint8_t*)malloc(sizeof(uint8_t) * 188);

      int counter = 0;
      int32_t TS_PacketId = 0;
      uint32_t aux_AFC = 0;

      while (counter < 50)//fgetc(file) != EOF)
      {
          int read = fread(buffer, sizeof(uint8_t), 188, file);
          //std::cout << read << std::endl;

          ///////////////////////////
          //
          // PROJEKT - MACIEK
          //
          if (read != xTS::TS_PacketLength) {
              break;
          }

          /////////////////////////////
          // lab01
          //

          TS_PacketHeader.Reset();
          TS_PacketHeader.Parse(buffer);
          printf("%010d ", TS_PacketId);
          //print each PacketHeader line
          TS_PacketHeader.Print();

          ///////////////////////////////
          // lab03 - resetowanie DAC w ifie, ¿eby u¿ywaæ AF do PES
          TS_PacketAF.Reset();
          if (TS_PacketHeader.getSyncByte() == 'G' && TS_PacketHeader.getPID() == 136) {
              /////////////////////////////
              // lab02
              //TEMP INFO: moja czêœæ
              if (TS_PacketHeader.getAFC() == 2 || TS_PacketHeader.getAFC() == 3) {
                  TS_PacketAF.Parse(buffer + 4, TS_PacketHeader.getAFC());          //buffer+4 ~offset by 4 bytes (length of header)
                  TS_PacketAF.Print();
                  //IDEA: Parse 6B data
              }

              /////////////////////
              // lab03 - continue

              xPES_Assembler::eResult Result = PES_Assembler.AbsorbPacket(buffer, &TS_PacketHeader, &TS_PacketAF);
              cout << "\t";
              switch (Result) {
              case xPES_Assembler::eResult::StreamPacketLost:
                  cout << "PcktLost";
                  //printf("PcktLost ");
                  break;
              case xPES_Assembler::eResult::AssemblingStarted:
                  cout << "Started";
                  //printf("Started ");
                  PES_Assembler.PrintPESH();
                  break;
              case xPES_Assembler::eResult::AssemblingContinue:
                  cout << "Continue";
                  //printf("Continue ");
                  break;
              case xPES_Assembler::eResult::AssemblingFinished:
                  cout << "Finished\t";
                  //printf("Finished ");
                  cout << "PES: Len= " << PES_Assembler.getNumPacketBytes();
                  break;
              default:
                  break;
              }

          }

          printf("\n");
          counter++;
          TS_PacketId++;
      }

      delete file;
  }

  return 0;
}