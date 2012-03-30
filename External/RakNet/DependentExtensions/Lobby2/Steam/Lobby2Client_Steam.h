#ifndef __LOBBY_2_CLIENT_STEAM_H
#define __LOBBY_2_CLIENT_STEAM_H

#include "Lobby2Plugin.h"
#include "DS_OrderedList.h"
#include "RakNetTypes.h"

namespace RakNet
{
// This is a pure interface for Lobby2Client_SteamImpl
class RAK_DLL_EXPORT Lobby2Client_Steam : public RakNet::Lobby2Plugin
{
public:	
	// GetInstance() and DestroyInstance(instance*)
	STATIC_FACTORY_DECLARATIONS(Lobby2Client_Steam)

	virtual ~Lobby2Client_Steam() {}

	virtual void SendMsg(Lobby2Message *msg)=0;
	virtual void GetRoomMembers(DataStructures::OrderedList<uint64_t, uint64_t> &_roomMembers)=0;
	virtual const char * GetRoomMemberName(uint64_t memberId)=0;
	virtual bool IsRoomOwner(const uint64_t cSteamID)=0;
	virtual bool IsInRoom(void) const=0;
	virtual uint64_t GetNumRoomMembers(const uint64_t roomid)=0;
	virtual uint64_t GetMyUserID(void)=0;
	virtual const char* GetMyUserPersonalName(void)=0;
	virtual uint64_t GetRoomID(void) const=0;

protected:

};

};

#endif
