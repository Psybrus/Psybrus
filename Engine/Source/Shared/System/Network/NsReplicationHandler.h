#pragma once

//////////////////////////////////////////////////////////////////////////
// @brief Replication handler.
// Implements the construction, serialisation, and deserialisation code
// for replication.
class NsReplicationHandler
{
public:
	NsReplicationHandler();
	virtual ~NsReplicationHandler();

	/**
	 * Called on construction.
	 * @param InData Input data for construction.
	 * @param InDataSize Input data size.
	 * @return Constructed object.
	 */ 
	virtual ReObject* onConstructObject( const void* InData, size_t InDataSize ) = 0;

	/**
	 * Called on destruction.
	 * @param InData Input data for destruction.
	 * @param InDataSize Input data size.
	 * @param Object Object to destruct.
	 * @return Constructed object.
	 */ 
	virtual void onDestructObject( const void* InData, size_t InDataSize, ReObject* Object ) = 0;

	/**
	 * Called when we need to serialise object.
	 * @param OutData Output data stream.
	 * @param OutDataSize Output data stream size.
	 * @param Object Object to serialise.
	 * @return Size of output data stream to send.
	 */
	virtual size_t onSerialiseObject( void* OutData, size_t OutDataSize, ReObject* Object ) = 0;

	/**
	 * Called when we need to serialise object.
	 * @param InData Input data for destruction.
	 * @param InDataSize Input data size.
	 * @param Object Object to deserialise into.
	 */
	virtual void onDeserialiseObject( const void* InData, size_t InDataSize, ReObject* Object ) = 0;

private:

};
