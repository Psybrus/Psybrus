#pragma once

#include "Base/BcTypes.h"
#include "System/Network/NsPlayer.h"

//////////////////////////////////////////////////////////////////////////
// @brief Network player.
class NsPlayerImpl:
	public NsPlayer
{
public:
	NsPlayerImpl( class NsSessionImpl* Session );
	virtual ~NsPlayerImpl();
	
	class NsSession* getSession() const override;
	BcU32 getID() const override;
	
private:
	friend class NsSessionImpl;

	void setID( BcU32 ID );
	NsPlayerState getState() const;

private:
	class NsSessionImpl* Session_;
	BcU32 ID_;
	std::string Name_;
	NsPlayerState State_;
};
