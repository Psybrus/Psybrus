﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics;
using System.Threading;
using System.Timers;
using System.IO;
using RakNet;

namespace SwigTestApp
{
    class TestMain
    {
        static void Main(string[] args)
        {
            Packet testPacket;
            int loopNumber;
            BitStream stringTestSendBitStream = new BitStream();
            BitStream rakStringTestSendBitStream = new BitStream();
            BitStream receiveBitStream = new BitStream();
            String holdingString;
            TimeSpan startTimeSpan;
            RakString rakStringTest = new RakString();

            RakPeerInterface testClient = RakPeer.GetInstance();
            
            testClient.Startup(1, new SocketDescriptor(60000, "127.0.0.1"), 1);

            RakPeerInterface testServer = RakPeer.GetInstance();
            testServer.Startup(1, new SocketDescriptor(60001, "127.0.0.1"), 1);
            testServer.SetMaximumIncomingConnections(1);

            Console.WriteLine("Press enter to start RakString send and receive loop using BitStream.\nBitStream read done into RakString");
            Console.WriteLine("Loop will run for 15 seconds");
            Console.ReadLine();

            testClient.Connect("127.0.0.1", 60001, "", 0);

            String sendString = "The test string";
            stringTestSendBitStream.Write((byte)DefaultMessageIDTypes.ID_USER_PACKET_ENUM);
            stringTestSendBitStream.Write(sendString);

            RakString testRakString = new RakString("Test RakString");
            rakStringTestSendBitStream.Write((byte)DefaultMessageIDTypes.ID_USER_PACKET_ENUM);
            rakStringTestSendBitStream.Write(testRakString);

            startTimeSpan = (DateTime.UtcNow - new DateTime(1970, 1, 1));
            loopNumber = 0;

            while (startTimeSpan.TotalSeconds + 15 > (DateTime.UtcNow - new DateTime(1970, 1, 1)).TotalSeconds)
            {
                testPacket = testServer.Receive();
                if (testPacket != null && testPacket.data[0] == (byte)DefaultMessageIDTypes.ID_USER_PACKET_ENUM)
                {
                    receiveBitStream.Reset();
                    receiveBitStream.Write(testPacket.data, testPacket.length);
                    receiveBitStream.IgnoreBytes(1);
                    receiveBitStream.Read(rakStringTest);
                    Console.WriteLine("Loop number: " + loopNumber + "\nData: " + rakStringTest.C_String());
                }
                testServer.DeallocatePacket(testPacket);
                loopNumber++;
                System.Threading.Thread.Sleep(50);
                testClient.Send(rakStringTestSendBitStream, PacketPriority.LOW_PRIORITY, PacketReliability.RELIABLE_ORDERED, (char)0, new AddressOrGUID(new SystemAddress("127.0.0.1", 60001)), false);
            }

            Console.WriteLine("Press enter to start String send and receive loop using BitStream.\nBitStream read done into String");
            Console.WriteLine("Loop will run for 15 seconds");
            Console.ReadLine();

            startTimeSpan = (DateTime.UtcNow - new DateTime(1970, 1, 1));
            loopNumber = 0;
            while (startTimeSpan.TotalSeconds + 15 > (DateTime.UtcNow - new DateTime(1970, 1, 1)).TotalSeconds)
            {
                testPacket = testServer.Receive();
                if (testPacket != null && testPacket.data[0] == (byte)DefaultMessageIDTypes.ID_USER_PACKET_ENUM)
                {
                    receiveBitStream.Reset();
                    receiveBitStream.Write(testPacket.data, testPacket.length);
                    receiveBitStream.IgnoreBytes(1);
                    receiveBitStream.Read(out holdingString);
                    Console.WriteLine("Loop number: " + loopNumber + "\nData: " + holdingString);
                }
                testServer.DeallocatePacket(testPacket);
                loopNumber++;
                System.Threading.Thread.Sleep(50);
                testClient.Send(stringTestSendBitStream, PacketPriority.LOW_PRIORITY, PacketReliability.RELIABLE_ORDERED, (char)0, new AddressOrGUID(new SystemAddress("127.0.0.1", 60001)), false);
            }
            //If RakString is not freed before program exit it will crash
            rakStringTest.Dispose();
            testRakString.Dispose();

            RakPeer.DestroyInstance(testClient);
            RakPeer.DestroyInstance(testServer);
            Console.WriteLine("Demo complete. Press Enter.");
            Console.Read();
        }
    }
}
