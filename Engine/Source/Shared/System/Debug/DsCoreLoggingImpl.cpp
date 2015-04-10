#include "System/Debug/DsCoreLoggingImpl.h"
#include "System/SysKernel.h"

#include "Psybrus.h"


//////////////////////////////////////////////////////////////////////////
// Creator
SYS_CREATOR(DsCoreLoggingImpl);

//////////////////////////////////////////////////////////////////////////
// Ctor
DsCoreLoggingImpl::DsCoreLoggingImpl()
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
DsCoreLoggingImpl::~DsCoreLoggingImpl()
{

}

void DsCoreLoggingImpl::open()
{

}

void DsCoreLoggingImpl::update()
{

}

void DsCoreLoggingImpl::close()
{
	std::lock_guard< std::mutex > Lock( Lock_ );

	Logs_.clear();
}

void DsCoreLoggingImpl::addLog( BcName Category, BcU32 TimeStamp, std::string Message )
{
	std::lock_guard< std::mutex > Lock( Lock_ );

	Logs_[ Category ].push_back( DsCoreLogEntry( TimeStamp, Category, Message ) );
	while ( Logs_[ Category ].size() > 30 )
	{
		Logs_[ Category ].pop_back();
	}

}

std::vector< DsCoreLogEntry > DsCoreLoggingImpl::getEntries( BcName* Categories, BcU32 CategoryCount )
{
	std::lock_guard< std::mutex > Lock( Lock_ );

	std::vector<DsCoreLogEntry> result;
	if ( CategoryCount > 0 )
	{
		for ( BcU32 Idx = 0; Idx < CategoryCount; ++Idx )
		{
			if ( ( Logs_.find( Categories[ Idx ] ) != Logs_.end() ) )
			{
				size_t size = Logs_[ Categories[ Idx ] ].size();
				for ( BcU32 Idx2 = 0; Idx2 < size; ++Idx2 )
				{
					result.push_back( Logs_[ Categories[ Idx ] ][ Idx2 ] );
				}
			}
		}
	}
	else
	{
		for ( auto logs : Logs_ )
		{
			size_t size = logs.second.size();
			for ( BcU32 Idx2 = 0; Idx2 < size; ++Idx2 )
			{
				result.push_back( logs.second[ Idx2 ] );
			}

		}
	}
	std::sort( result.begin(), result.end(),
				[]( const DsCoreLogEntry &a, const DsCoreLogEntry &b ) -> bool
					{
						return a.Timestamp_ < b.Timestamp_;
					}
		);
	return result;
}
