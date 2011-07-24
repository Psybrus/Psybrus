// ============================================================================
//
// Copyright © 2010 Alice Blunt & Neil Richardson.
//
// This file is part of ExcaliburEx.
//
// ExcaliburEx is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ExcaliburEx is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with ExcaliburEx.  If not, see <http://www.gnu.org/licenses/>.
//
// ============================================================================

#ifndef __EX_SSFILEOGG_H__
#define __EX_SSFILEOGG_H__

#include "exSsFile.h"

// ============================================================================
// exSsFileOGG
class exSsFileOGG: public exSsFile
{
private:
	

public:
	exSsFileOGG();
	
	exSsDataFormat		DataFormat() const;
	exU32				NumChannels() const;
	exU32				SampleRate() const;
	exBool				DecodeNextBuffer( void*& Data, exU32& DataSize );
	
};


#endif
