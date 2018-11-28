/**
 * WarpIN archive viewer / unpacker for Windows
 *
 * This file Copyright (C) 2001 Cornelis Bockemühl.
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, in version 2 as it comes in the COPYING
 * file of this distribution.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <time.h>
#include <io.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys\stat.h>
#include <string.h>

#include <iostream.h>
#include <iomanip.h>

#include <os2.h>

#include <helpers/xstring.h>
#include <base/bs_base.h>
#include <base/bs_list.h>
#include <base/bs_string.h>

#include <wiarchive/wiarchive.h>

enum ACTION {
	DO_NOTHING,
	DO_LIST_FILES,
	DO_LIST_PACKAGES,
	DO_XTRACT_FILES,
	DO_XTRACT_SCRIPT
};

class FuncParam
{
public:
	FuncParam( WIArchive& pArch, char* pFilter )
	:	arch( pArch ),
		filter( pFilter ),
		skipped( 0 ),
		size( 0 ),
		mode( ' ' ) {}

	WIArchive& arch;
	char* filter;
	unsigned long skipped;
	unsigned long size;
	char mode;
};

void help();
void info( WIArchive& arch );
void flist( WIArchive& arch, unsigned long package, char* filter );
int do_flist( WIFileHeader* fhdr, unsigned long pFuncParam );
void plist( WIArchive& arch );
void extract( WIArchive& arch, unsigned long package, char* filter );
int do_extract( WIFileHeader* fhdr, unsigned long pFuncParam );
void script( WIArchive& arch );
bool filter( const string fname, const string mask );

typedef list<WIPackHeaderLI*> pack_header_list;
unsigned long phl_size( pack_header_list& plist );
string char_string( unsigned long len, char ch );

int main( int argc, char* argv[] )
{
	char archive[260];
	char* filter = 0;
	unsigned long package = 0;
	ACTION action = DO_NOTHING;

	// called with no parameters: give a little helping hand
	if( 1 == argc )
	{
		help();
		return 0;
	}

	// get the archive file name
	strcpy( archive, argv[1] );

	// now we need a command
	if( 3 > argc )
	{
		// default is: listing
		action = DO_LIST_FILES;
	}
	else
	{
		// make sure the switch starts with -
		if( '-' != argv[2][0] )
		{
			cout << "Not a command: " << argv[2] << endl;
		}

		switch( argv[2][1] )
		{
			case 'l':
				action = DO_LIST_FILES;
				break;
			case 'p':
				action = DO_LIST_PACKAGES;
				break;
			case 'x':
				action = DO_XTRACT_FILES;
				break;
			case 'X':
				action = DO_XTRACT_SCRIPT;
				break;
			default:
				cout << "Invalid command: " << argv[2] << endl;
		}

		// check if the next argument exists
		if( 4 <= argc )
		{
			// see if we have a package number
			if( isdigit( argv[3][0] ))
			{
				package = atoi( argv[3] );

				// if even more arguments: next ist a file filter
				if( 5 <= argc )
				{
					filter = argv[4];
				}
			}

			// ok, then it's a file filter
			else
			{
				filter = argv[3];
			}
		}
	}

	// open the archive
	WIArchive arch;

	if( 0 != arch.open( archive ))
	{
		strcat( archive, ".wpi" );

		if( 0 != arch.open( archive ))
		{
			cout << "Couldn\'t open archive " << archive << endl;
			return 1;
		}
	}

	// show general info about the archive
	info( arch );

	// now comes the action
	switch( action )
	{
		case DO_LIST_FILES:
		{
			flist( arch, package, filter );
		} break;

		case DO_LIST_PACKAGES:
		{
			plist( arch );
		} break;

		case DO_XTRACT_FILES:
		{
			extract( arch, package, filter );
		} break;

		case DO_XTRACT_SCRIPT:
		{
			script( arch );
		} break;
	}

	// close and terminate
	arch.close();
	return 0;
}

// help screen
void help()
{
	cout << "WinWpi - WarpIN archive viewer / unpacker for Windows v0.21" << endl;
	cout << "(c) 2001-2002 Cornelis Bockemhl" << endl;
	cout << "WarpIN backend (archiver): (c) 1998-2000 Jens B„ckman, Ulrich M”ller" << endl;
	cout << "BzLib2: (c) 1996-2000 Julian R Seward" << endl;
	cout << "Published under the GNU General Public Licence (see the COPYING file)." << endl << endl;
	cout << "Use: winwpi <archive> -<cmd> [<package>] [<files>]" << endl;
	cout << "With: <archive>: name of the archive file" << endl;
	cout << "      -<cmd>:    -l = list files" << endl;
	cout << "                 -p = list packages" << endl;
	cout << "                 -x = extract files" << endl;
	cout << "                 -X = extract script" << endl;
	cout << "      <package>: optional package number" << endl;
	cout << "      <files>:   file name or mask (using * and ? jokers)" << endl;
}

// general archive info
void info( WIArchive& arch )
{
	const WIArcHeader* hdr = arch.getArcHeader();

	cout << "Application:           " << hdr->szAppName << endl;
	cout << "Revision:              " << hdr->sAppRevision << endl;
	cout << "Author:                " << hdr->szDeveloper << endl;
	cout << "Internet download:     " << hdr->szInternetAddr << endl;
	cout << "Operating system:      " << hdr->os << endl;
	cout << "Number of packages:    " << hdr->sPackages << endl;
	cout << "Min. WarpIN revision:  " << hdr->wi_revision_needed << endl;
	cout << "Note: This version of WinWpi supports archives up to WarpIN revision 4"
		 << endl << endl;
}

// list the files
void flist( WIArchive& arch, unsigned long package, char* filter )
{
	pack_header_list* plist = arch.getPackList();
	if( package > phl_size( *plist ))
	{
		cout << "Package number out of range" << endl;
	}

	cout << "Pck  Created               Original    Packed    Ratio  Name" << endl;
	cout << char_string( 79, '-' ).c_str() << endl;

	pack_header_list::iterator it;
	bool all = 0 == package--;
	unsigned long pck, count = 0;
	long rc;
	FuncParam fparam( arch, filter );

	for( it = plist->begin(), pck = 0;
		 it != plist->end();
		 it++, pck++ )
	{
		if( all || ( pck == package ))
		{
			fparam.skipped = 0;
			rc = arch.forAllFiles( (*it)->_p, do_flist, (unsigned long)&fparam );
			if( 0 > rc )
			{
				cout << "Error in archive file" << endl;
				exit( 1 );
			}
			count += rc - fparam.skipped;
		}
	}

	if( 0 == count )
	{
		cout << "No files found" << endl;
	}
	else
	{
		cout << char_string( 79, '-' ).c_str() << endl;
		cout << "Total "
			 << ( all ? phl_size( *plist ) : 1 ) << " package(s), "
			 << count << " file(s), "
			 << fparam.size << " bytes unpacked" << endl;
	}
}

int do_flist( WIFileHeader* fhdr, unsigned long pFuncParam )
{
	FuncParam* fparam = (FuncParam*)pFuncParam;

	if(( 0 == fparam->filter ) ||
	   filter( fhdr->name, fparam->filter ))
	{
		fparam->size += fhdr->origsize;
		double ratio = 100. * fhdr->compsize / fhdr->origsize;
		struct tm* t = localtime( (const long*)&fhdr->creation );

		cout << setw( 3 ) << fhdr->package << "  "
			 << setw( 4 ) << t->tm_year + 1900 << "-"
			 << setw( 2 ) << setfill( '0' ) << t->tm_mon << "-"
			 << setw( 2 ) << t->tm_mday << " "
			 << setw( 2 ) << t->tm_hour << ":"
			 << setw( 2 ) << t->tm_min << ":"
			 << setw( 2 ) << t->tm_sec << "  "
			 << setw( 10 ) << setfill( ' ' ) << fhdr->origsize << "  "
			 << setw( 10 ) << fhdr->compsize << "  "
			 << setw( 3 ) << (unsigned long)floor( ratio + .5 ) << "%  "
			 << fhdr->name << endl;
	}

	else
	{
		fparam->skipped++;
	}

	return 0;
}

// list the packages
void plist( WIArchive& arch )
{
	cout << "Nr   Files   Name" << endl;
	cout << char_string( 60, '-' ).c_str() << endl;

	pack_header_list* plist = arch.getPackList();
	for( list<WIPackHeaderLI*>::iterator it = plist->begin();
		 it != plist->end(); it++ )
	{
		cout << setw( 3 ) << (*it)->_p->number << "  "
			 << setw( 6 ) << (*it)->_p->files << "  "
			 << (*it)->_p->name << endl;
	}
}

// extract files
void extract( WIArchive& arch, unsigned long package, char* filter )
{
	pack_header_list* plist = arch.getPackList();
	if( package > phl_size( *plist ))
	{
		cout << "Package number out of range" << endl;
	}

	list<WIPackHeaderLI*>::iterator it;
	bool all = 0 == package--;
	unsigned long pck, count = 0;

	FuncParam fparam( arch, filter );
	long rc;

	for( it = plist->begin(), pck = 0;
		 it != plist->end();
		 it++, pck++ )
	{
		if( all || ( pck == package ))
		{
			fparam.skipped = 0;
			rc = arch.forAllFiles( (*it)->_p, do_extract, (unsigned long)&fparam );
			if( 0 > rc )
			{
				cout << "Error in archive file" << endl;
				exit( 1 );
			}
			count += rc - fparam.skipped;
		}
	}

	if( 0 == count )
	{
		cout << "No files found" << endl;
	}
}

int do_extract( WIFileHeader* fhdr, unsigned long pFuncParam )
{
	FuncParam* fparam = (FuncParam*)pFuncParam;
	long rc;

	if(( 0 == fparam->filter ) ||
	   filter( fhdr->name, fparam->filter ))
	{
		// create directories, if necessary
		fparam->arch.MakeDirectories( fhdr->name );

		// check if file already exists; ask the user, if yes
		if( 'O' != fparam->mode )
		{
			fparam->arch._hfFile = ::open( fhdr->name,
                                           O_RDONLY | O_TEXT,
                                           S_IREAD ); // fopen "wb");
			if( -1 != fparam->arch._hfFile )
			{
				::close( fparam->arch._hfFile );

				if( 'S' != fparam->mode )
				{
					cout << "File " << fhdr->name << " already exists!" << endl;

					while( true )
					{
						cout << "(s)kip - (S)kip all - (o)verwrite - (O)verwrite all - (a)bort ";
						cout.flush();
						cin >> fparam->mode;

						switch( fparam->mode )
						{
							case 's':
							case 'S':
								// don't continue with this file
								cout << "Skipping " << fhdr->name << endl;
								return 0;

							case 'o':
							case 'O':
								// go on with the file
								break;

							case 'a':
								// the user wants to give up
								cout << "Aborting extraction" << endl;
								exit( 1 );

							default:
								// ask again: we want to get one of the valid letters!
								continue;
						}

						// end the loop
						break;
					}
				}

				else
				{
					// skip all existing mode
					cout << "Skipping " << fhdr->name << endl;
					return 0;
				}
			}
		}

		// tell the user about activities
		cout << (( 0 == fhdr->method ) ? "Extracting " : "Expanding " )
			 << fhdr->name << endl;

		// try to create the file
		fparam->arch._hfFile = ::open( fhdr->name,
                                       O_CREAT | O_TRUNC | O_RDWR | O_BINARY,
                                       S_IWRITE | S_IREAD ); // fopen "wb");
		if( -1 == fparam->arch._hfFile )
		{
			cout << "Could not create file" << endl;
			exit( 1 );
		}

		// keep the position
		int arch_pos=_tell(fparam->arch._hfArchive);

		// extract file
		switch( fhdr->method )
		{
			case 0: // stored
			{
				rc = fparam->arch.Extract( fhdr );
			} break;

			case 1: // compressed
			{
				rc = fparam->arch.Expand( fhdr );
			} break;
		}

		// close the new file
		::close( fparam->arch._hfFile );

		// restore the position
		lseek(fparam->arch._hfArchive,arch_pos,SEEK_SET);

		// error checking
		if( 0 < rc )
		{
			cout << "Error writing file" << endl;
			exit( 1 );
		}
		else if( 0 > rc )
		{
			cout << "Error in archive file" << endl;
			exit( 1 );
		}
	}

	else
	{
		fparam->skipped++;
	}

	return 0;
}

// extract the script
void script( WIArchive& arch )
{
	cout << arch.getScript() << endl;
}

// check if a filenames fits to a filter mask
// note: this filter is more "intelligent" than the normal
// DOS command file filter, in that it allows as many jokers
// within the mask as you want, like i*b??s* or whatever
bool filter( const string fname, const string mask )
{
	unsigned long pn = 0, pm = 0, p;
	bool joker;

	// no mask: fits always
	if( 0 == mask.size() )
	{
		return true;
	}

	while(( pn < fname.size() ) && ( pm < mask.size() ))
	{
		// single question mark fits every character
		if( '?' == mask[pm] )
		{
			pn++;
			pm++;
			continue;
		}

		// mask segment without jokers: from pm to p
		joker = '*' == mask[pm];
		if( joker )
		{
			pm = mask.find_first_not_of( '*', pm );
		}
		if( string::npos == pm )
		{
			// no more non-* chars: mask fits anyway
			return true;
		}
		else
		{
			p = mask.find_first_of( "*?", pm );
			if( string::npos == p )
			{
				p = mask.size();
			}
		}

			// joker: find any occurrence of the segment
		if(( joker &&
			 ( string::npos != ( pn = fname.find( mask.substr( pm, p - pm ), pn )))) ||
			// no joker: next part of name must fit the mask segment
		   ( !joker &&
			 (fname.substr( pn, p - pm ) == mask.substr( pm, p - pm ))))
		{
			pn += p - pm;
			pm = p;
			continue;
		}

		else
		{
			// segment matching failed
			return false;
		}
	}

	// if mask not terminated, it must only contain jokers
	if( pm < mask.size() )
	{
		return string::npos == mask.find_first_not_of( "*?", pm );
	}

	// if file name not terminated, it doesn't fit; otherwise ok
	return pn >= fname.size();
}

unsigned long phl_size( pack_header_list& plist )
{
    int count = 0;

    for( pack_header_list::iterator it = plist.begin();
         it != plist.end();
         it++ )
    {
        count++;
    }

    return count;
}

string char_string( unsigned long len, char ch )
{
    string str;

    for( unsigned long p = 0;
         p < len;
         p++ )
    {
        str += ch;
    }

    return str;
}
