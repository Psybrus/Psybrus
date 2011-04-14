///////////////////////////////////////////////////////////////////////////////
// Name:        tests/streams/memstream.cpp
// Purpose:     Test wxMemoryInputStream/wxMemoryOutputStream
// Author:      Hans Van Leemputten
// RCS-ID:      $Id: memstream.cpp 33293 2005-04-02 22:37:58Z VZ $
// Copyright:   (c) 2004 Hans Van Leemputten
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx/wx.h".
// and "wx/cppunit.h"
#include "testprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

// for all others, include the necessary headers
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "wx/mstream.h"

#include "bstream.h"

#define DATABUFFER_SIZE     256

///////////////////////////////////////////////////////////////////////////////
// The test case
//
// Try to fully test wxMemoryInputStream and wxMemoryOutputStream

class memStream : public BaseStreamTestCase<wxMemoryInputStream, wxMemoryOutputStream>
{
public:
    memStream();
    virtual ~memStream();

    CPPUNIT_TEST_SUITE(memStream);
        // Base class stream tests the memStream supports.
        CPPUNIT_TEST(Input_GetSize);
        CPPUNIT_TEST(Input_GetC);
        CPPUNIT_TEST(Input_Read);
        CPPUNIT_TEST(Input_Eof);
        CPPUNIT_TEST(Input_LastRead);
        CPPUNIT_TEST(Input_SeekI);
        CPPUNIT_TEST(Input_TellI);
        CPPUNIT_TEST(Input_Peek);
        CPPUNIT_TEST(Input_Ungetch);

        CPPUNIT_TEST(Output_PutC);
        CPPUNIT_TEST(Output_Write);
        CPPUNIT_TEST(Output_LastWrite);
        CPPUNIT_TEST(Output_SeekO);
        CPPUNIT_TEST(Output_TellO);

        // Other test specific for Memory stream test case.
        CPPUNIT_TEST(Ctor_InFromOut);
    CPPUNIT_TEST_SUITE_END();

protected:
    // Add own test here.
    void Ctor_InFromOut();

private:
    const char *GetDataBuffer();

private:
    // Implement base class functions.
    virtual wxMemoryInputStream  *DoCreateInStream();
    virtual wxMemoryOutputStream *DoCreateOutStream();

private:
    char    m_DataBuffer[DATABUFFER_SIZE];
};

memStream::memStream()
{
    // Init the data buffer.
    for (size_t i = 0; i < DATABUFFER_SIZE; i++)
        m_DataBuffer[i] = (i % 0xFF);
}

memStream::~memStream()
{
    /* Nothing extra for now. */
}

const char *memStream::GetDataBuffer()
{
    return m_DataBuffer;
}

wxMemoryInputStream *memStream::DoCreateInStream()
{
    wxMemoryInputStream *pMemInStream = new wxMemoryInputStream(GetDataBuffer(), DATABUFFER_SIZE);
    CPPUNIT_ASSERT(pMemInStream->IsOk());
    return pMemInStream;
}
wxMemoryOutputStream *memStream::DoCreateOutStream()
{
    wxMemoryOutputStream *pMemOutStream = new wxMemoryOutputStream();
    CPPUNIT_ASSERT(pMemOutStream->IsOk());
    return pMemOutStream;
}

void memStream::Ctor_InFromOut()
{
    wxMemoryOutputStream *pMemOutStream = DoCreateOutStream();
    pMemOutStream->Write(GetDataBuffer(), DATABUFFER_SIZE);
    wxMemoryInputStream *pMemInStream = new wxMemoryInputStream(*pMemOutStream);
    CPPUNIT_ASSERT(pMemInStream->IsOk());
    CPPUNIT_ASSERT(pMemInStream->GetLength() == pMemOutStream->GetLength());
    int len = pMemInStream->GetLength();
    wxStreamBuffer *in = pMemInStream->GetInputStreamBuffer();
    wxStreamBuffer *out = pMemOutStream->GetOutputStreamBuffer();
    void *pIn = in->GetBufferStart();
    void *pOut = out->GetBufferStart();
    CPPUNIT_ASSERT(pIn != pOut);
    CPPUNIT_ASSERT(memcmp(pIn, pOut, len) == 0);
}

// Register the stream sub suite, by using some stream helper macro.
// Note: Don't forget to connect it to the base suite (See: bstream.cpp => StreamCase::suite())
STREAM_TEST_SUBSUITE_NAMED_REGISTRATION(memStream)
