//
// This is Penguin's i2c subsystem, 2015.4
// This program is used to investigate linux i2c subsystem
//

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#define OP_NONE       0
#define OP_SCAN_ADDR  1

typedef struct _PARAMETERS {
  char dev_path[256];
  int  operate;
} PARAMETER;


///////////////
// PROTOTYPES
///////////////
void print_usage(void);
void parse_options(PARAMETER *param, int argc, char **argv);
unsigned int checkAddr(char* const dev_path);
int i2cReadByte(int file, int reg, unsigned char *byteData);
void listI2cDev(char* const dev_path);

///////////////
// LOCALS
///////////////
static int lasterror;
const char* const short_options = "hd:o:D";
const struct option long_options[] = {
  {"help", 0, NULL, 'h'},
  {"devpath", 1, NULL, 'd'},
  {"operate", 1, NULL, 'o'},
  {"debug", 0, NULL, 'D'}
};
PARAMETER scsi_param;

unsigned int isDebug;

///////////////
// FUNCTIONS
///////////////
int main(int argc, char* argv[])
{

  printf("This is Penguin's i2c sub-system test\n");

  parse_options(&scsi_param, argc, argv);

  switch (scsi_param.operate)
  {
    case OP_SCAN_ADDR:
      listI2cDev(scsi_param.dev_path);
      break;

    default:
      printf("Undefined operate\n");
      print_usage();
      break;
  }

  return 0;
}

void print_usage(void)
{
  printf("  -h  --help            Display usage information\n");
  printf("  -d  --devpath         Specify test scsi device path\n");
  printf("  -o  --operate=OP_XXX  Specify operetion\n");
  printf("  -D  --debug           print debug info\n");
}

void parse_options(PARAMETER *param, int argc, char **argv)
{
  int option;
  char *opt_arg = NULL;

  if (argc == 1)
  {
    print_usage();
    exit(0);
  }

  param->operate = OP_NONE;

  do
  {
    option = getopt_long(argc, argv, short_options, long_options, NULL);
    switch(option)
    {
      case 'h':
        print_usage();
        exit(0);

      case 'd':
        opt_arg = optarg;
        strcpy(param->dev_path, opt_arg);
        break;

      case 'o':
        opt_arg = optarg;
        if (*opt_arg == 's')
          param->operate = OP_SCAN_ADDR;
        else
        {
          printf("unsupported operation of option -o\n");
          print_usage();
          exit(0);
        }
        break;

      case 'D':
        isDebug = 1;
        break;

      case -1:
        break;

      default:
        printf("Undefined parameter!\n");
        print_usage();
        exit(0);
    }
  } while (option != -1);

}

void listI2cDev(char* const dev_path)
{
  char szFilename[20];
  int  adapter;
  int  file;
  int  addr;
  unsigned char byte;

  for (adapter = 0; adapter < 0x20; adapter++)
  {
    snprintf(szFilename, 19, "/dev/i2c-%d", adapter);
    file = open(szFilename, O_RDWR);
    if (file < 0)
    {
//      lasterror = errno;
//      printf("ERROR, open %s, %s: line %d, (%d) - %s\n", szFilename, __func__, __LINE__, lasterror, strerror(lasterror));
      continue;
    }

    printf("scan addr, adapter %s : ", szFilename);
    for (addr = 0; addr < 128; addr++)
    {
      if (ioctl(file, I2C_SLAVE, addr) != 0)
        continue;

      if (i2cReadByte(file, 0, &byte) == 0)
      {
        printf("0x%02x | ", addr << 1);
      }
    }
    printf("\n");
    
    close(file);
  }
}

int i2cReadByte(int file, int reg, unsigned char *byteData)
{
  union     i2c_smbus_data data;
  struct    i2c_smbus_ioctl_data smbusArgs;

  smbusArgs.command    = reg;
  smbusArgs.read_write = I2C_SMBUS_READ;
  smbusArgs.data       = &data;
  smbusArgs.size       = I2C_SMBUS_BYTE_DATA;
  if (ioctl(file, I2C_SMBUS, &smbusArgs) < 0)
  {
    lasterror = errno;
//    printf("ERROR read reg %d filed, %s: line %d, (%d) - %s\n", reg, __func__, __LINE__, lasterror, strerror(lasterror));
    return -1;
  }
  *byteData = data.byte;
  return 0;
}
