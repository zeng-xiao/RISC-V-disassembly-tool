#ifndef SECTION_HEADER_H
#define SECTION_HEADER_H
#include "elf.h"

int processSectionHeader(const char *inputFileName);

typedef struct {
  const char *i_sh_name;
  const char *i_sh_type;
  const char *i_sh_addr;
  const char *i_sh_flags;
  Elf64_Off i_sh_offset;
  Elf32_Word i_sh_size;
  Elf32_Word i_sh_link;
  Elf32_Word i_sh_info;
  Elf32_Word i_sh_addralign;
  Elf64_Xword i_sh_entsize;
} Elf64_Info_Shdr;

const char *sHdrStrTab[10000];
#endif
