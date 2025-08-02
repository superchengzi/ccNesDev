#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>


/*
 从 nes rom 文件中，拿到程序代码
 
 这里有很多 nes 的测试 rom
 我们现在只想测试是否能拿到程序代码，所以选择了一个 cpu 相关的 rom
 https://github.com/christopherpow/nes-test-roms/blob/master/cpu_dummy_reads/cpu_dummy_reads.nes
 */

typedef struct {
    char magic[4];      // "NES" + 0x1A

    uint8_t prg_size;   // PRG ROM大小 (16KB单位)
    uint8_t chr_size;   // CHR ROM大小 (8KB单位)
    uint8_t flags6;     // 镜像/电池/Mapper低4位
    uint8_t flags7;     // NES2.0/Mapper高4位
    uint8_t prg_ram;    // PRG RAM大小 (8KB单位)
    uint8_t tv_system;  // 0=NTSC, 1=PAL
    uint8_t reserved[6];// 保留字节
} nes_cartridge_header;

int main(int argc, char* argv[]) {
    
    FILE* rom_file = fopen("cpu_dummy_reads.nes", "rb");
    if (!rom_file) {
        perror("Error opening file");
        return 1;
    }
    
    // 1. 读取文件头
    nes_cartridge_header header;
    if (fread(&header, sizeof(nes_cartridge_header), 1, rom_file) != 1) {
        perror("Header read error");
        fclose(rom_file);
        return 1;
    }
    
    // 2. 验证文件标识符
    if (header.magic[0] != 'N' ||
        header.magic[1] != 'E' ||
        header.magic[2] != 'S' ||
        header.magic[3] != 0x1A) {
        printf("Invalid iNES header!\n");
        fclose(rom_file);
        return 1;
    }
    
    // 3. 解析基础信息
    uint8_t mapper = (header.flags7 & 0xF0) | (header.flags6 >> 4);
    uint16_t prg_kb = header.prg_size * 16;
    uint16_t chr_kb = header.chr_size * 8;
    uint8_t has_battery = (header.flags6 >> 1) & 1;
    uint8_t has_trainer = (header.flags6 >> 2) & 1;
    
    printf("PRG ROM: %d KB\n", prg_kb);
    printf("CHR ROM: %d KB\n", chr_kb);
    printf("Mapper: %d\n", mapper);
    printf("Battery: %s\n", has_battery ? "Yes" : "No");
    printf("Trainer: %s\n", has_trainer ? "Yes" : "No");
    
    // 4. 跳过训练器区域（如果存在）
    if (has_trainer) {
        printf("Skipping 512-byte trainer...\n");
        fseek(rom_file, 512, SEEK_CUR);
    }
    
    // 5. 读取PRG ROM数据
    size_t prg_size = prg_kb * 1024;
    uint8_t* prg_rom = malloc(prg_size);
    if (!prg_rom) {
        perror("Memory allocation error");
        fclose(rom_file);
        return 1;
    }
    
    if (fread(prg_rom, 1, prg_size, rom_file) != prg_size) {
        perror("PRG ROM read error");
        free(prg_rom);
        fclose(rom_file);
        return 1;
    }
    
    printf("Loaded %zu bytes of PRG ROM\n", prg_size);
    
    // 6. 读取CHR ROM数据
    size_t chr_size = chr_kb * 1024;
    uint8_t* chr_rom = NULL;
    
    if (chr_size > 0) {
        chr_rom = malloc(chr_size);
        if (!chr_rom) {
            perror("CHR ROM memory error");
            free(prg_rom);
            fclose(rom_file);
            return 1;
        }
        
        if (fread(chr_rom, 1, chr_size, rom_file) != chr_size) {
            perror("CHR ROM read error");
            free(prg_rom);
            free(chr_rom);
            fclose(rom_file);
            return 1;
        }
        printf("Loaded %zu bytes of CHR ROM\n", chr_size);
    } else {
        printf("No CHR ROM - using CHR RAM\n");
        // 模拟器需分配8KB CHR RAM空间
    }
    
    // 7. 清理资源
    free(prg_rom);
    if (chr_rom) free(chr_rom);
    fclose(rom_file);
    
    return 0;
}

