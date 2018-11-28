
/*
 *@@sourcefile wiarchive.h:
 *      declarations of the main back-end things,
 *      such as the WIArchive class.
 *
 *@@added V0.9.1 (2000-02-08) [umoeller]
 *@@include #include "wiarchive\wiarchive.h"
 */

/*
 *      This file Copyright (C) 1998-2001 Jens B„ckman, Ulrich M”ller.
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation, in version 2 as it comes in the COPYING
 *      file of this distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#ifndef WIARCHIVE_INCLUDE
    #define WIARCHIVE_INCLUDE

    /* ******************************************************************
     *
     *  Misc definitions
     *
     ********************************************************************/

    // this is the maximum path length allowed on OS/2
    #define MAXPATHLEN 256

    // the WarpIN self-install package index for stub support
    #define WARPIN_SELF_PACKAGE_FOR_STUB            30000
    // the additional files to be unpacked for a full install
    #define WARPIN_MORE_PACKAGE_FOR_STUB            30001

    /* ******************************************************************
     *
     *  Error definitions
     *
     ********************************************************************/

    #define WIERR_NO_ERROR                  0
    #define WIERR_FIRST_CODE                15000
    #define WIERR_UNSPECIFIED_ERROR         (WIERR_FIRST_CODE + 1)
                // no further, specific error indication given
    #define WIERR_IO_OPEN                   (WIERR_FIRST_CODE + 2)
                // I/O: open failed
    #define WIERR_IO_READ                   (WIERR_FIRST_CODE + 3)
                // I/O: read failed
    #define WIERR_IO_WRITE                  (WIERR_FIRST_CODE + 6)
                // I/O: write failed
    #define WIERR_IO_SEEK                   (WIERR_FIRST_CODE + 7)
                // I/O: seek failed
    #define WIERR_IO_CLOSE                  (WIERR_FIRST_CODE + 8)
                // I/O: close failed
    #define WIERR_FILE_MAKEDIRECTORY        (WIERR_FIRST_CODE + 9)
                // File: creation of directory failed
    #define WIERR_ARCHIVE_CORRUPT           (WIERR_FIRST_CODE + 10)
                // Archive: corruption detected
    #define WIERR_INTERNAL                  (WIERR_FIRST_CODE + 11)
                // Generic internal error
    #define WIERR_OUTDATED_ARCHIVE          (WIERR_FIRST_CODE + 12)
                // added (99-11-03) [umoeller]
    #define WIERR_INVALID_HEADER            (WIERR_FIRST_CODE + 13)
                // added (99-11-03) [umoeller]
    #define WIERR_BZDECOMPRESS              (WIERR_FIRST_CODE + 14)
                // added V0.9.2 (2000-03-10) [umoeller]
    #define WIERR_FILENOTFOUND              (WIERR_FIRST_CODE + 15)
                // added V0.9.2 (2000-03-10) [umoeller]
    #define WIERR_NEWER_ARCHIVE             (WIERR_FIRST_CODE + 16)
                // added V0.9.13 (2001-07-02) [paperino]
    #define WIERR_CANNOT_CREATE             (WIERR_FIRST_CODE + 17)
                // V0.9.13 (2001-07-02) [umoeller]
    #define WIERR_UNKNOWN_METHOD            (WIERR_FIRST_CODE + 18)
                // V0.9.13 (2001-07-02) [umoeller]
    #define WIERR_INVALID_INDEX             (WIERR_FIRST_CODE + 19)
                // V0.9.13 (2001-07-02) [umoeller]
    #define WIERR_NOT_ENOUGH_MEMORY         (WIERR_FIRST_CODE + 20)
                // V0.9.14 (2001-08-23) [umoeller]
    #define WIERR_ANCIENT_WARPIN_INSTALLED  (WIERR_FIRST_CODE + 21)
                // V0.9.14 (2001-08-23) [umoeller]
    #define WIERR_NO_WARPIN_INSTALLED       (WIERR_FIRST_CODE + 22)
                // V0.9.18 (2002-03-03) [umoeller]
    #define WIERR_SCRIPT_TOO_BIG            (WIERR_FIRST_CODE + 23)
                // V0.9.19 (2002-07-01) [umoeller]
    #define WIERR_CANNOT_READ_STUB          (WIERR_FIRST_CODE + 24)
                // V0.9.19 (2002-07-01) [umoeller]
    #define WIERR_OPERATION_CANCELLED       (WIERR_FIRST_CODE + 25)
                // V0.9.20 (2002-07-03) [umoeller]

    /* ******************************************************************
     *
     *  Standard CID error codes V0.9.20 (2002-07-03) [umoeller]
     *
     ********************************************************************/

    /* The following are standard OS/2 CID exit codes
       and are used only for that purpose in WIC, not
       within the program for signalling errors. */

    #define CID_NOERROR                     0x0000
                // Successful program termination occurred.

    #define CIDMASK_WARNING                 0x0004
                // Successful program processing; warning messages logged.
    #define CIDMASK_ERROR                   0x0008
                // Successful program processing; error messages logged.
    #define CIDMASK_SEVERE                  0x0012
                // Successful program processing; severe error messages logged.

    #define CID_RESOURCENOTFOUND            0x0800
                // Data resource was not found.
    #define CID_RESOURCEINUSE               0x0804
                // Data resource access denied because resource is already in use.
    #define CID_RESOURCE_NOT_AUTHORIZED     0x0808
                // Data resource access denied because authorization is missing.
    #define CID_RESOURCE_PATH_NOT_FOUND     0x0812
                // Data path was not found.
    #define CID_PRODUCT_NOT_CONFIGURED      0x0816
                // Product is not configured.
    #define CID_IOERROR                     0x1200
                // Storage medium exception (I/O error) occurred.
    #define CID_DEVICE_NOT_READY            0x1204
                // Device is not ready.
    #define CID_NOT_ENOUGH_DISK_SPACE       0x1208
                // Not enough disk space is available.
    #define CID_INCORRECT_INVOCATION        0x1600
                // Incorrect program invocation was used.
    #define CID_UNEXPECTED_CONDITION        0x1604
                // Unexpected condition occurred.

    #define CID_SUCCESS_REBOOT              0xFE00
                // Successful program processing; reboot queued but no callback required.
    /*
    FE 04 Successful program processing; warning messages logged; reboot
            queued but no callback required.
    FE 08 Successful program processing; error messages logged; reboot
            queued but no callback required.
    FE 12 Successful program processing; severe error messages logged;
            reboot queued but no callback required.
    */
    #define CID_SUCCESS_NO_REBOOT           0xFE40
                // Successful Program Termination - No Reboot required;
                // special meanings can be documented.

    #define CID_ERROR_NO_REBOOT             0xFE80 // - FEFF
                // Unsuccessful Program Termination - No Reboot required;
                // special meanings can be documented.

    #define CID_SUCCESS_REBOOT_CALLBACK     0xFF00
                // Successful program processing;
                // reboot queued and callback required.

    /* ******************************************************************
     *
     *  WIVersion
     *
     ********************************************************************/

    /*
     *@@ WIVersion:
     *      digit representation of a package version number.
     *      Not really used in the backend, but we declare it
     *      here so the stub can use that too.
     *
     *@@added V0.9.18 (2002-03-03) [umoeller]
     */

    struct WIVersion
    {
        unsigned long   _ulMajor,
                        _ulMinor,
                        _ulRevision,
                        _ulFixlevel;

        WIVersion()
        {
            _ulMajor = 0;
            _ulMinor = 0;
            _ulRevision = 0;
            _ulFixlevel = 0;
        }

        unsigned long Compare(const WIVersion &v) const;
    };

    #define IDCOMP_SAMEVERSION  0x00001
    #define IDCOMP_THISNEWER    0x00002
    #define IDCOMP_THISOLDER    0x00004

    /* ******************************************************************
     *
     *  Archive structures
     *
     ********************************************************************/

    /*
     *@@ WIFile:
     *      structure representing a file in an archive.
     */

    struct WIFile
    {
        char    *name;              // Name of file
        char    *extra;             // Possible extra information
        short   package;            // Which package it belongs to
    };

    /*
     *@@ WIFileLI:
     *      listable pointer to a WIFile.
     *
     *      We can't add constructors and destructors
     *      to WIFile itself because that would
     *      garble the binary structure which must be
     *      written as such into the archive. So we
     *      have this wrapper around it which also
     *      takes care of cleanup.
     *
     *@@added V0.9.14 (2001-07-24) [umoeller]
     */

    class WIFileLI : public BSRoot
    {
        public:
            static BSClassID tWIFileLI;

        public:
            WIFile  *_p;

            WIFileLI(WIFile *p) : BSRoot(tWIFileLI)
            {
                _p = p;
            }

            ~WIFileLI()
            {
                if (_p->name)
                    free(_p->name);
                if (_p->extra)
                    free(_p->extra);
                free(_p);
            }
    };

    /*
     *@@ WIArcHeader:
     *      structure representing the header of an archive.
     *      This is stored in binary in the archive, but also
     *      a member of WIArchive.
     *
     *@@changed V0.9.19 (2002-07-01) [umoeller]: renamed members for consistency
     *@@changed V0.9.19 (2002-07-01) [umoeller]: made script sizes unsigned short to allow 64K
     */

    struct WIArcHeader
    {
    // 0x0000:
        unsigned char   v1, v2, v3, v4; // archive verification
    // 0x0004:
        short   wi_revision_needed;     // minimum WarpIN version required
    // 0x0006:
        char    szPath[MAXPATHLEN];     // installation path
                                        // (presently not used)
    // 0x0106:
        char    szAppName[64];          // application name
                                        // (presently not used)
    // 0x0146:
        char    szDeveloper[64];        // delevoper name
                                        // (presently not used)
    // 0x0186
        char    szInternetAddr[128];    // internet download address
                                        // (presently not used)
    // 0x0206
        short   sAppRevision;           // application revision
                                        // (presently not used)
    // 0x0208
        short   os;                     // operating system to install to
                                        // (presently not used)
    // 0x020a
        short   sPackages;              // number of packages in archive
    // 0x020c
        unsigned short usScriptOrig;    // original size of installation script
                                        // made this unsigned V0.9.19 (2002-07-01) [umoeller]
    // 0x020e
        unsigned short usScriptCompr;   // compressed size of installation script
                                        // made this unsigned V0.9.19 (2002-07-01) [umoeller]
    // 0x0210
        long    lExtended;              // extended data
    };

    /*
     *@@ WIArcExtHeader4:
     *      For backward compatibility, add new fields at end of structure.
     *
     *@@changed V0.9.13 (2001-07-01) [umoeller]: added couple of reserved fields for future expansion
     */

    struct WIArcExtHeader4
    {
        long  cbSize;               // size of extended header
        long  stubSize;             // Size of stub
        long  lReserved1,
              lReserved2,
              lReserved3,
              lReserved4,
              lReserved5,
              lReserved6,
              lReserved7,
              lReserved8;
    };

    /*
     *@@ WIPackHeader:
     *      structure representing a package in an archive.
     *      This is stored in binary in the archive.
     */

    struct WIPackHeader
    {
        short number;               // Package number
        short files;                // Number of files in package
        long  pos;                  // Position of first WIFileHeader for this package in archive
        long  origsize;             // Size of package (original)
        long  compsize;             // Size of package (compressed)
        char  name[32];             // Name of package
    };

    /*
     *@@ WIPackHeaderLI:
     *      listable pointer to a WIPackHeader.
     *
     *      We can't add constructors and destructors
     *      to WIPackHeader itself because that would
     *      garble the binary structure which must be
     *      written as such into the archive. So we
     *      have this wrapper around it which also
     *      takes care of cleanup.
     *
     *@@added V0.9.14 (2001-07-24) [umoeller]
     */

    class WIPackHeaderLI : public BSRoot
    {
        public:
            DECLARE_CLASS(WIPackHeaderLI);

        public:
            WIPackHeader  *_p;

            WIPackHeaderLI(WIPackHeader *p) : BSRoot(tWIPackHeaderLI)
            {
                _p = p;
            }

            ~WIPackHeaderLI()
            {
                free(_p);
            }
    };

    const unsigned short WIFH_MAGIC = 0xF012;  // any value for "magic" in file header

    #pragma pack(1)                 // save space in archive

    /*
     *@@ WIFileHeader:
     *      structure representing a file in an archive.
     *      This is stored in binary in the archive.
     *
     *      In the archive, a WIFileHeader comes directly
     *      before the actual file data. To extract files
     *      for a package, we must find the first WIFileHeader
     *      for a package (which is stored in WIPackHeader.pos)
     *      and get the data which comes directly afterwards.
     *
     *      The next WIFileHeader is then at after the file
     *      data, whose size is stored in the "compsize"
     *      member.
     *
     *@@changed V0.9.14 (2001-08-03) [umoeller]: added VAC 3.6.5 time_t fix
     */

    struct WIFileHeader
    {
        unsigned short magic;       // must be WIFH_MAGIC for security
        short   checksum;           // header checksum
        short   method;             // compression method used:
                                    // -- 0: stored (use WIArchive::Extract)
                                    // -- 1: compressed (use WIArchive::Expand)
        short   package;            // which package it belongs to
        long    origsize;           // size of file (original)
        long    compsize;           // size of file (compressed)
        unsigned long crc;          // file CRC checksum
        char    name[MAXPATHLEN];   // filename (*UM#3)
        unsigned long lastwrite;    // file's last write date/time (req. time.h) (*UM#3)
        unsigned long creation;     // file's creation date/time (req. time.h) (*UM#3)
                // we don't need last access date/time
                // V0.9.14 (2001-08-03) [umoeller]:
                // changed lastwrite and creation from time_t to
                // unsigned long because VAC 3.6.5 doesn't use that...
                // VAC 3.6.5 builds break archive binary compatibility

        char  extended;             // size of extended information (if any)
    };

    /*
     *@@ WIFileHeaderLI:
     *      listable pointer to a WIWIFileHeader.
     *
     *      We can't add constructors and destructors
     *      to WIFileHeader itself because that would
     *      garble the binary structure which must be
     *      written as such into the archive. So we
     *      have this wrapper around it which also
     *      takes care of cleanup.
     *
     *@@added V0.9.14 (2001-07-24) [umoeller]
     */

    class WIFileHeaderLI : public BSRoot
    {
        public:
            DECLARE_CLASS(WIFileHeaderLI);

        public:
            WIFileHeader  *_p;

            WIFileHeaderLI(WIFileHeader *p) : BSRoot(tWIFileHeaderLI)
            {
                _p = p;
            }

            ~WIFileHeaderLI()
            {

            }
    };

    #pragma pack()                  // save space in archive

    /* ******************************************************************
     *
     *  Callback definitions
     *
     ********************************************************************/

    /*
     *@@ enCallbackMode:
     *      enumeration for the WICallback "mode" parameter.
     *      This is now an enum to avoid confusion.
     *
     *@@added V0.9.20 (2002-07-03) [umoeller]
     */

    enum enCallbackMode
    {
        CBM_PERCENTAGE,
                    // in this case, "param" contains the percentage of the current
                    // file being processed, and pwifh the current file header.
                    // The return value doesn't matter.
        CBM_UPDATING,
                    // this is for compression after a file has been added to the
                    // archive and the archive is to be updated; looks better if we
                    // have an additional message then.
                    // In this case, "param" has the compression that was achieved,
                    // and pwifh the current file header.
                    // The return value doesn't matter.
        CBM_NEXTFILE,
                    // this comes just before the WIArchive class attempts to open
                    // a new output file for writing. "param" is always 0, "pwifh"
                    // points to the file header which is about to be opened.
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
        CBM_ERR_MEMORY,
                    // Somehow, there wasn't enough memory to do something.  Not even
                    // in swap space?  Well...  We're here, anyway.
                    // We should not return from the callback then, but only terminate
                    // WarpIn.
        CBM_ERR_WRITE,
                    // error writing into output file; this is most probable when the
                    // target disk is full.
                    // "param" then has one of the CBREC_* values below, which identify
                    // whether the error is recoverable. Depending on that value,
                    // we may return one of the CBRC_* values.
        CBM_ERR_READ,
                    // error reading from input file; maybe the download is corrupt.
                    // "param" then has one of the CBREC_* values below, which identify
                    // whether the error is recoverable. Depending on that value,
                    // we may return one of the CBRC_* values.
        CBM_ERR_ZLIB
                    // error somewhere in the zlib decompression part of WIArchive;
                    // this usually signifies that the archive is broken. We then
                    // terminate installation, since there's no way to recover.
                    // "param" then contains one of the ZLIBERR_* flags below.
    };

    // generic callback function prototype
    // this has now three parameters:
    //      short         mode   one of the CBM_* flags below
    //      short         param  meaning depends on CBM_* value
    //      WIFileHeader* pwifh  the file header of the file in question
    typedef int (FNWICALLBACK)(enCallbackMode, short, WIFileHeader*, void*);
    typedef FNWICALLBACK *PFNWICALLBACK;

    // ----- Error descriptions in case of CBM_ERR_ZLIB
    // the following are from zlib.h
    /* const int ZLIBERR_ERRNO           = -1;
                // unknown meaning
    const int ZLIBERR_STREAM_ERROR    = -2;
                // inconsistent stream data, e.g. unexpected NULL
    const int ZLIBERR_DATA_ERROR      = -3;
                // input stream does not match zlib format
    const int ZLIBERR_MEM_ERROR       = -4;
                // not enough memory
    const int ZLIBERR_BUF_ERROR       = -5;
                // not enough room in output buffer
    const int ZLIBERR_VERSION_ERROR   = -6; */
    // removed these, these didn't match zlib.h any more V0.9.12 (2001-04-29) [umoeller]

                // unknown meaning
    // the following are added by WIArchive
    const int ZLIBERR_NO_WIFH_MAGIC   = -99;
                // this occurs if the first "short" in WIFileHeader
                // does not match WIFH_MAGIC; this check is added
                // to avoid crashes in the decompression routines

    // ----- Error recovery flags for callback function
    // If mode == CBM_ERR_xxx, the callback gets one of the following flags
    // in "param", signalling whether the error is recoverable or not.
    // Depending on this flag, we may return one of the CBRC_* values.
    // NOTE: We must never return CBRC_ABORT, which the WIArchive class cannot
    // handle, but terminate the install thread ourselves instead.
    const int CBREC_CANCEL              = 1;
                // error is not recoverable: show "Cancel" only
    const int CBREC_RETRYCANCEL         = 2;
                // show "Cancel" and "Retry"
    const int CBREC_ABORTRETRYIGNORE    = 3;
                // show "Abort", "Retry", "Ignore" (== skip file)

    // ----- Return values for the callback function
    // If mode == CBM_ERR_xxx, the callback must return one of these three.
    const int CBRC_ABORT = 1;
    const int CBRC_RETRY = 2;
    const int CBRC_IGNORE = 3;

    // If mode == CBM_NEXTFILE, return one of these.
    const int CBRC_PROCEED = 4;
    const int CBRC_SKIP = 5;

    // the following are only used in the front-end, but have
    // been added here for consistency of return codes
    // V0.9.9 (2001-03-30) [umoeller]
    const int CBRC_UNLOCK = 6;          // locked file, use DosReplaceModule
    const int CBRC_DEFER = 7;           // locked file, defer thru CONFIG.SYS

    // In all other modes, the return value is not examined by the back-end.

    // Note:
    // If you offer a "Cancel" option in your error / "File exists" dialogs,
    // it is your own responsibility to terminate the install process. The
    // WIArchive class cannot handle a "Cancel" or "Abort" return value.
    // So the safest way to do all this is have the WIArchive class run in
    // a separate thread while unpacking the files, and terminate the thread
    // simply. This is what happens in the WarpIn front-end.

    /* ******************************************************************
     *
     *  Archive-internal things
     *
     ********************************************************************/

    #ifdef WIARCHIVE_INTERNAL
        const unsigned char WI_VERIFY4 = 0xBE;
        const unsigned char WI_VERIFY3 = 0x02;
        const unsigned char WI_VERIFY2 = 0x04;
        const unsigned char WI_VERIFY1 = 0x77;

        const unsigned short WI_OS_OS2 = 1;          // 1 - OS/2

        const unsigned long INIT_CRC = 0xFFFFFFFF;
        const unsigned long CRCPOLY = 0xEBD88320;
        #define UPDATE_CRC(c) \
            head->crc = _crctable[(head->crc ^ (c)) & 0xFF] ^ (head->crc >> CHAR_BIT)

        const unsigned long BUFFERSIZE = 0x6000;

        // extern char *text;
        // extern unsigned long crctable[];
    #endif

    const short WIARCHIVE_REVISION_3 = 3;           // revision for WIArcHeader3
    const short WIARCHIVE_REVISION_4 = 4;           // changed V0.9.13 (2001-06-23) [umoeller]
    // define as most current value
    // const short WIARCHIVE_REVISION        = WIARCHIVE_REVISION_4;
    // const short WIARCHIVE_REVISION_NEEDED = WIARCHIVE_REVISION_4; // changed V0.9.13 (2001-06-23) [umoeller]

    /* ******************************************************************
     *
     *  WIArchive
     *
     ********************************************************************/

    /*
     *@@ PFNFORALLFILESCALLBACK:
     *      callback prototype for WIArchive::for_all_files
     */

    typedef int XWPENTRY FNFORALLFILESCALLBACK(WIFileHeader*, unsigned long);
    typedef FNFORALLFILESCALLBACK *PFNFORALLFILESCALLBACK;

    /*
     *@@ WIArchive:
     *      the back-end WIArchive class, which does file
     *      management for the packages in an archive.
     *
     *      Starting with V0.9.14, the plain WIArchive is
     *      capable of decompression _only_. If you want
     *      to pack (compress) files, use WIArchiveRW, which
     *      inherits from WIArchive.
     *
     *      While the WIArchive class handles directory and
     *      file management within an archive (WPI file), it
     *      has no idea what the contents of an installation
     *      script mean. This is the job of the frontend
     *      (see src\frontend), which creates an instance of
     *      WIArchive to open an archive file, then extracts
     *      the install script and starts parsing and displaying
     *      pages and such.
     *
     *      The file format of a WPI file is roughly as follows:
     *
     +      --  EXE stub, if any
     +
     +      --  WIArcHeader (constant)
     +
     +      --  WIArcExtHeader4 (constant)
     +
     +      --  install script, if any; this has
     +          WIArcHeader.compscript bytes
     +          (if that is 0, there's no such data)
     +
     +      --  extended data, if any; this has
     +          WIArcHeader.extended bytes
     +          (if that is 0, there's no such data)
     +
     +      --  x WIPackHeader structures (count is
     +          specified in WIArcHeader.packs)
     +
     +      --  y WIFileHeader structures, each followed
     +          by each file's compressed or uncompressed
     +          data
     *
     *      WIPackHeader.files specifies the no. of files
     *      in each package, and WIPackHeader.pos specifies
     *      the offset of the package's first WIFileHeader.
     *
     *      Each WIFileHeader has the compsize field, which
     *      has the size of the compressed data (after which
     *      the next WIFileHeader should come up, if any).
     *
     *@@changed V0.9.13 (2001-07-02) [umoeller]: removed _lastError, added proper error return codes to methods
     *@@changed V0.9.14 (2001-07-24) [umoeller]: stripped out all compression code into WIArchiveRW
     *@@changed V0.9.14 (2001-07-24) [umoeller]: replaced all stdio FILE* calls with low-level C library calls (saved another 8 KB)
     *@@changed V0.9.14 (2001-08-03) [umoeller]: replaced all new/delete with malloc/free calls
     */

    class WIArchive : public BSRoot
    {
        public:
            DECLARE_CLASS(WIArchive);

//        protected:  // WinWpi needs some of these as "public"
#ifndef BUILDING_STUB_CODE
            short CalcPercentage(long, long);

            char    _szLastDirectoryMade[MAXPATHLEN];        // V0.9.20 (2002-08-10) [umoeller]

            int MakeDirectories(const char *);
#endif
            int ReadArcHeader(int);
            int ReadFilenames(int);
            int ReadPackHeaders(int);
            short Expand(WIFileHeader *);
            short Extract(WIFileHeader *);

            int WriteChecked(int handle,
                             const void *buffer,
                             unsigned int count);

            WIArcHeader     _ArcHeader;
            WIArcExtHeader4 _ArcExtHeader4;

            int             _fHasStub;          // 1 if a stub was set and should be written;
                                                // only in that case, _ArcHeader will require
                                                // WIArchive version 4
                                                // V0.9.13 (2001-07-06) [umoeller]

            list<WIFileLI*>   _FileList;
            list<WIPackHeaderLI*>
                            _PackList;
            int             _hfArchive;
            int             _hfOldArchive;
            int             _hfFile;             // single input/output file that we're
                                                // currently working on
            char            *_pszScript;
            char            *_pExtended;
            char            _ArchiveName[128];
            char            _StubArchiveName[MAXPATHLEN];
#ifndef BUILDING_STUB_CODE
            PFNWICALLBACK   _pfnCallback;
            void            *_pvCallbackArg;
#endif
            long            _ArcStart;
            unsigned long   _crctable[256];

        public:
            WIArchive();
            virtual ~WIArchive();
            int open(const char *);
            int close();
            void closeAbort();
#ifndef BUILDING_STUB_CODE
            const WIArcHeader *getArcHeader();
            unsigned long getStubSize();
            list<WIFileLI*>* getFileList();
            list<WIPackHeaderLI*>* getPackList();
            const char* getScript();
            void setCallbackFunc(PFNWICALLBACK, void*);
#endif
            int unpack(short);
#ifndef BUILDING_STUB_CODE
            int forAllFiles(WIPackHeader*, PFNFORALLFILESCALLBACK, unsigned long);
#endif

            // int             _lastError;  // added (99-11-03) [umoeller]
                                            // removed V0.9.13 (2001-07-02) [umoeller]
    };

#ifndef BUILDING_STUB_CODE

    /*
     *@@ WIArchiveRW:
     *      read-write version of WIArchive. While the
     *      plain WIArchive class is only good for
     *      unpacking, this thing can also create archives.
     *
     *      This second class, derived from WIArchive,
     *      has been added with V0.9.14 to allow for
     *      a reduced WIArchive if only unpack is needed.
     *      This came in helpful for the stub code which
     *      was significantly smaller this way.
     *
     *      WIC now uses WIArchiveRW, whose interface is
     *      identical to the old WIArchive.
     *
     *@@added V0.9.14 (2001-07-24) [umoeller]
     *@@changed V0.9.14 (2001-07-24) [umoeller]: replaced all stdio FILE* calls with low-level C library calls (saved another 8 KB)
     */

    class WIArchiveRW : public WIArchive
    {
        public:
            DECLARE_CLASS(WIArchiveRW);

        protected:
            int Update();
            int OpenTemp(const char *);
            int AppendArcHeader(int);
            int WriteArcHeader(int);
            int WritePackHeaders(int);
            int Compress(WIFileHeader *);
            int Store(WIFileHeader *);

            bool            _fOpenForReadOnly;
            list<WIFileLI*> _ToDoList;
            int             _hfTempFile;
            char            _TempArchiveName[128];

        public:
            WIArchiveRW();
            virtual ~WIArchiveRW();

            void add(const char *, const char *, short);
            void setStubFile( char*);

            int open(const char *, int = 0);
            int close();
            void closeAbort();

            void remove(const char *, short);
            void setArcHeader(const WIArcHeader &);
            void setPackage(short, const char *);
            void setScript(const char *);
    };

#endif // BUILDING_STUB_CODE

#endif

