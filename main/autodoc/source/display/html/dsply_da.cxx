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

#include <precomp.h>
#include "dsply_da.hxx"


// NOT FULLY DEFINED SERVICES
#include <cosv/tpl/tpltools.hxx>
#include <ary/cpp/c_gate.hxx>
#include <ary/cpp/c_vari.hxx>
#include <ary/doc/d_docu.hxx>
#include <ary/loc/loc_file.hxx>
#include <ary/loc/locp_le.hxx>
#include <udm/html/htmlitem.hxx>
#include "hd_docu.hxx"
#include "hdimpl.hxx"
#include "html_kit.hxx"
#include "opageenv.hxx"
#include "pagemake.hxx"


using namespace csi;



DataDisplay::DataDisplay( OuputPage_Environment & io_rEnv )
    :   aMap_GlobalDataDisplay(),
        pClassMembersDisplay(0),
        pEnv( &io_rEnv ),
        pDocuShow( new Docu_Display(io_rEnv) )
{
}

DataDisplay::~DataDisplay()
{
    csv::erase_map_of_heap_ptrs( aMap_GlobalDataDisplay );
}

void
DataDisplay::PrepareForConstants()
{
    if (pClassMembersDisplay)
        pClassMembersDisplay = 0;

    csv::erase_map_of_heap_ptrs( aMap_GlobalDataDisplay );
}

void
DataDisplay::PrepareForVariables()
{
    // Doesn't need to do anything yet.
}

void
DataDisplay::PrepareForStdMembers()
{
    csv::erase_map_of_heap_ptrs( aMap_GlobalDataDisplay );

    pClassMembersDisplay = new PageDisplay(*pEnv);
    const ary::cpp::Class * pClass = pEnv->CurClass();
    csv_assert( pClass != 0 );
    pClassMembersDisplay->Setup_DataFile_for(*pClass);
}

void
DataDisplay::PrepareForStaticMembers()
{
    // Doesn't need to do anything yet.
}

void
DataDisplay::Create_Files()
{
    if (pClassMembersDisplay)
    {
        pClassMembersDisplay->Create_File();
        pClassMembersDisplay = 0;
    }
    else
    {
     	for ( Map_FileId2PagePtr::const_iterator it = aMap_GlobalDataDisplay.begin();
              it != aMap_GlobalDataDisplay.end();
              ++it )
        {
         	(*it).second->Create_File();
        }
        csv::erase_map_of_heap_ptrs( aMap_GlobalDataDisplay );
    }
}

void
DataDisplay::do_Process( const ary::cpp::Variable & i_rData )
{
    if ( Ce_IsInternal(i_rData) )
        return;

    PageDisplay & rPage = FindPage_for( i_rData );

    csi::xml::Element & rOut = rPage.CurOut();
    Display_SglDatum( rOut, i_rData );
}

const ary::cpp::Gate *
DataDisplay::inq_Get_ReFinder() const
{
    return & pEnv->Gate();
}

PageDisplay &
DataDisplay::FindPage_for( const ary::cpp::Variable & i_rData )
{
    if ( pClassMembersDisplay )
        return *pClassMembersDisplay;

    SourceFileId
            nSourceFile = i_rData.Location();
    PageDisplay *
            pFound = csv::value_from_map( aMap_GlobalDataDisplay, nSourceFile, (PageDisplay*)0 );
    if ( pFound == 0 )
    {
     	pFound = new PageDisplay( *pEnv );
        const ary::loc::File &
                rFile = pEnv->Gate().Locations().Find_File( nSourceFile );
        pFound->Setup_DataFile_for(rFile);
        aMap_GlobalDataDisplay[nSourceFile] = pFound;
    }

    return *pFound;
}

void
DataDisplay::Display_SglDatum( csi::xml::Element &        rOut,
                               const ary::cpp::Variable & i_rData )
{
    adcdisp::ExplanationList aDocu(rOut, true);
    aDocu.AddEntry( 0 );

    aDocu.Term()
        >> *new html::Label( DataLabel(i_rData.LocalName()) )
            << " ";
    aDocu.Term()
        << i_rData.LocalName();

    dshelp::Get_LinkedTypeText( aDocu.Def(), *pEnv, i_rData.Type() );
    aDocu.Def()
        << " "
        >> *new html::Strong
            << i_rData.LocalName();
    if ( i_rData.ArraySize().length() > 0 )
    {
        aDocu.Def()
            << "["
            << i_rData.ArraySize()
            << "]";
    }
    if ( i_rData.Initialisation().length() > 0 )
    {
        aDocu.Def()
            << " = "
            << i_rData.Initialisation();
    }
    aDocu.Def()
        << ";"
        << new html::LineBreak
        << new html::LineBreak;

    aDocu.AddEntry_NoTerm();

    pDocuShow->Assign_Out(aDocu.Def());
    pDocuShow->Process(i_rData.Docu());
    pDocuShow->Unassign_Out();

    rOut << new html::HorizontalLine;
}
