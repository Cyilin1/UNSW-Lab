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

// 辅助函数：将整数值按小端序写入文件

void WriteLittleEndian(FILE *file, unsigned int value, size_t bytes)
{
    for (size_t i = 0; i < bytes; i++)
    {
        fputc((value >> (i * 8)) & 0xFF, file);
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
    FILE *tabi_file = fopen(out_filename, "wb");
    char *magic_value = "TABI";
    if (tabi_file == NULL)
    {
        perror("cannout create TABI, please check your instruction!");
        return;
    }
    fwrite(magic_value, 1, 4, tabi_file);
    // 写入记录数（1字节，8位）
    fputc(num_in_filenames, tabi_file);

    // 遍历输入文件名
    for (size_t i = 0; i < num_in_filenames; i++)
    {
        // struct stat 用于保存文件的各种信息。
        struct stat file_stat;
        if (stat(in_filenames[i], &file_stat) != 0)
        {
            perror("无法获取文件大小");
            continue;
        }
        size_t file_size = (size_t)file_stat.st_size;

        // 计算所需块数
        size_t num_blocks = number_of_blocks_in_file(file_size);

        // 写入文件路径的长度（2字节小端序）
        WriteLittleEndian(tabi_file, (unsigned int)strlen(in_filenames[i]), 2);

        // 写入文件路径
        fwrite(in_filenames[i], 1, strlen(in_filenames[i]), tabi_file);

        // 写入块数（3字节小端序）
        WriteLittleEndian(tabi_file, (unsigned int)num_blocks, 3);

        // 计算并写入块的哈希值
        FILE *file = fopen(in_filenames[i], "rb");
        if (file == NULL)
        {
            perror("无法打开文件");
            continue;
        }
        for (size_t j = 0; j < num_blocks; j++)
        {
            char block_data[256];
            size_t bytes_read = fread(block_data, 1, 256, file);
            if (bytes_read < 256)
            {
                // 如果不足256字节，填充零
                memset(block_data + bytes_read, 0, 256 - bytes_read);
            }
            unsigned long long block_hash = hash_block(block_data, 256); // 调用哈希函数
            fwrite(&block_hash, 8, 1, tabi_file);
        }
        fclose(file);
    }

    // 关闭TABI文件
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
