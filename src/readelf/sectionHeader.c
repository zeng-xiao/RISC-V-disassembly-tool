#include <stdio.h>

#include "elf.h"

#include "dataOperate.h"
#include "sectionHeader.h"

extern Elf64_Off sectionHeadersAddress;
extern Elf64_Half sectionNumber;
extern Elf64_Half sectionSize;
extern Elf64_Half sectionShstrtabIndex;

static void printfElf64Header(Elf64_Info_Shdr *elfInfo) {
  fprintf(stderr, "\n\n");

  fprintf(stderr, "Section Headers Info:\n");
  fprintf(stderr, "  [Nr] Name              Type            Address "
                  "Off    Size   EntrySize Flags Link Info Alignment\n");
  for (int i = 0; i < sectionNumber; i++) {
    fprintf(stderr,
            "  %d %s              %s            %ld          "
            "%ld    %d   %ld %s %d %d %d\n",
            i, elfInfo[i].i_sh_name, elfInfo[i].i_sh_type, elfInfo[i].i_sh_addr,
            elfInfo[i].i_sh_offset, elfInfo[i].i_sh_size,
            elfInfo[i].i_sh_entsize, elfInfo[i].i_sh_flags,
            elfInfo[i].i_sh_link, elfInfo[i].i_sh_info,
            elfInfo[i].i_sh_addralign);
  }
  fprintf(stderr, "\n\n");
}

static short unsigned int strShdrIndex = 20;

// static const char *sectionName(const Elf64_Shdr *strShdr,
//                                const Elf64_Shdr *hdr) {
//   return (void *)(strShdr->sh_offset + hdr->sh_name);
// }

static void sh_name(int index, Elf64_Info_Shdr *sHdrInfo, Elf64_Shdr *elfShdr,
                    Elf64_Auxiliary_Shdr *aShdr) {
  elfShdr->sh_name =
      byte_get_little_endian(aShdr->a_sh_name, sizeof(elfShdr->sh_name));
}

static void sh_name_str(int index, Elf64_Info_Shdr *sHdrInfo,
                        Elf64_Shdr *elfShdr, Elf64_Auxiliary_Shdr *aShdr) {
  if (!index)
    sHdrInfo->i_sh_name = "  ";
  else
    sHdrInfo->i_sh_name =
        (void *)(elfShdr[strShdrIndex].sh_offset + elfShdr[index].sh_name);

  fprintf(stderr, "[index = %d] sHdrInfo->i_sh_name = %s\n", index,
          sHdrInfo->i_sh_name);
}

static void sh_type(int index, Elf64_Info_Shdr *sHdrInfo, Elf64_Shdr *elfShdr,
                    Elf64_Auxiliary_Shdr *aShdr) {
  elfShdr->sh_type =
      byte_get_little_endian(aShdr->a_sh_type, sizeof(elfShdr->sh_type));

  switch (elfShdr->sh_type) {
  case SHT_NULL:
    sHdrInfo->i_sh_type = "NULL";
    break;
  case SHT_DYNSYM:
    sHdrInfo->i_sh_type = "DYNSYM";
    break;

  case SHT_STRTAB:
    sHdrInfo->i_sh_type = "STRTAB";
    break;

  case SHT_SYMTAB_SHNDX:
    sHdrInfo->i_sh_type = "SYMTAB_SHNDX";
    break;

  case SHT_SYMTAB:
    sHdrInfo->i_sh_type = "SYMTAB";
    break;

  case SHT_GROUP:
    sHdrInfo->i_sh_type = "GROUP";
    break;

  case SHT_REL:
    sHdrInfo->i_sh_type = "REL";
    break;

  case SHT_RELA:
    sHdrInfo->i_sh_type = "RELA";
    break;

  case SHT_RELR:
    sHdrInfo->i_sh_type = "RELR";
    break;

    /* Having a zero sized section is not illegal according to the ELF standard,
     * but it might be an indication that something is wrong.  So issue a
     * warning if we are running in lint mode.  */
  case SHT_NOTE:
    sHdrInfo->i_sh_type = "NOTE";
    break;
  case SHT_NOBITS:
    sHdrInfo->i_sh_type = "NOBITS";
    break;
  case SHT_PROGBITS:
    sHdrInfo->i_sh_type = "PROGBITS";
    break;
  case SHT_RISCV_ATTRIBUTES:
    sHdrInfo->i_sh_type = "RISCV_ATTRIBUTES";
  default:
    break;
  }

  if (!sHdrInfo->i_sh_type)
    sHdrInfo->i_sh_type = "unknown";
  fprintf(stderr, "[index = %d] sHdrInfo->i_sh_type = %s\n", index,
          sHdrInfo->i_sh_type);
}

static void sh_flags(int index, Elf64_Info_Shdr *sHdrInfo, Elf64_Shdr *elfShdr,
                     Elf64_Auxiliary_Shdr *aShdr) {
  static char buff[1024];
  char *p = buff;
  elfShdr->sh_flags =
      byte_get_little_endian(aShdr->a_sh_flags, sizeof(elfShdr->sh_flags));

  while (elfShdr->sh_flags) {
    uint64_t flag;

    flag = elfShdr->sh_flags & -elfShdr->sh_flags;
    elfShdr->sh_flags &= ~flag;
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

  fprintf(stderr, "[index = %d] sHdrInfo->i_sh_flags = %s\n", index, buff);
}

static void sh_addr(int index, Elf64_Info_Shdr *sHdrInfo, Elf64_Shdr *elfShdr,
                    Elf64_Auxiliary_Shdr *aShdr) {
  elfShdr->sh_addr =
      byte_get_little_endian(aShdr->a_sh_addr, sizeof(elfShdr->sh_addr));
  // sHdrInfo->i_sh_addr = elfShdr->sh_addr;
  fprintf(stderr, "[index = %d] sHdrInfo->i_sh_addr = 0x%lx\n", index,
          elfShdr->sh_addr);
}

static void sh_offset(int index, Elf64_Info_Shdr *sHdrInfo, Elf64_Shdr *elfShdr,
                      Elf64_Auxiliary_Shdr *aShdr) {
  elfShdr->sh_offset =
      byte_get_little_endian(aShdr->a_sh_offset, sizeof(elfShdr->sh_offset));
  // sHdrInfo->i_sh_offset = elfShdr->sh_offset;
  fprintf(stderr, "[index = %d] sHdrInfo->i_sh_offset = 0x%lx\n", index,
          elfShdr->sh_offset);
}

static void sh_size(int index, Elf64_Info_Shdr *sHdrInfo, Elf64_Shdr *elfShdr,
                    Elf64_Auxiliary_Shdr *aShdr) {
  elfShdr->sh_size =
      byte_get_little_endian(aShdr->a_sh_size, sizeof(elfShdr->sh_size));
  // sHdrInfo->i_sh_size = elfShdr->sh_size;
  fprintf(stderr, "[index = %d] sHdrInfo->i_sh_size = 0x%lx\n", index,
          elfShdr->sh_size);
}

static void sh_link(int index, Elf64_Info_Shdr *sHdrInfo, Elf64_Shdr *elfShdr,
                    Elf64_Auxiliary_Shdr *aShdr) {
  elfShdr->sh_link =
      byte_get_little_endian(aShdr->a_sh_link, sizeof(elfShdr->sh_link));
  // sHdrInfo->i_sh_link = elfShdr->sh_link;
  fprintf(stderr, "[index = %d] sHdrInfo->i_sh_link = %d\n", index,
          elfShdr->sh_link);
}

static void sh_info(int index, Elf64_Info_Shdr *sHdrInfo, Elf64_Shdr *elfShdr,
                    Elf64_Auxiliary_Shdr *aShdr) {
  elfShdr->sh_info =
      byte_get_little_endian(aShdr->a_sh_info, sizeof(elfShdr->sh_info));
  // sHdrInfo->i_sh_info = elfShdr->sh_info;
  fprintf(stderr, "[index = %d] sHdrInfo->i_sh_info = %d\n", index,
          elfShdr->sh_info);
}

static void sh_addralign(int index, Elf64_Info_Shdr *sHdrInfo,
                         Elf64_Shdr *elfShdr, Elf64_Auxiliary_Shdr *aShdr) {
  elfShdr->sh_addralign = byte_get_little_endian(aShdr->a_sh_addralign,
                                                 sizeof(elfShdr->sh_addralign));
  // sHdrInfo->i_sh_addralign = elfShdr->sh_addralign;
  fprintf(stderr, "[index = %d] sHdrInfo->i_sh_addralign = 0x%lx\n", index,
          elfShdr->sh_addralign);
}

static void sh_entsize(int index, Elf64_Info_Shdr *sHdrInfo,
                       Elf64_Shdr *elfShdr, Elf64_Auxiliary_Shdr *aShdr) {
  elfShdr->sh_entsize =
      byte_get_little_endian(aShdr->a_sh_entsize, sizeof(elfShdr->sh_entsize));
  fprintf(stderr, "[index = %d] sHdrInfo->i_sh_entsize = 0x%lx\n", index,
          elfShdr->sh_entsize);
}

int processSectionHeader(const char *inputFileName) {

  FILE *fileHandle = fopen(inputFileName, "rb");

  Elf64_Shdr sectionHdr[sectionNumber];
  Elf64_Info_Shdr infoElf64Shdr[sectionNumber];
  Elf64_Auxiliary_Shdr auxiliaryElf64Shdr[sectionNumber];

  if (!fseek(fileHandle, sectionHeadersAddress, SEEK_SET))
    for (int i = 0; i < sectionNumber; i++) {
      fprintf(stderr, "sectionSize = %d\nsectionNumber = %d\n", sectionSize,
              sectionNumber);
      fread(&auxiliaryElf64Shdr[i], sectionSize, 1, fileHandle);
      sh_name(i, &infoElf64Shdr[i], &sectionHdr[i], &auxiliaryElf64Shdr[i]);
      sh_type(i, &infoElf64Shdr[i], &sectionHdr[i], &auxiliaryElf64Shdr[i]);
      sh_flags(i, &infoElf64Shdr[i], &sectionHdr[i], &auxiliaryElf64Shdr[i]);
      sh_addr(i, &infoElf64Shdr[i], &sectionHdr[i], &auxiliaryElf64Shdr[i]);
      sh_offset(i, &infoElf64Shdr[i], &sectionHdr[i], &auxiliaryElf64Shdr[i]);
      sh_size(i, &infoElf64Shdr[i], &sectionHdr[i], &auxiliaryElf64Shdr[i]);
      sh_link(i, &infoElf64Shdr[i], &sectionHdr[i], &auxiliaryElf64Shdr[i]);
      sh_info(i, &infoElf64Shdr[i], &sectionHdr[i], &auxiliaryElf64Shdr[i]);
      sh_addralign(i, &infoElf64Shdr[i], &sectionHdr[i],
                   &auxiliaryElf64Shdr[i]);
      sh_entsize(i, &infoElf64Shdr[i], &sectionHdr[i], &auxiliaryElf64Shdr[i]);
    }
  for (int i = 0; i < sectionNumber; i++)
    sh_name_str(i, &infoElf64Shdr[i], &sectionHdr[i], &auxiliaryElf64Shdr[i]);

  closeFile(fileHandle);

  printfElf64Header(infoElf64Shdr);

  return 0;
}
