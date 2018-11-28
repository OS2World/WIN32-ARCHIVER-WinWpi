
/***************************************************************************
 *
 *  Decompression routines for the class WIArchive
 *
 *  This file Copyright (C) 1998-2002 Jens B„ckman, Ulrich M”ller.
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, in version 2 as it comes in the COPYING
 *  file of this distribution.
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 ***************************************************************************/

#include "setup.h"              // added V0.9.2 (2000-03-15) [umoeller]

#include <io.h>
#include <string.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <sys\utime.h>
#include <stdio.h>
#include <limits.h>
#include <time.h>               // needed for WIFileHeader

#include "libbz2\bzlib.h"

#include "base\bs_base.h"
#include "base\bs_list.h"

#define WIARCHIVE_INTERNAL
#include "wiarchive\wiarchive.h"

#pragma hdrstop

/****************************************************************************
 * Decompress a previously compressed file
 *
 * Arguments - head: Pointer to file header
 *
 */

/*
 *@@ Expand:
 *      decompresses a file previously compressed with
 *      WIArchiveRW::Compress.
 *
 *      Error handling is as follows:
 *      --  if a file read error occurs with the Archive file,
 *          the callback is called;
 *      --  in all other error situations, you get a return value
 *          != 0.
 *
 *      Returns:
 *      --  0:everything OK
 *
 *      --  >= 1: output file handling error (e.g. disk full);
 *          this is the return value from ferror(). You should
 *          then check errno or (with VAC++) _doserrno to find
 *          out what happened.
 *
 *      --  < 1: decompression error from zlib, which are:
 *               Z_DATA_ERROR
 *               Z_STREAM_ERROR
 *               Z_MEM_ERROR
 *               Z_BUF_ERROR
 *          See inflate() in zlib.h for details. This is most
 *          definitely the result of a corrupt archive.
 */

short WIArchive::Expand(WIFileHeader *head)     // in: header of file to decompress
{
    bz_stream z;
    char *outbuf;
    int status;
    long readcount, l;
    unsigned long ul;

    // First of all, do some initialization
    head->crc = INIT_CRC;
    char *text = (char*) malloc(BUFFERSIZE);
    outbuf = (char*) malloc(BUFFERSIZE);
    z.bzalloc = 0;
    z.bzfree  = 0;
    z.opaque = NULL;
    z.bzalloc = NULL;
    z.bzfree = NULL;
    z.next_in = text;
    readcount = head->compsize;
    if (readcount < (long)BUFFERSIZE)
        l = readcount;
    else
        l = BUFFERSIZE;
    z.avail_in = read(_hfArchive, text, l);
    readcount -= l;

    // read error checking added
    if (z.avail_in < 0)
    {
#ifndef BUILDING_STUB_CODE
        if (_pfnCallback)
            _pfnCallback(CBM_ERR_READ,
                         CBREC_CANCEL,   // allow only "Cancel"
                         head,
                         _pvCallbackArg);
#endif
        return (1);
    }

    BZ2_bzDecompressInit(&z, 0, 0);

    // Let the decompression begin!
    z.next_out = outbuf;
    z.avail_out = BUFFERSIZE;
    for (;;)
    {
#ifndef BUILDING_STUB_CODE
        if (_pfnCallback)
            _pfnCallback(CBM_PERCENTAGE,
                         CalcPercentage(_tell(_hfFile),
                                        head->origsize),
                         head,
                         _pvCallbackArg);
#endif

        if (z.avail_in == 0)
             break;
        while (z.avail_in)
        {
            status = BZ2_bzDecompress(&z);

            if (status < 0)
                // < 0 means ZLIB error
                return (status);    // < 0

            for (ul = 0;
                 ul < BUFFERSIZE - z.avail_out;
                 ++ul)
                UPDATE_CRC (text[ul]);

            // write error checking added
            if (BUFFERSIZE - z.avail_out)
            {
                size_t BytesWritten = write(_hfFile,
                                            outbuf,
                                            BUFFERSIZE - z.avail_out);

                if (BytesWritten != (BUFFERSIZE - z.avail_out))
                {
                    // check if it's really an error and not maybe EOF
                    /* int ferr = ferror(_File);
                    if (ferr != 0)
                    {
                        // error occured:
                        // report error to caller
                        return (abs(ferr));        // > 0
                    } */
                    return (1);
                }
            }

            z.next_out = outbuf;
            z.avail_out = BUFFERSIZE;
        }
        if (status == BZ_STREAM_END)  break;
        z.next_in = text;
        if (readcount < (long)BUFFERSIZE)
            l = readcount;
        else
            l = BUFFERSIZE;
        z.avail_in = read(_hfArchive, text, l);
        readcount -= l;
    }

    // We're done.  Yee-hah...
    BZ2_bzDecompressEnd (&z);
    free(outbuf);
    free(text);
    return 0;
}

/*
 *@@ Extract:
 *      extracts a file previously stored with
 *      WIArchiveRW::Store.
 */

short WIArchive::Extract (WIFileHeader *head) // in: header of file to decompress
{
    unsigned long crc, count;
    unsigned long bytes, ul;

    count = head->compsize;
    crc = INIT_CRC;

    // ----- Extract the stored file
#ifndef BUILDING_STUB_CODE
    if (_pfnCallback)
        _pfnCallback(CBM_PERCENTAGE,
                     CalcPercentage(_tell(_hfFile),
                                       head->origsize),
                     head,
                     _pvCallbackArg);
#endif

    char *text = (char*)malloc(BUFFERSIZE);
    while (count)
    {
        if (count >= BUFFERSIZE)
            bytes = BUFFERSIZE;
        else
            bytes = count;

        // read error checking added
        size_t BytesRead = read(_hfArchive, text, bytes);
        if (BytesRead < 0)
        {
            // check if it's really an error and not maybe EOF
#ifndef BUILDING_STUB_CODE
            if (_pfnCallback)
                _pfnCallback(CBM_ERR_READ,
                             CBREC_CANCEL,   // allow only "Cancel"
                             head,
                             _pvCallbackArg);
#endif
            return 1;
        }

        for (ul = 0; ul < bytes; ++ul)
            UPDATE_CRC (text[ul]);

        // write error checking added
        size_t BytesWritten = write(_hfFile, text, bytes);
        if (BytesWritten != bytes)
        {
/*      do not call the callback here, return an error instead,
        because WIArchive::unpack calls the callback
#ifndef BUILDING_STUB_CODE
            if (_pfnCallback)
                _pfnCallback(CBM_ERR_WRITE,
                             CBREC_CANCEL,   // allow only "Cancel"
                             head,
                             _pvCallbackArg);
#endif
*/
            return 1;
        }

#ifndef BUILDING_STUB_CODE
        if (_pfnCallback)
            _pfnCallback(CBM_PERCENTAGE,
                         CalcPercentage(_tell(_hfFile),
                                        head->origsize),
                         head,
                         _pvCallbackArg);
#endif

        count -= bytes;
    }
    free(text);

    // ----- Check if the CRC matched
    crc ^= INIT_CRC;
    if (crc == head->crc)
        return 1;
    else
        return 0;
}

