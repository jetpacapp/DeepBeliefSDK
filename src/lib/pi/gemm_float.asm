include(`helpers.asm')

define(`VECTORS_PER_PASS', 8)
define(`ELEMENTS_PER_PASS', `eval(VECTORS_PER_PASS * 16)')
define(`ELEMENTS_PER_PASS_MINUS_ONE', `eval(ELEMENTS_PER_PASS - 1)')
define(`BYTES_PER_PASS', `eval(ELEMENTS_PER_PASS * 4)')
define(`ELEMENTS_PER_FINISH_PASS', 16)
define(`ELEMENTS_PER_FINISH_PASS_MINUS_ONE', `eval(ELEMENTS_PER_FINISH_PASS - 1)')
define(`BYTES_PER_FINISH_PASS', `eval(ELEMENTS_PER_FINISH_PASS * 4)')
define(`NUM_QPUS', 8)
define(`ALL_DONE_SEMA', 0)

# Register allocations
define(`rM', ra0)
define(`rN', ra1)
define(`rK', ra2)
define(`rAlpha', ra3)
define(`rAAddress', ra4)
define(`rAMin', ra5)
define(`rAMax', ra6)
define(`rLDA', ra7)
define(`rBAddress', ra8)
define(`rLDB', ra9)
define(`rBeta', ra10)
define(`rCAddress', ra11)
define(`rLDC', ra12)
define(`rDebugAddress', ra24)
define(`rWhichQPU', ra26)

define(`rI', ra13)
define(`rJ', ra14)
define(`rL', ra15)
define(`rCurrentA', ra16)
define(`rCurrentB', ra17)
define(`rCurrentC', ra20)
define(`rElementsToRead', ra23)
define(`rDebugOutput', ra25)
define(`rDMALoadAddrY', ra27)
define(`rVPMReadAddr', ra28)
define(`rVPMWriteAddr', ra29)
define(`rDMAStoreAddrY', ra30)
define(`rA0to15', rb0)
define(`rA16to31', rb1)
define(`rA32to47', rb2)
define(`rA48to63', rb3)
define(`rA64to79', rb4)
define(`rA80to95', rb5)
define(`rA96to111', rb6)
define(`rA112to127', rb7)

define(`rAccum0', r0)
define(`rAccum1', r1)
define(`rVectorsThisPass', r2)
define(`rTotal', r3)

# Load arguments
or rM, raReadUniform, 0; nop
or rN, raReadUniform, 0; nop
or rK, raReadUniform, 0; nop
or rAlpha, raReadUniform, 0; nop
or rAAddress, raReadUniform, 0; nop
or rAMin, raReadUniform, 0; nop
or rAMax, raReadUniform, 0; nop
or rLDA, raReadUniform, 0; nop
or rBAddress, raReadUniform, 0; nop
or rLDB, raReadUniform, 0; nop
or rBeta, raReadUniform, 0; nop
or rCAddress, raReadUniform, 0; nop
or rLDC, raReadUniform, 0; nop
or rDebugAddress, raReadUniform, 0; nop
or rWhichQPU, raReadUniform, 0; nop

ldi rDebugOutput, 0x3f000000

nop rb39, r0, r0; mul24 rTotal, rWhichQPU, VECTORS_PER_PASS
ldi rAccum0, VPM_DMA_LOAD_SETUP_ADDRY_SHIFT
shl rDMALoadAddrY, rTotal, rAccum0; nop
ldi rAccum0, VPM_BLOCK_READ_SETUP_ADDR_SHIFT
shl rVPMReadAddr, rTotal, rAccum0; nop
ldi rAccum0, VPM_BLOCK_WRITE_SETUP_ADDR_SHIFT
shl rVPMWriteAddr, rTotal, rAccum0; nop
ldi rAccum0, VPM_DMA_STORE_SETUP_ADDRY_SHIFT
shl rDMAStoreAddrY, rTotal, rAccum0; nop

ldi rJ, 0
loop_j:
or rAccum0, rN, 0; nop
sub ra39, rJ, rAccum0; nop
brr.ne ra39, loop_j_break
NOP
NOP
NOP

or rI, rWhichQPU, 0; nop
loop_i:
or rAccum0, rM, 0; nop
sub ra39, rI, rAccum0; nop
brr.ne ra39, loop_i_break
NOP
NOP
NOP

shl rAccum0, rLDA, 2; nop
nop rb39, r0, r0; mul24 rAccum0, rI, rAccum0
add rCurrentA, rAAddress, rAccum0; nop

shl rAccum0, rLDB, 2; nop
nop rb39, r0, r0; mul24 rAccum0, rJ, rAccum0
add rCurrentB, rBAddress, rAccum0; nop

ldi rTotal, 0

ldi rL, 0
main_loop_l:
ldi rAccum0, ELEMENTS_PER_PASS_MINUS_ONE
sub rAccum0, rK, rAccum0; nop
sub ra39, rL, rAccum0; nop
brr.ne ra39, main_loop_l_break
NOP
NOP
NOP

MUTEX_ACQUIRE()
define(`MPITCH', 2)
define(`ROWLEN', 16)
define(`NROWS', VECTORS_PER_PASS)
define(`VPITCH', 1)
define(`ADDRY', 0)
define(`ADDRX', 0)
ldi rAccum0, VPM_DMA_LOAD_SETUP_VALUE(MODEW_32_BIT, MPITCH, ROWLEN, NROWS, VPITCH, NOT_VERT, ADDRY, ADDRX)
or ra49, rAccum0, rDMALoadAddrY; nop

VPM_DMA_LOAD_START(rCurrentA)
VPM_DMA_LOAD_WAIT_FOR_COMPLETION()
MUTEX_RELEASE()

define(`NUM', VECTORS_PER_PASS)
define(`STRIDE', 1)
define(`ADDR', 0)
ldi rAccum0, VPM_BLOCK_READ_SETUP_VALUE(NUM, STRIDE, IS_HORIZ, NOT_LANED, SIZE_32_BIT, ADDR)
or ra49, rAccum0, rVPMReadAddr; nop

# Read 128 A values from VPM
or rA0to15, rVpmReadFifo, 0;  nop
or rA16to31, rVpmReadFifo, 0;  nop
or rA32to47, rVpmReadFifo, 0;  nop
or rA48to63, rVpmReadFifo, 0;  nop
or rA64to79, rVpmReadFifo, 0;  nop
or rA80to95, rVpmReadFifo, 0;  nop
or rA96to111, rVpmReadFifo, 0;  nop
or rA112to127, rVpmReadFifo, 0;  nop

MUTEX_ACQUIRE()
define(`MPITCH', 2)
define(`ROWLEN', 16)
define(`NROWS', VECTORS_PER_PASS)
define(`VPITCH', 1)
define(`ADDRY', 0)
define(`ADDRX', 0)
ldi rAccum0, VPM_DMA_LOAD_SETUP_VALUE(MODEW_32_BIT, MPITCH, ROWLEN, NROWS, VPITCH, NOT_VERT, ADDRY, ADDRX)
or ra49, rAccum0, rDMALoadAddrY; nop

VPM_DMA_LOAD_START(rCurrentB)
VPM_DMA_LOAD_WAIT_FOR_COMPLETION()
MUTEX_RELEASE()

define(`NUM', VECTORS_PER_PASS)
define(`STRIDE', 1)
define(`ADDR', 0)
ldi rAccum0, VPM_BLOCK_READ_SETUP_VALUE(NUM, STRIDE, IS_HORIZ, NOT_LANED, SIZE_32_BIT, ADDR)
or ra49, rAccum0, rVPMReadAddr; nop

# Read 128 B values from VPM
or rAccum0, rVpmReadFifo, 0;  nop
nop rb39, r0, r0; fmul rAccum0, rA0to15, rAccum0
fadd rTotal, rTotal, rAccum0; nop

or rAccum0, rVpmReadFifo, 0;  nop
nop rb39, r0, r0; fmul rAccum0, rA16to31, rAccum0
fadd rTotal, rTotal, rAccum0; nop

or rAccum0, rVpmReadFifo, 0;  nop
nop rb39, r0, r0; fmul rAccum0, rA32to47, rAccum0
fadd rTotal, rTotal, rAccum0; nop

or rAccum0, rVpmReadFifo, 0;  nop
nop rb39, r0, r0; fmul rAccum0, rA48to63, rAccum0
fadd rTotal, rTotal, rAccum0; nop

or rAccum0, rVpmReadFifo, 0;  nop
nop rb39, r0, r0; fmul rAccum0, rA64to79, rAccum0
fadd rTotal, rTotal, rAccum0; nop

or rAccum0, rVpmReadFifo, 0;  nop
nop rb39, r0, r0; fmul rAccum0, rA80to95, rAccum0
fadd rTotal, rTotal, rAccum0; nop

or rAccum0, rVpmReadFifo, 0;  nop
nop rb39, r0, r0; fmul rAccum0, rA96to111, rAccum0
fadd rTotal, rTotal, rAccum0; nop

or rAccum0, rVpmReadFifo, 0;  nop
nop rb39, r0, r0; fmul rAccum0, rA112to127, rAccum0
fadd rTotal, rTotal, rAccum0; nop

ldi rAccum0, BYTES_PER_PASS
add rCurrentA, rCurrentA, rAccum0; nop
add rCurrentB, rCurrentB, rAccum0; nop

ldi rAccum0, ELEMENTS_PER_PASS
add rL, rL, rAccum0; nop
brr ra39, main_loop_l
NOP
NOP
NOP

main_loop_l_break:

or rAccum0, rK, 0; nop
sub rAccum0, rAccum0, rL; nop
asr rAccum0, rAccum0, 4; nop
brr.ne rAccum0, partial_loop_l_break
NOP
NOP
NOP

or rDebugOutput, rAccum0, 0; nop

or rVectorsThisPass, rK, 0; nop
sub rVectorsThisPass, rVectorsThisPass, rL; nop
asr rVectorsThisPass, rVectorsThisPass, 4; nop

MUTEX_ACQUIRE()
define(`MPITCH', 2)
define(`ROWLEN', 16)
define(`NROWS', 0) # Overriden below
define(`VPITCH', 1)
define(`ADDRY', 0)
define(`ADDRX', 0)
ldi rAccum0, VPM_DMA_LOAD_SETUP_VALUE(MODEW_32_BIT, MPITCH, ROWLEN, NROWS, VPITCH, NOT_VERT, ADDRY, ADDRX)
ldi rAccum1, VPM_DMA_LOAD_SETUP_NROWS_SHIFT
shl rAccum1, rVectorsThisPass, rAccum1; nop
or rAccum0, rAccum0, rAccum1; nop
or ra49, rAccum0, rDMALoadAddrY; nop

VPM_DMA_LOAD_START(rCurrentA)
VPM_DMA_LOAD_WAIT_FOR_COMPLETION()
MUTEX_RELEASE()

shl rAccum0, rVectorsThisPass, 6; nop
add rCurrentA, rCurrentA, rAccum0; nop

define(`NUM', 0) # Overriden below
define(`STRIDE', 1)
define(`ADDR', 0)
ldi rAccum0, VPM_BLOCK_READ_SETUP_VALUE(NUM, STRIDE, IS_HORIZ, NOT_LANED, SIZE_32_BIT, ADDR)
ldi rAccum1, VPM_BLOCK_READ_SETUP_NUM_SHIFT
shl rAccum1, rVectorsThisPass, rAccum1; nop
or rAccum0, rAccum0, rAccum1; nop
or ra49, rAccum0, rVPMReadAddr; nop

# Read 16 to 112 A values from VPM
or rA0to15, rVpmReadFifo, 0;  nop
sub rVectorsThisPass, rVectorsThisPass, 1; nop
brr.zs ra39, a_read_break;
NOP
NOP
NOP

or rA16to31, rVpmReadFifo, 0;  nop
sub rVectorsThisPass, rVectorsThisPass, 1; nop
brr.zs ra39, a_read_break;
NOP
NOP
NOP

or rA32to47, rVpmReadFifo, 0;  nop
sub rVectorsThisPass, rVectorsThisPass, 1; nop
brr.zs ra39, a_read_break;
NOP
NOP
NOP

or rA48to63, rVpmReadFifo, 0;  nop
sub rVectorsThisPass, rVectorsThisPass, 1; nop
brr.zs ra39, a_read_break;
NOP
NOP
NOP

or rA64to79, rVpmReadFifo, 0;  nop
sub rVectorsThisPass, rVectorsThisPass, 1; nop
brr.zs ra39, a_read_break;
NOP
NOP
NOP

or rA80to95, rVpmReadFifo, 0;  nop
sub rVectorsThisPass, rVectorsThisPass, 1; nop
brr.zs ra39, a_read_break;
NOP
NOP
NOP

or rA96to111, rVpmReadFifo, 0;  nop

a_read_break:

or rVectorsThisPass, rK, 0; nop
sub rVectorsThisPass, rVectorsThisPass, rL; nop
asr rVectorsThisPass, rVectorsThisPass, 4; nop

MUTEX_ACQUIRE()
define(`MPITCH', 2)
define(`ROWLEN', 16)
define(`NROWS', 0) # Overriden below
define(`VPITCH', 1)
define(`ADDRY', 0)
define(`ADDRX', 0)
ldi rAccum0, VPM_DMA_LOAD_SETUP_VALUE(MODEW_32_BIT, MPITCH, ROWLEN, NROWS, VPITCH, NOT_VERT, ADDRY, ADDRX)
ldi rAccum1, VPM_DMA_LOAD_SETUP_NROWS_SHIFT
shl rAccum1, rVectorsThisPass, rAccum1; nop
or rAccum0, rAccum0, rAccum1; nop
or ra49, rAccum0, rDMALoadAddrY; nop

VPM_DMA_LOAD_START(rCurrentB)
VPM_DMA_LOAD_WAIT_FOR_COMPLETION()
MUTEX_RELEASE()

shl rAccum0, rVectorsThisPass, 6; nop
add rCurrentB, rCurrentB, rAccum0; nop

define(`NUM', 0) # Overriden below
define(`STRIDE', 1)
define(`ADDR', 0)
ldi rAccum0, VPM_BLOCK_READ_SETUP_VALUE(NUM, STRIDE, IS_HORIZ, NOT_LANED, SIZE_32_BIT, ADDR)
ldi rAccum1, VPM_BLOCK_READ_SETUP_NUM_SHIFT
shl rAccum1, rVectorsThisPass, rAccum1; nop
or rAccum0, rAccum0, rAccum1; nop
or ra49, rAccum0, rVPMReadAddr; nop

# Read 16 to 112 B values from VPM
or rAccum0, rVpmReadFifo, 0;  nop
sub rVectorsThisPass, rVectorsThisPass, 1; nop
brr.zs ra39, b_read_break;
nop rb39, r0, r0; fmul rAccum0, rA0to15, rAccum0
fadd rTotal, rTotal, rAccum0; nop
NOP

or rAccum0, rVpmReadFifo, 0;  nop
sub rVectorsThisPass, rVectorsThisPass, 1; nop
brr.zs ra39, b_read_break;
nop rb39, r0, r0; fmul rAccum0, rA16to31, rAccum0
fadd rTotal, rTotal, rAccum0; nop
NOP

or rAccum0, rVpmReadFifo, 0;  nop
sub rVectorsThisPass, rVectorsThisPass, 1; nop
brr.zs ra39, b_read_break;
nop rb39, r0, r0; fmul rAccum0, rA32to47, rAccum0
fadd rTotal, rTotal, rAccum0; nop
NOP

or rAccum0, rVpmReadFifo, 0;  nop
sub rVectorsThisPass, rVectorsThisPass, 1; nop
brr.zs ra39, b_read_break;
nop rb39, r0, r0; fmul rAccum0, rA48to63, rAccum0
fadd rTotal, rTotal, rAccum0; nop
NOP

or rAccum0, rVpmReadFifo, 0;  nop
sub rVectorsThisPass, rVectorsThisPass, 1; nop
brr.zs ra39, b_read_break;
nop rb39, r0, r0; fmul rAccum0, rA64to79, rAccum0
fadd rTotal, rTotal, rAccum0; nop
NOP

or rAccum0, rVpmReadFifo, 0;  nop
sub rVectorsThisPass, rVectorsThisPass, 1; nop
brr.zs ra39, b_read_break;
nop rb39, r0, r0; fmul rAccum0, rA80to95, rAccum0
fadd rTotal, rTotal, rAccum0; nop
NOP

or rAccum0, rVpmReadFifo, 0;  nop
nop rb39, r0, r0; fmul rAccum0, rA96to111, rAccum0
fadd rTotal, rTotal, rAccum0; nop

b_read_break:

or rVectorsThisPass, rK, 0; nop
sub rVectorsThisPass, rVectorsThisPass, rL; nop
asr rVectorsThisPass, rVectorsThisPass, 4; nop
shl rAccum0, rVectorsThisPass, 4; nop
add rL, rL, rAccum0; nop

partial_loop_l_break:

finish_loop_l:

or rAccum0, rK, 0; nop
sub ra39, rL, rAccum0; nop
brr.ne ra39, finish_loop_l_break
NOP
NOP
NOP

or rAccum0, rK, 0; nop
sub rElementsToRead, rAccum0, rL; nop
ldi rAccum0, ELEMENTS_PER_FINISH_PASS
min rElementsToRead, rElementsToRead, rAccum0; nop
ldi rAccum0, VPM_DMA_LOAD_SETUP_ROWLEN_SHIFT
shl rElementsToRead, rElementsToRead, rAccum0; nop

define(`STRIDE', 1)
define(`ADDR', 0)
ldi rAccum0, VPM_BLOCK_WRITE_SETUP_VALUE(STRIDE, IS_HORIZ, NOT_LANED, SIZE_32_BIT, ADDR)
or rb49, rAccum0, rVPMWriteAddr; nop

# Zero out the scratch memory before loading possibly-partial vectors to it
and rVpmWriteFifo, r0, 0; nop

MUTEX_ACQUIRE()
define(`MPITCH', 2)
define(`ROWLEN', 0) # Overridden below
define(`NROWS', 1)
define(`VPITCH', 1)
define(`ADDRY', 0)
define(`ADDRX', 0)
ldi rAccum0, VPM_DMA_LOAD_SETUP_VALUE(MODEW_32_BIT, MPITCH, ROWLEN, NROWS, VPITCH, NOT_VERT, ADDRY, ADDRX)
or rAccum0, rAccum0, rDMALoadAddrY; nop
or ra49, rAccum0, rElementsToRead; nop

VPM_DMA_LOAD_START(rCurrentA)
VPM_DMA_LOAD_WAIT_FOR_COMPLETION()
MUTEX_RELEASE()

define(`NUM', 1)
define(`STRIDE', 1)
define(`ADDR', 0)
ldi rAccum0, VPM_BLOCK_READ_SETUP_VALUE(NUM, STRIDE, IS_HORIZ, NOT_LANED, SIZE_32_BIT, ADDR)
or ra49, rAccum0, rVPMReadAddr; nop

# Read 16 or fewer A values from VPM
or rA0to15, rVpmReadFifo, 0;  nop

define(`STRIDE', 1)
define(`ADDR', 0)
ldi rAccum0, VPM_BLOCK_WRITE_SETUP_VALUE(STRIDE, IS_HORIZ, NOT_LANED, SIZE_32_BIT, ADDR)
or rb49, rAccum0, rVPMWriteAddr; nop

# Zero out the scratch memory before loading possibly-partial vectors to it
and rVpmWriteFifo, r0, 0; nop

MUTEX_ACQUIRE()
define(`MPITCH', 2)
define(`ROWLEN', 0) # Overridden below
define(`NROWS', 1)
define(`VPITCH', 1)
define(`ADDRY', 0)
define(`ADDRX', 0)
ldi rAccum0, VPM_DMA_LOAD_SETUP_VALUE(MODEW_32_BIT, MPITCH, ROWLEN, NROWS, VPITCH, NOT_VERT, ADDRY, ADDRX)
or rAccum0, rAccum0, rDMALoadAddrY; nop
or ra49, rAccum0, rElementsToRead; nop

VPM_DMA_LOAD_START(rCurrentB)
VPM_DMA_LOAD_WAIT_FOR_COMPLETION()
MUTEX_RELEASE()

define(`NUM', 1)
define(`STRIDE', 1)
define(`ADDR', 0)
ldi rAccum0, VPM_BLOCK_READ_SETUP_VALUE(NUM, STRIDE, IS_HORIZ, NOT_LANED, SIZE_32_BIT, ADDR)
or ra49, rAccum0, rVPMReadAddr; nop

# Read 16 or fewer B values from VPM
or rAccum0, rVpmReadFifo, 0;  nop

# Multiply the two arrays together
nop rb39, r0, r0; fmul rAccum0, rA0to15, rAccum0

fadd rTotal, rTotal, rAccum0; nop

ldi rAccum0, BYTES_PER_FINISH_PASS
add rCurrentA, rCurrentA, rAccum0; nop
add rCurrentB, rCurrentB, rAccum0; nop

ldi rAccum0, ELEMENTS_PER_FINISH_PASS
add rL, rL, rAccum0; nop
brr ra39, finish_loop_l
NOP
NOP
NOP

finish_loop_l_break:

or r0, rTotal, 0; nop
or r3, rTotal, 0; nop
nop rb39, r0, <<1; v8max r0, r0, r0
fadd rTotal, rTotal, r0; nop
or r0, rTotal, 0; nop
or r3, rTotal, 0; nop
nop rb39, r0, <<2; v8max r0, r0, r0
fadd rTotal, rTotal, r0; nop
or r0, rTotal, 0; nop
or r3, rTotal, 0; nop
nop rb39, r0, <<4; v8max r0, r0, r0
fadd rTotal, rTotal, r0; nop
or r0, rTotal, 0; nop
or r3, rTotal, 0; nop
nop rb39, r0, <<8; v8max r0, r0, r0
fadd rTotal, rTotal, r0; nop

nop rb39, r0, r0; fmul rTotal, rTotal, rAlpha;

define(`STRIDE', 1)
define(`ADDR', 0)
ldi rAccum0, VPM_BLOCK_WRITE_SETUP_VALUE(STRIDE, IS_HORIZ, NOT_LANED, SIZE_32_BIT, ADDR)
or rb49, rAccum0, rVPMWriteAddr; nop

or rVpmWriteFifo, rTotal, 0; nop

shl rAccum0, rLDC, 2; nop
nop rb39, r0, r0; mul24 rAccum0, rJ, rAccum0
add rCurrentC, rCAddress, rAccum0; nop
shl rAccum0, rI, 2; nop
add rCurrentC, rCurrentC, rAccum0; nop

MUTEX_ACQUIRE()
define(`UNITS', 1)
define(`DEPTH', 1)
define(`ADDRY', 0)
define(`ADDRX', 0)
ldi rAccum0, VPM_DMA_STORE_SETUP_VALUE(UNITS, DEPTH, IS_HORIZ, ADDRY, ADDRX, MODEW_32_BIT)
or rb49, rAccum0, rDMAStoreAddrY; nop

VPM_DMA_STORE_START(rCurrentC)
VPM_DMA_STORE_WAIT_FOR_COMPLETION()
MUTEX_RELEASE()

add rI, rI, NUM_QPUS; nop
brr ra39, loop_i
NOP
NOP
NOP

loop_i_break:

add rJ, rJ, 1; nop
brr ra39, loop_j
NOP
NOP
NOP

loop_j_break:

MUTEX_ACQUIRE()

define(`STRIDE', 1)
define(`ADDR', 0)
ldi rAccum0, VPM_BLOCK_WRITE_SETUP_VALUE(STRIDE, IS_HORIZ, NOT_LANED, SIZE_32_BIT, ADDR)
or rb49, rAccum0, rVPMWriteAddr; nop

or rVpmWriteFifo, rDebugOutput, 0; nop

define(`UNITS', 1)
define(`DEPTH', 16)
define(`ADDRY', 0)
define(`ADDRX', 0)
ldi rAccum0, VPM_DMA_STORE_SETUP_VALUE(UNITS, DEPTH, IS_HORIZ, ADDRY, ADDRX, MODEW_32_BIT)
or rb49, rAccum0, rDMAStoreAddrY; nop

VPM_DMA_STORE_START(rDebugAddress)
VPM_DMA_STORE_WAIT_FOR_COMPLETION()
MUTEX_RELEASE()

sema up, ALL_DONE_SEMA

or rb39, rWhichQPU, 0; nop
brr.zc rb39, non_master_finish
NOP
NOP
NOP

# The number of 'down's must match the number of QPUs being run
sema down, ALL_DONE_SEMA
sema down, ALL_DONE_SEMA
sema down, ALL_DONE_SEMA
sema down, ALL_DONE_SEMA
sema down, ALL_DONE_SEMA
sema down, ALL_DONE_SEMA
sema down, ALL_DONE_SEMA
sema down, ALL_DONE_SEMA

END_PROGRAM_HARD()

non_master_finish:

END_PROGRAM_SOFT()