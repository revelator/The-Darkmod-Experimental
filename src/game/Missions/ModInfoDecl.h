/*****************************************************************************
					The Dark Mod GPL Source Code

					This file is part of the The Dark Mod Source Code, originally based
					on the Doom 3 GPL Source Code as published in 2011.

					The Dark Mod Source Code is free software: you can redistribute it
					and/or modify it under the terms of the GNU General Public License as
					published by the Free Software Foundation, either version 3 of the License,
					or (at your option) any later version. For details, see LICENSE.TXT.

					Project: The Dark Mod (http://www.thedarkmod.com/)

					$Revision$ (Revision of last commit)
					$Date$ (Date of last commit)
					$Author$ (Author of last commit)

					******************************************************************************/

#ifndef _MOD_INFO_DECL_H_
#define _MOD_INFO_DECL_H_

#include "precompiled_game.h"
#include <boost/shared_ptr.hpp>

class CModInfoDecl {
private:
	// The body text used for saving
	idStr _bodyText;

public:
	// Construct this declaration from the given token stream
	bool Parse( idLexer &src );

	/// Key/value data parsed from the mod info decl.
	idDict data;

	// Regenerates the declaration body using the given name as decl name
	void Update( const idStr &name );

	// Append the data to the given file
	void SaveToFile( idFile *file );

	static const char *const TYPE_NAME;
};
typedef boost::shared_ptr<CModInfoDecl> CModInfoDeclPtr;

#endif /* _MOD_INFO_DECL_H_ */
