#include "PacketDropPlugin.h"

PacketDropPlugin::PacketDropPlugin(void)
{
	timer.SetTimerLength(500);
}

PacketDropPlugin::~PacketDropPlugin(void)
{
}

PluginReceiveResult PacketDropPlugin::OnReceive(Packet *packet)
{
	if (timer.IsExpired())
	{
		return RR_CONTINUE_PROCESSING;
	}
	else
	{
		return RR_STOP_PROCESSING;
	}

}

void PacketDropPlugin::StartTest()
{
	timer.Start();

}
