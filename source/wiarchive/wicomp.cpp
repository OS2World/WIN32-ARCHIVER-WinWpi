
/***************************************************************************
 *
 *  Compression routines for the class WIArchive
 *
 *  This file Copyright (C) 1998-2000 Jens B„ckman, Ulrich M”ller.
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
#include "wiarchive.h"

#pragma hdrstop

/*
 *@@ Compress:
 *      writes one file into the archive, using Lempel-Ziv and Huffman
 *      compression to reduce the size of the file.
 *
 *@@changed V0.9.19 (2002-07-01) [umoeller]: now returning error code
 */

int WIArchiveRW::Compress(WIFileHeader *head)   // in: header of file to write
{
    int rc = 0;     // V0.9.19 (2002-07-01) [umoeller]

    char *text = 0, *outbuf = 0;

    // First of all, do some initialization
    head->method = 1;
    head->crc = INIT_CRC;
    if (!(text = (char*)malloc(BUFFERSIZE)))
        rc = WIERR_NOT_ENOUGH_MEMORY;
    else if (!(outbuf = (char*)malloc(BUFFERSIZE)))
        rc = WIERR_NOT_ENOUGH_MEMORY;
    else
    {
        bz_stream z;
        int status, count;
        unsigned int i;

        z.bzalloc = 0;
        z.bzfree  = 0;
        z.opaque = NULL;
        z.bzalloc = NULL;
        z.bzfree = NULL;
        BZ2_bzCompressInit (&z, 9, 0, 30);
        // Now, begin the compression!
        z.avail_in = 0;
        z.next_out = outbuf;
        z.avail_out = BUFFERSIZE;
        head->compsize = 0;
        for (;;)
        {
            if (_pfnCallback)
                _pfnCallback(CBM_PERCENTAGE,
                             CalcPercentage(_tell(_hfFile),
                                            head->origsize),
                             head,
                             _pvCallbackArg);
            if (z.avail_in == 0)
            {
                z.next_in = text;
                z.avail_in = read(_hfFile, text, BUFFERSIZE);
            }

            if (z.avail_in <= 0)
                 break;
            for (i = 0; i < z.avail_in; i++)
                UPDATE_CRC (text[i]);
            status = BZ2_bzCompress (&z, BZ_RUN);
            count = BUFFERSIZE - z.avail_out;
            head->compsize += count;
            if (count)
                if (write(_hfArchive, outbuf, count)
                     < count)
                {
                    rc = WIERR_IO_WRITE;
                    break;
                }

            z.next_out = outbuf;
            z.avail_out = BUFFERSIZE;
        }

        if (!rc)
        {
            // flush pending data and wrap things up
            if (head->origsize != 0)
            {
                do
                {
                    status = BZ2_bzCompress (&z, BZ_FINISH);
                    count = BUFFERSIZE - z.avail_out;
                    head->compsize += count;
                    if (count)
                        if (write(_hfArchive, outbuf, count)
                            < count)
                        {
                            rc = WIERR_IO_WRITE;
                            break;
                        }

                    z.next_out = outbuf;
                    z.avail_out = BUFFERSIZE;
                } while (status != BZ_STREAM_END);
            }
        }

        BZ2_bzCompressEnd (&z);
    }

    if (outbuf)
        free(outbuf);
    if (text)
        free(text);

    return rc;
}

/*
 *@@ Store:
 *      writes one file into the archive, but without any compression,
 *      just calculating the CRC and storing it 1:1.
 *
 *      This is used if the compressed version of the file turns out
 *      to be bigger than the original.
 *
 *@@changed V0.9.19 (2002-07-01) [umoeller]: now returning error code
 */

int WIArchiveRW::Store(WIFileHeader *head) // in: header of file to write
{
    int rc = 0;     // V0.9.19 (2002-07-01) [umoeller]

    unsigned short bytes, i;
    unsigned long crc;

    head->compsize = head->origsize;
    head->method = 0;
    crc = INIT_CRC;

    // ----- Store the file without compression
    char *text;
    if (!(text = (char*)malloc(BUFFERSIZE)))
        rc = WIERR_NOT_ENOUGH_MEMORY;
    else
    {
        bytes = read(_hfFile, text, BUFFERSIZE);
        while (bytes > 0)
        {
            if (_pfnCallback)
                _pfnCallback(CBM_PERCENTAGE,
                             CalcPercentage(_tell(_hfFile),
                                            head->origsize),
                             head,
                             _pvCallbackArg);
            for (i = 0; i < bytes; i++)
                UPDATE_CRC (text[i]);
            if (write(_hfArchive, text, bytes)
                < bytes)
            {
                rc = WIERR_IO_WRITE;
                break;
            }

            bytes = read(_hfFile, text, BUFFERSIZE);
        }
        free(text);
    }

    crc ^= INIT_CRC;
    head->crc = crc;

    return rc;
}










