#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>

#include "basin.h"
#define MAGIC_VALUE "TABI"

uint32_t read_little_endian_int(FILE *file, size_t bytes)
{
    uint32_t value = 0;
    for (size_t i = 0; i < bytes; i++)
    {
        value |= ((uint32_t)fgetc(file)) << (i * 8);
    }
    return value;
}

void basin_show(const char *tabi_filename)
{
    FILE *tabi_file = fopen(tabi_filename, "rb");
    if (tabi_file == NULL)
    {
        perror("无法打开TABI文件");
        return;
    }

    // 读取并验证魔术值
    char magic_check[4];
    fread(magic_check, 1, 4, tabi_file);
    if (memcmp(magic_check, MAGIC_VALUE, 4) != 0)
    {
        printf("不是有效的TABI文件\n");
        fclose(tabi_file);
        return;
    }

    // 读取记录数
    uint32_t num_records = (uint32_t)fgetc(tabi_file);

    printf("Field name         Offset        Bytes                    ASCII/Numeric\n");
    printf("-----------------------------------------------------------------------\n");
    printf("magic              0x00000000    54 41 42 49              chr TABI\n");
    printf("num records        0x00000004    %02x                       dec %u\n", num_records, num_records);

    // 遍历每个记录
    for (uint32_t record_index = 0; record_index < num_records; record_index++)
    {
        printf("============================= Record   %u ==============================\n", record_index);

        // 读取并显示文件路径长度
        uint16_t path_len = read_little_endian_int(tabi_file, 2);
        printf("pathname len       0x%08x    %02x %02x                    dec %u\n", path_len, (path_len & 0xFF), (path_len >> 8), path_len);

        // 读取并显示文件路径
        char path_buffer[512];
        fread(path_buffer, 1, path_len, tabi_file);
        printf("pathname           0x%08x    ", ftell(tabi_file));
        for (uint16_t i = 0; i < path_len; i++)
        {
            printf("%c", path_buffer[i]);
        }
        printf("  chr ");
        for (uint16_t i = 0; i < path_len; i++)
        {
            printf("%02x ", (unsigned char)path_buffer[i]);
        }
        printf("\n");

        // 读取并显示块数
        uint32_t num_blocks = read_little_endian_int(tabi_file, 3);
        printf("num blocks         0x%08x    %02x %02x %02x                 dec %u\n", num_blocks, (num_blocks & 0xFF), ((num_blocks >> 8) & 0xFF), (num_blocks >> 16), num_blocks);

        // 遍历块并读取哈希值
        for (uint32_t block_index = 0; block_index < num_blocks; block_index++)
        {
            uint64_t block_hash = 0;
            for (int i = 0; i < 8; i++)
            {
                block_hash |= ((uint64_t)fgetc(tabi_file)) << (i * 8);
            }
            printf("hashes[%u]          0x%08x    ", block_index, ftell(tabi_file));
            for (int i = 0; i < 8; i++)
            {
                printf("%02x ", (unsigned char)(block_hash & 0xFF));
                block_hash >>= 8;
            }
            printf("  chr ");
            for (int i = 0; i < 8; i++)
            {
                printf("%02x ", (unsigned char)(block_hash & 0xFF));
                block_hash >>= 8;
            }
            printf("\n");
        }
    }

    fclose(tabi_file);
}

int main(int argc, char **argv)
{
    int stage = 0;
    for (;;)
    {
        int option_index;
        int opt = getopt_long(
            argc, argv,
            ":1234",
            (struct option[]){
                {"stage-1", no_argument, NULL, 1},
                {"stage-2", no_argument, NULL, 2},
                {"stage-3", no_argument, NULL, 3},
                {"stage-4", no_argument, NULL, 4},
                {0, 0, 0, '?'},
            },
            &option_index);

        if (opt == -1)
        {
            break;
        }

        switch (opt)
        {
        case 1:
        case 2:
        case 3:
        case 4:
        {
            stage = opt;
            break;
        }
        case ':':
        case '?':
        default:
        {
            fprintf(stderr, "Usage: %s [--stage-1|--stage-2|--stage-3|--stage-4]\n", argv[0]);
            return EXIT_FAILURE;
        }
        }
    }

    switch (stage)
    {
    case 1:
    {
        if (argc - optind < 1)
        {
            fprintf(stderr, "Usage: %s --stage-1 <outfile> [<file> ...]\n", argv[0]);
            return EXIT_FAILURE;
        }
        char *outfile = argv[optind];
        char **files = argv + optind + 1;
        size_t num_files = argc - optind - 1;
        stage_1(outfile, files, num_files);
        break;
    }
    case 2:
    {
        if (argc - optind != 2)
        {
            fprintf(stderr, "Usage: %s --stage-2 <outfile> <infile>\n", argv[0]);
            return EXIT_FAILURE;
        }
        char *outfile = argv[optind];
        char *infile = argv[optind + 1];
        stage_2(outfile, infile);
        break;
    }
    case 3:
    {
        if (argc - optind != 2)
        {
            fprintf(stderr, "Usage: %s --stage-3 <outfile> <infile>\n", argv[0]);
            return EXIT_FAILURE;
        }
        char *outfile = argv[optind];
        char *infile = argv[optind + 1];
        stage_3(outfile, infile);
        break;
    }
    case 4:
    {
        if (argc - optind != 1)
        {
            fprintf(stderr, "Usage: %s --stage-4 <infile>\n", argv[0]);
            return EXIT_FAILURE;
        }
        char *infile = argv[optind];
        stage_4(infile);
        break;
    }
    case 0:
    {
        fprintf(stderr, "Usage: %s [--stage-1|--stage-2|--stage-3|--stage-4]\n", argv[0]);
        return EXIT_FAILURE;
    }
    }

    return EXIT_SUCCESS;
}
