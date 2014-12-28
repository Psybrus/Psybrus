#include "DsTemplate.h"
#include "Base/BcFile.h"
#include "rapidxml.hpp"
#include "Base/BcLog.h"
using namespace rapidxml;
BcHtmlNode DsTemplate::loadTemplate( BcHtmlNode node, std::string filename )
{
	char* out = loadTemplateFile( filename );


	xml_document<> doc;
	try
	{
		doc.parse< 0 >( &out[ 0 ] );
	}
	catch ( rapidxml::parse_error &e )
	{
		char buffer[ 64 ];
		
		/*sprintf_s( buffer, 200, "%s", e.what() );
		sprintf_s( buffer, 200, "%s", e.where<char>() );

		BcLog::pImpl()->write(buffer);/**/
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
char* DsTemplate::loadTemplateFile( std::string filename )
{
	BcFile file;
	std::string f = filename;
	file.open( f.c_str() );
	if ( !file.isOpen() )
		return 0;
	char* data;// = new BcU8[file.size()];
	data = ( char* ) file.readAllBytes();
	std::string output = data;

	return data;
}
