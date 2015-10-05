
#include <byteswap.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/mman.h>

#define SPIDEV               "/dev/spidev32766.0"
#define CLKING_AXI_ADDR      0x51010000
#define PULP_CTRL_AXI_ADDR   0x51000000


#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

int clock_manager();
int process_file(char* buffer, size_t size);

int pulp_ctrl(int fetch_en, int reset) {
  char* ctrl_map = MAP_FAILED;
  char* gpio_base;
  int mem_fd;
  int retval = 0;

  if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
    printf("can't open /dev/mem \n");

    retval = -1;
    goto fail;
  }

  ctrl_map = (char*)mmap(
      NULL,
      MAP_SIZE,
      PROT_READ|PROT_WRITE,
      MAP_SHARED,
      mem_fd,
      PULP_CTRL_AXI_ADDR & ~MAP_MASK
      );


  if (ctrl_map == MAP_FAILED) {
    perror("mmap error\n");

    retval = -1;
    goto fail;
  }

  gpio_base = ctrl_map + (PULP_CTRL_AXI_ADDR & MAP_MASK);
  volatile uint32_t* gpio2 = (volatile uint32_t*)(gpio_base + 0x8);
  volatile uint32_t* dir2  = (volatile uint32_t*)(gpio_base + 0xC);

  // now we can actually write to the peripheral
  uint32_t val = 0x0;
  if (reset == 0)
    val |= (1 << 31); // reset is active low

  if (fetch_en)
    val |= (1 << 0);

  *dir2  = 0x0; // configure as output
  *gpio2 = val;

fail:
  close(mem_fd);

  if(ctrl_map != MAP_FAILED)
    munmap(ctrl_map, MAP_SIZE);

  return retval;
}

int spi_read_reg(unsigned int addr) {
  int fd;
  char wr_buf[256];
  char rd_buf[256];
  unsigned int i;
  size_t transfer_len = 5;
  int retval = 0;


  // open spidev
  fd = open(SPIDEV, O_RDWR);
  if (fd <= 0) {
    perror("Device not found\n");

    retval = -1;
    goto fail;
  }

  struct spi_ioc_transfer transfer = {
    .tx_buf        = 0,
    .rx_buf        = 0,
    .len           = 0,
    .delay_usecs   = 0,
    .speed_hz      = 0,
    .bits_per_word = 0,
  };

  transfer.tx_buf = (unsigned long)wr_buf;
  transfer.rx_buf = (unsigned long)rd_buf;
  transfer.len    = transfer_len;

  memset(wr_buf, 0, transfer.len);
  memset(rd_buf, 0, transfer.len);

  switch(addr) {
    case 0: wr_buf[0] = 0x05; break; // read reg0
    case 1: wr_buf[0] = 0x07; break; // read reg1
    case 2: wr_buf[0] = 0x21; break; // read reg2
    case 3: wr_buf[0] = 0x30; break; // read reg3
    default:
            printf("Not a valid address for reading a register\n");
            goto fail;
  }


  // check if write was sucessful
  if (ioctl(fd, SPI_IOC_MESSAGE(1), &transfer) < 0) {
    perror("SPI_IOC_MESSAGE");
    retval = -1;
    goto fail;
  }

  for(i = 4; i < transfer.len; i++)
    printf("Got %X\n", rd_buf[i]);

fail:
  // close spidev
  close(fd);

  return retval;
}

int spi_load(uint32_t addr, char* in_buf, size_t in_size) {
  int fd;
  char* wr_buf;
  char* rd_buf;
  unsigned int i;
  size_t size;
  size_t transfer_len;
  int retval = 0;

  // make sure transfers are 32 bit aligned
  if ((in_size & 0x3) == 0)
    size = in_size;
  else
    size = (in_size & (~0x3)) + 0x4;

  transfer_len = size + 9 + 4 + 4;

  wr_buf = (char*)malloc(transfer_len);
  if (wr_buf == NULL) {
    printf("Unable to acquire write buffer\n");

    retval = -1;
    goto fail;
  }

  memset(wr_buf, 0, transfer_len);

  wr_buf[0] = 0x02; // write command
  // address
  wr_buf[1] = addr >> 24;
  wr_buf[2] = addr >> 16;
  wr_buf[3] = addr >> 8;
  wr_buf[4] = addr;

  memcpy(wr_buf + 5, in_buf, in_size);

  // open spidev
  fd = open(SPIDEV, O_RDWR);
  if (fd <= 0) {
    perror("Device not found\n");

    retval = -1;
    goto fail;
  }

  // write to spidev
  if (write(fd, wr_buf, size + 5) != (size + 5)) {
    perror("Write Error");

    retval = -1;
    goto fail;
  }


  // prepare for readback
  rd_buf = (char*)malloc(transfer_len);
  if (rd_buf == NULL) {
    printf("Unable to acquire buffer to check if write was sucessful\n");

    retval = -1;
    goto fail;
  }

  struct spi_ioc_transfer transfer = {
    .tx_buf        = 0,
    .rx_buf        = 0,
    .len           = 0,
    .delay_usecs   = 0,
    .speed_hz      = 0,
    .bits_per_word = 0,
  };

  transfer.tx_buf = (unsigned long)wr_buf;
  transfer.rx_buf = (unsigned long)rd_buf;
  transfer.len    = transfer_len;

  memset(wr_buf, 0, transfer.len);
  memset(rd_buf, 0, transfer.len);

  wr_buf[0] = 0x0B; // read command
  // address
  wr_buf[1] = addr >> 24;
  wr_buf[2] = addr >> 16;
  wr_buf[3] = addr >> 8;
  wr_buf[4] = addr;

  // check if write was sucessful
  if (ioctl(fd, SPI_IOC_MESSAGE(1), &transfer) < 0) {
    perror("SPI_IOC_MESSAGE");
    retval = -1;
    goto fail;
  }

  // shift everything by one bit
  for(i = 0; i < transfer.len-1; i++) {
    rd_buf[i] = (rd_buf[i] << 1) | ((rd_buf[i+1] & 0x80) >> 7);
  }

  for(i = 0; i < in_size; i++) {
    if (in_buf[i] != rd_buf[i + 13]) {
      printf("Read check failed at idx %d: Expected %02X, got %02X\n", i, in_buf[i], rd_buf[i + 13]);
    }
  }

fail:
  // close spidev
  close(fd);

  if (wr_buf != NULL)
    free(wr_buf);

  if (rd_buf != NULL)
    free(rd_buf);

  return retval;
}

int main(int argc, char **argv)
{
  int fd;
  char* buffer;
  unsigned int size;
  int i;

  clock_manager();

  if (argc < 2) {
    printf("Usage spiload <BINARY>\n");
    return 1;
  }

  // open binary and get data
  fd = open(argv[1], O_RDWR);
  if (fd <= 0) {
    perror("File could not be opened\n");
    return 1;
  }

  size = lseek(fd, 0, SEEK_END);
  if (size == -1) {
    perror("Could not determine file size\n");
    return 1;
  }

  buffer = (char*)malloc(size);
  if(buffer == NULL) {
    printf("Could not allocate memory for file buffer\n");
  }

  if (lseek(fd, 0, SEEK_SET) == -1) {
    perror("Could not jump to start of file\n");
    return 1;
  }

  if (read(fd, buffer, size) != size) {
    close(fd);
    perror("Read Error");
    return -1;
  }

  // reset device
  pulp_ctrl(0, 1);
  pulp_ctrl(0, 0);

  printf("Device has been reset\n");

  process_file(buffer, size);

  printf("Starting device\n");
  pulp_ctrl(1, 0);

  free(buffer);
  close(fd);

  return 0;
}

#define NUM_ENTRIES  32768

int process_file(char* buffer, size_t size) {
  uint32_t addr[NUM_ENTRIES];
  uint32_t data[NUM_ENTRIES];
  unsigned int entries = 0;

  // extract lines
  const char const* buffer_end = buffer + size;
  char line[20];
  unsigned int i;

  while(buffer != buffer_end) {
    // build lines
    i = 0;
    while (buffer != buffer_end) {
      line[i] = *buffer++;

      if (line[i] == '\n') {
        line[i] = '\0';
        break;
      }

      if (buffer == buffer_end) {
        line[i+1] = '\0';
        break;
      }

      i++;
      if (i == 18) {
        printf("Failed to parse, couldn't find line\n");
        return -1;
      }
    }

    sscanf(line, "%X_%X", &addr[entries], &data[entries]);

    // convert data
    data[entries] = __bswap_32(data[entries]);
    //printf("Line %s, addr %08X, data %08X\n", line, addr[entries], data[entries]);

    entries++;

    if (entries == NUM_ENTRIES) {
      printf("Too many entries in file\n");
      return -1;
    }
  }

  if (entries == 0) {
    printf("No entries found\n");
    return -1;
  }

  // now find consecutive addresses to build blocks
  unsigned int start_idx = 0;
  for(i = 1; i < entries; i++) {
    if(addr[i] != (addr[i-1] + 0x4) || (i - start_idx) == 255 || i == (entries - 1)) {
      // send block
      printf("Sending block addr %08X with %d entries\n", addr[start_idx], i - start_idx + 1);
      spi_load(addr[start_idx], (char*)&data[start_idx], (i - start_idx + 1) * 4);
      start_idx = i;
    }
  }

  return 0;
}

int clock_manager() {
  char* clk_map = MAP_FAILED;
  char* clk_base;
  int mem_fd;
  int retval = 0;

  if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
    printf("can't open /dev/mem \n");

    retval = -1;
    goto fail;
  }

  clk_map = (char*)mmap(
      NULL,
      MAP_SIZE,
      PROT_READ|PROT_WRITE,
      MAP_SHARED,
      mem_fd,
      CLKING_AXI_ADDR & ~MAP_MASK
      );


  if (clk_map == MAP_FAILED) {
    perror("mmap error\n");

    retval = -1;
    goto fail;
  }

  clk_base = clk_map + (CLKING_AXI_ADDR & MAP_MASK);
  volatile uint32_t* sr = (volatile uint32_t*)(clk_base + 0x4);
  volatile uint32_t* ccr0  = (volatile uint32_t*)(clk_base + 0x200);

  printf("SR   is %08X\n", *sr);
  printf("CCR0 is %08X\n", *ccr0);

fail:
  close(mem_fd);

  if(clk_map != MAP_FAILED)
    munmap(clk_map, MAP_SIZE);

  return retval;
}