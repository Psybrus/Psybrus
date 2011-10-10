/*
     File:       FSpCompat.h
 
     Contains:   FSSpec compatibility functions.
 
     Version:    Technology: MoreFiles
                 Release:    1.5.2
 
     Copyright:  � 1992-2001 by Apple Computer, Inc., all rights reserved.
 
     Bugs?:      For bug reports, consult the following page on
                 the World Wide Web:
 
                     http://developer.apple.com/bugreporter/
 
*/

/*
    You may incorporate this sample code into your applications without
    restriction, though the sample code has been provided "AS IS" and the
    responsibility for its operation is 100% yours.  However, what you are
    not permitted to do is to redistribute the source as "DSC Sample Code"
    after having made changes. If you're going to re-distribute the source,
    we require that you make it clear in the source that the code was
    descended from Apple Sample Code, but that you've made changes.
*/

#ifndef __FSPCOMPAT__
#define __FSPCOMPAT__

#ifndef __MACTYPES__
#include <MacTypes.h>
#endif

#ifndef __FILES__
#include <Files.h>
#endif

#include "Optimization.h"


#if PRAGMA_ONCE
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if PRAGMA_IMPORT
#pragma import on
#endif

#if PRAGMA_STRUCT_ALIGN
    #pragma options align=mac68k
#elif PRAGMA_STRUCT_PACKPUSH
    #pragma pack(push, 2)
#elif PRAGMA_STRUCT_PACK
    #pragma pack(2)
#endif

/*****************************************************************************/

EXTERN_API( OSErr )
FSMakeFSSpecCompat(
  short              vRefNum,
  long               dirID,
  ConstStr255Param   fileName,
  FSSpec *           spec);


/*
    The FSMakeFSSpecCompat function fills in the fields of an FSSpec record.
    If the file system can't create the FSSpec, then the compatibility code
    creates a FSSpec that is exactly like an FSSpec except that spec.name
    for a file may not have the same capitalization as the file's catalog
    entry on the disk volume. That is because fileName is parsed to get the
    name instead of getting the name back from the file system. This works
    fine with System 6 where FSMakeSpec isn't available.
    
    vRefNum     input:  Volume specification.
    dirID       input:  Directory ID.
    fileName    input:  Pointer to object name, or nil when dirID specifies
                        a directory that's the object.
    spec        output: A file system specification to be filled in by
                        FSMakeFSSpecCompat.
    
    Result Codes
        noErr               0       No error    
        nsvErr              -35     Volume doesn�t exist    
        fnfErr              -43     File or directory does not exist
                                    (FSSpec is still valid) 
*/

/*****************************************************************************/

EXTERN_API( OSErr )
FSpOpenDFCompat(
  const FSSpec *  spec,
  char            permission,
  short *         refNum);


/*
    The FSpOpenDFCompat function opens the data fork of the file specified
    by spec.
    Differences from FSpOpenDF: If FSpOpenDF isn't available,
    FSpOpenDFCompat uses PHBOpen because System 6 doesn't support PBHOpenDF.
    This means FSpOpenDFCompat could accidentally open a driver if the
    spec->name begins with a period.
    
    spec        input:  An FSSpec record specifying the file whose data
                        fork is to be opened.
    permission  input:  A constant indicating the desired file access
                        permissions.
    refNum      output: A reference number of an access path to the file's
                        data fork.
    
    Result Codes
        noErr               0       No error    
        nsvErr              -35     No such volume  
        ioErr               -36     I/O error   
        bdNamErr            -37     Bad filename    
        tmfoErr             -42     Too many files open 
        fnfErr              -43     File not found  
        opWrErr             -49     File already open for writing   
        permErr             -54     Attempt to open locked file for writing 
        dirNFErr            -120    Directory not found or incomplete pathname
        afpAccessDenied     -5000   User does not have the correct access to
                                    the file
    
    __________
    
    See also:   FSpOpenAware
*/

/*****************************************************************************/

EXTERN_API( OSErr )
FSpOpenRFCompat(
  const FSSpec *  spec,
  char            permission,
  short *         refNum);


/*
    The FSpOpenRFCompat function opens the resource fork of the file
    specified by spec.
    
    spec        input:  An FSSpec record specifying the file whose resource
                        fork is to be opened.
    permission  input:  A constant indicating the desired file access
                        permissions.
    refNum      output: A reference number of an access path to the file's
                        resource fork.
    
    Result Codes
        noErr               0       No error    
        nsvErr              -35     No such volume  
        ioErr               -36     I/O error   
        bdNamErr            -37     Bad filename    
        tmfoErr             -42     Too many files open 
        fnfErr              -43     File not found  
        opWrErr             -49     File already open for writing   
        permErr             -54     Attempt to open locked file for writing 
        dirNFErr            -120    Directory not found or incomplete pathname
        afpAccessDenied     -5000   User does not have the correct access to
                                    the file
    
    __________
    
    See also:   FSpOpenRFAware
*/

/*****************************************************************************/

EXTERN_API( OSErr )
FSpCreateCompat(
  const FSSpec *  spec,
  OSType          creator,
  OSType          fileType,
  ScriptCode      scriptTag);


/*
    The FSpCreateCompat function creates a new file with the specified
    type, creator, and script code.
    Differences from FSpCreate: FSpCreateCompat correctly sets the
    fdScript in the file's FXInfo record to scriptTag if the problem
    isn't fixed in the File Manager code.
    
    spec        input:  An FSSpec record specifying the file to create.
    creator     input:  The creator of the new file.
    fileType    input   The file type of the new file.
    scriptCode  input:  The code of the script system in which the file
                        name is to be displayed.
    
    Result Codes
        noErr               0       No error    
        dirFulErr           -33     File directory full 
        dskFulErr           -34     Disk is full    
        nsvErr              -35     No such volume  
        ioErr               -36     I/O error   
        bdNamErr            -37     Bad filename    
        fnfErr              -43     Directory not found or incomplete pathname  
        wPrErr              -44     Hardware volume lock    
        vLckdErr            -46     Software volume lock    
        dupFNErr            -48     Duplicate filename and version  
        dirNFErrdirNFErr    -120    Directory not found or incomplete pathname  
        afpAccessDenied     -5000   User does not have the correct access   
        afpObjectTypeErr    -5025   A directory exists with that name   
*/

/*****************************************************************************/

EXTERN_API( OSErr )
FSpDirCreateCompat(
  const FSSpec *  spec,
  ScriptCode      scriptTag,
  long *          createdDirID);


/*
    The FSpDirCreateCompat function creates a new directory and returns the
    directory ID of the newDirectory.
    
    spec            input:  An FSSpec record specifying the directory to
                            create.
    scriptCode      input:  The code of the script system in which the
                            directory name is to be displayed.
    createdDirID    output: The directory ID of the directory that was
                            created.
    
    Result Codes
        noErr               0       No error    
        dirFulErr           -33     File directory full 
        dskFulErr           -34     Disk is full    
        nsvErr              -35     No such volume  
        ioErr               -36     I/O error   
        bdNamErr            -37     Bad filename    
        fnfErr              -43     Directory not found or incomplete pathname  
        wPrErr              -44     Hardware volume lock    
        vLckdErr            -46     Software volume lock    
        dupFNErr            -48     Duplicate filename and version  
        dirNFErrdirNFErr    -120    Directory not found or incomplete pathname  
        wrgVolTypErr        -123    Not an HFS volume   
        afpAccessDenied     -5000   User does not have the correct access   
*/

/*****************************************************************************/

EXTERN_API( OSErr )
FSpDeleteCompat(const FSSpec * spec);


/*
    The FSpDeleteCompat function deletes a file or directory.
    
    spec            input:  An FSSpec record specifying the file or 
                            directory to delete.
    
    Result Codes
        noErr               0       No error    
        nsvErr              -35     No such volume  
        ioErr               -36     I/O error   
        bdNamErr            -37     Bad filename    
        fnfErr              -43     File not found  
        wPrErr              -44     Hardware volume lock    
        fLckdErr            -45     File is locked  
        vLckdErr            -46     Software volume lock    
        fBsyErr             -47     File busy, directory not empty, or
                                    working directory control block open    
        dirNFErrdirNFErr    -120    Directory not found or incomplete pathname  
        afpAccessDenied     -5000   User does not have the correct access   
*/

/*****************************************************************************/

EXTERN_API( OSErr )
FSpGetFInfoCompat(
  const FSSpec *  spec,
  FInfo *         fndrInfo);


/*
    The FSpGetFInfoCompat function gets the finder information for a file.

    spec        input:  An FSSpec record specifying the file.
    fndrInfo    output: If the object is a file, then its FInfo.
    
    Result Codes
        noErr               0       No error    
        nsvErr              -35     No such volume  
        ioErr               -36     I/O error   
        bdNamErr            -37     Bad filename    
        fnfErr              -43     File not found  
        paramErr            -50     No default volume   
        dirNFErrdirNFErr    -120    Directory not found or incomplete pathname  
        afpAccessDenied     -5000   User does not have the correct access   
        afpObjectTypeErr    -5025   Directory not found or incomplete pathname  
    
    __________
    
    Also see:   FSpGetDInfo
*/

/*****************************************************************************/

EXTERN_API( OSErr )
FSpSetFInfoCompat(
  const FSSpec *  spec,
  const FInfo *   fndrInfo);


/*
    The FSpSetFInfoCompat function sets the finder information for a file.

    spec        input:  An FSSpec record specifying the file.
    fndrInfo    input:  The FInfo.
    
    Result Codes
        noErr               0       No error    
        nsvErr              -35     No such volume  
        ioErr               -36     I/O error   
        bdNamErr            -37     Bad filename    
        fnfErr              -43     File not found  
        wPrErr              -44     Hardware volume lock    
        fLckdErr            -45     File is locked  
        vLckdErr            -46     Software volume lock    
        dirNFErrdirNFErr    -120    Directory not found or incomplete pathname  
        afpAccessDenied     -5000   User does not have the correct access   
        afpObjectTypeErr    -5025   Object was a directory  
    
    __________
    
    Also see:   FSpSetDInfo
*/

/*****************************************************************************/

EXTERN_API( OSErr )
FSpSetFLockCompat(const FSSpec * spec);


/*
    The FSpSetFLockCompat function locks a file.

    spec        input:  An FSSpec record specifying the file.
    
    Result Codes
        noErr               0       No error    
        nsvErr              -35     No such volume  
        ioErr               -36     I/O error   
        fnfErr              -43     File not found  
        wPrErr              -44     Hardware volume lock    
        vLckdErr            -46     Software volume lock    
        dirNFErrdirNFErr    -120    Directory not found or incomplete pathname  
        afpAccessDenied     -5000   User does not have the correct access to
                                    the file    
        afpObjectTypeErr    -5025   Folder locking not supported by volume  
*/

/*****************************************************************************/

EXTERN_API( OSErr )
FSpRstFLockCompat(const FSSpec * spec);


/*
    The FSpRstFLockCompat function unlocks a file.

    spec        input:  An FSSpec record specifying the file.
    
    Result Codes
        noErr               0       No error    
        nsvErr              -35     No such volume  
        ioErr               -36     I/O error   
        fnfErr              -43     File not found  
        wPrErr              -44     Hardware volume lock    
        vLckdErr            -46     Software volume lock    
        dirNFErrdirNFErr    -120    Directory not found or incomplete pathname  
        afpAccessDenied     -5000   User does not have the correct access to
                                    the file    
        afpObjectTypeErr    -5025   Folder locking not supported by volume  
*/

/*****************************************************************************/

EXTERN_API( OSErr )
FSpRenameCompat(
  const FSSpec *     spec,
  ConstStr255Param   newName);


/*
    The FSpRenameCompat function renames a file or directory.

    spec        input:  An FSSpec record specifying the file.
    newName     input:  The new name of the file or directory.
    
    Result Codes
        noErr               0       No error    
        dirFulErr           -33     File directory full 
        dskFulErr           -34     Volume is full  
        nsvErr              -35     No such volume  
        ioErr               -36     I/O error   
        bdNamErr            -37     Bad filename    
        fnfErr              -43     File not found  
        wPrErr              -44     Hardware volume lock    
        fLckdErr            -45     File is locked  
        vLckdErr            -46     Software volume lock    
        dupFNErr            -48     Duplicate filename and version  
        paramErr            -50     No default volume   
        fsRnErr             -59     Problem during rename   
        dirNFErrdirNFErr    -120    Directory not found or incomplete pathname  
        afpAccessDenied     -5000   User does not have the correct access to
                                    the file    
*/

/*****************************************************************************/

EXTERN_API( OSErr )
FSpCatMoveCompat(
  const FSSpec *  source,
  const FSSpec *  dest);


/*
    The FSpCatMoveCompat function moves a file or directory to a different
    location on on the same volume.

    source      input:  An FSSpec record specifying the file or directory.
    dest        input:  An FSSpec record specifying the name and location
                        of the directory into which the source file or
                        directory is to be moved.
    
    Result Codes
        noErr               0       No error    
        nsvErr              -35     No such volume  
        ioErr               -36     I/O error   
        bdNamErr            -37     Bad filename or attempt to move into
                                    a file  
        fnfErr              -43     File not found  
        wPrErr              -44     Hardware volume lock    
        fLckdErr            -45     Target directory is locked  
        vLckdErr            -46     Software volume lock    
        dupFNErr            -48     Duplicate filename and version  
        paramErr            -50     No default volume   
        badMovErr           -122    Attempt to move into offspring  
        wrgVolTypErr        -123    Not an HFS volume   
        afpAccessDenied     -5000   User does not have the correct access to
                                    the file    
*/

/*****************************************************************************/

EXTERN_API( OSErr )
FSpExchangeFilesCompat(
  const FSSpec *  source,
  const FSSpec *  dest);


/*
    The FSpExchangeFilesCompat function swaps the data in two files by
    changing the information in the volume's catalog and, if the files
    are open, in the file control blocks.
    Differences from FSpExchangeFiles: Correctly exchanges files on volumes
    that don't support PBExchangeFiles. FSpExchangeFiles attempts to support
    volumes that don't support PBExchangeFiles, but in System 7, 7.0.1, 7.1,
    and 7 Pro, the compatibility code just doesn't work on volumes that
    don't support PBExchangeFiles (even though you may get a noErr result).
    System Update 3.0 and System 7.5 and later have the problems in
    FSpExchangeFiles corrected.
    
    Result Codes
        noErr               0       No error    
        nsvErr              -35     Volume not found    
        ioErr               -36     I/O error   
        fnfErr              -43     File not found  
        fLckdErr            -45     File is locked  
        vLckdErr            -46     Volume is locked or read-only   
        paramErr            -50     Function not supported by volume    
        volOfflinErr        -53     Volume is offline   
        wrgVolTypErr        -123    Not an HFS volume   
        diffVolErr          -1303   Files on different volumes  
        afpAccessDenied     -5000   User does not have the correct access   
        afpObjectTypeErr    -5025   Object is a directory, not a file   
        afpSameObjectErr    -5038   Source and destination files are the same   
*/

/*****************************************************************************/

EXTERN_API( short )
FSpOpenResFileCompat(
  const FSSpec *  spec,
  SignedByte      permission);


/*
    The FSpOpenResFileCompat function opens the resource file specified
    by spec.
    
    spec            input:  An FSSpec record specifying the file whose
                            resource file is to be opened.
    permission      input:  A constant indicating the desired file access
                            permissions.
    function result output: A resource file reference number, or if there's
                            an error -1.
    
    Result Codes
        noErr               0       No error
        nsvErr              �35     No such volume
        ioErr               �36     I/O error
        bdNamErr            �37     Bad filename or volume name (perhaps zero
                                    length)
        eofErr              �39     End of file
        tmfoErr             �42     Too many files open
        fnfErr              �43     File not found
        opWrErr             �49     File already open with write permission
        permErr             �54     Permissions error (on file open)
        extFSErr            �58     Volume belongs to an external file system
        memFullErr          �108    Not enough room in heap zone
        dirNFErr            �120    Directory not found
        mapReadErr          �199    Map inconsistent with operation
*/

/*****************************************************************************/

EXTERN_API( void )
FSpCreateResFileCompat(
  const FSSpec *  spec,
  OSType          creator,
  OSType          fileType,
  ScriptCode      scriptTag);


/*
    The FSpCreateResFileCompat function creates a new resource file with
    the specified type, creator, and script code.
    Differences from FSpCreateResFile: FSpCreateResFileCompat correctly
    sets the fdScript in the file's FXInfo record to scriptTag if the
    problem isn't fixed in the File Manager code.
    
    spec        input:  An FSSpec record specifying the resource file to create.
    creator     input:  The creator of the new file.
    fileType    input   The file type of the new file.
    scriptCode  input:  The code of the script system in which the file
                        name is to be displayed.
    
    Result Codes
        noErr               0       No error
        dirFulErr           �33     Directory full
        dskFulErr           �34     Disk full
        nsvErr              �35     No such volume
        ioErr               �36     I/O error
        bdNamErr            �37     Bad filename or volume name (perhaps zero
                                    length)
        tmfoErr             �42     Too many files open
        wPrErrw             �44     Disk is write-protected
        fLckdErr            �45     File is locked
*/

/*****************************************************************************/

#include "OptimizationEnd.h"

#if PRAGMA_STRUCT_ALIGN
    #pragma options align=reset
#elif PRAGMA_STRUCT_PACKPUSH
    #pragma pack(pop)
#elif PRAGMA_STRUCT_PACK
    #pragma pack()
#endif

#ifdef PRAGMA_IMPORT_OFF
#pragma import off
#elif PRAGMA_IMPORT
#pragma import reset
#endif

#ifdef __cplusplus
}
#endif

#endif /* __FSPCOMPAT__ */

