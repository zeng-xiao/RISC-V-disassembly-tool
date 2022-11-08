#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "elf.h"

#include "dataOperate.h"
#include "sectionHeader.h"

extern Elf64_Off shdrAddress;
extern Elf64_Half shdrNumber;
extern Elf64_Half shdrSize;
extern Elf64_Half shdrStrtabIndex;

static void printfElf64Header(Elf64_Info_Shdr *i_shdr) {
  fprintf(stderr, "\n\n");

  fprintf(stderr, "Section Headers Info:\n");
  fprintf(stderr,
          "  [Nr]    Name                     SectionType       Address   "
          "       SectionOffsetInFile SectionSize "
          "EntrySize Flags "
          "LinkToSection Info "
          "Alignment\n");
  for (int index = 0; index < shdrNumber; index++) {
    fprintf(
        stderr,
        "  [%02d]    %-19s      %-16s  %016lx "
        "%06lx              %06lx      %03lx       %s    %02d            %02d  "
        "     %lx\n",
        index, i_shdr[index].i_sh_name, i_shdr[index].i_sh_type,
        i_shdr[index].i_sh_addr, i_shdr[index].i_sh_offset,
        i_shdr[index].i_sh_size, i_shdr[index].i_sh_entsize,
        i_shdr[index].i_sh_flags, i_shdr[index].i_sh_link,
        i_shdr[index].i_sh_info, i_shdr[index].i_sh_addralign);
  }
  fprintf(stderr, "\n\n");
}

static void sh_name(int index, Elf64_Shdr *shdr, Elf64_Auxiliary_Shdr *a_shdr,
                    Elf64_Info_Shdr *i_shdr) {
  shdr[index].sh_name = byte_get_little_endian(a_shdr[index].a_sh_name,
                                               sizeof(shdr[index].sh_name));
}

static void sh_name_str(int index, Elf64_Shdr *shdr, Elf64_Info_Shdr *i_shdr,
                        FILE *fileHandle) {
  char *strBuffer = (char *)malloc(1024);
  uint64_t strOffset;

  strOffset = shdr[shdrStrtabIndex].sh_offset + shdr[index].sh_name;
  if (!fseek(fileHandle, strOffset, SEEK_SET))
    fscanf(fileHandle, "%s", strBuffer);
  i_shdr[index].i_sh_name = strBuffer;
}

static void sh_type(int index, Elf64_Shdr *shdr, Elf64_Auxiliary_Shdr *a_shdr,
                    Elf64_Info_Shdr *i_shdr) {
  shdr[index].sh_type = byte_get_little_endian(a_shdr[index].a_sh_type,
                                               sizeof(shdr[index].sh_type));

  switch (shdr[index].sh_type) {
  case SHT_NULL:
    i_shdr[index].i_sh_type = "NULL";
    break;
  case SHT_DYNSYM:
    i_shdr[index].i_sh_type = "DYNSYM";
    break;

  case SHT_STRTAB:
    i_shdr[index].i_sh_type = "STRTAB";
    break;

  case SHT_SYMTAB_SHNDX:
    i_shdr[index].i_sh_type = "SYMTAB_SHNDX";
    break;

  case SHT_SYMTAB:
    i_shdr[index].i_sh_type = "SYMTAB";
    break;

  case SHT_GROUP:
    i_shdr[index].i_sh_type = "GROUP";
    break;

  case SHT_REL:
    i_shdr[index].i_sh_type = "REL";
    break;

  case SHT_RELA:
    i_shdr[index].i_sh_type = "RELA";
    break;

  case SHT_RELR:
    i_shdr[index].i_sh_type = "RELR";
    break;

    /* Having a zero sized section is not illegal according to the ELF
     * standard, but it might be an indication that something is wrong.  So
     * issue a warning if we are running in lint mode.  */
  case SHT_NOTE:
    i_shdr[index].i_sh_type = "NOTE";
    break;
  case SHT_NOBITS:
    i_shdr[index].i_sh_type = "NOBITS";
    break;
  case SHT_PROGBITS:
    i_shdr[index].i_sh_type = "PROGBITS";
    break;
  case SHT_RISCV_ATTRIBUTES:
    i_shdr[index].i_sh_type = "RISCV_ATTRIBUTES";
  default:
    break;
  }

  if (!i_shdr[index].i_sh_type)
    i_shdr[index].i_sh_type = "unknown";
}

static void sh_flags(int index, Elf64_Shdr *shdr, Elf64_Auxiliary_Shdr *a_shdr,
                     Elf64_Info_Shdr *i_shdr) {
  static char flagBuffer[1024];
  char *p = flagBuffer;
  shdr[index].sh_flags = byte_get_little_endian(a_shdr[index].a_sh_flags,
                                                sizeof(shdr[index].sh_flags));

  while (shdr[index].sh_flags) {
    uint64_t flag;

    flag = shdr[index].sh_flags & -shdr[index].sh_flags;
    shdr[index].sh_flags &= ~flag;
    switch (flag) {
    case SHF_WRITE:
      *p = 'W';
      break;
    case SHF_ALLOC:
      *p = 'A';
      break;
    case SHF_EXECINSTR:
      *p = 'X';
      break;
    case SHF_MERGE:
      *p = 'M';
      break;
    case SHF_STRINGS:
      *p = 'S';
      break;
    case SHF_INFO_LINK:
      *p = 'I';
      break;
    case SHF_LINK_ORDER:
      *p = 'L';
      break;
    case SHF_OS_NONCONFORMING:
      *p = 'O';
      break;
    case SHF_GROUP:
      *p = 'G';
      break;
    case SHF_TLS:
      *p = 'T';
      break;
    case SHF_EXCLUDE:
      *p = 'E';
      break;
    case SHF_COMPRESSED:
      *p = 'C';
      break;
    default:
      break;
    }
    p++;
  }
  i_shdr[index].i_sh_flags = flagBuffer;
}

static void sh_addr(int index, Elf64_Shdr *shdr, Elf64_Auxiliary_Shdr *a_shdr,
                    Elf64_Info_Shdr *i_shdr) {
  shdr[index].sh_addr = byte_get_little_endian(a_shdr[index].a_sh_addr,
                                               sizeof(shdr[index].sh_addr));
  i_shdr[index].i_sh_addr = shdr[index].sh_addr;
}

static void sh_offset(int index, Elf64_Shdr *shdr, Elf64_Auxiliary_Shdr *a_shdr,
                      Elf64_Info_Shdr *i_shdr) {
  shdr[index].sh_offset = byte_get_little_endian(a_shdr[index].a_sh_offset,
                                                 sizeof(shdr[index].sh_offset));
  i_shdr[index].i_sh_offset = shdr[index].sh_offset;
}

static void sh_size(int index, Elf64_Shdr *shdr, Elf64_Auxiliary_Shdr *a_shdr,
                    Elf64_Info_Shdr *i_shdr) {
  shdr[index].sh_size = byte_get_little_endian(a_shdr[index].a_sh_size,
                                               sizeof(shdr[index].sh_size));
  i_shdr[index].i_sh_size = shdr[index].sh_size;
}

static void sh_link(int index, Elf64_Shdr *shdr, Elf64_Auxiliary_Shdr *a_shdr,
                    Elf64_Info_Shdr *i_shdr) {
  shdr[index].sh_link = byte_get_little_endian(a_shdr[index].a_sh_link,
                                               sizeof(shdr[index].sh_link));
  i_shdr[index].i_sh_link = shdr[index].sh_link;
}

static void sh_info(int index, Elf64_Shdr *shdr, Elf64_Auxiliary_Shdr *a_shdr,
                    Elf64_Info_Shdr *i_shdr) {
  shdr[index].sh_info = byte_get_little_endian(a_shdr[index].a_sh_info,
                                               sizeof(shdr[index].sh_info));
  i_shdr[index].i_sh_info = shdr[index].sh_info;
}

static void sh_addralign(int index, Elf64_Shdr *shdr,
                         Elf64_Auxiliary_Shdr *a_shdr,
                         Elf64_Info_Shdr *i_shdr) {
  shdr[index].sh_addralign = byte_get_little_endian(
      a_shdr[index].a_sh_addralign, sizeof(shdr[index].sh_addralign));
  i_shdr[index].i_sh_addralign = shdr[index].sh_addralign;
}

static void sh_entsize(int index, Elf64_Shdr *shdr,
                       Elf64_Auxiliary_Shdr *a_shdr, Elf64_Info_Shdr *i_shdr) {
  shdr[index].sh_entsize = byte_get_little_endian(
      a_shdr[index].a_sh_entsize, sizeof(shdr[index].sh_entsize));
  i_shdr[index].i_sh_entsize = shdr[index].sh_entsize;
}

int processSectionHeader(const char *inputFileName) {
  FILE *fileHandle = fopen(inputFileName, "rb");

  Elf64_Shdr shdr[shdrNumber];
  Elf64_Info_Shdr i_shdr[shdrNumber];
  Elf64_Auxiliary_Shdr a_shdr[shdrNumber];

  if (!fseek(fileHandle, shdrAddress, SEEK_SET))
    for (int index = 0; index < shdrNumber; index++) {
      fread(&a_shdr[index], shdrSize, 1, fileHandle);

      sh_name(index, shdr, a_shdr, i_shdr);
      sh_type(index, shdr, a_shdr, i_shdr);
      sh_flags(index, shdr, a_shdr, i_shdr);
      sh_addr(index, shdr, a_shdr, i_shdr);
      sh_offset(index, shdr, a_shdr, i_shdr);
      sh_size(index, shdr, a_shdr, i_shdr);
      sh_link(index, shdr, a_shdr, i_shdr);
      sh_info(index, shdr, a_shdr, i_shdr);
      sh_addralign(index, shdr, a_shdr, i_shdr);
      sh_entsize(index, shdr, a_shdr, i_shdr);
    }

  for (int index = 0; index < shdrNumber; index++)
    sh_name_str(index, shdr, i_shdr, fileHandle);

  closeFile(fileHandle);

  printfElf64Header(i_shdr);

  return 0;
}
