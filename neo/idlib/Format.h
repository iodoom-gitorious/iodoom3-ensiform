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

#ifndef __FORMAT_H__
#define __FORMAT_H__

// ==============================================================================
//! Set precision
// ==============================================================================
class idSetPrecision {
public:
	// ==============================================================================
	//! Constructor
	//!
	//! @param	value	The precision value for the next input
	// ==============================================================================
	idSetPrecision( int value ) : floatPrecision(value) {}

private:
	friend class idFormat;
	int floatPrecision;	//!< The float precision
};

// ==============================================================================
//! Set fill character
// ==============================================================================
class idSetFill {
public:
	// ==============================================================================
	//! Constructor
	//!
	//! @param	fillCh	The new fill character
	// ==============================================================================
	idSetFill( char fillCh ) : ch(fillCh) {}

private:
	friend class idFormat;
	char	ch;		//!< The new fill character
};

// ==============================================================================
//! Format
//!
//! Type safe alternative to VA ( sprintf ).
//! @note
//! @li $*		=> Non-formatted value
//! @li $+23*	=> Left aligned text, fill up 23 fillChars on the right
//! @li $-23*	=> Right aligned text, fill up 23 fillChars on the left
//! @li $?*		=> Accepts an extra input value, but works like the 2 above
// ==============================================================================
class idFormat {
public:
	// ==============================================================================
	//! Constructor
	//!
	//! @param	fmt	Describes the format to use
	// ==============================================================================
	idFormat( const char *fmt = NULL );

	// ==============================================================================
	//! Destructor
	// ==============================================================================
	~idFormat();
		
	// ==============================================================================
	//! Reset the values already entered and (optional) the format
	//!
	//! @param	keep	true to keep the old format
	//! @param	fmt		Describes the format to use
	// ==============================================================================
	void	Reset( bool keep = true, const char *fmt = NULL );

	// ==============================================================================
	//! Byte length
	//!
	//! @return	How many bytes have been written already
	// ==============================================================================
	int		ByteLength( void ) { return offset; }

	// ==============================================================================
	//! GetFloatPrecision
	//!
	//! @return	The float precision
	// ==============================================================================
	int		GetFloatPrecision( void ) { return floatPrecision; }

	// ==============================================================================
	//! Get the formatted buffer
	//!
	//! @return	Pointer to the formatted buffer
	// ==============================================================================
	const char *c_str( void )const { return buffer; }

	// Return the formated buffer
	operator const char *( void ) const { return c_str(); }

	// ==============================================================================
	//! Try to print some value into the buffer, call Error if failed
	//!
	//! @note	Make sure you know what this is before you call it!
	//! @param	fmt	Describes the format to use
	// ==============================================================================
	void	TryPrint( const char *fmt, ... );

	// ==============================================================================
	//! Finishes appending a value
	//!
	//! @note	Make sure you know what this is before you call it!
	//! @return	self
	// ==============================================================================
	idFormat &	Finish( void ) {
		floatPrecision = -1;
		if ( paramCount != -1 )
			OnAppend();
		return *this;
	}

	// Fill with input
	idFormat &operator << ( int value );
	idFormat &operator << ( unsigned int value );
	idFormat &operator << ( short value );
	idFormat &operator << ( unsigned short value );
	//idFormat &operator << ( unsigned long long value );
	idFormat &operator << ( char value );
	idFormat &operator << ( byte value );
	idFormat &operator << ( float value );
	idFormat &operator << ( const char *value );
	idFormat &operator << ( const idStr &value );

	// Manipulators
	idFormat &operator << ( const idSetFill &value ) { fillChar = value.ch; return *this; }
	idFormat &operator << ( const idSetPrecision &value ) { floatPrecision = value.floatPrecision; return *this; }

protected:

	// ==============================================================================
	//! Remove all format entries
	// ==============================================================================
	void	DeleteFormatEntries( void );

	// ==============================================================================
	//! Finished the formating after appending a value
	// ==============================================================================
	void	OnAppend( void );
		
	// ==============================================================================
	//! Check for a variable input ( currently only for fieldWidth filling )
	// ==============================================================================
	bool	CheckVariableInput( void );

	// ==============================================================================
	//! Format entry, used to define what input is expected and what to append after it
	//!
	//! @todo	Move float precision here too ?
	// ==============================================================================
	struct FormatEntry {
		FormatEntry() : append(NULL), fieldWidth(0), takeInput(0), next(NULL) {}

		const char *append;		//!< What to append after getting input
		int			fieldWidth;	//!< Field width ( fill up the rest with fillChar, align right if negative )
		int			takeInput;	//!< Accept variable input instead of a fixed value
		FormatEntry *next;		//!< The next format entry
	};
	FormatEntry *GetFormatEntry( int index );

	static const int bufferSize = 16384;	//!< Size of the buffer
	char	buffer[bufferSize];	//!< The buffer
	int		offset;				//!< The offset into the buffer

	char	fmtBuffer[256];		//!< Format buffer
	FormatEntry *firstFormatEntry;//!< First format entry
	FormatEntry *lastFormatEntry;//!< Last format entry
	int		numFormatEntries;	//!< Number of format entries
	bool	hasFormat;			//!< true if it has format
	int		paramCount;			//!< Number of parameters to be expected
	char	fillChar;			//!< The character to use to fill up

	int		floatPrecision;		//!< The float precision
};

#endif
