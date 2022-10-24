static Filedata *
open_file (const char * pathname)
{
  struct stat  statbuf;
  Filedata *   filedata = NULL;

  if (stat (pathname, & statbuf) < 0
      || ! S_ISREG (statbuf.st_mode))
    goto fail;

  filedata = calloc (1, sizeof * filedata);
  if (filedata == NULL)
    goto fail;

  filedata->handle = fopen (pathname, "rb");
  if (filedata->handle == NULL)
    goto fail;

  filedata->file_size = (bfd_size_type) statbuf.st_size;
  filedata->file_name = pathname;

  if (! get_file_header (filedata))
    goto fail;

  if (filedata->file_header.e_shoff)
    {
      bfd_boolean res;

      /* Read the section headers again, this time for real.  */
      if (is_32bit_elf)
	res = get_32bit_section_headers (filedata, FALSE);
      else
	res = get_64bit_section_headers (filedata, FALSE);

      if (!res)
	goto fail;
    }

  return filedata;

 fail:
  if (filedata)
    {
      if (filedata->handle)
        fclose (filedata->handle);
      free (filedata);
    }
  return NULL;
}



static bfd_boolean
get_file_header (Filedata * filedata)
{
  /* Read in the identity array.  */
  if (fread (filedata->file_header.e_ident, EI_NIDENT, 1, filedata->handle) != 1)
    return FALSE;

  /* Determine how to read the rest of the header.  */
  switch (filedata->file_header.e_ident[EI_DATA])
    {
    default:
    case ELFDATANONE:
    case ELFDATA2LSB:
      byte_get = byte_get_little_endian;
      byte_put = byte_put_little_endian;
      break;
    case ELFDATA2MSB:
      byte_get = byte_get_big_endian;
      byte_put = byte_put_big_endian;
      break;
    }

  /* For now we only support 32 bit and 64 bit ELF files.  */
  is_32bit_elf = (filedata->file_header.e_ident[EI_CLASS] != ELFCLASS64);

  /* Read in the rest of the header.  */
  if (is_32bit_elf)
    {
      Elf32_External_Ehdr ehdr32;

      if (fread (ehdr32.e_type, sizeof (ehdr32) - EI_NIDENT, 1, filedata->handle) != 1)
	return FALSE;

      filedata->file_header.e_type      = BYTE_GET (ehdr32.e_type);
      filedata->file_header.e_machine   = BYTE_GET (ehdr32.e_machine);
      filedata->file_header.e_version   = BYTE_GET (ehdr32.e_version);
      filedata->file_header.e_entry     = BYTE_GET (ehdr32.e_entry);
      filedata->file_header.e_phoff     = BYTE_GET (ehdr32.e_phoff);
      filedata->file_header.e_shoff     = BYTE_GET (ehdr32.e_shoff);
      filedata->file_header.e_flags     = BYTE_GET (ehdr32.e_flags);
      filedata->file_header.e_ehsize    = BYTE_GET (ehdr32.e_ehsize);
      filedata->file_header.e_phentsize = BYTE_GET (ehdr32.e_phentsize);
      filedata->file_header.e_phnum     = BYTE_GET (ehdr32.e_phnum);
      filedata->file_header.e_shentsize = BYTE_GET (ehdr32.e_shentsize);
      filedata->file_header.e_shnum     = BYTE_GET (ehdr32.e_shnum);
      filedata->file_header.e_shstrndx  = BYTE_GET (ehdr32.e_shstrndx);
    }
  else
    {
      Elf64_External_Ehdr ehdr64;

      /* If we have been compiled with sizeof (bfd_vma) == 4, then
	 we will not be able to cope with the 64bit data found in
	 64 ELF files.  Detect this now and abort before we start
	 overwriting things.  */
      if (sizeof (bfd_vma) < 8)
	{
	  error (_("This instance of readelf has been built without support for a\n\
64 bit data type and so it cannot read 64 bit ELF files.\n"));
	  return FALSE;
	}

      if (fread (ehdr64.e_type, sizeof (ehdr64) - EI_NIDENT, 1, filedata->handle) != 1)
	return FALSE;

      filedata->file_header.e_type      = BYTE_GET (ehdr64.e_type);
      filedata->file_header.e_machine   = BYTE_GET (ehdr64.e_machine);
      filedata->file_header.e_version   = BYTE_GET (ehdr64.e_version);
      filedata->file_header.e_entry     = BYTE_GET (ehdr64.e_entry);
      filedata->file_header.e_phoff     = BYTE_GET (ehdr64.e_phoff);
      filedata->file_header.e_shoff     = BYTE_GET (ehdr64.e_shoff);
      filedata->file_header.e_flags     = BYTE_GET (ehdr64.e_flags);
      filedata->file_header.e_ehsize    = BYTE_GET (ehdr64.e_ehsize);
      filedata->file_header.e_phentsize = BYTE_GET (ehdr64.e_phentsize);
      filedata->file_header.e_phnum     = BYTE_GET (ehdr64.e_phnum);
      filedata->file_header.e_shentsize = BYTE_GET (ehdr64.e_shentsize);
      filedata->file_header.e_shnum     = BYTE_GET (ehdr64.e_shnum);
      filedata->file_header.e_shstrndx  = BYTE_GET (ehdr64.e_shstrndx);
    }

  if (filedata->file_header.e_shoff)
    {
      /* There may be some extensions in the first section header.  Don't
	 bomb if we can't read it.  */
      if (is_32bit_elf)
	get_32bit_section_headers (filedata, TRUE);
      else
	get_64bit_section_headers (filedata, TRUE);
    }

  return TRUE;
}
