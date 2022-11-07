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

/* ELF Header (64-bit implementations) */
typedef struct {
  unsigned char a_sh_name[4];
  unsigned char a_sh_type[4];
  unsigned char a_sh_flags[8];
  unsigned char a_sh_addr[8];
  unsigned char a_sh_offset[8];
  unsigned char a_sh_size[8];
  unsigned char a_sh_link[4];
  unsigned char a_sh_info[4];
  unsigned char a_sh_addralign[8];
  unsigned char a_sh_entsize[8];
} Elf64_Auxiliary_Shdr;

#endif
