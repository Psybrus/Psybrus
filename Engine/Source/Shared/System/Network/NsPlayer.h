#pragma once

#include "Base/BcTypes.h"


//////////////////////////////////////////////////////////////////////////
// Network player state.
enum class NsPlayerState
{
	NOT_IN_SESSION,
	JOINING_SESSION,
	IN_SESSION,
	LEAVING_SESSION,
};

//////////////////////////////////////////////////////////////////////////
// NsPlayerInfo
struct NsPlayerInfo
{
	enum
	{
		MAX_NAME_LENGTH = 64
	};

	/// UTF-8 encoded player name.
	BcChar Name_[ 64 ];
};

//////////////////////////////////////////////////////////////////////////
// Network player.
class NsPlayer
{
public:
	NsPlayer();
	virtual ~NsPlayer();

	/**
	 * @return Session player belongs to.
	 */
	virtual class NsSession* getSession() const = 0;

	/**
	 * @return ID of player.
	 */
	virtual BcU32 getID() const = 0;
};
