#pragma once

//////////////////////////////////////////////////////////////////////////
// Priority. Maps directly onto RakNet's priority.
enum class NsPriority
{
	IMMEDIATE,
	HIGH,
	MEDIUM,
	LOW
};

//////////////////////////////////////////////////////////////////////////
// Reliability. Maps directly onto RakNet's
enum class NsReliability
{
	/// Same as regular UDP, except that it will also discard duplicate datagrams.  RakNet adds (6 to 17) + 21 bits of overhead, 16 of which is used to detect duplicate packets and 6 to 17 of which is used for message length.
	UNRELIABLE,

	/// Regular UDP with a sequence counter.  Out of order messages will be discarded.
	/// Sequenced and ordered messages sent on the same channel will arrive in the order sent.
	UNRELIABLE_SEQUENCED,

	/// The message is sent reliably, but not necessarily in any order.  Same overhead as UNRELIABLE.
	RELIABLE,

	/// This message is reliable and will arrive in the order you sent it.  Messages will be delayed while waiting for out of order messages.  Same overhead as UNRELIABLE_SEQUENCED.
	/// Sequenced and ordered messages sent on the same channel will arrive in the order sent.
	RELIABLE_ORDERED,

	/// This message is reliable and will arrive in the sequence you sent it.  Out or order messages will be dropped.  Same overhead as UNRELIABLE_SEQUENCED.
	/// Sequenced and ordered messages sent on the same channel will arrive in the order sent.
	RELIABLE_SEQUENCED,
};

//////////////////////////////////////////////////////////////////////////
// NsEventProxyID
using NsEventProxyID = BcU8;
