#define CHIPS_IMPL
#include "m6502.h"
#include <stdint.h>
#include <stdio.h>

/*
 6502 的执行流程是固定前 7 个周期执行 reset sequence
 这 7 个周期做了这些事:
 1. 重置 cpu 内部寄存器的状态
 2. 跳转到用户程序入口
    从内存地址 0xfffc 0xfffd 读取两个字节，这两个字节拼成一个 16 位的地址
    跳转到这个地址，这个地址就是用户程序的入口。
    因为用户程序入口由 0xfffc 0xfffd 这两个字节决定，所以这两个字节也称之为 reset vector
 */

int main(void) {
    // 64 KB zero-initialized memory
    uint8_t mem[(1<<16)] = { };

    // put an LDA #$33 instruction at address 0
    // 因为内存数据都是 0，所以 reset vector 对应的值也是 0
    // 因此 address 0 就是用户程序的入口
    mem[0] = 0xA9;
    mem[1] = 0x33;

    // initialize a 6502 instance:
    m6502_t cpu;
    uint64_t pins = m6502_init(&cpu, &(m6502_desc_t){ });
    
    // run for 9 ticks (7 ticks reset sequence, plus 2 ticks for LDA #$33)
    for (int i = 0; i < 9; i++) {
        // m6502_tick 代表执行一个 cycle
        pins = m6502_tick(&cpu, pins);
        // 地址是 16 位的
        const uint16_t addr = M6502_GET_ADDR(pins);
        if (pins & M6502_RW) {
            printf("tick(%d) set cpu pins from memory address(%d) value(%d)\n", i, addr, mem[addr]);
            M6502_SET_DATA(pins, mem[addr]);
        }
        else {
            mem[addr] = M6502_GET_DATA(pins);
        }
    }
    // the A register should now be 0x33:
    printf("A: %02X\n", m6502_a(&cpu));

    return 0;
}

