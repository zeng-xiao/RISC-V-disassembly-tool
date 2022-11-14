#ifndef ELF_HEADER_H
#define ELF_HEADER_H

#include <stdint.h>

int processElfHeader(const uint8_t *inputFileName);

// This is an auxiliary data structure created by converting bytes
// read from the outside into corresponding data sizes

/* ELF Header (32-bit implementations) */
typedef struct {
  uint8_t a_e_ident[16];    /* ELF "magic number" */
  uint8_t a_e_type[2];      /* Identifies object file type */
  uint8_t a_e_machine[2];   /* Specifies required architecture */
  uint8_t a_e_version[4];   /* Identifies object file version */
  uint8_t a_e_entry[4];     /* Entry point virtual address */
  uint8_t a_e_phoff[4];     /* Program header table file offset */
  uint8_t a_e_shoff[4];     /* Section header table file offset */
  uint8_t a_e_flags[4];     /* Processor-specific flags */
  uint8_t a_e_ehsize[2];    /* ELF header size in bytes */
  uint8_t a_e_phentsize[2]; /* Program header table entry size */
  uint8_t a_e_phnum[2];     /* Program header table entry count */
  uint8_t a_e_shentsize[2]; /* Section header table entry size */
  uint8_t a_e_shnum[2];     /* Section header table entry count */
  uint8_t a_e_shstrndx[2];  /* Section header string table index */
} Elf32_Auxiliary_Ehdr;

/* ELF Header (64-bit implementations) */
typedef struct {
  uint8_t a_e_ident[16];    /* ELF "magic number" */
  uint8_t a_e_type[2];      /* Identifies object file type */
  uint8_t a_e_machine[2];   /* Specifies required architecture */
  uint8_t a_e_version[4];   /* Identifies object file version */
  uint8_t a_e_entry[8];     /* Entry point virtual address */
  uint8_t a_e_phoff[8];     /* Program header table file offset */
  uint8_t a_e_shoff[8];     /* Section header table file offset */
  uint8_t a_e_flags[4];     /* Processor-specific flags */
  uint8_t a_e_ehsize[2];    /* ELF header size in bytes */
  uint8_t a_e_phentsize[2]; /* Program header table entry size */
  uint8_t a_e_phnum[2];     /* Program header table entry count */
  uint8_t a_e_shentsize[2]; /* Section header table entry size */
  uint8_t a_e_shnum[2];     /* Section header table entry count */
  uint8_t a_e_shstrndx[2];  /* Section header string table index */
} Elf64_Auxiliary_Ehdr;

#define EI_NIDENT (16)

typedef struct {
  uint8_t i_ident[EI_NIDENT]; /* Magic number and other info */

  const uint8_t *i_class;
  const uint8_t *i_dataEncodingForm;
  uint32_t i_fileVersion; /* File version */
  const uint8_t *i_osAbi;
  uint32_t i_abiVersion; /* ABI version */

  const uint8_t *i_type;    /* Object file type */
  const uint8_t *i_machine; /* Architecture */
  uint32_t i_objectVersion; /* Object file version */
  uint64_t i_entry;         /* Entry point virtual address */
  uint64_t i_phoff;         /* Program header table file offset */
  uint64_t i_shoff;         /* Section header table file offset */

  uint32_t i_flags;               /* Processor-specific flags */
  const uint8_t *i_flagStatement; /* Architecture */

  uint16_t i_ehsize;    /* ELF header size in bytes */
  uint16_t i_phentsize; /* Program header table entry size */
  uint16_t i_phnum;     /* Program header table entry count */
  uint16_t i_shentsize; /* Section header table entry size */
  uint16_t i_shnum;     /* Section header table entry count */
  uint16_t i_shstrndx;  /* Section header string table index */
} Elf64_Info_Ehdr;

#endif
