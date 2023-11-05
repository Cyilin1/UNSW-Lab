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
    if (tabi_file == NULL)
    {
        perror("Failed to open TABI file for reading");
        exit(1);
    }

    FILE *tbbi_file = fopen(out_filename, "wb");
    if (tbbi_file == NULL)
    {
        perror("Failed to create TBBI file");
        fclose(tabi_file);
        exit(1);
    }

    // Write TBBI magic number (4 bytes)
    char *magic_value = "TBBI";
    fwrite(magic_value, 1, 4, tbbi_file);
    fseek(tabi_file, 4, SEEK_SET);
    uint8_t num_records = (uint8_t)fgetc(tabi_file);
    fwrite(&num_records, 1, 1, tbbi_file);

    // Initialize a buffer to read data from TABI file
    uint16_t pathname_length;
    char pathname[256];
    uint32_t num_blocks;
    uint64_t *hashes;

    // Read and write records from TABI to TBBI
    while (fread(&pathname_length, sizeof(uint16_t), 1, tabi_file) == 1)
    {
        fread(pathname, sizeof(char), pathname_length, tabi_file);
        fread(&num_blocks, 3, 1, tabi_file);

        // Calculate the number of match bytes needed
        size_t num_match_bytes = (num_blocks + 7) / 8;

        // Allocate memory for matches
        uint8_t *matches = (uint8_t *)malloc(num_match_bytes);

        // Ensure successful allocation
        if (matches == NULL)
        {
            perror("Memory allocation error");
            exit(1);
        }

        // Read hashes, compare them, and set corresponding match bits
        hashes = (uint64_t *)malloc(num_blocks * sizeof(uint64_t));
        fread(hashes, sizeof(uint64_t), num_blocks, tabi_file);
        memset(matches, 0, num_match_bytes); // Initialize all match bytes to 0

        // Process the hashes and set match bits as needed
        for (uint32_t i = 0; i < num_blocks; i++)
        {
            // You need to implement hash comparison logic here
            // If the hashes match, set the corresponding match bit to 1
        }

        // Write the TBBI record
        create_tbbi_record(tbbi_file, pathname, num_blocks, matches);

        // Free memory used for matches and hashes
        free(matches);
        free(hashes);
    }

    // Close files
    fclose(tabi_file);
    fclose(tbbi_file);
}
// Function to create a TBBI record based on the provided information
void create_tbbi_record(FILE *tbbi_file, const char *pathname, uint32_t num_blocks, const uint8_t *matches)
{
    // Write the pathname length (16-bit little-endian)
    uint16_t pathname_length = (uint16_t)strlen(pathname);
    fwrite(&pathname_length, sizeof(uint16_t), 1, tbbi_file);

    // Write the pathname
    fwrite(pathname, sizeof(char), pathname_length, tbbi_file);

    // Write the number of blocks (24-bit little-endian)
    fwrite(&num_blocks, 3, 1, tbbi_file);

    // Write the matches field
    size_t num_match_bytes = (num_blocks + 7) / 8; // Calculate the number of match bytes needed
    fwrite(matches, 1, num_match_bytes, tbbi_file);
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
