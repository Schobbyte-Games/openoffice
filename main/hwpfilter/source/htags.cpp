/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/



/* NAME
 * PURPOSE
 *
 * NOTES
 *
 * HISTORY
 *        frog - Aug 6, 1998: Created.
 */

#include "precompile.h"

#ifdef __GNUG__
#pragma implementation "htags.h"
#endif

#include <string.h>

#include "hwplib.h"
#include "hwpfile.h"
#include "htags.h"

bool HyperText::Read(HWPFile & hwpf)
{
    hwpf.Read1b(filename, 256);
    hwpf.Read2b(bookmark, 16);
    hwpf.Read1b(macro, 325);
    hwpf.Read1b(&type, 1);
    hwpf.Read1b(reserve, 3);
    if( type == 2 )
    {
        for( int i = 1; i < 256; i++)
        {
            filename[i-1] = filename[i];
            if( filename[i] == 0 )
                break;
        }
    }
    return true;
}


EmPicture::EmPicture(int tsize):size(tsize - 32)
{
    if (size <= 0)
        data = 0;
    else
        data = new uchar[size];
}
#ifdef WIN32
#define unlink _unlink
#endif
EmPicture::~EmPicture(void)
{
    if (data)
        delete[]data;
};

bool EmPicture::Read(HWPFile & hwpf)
{
    if (size <= 0)
        return false;
    hwpf.Read1b(name, 16);
    hwpf.Read1b(type, 16);
    name[0] = 'H';
    name[1] = 'W';
    name[2] = 'P';
    if (hwpf.ReadBlock(data, size) == 0)
        return false;
    return true;
}


OlePicture::OlePicture(int tsize)
{
    size = tsize - 4;
    if (size <= 0)
        return;
#ifdef WIN32
	 pis = 0L;
#else
	 pis = new char[size];
#endif
};

OlePicture::~OlePicture(void)
{
#ifdef WIN32
	 if( pis )
		  pis->Release();
#else
	 delete[] pis;
#endif
};

#define FILESTG_SIGNATURE_NORMAL 0xF8995568

bool OlePicture::Read(HWPFile & hwpf)
{
    if (size <= 0)
        return false;

// We process only FILESTG_SIGNATURE_NORMAL.
    hwpf.Read4b(&signature, 1);
    if (signature != FILESTG_SIGNATURE_NORMAL)
        return false;
#ifdef WIN32
	 char *data;
	 data = new char[size];
	 if( data == 0 || hwpf.ReadBlock(data,size) == 0 )
		  return false;
	 FILE *fp;
	 char tname[200];
	 wchar_t wtname[200];
	 tmpnam(tname);
     if (0 == (fp = fopen(tname, "wb")))
		  return false;
	 fwrite(data, size, 1, fp);
	 fclose(fp);
	 MultiByteToWideChar(CP_ACP, 0, tname, -1, wtname, 200);
	 if( StgOpenStorage(wtname, NULL,
					 STGM_READWRITE|STGM_SHARE_EXCLUSIVE|STGM_TRANSACTED,
					 NULL, 0, &pis) != S_OK ) {
		  pis = 0;
		  unlink(tname);
		  return false;
	 }
	 unlink(tname);
	 delete [] data;
#else
    if (pis == 0 || hwpf.ReadBlock(pis, size) == 0)
        return false;
#endif

    return true;
}
