#pragma once

#include "Reflection/ReReflection.h"

#include "System/Network/NsSession.h"

#include <unordered_map>

//////////////////////////////////////////////////////////////////////////
// IDs.
using NsReplicationClassID = BcU8;

//////////////////////////////////////////////////////////////////////////
// @brief Replication management.
class NsReplication:
	public NsSessionMessageHandler
{
public:
	NsReplication();
	~NsReplication();

	/**
	 * Add object.
	 */
	void addObject( ReObject* Object );

	/**
	 * Remove object.
	 */
	void removeObject( ReObject* Object );

	/**
	 * Add handler.
	 * This handles serialisation of object types.
	 * @param Class Class type to handle serialisation of.
	 * @param Handler Derived implementation of a handler.
	 */
	void addHandler( const ReClass* Class, class NsReplicationHandler* Handler );

	/**
	 * Remove handler.
	 * This handles serialisation of object types.
	 * @param Class Class type to handle serialisation of.
	 * @param Handler Derived implementation of a handler.
	 */
	void removeHandler( const ReClass* Class, class NsReplicationHandler* Handler );

	/**
	 * Find class ID.
	 * @pre Class already has a registered handler.
	 * @post Return a valid ID for class.
	 */
	NsReplicationClassID findClassID( const ReClass* Class ) const;

	/**
	 * Update.
	 */
	void update();

private:
	void onMessageReceived( const void* Data, size_t DataSize ) override;


private:
	static const size_t NUM_CLASS_BITS = 8;
	static const size_t NUM_CLASSES = ( 2 << NUM_CLASS_BITS );
	static_assert( sizeof( NsReplicationClassID ) == ( NUM_CLASS_BITS / 8 ),
		"NsReplicationClassID is too large. Expecting to be NUM_CLASS_BITS." );

	struct ClassHandlerEntry
	{
		const ReClass* Class_;
		class NsReplicationHandler* Handler_;
	};

	using ClassHandlerList = std::array< ClassHandlerEntry, NUM_CLASSES >;
	using ObjectList = std::vector< ReObject* >;
	using ObjectLists = std::array< ObjectList, NUM_CLASSES >; 

	struct PeerData
	{
		ObjectLists ObjectLists_;
	};

	using PeerDataList = std::vector< PeerData >;
	
	ClassHandlerList ClassHandlerList_;
	PeerDataList PeerDataList_;
};
