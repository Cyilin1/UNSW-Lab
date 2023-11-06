////////////////////////////////////////////////////////////////////////
// DPST1092 23T3 --- Assignment 2: `basin', a simple file synchroniser
// <https://cgi.cse.unsw.edu.au/~dp1092/23T3/assignments/ass2/index.html>
//
// Written by YOUR-NAME-HERE (z5555555) on INSERT-DATE-HERE.
// INSERT-DESCRIPTION-OF-PROGAM-HERE
//

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "basin.h"

/*
 * @brief 将整数值按小端序写入文件
 *
 * @param file 要写入的文件
 * @param value 要写入的值
 * @param bytes 这个值占多少个byte
 */

// 0x123456     78(0111 1000)
// 0x000000     FF(1111 1111)   (& 0xFF)
// 0x000000     78(0111 1000)

// 0x00123456
void Little_Endian_To_File(FILE *file, uint64_t value, size_t bytes)
{
    for (size_t i = 0; i < bytes; i++)
    {
        fputc((value >> (8 * i)) & 0xFF, file);
    }
}

/// @brief Create a TABI file from an array of filenames.
/// @param out_filename A path to where the new TABI file should be created.
/// @param in_filenames An array of strings containing, in order, the files
//                      that should be placed in the new TABI file.
/// @param num_in_filenames The length of the `in_filenames` array. In
///                         subset 5, when this is zero, you should include
///                         everything in the current directory.
void stage_1(char *out_filename, char *in_filenames[], size_t num_in_filenames)
{
    // TODO: implement this.
    char *magic_string = "TABI";
    FILE *tabi_file = fopen(out_filename, "w");
    if (tabi_file == NULL)
    {
        perror("file cannot open!");
        exit(1);
    }
    // write MagicNumer
    fwrite(magic_string, 1, 4, tabi_file);
    // write number of records
    fputc(num_in_filenames, tabi_file);
    // 遍历文件，写入每一个文件的信息write records，把下面的内容一项一项地写进去
    // 注意，如果大于2字节就要使用我们自己的函数
    // pathname len       0x00000005    0a 00                    dec 10
    // pathname           0x00000007    65 6d 6f 6a 69 73 2e 74  chr emojis.t
    // num blocks         0x00000011    03 00 00                 dec 3
    // hashes[0]          0x00000014    90 30 e3 14 6e e7 0a 90  chr .0..n...
    for (int i = 0; i < num_in_filenames; i++)
    {
        struct stat record_stat;
        // error handing
        if (stat(in_filenames[i], &record_stat) != 0)
        {
            perror("file cannot open!");
            exit(1);
        }
        // write length
        unsigned short file_len = strlen(in_filenames[i]);
        Little_Endian_To_File(tabi_file, file_len, 2);
        // write filename
        fwrite(in_filenames[i], 1, file_len, tabi_file);
        size_t file_size = record_stat.st_size;
        size_t block_num = number_of_blocks_in_file(file_size);
        Little_Endian_To_File(tabi_file, block_num, 3);
        FILE *arg_file = fopen(in_filenames[i], "rb");
        if (arg_file == NULL)
        {
            perror("file cannot open!");
            exit(1);
        }
        for (int j = 0; j < block_num; j++)
        {
            // 定义一个256字节的数组
            char block_data[256];
            // 从我们的arg_file文件中读入256个字节到block_data
            // bytes_read是我们读进来的字节长度 257 = 256 + 1
            size_t bytes_read = fread(block_data, 1, 256, arg_file);
            // 调用provided function，得到我们的hash值
            uint64_t block_hash = hash_block(block_data, bytes_read);
            Little_Endian_To_File(tabi_file, block_hash, 8);
        }
        fclose(arg_file);
    }
    fclose(tabi_file);
    // printf("TABI文件已创建：%s\n", out_filename);
}

/// @brief Create a TBBI file from a TABI file.
/// @param out_filename A path to where the new TBBI file should be created.
/// @param in_filename A path to where the existing TABI file is located.
void stage_2(char *out_filename, char *in_filename)
{
    // TODO: implement this.
}

/// @brief Create a TCBI file from a TBBI file.
/// @param out_filename A path to where the new TCBI file should be created.
/// @param in_filename A path to where the existing TBBI file is located.
void stage_3(char *out_filename, char *in_filename)
{
    // TODO: implement this.
}

/// @brief Apply a TCBI file to the filesystem.
/// @param in_filename A path to where the existing TCBI file is located.
void stage_4(char *in_filename)
{
    // TODO: implement this.
}
