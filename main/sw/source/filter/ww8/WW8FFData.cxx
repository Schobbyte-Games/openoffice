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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include "WW8FFData.hxx"
#include <tools/stream.hxx>
#include <doc.hxx>
#include "writerwordglue.hxx"
#include "wrtww8.hxx"

namespace sw 
{

using sw::types::msword_cast;

WW8FFData::WW8FFData()
    : 
    mnType(0),
    mnResult(0),
    mbOwnHelp(false),
    mbOwnStat(false),
    mbProtected(false),
    mbSize(false),
    mnTextType(0),
    mbRecalc(false),
    mbListBox(false),
    mnMaxLen(0),
    mnCheckboxHeight(0),
    mnDefault(0)
{
}

WW8FFData::~WW8FFData()
{
}

void WW8FFData::setHelp(const ::rtl::OUString & rHelp)
{
    msHelp = rHelp;
    mbOwnHelp = true;
}
    
void WW8FFData::setStatus(const ::rtl::OUString & rStatus)
{
    msStatus = rStatus;
    mbOwnStat = true;
}

void WW8FFData::addListboxEntry(const ::rtl::OUString & rEntry)
{
    mbListBox = true;
    msListEntries.push_back(rEntry);
}

void WW8FFData::WriteOUString(SvStream * pDataStrm, const ::rtl::OUString & rStr,
    bool bAddZero)
{
    sal_uInt16 nStrLen = msword_cast<sal_uInt16>(rStr.getLength());
    *pDataStrm << nStrLen;
    SwWW8Writer::WriteString16(*pDataStrm, rStr, bAddZero);    
}

void WW8FFData::Write(SvStream * pDataStrm)
{
    sal_uLong nDataStt = pDataStrm->Tell();
    
    static const sal_uInt8 aHeader[] =
    {
        0,0,0,0,        // len of struct
        0x44,0,         // the start of "next" data
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // PIC
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0
    };

    pDataStrm->Write( aHeader, sizeof(aHeader) );

    sal_uInt8 aData[10] = {
        0xff, 0xff, 0xff, 0xff,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0
    };
    
    aData[4] = mnType | (mnResult << 2);
    
    if (mbOwnHelp)
        aData[4] |= (1 << 7);
        
    aData[5] = (mnTextType << 3);

    if (mbOwnStat)
        aData[5] |= 1;
        
    if (mbProtected)
        aData[5] |= (1 << 1);
            
    if (mbSize)
        aData[5] |= (1 << 2);
    
    if (mbRecalc)
        aData[5] |= (1 << 6);
    
    if (mbListBox)
        aData[5] |= (1 << 7);
            
    aData[6] = ::sal::static_int_cast<sal_uInt8>(mnMaxLen & 0xffff);
    aData[7] = ::sal::static_int_cast<sal_uInt8>(mnMaxLen >> 8);
    aData[8] = ::sal::static_int_cast<sal_uInt8>(mnCheckboxHeight & 0xffff);
    aData[9] = ::sal::static_int_cast<sal_uInt8>(mnCheckboxHeight >> 8);
    
    pDataStrm->Write(aData, sizeof(aData));
    
    WriteOUString(pDataStrm, msName, true);
    
    if (mnType == 0)
        WriteOUString(pDataStrm, msDefault, true);
    else
        *pDataStrm << mnDefault;
    
    WriteOUString(pDataStrm, msFormat, true);
    WriteOUString(pDataStrm, msHelp, true);
    WriteOUString(pDataStrm, msStatus, true);
    WriteOUString(pDataStrm, msMacroEnter, true);
    WriteOUString(pDataStrm, msMacroExit, true);
    
    if (mnType == 2)
    {
        sal_uInt8 aData1[2] = { 0xff, 0xff };
        pDataStrm->Write(aData1, sizeof(aData1));
        
        sal_uInt32 nListboxEntries = msListEntries.size();
        *pDataStrm << nListboxEntries;
        
        ::std::vector< ::rtl::OUString >::const_iterator aIt = msListEntries.begin();
        
        while (aIt != msListEntries.end())
        {
            const ::rtl::OUString & rEntry = *aIt;
            WriteOUString(pDataStrm, rEntry, false);
            
            aIt++;
        }
    }
    
    SwWW8Writer::WriteLong( *pDataStrm, nDataStt,
                           pDataStrm->Tell() - nDataStt );
}
    
}

