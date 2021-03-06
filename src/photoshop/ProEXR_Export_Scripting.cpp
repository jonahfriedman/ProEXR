/* ---------------------------------------------------------------------
// 
// ProEXR - OpenEXR plug-ins for Photoshop and After Effects
// Copyright (c) 2007-2017,  Brendan Bolles, http://www.fnordware.com
// 
// This file is part of ProEXR.
//
// ProEXR is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
// 
// -------------------------------------------------------------------*/

#include "PIDefines.h"
#include "ProEXR.h"

#include "ProEXR_Terminology.h"

#define gStuff				(globals->exportParamBlock)
		

enum
{
    KEY_NO_COMPRESSION  = 0,	// no compression
    KEY_RLE_COMPRESSION,	// run length encoding
    KEY_ZIPS_COMPRESSION,	// zlib compression, one scan line at a time
    KEY_ZIP_COMPRESSION,	// zlib compression, in blocks of 16 scan lines
    KEY_PIZ_COMPRESSION,	// piz-based wavelet compression
    KEY_PXR24_COMPRESSION,	// lossy 24-bit float compression
	KEY_B44_COMPRESSION,	// lossy 16-bit float compression
	KEY_B44A_COMPRESSION,	// B44 with region squeeze
	KEY_DWAA_COMPRESSION,	// DCT, 32 scanlines
	KEY_DWAB_COMPRESSION,	// DCT, 256 scanlines

    KEY_NUM_COMPRESSION_METHODS	// number of different compression methods
};

static A_u_char KeyToCompression(OSType key)
{
	return	(key == compressionNone)	? KEY_NO_COMPRESSION :
			(key == compressionRLE)		? KEY_RLE_COMPRESSION :
			(key == compressionZip)		? KEY_ZIPS_COMPRESSION :
			(key == compressionZip16)	? KEY_ZIP_COMPRESSION :
			(key == compressionPiz)		? KEY_PIZ_COMPRESSION :
			(key == compressionPXR24)	? KEY_PXR24_COMPRESSION :
			(key == compressionB44)		? KEY_B44_COMPRESSION :
			(key == compressionB44A)	? KEY_B44A_COMPRESSION :
			(key == compressionDWAA)	? KEY_DWAA_COMPRESSION :
			(key == compressionDWAB)	? KEY_DWAB_COMPRESSION :
			KEY_PIZ_COMPRESSION;
}


Boolean ReadScriptParamsOnWrite (ExGPtr globals)
{
	PIReadDescriptor			token = NULL;
	DescriptorKeyID				key = 0;
	DescriptorTypeID			type = 0;
	DescriptorKeyIDArray		array = { NULLID };
	int32						flags = 0;
	OSErr						stickyError = noErr;
	Boolean						returnValue = true;
	DescriptorEnumID			ostypeStoreVelue;
	Boolean						boolStoreValue;
	
	if (DescriptorAvailable(NULL))
	{ /* playing back.  Do our thing. */
		token = OpenReader(array);
		if (token)
		{
			while (PIGetKey(token, &key, &type, &flags))
			{
				switch (key)
				{
					case keyIn:
							PIGetAlias(token, (Handle*)&gAliasHandle);
							break;

					case keyEXRcompression:
							PIGetEnum(token, &ostypeStoreVelue);
							gOptions.compression_type = KeyToCompression(ostypeStoreVelue);
							break;
					
					case keyEXRfloat:
							PIGetBool(token, &boolStoreValue);
							gOptions.float_not_half = boolStoreValue;
							break;

					case keyEXRlumichrom:
							PIGetBool(token, &boolStoreValue);
							gOptions.luminance_chroma = boolStoreValue;
							break;

					case keyEXRcomposite:
							PIGetBool(token, &boolStoreValue);
							gOptions.layer_composite = boolStoreValue;
							break;

					case keyEXRhidden:
							PIGetBool(token, &boolStoreValue);
							gOptions.hidden_layers = boolStoreValue;
							break;
				}
			}

			stickyError = CloseReader(&token); // closes & disposes.
				
			if (stickyError)
			{
				if (stickyError == errMissingParameter) // missedParamErr == -1715
					;
					/* (descriptorKeyIDArray != NULL)
					   missing parameter somewhere.  Walk IDarray to find which one. */
				else
					gResult = stickyError;
			}
		}
		
		returnValue = PlayDialog();
		/* return TRUE if want to show our Dialog */		
	}
	
	return returnValue;
}



static OSType CompressionToKey(A_u_char compression)
{
	return	(compression == KEY_NO_COMPRESSION)		? compressionNone :
			(compression == KEY_RLE_COMPRESSION)	? compressionRLE :
			(compression == KEY_ZIPS_COMPRESSION)	? compressionZip :
			(compression == KEY_ZIP_COMPRESSION)	? compressionZip16 :
			(compression == KEY_PIZ_COMPRESSION)	? compressionPiz :
			(compression == KEY_PXR24_COMPRESSION)	? compressionPXR24 :
			(compression == KEY_B44_COMPRESSION)	? compressionB44 :
			(compression == KEY_B44A_COMPRESSION)	? compressionB44A :
			(compression == KEY_DWAA_COMPRESSION)	? compressionDWAA :
			(compression == KEY_DWAB_COMPRESSION)	? compressionDWAB :
			0;
}

OSErr WriteScriptParamsOnWrite (ExGPtr globals)
{
	PIWriteDescriptor			token = nil;
	OSErr						gotErr = noErr;
			
	if (DescriptorAvailable(NULL))
	{ /* recording.  Do our thing. */
		token = OpenWriter();
		
		if (token)
		{
			// write keys here
			if(gAliasHandle)
				PIPutAlias(token, keyIn, (Handle)gAliasHandle);
				
			PIPutEnum(token, keyEXRcompression, typeCompression, CompressionToKey(gOptions.compression_type) );
			PIPutBool(token, keyEXRfloat, gOptions.float_not_half);
			PIPutBool(token, keyEXRlumichrom, gOptions.luminance_chroma);
			PIPutBool(token, keyEXRcomposite, gOptions.layer_composite);
			PIPutBool(token, keyEXRhidden, gOptions.hidden_layers);
			gotErr = CloseWriter(&token); /* closes and sets dialog optional */
			/* done.  Now pass handle on to Photoshop */
		}
	}
	return gotErr;
}

