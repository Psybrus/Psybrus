#include "DsTemplate.h"
#include "System/Os/OsCore.h"
#include "Base/BcFile.h"
#include "rapidxml.hpp"
#include "Base/BcLog.h"
using namespace rapidxml;
BcHtmlNode DsTemplate::loadTemplate( BcHtmlNode node, std::string filename )
{
	std::string out = loadTemplateFile( filename );

	if ( out == "" )
	{
		BcHtmlNode out = node.createChildNode( "div" );
		out.setAttribute( "id", "error" );
		out.setContents( "Could not load template file: " + filename );

		return out;
	}
	xml_document<> doc;
	try
	{
		doc.parse< 0 >( &out[ 0 ] );
	}
	catch ( rapidxml::parse_error &e )
	{
		char buffer[ 64 ];
		BcPrintf( "Error in file %s: %s\n   Error at %d\n", filename.c_str(), e.what(), ( int ) ( e.where<char>() - out.c_str() ) );

	}

	xml_node<> *pRoot = doc.first_node();
	return loadNode( node, pRoot );
}

BcHtmlNode DsTemplate::loadNode( BcHtmlNode parent, void* xmlNode )
{
	xml_node<> *pNode = ( xml_node<>* )xmlNode;
	BcHtmlNode node = parent.createChildNode( pNode->name() );

	if ( node.getTag() == "" )
	{
		node.setContents( pNode->value( ) );
		return node;
	}

	xml_attribute<> *pAttr = pNode->first_attribute( );
	while ( pAttr != nullptr )
	{
		node.setAttribute( pAttr->name(), pAttr->value() );
		pAttr = pAttr->next_attribute();
	}

	xml_node<> *pChild = pNode->first_node();
	while ( pChild != nullptr )
	{
		loadNode( node, pChild );
		pChild = pChild->next_sibling();
	}
	return node;

}

////////////////////////////////////////////////////////////////////////// 
// Gets a plain text file
std::string DsTemplate::loadTemplateFile( std::string filename )
{
	BcFile file;
	std::string f = filename;
	file.open( f.c_str() );
	BcPrintf("Loading file: %s (size: %d)\n", filename.c_str(), file.size());
	if ( !file.isOpen() )
		return "";
	char* data = new char[file.size() + 1];

	BcMemZero(data, file.size() + 1);
	file.read(data, file.size());
	std::string output = data;
	delete data;
	return output;
}
