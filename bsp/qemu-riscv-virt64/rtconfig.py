from http.client import PRECONDITION_FAILED
import os
from telnetlib import PRAGMA_HEARTBEAT

# toolchains options
ARCH        ='risc-v'
CPU         ='virt64'
CROSS_TOOL  ='gcc' # gcc, llvm(not work)

if os.getenv('RTT_ROOT'):
    RTT_ROOT = os.getenv('RTT_ROOT')
else:
    RTT_ROOT = os.path.join(os.getcwd(), '..', '..')

if os.getenv('RTT_CC'):
    CROSS_TOOL = os.getenv('RTT_CC')

if  CROSS_TOOL == 'gcc':
    PLATFORM    = 'gcc'
    EXEC_PATH   = r'/usr/bin'
elif CROSS_TOOL == 'llvm':
    PLATFORM    = 'llvm'
    EXEC_PATH   = r'/usr/bin'
else:
    print('Please make sure your toolchains is GNU GCC or llvm!')
    exit(0)

if os.getenv('RTT_EXEC_PATH'):
    EXEC_PATH = os.getenv('RTT_EXEC_PATH')

BUILD = ''

if PLATFORM == 'gcc':
    # toolchains
    PREFIX  = 'riscv64-unknown-linux-gnu-'
    CC      = PREFIX + 'gcc'
    CXX     = PREFIX + 'g++'
    AS      = PREFIX + 'gcc'
    AR      = PREFIX + 'ar'
    LINK    = PREFIX + 'ld'
    TARGET_EXT = 'elf'
    SIZE    = PREFIX + 'size'
    OBJDUMP = PREFIX + 'objdump'
    OBJCPY  = PREFIX + 'objcopy'

    DEVICE  = ' -fno-pic -mcmodel=medany -march=rv64imfd -mabi=lp64d'
    CFLAGS  = DEVICE + ' -ffreestanding -fno-common -ffunction-sections -fdata-sections -fstrict-volatile-bitfields '
    AFLAGS  = ' -c' + DEVICE + ' -x assembler-with-cpp'
    LFLAGS  = ' --gc-sections -Map=rtthread.map -cref -u _start -T link.lds '
    CPATH   = ''
    LPATH   = ''

    if BUILD == 'debug':
        CFLAGS += ' -O0 -ggdb -fvar-tracking'
        AFLAGS += ' -ggdb'
    else:
        CFLAGS += ' -O2 -Os -U_FORTIFY_SOURCE'

    CXXFLAGS = CFLAGS

elif PLATFORM == 'llvm':
    # toolchains
    CROSS_PATH        = '/nfs/home/zhangzifei/rvv/cgk/rvv/build/bin/'
    PREFIX            = CROSS_PATH + 'llvm-'
    CC                = CROSS_PATH + 'clang-12'
    CXX               = CC
    AS                = CROSS_PATH + 'as'
    AR                = CROSS_PATH + 'ar'
    LINK              = CROSS_PATH + 'link'
    TARGET_EXT        = 'elf'
    SIZE              = CROSS_PATH + 'size'
    OBJCPY            = CROSS_PATH + 'objcopy'
    OBJDUMP           = CROSS_PATH + 'objdump'

    DEVICE            = ' -march=rv64gv0p9 -nostdlib -mcmodel=medany'
    CFLAGS            = ' -menable-experimental-extensions' + DEVICE 
    CXXFLAGS          = CFLAGS
    AFLAGS            = ' -c' + DEVICE + ' -x assembler-with-cpp'
    
    LFLAGS            = ' -u _start -T link.lds'

    CPATH             = ''
    LPATH             = ''


    CFLAGS           += ' -O2'


DUMP_ACTION = OBJDUMP + ' -D -S $TARGET > rtthread.asm\n'
POST_ACTION = OBJCPY + ' -O binary $TARGET rtthread.bin\n' + SIZE + ' $TARGET \n'
