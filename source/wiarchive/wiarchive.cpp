
/*
 *@@sourcefile wiarchive.cpp:
 *      this has the WarpIN back-end class WIArchive.
 *
 *      Starting with V0.9.14, we have two classes:
 *
 *      --  WIArchive is the base class and handles
 *          decompression _only_.
 *
 *      --  WIArchiveRW is derived from that and can
 *          handle compression also. This is now in
 *          wiarchiverw.cpp.
 *
 *@@header "wiarchive\wiarchive.h"
 */

/*
 *
 *  This file Copyright (C) 1998-2001 Jens B„ckman, Ulrich M”ller.
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, in version 2 as it comes in the COPYING
 *  file of this distribution.
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 */

#include <io.h>
#include <string.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <sys\utime.h>

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>               // needed for WIFileHeader
#include <limits.h>
#include <time.h>               // needed for WIFileHeader
#include <errno.h>

// the following is required for VAC++
#ifdef __IBMCPP__
#include <direct.h>
#endif

#ifdef __DEBUG__
    #include <os2.h>            // otherwise pmpf won't work V0.9.14 (2001-07-24) [umoeller]
#endif

#include "setup.h"              // added V0.9.2 (2000-03-15) [umoeller]

#include "libbz2\bzlib.h"

#include "base\bs_base.h"
#include "base\bs_list.h"

#define WIARCHIVE_INTERNAL
#include "wiarchive\wiarchive.h"

#pragma hdrstop

DEFINE_CLASS(WIArchive, BSRoot);
DEFINE_CLASS(WIFileLI, BSRoot);
DEFINE_CLASS(WIPackHeaderLI, BSRoot);
DEFINE_CLASS(WIFileHeaderLI, BSRoot);

/* ******************************************************************
 *
 *  WIVersion
 *
 ********************************************************************/

/*
 *@@ Compare:
 *      compares the member versions with those of the
 *      given other WIVersion.
 *
 *      This returns one of:
 *
 *      -- IDCOMP_SAMEVERSION (0x00001): both version numbers
 *                  are exactly the same.
 *      -- IDCOMP_THISNEWER (0x00002): "this" has a higher
 *                  version number than "id".
 *      -- IDCOMP_THISOLDER (0x00004): "this" has a lower
 *                  version number than "id".
 *
 *@@added V0.9.18 (2002-03-08) [umoeller]
 */

unsigned long WIVersion::Compare(const WIVersion &v)
                         const
{
    unsigned long ulrc = 0;

    // now compare major version numbers
    if (_ulMajor < v._ulMajor)
        ulrc |= IDCOMP_THISOLDER;
    else if (_ulMajor > v._ulMajor)
        ulrc |= IDCOMP_THISNEWER;
    else
    {
        // major versions match: compare minor versions
        if (_ulMinor < v._ulMinor)
            ulrc |= IDCOMP_THISOLDER;
        else if (_ulMinor > v._ulMinor)
            ulrc |= IDCOMP_THISNEWER;
        else
        {
            // minor versions match as well: compare revisions
            if (_ulRevision < v._ulRevision)
                ulrc |= IDCOMP_THISOLDER;
            else if (_ulRevision > v._ulRevision)
                ulrc |= IDCOMP_THISNEWER;
            else
            {
                // revisions match as well: compare fixlevels
                if (_ulFixlevel < v._ulFixlevel)
                    ulrc |= IDCOMP_THISOLDER;
                else if (_ulFixlevel > v._ulFixlevel)
                    ulrc |= IDCOMP_THISNEWER;
                else
                    // fixlevels match too: that's the same, finally
                    ulrc |= IDCOMP_SAMEVERSION;
            }
        }
    }

    return ulrc;
}

/* ******************************************************************
 *
 *  WIArchive
 *
 ********************************************************************/

/*
 *@@ WIArchive:
 *      the constructor.
 *
 *@@changed V0.9.6 (2000-11-23) [umoeller]: now finally initializing all members...
 */

WIArchive::WIArchive()
    : BSRoot(tWIArchive),
      _FileList(STORE),
      _PackList(STORE)
{
    memset(&_ArcHeader, 0, sizeof(_ArcHeader));
    memset(&_ArcExtHeader4, 0, sizeof(WIArcExtHeader4));

    _StubArchiveName[0] = '\0';

    _fHasStub = 0;

    _hfArchive = 0;
    _hfOldArchive = 0;
    _hfFile = 0;
    _pszScript = 0;
    _pExtended = 0;
    _ArchiveName[0] = 0;

#ifndef BUILDING_STUB_CODE
    _pfnCallback = 0;
#endif

    _ArcStart = 0;

    // set up the CRC table
    unsigned long i, j, r;
    // for (i = 0; i <= 256; i++)
    for (i = 0; i < 256; ++i)       // V0.9.6 (2000-11-23) [umoeller]
    {
        r = i;
        for (j = 0; j < 8; ++j)
            if (r & 1)
                r = (r >> 1) ^ CRCPOLY;
            else
                r >>= 1;

        _crctable[i] = r;
    }

    // _lastError = 0;         // added (99-11-03) [umoeller]
}

/*
 *@@ ~WIArchive:
 *      destructor.
 */

WIArchive::~WIArchive()
{
    // clean up all the lists V0.9.14 (2001-07-24) [umoeller]
    // these are all in STORE mode, thus clearing the list
    // will also nuke the list items. Note that WIFileLI
    // and WIPackHeaderLI each have a destructor which takes
    // care of cleanup.

#ifndef BUILDING_STUB_CODE      // skip this for the stub, we can os2 let handle this
    _FileList.clear();
    _PackList.clear();

    if (_pszScript != NULL)
        free(_pszScript);
    if (_pExtended != NULL)
        free(_pExtended);
#endif
}

#ifndef BUILDING_STUB_CODE

/*
 *@@ CalcPercentage:
 *      Calculates a percentage value using the formula:
 *
 +          (100a + (b/2)) / b
 *
 *      a and b are two long numbers to use during calculations.
 *      Returns the percentage calculated.
 */

short WIArchive::CalcPercentage(long a, long b)
{
    int i;

    for (i = 0; i < 2; ++i)
        if (a <= LONG_MAX / 10)
            a *= 10;
        else
            b /= 10;
    if ((long) (a + (b >> 1)) < a)
    {
        a >>= 1;
        b >>= 1;
    }
    if (b == 0)
        return 0;

    return (short) ((a + (b >> 1)) / b);
}

/*
 *@@ DoesDirExist:
 *
 *@@added V0.9.20 (2002-08-10) [umoeller]
 */

int DoesDirExist(const char *path)
{
    struct stat s;
    if (stat(path, &s))
        return 0;

    if (s.st_mode & S_IFDIR)
        return 1;

    return 0;
}

/*
 *@@ MakeDirectories:
 *      for the given filename, this checks if all directories
 *      (path components, if given) exist. If they don't, they
 *      are created ("deep" makedir).
 *
 *      "filename" can, but does not have to be, fully qualified.
 *
 *@@changed V0.9.9 (2001-03-13) [phaller]: added error checking to avoid hangs/memory hogging
 *@@changed V0.9.9 (2001-04-04) [umoeller]: rewritten, dammit, this didn't work at all
 *@@changed V0.9.20 (2002-08-10) [umoeller]: this never worked for directories several paths deep, fixed
 *@@changed V0.9.20 (2002-08-10) [umoeller]: fixed error handling, which was completely broken
 *@@changed V0.9.20 (2002-08-10) [umoeller]: major speedup for multiple files in the same dir
 */

int WIArchive::MakeDirectories(const char *filename)
{
    // 1) filename is something like "dir\subdir\subsubdir\file.txt"

    const char *pLastBackslash;
    if (pLastBackslash = strrchr(filename, '\\'))
    {
        // we have a path at all:
        unsigned long cb = pLastBackslash - filename;
        char path[MAXPATHLEN];
        memcpy(path, filename, cb);
        path[cb] = '\0';

        // path now contains "dir\subdir\subsubdir"
        //                    01234567

        // optimization for several files in the same dir:
        // V0.9.20 (2002-08-10) [umoeller]

        // have we been here before?
        if (_szLastDirectoryMade[0])
            // yes: is this path the same as last time?
            if (!memicmp(_szLastDirectoryMade,
                         path,
                         cb + 1))       // include null byte!
                // yes: then stop
                return WIERR_NO_ERROR;

        // check the full path for whether it exists...
        // then we don't have to check each particle
        // V0.9.20 (2002-08-10) [umoeller]
        if (!DoesDirExist(path))
        {
            // full path does NOT exist:

            // now go for each path particle

            const char *pStartThis;

            pStartThis = path;

            while (1)
            {
                int rc = 0;

                char *pEndThis;
                if (pEndThis = strchr(pStartThis + 1, '\\'))
                    *pEndThis = '\0';       // temporary terminator

                // check if the dir exists instead of blindly
                // creating this over and over again
                // V0.9.20 (2002-08-10) [umoeller]
                if (!DoesDirExist(path))
                {
                    // directory does not exist:

//                    #ifdef __IBMCPP__
                        rc = mkdir(path);       // IBM VAC++ version
//					#else
//                        rc = mkdir(path, 0);
//                    #endif

                    if (-1 == rc)
                        // return error code
                        return WIERR_FILE_MAKEDIRECTORY;
                }

                if (pEndThis)
                {
                    // restore old char
                    *pEndThis = '\\';

                    // pStartThis = pEndThis;
                    // no, no, no!!! V0.9.20 (2002-08-10) [umoeller]
                    pStartThis = pEndThis + 1;
                }
                else
                    // this was the last component:
                    break;
            }
        }

        // copy the full path of the directory we just created
        // so if the next file is in the same dir, we can
        // save ourselves some work
        // V0.9.20 (2002-08-10) [umoeller]
        memcpy(_szLastDirectoryMade,
               filename,
               cb);
        _szLastDirectoryMade[cb] = '\0';

    }

    return WIERR_NO_ERROR; // OK, done
}

#endif // BUILDING_STUB_CODE

/*
 *@@ ReadArcHeader:
 *      reads in the archive header from the specified archive file
 *      and does some integrity checks.
 *
 *      Returns:
 *
 *      --  NO_ERROR (0): archive header read.
 *
 *      --  WIERR_IO_SEEK
 *
 *      --  WIERR_INVALID_HEADER
 *
 *      --  WIERR_OUTDATED_ARCHIVE
 *
 *      --  WIERR_NEWER_ARCHIVE
 *
 *      --  WIERR_BZDECOMPRESS
 *
 *@@changed V0.9.9 (2001-03-13) [phaller]: added error checking to avoid hangs/memory hogging
 *@@changed V0.9.13 (2001-07-02) [paperino]: fixed some bad header version reports
 *@@changed V0.9.13 (2001-07-02) [umoeller]: finally returning better error codes
 */

int WIArchive::ReadArcHeader(int file) // FILE *file)
{
    bz_stream z;
    long l = 0;             // was missing V0.9.13 (2001-07-02) [umoeller]
    int irc;

_Pmpf(( "WIArchive::ReadArcHeader"));
// begin V0.9.13 (2001-06-23) [paperino]

    // step 1: load arc header from the tail of the file...
    // if this is a new-style archive (with stub), it has
    // two headers, one at the head and one at the tail of
    // the archive (for EXE stub support)
    long pos = (   (   _filelength(file) // _filelength(fileno(file))
                     - sizeof(WIArcHeader)
                     - sizeof(WIArcExtHeader4)) / 512) * 512;
    lseek(file, pos, SEEK_SET);// fseek (file, pos, SEEK_SET);
    long nb = read(file,
                   (char*)&_ArcHeader,
                   sizeof(WIArcHeader));
    _Pmpf(( "WIArchive::ReadArcHeader read %d (vs %d) bytes", nb, sizeof(WIArcHeader)));

    if (    (_ArcHeader.v1 != WI_VERIFY1)
         || (_ArcHeader.v2 != WI_VERIFY2)
         || (_ArcHeader.v3 != WI_VERIFY3)
         || (_ArcHeader.v4 != WI_VERIFY4)
       )
    {
        _Pmpf(( "  tail is not a valid header, searching front"));
        // hmmm, tail did not have a valid header:
        // check front then (old-style archive);
        // the following is the old arc header code
        int fHeaderFound = 0;

        for (l = 0; l < 200000; ++l)
        {
            // seek and check for errors
            if (-1 == lseek(file, l, SEEK_SET))
            {
                _Pmpf(("  returning WIERR_IO_SEEK"));
                return WIERR_IO_SEEK;  // return with error status, V0.9.9 (2001-03-13) [phaller]
            }

            irc = read(file,
                       (char*)&_ArcHeader,
                       sizeof(WIArcHeader));
            if (irc <= 0)  // check operation return code, V0.9.9 (2001-03-13) [phaller]
                // if (ferror(file) != 0) // really a file i/o error ?
                    return WIERR_INVALID_HEADER;  // return with error status

            if (    (_ArcHeader.v1 == WI_VERIFY1)
                 && (_ArcHeader.v2 == WI_VERIFY2)
                 && (_ArcHeader.v3 == WI_VERIFY3)
                 && (_ArcHeader.v4 == WI_VERIFY4)
               )
            {
                fHeaderFound = 1;
                break;  // found header, exit loop
            }
        }

        // check if valid header at loop exit
        if (!fHeaderFound)
        {
            // _still_ not found:
            // get outta here
            _Pmpf(("  returning WIERR_INVALID_HEADER"));
            return WIERR_INVALID_HEADER;
        }
    }

    // we found a valid header: check for extended headers
    switch(_ArcHeader.wi_revision_needed)
    {
        case 0:
        case 1:
        case 2:
            // Paperino: Ulrich, are these still valid?
            // Ulrich: These were used in early alpha versions...
            // Alpha #4 had these, I think.
            _Pmpf(("  returning WIERR_OUTDATED_ARCHIVE"));
            return WIERR_OUTDATED_ARCHIVE;

        case WIARCHIVE_REVISION_3: // warpin releases until 0.9.12
            // ok, no extended headers to read
            break;

        case WIARCHIVE_REVISION_4: // warpin releases from 0.9.13
            // header rev.4 found
            if (read(file,
                     (char*)&_ArcExtHeader4,
                     sizeof(WIArcExtHeader4))
                 < 0)
                return WIERR_INVALID_HEADER;

            // calculate new archive start, seek() is required when extended
            // archive is read at end of file
            l = _ArcExtHeader4.stubSize + sizeof( _ArcHeader) + _ArcExtHeader4.cbSize;
            lseek(file, l, SEEK_SET);

            _Pmpf(( "ReadArcHeader: ext header found, stub size is %d", _ArcExtHeader4.stubSize));
            break;

        default:
            // this release can't handle this revision because file format
            // is unknown at compile time
            _Pmpf(("  returning WIERR_NEWER_ARCHIVE"));
            return WIERR_NEWER_ARCHIVE;
    }

    // read script, if we have one

    _ArcStart = l;
    if (_ArcHeader.usScriptCompr > 0)
    {
        char* pTempBuffer = (char*)malloc(_ArcHeader.usScriptCompr);
        if (!(_pszScript = (char*)malloc(_ArcHeader.usScriptOrig + 1)))
            return WIERR_NOT_ENOUGH_MEMORY;     // V0.9.19 (2002-07-01) [umoeller]

        irc = read(file, pTempBuffer, _ArcHeader.usScriptCompr);
        if (irc <= 0)  // check operation return code, V0.9.9 (2001-03-13) [phaller]
            // if (ferror(file) != 0) // really a file i/o error ?
                return WIERR_INVALID_HEADER;  // return with error status

        // ----- Decompress the installation script
        z.bzalloc = 0;
        z.bzfree  = 0;
        z.opaque = NULL;
        z.bzalloc = NULL;
        z.bzfree = NULL;
        z.next_in = pTempBuffer;
        z.avail_in = _ArcHeader.usScriptCompr;
        z.next_out = _pszScript;
        z.avail_out = _ArcHeader.usScriptOrig + 1;
        int irc = BZ2_bzDecompressInit(&z, 0, 0);
        if (irc != BZ_OK)       // V0.9.2 (2000-03-10) [umoeller]
            return WIERR_BZDECOMPRESS;

        do
        {
            irc = BZ2_bzDecompress(&z);
            if (    (irc != BZ_OK)
                 && (irc != BZ_STREAM_END)
               )
                return WIERR_BZDECOMPRESS;
        }
        while (irc != BZ_STREAM_END);

        BZ2_bzDecompressEnd(&z);
        _pszScript[_ArcHeader.usScriptOrig] = '\0';
    }

    // read extended header, if we have one

    if (_ArcHeader.lExtended > 0)
    {
        if (!(_pExtended = (char*)malloc(_ArcHeader.lExtended)))
            return WIERR_NOT_ENOUGH_MEMORY;     // V0.9.14 (2001-08-23) [umoeller]

        irc = read(file, _pExtended, _ArcHeader.lExtended);
        if (irc <= 0)  // check operation return code, V0.9.9 (2001-03-13) [phaller]
            // if (ferror(file) != 0) // really a file i/o error ?
                return WIERR_INVALID_HEADER;  // return with error status
    }

    return 0;       // no error
}

/*
 *@@ ReadFilenames:
 *      reads all the file names which are stored in the
 *      specified archive.
 *
 *      This does not save the current position!
 *
 *@@changed V0.9.9 (2001-03-13) [phaller]: added error checking to avoid hangs/memory hogging
 */

int WIArchive::ReadFilenames(int file)
{
    WIFileHeader fhead;
    unsigned long pos;
    int i;
    int irc; // operation return code

    list<WIPackHeaderLI*>::iterator pstart = _PackList.begin(),
                                    pend = _PackList.end ();
    for (; pstart != pend; ++pstart)
    {
        WIPackHeader *pPackHeaderThis = (*pstart)->_p;    // V0.9.3 (2000-05-11) [umoeller]

        i = pPackHeaderThis->files;
        // ensure ->files is valid and positive>0!
        if (i <= 0)
            return WIERR_ARCHIVE_CORRUPT;  // return with error status

        pos = pPackHeaderThis->pos;
        // The subsequent fseek calls
        // ensure POS remains within file size. V0.9.9 (2001-03-13) [phaller]

        while (i--)
        {
            if (-1 == lseek(file, pos, SEEK_SET))
                return (WIERR_IO_SEEK); // return with error status

            irc = read(file, (char *)&fhead, sizeof(WIFileHeader));
            if (irc <= 0)  // check operation return code, V0.9.9 (2001-03-13) [phaller]
                return WIERR_IO_READ;

            WIFile *wifile = (WIFile*) malloc(sizeof(WIFile));
            wifile->package = fhead.package;
            wifile->extra = NULL;

            // ensure maximum allowed length of filename
            irc = strlen(fhead.name);
            if (irc > MAXPATHLEN)
                return WIERR_ARCHIVE_CORRUPT; // return with error status

            wifile->name = (char*) malloc(irc + 1);
            strcpy (wifile->name, fhead.name);
            _FileList.push_back(new WIFileLI(wifile));
            pos += sizeof(WIFileHeader) + fhead.compsize;
        }
    }

    return WIERR_NO_ERROR; // OK, done
}

/*
 *@@ ReadPackHeaders:
 *      reads in the contents of all package headers in the specified
 *      archive. This reads from the current position!
 *
 *@@changed V0.9.9 (2001-03-13) [phaller]: added error checking to avoid hangs/memory hogging
 *@@changed V0.9.13 (2001-06-09)   [bird]: changed i < 1 check to i < 0. It IS possible to create .wpis without any packets.
 */

int WIArchive::ReadPackHeaders(int file)
{
    WIPackHeader *p;
    int i;
    int irc;

    i = _ArcHeader.sPackages;
    // ensure .packs is valid and positive>=0!
    if (i < 0)
        return WIERR_ARCHIVE_CORRUPT; // return with error status

    while (i--)
    {
        p = (WIPackHeader*)malloc(sizeof(WIPackHeader));
        irc = read(file,
                   (char*)p,
                   sizeof(WIPackHeader));
        if (irc <= 0)  // check operation return code, V0.9.9 (2001-03-13) [phaller]
            // if (ferror(file) != 0) // really a file i/o error ?
                return WIERR_IO_READ;  // return with error status

        _PackList.push_back(new WIPackHeaderLI(p));
    }

    return WIERR_NO_ERROR; // OK, done
}

/*
 *@@ WriteChecked:
 *      wrapper around write() from the C runtime which
 *      checks if the write was successful and returns
 *      0 (NO_ERROR) in that case.
 *
 *      If we have a problem, we call the callback,if
 *      defined, and return WIERR_IO_WRITE.
 *
 *@@added V0.9.19 (2002-07-01) [umoeller]
 */

int WIArchive::WriteChecked(int handle,
                            const void *buffer,
                            unsigned int count)
{
    if (::write(handle, buffer, count)
        < (signed int)count)
    {
#ifndef BUILDING_STUB_CODE
        if (_pfnCallback)
            _pfnCallback(CBM_ERR_WRITE,
                         CBREC_CANCEL,
                         NULL,
                         _pvCallbackArg);
#endif
        return WIERR_IO_WRITE;
    }

    return 0;
}

/*
 *@@ close:
 *      closes the archive.
 *
 *      Note that WIArchiveRW overrides this method in order to
 *      call WIArchiveRW::Update in turn.
 *
 *@@changed V0.9.19 (2002-07-01) [umoeller]: now returning error code
 */

int WIArchive::close()
{
    ::close(_hfArchive);
    _hfArchive = 0;

    return 0;
}

/*
 *@@ closeAbort:
 *      closes the archive without performing an archive
 *      update. Use this for signal handlers to quickly
 *      close open files.
 *
 *@@added V0.9.4 (2000-07-22) [umoeller]
 */

void WIArchive::closeAbort()
{
    ::close(_hfArchive);
    _hfArchive = 0;
}

#ifndef BUILDING_STUB_CODE

/*
 *@@ getArcHeader:
 *      returns a const pointer to the member archive header.
 */

const WIArcHeader* WIArchive::getArcHeader()
{
    return &_ArcHeader;
}

/*
 *@@ getStubSize:
 *      returns the size of the stub, or 0 if there's none.
 *
 *@@added V0.9.14 (2001-08-23) [umoeller]
 */

unsigned long WIArchive::getStubSize()
{
    if (_fHasStub)
        return (_ArcExtHeader4.stubSize);

    return (0);
}

/*
 *@@ getFileList:
 *      returns a pointer to the member files list.
 */

list<WIFileLI*>* WIArchive::getFileList()
{
    return &_FileList;
}

/*
 *@@ getPackList:
 *      returns a pointer to the member list of packages.
 */

list<WIPackHeaderLI*>* WIArchive::getPackList()
{
    return &_PackList;
}

/*
 *@@ getScript:
 *      returns the decompressed install script or NULL if
 *      none was found.
 */

const char *WIArchive::getScript()
{
    return _pszScript;
}

#endif

/*
 *@@ open:
 *      opens the archive and reads the basic stuff in.
 *
 *      This calls WIArchive::ReadArcHeader, WIArchive::ReadPackHeaders,
 *      and WIArchive::ReadFilenames in turn.
 *
 *      If (mode == 0), the archive is opened in "read-only" mode.
 *
 *      If (mode == 1), the archive is opened in "read-write" mode.
 *      In that case, this method calls WIArchive::OpenTemp to create
 *      a temporary file.
 *
 *      Returns (warning, this has changed with 0.9.13):
 *
 *      --  NO_ERROR (0): archive successfully opened.
 *
 *      --  WIERR_FILENOTFOUND: I/O error opening file.
 *
 *      plus the error codes from WIArchive::ReadArcHeader,
 *      WIArchive::ReadPackHeaders, WIArchive::ReadFilenames.
 *
 *      WIERR_* codes are 15000 and above (see wiarchive.h).
 *
 *@@changed V0.9.13 (2001-07-02) [umoeller]: finally returning better error codes
 */

int WIArchive::open(const char *filename)
{
    strcpy(_ArchiveName, filename);
    _hfArchive = ::open(filename,
                        O_RDONLY | O_BINARY,
                        S_IREAD); // "rb");

    if (_hfArchive != -1)
    {
        // The file exists - check if it's a valid archive
        int i;
        if ((i = ReadArcHeader(_hfArchive)))
            return i;               // return error code V0.9.13 (2001-07-02) [umoeller]
        else
        {
            // Read all the files and packages in this archive
            if ((i = ReadPackHeaders(_hfArchive)))
                return i;           // return error code V0.9.13 (2001-07-02) [umoeller]

            if ((i = ReadFilenames(_hfArchive)))
                return i;           // return error code V0.9.13 (2001-07-02) [umoeller]
        }
    }
    else
        return (WIERR_FILENOTFOUND);

    return 0;               // V0.9.13 (2001-07-02) [umoeller]
}

#ifndef BUILDING_STUB_CODE

/*
 *@@ setCallbackFunc:
 *      sets the WIArchive callback function, which is
 *      used for all kinds of notification to the caller
 *      code, such as progress display and errors.
 *
 *@@changed V0.9.9 (2001-04-04) [umoeller]: added pvCallbackArg
 */

void WIArchive::setCallbackFunc(PFNWICALLBACK pfnCallbackNew,   // in: new callback func
                                void *pvCallbackArg)            // in: user param for callback
{
    _pfnCallback = pfnCallbackNew;
    _pvCallbackArg = pvCallbackArg;
}

#endif

/*
 *@@ unpack:
 *      unpacks the specified package to disk.
 *
 *      This calls WIArchive::MakeDirectories.
 *
 *      Preconditions:
 *
 *      -- The packages list must have been initialized.
 *
 *      Returns (warning, this has changed with 0.9.13):
 *
 *      --  NO_ERROR (0): package unpacked.
 *
 *      --  WIERR_INVALID_INDEX: package doesn't exist.
 *
 *      --  WIERR_IO_SEEK
 *
 *      --  WIERR_ARCHIVE_CORRUPT
 *
 *      --  WIERR_OPERATION_CANCELLED V0.9.20 (2002-07-03) [umoeller]
 *
 *@@changed V0.9.9 (2001-03-13) [phaller]: added error checking to avoid hangs/memory hogging
 *@@changed V0.9.9 (2001-04-06) [umoeller]: now skipping make directories if we got CBRC_SKIP
 *@@changed V0.9.13 (2001-06-21) [umoeller]: fixed CBRC_ABORT which kept running, thanks Yuri Dario
 *@@changed V0.9.13 (2001-07-02) [umoeller]: finally returning better error codes
 *@@changed V0.9.20 (2002-07-03) [umoeller]: added WIERR_OPERATION_CANCELLED
 */

int WIArchive::unpack(short package)
{
    char filename[256];
    int fFound = 0;

    // find the correct package information and start processing the info
    WIPackHeader *pPckHeader = 0;
    list<WIPackHeaderLI*>::iterator  start = _PackList.begin(),
                                     end = _PackList.end();
    for (; start != end; ++start)
    {
        pPckHeader = (**start)._p;
        if (pPckHeader->number == package)
        {
            // found:
            fFound = 1;
            break;
        }
    }

    if (!fFound)
        return WIERR_INVALID_INDEX;       // V0.9.13 (2001-07-02) [umoeller]

    // move file pointer to position in archive,
    // as specified in the package header

    ULONG ulPathLen = strlen(_ArcHeader.szPath);

    // seek and check for errors
    if (-1 == lseek(_hfArchive, pPckHeader->pos, SEEK_SET))
        return WIERR_IO_SEEK;     // V0.9.13 (2001-07-02) [umoeller]

    // get no. of files
    long files = pPckHeader->files;

#ifndef BUILDING_STUB_CODE
    // for each package, reset the last directory
    // cache for MakeDirectories()
    // V0.9.20 (2002-08-10) [umoeller]
    _szLastDirectoryMade[0] = '\0';
#endif

    // The uncompress-all-files-we-can-find loop
    while (files--)
    {
        // read in file header
        WIFileHeader fhead;
        int irc = read(_hfArchive,
                       (char*)&fhead,
                       sizeof(WIFileHeader));
        if (irc <= 0)  // check operation return code, V0.9.9 (2001-03-13) [phaller]
            // if (ferror(_Archive) != 0) // really a file i/o error ?
                return WIERR_IO_READ; // V0.9.13 (2001-07-02) [umoeller]

        // check for magic bytes
        if (fhead.magic != WIFH_MAGIC)  // wiarchive.h
        {
#ifndef BUILDING_STUB_CODE
            if (_pfnCallback)
                // magic byte doesn't match:
                // report decompression error
                _pfnCallback(CBM_ERR_ZLIB,
                             -99,
                             &fhead,
                             _pvCallbackArg);
                    // and stop!
#endif
            return WIERR_ARCHIVE_CORRUPT;     // V0.9.13 (2001-07-02) [umoeller]
        }

        // else go on
        memcpy(filename, _ArcHeader.szPath, ulPathLen);
        strcpy(filename + ulPathLen,
               fhead.name);
        // MakeDirectories(filename);         // Make directories, if they don't exist
                // V0.9.9 (2001-04-06) [umoeller]: moved this down, we only
                // want to create directories if we didn't get "skip"

        int iStatus = 0;
            // error status, meaning:
            //  0       OK
            //  >0      file open/write error
            //  <0      decompression error (zlib)
        int iCallbackRC = CBRC_PROCEED;
            // default is still to overwrite the file,
            // if no callback exists

        // current position in archive
        long int lCurPos = _tell(_hfArchive);

        // call the "next file" callback.
#ifndef BUILDING_STUB_CODE
        if (_pfnCallback)
            // This allows the front-end to do two things:
            // a)   update the "current file" display
            // b)   check for whether that file exists already:
            //      -- if not, we return CBRC_PROCEED;
            //      -- if it does exist, we ask the user for whether the file
            //         may be overwritten; if so, we try to delete it.
            //         Otherwise, we return CBRC_SKIP, and the back-end does
            //         not touch the file.
            // This new approach (Alpha #3) is required because sometimes
            // the target file which already exists is _locked_, e.g. if it's
            // a system DLL in use. We then need to do a DosReplaceModule,
            // which we cannot do in the back-end if we want to keep it
            // platform-independent. Sorry for the hassle.
            iCallbackRC = _pfnCallback(CBM_NEXTFILE,
                                       0,
                                       &fhead,
                                       _pvCallbackArg);
#endif

        if (iCallbackRC == CBRC_ABORT)
            return WIERR_OPERATION_CANCELLED;

        if (iCallbackRC == CBRC_PROCEED)
        {
#ifndef BUILDING_STUB_CODE // V0.9.14 (2001-08-03) [umoeller]
            int rc;
            if (rc = MakeDirectories(filename))         // make directories, if they don't exist
                        // check the return code dammit
                return rc;
#endif
                    // moved this here V0.9.9 (2001-04-06) [umoeller]

            // do-while loop in case the user presses
            // "Retry" on errors below
            // normally, we don't loop
            int fRepeat;
            do
            {
                fRepeat = 0;

                // create the target file!
                _hfFile = ::open(filename,
                                 O_CREAT | O_TRUNC | O_RDWR | O_BINARY,
                                 S_IWRITE | S_IREAD); // fopen "wb");

                if (_hfFile != -1)
                {
                    // file successfully opened for writing:
                    // **** go!
                    switch (fhead.method)
                    {
                        case 0: // stored
                            iStatus = Extract(&fhead);
                        break;

                        case 1: // compressed
                            iStatus = Expand(&fhead);
                        break;

                        default:
                            ::close(_hfFile);
                            return WIERR_UNKNOWN_METHOD;
                    }
                    ::close(_hfFile);
                }
                else
                    // file could not be opened:
                    iStatus = 99;

                // the following blocks added
                if (iStatus == 0)
                {
                    // no error occured: set file date/time
#ifndef BUILDING_STUB_CODE
                    struct utimbuf utb;
                    // utimbuf is declared in sys\utime.h as follows (VAC++):
                    /*   struct utimbuf
                         {
                            time_t actime;          // access time
                            time_t modtime;         // modification time
                         }; */

                    utb.actime  = fhead.lastwrite;
                    utb.modtime = fhead.lastwrite;
                    utime(filename, &utb);
#endif
                }
                else
                {
                    // iStatus is
                    //      99      if the file could not be opened above
                    //   other > 1  if Extract or Expand couldn't write to the file
                    //     < 1      if the zlib had a decompression error

                    // default: skip the file (see below)
                    iCallbackRC = CBRC_SKIP;

#ifndef BUILDING_STUB_CODE
                    if (_pfnCallback)
                    {
                        if (iStatus < 0)
                        {
                            // decompression error:
                            _pfnCallback(CBM_ERR_ZLIB,
                                         iStatus,
                                         &fhead,
                                         _pvCallbackArg);
                                // and stop!
                        }

                        // else: error related to file handling
                        int irc = 0;
                        irc = _pfnCallback(CBM_ERR_WRITE,
                                           CBREC_ABORTRETRYIGNORE,
                                                    // allow all selections
                                           &fhead,
                                           _pvCallbackArg);
                        switch (irc)
                        {
                            case CBRC_ABORT:
                                iCallbackRC = CBRC_ABORT;
                            break;

                            case CBRC_RETRY:
                                fRepeat = 1;    // stay in the do-while loop
                            break;

                            // if the user presses "Ignore", the default
                            // CBRC_SKIP will be used (above)
                        }
                    }
#endif
                }
            } while (fRepeat);  // only if CBRC_RETRY is returned on errors
        }

        switch (iCallbackRC)
        {
            case CBRC_SKIP:
                // file is to be skipped: move the file pointer
                // past the file data, or we'll get garbage for
                // the next file header
                // this is the case if
                // a)   CBM_NEXTFILE returned CBRC_SKIP
                //      (if the target file existed)
                // b)   CBM_ERR_WRITE returned CBRC_SKIP
                //      (if an error occured writing the file)
                // seek and check for errors
                if (-1 == lseek(_hfArchive,
                                lCurPos + fhead.compsize,
                                SEEK_SET))
                    return WIERR_IO_SEEK;   // V0.9.13 (2001-07-02) [umoeller]
            break;

            case CBRC_ABORT:
                return WIERR_OPERATION_CANCELLED;
                        // V0.9.20 (2002-07-03) [umoeller]
            break;
        }
    } // while (files--)

    return 0;
}

#ifndef BUILDING_STUB_CODE

/*
 *@@ forAllFiles:
 *      this calls pfnWICallback for all files which belong to the
 *      specified package. If you want to list all the files, you
 *      need to call this method for each package (by going thru
 *      the package list).
 *
 *      This method accesses the archive file on disk, because
 *      the class-internal list does not have the WIFileHeader's,
 *      which we therefore must read in here. For large archives,
 *      this function can take a bit of time.
 *
 *      The callback must have the following prototype:
 *
 +          int XWPENTRY ForAllFilesCallback(WIFileHeader*, unsigned long)
 *
 *      The callback gets called with the following:
 *
 *      --   WIFileHeader* pwifh  the file header of the file in question
 *
 *      --   unsigned long ulUser the user parameter passed to this function;
 *                                you can use this for a pointer to some
 *                                internal data
 *
 *      This function returns the number of files which were found,
 *      or 0 if no files were found, or -1 if an error occured.
 *
 *@@changed V0.9.9 (2001-03-13) [phaller]: added error checking to avoid hangs/memory hogging
 */

int WIArchive::forAllFiles(WIPackHeader* pckHeader,   // in: package to list files for
                           PFNFORALLFILESCALLBACK pfncbForAllFiles,
                                              // in: callback to call for each file
                           unsigned long ulUser)
                                              // in: user parameter passed to callback
{
    short sFileCount = 0;

    if (pckHeader)
    {
        // found our package:
        // seek and check for errors, V0.9.9 (2001-03-13) [phaller]
        if (-1 == lseek(_hfArchive, pckHeader->pos, SEEK_SET))
            return -1;  // return with error status

        long files = pckHeader->files;

        WIFileHeader fhead;

        // go thru all the files
        while (files--)
        {
			int pp=tell(_hfArchive);

            int irc = read(_hfArchive,
                           (char*)&fhead,
                           sizeof(WIFileHeader));
            if (irc <= 0)  // check operation return code, V0.9.9 (2001-03-13) [phaller]
                // if (ferror(_Archive) != 0) // really a file i/o error ?
                    return -1;  // return with error status

            // check for magic bytes
            if (fhead.magic != WIFH_MAGIC)  // wiarchive.h
            {
                // stop!
                return (-1);
            }

            // else: OK, call callback
            pfncbForAllFiles(&fhead, ulUser);

            // next file: skip data
            long int lCurPos = _tell(_hfArchive);

            // seek and check for error, V0.9.9 (2001-03-13) [phaller]
            if (-1 == lseek(_hfArchive,
                            lCurPos + fhead.compsize,
                            SEEK_SET))
                return -1;  // return with error status
            ++sFileCount;
        }
    }

    return (sFileCount);
}

#endif

extern "C" {

/*
 *@@ bz_internal_error:
 *      error function for bzlib.
 *
 *      With V0.9.14, I added the BZ_NO_STDIO define
 *      to the bzlib makefile to kick out all printf
 *      calls on errors. bzlib then expects an external
 *      C func called bz_internal_error for internal
 *      bzlib errors. Here it is... we can probably
 *      do better still.
 *
 *@@added V0.9.14 (2001-07-24) [umoeller]
 */

void _Export bz_internal_error(int errcode)
{
    exit(98);
}

} // extern "C"


