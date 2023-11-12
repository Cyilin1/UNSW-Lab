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

void WriteLittleEndian(FILE *file, uint64_t value, size_t bytes)
{
    for (size_t i = 0; i < bytes; i++)
    {
        fputc((value >> (i * 8)) & 0xFF, file);
    }
}

uint64_t ReadLittleEndian(FILE *file, size_t bytes)
{
    uint64_t value = 0;
    for (size_t i = 0; i < bytes; i++)
    {
        value |= ((uint64_t)fgetc(file) << (i * 8));
    }
    return value;
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
            exit(1);
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
            exit(1);
        }
        for (size_t j = 0; j < num_blocks; j++)
        {
            char block_data[256];
            size_t bytes_read = fread(block_data, 1, 256, file);
            uint64_t block_hash = hash_block(block_data, bytes_read); // 调用哈希函数
            WriteLittleEndian(tabi_file, block_hash, 8);
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
    FILE *tabi_file = fopen(in_filename, "rb");
    FILE *tbbi_file = fopen(out_filename, "wb");

    if (!tabi_file || !tbbi_file)
    {
        perror("Error opening files");
        exit(1);
    }

    // Write TBBI magic number (4 bytes)
    char *magic_value = "TBBI";
    fwrite(magic_value, 1, 4, tbbi_file);
    // Read and check the magic number.
    char magic[4];
    if (fread(magic, 1, 4, tabi_file) != 4 || strncmp(magic, "TABI", 4) != 0)
    {
        perror("Error: Invalid magic number in TABI file");
        exit(1);
    }

    // fseek(tabi_file, 4, SEEK_SET);
    uint8_t num_records = (uint8_t)fgetc(tabi_file);
    fwrite(&num_records, 1, 1, tbbi_file);

    // Process each record in the TABI file.
    for (uint8_t record = 0; record < num_records; record++)
    {
        uint16_t pathname_length = ReadLittleEndian(tabi_file, 2);
        WriteLittleEndian(tbbi_file, pathname_length, 2);
        char pathname[pathname_length + 1];
        if (fread(pathname, sizeof(char), pathname_length, tabi_file) != pathname_length)
        {
            perror("Error reading pathname from TABI");
            exit(1);
        }
        pathname[pathname_length] = '\0';
        fwrite(pathname, 1, pathname_length, tbbi_file);
        uint64_t num_blocks = ReadLittleEndian(tabi_file, 3);
        WriteLittleEndian(tbbi_file, num_blocks, 3);
        //  Calculate the number of match bytes needed for this record.
        uint32_t num_match_bytes = num_tbbi_match_bytes(num_blocks);
        // Initialize an array to store match bytes.
        uint8_t match_bytes[num_match_bytes];
        memset(match_bytes, 0, num_match_bytes);
        FILE *receive_file = fopen(pathname, "rb");
        if (receive_file == NULL)
        {
            fwrite(match_bytes, 1, num_match_bytes, tbbi_file);
            fseek(tabi_file, 8 * num_blocks, SEEK_CUR);
            continue;
        }
        for (int i = 0; i < num_blocks; i++)
        { // Use num_blocks here
            char block_data[256];
            size_t bytes_read = fread(block_data, 1, 256, receive_file);
            uint64_t rev_block_hash = hash_block(block_data, bytes_read);
            uint64_t sent_block_hash = ReadLittleEndian(tabi_file, 8);

            if (rev_block_hash == sent_block_hash)
            {
                match_bytes[i / 8] |= (1 << (7 - (i % 8))); // Use | to set the bit, and adjust bit position.
            }
            else
            {
                // No need to do anything here; it's already initialized to 0.
            }
        }
        fwrite(match_bytes, 1, num_match_bytes, tbbi_file);
    }
    // if (!feof(tabi_file))
    // {
    //     // There is extra data in the file, report an error
    //     perror("");
    //     exit(1);
    // }
    char temp;
    if (fread(&temp, 1, 1, tabi_file) != 0)
    {
        fprintf(stderr, "Error: Extra data found in the TABI file");
        exit(1);
    }

    // Close the files.
    fclose(tabi_file);
    fclose(tbbi_file);
}

/// @brief Create a TCBI file from a TBBI file.
/// @param out_filename A path to where the new TCBI file should be created.
/// @param in_filename A path to where the existing TBBI file is located.
void stage_3(char *out_filename, char *in_filename)
{
    FILE *tbbi_file = fopen(in_filename, "rb");
    FILE *tcbi_file = fopen(out_filename, "wb");

    if (!tbbi_file || !tcbi_file)
    {
        perror("Error opening files");
        exit(1);
    }

    // Write TCBI magic number.
    char *magic_value = "TCBI";
    fwrite(magic_value, 1, 4, tcbi_file);

    // Read and check the magic number.
    char magic[4];
    if (fread(magic, 1, 4, tbbi_file) != 4 || strncmp(magic, "TBBI", 4) != 0)
    {
        perror("Error: Invalid magic number in TBBI file");
        exit(1);
    }

    // Count the number of records in the TBBI file.
    // Read and count TBBI records here.
    uint8_t num_records = (uint8_t)fgetc(tbbi_file);
    fwrite(&num_records, 1, 1, tcbi_file);
    // Process each record in the TBBI file.
    for (uint8_t record = 0; record < num_records; record++)
    {
        // Read and process TBBI record.
        uint16_t pathname_length = ReadLittleEndian(tbbi_file, 2);
        printf("%u\n", pathname_length);
        WriteLittleEndian(tcbi_file, pathname_length, 2);
        char pathname[pathname_length + 1];
        if (fread(pathname, sizeof(char), pathname_length, tbbi_file) != pathname_length)
        {
            perror("Error reading pathname from TABI");
            exit(1);
        }
        printf("%s\n", pathname);

        pathname[pathname_length] = '\0';
        fwrite(pathname, 1, pathname_length, tcbi_file);
        FILE *receive_file = fopen(pathname, "rb");
        if (receive_file == NULL)
        {
            continue;
        }
        uint32_t num_blocks = ReadLittleEndian(tbbi_file, 3);

        struct stat file_stat;
        if (stat(&pathname, &file_stat) != 0)
        {
            perror("无法获取文件大小");
            exit(1);
        }
        // write isDir
        char mode_isDIR = S_ISDIR(file_stat.st_mode) ? 'd' : '-';
        fwrite(&mode_isDIR, 1, 1, tcbi_file);
        // write Owner
        char mode_usr_pms[3];
        mode_usr_pms[0] = (file_stat.st_mode & S_IRUSR) ? 'r' : '-';
        mode_usr_pms[1] = (file_stat.st_mode & S_IWUSR) ? 'w' : '-';
        mode_usr_pms[2] = (file_stat.st_mode & S_IXUSR) ? 'x' : '-';
        fwrite(mode_usr_pms, 1, 3, tcbi_file);
        // write Group
        char mode_grp_pms[3];
        mode_grp_pms[0] = (file_stat.st_mode & S_IRGRP) ? 'r' : '-';
        mode_grp_pms[1] = (file_stat.st_mode & S_IWGRP) ? 'w' : '-';
        mode_grp_pms[2] = (file_stat.st_mode & S_IXGRP) ? 'x' : '-';
        fwrite(mode_grp_pms, 1, 3, tcbi_file);
        // write Others
        char mode_oth_pms[3];
        mode_oth_pms[0] = (file_stat.st_mode & S_IROTH) ? 'r' : '-';
        mode_oth_pms[1] = (file_stat.st_mode & S_IWOTH) ? 'w' : '-';
        mode_oth_pms[2] = (file_stat.st_mode & S_IXOTH) ? 'x' : '-';
        fwrite(mode_oth_pms, 1, 3, tcbi_file);
        // size
        uint32_t file_size = file_stat.st_size;
        WriteLittleEndian(tcbi_file, file_size, 4);
        //
        uint32_t num_need_update = 0;
        uint16_t index_capacity = 8;
        uint32_t *array = (uint32_t *)malloc(index_capacity * sizeof(uint32_t));
        if (array == NULL)
        {
            perror("Memory allocation error");
            exit(1);
        }
        uint32_t num_match_bytes = num_tbbi_match_bytes(num_blocks);

        for (int i = 0; i < num_match_bytes; i++)
        {
            uint8_t match_byte;
            if (fread(&match_byte, 1, 1, tbbi_file) != 1)
            {
                perror("x");
                exit(1);
            }

            // 统计字节中二进制位为0的个数
            for (int k = 0; k < 8; k++)
            {
                int index = i * 8 + k;
                if ((match_byte & (1 << (7 - k))) == 0)
                {
                    // 输出位置信息
                    // printf("i=%d, k=%d\n", i, k);
                    if (index >= num_blocks)
                        break;

                    array[num_need_update] = index;
                    num_need_update++;
                    if (num_need_update >= index_capacity)
                    {
                        // 如果动态数组已满，需要重新分配更大的内存
                        index_capacity *= 2;
                        uint32_t *new_array = (uint32_t *)realloc(array, index_capacity * sizeof(uint32_t));

                        if (new_array == NULL)
                        {
                            perror("Memory reallocation error");
                            free(array); // 释放旧内存
                            exit(1);
                        }
                        array = new_array;
                    }
                    // printf("%d\n", index);
                }
                else if (index + 1 > num_blocks)
                {
                    perror("x");
                    exit(1);
                }
            }
        }
        WriteLittleEndian(tcbi_file, num_need_update, 3);
        // printf("%u\n", num_need_update);
        for (int i = 0; i < num_need_update; i++)
        {
            WriteLittleEndian(tcbi_file, array[i], 3);
            uint32_t update_data_len;
            // printf("%d", 62 / 256);
            // todo
            // 如果是最后一块可以直接算
            if (array[i] != (num_blocks - 1))
            {
                update_data_len = 256;
            }
            else
            {
                update_data_len = file_size - 256 * (num_blocks - 1);
            }
            // printf("%u\n", update_data_len);
            WriteLittleEndian(tcbi_file, update_data_len, 2);

            char update_data[update_data_len];
            fseek(receive_file, array[i] * 256, SEEK_SET);
            fread(update_data, 1, update_data_len, receive_file);
            fwrite(update_data, 1, update_data_len, tcbi_file);
        }

        free(array);

        // // printf("%u", num_need_update);
    }
    // Close the files.
    char temp;
    if (fread(&temp, 1, 1, tbbi_file) != 0)
    {
        fprintf(stderr, "Error: Extra data found in the TABI file");
        exit(1);
    }
    fclose(tbbi_file);
    fclose(tcbi_file);
}

/// @brief Apply a TCBI file to the filesystem.
/// @param in_filename A path to where the existing TCBI file is located.
void stage_4(char *in_filename)
{
    // TODO: implement this.
}
