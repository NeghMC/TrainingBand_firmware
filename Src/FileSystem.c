#include <ext_FLASH.h>
#include <stm32l031xx.h>
#include <SPI.h>
//#include <lfs.h>
#include "spiffs.h"
#include "spiffs_nucleus.h"

static struct {
	uint8_t opto;
	uint32_t address :24;
} __attribute__((packed)) flashCmd;

/*
 static int block_device_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size) {
 flashCmd.opto = EXT_FLASH_COMMAND_READ_ARRAY;
 flashCmd.address = block * c->block_size + off;

 SPI_csClear();
 SPI_blokingWrite((uint8_t*)&flashCmd, sizeof(flashCmd));
 SPI_blokingRead(buffer, size);
 SPI_csSet();

 return 0;
 }

 static int block_device_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size) {
 flashCmd.opto = EXT_FLASH_COMMAND_WRITE_PAGE;
 flashCmd.address = block * c->block_size + off;

 SPI_csClear();
 SPI_blokingWrite((uint8_t*)&flashCmd, sizeof(flashCmd));
 SPI_blokingWrite((uint8_t*)buffer, size);
 SPI_csSet();

 return 0;
 }

 static int block_device_erase(const struct lfs_config *c, lfs_block_t block) {
 flashCmd.opto = EXT_FLASH_COMMAND_ERASE_4K;
 flashCmd.address = block * c->block_size;

 SPI_csClear();
 SPI_blokingWrite((uint8_t*)&flashCmd, sizeof(flashCmd));
 SPI_csSet();

 return 0;
 }

 static int block_device_sync(const struct lfs_config *c) {
 return 0;
 }

 // configuration of the filesystem is provided by this struct
 static const struct lfs_config cfg = {
 // block device operations
 .read  = block_device_read,
 .prog  = block_device_prog,
 .erase = block_device_erase,
 .sync  = block_device_sync,

 // block device configuration
 .read_size = 16,
 .prog_size = 16,
 .block_size = 4096,
 .block_count = 128,
 .cache_size = 16,
 .lookahead_size = 16,
 .block_cycles = 500,
 };

 static lfs_t lfs;

 void fs_init(void) {
 // initialise SPI
 SPI_init();

 // set Write permission to FLASH memory
 SPI_csClear();
 flashCmd.opto = EXT_FLASH_COMMAND_WRITE_ENABLE;
 SPI_blokingWrite((uint8_t*)&flashCmd, 1);
 SPI_csSet();

 // mount the filesystem
 if (lfs_mount(&lfs, &cfg)) {
 // reformat if we can't mount the filesystem this should only happen on the first boot
 lfs_format(&lfs, &cfg);
 lfs_mount(&lfs, &cfg);
 }
 }

 int fs_open(lfs_file_t * file, const char * path, int flags) {
 return lfs_file_open(&lfs, file, path, flags);
 }

 lfs_ssize_t fs_read(lfs_file_t * file, void * buffer, lfs_size_t size) {
 return lfs_file_read(&lfs, file, buffer, size);
 }

 lfs_soff_t fs_seek(lfs_file_t *file, lfs_soff_t off, int whence) {
 return lfs_file_seek(&lfs, file, off, whence);
 }

 lfs_ssize_t fs_write(lfs_file_t *file, const void *buffer, lfs_size_t size) {
 return lfs_file_write(&lfs, file, buffer, size);
 }

 int fs_flush(lfs_file_t *file) {
 return lfs_file_sync(&lfs, file);
 }

 int fs_close(lfs_file_t *file) {
 return lfs_file_close(&lfs, file);
 }*/

static int32_t _spiffs_erase(uint32_t addr, uint32_t len) {
	uint32_t i = 0;
	uint32_t erase_count = (len + 4096 - 1) / 4096;
	for (i = 0; i < erase_count; i++) {
		//BSP_QSPI_Erase_Block(addr + i * 4096);

		flashCmd.opto = EXT_FLASH_COMMAND_ERASE_4K;
		flashCmd.address = addr;

		SPI_csClear();
		SPI_blokingWrite((uint8_t*) &flashCmd, sizeof(flashCmd));
		SPI_csSet();
	}
	return 0;
}

static int32_t _spiffs_read(uint32_t addr, uint32_t size, uint8_t *dst) {
	flashCmd.opto = EXT_FLASH_COMMAND_READ_ARRAY;
	flashCmd.address = addr;

	SPI_csClear();
	SPI_blokingWrite((uint8_t*) &flashCmd, sizeof(flashCmd));
	SPI_blokingRead(dst, size);
	SPI_csSet();

	return 0;
}

static int32_t _spiffs_write(uint32_t addr, uint32_t size, uint8_t *dst) {
	flashCmd.opto = EXT_FLASH_COMMAND_WRITE_PAGE;
	flashCmd.address = addr;

	SPI_csClear();
	SPI_blokingWrite((uint8_t*) &flashCmd, sizeof(flashCmd));
	SPI_blokingWrite(dst, size);
	SPI_csSet();

	return 0;
}

static spiffs fs;
static spiffs_config cfg = {
		.hal_erase_f = _spiffs_erase,
		.hal_read_f = _spiffs_read,
		.hal_write_f = _spiffs_write
};

uint8_t FS_Work_Buf[128 * 2];
uint8_t FS_FDS[32 * 2];
uint8_t FS_Cache_Buf[(128 + 32) * 2];

spiffs * fs_init(void) {
// initialise SPI
	SPI_init();

// set Write permission to FLASH memory
	SPI_csClear();
	flashCmd.opto = EXT_FLASH_COMMAND_WRITE_ENABLE;
	SPI_blokingWrite((uint8_t*) &flashCmd, 1);
	SPI_csSet();
	if (SPIFFS_mount(&fs,
	                 &cfg,
	                 FS_Work_Buf,
	                 FS_FDS, sizeof(FS_FDS),
					 FS_Cache_Buf, sizeof(FS_Cache_Buf),
	                 NULL) != SPIFFS_OK) {
		if(SPIFFS_errno(&fs) != SPIFFS_ERR_NOT_A_FS) {
			return NULL;
		} else {
			// format needed
			if (SPIFFS_format(&fs) != SPIFFS_OK) {
				return NULL;
	        } else {
	            if(SPIFFS_mount(&fs,
	                           &cfg,
	                           FS_Work_Buf,
	                           FS_FDS, sizeof(FS_FDS),
	                           FS_Cache_Buf, sizeof(FS_Cache_Buf),
	                           NULL) != SPIFFS_OK) {
	            	return NULL;
	            }
	        }
		}
	}
	return &fs; // success
}

/* example code:
 * {
        spiffs_file fd = SPIFFS_open(&fs, "my_file", SPIFFS_CREAT | SPIFFS_TRUNC | SPIFFS_RDWR, 0);
        SEGGER_RTT_printf("errno %d\n", SPIFFS_errno(&fs));
        if (SPIFFS_write(&fs, fd, WriteBuf, sizeof(WriteBuf)) < 0)
            SEGGER_RTT_printf("errno %d\n", SPIFFS_errno(&fs));
        SPIFFS_close(&fs, fd);

        fd = SPIFFS_open(&fs, "my_file", SPIFFS_RDWR, 0);
        if (SPIFFS_read(&fs, fd, ReadBuf, sizeof(WriteBuf)) < 0)
            SEGGER_RTT_printf("errno %d\n", SPIFFS_errno(&fs));
        SPIFFS_close(&fs, fd);
        LL_mDelay(1000);
        SPIFFS_info(&fs, &total, &used_space);
        SEGGER_RTT_printf("mybuf = %s\n", ReadBuf);
    }
 */

