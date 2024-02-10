#define _LARGEFILE_SOURCE
#define _LARGEFILE64_SOURCE
#define _FILE_OFFSET_BITS 64
#define LARGEFILES
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#ifndef _MAX_PATH
#define _MAX_PATH 1000
#endif
#ifndef _MAX_PATH
#define _MAX_PATH 206
#endif
#ifndef fopen64
#define fopen64 fopen
#define _fseeki64 fseek
#endif
#ifdef off64_t
#define off_t off64_t
#endif
typedef struct  {
    off_t begin;
    off_t end;
} num_set_tuple;
typedef struct {
    char *cmd;
    char *src;
} list_data;
void rangeset(char * src, int *len, num_set_tuple *data) {
    int times = 0;
    char *src_set[200];
    int num_set[_MAX_PATH];
    char src_copied[_MAX_PATH];
    strcpy(src_copied, src);
    char *src_set_tmp = strtok(src_copied, ",");
    while (src_set_tmp != NULL) {
        src_set[times] = malloc(strlen(src_set_tmp) + 1); // 分配足够的内存空间
        strcpy(src_set[times], src_set_tmp); // 复制子字符串到新分配的内存中
        times++;
        src_set_tmp = strtok(NULL, ",");
    }
    int num_set_len = times;
    for (int times2 = 0;times2 < times; times2++) {
        num_set[times2] = atoi(src_set[times2]);
    }
    * len = num_set_len / 2;
    if (num_set_len != num_set[0]+1) {
        fprintf(stderr, "Error on parsing following data to rangeset(%d:%d):\n%s\n", num_set_len, num_set[0]+1, src);
        exit(1);
    }
    times = 0;
    for (int i = 1; i < num_set_len; i += 2) {
        data[times].begin = num_set[i];
        data[times].end = num_set[i+1];
        times++;
    }
}
void parse_transfer_list_file(const char *TRANSFER_LIST_FILE, list_data * parse_data, int *new_blocks) {
    FILE *trans_list = fopen(TRANSFER_LIST_FILE, "rt");
    if (trans_list == NULL) {
        printf("Failed To open TRANSFER_LIST_FILE");
        exit(1);
    }
    char *lines[8096];
    int offset=2;
    int l =0;

    lines[0] = malloc(1000);
    for(;fgets(lines[l], 1000, trans_list)!=NULL ;l++) {
        lines[l+1] = malloc(1000);
    }
    fclose(trans_list);
    int version = atoi(lines[0]);
    *new_blocks = atoi(lines[1]);
    switch (version) {
        case 1:
            printf("Android Lollipop 5.0 detected!\n");
            break;
        case 2:
            printf("Android Lollipop 5.1 detected!\n");
        break;
        case 3:
            printf("Android Marshmallow 6.x detected!\n");
        break;
        case 4:
            printf("Android Nougat 7.x / Oreo 8.x detected!\n");
        break;
        default:
            printf("Unknown Android version!\n");
        break;
    }
    if (version >= 2)
        offset+=2;
    for (int i = 0; i < l; i++) {
        parse_data[i].cmd = strtok(lines[i+offset], " ");
        parse_data[i].src = strtok(NULL, " ");
    }
}
void sdat2img(const char * TRANSFER_LIST_FILE, const char * NEW_DATA_FILE, char * OUTPUT_IMAGE_FILE) {
    printf("sdat2img binary - version: 1.3\n");
    list_data parse_data[8196];
    int new_blocks;
    parse_transfer_list_file(TRANSFER_LIST_FILE, (list_data *)&parse_data, &new_blocks);
    FILE * output_img = fopen64(OUTPUT_IMAGE_FILE, "wb");
    FILE *new_data_file = fopen64(NEW_DATA_FILE, "rb");
    if (new_data_file == NULL) {
        printf("Error: Cannot Creat NEW_DATA Images\n");
        exit(1);
    }
    for (int i=0; parse_data[i].cmd != '\0';i++) {
        if (strcmp(parse_data[i].cmd, "new") == 0) {
            int len = 0;
            num_set_tuple data[_MAX_PATH];
            rangeset(parse_data[i].src, &len, (num_set_tuple *)&data);
            for (int i_ =0; i_ < len; i_++) {
                int block_count = data[i_].end - data[i_].begin;
                printf("Copying %d blocks into position %ld...\n", block_count, data[i_].begin);
                _fseeki64(output_img, data[i_].begin * BLOCK_SIZE, SEEK_SET);
                while(block_count > 0) {
                    char* file_data[4096];
                    fread(file_data, BLOCK_SIZE, 1, new_data_file);
                    fwrite(file_data, 4096,1 , output_img);
                    block_count-=1;
                }
            }
        } else {
            printf("Skipping command |%s|...\n", parse_data[i].cmd);
        }

    }
    printf("Done! Output image: %s\n" ,OUTPUT_IMAGE_FILE);
    fclose(output_img);
    fclose(new_data_file);
}
int main(const int argc, char *argv[]) {
    if (argc == 4) {
        sdat2img(argv[1], argv[2], argv[3]);
    } else {
        printf("sdat2img <transfer_list> <system_new_file> [system_img]\n");
        printf("Write By ColdWindScholar(3590361911@qq.com)\n");
    }
}