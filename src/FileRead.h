/*=============================================================================
Blobby Volley 2
Copyright (C) 2006 Jonathan Sieber (jonathan_sieber@yahoo.de)
Copyright (C) 2006 Daniel Knobe (daniel-knobe@web.de)

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
=============================================================================*/

#pragma once

#include "File.h"
#include <memory>
 
// forward declarations for convenience functions
struct lua_State;
namespace tinyxml2
{
    class XMLDocument;
}

using XMLDocumentPtr = std::shared_ptr<tinyxml2::XMLDocument>;

/**
	\class FileRead
	\brief Extension of file interface for reading file access.	
	\details Provides various methods
			for reading numbers, strings and raw bytes from a file.
	\todo add more convenience methods for easier integration with lua script loading
			and tinyXML.
	\sa FileWrite
*/
class FileRead : public File
{
	public:
	
		/// \brief default ctor
		/// \details File has to be opended with open()
		/// \throw nothing
		explicit FileRead();
		
		/// \brief constructor which opens a file.
		/// \param filename File to be opened for reading
		/// \throw FileLoadException, if the file could not be loaded
		explicit FileRead(const std::string& filename);
		
		/// \brief opens a file.
		/// \param filename File to be opened for reading
		/// \throw FileLoadException, if the file could not be loaded
		/// \pre No file is currently opened.
		void open(const std::string& filename);
		
		/// destructor, closes the file (if any open)
		/// \sa close()
		/// \throw nothing
		~FileRead();
		
		// ------------------------------------
		//  reading interface
		// ------------------------------------
		/// reads bytes into a buffer
		/// \param target buffer to read into
		/// \param num_of_bytes number of bytes to read
		/// \throw PhysfsFileException when nothing could be read
		/// \throw NoFileOpenedException when called while no file is opened.
		/// \throw EOFException when cless than \p num_of_bytes bytes are available.
		uint32_t readRawBytes( char* target, std::size_t num_of_bytes );
		
		
		/// reads bytes and returns a safe-pointed buffer
		/// the buffer is allocated by this function and has a size of \p num_of_bytes
		/// \param num_of_bytes Number of bytes to read; size of buffer
		/// \throw PhysfsFileException when nothing could be read
		/// \throw NoFileOpenedException when called while no file is opened.
		/// \throw EOFException when cless than \p num_of_bytes bytes are available.
		boost::shared_array<char> readRawBytes( std::size_t num_of_bytes );
		
		/// reads exactly one byte
		/// \throw PhysfsFileException when Physfs reports an error
		/// \throw NoFileOpenedException when called while no file is opened.
		char readByte();
		
		/// reads an unsinged 32 bit integer from the next four bytes in the file
		/// the integer is expected to be in little-endian-order and is converted
		/// to the native format.
		/// \throw PhysfsFileException when Physfs reports an error
		/// \throw NoFileOpenedException when called while no file is opened.
		uint32_t readUInt32();
		
		/// reads a 32 bit float from the next four bytes in the file
		/// \throw PhysfsFileException when Physfs reports an error
		/// \throw NoFileOpenedException when called while no file is opened.
		float readFloat();
		
		/// reads a null-terminated string from the file
		/// \throw PhysfsFileException when Physfs reports an error
		/// \throw NoFileOpenedException when called while no file is opened.
		std::string readString();
		
		
		// helper function for checksum
		/// calculates a crc checksum of the file contents beginning at posInFile till the end of the file.
		uint32_t calcChecksum(uint32_t start);
		
		
		// -----------------------------------------------------------------------------------------
		// 								LUA/XML reading helper function
		// -----------------------------------------------------------------------------------------
		static std::string makeLuaFilename(std::string filename);
		static int readLuaScript(const std::string& filename, lua_State* mState);
		
		static XMLDocumentPtr readXMLDocument(const std::string& filename);
};
