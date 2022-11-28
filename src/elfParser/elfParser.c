#include "elfHeader.h"
#include "programHeader.h"
#include "sectionHeader.h"

#include "GCC.command.line.h"
#include "comment.h"
#include "debug_str.h"
#include "riscv_attributes.h"

#include "debug_frame.h"

#include "dumpTextSection.h"

void static dump_str(const uint8_t *inputFileName) {
  dump_debug_str_section(inputFileName);
  dump_comment_section(inputFileName);
  dump_riscv_attributes_section(inputFileName);
  dump_GCC_command_line_section(inputFileName);
}

int main(int argc, char **argv) {
  const char *inputFileName = argv[1];

  process_elf_header(inputFileName);
  process_section_header(inputFileName);
  process_program_header(inputFileName);

  dump_str(inputFileName);

  parser_debug_frame(inputFileName);

  disassemble_text_section(inputFileName);

  return 0;
}
