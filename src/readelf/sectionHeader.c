#include <stdio.h>

#include "elf.h"

#include "sectionHeader.h"

extern Elf64_Off sectionHeadersAddress;
extern Elf64_Half sectionNumber;
extern Elf64_Half sectionSize;

static void printfElf64Header(Elf64_Info_Shdr *elfInfo) {
  fprintf(stderr, "\n\n");

  fprintf(stderr, "Section Headers Info:\n");
  fprintf(stderr, "  [Nr] Name              Type            Address          "
                  "Off    Size   EntrySize Flags Link Info Alignment\n");
  for (int i = 0; i < sectionNumber; i++) {
    fprintf(stderr,
            "  %d %d              %d            %x          "
            "%x    %d   %d %d %d %d %d\n",
            i, elfInfo[i].i_sh_name, elfInfo[i].i_sh_type, elfInfo[i].i_sh_addr,
            elfInfo[i].i_sh_offset, elfInfo[i].i_sh_size,
            elfInfo[i].i_sh_entsize, elfInfo[i].i_sh_flags,
            elfInfo[i].i_sh_link, elfInfo[i].i_sh_info,
            elfInfo[i].i_sh_addralign);
  }
  fprintf(stderr, "\n\n");
}

static void shstrtab(int index, const char *sHdrStrTab, Elf64_Shdr *elfShdr) {
  elfShdr->sh_addr =
      byte_get_little_endian(elfShdr->sh_addr, sizeof(elfShdr->sh_addr));
  sHdrStrTab;
}

static void sh_name(int index, Elf64_Info_Shdr *sHdrInfo, Elf64_Shdr *elfShdr) {
  elfShdr->sh_name =
      byte_get_little_endian(elfShdr->sh_name, sizeof(elfShdr->sh_name));
  sHdrInfo->i_sh_name = elfShdr->sh_name;
}

static void sh_type(int index, Elf64_Info_Shdr *sHdrInfo, Elf64_Shdr *elfShdr) {
  elfShdr->sh_type =
      byte_get_little_endian(elfShdr->sh_type, sizeof(elfShdr->sh_type));
  sHdrInfo->i_sh_type = elfShdr->sh_type;
}

static void sh_flags(int index, Elf64_Info_Shdr *sHdrInfo,
                     Elf64_Shdr *elfShdr) {
  elfShdr->sh_flags =
      byte_get_little_endian(elfShdr->sh_flags, sizeof(elfShdr->sh_flags));
  sHdrInfo->i_sh_flags = elfShdr->sh_flags;
}

static void sh_addr(int index, Elf64_Info_Shdr *sHdrInfo, Elf64_Shdr *elfShdr) {
  elfShdr->sh_addr =
      byte_get_little_endian(elfShdr->sh_addr, sizeof(elfShdr->sh_addr));
  sHdrInfo->i_sh_addr = elfShdr->sh_addr;
}

static void sh_offset(int index, Elf64_Info_Shdr *sHdrInfo,
                      Elf64_Shdr *elfShdr) {
  elfShdr->sh_offset =
      byte_get_little_endian(elfShdr->sh_offset, sizeof(elfShdr->sh_offset));
  sHdrInfo->i_sh_offset = elfShdr->sh_offset;
}

static void sh_size(int index, Elf64_Info_Shdr *sHdrInfo, Elf64_Shdr *elfShdr) {
  elfShdr->sh_size =
      byte_get_little_endian(elfShdr->sh_size, sizeof(elfShdr->sh_size));
  sHdrInfo->i_sh_size = elfShdr->sh_size;
}

static void sh_link(int index, Elf64_Info_Shdr *sHdrInfo, Elf64_Shdr *elfShdr) {
  elfShdr->sh_link =
      byte_get_little_endian(elfShdr->sh_link, sizeof(elfShdr->sh_link));
  sHdrInfo->i_sh_link = elfShdr->sh_link;
}

static void sh_info(int index, Elf64_Info_Shdr *sHdrInfo, Elf64_Shdr *elfShdr) {
  elfShdr->sh_info =
      byte_get_little_endian(elfShdr->sh_info, sizeof(elfShdr->sh_info));
  sHdrInfo->i_sh_info = elfShdr->sh_info;
}

static void sh_addralign(int index, Elf64_Info_Shdr *sHdrInfo,
                         Elf64_Shdr *elfShdr) {
  elfShdr->sh_addralign = byte_get_little_endian(elfShdr->sh_addralign,
                                                 sizeof(elfShdr->sh_addralign));
  sHdrInfo->i_sh_addralign = elfShdr->sh_addralign;
}

static void sh_entsize(int index, Elf64_Info_Shdr *sHdrInfo,
                       Elf64_Shdr *elfShdr) {
  elfShdr->sh_entsize =
      byte_get_little_endian(elfShdr->sh_entsize, sizeof(elfShdr->sh_entsize));
}

int processSectionHeader(const char *inputFileName) {

  FILE *fileHandle = fopen(inputFileName, "rb");
  closeFile(fileHandle);

  Elf64_Shdr sectionHdr[sectionNumber];
  Elf64_Info_Shdr infoElf64Shdr[sectionNumber];

  shstrtab(sectionNumber - 1, sHdrStrTab, &sectionHdr[sectionNumber - 1]);

  for (int i = 0; i < sectionNumber; i++) {
    sh_name(i, &infoElf64Shdr[i], &sectionHdr[i]);
    sh_type(i, &infoElf64Shdr[i], &sectionHdr[i]);
    sh_flags(i, &infoElf64Shdr[i], &sectionHdr[i]);
    sh_addr(i, &infoElf64Shdr[i], &sectionHdr[i]);
    sh_offset(i, &infoElf64Shdr[i], &sectionHdr[i]);
    sh_size(i, &infoElf64Shdr[i], &sectionHdr[i]);
    sh_link(i, &infoElf64Shdr[i], &sectionHdr[i]);
    sh_info(i, &infoElf64Shdr[i], &sectionHdr[i]);
    sh_addralign(i, &infoElf64Shdr[i], &sectionHdr[i]);
    sh_entsize(i, &infoElf64Shdr[i], &sectionHdr[i]);
  }

  printfElf64Header(infoElf64Shdr);

  return 0;
}
