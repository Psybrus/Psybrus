/**************************************************************************
*
* File:		RsFont.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Font class
*		
*
*
* 
**************************************************************************/

#include "RsFont.h"
#include "RsMaterial.h"
#include "BcString.h"
#include "BcUTF8.h"

//////////////////////////////////////////////////////////////////////////
// Constructor
RsFont::RsFont()
{

}

//////////////////////////////////////////////////////////////////////////
// Destructor
RsFont::~RsFont()
{

}

//////////////////////////////////////////////////////////////////////////
// drawText
BcReal RsFont::drawText( const RsFontStyle& Style, const BcVec2d& Position, const BcVec2d& Dimensions, const BcU32 SortOrder, const BcChar* pText, ... )
{
	va_list VAHead;
	va_start( VAHead, pText );
	unpackText( pText, BcStrLength( pText ), VAHead );
	va_end( VAHead );

	return internalDraw( Style, Position, Dimensions, SortOrder);
}

//////////////////////////////////////////////////////////////////////////
// unpackText
void RsFont::unpackText( const BcChar* pText, BcU32 Length, va_list Args )
{
	nPrintedChars_ = 0;

	while( Length > 0 )
	{
		RsFontCharIdx& PrintedChar = PrintedChars_[ nPrintedChars_++ ];
		BcU32 nChars;
		BcU32 Char;

		// Decode the character
		nChars = BcUTF8::toUCS4( pText, Char );
		Length -= nChars;
		pText += nChars;

		if ( Char == '\\' )
		{
			nChars = BcUTF8::toUCS4( pText, Char );
			Length -= nChars;
			pText += nChars;
			
			if( Char == 'n' )
			{
				PrintedChar.Type_ = rsFCT_NEWLINE;
				PrintedChar.Char_ = 0;
			}
		}
		/*else if ( Char == '%' )
		{
			nChars = BcUTF8::toUCS4( pText, Char );
			Length -= nChars;
			pText += nChars;

			// TODO: Valist unpacking.            
		}*/
		else if ( Char == '\r' )
		{
			PrintedChar.Type_ = rsFCT_LETTER;
			PrintedChar.Char_ = ' ';
		}
		else if ( Char == '\n' )
		{
			PrintedChar.Type_ = rsFCT_NEWLINE;
			PrintedChar.Char_ = 0;
		}
		else if ( Char == '	' )
		{
			PrintedChar.Type_ = rsFCT_LETTER;
			PrintedChar.Char_ = ' ';
		}
		else
		{
			PrintedChar.Type_ = rsFCT_LETTER;
            PrintedChar.Char_ = Char;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// lineWidth
BcReal RsFont::lineWidth( BcU32 iFirstChar )
{
	BcReal Width = 0;

	for( BcU32 i = iFirstChar; i < nPrintedChars_; ++i )
	{
		// Stop at end or on newline.
		RsFontCharIdx& PrintedChar = PrintedChars_[ i ];

		//
		if( PrintedChar.Type_ == rsFCT_LETTER )
		{
			const RsFontChar& Character = CharacterMap_[ PrintedChar.Char_ ];
			Width += Character.XAdv_;
		}
		else
		{
			break;
		}
	}
	
	return Width;
}

//////////////////////////////////////////////////////////////////////////
// breakUpLines
void RsFont::breakUpLines( BcReal MaxWidth )
{
	BcReal Width = 0;

	BcU32 LastSpaceIndex = BcErrorCode;

	for( BcU32 i = 0; i < nPrintedChars_; ++i )
	{
		// Stop at end or on newline.
		RsFontCharIdx& PrintedChar = PrintedChars_[ i ];

		// Work out if we should break the line up or not.
		if( PrintedChar.Type_ == rsFCT_LETTER )
		{
			// If we have a point to insert a newline and we need to,
			// do it.
			const RsFontChar& Character = CharacterMap_[ PrintedChar.Char_ ];
			Width += Character.XAdv_;

			if( Width > MaxWidth )
			{
				if( LastSpaceIndex != BcErrorCode )
				{
					PrintedChars_[ LastSpaceIndex ].Type_ = rsFCT_NEWLINE;
					PrintedChars_[ LastSpaceIndex ].Char_ = 0;

					LastSpaceIndex = BcErrorCode;
					Width = 0.0f;

					// Goto where we inserted the newline
					i = LastSpaceIndex + 1;
				}
			}
			else if ( PrintedChar.Char_ == ' ' )
			{
				LastSpaceIndex = i;
			}
		}
		else if( PrintedChar.Type_ == rsFCT_NEWLINE )
		{
			LastSpaceIndex = BcErrorCode;
			Width = 0.0f;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// textHeight
BcReal RsFont::textHeight( const BcVec2d& Dimensions, const BcChar* pText, ... )
{
	va_list VAHead;
	va_start( VAHead, pText );
	unpackText( pText, BcStrLength( pText ), VAHead );
	va_end( VAHead );

	//
	BcVec2d CurrPosition( 0.0f, 0.0f );

	//
	if( Dimensions.x() > 0.0f )
	{
		breakUpLines( Dimensions.x() );
	}

	// Draw all characters
	for( BcU32 i = 0; i < nPrintedChars_; ++i )
	{
		switch( PrintedChars_[ i ].Type_ )
		{
		case rsFCT_LETTER:
			{
				// Find our character
				const RsFontChar& Character = CharacterMap_[ PrintedChars_[ i ].Char_ ];
				//

				CurrPosition.x( CurrPosition.x() + ( Character.XAdv_ ) );
			}
			break;

		case rsFCT_NEWLINE:
			{
				CurrPosition.y( CurrPosition.y() + pFileData_->LineHeight_ );
			}
			break;
		}
	}

	//
	return CurrPosition.y() + pFileData_->LineHeight_;
}

//////////////////////////////////////////////////////////////////////////
// internalDraw
//virtual
BcReal RsFont::internalDraw( const RsFontStyle& Style, const BcVec2d& Position, const BcVec2d& Dimensions, const BcU32 Layer )
{
	/*
	BcReal MaxLineWidth = 0;
	BcVec2d CurrPosition;
	BcU32 CurrentPage = 0;

	CsResourceHandle< RsTexture > Texture;

	//
	startDraw( Style, nPrintedChars_ );

	//
	if( Dimensions.x() > 0.0f )
	{
		breakUpLines( Dimensions.x() );
	}

	// Setup starting alignment
	BcReal LineWidth = lineWidth( 0 );
	MaxLineWidth = LineWidth;

	switch( Style.hAlign() )
	{
	case RsFontStyle::HALIGN_LEFT:
		CurrPosition.x( Position.x() );
		CurrPosition.y( Position.y() );
		break;

	case RsFontStyle::HALIGN_RIGHT:
		CurrPosition.x( Position.x() + ( Dimensions.x() - LineWidth ) );
		CurrPosition.y( Position.y() );
		break;

	case RsFontStyle::HALIGN_CENTRE:
		CurrPosition.x( ( ( (Position.x()*2) + Dimensions.x() ) - LineWidth ) * 0.5f );
		CurrPosition.y( Position.y() );
		break;
	}

	// Draw all characters
	for( BcU32 i = 0; i < nPrintedChars_; ++i )
	{
		switch( PrintedChars_[ i ].Type_ )
		{
		case rsFCT_LETTER:
			{
				// Find our character
				const RsFontChar& Character = CharacterMap_[ PrintedChars_[ i ].Char_ ];

				// If we are using a different material for this character, submit the
				// last lot.
				if( Character.Page_ != CurrentPage )
				{
					// Submit only if the texture is valid.
					if( aMaterials_[ CurrentPage ]->getTexture( rsTT_DIFFUSE, Texture ) )
					{
						submitCharacters( Layer, Texture );
					}
					
					CurrentPage = Character.Page_;
				}
				
				// Make the current character
				makeCharacter( Character, CurrPosition );
				
				//
				CurrPosition.x( CurrPosition.x() + ( Character.XAdv_ ) );
			}
			break;

		case rsFCT_NEWLINE:
			{
				// Set new line position.
				BcReal LineWidth = lineWidth( i + 1 );
				if(LineWidth > MaxLineWidth)
				{
					MaxLineWidth = LineWidth;
				}

				switch( Style.hAlign() )
				{
				case RsFontStyle::HALIGN_LEFT:
					CurrPosition.x( Position.x() );
					CurrPosition.y( CurrPosition.y() + pFileData_->LineHeight_ );
					break;

				case RsFontStyle::HALIGN_RIGHT:
					CurrPosition.x( Position.x() + ( Dimensions.x() - LineWidth ) );
					CurrPosition.y( CurrPosition.y() + pFileData_->LineHeight_ );
					break;

				case RsFontStyle::HALIGN_CENTRE:
					CurrPosition.x( ( ( (Position.x()*2) + Dimensions.x() ) - LineWidth ) * 0.5f );
					CurrPosition.y( CurrPosition.y() + pFileData_->LineHeight_ );
					break;
				}
			}
			break;
		}
	}

	// Submit remaining characters.
	if( aMaterials_[ CurrentPage ]->getTexture( rsTT_DIFFUSE, Texture ) )
	{
		submitCharacters( Layer, Texture );
	}
	return MaxLineWidth;
	 */
	
}
