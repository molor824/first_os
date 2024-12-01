#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

typedef struct
{
    uint8_t boot_jump_instruction[3];
    uint8_t oem_identifier[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_count;
    uint16_t dir_entries_count;
    uint16_t total_sectors;
    uint8_t media_descriptor_type;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t heads;
    uint32_t hidden_sectors;
    uint32_t large_sector_count;
    uint8_t drive_number;
    uint8_t _reserved;
    uint8_t signature;
    uint32_t volume_id;
    uint8_t volume_label[11];
    uint8_t system_id[8];

} __attribute__((packed)) BootSector;

typedef struct
{
    uint8_t name[11];
    uint8_t attributes;
    uint8_t _reserved;
    uint8_t created_time_tenths;
    uint16_t created_time;
    uint16_t created_date;
    uint16_t accessed_date;
    uint16_t first_cluster_high;
    uint16_t modified_time;
    uint16_t modified_date;
    uint16_t first_cluster_low;
    uint32_t size;
} __attribute__((packed)) DirEntry;

BootSector boot_sector;
DirEntry *root_directory = NULL;
uint32_t root_directory_end;
uint8_t *fat = NULL;

bool read_boot_sector(FILE *disk)
{
    return fread(&boot_sector, sizeof(BootSector), 1, disk) != 0;
}
bool read_sectors(FILE *disk, uint32_t lba, uint32_t count, void *buffer)
{
    bool ok = true;
    ok = ok && (fseek(disk, lba * boot_sector.bytes_per_sector, SEEK_SET) == 0);
    ok = ok && (fread(buffer, boot_sector.bytes_per_sector, count, disk) == count);
    return ok;
}
bool read_fat(FILE *disk)
{
    fat = (uint8_t *)malloc(boot_sector.sectors_per_fat * boot_sector.bytes_per_sector);
    return read_sectors(disk, boot_sector.reserved_sectors, boot_sector.sectors_per_fat, fat);
}
bool read_root_directory(FILE *disk)
{
    uint32_t lba = boot_sector.reserved_sectors + boot_sector.sectors_per_fat * boot_sector.fat_count;
    uint32_t size = sizeof(DirEntry) * boot_sector.dir_entries_count;
    uint32_t sectors = size / boot_sector.bytes_per_sector;
    if (size % boot_sector.bytes_per_sector > 0)
        sectors++;

    root_directory_end = lba + sectors;
    root_directory = (DirEntry *)malloc(sectors * boot_sector.bytes_per_sector);
    return read_sectors(disk, lba, sectors, root_directory);
}
DirEntry *find_file(const char *name)
{
    for (uint32_t i = 0; i < boot_sector.dir_entries_count; i++)
    {
        if (memcmp(name, root_directory[i].name, 11) == 0)
        {
            return &root_directory[i];
        }
    }

    return NULL;
}
bool read_file(DirEntry *entry, FILE *disk, uint8_t *buffer)
{
    uint16_t currentCluster = entry->first_cluster_low;

    while (1)
    {
        uint32_t lba = root_directory_end + (currentCluster - 2) * boot_sector.sectors_per_cluster;
        if (!read_sectors(disk, lba, boot_sector.sectors_per_cluster, buffer))
            return false;

        buffer += boot_sector.sectors_per_cluster * boot_sector.bytes_per_sector;

        uint32_t fat_index = currentCluster * 3 / 2;
        if (currentCluster % 2 == 0)
            currentCluster = (*(uint16_t *)(fat + fat_index)) & 0x0FFF;
        else
            currentCluster = (*(uint16_t *)(fat + fat_index)) >> 4;

        if (currentCluster >= 0xFF8)
            return true;
    }

    return false;
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("Syntax: %s <disk image> <file name> \n", argv[0]);
        return -1;
    }

    FILE *disk = fopen(argv[1], "rb");
    if (!disk)
    {
        fprintf(stderr, "Cannot open disk image!\n");
        return -1;
    }

    if (!read_boot_sector(disk))
    {
        fprintf(stderr, "Cannot read boot sector!\n");
        return -1;
    }
    if (!read_fat(disk))
    {
        fprintf(stderr, "Cannot read FAT!\n");
        return -1;
    }
    if (!read_root_directory(disk))
    {
        fprintf(stderr, "Cannot read root directory!\n");
        return -1;
    }

    DirEntry *file = find_file(argv[2]);
    if (!file)
    {
        fprintf(stderr, "File not found!\n");
        return -1;
    }

    uint8_t *buffer = (uint8_t *)malloc(file->size);
    if (!read_file(file, disk, buffer))
    {
        fprintf(stderr, "Cannot read file!\n");
        return -1;
    }

    for (size_t i = 0; i < file->size; i++)
    {
        if (isprint(buffer[i]))
            printf("%c", buffer[i]);
        else
            printf("[%02x]", buffer[i]);
    }

    return 0;
}