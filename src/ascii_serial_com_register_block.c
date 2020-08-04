#include "ascii_serial_com_register_block.h"
#include "asc_exception.h"
#include <assert.h>

#if REGWIDTHBYTES == 1
#define put_val_into_bytes(x, b) convert_uint8_to_hex(x, (b), true)
#define get_val_from_bytes(b) convert_hex_to_uint8((b))
#elif REGWIDTHBYTES == 2
#define put_val_into_bytes(x, b) convert_uint16_to_hex(x, (b), true)
#define get_val_from_bytes(b) convert_hex_to_uint16((b))
#elif REGWIDTHBYTES == 4
#define put_val_into_bytes(x, b) convert_uint32_to_hex(x, (b), true)
#define get_val_from_bytes(b) convert_hex_to_uint32((b))
#endif

void ascii_serial_com_register_block_init(
    ascii_serial_com_register_block *register_block_state, REGTYPE *block,
    uint16_t n_regs) {
  register_block_state->block = block;
  register_block_state->n_regs = n_regs;
}

void ascii_serial_com_register_block_handle_message(
    ascii_serial_com *asc, char ascVersion, char appVersion, char command,
    char *data, size_t dataLen, void *register_block_state_vp) {
  if (!register_block_state_vp) {
    Throw(ASC_ERROR_REG_BLOCK_NULL);
  }
  const ascii_serial_com_register_block *register_block_state =
      (ascii_serial_com_register_block *)register_block_state_vp;
  if (command != 'r' && command != 'w') {
    Throw(ASC_ERROR_UNEXPECTED_COMMAND);
  }
  if (dataLen < 4) { // need room for reg num
    Throw(ASC_ERROR_DATA_TOO_SHORT);
  }
  uint16_t reg_num = convert_hex_to_uint16(data);
  if (reg_num >= register_block_state->n_regs) {
    Throw(ASC_ERROR_REGNUM_OOB);
  }
  if (command == 'r') {
    REGTYPE reg_val = register_block_state->block[reg_num];
    data[4] = ',';
    put_val_into_bytes(reg_val, data + 5);
    dataLen = 5 + REGWIDTHBYTES * 2;
  } else {
    if (dataLen < 5 + REGWIDTHBYTES) {
      Throw(ASC_ERROR_REGVAL_LEN);
    }
    REGTYPE new_reg_val = get_val_from_bytes(data + 5);
    register_block_state->block[reg_num] = new_reg_val;
    dataLen = 4;
  }
  ascii_serial_com_put_message_in_output_buffer(asc, ascVersion, appVersion,
                                                command, data, dataLen);
}
