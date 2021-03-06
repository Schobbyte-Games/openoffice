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
#include "precompiled_xmloff.hxx"

#include "XFormsModelContext.hxx"

#include "XFormsBindContext.hxx"
#include "XFormsSubmissionContext.hxx"
#include "XFormsInstanceContext.hxx"
#include "SchemaContext.hxx"
#include "xformsapi.hxx"

#include <xmloff/xmlimp.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmltoken.hxx>
#include "xmloff/xmlerror.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/util/XUpdatable.hpp>
#include <com/sun/star/xforms/XModel.hpp>

#include <tools/debug.hxx>


using rtl::OUString;
using com::sun::star::xml::sax::XAttributeList;
using com::sun::star::beans::XPropertySet;
using com::sun::star::util::XUpdatable;
using namespace com::sun::star::uno;
using namespace xmloff::token;




static SvXMLTokenMapEntry aAttributes[] =
{
    TOKEN_MAP_ENTRY( NONE, ID ),
    TOKEN_MAP_ENTRY( NONE, SCHEMA ),
    XML_TOKEN_MAP_END
};

static SvXMLTokenMapEntry aChildren[] =
{
    TOKEN_MAP_ENTRY( XFORMS, INSTANCE ),
    TOKEN_MAP_ENTRY( XFORMS, BIND ),
    TOKEN_MAP_ENTRY( XFORMS, SUBMISSION ),
    TOKEN_MAP_ENTRY( XSD,    SCHEMA ),
    XML_TOKEN_MAP_END
};


XFormsModelContext::XFormsModelContext( SvXMLImport& rImport, 
                                        sal_uInt16 nPrefix,
                                        const OUString& rLocalName ) :
    TokenContext( rImport, nPrefix, rLocalName, aAttributes, aChildren ),
    mxModel( lcl_createXFormsModel() )
{
}

XFormsModelContext::~XFormsModelContext()
{
}


Reference<XPropertySet> XFormsModelContext::getModel()
{
    return mxModel;
}


void XFormsModelContext::HandleAttribute( 
    sal_uInt16 nToken, 
    const OUString& rValue )
{
    switch( nToken )
    {
    case XML_ID:
        mxModel->setPropertyValue( OUSTRING("ID"), makeAny( rValue ) );
        break;
    case XML_SCHEMA:
        GetImport().SetError( XMLERROR_XFORMS_NO_SCHEMA_SUPPORT );
        break;
    default:
        DBG_ERROR( "this should not happen" );
        break;
    }
}        

SvXMLImportContext* XFormsModelContext::HandleChild( 
    sal_uInt16 nToken,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList>& )
{
    SvXMLImportContext* pContext = NULL;

    switch( nToken )
    {
    case XML_INSTANCE:
        pContext = new XFormsInstanceContext( GetImport(), nPrefix, rLocalName,
                                              mxModel );
        break;
    case XML_BIND:
        pContext = new XFormsBindContext( GetImport(), nPrefix, rLocalName, 
                                          mxModel );
        break;
    case XML_SUBMISSION:
        pContext = new XFormsSubmissionContext( GetImport(), nPrefix, 
                                                rLocalName, mxModel );
        break;
    case XML_SCHEMA:
        pContext = new SchemaContext( 
            GetImport(), nPrefix, rLocalName,
            Reference<com::sun::star::xforms::XModel>( mxModel, 
                                                       UNO_QUERY_THROW )
                ->getDataTypeRepository() );
        break;
    default:
        DBG_ERROR( "Boooo!" );
        break;
    }

    return pContext;
}

void XFormsModelContext::EndElement()
{
    // update before putting model into document
    Reference<XUpdatable> xUpdate( mxModel, UNO_QUERY );
    if( xUpdate.is() )
        xUpdate->update();

    GetImport().initXForms();
    lcl_addXFormsModel( GetImport().GetModel(), getModel() );
}
