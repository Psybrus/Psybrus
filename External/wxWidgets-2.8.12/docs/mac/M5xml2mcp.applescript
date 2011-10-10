-----------------------------------------------------------------------------
-- Name:        docs/mac/M5xml2mcp.applescript
-- Purpose:     Automatic import of CodeWarrior 5 xml files to projects
-- Author:      Gilles Depeyrot
-- Modified by:
-- Created:     30.11.2001
-- RCS-ID:      $Id: M5xml2mcp.applescript 12831 2001-12-02 20:02:17Z GD $
-- Copyright:   (c) 2001 Gilles Depeyrot
-- Licence:     wxWindows licence
-----------------------------------------------------------------------------
--
-- This AppleScript automatically recurses through the selected folder looking for
-- and importing CodeWarrior xml files to projects
-- To use this script, simply open it with the 'Script Editor' and run it.
--

--
-- Suffix used to recognize CodeWarrior xml files
--
property gXmlSuffix : "M5.xml"

--
-- Project and build success count
--
set theXmlCount to 0
set theXmlSuccessCount to 0

--
-- Ask the user to select the wxWindows samples folder
--
set theFolder to choose folder with prompt "Select the wxWindows folder"

ImportProjects(theFolder)

tell me to display dialog "Imported " & theXmlSuccessCount & " xml files out of " & theXmlCount buttons {"OK"}

--
-- ImportProjects
--
on ImportProjects(inFolder)
	global theXmlCount, theXmlSuccessCount
	
	tell application "Finder" to update inFolder
	
	try
		tell application "Finder" to set theXml to ((the first file of inFolder whose name ends with gXmlSuffix) as string)
	on error
		set theXml to ""
	end try
	
	if theXml is not "" then
		set theXmlCount to theXmlCount + 1
		
		-- save the current text delimiters
		set theDelimiters to my text item delimiters
		
		-- replace the ".xml" extension with ".mcp"
		set my text item delimiters to "."
		set theList to (every text item of theXml)
		set theList to (items 1 thru -2 of theList)
		set theImport to (theList as string) & ".mcp"
		
		-- restore the text delimiters
		set my text item delimiters to theDelimiters
		
		tell application "CodeWarrior IDE 4.0.4"
			--
			-- Import the selected xml file
			--
			try
				make new project document as theImport with data theXml
				set theXmlSuccessCount to theXmlSuccessCount + 1
				--
				-- Close the project
				--
				Close Project
			on error number errnum
				tell me to display dialog "Error " & errnum & " importing " & theXml & " to " & theImport
			end try
		end tell
	end if
	
	tell application "Finder" to set theSubFolders to every folder of inFolder whose name does not end with " Data"
	repeat with theFolder in theSubFolders
		ImportProjects(theFolder)
	end repeat
	
end ImportProjects
