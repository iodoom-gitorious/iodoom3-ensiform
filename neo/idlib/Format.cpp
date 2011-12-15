/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company. 

This file is part of the Doom 3 GPL Source Code (?Doom 3 Source Code?).  

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/
/*
===========================================================================
The Open Game Libraries.
Copyright (C) 2007-2010 Lusito Software

Author:         Santo Pfingsten (TTK-Bandit)
Author (Edit):  Jeremy Davis (Ensiform)
Purpose:        Text Formatting
Edit Reason:    Make Compatible with idLib
-----------------------------------------

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

3. This notice may not be removed or altered from any source distribution.
===========================================================================
*/

#include "precompiled.h"
#pragma hdrstop

#if _MSC_VER
	#define ID_DEBUG_BREAK() { __debugbreak(); }
#elif __GNUC__
	#define ID_DEBUG_BREAK() { __asm__ __volatile__ ("int $0x03"); }
#elif OG_MACOS_X //! @todo	compiler specific ?
	#define ID_DEBUG_BREAK() { kill( getpid(), SIGINT ); }
#endif

/*
==============================================================================

  idFormat

==============================================================================
*/

/*
================
idFormat::idFormat
================
*/
idFormat::idFormat( const char *fmt ) {
	firstFormatEntry = NULL;
	lastFormatEntry = NULL;
	numFormatEntries = 0;
	Reset( false, fmt );
}
/*
================
Format::~Format
================
*/
idFormat::~idFormat() {
	DeleteFormatEntries();
}

/*
================
idFormat::DeleteFormatEntries
================
*/
void idFormat::DeleteFormatEntries( void ){
	FormatEntry *temp;
	while( firstFormatEntry ) {
		temp = firstFormatEntry;
		firstFormatEntry = firstFormatEntry->next;
		delete temp;
	}
	numFormatEntries = 0;
}

/*
================
idFormat::Reset
================
*/
void idFormat::Reset( bool keep, const char *fmt ) {
	floatPrecision = -1;
	fillChar = ' ';

	if ( keep ) {
		if ( !hasFormat ) {
			offset = 0;
			paramCount = -1;
		} else {
			offset = strlen( fmtBuffer );
			memcpy( buffer, fmtBuffer, offset );
			if ( paramCount > 0 )
				paramCount = 0;

			FormatEntry *entry = firstFormatEntry;
			while( entry ) {
				if ( entry->takeInput == 1 )
					entry->takeInput = 2;
				entry = entry->next;
			}
		}
		buffer[offset] = '\0';
		return;
	}
	DeleteFormatEntries();

	hasFormat = (fmt != NULL);
	if ( !hasFormat ) {
		offset = 0;
		paramCount = -1;
		fmtBuffer[0] = '\0';
	} else {
		strcpy( fmtBuffer, fmt );
		fmtBuffer[255] = '\0';

		int end;
		offset = -1;
		for( int r=0, w=0; fmtBuffer[r] != '\0'; r++, w++ ) {
			if ( fmtBuffer[r] == '$' ) {
				if ( fmtBuffer[r+1] == '$' )
					r++;
				else {
					FormatEntry *entry = new FormatEntry;
					if ( firstFormatEntry == NULL )
						firstFormatEntry = lastFormatEntry = entry;
					else
						lastFormatEntry = lastFormatEntry->next = entry;
					numFormatEntries++;

					entry->fieldWidth = 0;
					end = r;
					if ( fmtBuffer[r+1] == '?' ) {
						entry->takeInput = 2;
						r++;
					} else if ( fmtBuffer[r+1] == '+' || fmtBuffer[r+1] == '-' ) {
						bool negative = fmtBuffer[r+1] == '-';
						entry->takeInput = 0;
						for ( r += 2; isdigit(fmtBuffer[r]); r++ )
							entry->fieldWidth = 10 * entry->fieldWidth + (fmtBuffer[r] - '0');
						if ( negative )
							entry->fieldWidth *= -1;
						r--;
					}
					if ( fmtBuffer[r+1] == '*' ) {
						fmtBuffer[w] = '\0';
						if ( offset == -1 ) {
							memcpy( buffer, fmtBuffer, r );
							offset = end;
						}
						r++;
						entry->append = fmtBuffer + r + 1;
						w = r;
						continue;
					}
					ID_DEBUG_BREAK()
					//! @todo	error ?
				}
			}
			fmtBuffer[w] = fmtBuffer[r];
		}
		// This assert gets triggered when you have no parameters specified in the format.
		//! @todo	add an error here ?
		assert( offset != -1 );
		if ( offset != -1 ) {
			paramCount = 0;
		} else {
			// no params specified, act like this is the first parameter
			offset = strlen( fmtBuffer );
			memcpy( buffer, fmtBuffer, offset );
			paramCount = -1;
		}
	}
	buffer[offset] = '\0';
}

/*
================
idFormat::GetFormatEntry
================
*/
idFormat::FormatEntry *idFormat::GetFormatEntry( int index ) {
	//! @todo: do this nicer
	FormatEntry *entry = firstFormatEntry;
	for( int i=0; i<index && entry; i++ )
		entry = entry->next;
	return entry;
}

/*
================
idFormat::TryPrint
================
*/
void idFormat::TryPrint( const char *fmt, ... ) {
	int size = bufferSize - offset;
	if ( size > 1 ) {
		va_list	list;
		va_start(list, fmt);
		int ret = idStr::vsnPrintf( buffer+offset, size, fmt, list );
		if ( ret > 0 ) {
			int fieldWidth = 0;
			if ( paramCount != -1 && paramCount < numFormatEntries ) {
				FormatEntry *entry = GetFormatEntry( paramCount );
				if ( entry )
					fieldWidth = entry->fieldWidth;
			}
			bool alignLeft = true;
			if ( fieldWidth < 0 ) {
				fieldWidth *= -1;
				alignLeft = false;
			}
			if ( ret >= fieldWidth )
				offset += ret;
			else {
				if ( !alignLeft ) {
					int max = Min( offset + fieldWidth - ret, bufferSize-1 );
					for( ; offset < max; offset++ )
						buffer[offset] = fillChar;
					size = bufferSize - offset;
					if ( size > 1 ) {
						ret = idStr::vsnPrintf( buffer+offset, size, fmt, list );
						if ( ret > 0 )
							offset += ret;
					}
				} else {
					offset += ret;
					int max = Min( offset + fieldWidth - ret, bufferSize-1 );
					for( ; offset < max; offset++ )
						buffer[offset] = fillChar;
					buffer[offset] = '\0';
				}
			}
		}
		va_end(list);
		if ( ret == -1 )
			common->Printf( S_COLOR_RED "ERROR: Buffer overflow in idFormat::TryPrint, size:%d.\n", size );
	}
}

/*
================
idFormat::OnAppend
================
*/
void idFormat::OnAppend( void ) {
	// This assert gets triggered when you add more parameters than where specified in the format.
	//! @todo	add an error here ?
	assert( paramCount < numFormatEntries );
	if ( paramCount < numFormatEntries ) {
		FormatEntry *entry = GetFormatEntry( paramCount++ );
		if ( entry )
			TryPrint( "%s", entry->append );
	}
}

/*
================
idFormat::CheckVariableInput
================
*/
bool idFormat::CheckVariableInput( void ) {
	// This assert gets triggered when you add more parameters than where specified in the format.
	//! @todo	add an error here ?
	assert( paramCount != -1 && paramCount < numFormatEntries );
	if ( paramCount < numFormatEntries ) {
		FormatEntry *entry = GetFormatEntry( paramCount );
		if ( entry && entry->takeInput == 2 ) {
			entry->takeInput = 1;
			return true;
		}
	}
	return false;
}

/*
================
idFormat::operator <<
================
*/
idFormat &idFormat::operator << ( int value ) {
	if ( paramCount != -1 && CheckVariableInput() ) {
		FormatEntry *entry = GetFormatEntry( paramCount );
		if ( entry )
			entry->fieldWidth = value;
		return *this;
	}
	TryPrint( "%d", value );
	return Finish();
}

idFormat &idFormat::operator << ( unsigned int value ) {
	if ( paramCount != -1 && CheckVariableInput() ) {
		FormatEntry *entry = GetFormatEntry( paramCount );
		if ( entry )
			entry->fieldWidth = value;
		return *this;
	}
	TryPrint( "%u", value );
	return Finish();
}

idFormat &idFormat::operator << ( short value ) {
	if ( paramCount != -1 && CheckVariableInput() ) {
		FormatEntry *entry = GetFormatEntry( paramCount );
		if ( entry )
			entry->fieldWidth = value;
		return *this;
	}
	TryPrint( "%d", value );
	return Finish();
}

idFormat &idFormat::operator << ( unsigned short value ) {
	if ( paramCount != -1 && CheckVariableInput() ) {
		FormatEntry *entry = GetFormatEntry( paramCount );
		if ( entry )
			entry->fieldWidth = value;
		return *this;
	}
	TryPrint( "%u", value );
	return Finish();
}

#if 0
idFormat &idFormat::operator << ( unsigned long long value ) {
	// Should this support fieldWidth?
	TryPrint( "%llu", value );
	return Finish();
}
#endif

idFormat &idFormat::operator << ( char value ) {
	TryPrint( "%c", value );
	return Finish();
}

idFormat &idFormat::operator << ( byte value ) {
	TryPrint( "%u", value );
	return Finish();
}

idFormat &idFormat::operator << ( float value ) {
	if ( floatPrecision >= 0 )
		TryPrint( "%.*f", floatPrecision, value );
	else
		TryPrint( "%f", value );
	return Finish();
}

idFormat &idFormat::operator << ( const char *value ) {
	TryPrint( "%s", value );
	return Finish();
}

idFormat &idFormat::operator << ( const idStr &value ) {
	TryPrint( "%s", value.c_str() );
	return Finish();
}
