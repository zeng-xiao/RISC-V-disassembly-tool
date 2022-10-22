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