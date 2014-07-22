define(`MUTEX_ACQUIRE',     `or ra39, ra51, rb39;           nop')
define(`MUTEX_RELEASE',     `or ra51, ra39, ra39;           nop')

# Hardwired IO registers
define(`rVpmWriteFifo', `rb48')
define(`rVpmReadFifo', `ra48')
define(`raReadUniform', `ra32')
define(`rbReadUniform', `rb32')
define(`raZero', `ra39')
define(`rbZero', `rb39')

# Macro argument constants
define(`MODEW_32_BIT', 0)
define(`MODEW_16_BIT_OFFSET_0', 2)
define(`MODEW_16_BIT_OFFSET_1', 3)
define(`MODEW_8_BIT_OFFSET_0', 4)
define(`MODEW_8_BIT_OFFSET_1', 5)
define(`MODEW_8_BIT_OFFSET_2', 6)
define(`MODEW_8_BIT_OFFSET_3', 7)
define(`SIZE_8_BIT', 0)
define(`SIZE_16_BIT', 1)
define(`SIZE_32_BIT', 2)
define(`IS_HORIZ', 1)
define(`NOT_HORIZ', 0)
define(`IS_VERT', 1)
define(`NOT_VERT', 0)
define(`IS_LANED', 1)
define(`NOT_LANED', 0)

# VPM_BLOCK_WRITE_SETUP
# ~~~~~~~~~~~~~~~~~~~~~
# Sets up things so writes go into the small VPM data cache.
# Once the data's been written (by outputting repeatedly to the VPM_WRITE_FIFO
# register rb48), you then call VPM_DMA_WRITE_SETUP to configure the main
# memory destination and writing pattern.
# Arguments:
#  STRIDE: 0-64 - How much to increment the ADDR after each write.
#  HORIZ: 0 or 1 - Whether the layout is horizontal (1) or vertical (0).
#  LANED: 0 or 1 - Whether the layout is laned (1) or packed (0).
#  SIZE: 0, 1, 2 - The data unit size, 8-bit (0), 16-bit(1), or 32-bit (2).
#  ADDR: 0-255 - Packed address, meaning depends on exact unit size and mode.
# See http://www.broadcom.com/docs/support/videocore/VideoCoreIV-AG100-R.pdf page 57
define(`VPM_BLOCK_WRITE_SETUP_ID_SHIFT', 30)
define(`VPM_BLOCK_WRITE_SETUP_STRIDE_SHIFT', 12)
define(`VPM_BLOCK_WRITE_SETUP_HORIZ_SHIFT', 11)
define(`VPM_BLOCK_WRITE_SETUP_LANED_SHIFT', 10)
define(`VPM_BLOCK_WRITE_SETUP_SIZE_SHIFT', 8)
define(`VPM_BLOCK_WRITE_SETUP_ADDR_SHIFT', 0)
define(`VPM_BLOCK_WRITE_SETUP_VALUE', `eval(
(0<<VPM_BLOCK_WRITE_SETUP_ID_SHIFT)|
($1<<VPM_BLOCK_WRITE_SETUP_STRIDE_SHIFT)|
($2<<VPM_BLOCK_WRITE_SETUP_HORIZ_SHIFT)|
($3<<VPM_BLOCK_WRITE_SETUP_LANED_SHIFT)|
($4<<VPM_BLOCK_WRITE_SETUP_SIZE_SHIFT)|
($5<<VPM_BLOCK_WRITE_SETUP_ADDR_SHIFT))')
define(`VPM_BLOCK_WRITE_SETUP', `ldi rb49, VPM_BLOCK_WRITE_SETUP_VALUE($1, $2, $3, $4, $5)')

# VPM_BLOCK_READ_SETUP
# ~~~~~~~~~~~~~~~~~~~~
# Controls how values are read from the VPM data cache into the QPU.
# Arguments:
#  NUM: 0-16 - How many elements to read at a time.
#  STRIDE: 0-64 - The amount to increment the address by after each read.
#  HORIZ: 0 or 1 - Whether the layour is horizontal (1) or vertical (0).
#  LANED: 0 or 1 - Whether the layout is laned (1) or packed (0).
#  SIZE: 0, 1, 2 - The data unit size, 8-bit (0), 16-bit(1), or 32-bit (2).
#  ADDR: 0-255 - Packed address, meaning depends on exact unit size and mode.
# See http://www.broadcom.com/docs/support/videocore/VideoCoreIV-AG100-R.pdf page 58
define(`VPM_BLOCK_READ_SETUP_ID_SHIFT', 30)
define(`VPM_BLOCK_READ_SETUP_NUM_SHIFT', 20)
define(`VPM_BLOCK_READ_SETUP_STRIDE_SHIFT', 12)
define(`VPM_BLOCK_READ_SETUP_HORIZ_SHIFT', 11)
define(`VPM_BLOCK_READ_SETUP_LANED_SHIFT', 10)
define(`VPM_BLOCK_READ_SETUP_SIZE_SHIFT', 8)
define(`VPM_BLOCK_READ_SETUP_ADDR_SHIFT', 0)
define(`VPM_BLOCK_READ_SETUP_VALUE', `eval(
(0<<VPM_BLOCK_READ_SETUP_ID_SHIFT)|
($1<<VPM_BLOCK_READ_SETUP_NUM_SHIFT)|
($2<<VPM_BLOCK_READ_SETUP_STRIDE_SHIFT)|
($3<<VPM_BLOCK_READ_SETUP_HORIZ_SHIFT)|
($4<<VPM_BLOCK_READ_SETUP_LANED_SHIFT)|
($5<<VPM_BLOCK_READ_SETUP_SIZE_SHIFT)|
($6<<VPM_BLOCK_READ_SETUP_ADDR_SHIFT))')
define(`VPM_BLOCK_READ_SETUP', `ldi ra49, VPM_BLOCK_READ_SETUP_VALUE($1, $2, $3, $4, $5, $6)')

# VPM_DMA_STORE_SETUP
# ~~~~~~~~~~~~~~~~~~~
# Configures the DMA controller to transfer data from the VPM cache to main memory.
# Once the setup's been done, you then need to call VPM_DMA_STORE_START to kick
# off the transfer.
# Arguments:
#  UNITS: 0-128 - Number of rows of 2D block in memory.
#  DEPTH: 0-128 - How long each row is (in bytes?).
#  HORIZ: 0 or 1 - Whether the layout is horizontal (1) or vertical (0).
#  ADDRY: The Y coordinate of the address in the VPM space to start from.
#  ADDRX: The X coordinate of the address in the VPM space to start from.
#  MODEW: 0-7 : 0 is 32-bit, 2-3 is 16-bit with offset, 4-7 is 8-bit with offset.
# See http://www.broadcom.com/docs/support/videocore/VideoCoreIV-AG100-R.pdf page 58
define(`VPM_DMA_STORE_SETUP_ID_SHIFT', 30)
define(`VPM_DMA_STORE_SETUP_UNITS_SHIFT', 23)
define(`VPM_DMA_STORE_SETUP_DEPTH_SHIFT', 16)
define(`VPM_DMA_STORE_SETUP_HORIZ_SHIFT', 14)
define(`VPM_DMA_STORE_SETUP_ADDRY_SHIFT', 7)
define(`VPM_DMA_STORE_SETUP_ADDRX_SHIFT', 3)
define(`VPM_DMA_STORE_SETUP_MODEW_SHIFT', 0)
define(`VPM_DMA_STORE_SETUP_VALUE', `eval(
(2<<VPM_DMA_STORE_SETUP_ID_SHIFT)|
($1<<VPM_DMA_STORE_SETUP_UNITS_SHIFT)|
($2<<VPM_DMA_STORE_SETUP_DEPTH_SHIFT)|
($3<<VPM_DMA_STORE_SETUP_HORIZ_SHIFT)|
($4<<VPM_DMA_STORE_SETUP_ADDRY_SHIFT)|
($5<<VPM_DMA_STORE_SETUP_ADDRX_SHIFT)|
($6<<VPM_DMA_STORE_SETUP_MODEW_SHIFT))')
define(`VPM_DMA_STORE_SETUP', `ldi rb49, VPM_DMA_STORE_SETUP_VALUE($1, $2, $3, $4, $5, $6)')

# VPM_DMA_STORE_START
# ~~~~~~~~~~~~~~~~~~~
# Kicks off the transfer of data from the local VPM data cache to main memory.
# It will use the settings from VPM_DMA_STORE_SETUP to control the copy process.
# Arguments:
#  address: A register name that holds the address in main memory to write to.
define(`VPM_DMA_STORE_START', `or rb50, $1, 0;          nop')

# VPM_DMA_STORE_WAIT_FOR_COMPLETION
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Pause until the previous DMA store operation has finished.
define(`VPM_DMA_STORE_WAIT_FOR_COMPLETION', `or rb39, rb50, rb50;       nop')

# VPM_DMA_LOAD_SETUP
# ~~~~~~~~~~~~~~~~~~
# Initializes the settings for transfering data from main memory into the VPM cache.
# Arguments:
#  MODEW: 0-7 : 0 is 32-bit, 2-3 is 16-bit with offset, 4-7 is 8-bit with offset.
#  MPITCH: 0-15: The amount to increment the memory pointer between rows, calculated as 8*2^MPITCH bytes.
#  ROWLEN: 0-15: The number of elements in each row in main memory.
#  NROWS: 0-15: How many rows to read from memory.
#  VPITCH: 0-15: How much to increment the VPM address by after each row is loaded.
#  VERT: 0 or 1 - Whether the layout is vertical (1) or horizontal (0). Be careful, this is inverted compared to normal.
#  ADDRY: 0-64 - The Y coordinate of the address in the VPM space to start loading into.
#  ADDRX: 0-16 - The X coordinate of the address in the VPM space to start loading into.
define(`VPM_DMA_LOAD_SETUP_ID_SHIFT', 31)
define(`VPM_DMA_LOAD_SETUP_MODEW_SHIFT', 28)
define(`VPM_DMA_LOAD_SETUP_MPITCH_SHIFT', 24)
define(`VPM_DMA_LOAD_SETUP_ROWLEN_SHIFT', 20)
define(`VPM_DMA_LOAD_SETUP_NROWS_SHIFT', 16)
define(`VPM_DMA_LOAD_SETUP_VPITCH_SHIFT', 12)
define(`VPM_DMA_LOAD_SETUP_VERT_SHIFT', 11)
define(`VPM_DMA_LOAD_SETUP_ADDRY_SHIFT', 4)
define(`VPM_DMA_LOAD_SETUP_ADDRX_SHIFT', 0)
define(`VPM_DMA_LOAD_SETUP_VALUE', `eval(
(1<<VPM_DMA_LOAD_SETUP_ID_SHIFT)|
($1<<VPM_DMA_LOAD_SETUP_MODEW_SHIFT)|
($2<<VPM_DMA_LOAD_SETUP_MPITCH_SHIFT)|
($3<<VPM_DMA_LOAD_SETUP_ROWLEN_SHIFT)|
($4<<VPM_DMA_LOAD_SETUP_NROWS_SHIFT)|
($5<<VPM_DMA_LOAD_SETUP_VPITCH_SHIFT)|
($6<<VPM_DMA_LOAD_SETUP_VERT_SHIFT)|
($7<<VPM_DMA_LOAD_SETUP_ADDRY_SHIFT)|
($8<<VPM_DMA_LOAD_SETUP_ADDRX_SHIFT))')
define(`VPM_DMA_LOAD_SETUP', `ldi ra49, VPM_DMA_LOAD_SETUP_VALUE($1, $2, $3, $4, $5, $6, $7, $8)')

# VPM_DMA_LOAD_START
# ~~~~~~~~~~~~~~~~~~~
# Kicks off the transfer of data from main memory to the local VPM data cache.
# It will use the settings from VPM_DMA_LOAD_SETUP to control the copy process.
# Arguments:
#  address: A register name that holds the address in main memory to read from.
define(`VPM_DMA_LOAD_START', `or ra50, $1, 0;          nop')

# VPM_DMA_LOAD_WAIT_FOR_COMPLETION
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Pause until the previous DMA load operation has finished.
define(`VPM_DMA_LOAD_WAIT_FOR_COMPLETION', `or rb39, ra50, ra50;       nop')

# END_PROGRAM
# ~~~~~~~~~~~
# Triggers a host interrupt to transfer control back to the main CPU.
define(`END_PROGRAM_HARD', `
or rb38, r0, 1;       nop
nop.tend ra39, ra39, ra39;       nop rb39, rb39, rb39
nop ra39, ra39, ra39;       nop rb39, rb39, rb39
nop ra39, ra39, ra39;       nop rb39, rb39, rb39')

define(`END_PROGRAM_SOFT', `
nop.tend ra39, ra39, ra39;      nop rb39, rb39, rb39
NOP
NOP
')

# NOP
# ~~~
# Do nothing on both pipes for a cycle
define(`NOP', `nop ra39, ra39, ra39;       nop rb39, rb39, rb39')
