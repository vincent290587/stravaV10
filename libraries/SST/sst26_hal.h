/*
 * sst26_hal.h
 *
 *  Created on: 25 juil. 2018
 *      Author: Vincent
 */

#ifndef LIBRARIES_SST_SST26_HAL_H_
#define LIBRARIES_SST_SST26_HAL_H_

/* SST26 Registers *******************************************************************/
/* Indentification register values */

#define SST26_MANUFACTURER         0xBF
#define SST26_MEMORY_TYPE          0x26

#define SST26_SST26VF016_CAPACITY    0x41 /* 16 M-bit */
#define SST26_SST26VF032_CAPACITY    0x42 /* 32 M-bit */
#define SST26_SST26VF064_CAPACITY    0x43 /* 64 M-bit */

#define SST26_CAPACITY       SST26_SST26VF032_CAPACITY // Select chip version/capacity

#define SST26_PAGE_SIZE       256  // Bytes
#define SST26_SECTOR_SIZE     4096 // Bytes

#if SST26_CAPACITY == SST26_SST26VF016_CAPACITY
/* SST26VF016 capacity is 2,097,152 bytes:
 * (512 sectors) * (4,096 bytes per sector)
 * (8192 pages) * (256 bytes per page)
 */
#define SST26_SECTOR_SHIFT  12    /* Sector size 1 << 15 = 65,536 */
#define SST26_NSECTORS      512
#define SST26_PAGE_SHIFT    8     /* Page size 1 << 8 = 256 */
#define SST26_NPAGES        8192

#elif SST26_CAPACITY == SST26_SST26VF032_CAPACITY
/* SST26VF032 capacity is 4,194,304 bytes:
 * (1,024 sectors) * (4,096 bytes per sector)
 * (16,384 pages) * (256 bytes per page)
 */
#define SST26_SECTOR_SHIFT  12    /* Sector size 1 << 15 = 65,536 */
#define SST26_NSECTORS      1024
#define SST26_PAGE_SHIFT    8     /* Page size 1 << 8 = 256 */
#define SST26_NPAGES        16384

#elif SST26_CAPACITY == SST26_SST26VF064_CAPACITY
/* SST26VF064 capacity is 8,388,608 bytes:
 * (2,048 sectors) * (4,096 bytes per sector)
 * (32,768 pages) * (256 bytes per page)
 */
#define SST26_SECTOR_SHIFT  12    /* Sector size 1 << 15 = 65,536 */
#define SST26_NSECTORS      2048
#define SST26_PAGE_SHIFT    8     /* Page size 1 << 8 = 256 */
#define SST26_NPAGES        32768

#else
#error "Unknown SST26 version"
#endif

/* Instructions */
/*      Command         Value      NN Description                    Addr Dummy  Data     */
#define SST26_NOP       0x00    /* 14 No Operation                    0     0     0       */
#define SST26_RSTEN     0x66    /* 14 Reset Enable                    0     0     0       */
#define SST26_RST       0x99    /* 14 Reset Memory                    0     0     0       */
#define SST26_EQIO      0x38    /*  1 Enable Quad I/O                 0     0     0       */
#define SST26_RSTQIO    0xFF    /*  4 Reset Quad I/O                  0     0     0       */
#define SST26_RDSR      0x05    /*  1 Read Status Register            0     0     >=1     */
/*  4 Read Status Register            0     1     >=1     */
#define SST26_WRSR      0x01    /* 14 Write Status Register           0     0     2       */
#define SST26_RDCR      0x35    /*  1 Read Config Register            0     0     >=1     */
/*  4 Read Config Register            0     1     >=1     */

#define SST26_READ      0x03    /*  1 Read Data Bytes                 3     0     >=1     */
#define SST26_FAST_READ 0x0b    /*  1 Higher speed read               3     1     >=1     */
/*  4 Higher speed read               3     3     >=1     */
#define SST26_SQOR      0x6b    /*  1 SQI Output Read                 3     1     >=1     */
#define SST26_SQIOR     0xeb    /*  1 SQI I/O Read                    3     3     >=1     */
#define SST26_SDOR      0x3b    /*  1 SDI Output Read                 3     1     >=1     */
#define SST26_SDIOR     0xbb    /*  1 SDI I/O Read                    3     1     >=1     */
#define SST26_SB        0xc0    /* 14 Set Burst Length                0     0     1       */
#define SST26_RBSQI     0x0c    /*  4 SQI Read Burst w/ Wrap          3     3     >=1     */
#define SST26_RBSPI     0xec    /*  1 SPI Read Burst w/ Wrap          3     3     >=1     */

#define SST26_RDID      0x9f    /*  1 Read Identification             0     0     >=3     */
#define SST26_QRDID     0xaf    /*  4 Quad Read Identification        0     1     >=3     */
#define SST26_SFDP      0x5a    /*  1 Serial Flash Discov. Par.       3     1     >=1     */

#define SST26_WREN      0x06    /* 14 Write Enable                    0     0     0       */
#define SST26_WRDI      0x04    /* 14 Write Disable                   0     0     0       */
#define SST26_SE        0x20    /* 14 Sector Erase                    3     0     0       */
#define SST26_BE        0xd8    /* 14 8/32/64K Block Erase            3     0     0       */
#define SST26_CE        0xc7    /* 14 Chip Erase                      0     0     0       */
#define SST26_PP        0x02    /*  1 Page Program                    3     0     1-256   */
#define SST26_QPP       0x32    /*  1 Quad Page Program               3     0     1-256   */
#define SST26_WRSU      0xb0    /* 14 Suspend Program/Erase           0     0     0       */
#define SST26_WRRE      0x30    /* 14 Resume Program/Erase            0     0     0       */

#define SST26_RBPR      0x72    /*  1 Read Block-Protection reg       0     0     1-18    */
/*  4 Read Block-Protection reg       0     1     1-18    */
#define SST26_WBPR      0x42    /* 14 Write Block-Protection reg      0     0     1-18    */
#define SST26_LBPR      0x8d    /* 14 Lock down Block-Prot. reg       0     0     0       */
#define SST26_NVWLDR    0xe8    /* 14 non-Volatile Write L-D reg      0     0     1-18    */
#define SST26_ULBPR     0x98    /* 14 Global Block Protection unlock  0     0     0       */
#define SST26_RSID      0x88    /* 14 Read Security ID                2     1     1-2048  */
/*  4 Read Security ID                2     3     1-2048  */
#define SST26_PSID      0xa5    /* 14 Program User Security ID area   2     0     1-256   */
#define SST26_LSID      0x85    /* 14 Lockout Security ID programming 0     0     0       */

/* NOTE 1: All parts.
 * NOTE 2: In SST26VF064 terminology, 0xd8 is block erase and 0x20
 *         is a sector erase.  Block erase provides a faster way to erase
 *         multiple 4K sectors at once.
 */

/* Status register bit definitions */

#define SST26_SR_WIP              (1 << 0)                /* Bit 0: Write in progress */
#define SST26_SR_WEL              (1 << 1)                /* Bit 1: Write enable latch */
#define SST26_SR_WSE              (1 << 2)                /* Bit 2: Write Suspend-Erase Status */
#define SST26_SR_WSP              (1 << 3)                /* Bit 3: Write Suspend-Program Status */
#define SST26_SR_WPLD             (1 << 4)                /* Bit 4: Write Protection Lock-Down Status */
#define SST26_SR_SEC              (1 << 5)                /* Bit 5: Security ID status */
#define SST26_SR_RES              (1 << 6)                /* Bit 6: RFU */
#define SST26_SR_WIP2             (1 << 7)                /* Bit 7: Write in progress */


#ifdef __cplusplus
extern "C" {
#endif

void _chip_enable_low();

void _chip_enable_high();

bool spi_send_sync(const uint8_t *data, size_t data_nb);

bool spi_receive_sync(uint8_t *data, size_t data_nb);

bool spi_transceive_sync(const uint8_t *data_in, size_t data_in_nb, uint8_t *data_out, size_t data_out_nb);

void sst_init_spi(void);

#ifdef __cplusplus
}
#endif

#endif /* LIBRARIES_SST_SST26_HAL_H_ */
