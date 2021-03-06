#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

struct machine {
    unsigned char* mem;
    int pc;
};

unsigned int read_pointer(unsigned char* memory, int offset) {
    unsigned char* addr = &memory[offset];
    unsigned int ptr = (addr[0])
        | (addr[1] << 8)
        | (addr[2] << 16)
        | (addr[3] << 24);
    return ptr;
}

unsigned int machine_fetch_pointer(struct machine* vm) {
    unsigned int ptr = read_pointer(vm->mem, vm->pc);
    vm->pc += 4;
    return ptr;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Missing argument.\n");
        exit(1);
    }
    FILE* f = fopen(argv[1], "rb");
    fseek(f, 0, SEEK_END);
    long size = ftell(f);

    rewind(f);

    unsigned char* memory = malloc(size);
    size_t got = fread(memory, 1, size, f);
    assert(got == size);
    (void)got;
    fclose(f);

    assert(memory[0] == 'R');
    assert(memory[1] == 'W');

    int isa_revision = memory[2] - 'a';
    int pointer_size = 1 << (memory[3] - '0');

    assert(isa_revision == 1);
    (void)isa_revision;
    assert(pointer_size == 4);

    unsigned int eof = read_pointer(memory, 4);
    unsigned int eom = read_pointer(memory, 4 + pointer_size);

    memory = realloc(memory, eom);
    memset(memory + eof, 0, eom - eof);

    struct machine vm;
    vm.mem = memory;
    vm.pc = 4 + 2 * pointer_size;
    while (1) {
        char inst = vm.mem[vm.pc++];
        unsigned int jumpptr;
        unsigned int dstptr;
        unsigned int srcptr;
        switch (inst) {
            case 0: // Halt
                exit(0);

            case 1: // Output Byte
                srcptr = machine_fetch_pointer(&vm);
                int c = vm.mem[srcptr];
                putchar(c);
                fflush(stdout);
                continue;

            case 2: // Branch If Plus
                jumpptr = machine_fetch_pointer(&vm);
                srcptr = machine_fetch_pointer(&vm);
                if (vm.mem[srcptr] < 128) {
                    vm.pc = jumpptr;
                }
                continue;

            case 3: // Subtract
                dstptr = machine_fetch_pointer(&vm);
                srcptr = machine_fetch_pointer(&vm);
                vm.mem[dstptr] -= vm.mem[srcptr];
                continue;

            case 4: // Input byte
                dstptr = machine_fetch_pointer(&vm);
                vm.mem[dstptr] = getchar();
                continue;

            default:
                printf("unknown instruction %d\n", inst);
                exit(1);
        }
    }

    return 0;    
}