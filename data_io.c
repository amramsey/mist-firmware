#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "user_io.h"
#include "data_io.h"
#include "debug.h"
#include "spi.h"

// core supports direct ROM upload via SS4
char rom_direct_upload = 0;

void data_io_set_index(char index) {
  EnableFpga();
  SPI(DIO_FILE_INDEX);
  SPI(index);
  DisableFpga();
}

///////////////////////////
// TRANSMIT FILE TO FPGA //
///////////////////////////

static void data_io_file_tx_prepare(FIL *file, char index, const char *ext) {
  char e[3];
  iprintf("Preparing transmission for index %d\n", index);

  e[0] = e[1] = e[2] = ' ';
  if (ext) {
    if (ext[0]) e[0] = toupper(ext[0]);
    if (ext[1]) e[1] = toupper(ext[1]);
    if (ext[2]) e[2] = toupper(ext[2]);
  }
  // set index byte (0=bios rom, 1-n=OSD entry index)
  data_io_set_index(index);

  // send directory entry

  EnableFpga();
  SPI(DIO_FILE_INFO);

  FSIZE_t fsize = f_size(file);
  spi_n(0, 8); // name
  spi8(e[0]);spi8(e[1]);spi8(e[2]); // ext
  spi_n(0, 1); // attr
  spi_n(0, 16);
  spi_write((void*)&fsize, 4);

  DisableFpga();

  // prepare transmission of new file
  EnableFpga();
  SPI(DIO_FILE_TX);
  SPI(0xff);
  DisableFpga();
}

static void data_io_file_tx_send(FIL *file) {
  FSIZE_t bytes2send = f_size(file);
  UINT br;

  /* transmit the entire file using one transfer */
  iprintf("Selected %llu bytes to send\n", bytes2send);

  while(bytes2send) {
    iprintf(".");

    unsigned short c, chunk = (bytes2send>512)?512:bytes2send;
    char *p;

    if (rom_direct_upload) {
      // upload directly from the SD-Card if the core supports that
      bytes2send = (file->obj.objsize + 511) & 0xfffffe00;
      file->obj.objsize = bytes2send;
      f_read(file, 0, bytes2send, &br);
      bytes2send = 0;
    } else {
      f_read(file, sector_buffer, chunk, &br);

      EnableFpga();
      SPI(DIO_FILE_TX_DAT);

      for(p = sector_buffer, c=0;c < chunk;c++)
        SPI(*p++);

      DisableFpga();
      bytes2send -= chunk;
    }
  }
}

static void data_io_file_tx_done(void) {
  // signal end of transmission
  EnableFpga();
  SPI(DIO_FILE_TX);
  SPI(0x00);
  DisableFpga();

  iprintf("\n");
}

static void data_io_file_tx_fill(unsigned char fill, unsigned int len) {

  EnableFpga();
  SPI(DIO_FILE_TX_DAT);
  while(len--) {
    SPI(fill);
  }
  DisableFpga();
}

void data_io_file_tx(FIL *file, char index, const char *ext) {
  data_io_file_tx_prepare(file, index, ext);
  data_io_file_tx_send(file);
  data_io_file_tx_done();
}

// send 'fill' byte 'len' times
void data_io_fill_tx(unsigned char fill, unsigned int len, char index) {
  data_io_file_tx_prepare(0, index, 0);
  data_io_file_tx_fill(fill, len);
  data_io_file_tx_done();
}

////////////////////////////
// RECEIVE FILE FROM FPGA //
////////////////////////////

static void data_io_file_rx_prepare(char index) {
  iprintf("Preparing receiving for index %d\n", index);

  // set index byte (0=bios rom, 1-n=OSD entry index)
  data_io_set_index(index);

  // prepare transmission of new file
  EnableFpga();
  SPI(DIO_FILE_RX);
  SPI(0xff);
  DisableFpga();
}

static void data_io_file_rx_receive(FIL *file, unsigned int len) {
  unsigned int bytes2receive = len;
  char first = 1;
  UINT bw;
  /* receive the entire file using one transfer */
  iprintf("Selected %lu bytes to receive\n", bytes2receive);

  while(bytes2receive) {
    iprintf(".");

    unsigned short c, chunk = (bytes2receive>512)?512:bytes2receive;
    char *p;

    EnableFpga();
    SPI(DIO_FILE_RX_DAT);
    if (first) {
      SPI(0);
      first=0;
    }

    for(p = sector_buffer, c=0;c < chunk;c++)
      *p++ = SPI(0xFF);

    DisableFpga();
    bytes2receive -= chunk;
    f_write(file, sector_buffer, chunk, &bw);
  }
}

static void data_io_file_rx_done(void) {
  // signal end of transmission
  EnableFpga();
  SPI(DIO_FILE_RX);
  SPI(0x00);
  DisableFpga();

  iprintf("\n");
}

void data_io_file_rx(FIL *file, char index, unsigned int len) {
  data_io_file_rx_prepare(index);
  data_io_file_rx_receive(file, len);
  data_io_file_rx_done();
}

////////////////
// ROM UPLOAD //
////////////////

void data_io_rom_upload(char *rname, char mode) {

  FIL file;
  FRESULT res;
  static char first = 1;
  char fname[67];

  // new ini parsing starts, prepare uploads
  if(mode == 0) {
    first = 1;
    return;
  }

  // ini parsing done
  if(mode == 2) {
    // has something been uploaded?
    // -> then end transfer
    if(!first) {
      iprintf("upload ends\n");

      data_io_file_tx_done();
      user_io_8bit_set_status(0, UIO_STATUS_RESET);
    }
    return;
  }

  // try to change into core dir. Stay in root if that doesn't exist
  user_io_change_into_core_dir();

  strncpy(fname, rname, sizeof(fname)-4);
  fname[sizeof(fname)-5] = 0;
  strcat(fname,".ROM");
  iprintf("rom upload '%s'\n", fname);

  res=f_open(&file, fname, FA_READ);
  if (res== FR_OK) {

    if(first) {
      // set reset
      user_io_8bit_set_status(UIO_STATUS_RESET, UIO_STATUS_RESET);
      data_io_file_tx_prepare(&file, 0, "ROM");
      first = 0;
    }

    //    user_io_file_tx(&f, 0);
    data_io_file_tx_send(&file);
  } else
    iprintf("Error opening file %s (%d)!\n", fname, res);

  ChangeDirectoryName("/");
  ScanDirectory(SCAN_INIT, "RBF",  SCAN_LFN | SCAN_SYSDIR);
}
