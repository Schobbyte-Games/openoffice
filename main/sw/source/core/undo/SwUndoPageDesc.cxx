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

#include <tools/resid.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <swundo.hxx>
#include <pagedesc.hxx>
#include <SwUndoPageDesc.hxx>
#include <SwRewriter.hxx>
#include <undobj.hxx>
#include <comcore.hrc>
#include <fmtcntnt.hxx>
#include <fmthdft.hxx>

#ifdef DEBUG
#include <ndindex.hxx>
#endif


#ifdef DEBUG
// Pure debug help function to have a quick look at the header/footer attributes.
void DebugHeaderFooterContent( const SwPageDesc& rPageDesc )
{
    sal_uLong nHeaderMaster = ULONG_MAX;
    sal_uLong nHeaderLeft = ULONG_MAX;
    sal_uLong nFooterMaster = ULONG_MAX;
    sal_uLong nFooterLeft = ULONG_MAX;
    int nHeaderCount = 0;
    int nLeftHeaderCount = 0;
    int nFooterCount = 0;
    int nLeftFooterCount = 0;
    bool bSharedHeader = false;
    bool bSharedFooter = false;

    SwFmtHeader& rHead = (SwFmtHeader&)rPageDesc.GetMaster().GetHeader();
    SwFmtFooter& rFoot = (SwFmtFooter&)rPageDesc.GetMaster().GetFooter();
    SwFmtHeader& rLeftHead = (SwFmtHeader&)rPageDesc.GetLeft().GetHeader();
    SwFmtFooter& rLeftFoot = (SwFmtFooter&)rPageDesc.GetLeft().GetFooter();
    if( rHead.IsActive() )
    {
        SwFrmFmt* pHeaderFmt = rHead.GetHeaderFmt();
        if( pHeaderFmt )
        {
            nHeaderCount = pHeaderFmt->GetClientCount();
            const SwFmtCntnt* pCntnt = &pHeaderFmt->GetCntnt();
            if( pCntnt->GetCntntIdx() )
                nHeaderMaster = pCntnt->GetCntntIdx()->GetIndex();
            else
                nHeaderMaster = 0;
        }
        bSharedHeader = rPageDesc.IsHeaderShared();
        SwFrmFmt* pLeftHeaderFmt = rLeftHead.GetHeaderFmt();
        if( pLeftHeaderFmt )
        {
            nLeftHeaderCount = pLeftHeaderFmt->GetClientCount();
            const SwFmtCntnt* pLeftCntnt = &pLeftHeaderFmt->GetCntnt();
            if( pLeftCntnt->GetCntntIdx() )
                nHeaderLeft = pLeftCntnt->GetCntntIdx()->GetIndex();
            else
                nHeaderLeft = 0;
        }
    }
    if( rFoot.IsActive() )
    {
        SwFrmFmt* pFooterFmt = rFoot.GetFooterFmt();
        if( pFooterFmt )
        {
            nFooterCount = pFooterFmt->GetClientCount();
            const SwFmtCntnt* pCntnt = &pFooterFmt->GetCntnt();
            if( pCntnt->GetCntntIdx() )
                nFooterMaster = pCntnt->GetCntntIdx()->GetIndex();
            else
                nFooterMaster = 0;
        }
        bSharedFooter = rPageDesc.IsFooterShared();
        SwFrmFmt* pLeftFooterFmt = rLeftFoot.GetFooterFmt();
        if( pLeftFooterFmt )
        {
            nLeftFooterCount = pLeftFooterFmt->GetClientCount();
            const SwFmtCntnt* pLeftCntnt = &pLeftFooterFmt->GetCntnt();
            if( pLeftCntnt->GetCntntIdx() )
                nFooterLeft = pLeftCntnt->GetCntntIdx()->GetIndex();
            else
                nFooterLeft = 0;
        }
    }
    int i = 0;
    ++i; // To set a breakpoint
}
#endif

SwUndoPageDesc::SwUndoPageDesc(const SwPageDesc & _aOld,
                               const SwPageDesc & _aNew,
                               SwDoc * _pDoc)
    : SwUndo( _aOld.GetName() != _aNew.GetName() ?
              UNDO_RENAME_PAGEDESC :
              UNDO_CHANGE_PAGEDESC ),
      aOld(_aOld, _pDoc), aNew(_aNew, _pDoc), pDoc(_pDoc), bExchange( false )
{
    ASSERT(0 != pDoc, "no document?");

#ifdef DEBUG
    DebugHeaderFooterContent( (SwPageDesc&)aOld );
    DebugHeaderFooterContent( (SwPageDesc&)aNew );
#endif

    /*
    The page description changes.
    If there are no header/footer content changes like header on/off or change from shared content
    to unshared etc., there is no reason to duplicate the content nodes (Crash i55547)
    But this happens, this Undo Ctor will destroy the unnecessary duplicate and manipulate the
    content pointer of the both page descriptions.
    */
    SwPageDesc &rOldDesc = (SwPageDesc&)aOld;
    SwPageDesc &rNewDesc = (SwPageDesc&)aNew;
    const SwFmtHeader& rOldHead = rOldDesc.GetMaster().GetHeader();
    const SwFmtHeader& rNewHead = rNewDesc.GetMaster().GetHeader();
    const SwFmtFooter& rOldFoot = rOldDesc.GetMaster().GetFooter();
    const SwFmtFooter& rNewFoot = rNewDesc.GetMaster().GetFooter();
    /* bExchange must not be set, if the old page descriptor will stay active.
    Two known situations:
    #i67735#: renaming a page descriptor
    #i67334#: changing the follow style
    If header/footer will be activated or deactivated, this undo will not work.
    */
    bExchange = ( aOld.GetName() == aNew.GetName() ) &&
        ( _aOld.GetFollow() == _aNew.GetFollow() ) &&
        ( rOldHead.IsActive() == rNewHead.IsActive() ) &&
        ( rOldFoot.IsActive() == rNewFoot.IsActive() );
    if( rOldHead.IsActive() && ( rOldDesc.IsHeaderShared() != rNewDesc.IsHeaderShared() ) )
        bExchange = false;
    if( rOldFoot.IsActive() && ( rOldDesc.IsFooterShared() != rNewDesc.IsFooterShared() ) )
        bExchange = false;
    if( bExchange )
    {
        if( rNewHead.IsActive() )
        {
            SwFrmFmt* pFormat = new SwFrmFmt( *rNewHead.GetHeaderFmt() );
            // The Ctor of this object will remove the duplicate!
            SwFmtHeader aFmtHeader( pFormat );
            if( !rNewDesc.IsHeaderShared() )
            {
                pFormat = new SwFrmFmt( *rNewDesc.GetLeft().GetHeader().GetHeaderFmt() );
                // The Ctor of this object will remove the duplicate!
                SwFmtHeader aFormatHeader( pFormat );
            }
        }
        // Same procedure for footers...
        if( rNewFoot.IsActive() )
        {
            SwFrmFmt* pFormat = new SwFrmFmt( *rNewFoot.GetFooterFmt() );
            // The Ctor of this object will remove the duplicate!
            SwFmtFooter aFmtFooter( pFormat );
            if( !rNewDesc.IsFooterShared() )
            {
                pFormat = new SwFrmFmt( *rNewDesc.GetLeft().GetFooter().GetFooterFmt() );
                // The Ctor of this object will remove the duplicate!
                SwFmtFooter aFormatFooter( pFormat );
            }
        }

        // After this exchange method the old page description will point to zero,
        // the new one will point to the node position of the original content nodes.
        ExchangeContentNodes( (SwPageDesc&)aOld, (SwPageDesc&)aNew );
#ifdef DEBUG
        DebugHeaderFooterContent( (SwPageDesc&)aOld );
        DebugHeaderFooterContent( (SwPageDesc&)aNew );
#endif
    }
}

SwUndoPageDesc::~SwUndoPageDesc()
{
}


void SwUndoPageDesc::ExchangeContentNodes( SwPageDesc& rSource, SwPageDesc &rDest )
{
    ASSERT( bExchange, "You shouldn't do that." );
    const SwFmtHeader& rDestHead = rDest.GetMaster().GetHeader();
    const SwFmtHeader& rSourceHead = rSource.GetMaster().GetHeader();
    if( rDestHead.IsActive() )
    {
        // Let the destination page descrition point to the source node position,
        // from now on this descriptor is responsible for the content nodes!
        const SfxPoolItem* pItem;
        rDest.GetMaster().GetAttrSet().GetItemState( RES_HEADER, sal_False, &pItem );
        SfxPoolItem *pNewItem = pItem->Clone();
        SwFrmFmt* pNewFmt = ((SwFmtHeader*)pNewItem)->GetHeaderFmt();
#ifdef DEBUG
        const SwFmtCntnt& rSourceCntnt = rSourceHead.GetHeaderFmt()->GetCntnt();
        (void)rSourceCntnt;
        const SwFmtCntnt& rDestCntnt = rDestHead.GetHeaderFmt()->GetCntnt();
        (void)rDestCntnt;
#endif
        pNewFmt->SetFmtAttr( rSourceHead.GetHeaderFmt()->GetCntnt() );
        delete pNewItem;

        // Let the source page description point to zero node position,
        // it loses the responsible and can be destroyed without removing the content nodes.
        rSource.GetMaster().GetAttrSet().GetItemState( RES_HEADER, sal_False, &pItem );
        pNewItem = pItem->Clone();
        pNewFmt = ((SwFmtHeader*)pNewItem)->GetHeaderFmt();
        pNewFmt->SetFmtAttr( SwFmtCntnt() );
        delete pNewItem;

        if( !rDest.IsHeaderShared() )
        {
            // Same procedure for unshared header..
            const SwFmtHeader& rSourceLeftHead = rSource.GetLeft().GetHeader();
            rDest.GetLeft().GetAttrSet().GetItemState( RES_HEADER, sal_False, &pItem );
            pNewItem = pItem->Clone();
            pNewFmt = ((SwFmtHeader*)pNewItem)->GetHeaderFmt();
#ifdef DEBUG
            const SwFmtCntnt& rSourceCntnt1 = rSourceLeftHead.GetHeaderFmt()->GetCntnt();
            (void)rSourceCntnt1;
            const SwFmtCntnt& rDestCntnt1 = rDest.GetLeft().GetHeader().GetHeaderFmt()->GetCntnt();
            (void)rDestCntnt1;
#endif
            pNewFmt->SetFmtAttr( rSourceLeftHead.GetHeaderFmt()->GetCntnt() );
            delete pNewItem;
            rSource.GetLeft().GetAttrSet().GetItemState( RES_HEADER, sal_False, &pItem );
            pNewItem = pItem->Clone();
            pNewFmt = ((SwFmtHeader*)pNewItem)->GetHeaderFmt();
            pNewFmt->SetFmtAttr( SwFmtCntnt() );
            delete pNewItem;
        }
    }
    // Same procedure for footers...
    const SwFmtFooter& rDestFoot = rDest.GetMaster().GetFooter();
    const SwFmtFooter& rSourceFoot = rSource.GetMaster().GetFooter();
    if( rDestFoot.IsActive() )
    {
        const SfxPoolItem* pItem;
        rDest.GetMaster().GetAttrSet().GetItemState( RES_FOOTER, sal_False, &pItem );
        SfxPoolItem *pNewItem = pItem->Clone();
        SwFrmFmt *pNewFmt = ((SwFmtFooter*)pNewItem)->GetFooterFmt();
        pNewFmt->SetFmtAttr( rSourceFoot.GetFooterFmt()->GetCntnt() );
        delete pNewItem;

#ifdef DEBUG
        const SwFmtCntnt& rFooterSourceCntnt = rSourceFoot.GetFooterFmt()->GetCntnt();
        (void)rFooterSourceCntnt;
        const SwFmtCntnt& rFooterDestCntnt = rDestFoot.GetFooterFmt()->GetCntnt();
        (void)rFooterDestCntnt;
#endif
        rSource.GetMaster().GetAttrSet().GetItemState( RES_FOOTER, sal_False, &pItem );
        pNewItem = pItem->Clone();
        pNewFmt = ((SwFmtFooter*)pNewItem)->GetFooterFmt();
        pNewFmt->SetFmtAttr( SwFmtCntnt() );
        delete pNewItem;

        if( !rDest.IsFooterShared() )
        {
            const SwFmtFooter& rSourceLeftFoot = rSource.GetLeft().GetFooter();
#ifdef DEBUG
            const SwFmtCntnt& rFooterSourceCntnt2 = rSourceLeftFoot.GetFooterFmt()->GetCntnt();
            const SwFmtCntnt& rFooterDestCntnt2 =
                rDest.GetLeft().GetFooter().GetFooterFmt()->GetCntnt();
            (void)rFooterSourceCntnt2;
            (void)rFooterDestCntnt2;
#endif
            rDest.GetLeft().GetAttrSet().GetItemState( RES_FOOTER, sal_False, &pItem );
            pNewItem = pItem->Clone();
            pNewFmt = ((SwFmtFooter*)pNewItem)->GetFooterFmt();
            pNewFmt->SetFmtAttr( rSourceLeftFoot.GetFooterFmt()->GetCntnt() );
            delete pNewItem;
            rSource.GetLeft().GetAttrSet().GetItemState( RES_FOOTER, sal_False, &pItem );
            pNewItem = pItem->Clone();
            pNewFmt = ((SwFmtFooter*)pNewItem)->GetFooterFmt();
            pNewFmt->SetFmtAttr( SwFmtCntnt() );
            delete pNewItem;
        }
    }
}

void SwUndoPageDesc::UndoImpl(::sw::UndoRedoContext &)
{
    // Move (header/footer)content node responsibility from new page descriptor to old one again.
    if( bExchange )
        ExchangeContentNodes( (SwPageDesc&)aNew, (SwPageDesc&)aOld );
    pDoc->ChgPageDesc(aOld.GetName(), aOld);
}

void SwUndoPageDesc::RedoImpl(::sw::UndoRedoContext &)
{
    // Move (header/footer)content node responsibility from old page descriptor to new one again.
    if( bExchange )
        ExchangeContentNodes( (SwPageDesc&)aOld, (SwPageDesc&)aNew );
    pDoc->ChgPageDesc(aNew.GetName(), aNew);
}

SwRewriter SwUndoPageDesc::GetRewriter() const
{
    SwRewriter aResult;

    aResult.AddRule(UNDO_ARG1, aOld.GetName());
    aResult.AddRule(UNDO_ARG2, SW_RES(STR_YIELDS));
    aResult.AddRule(UNDO_ARG3, aNew.GetName());

    return aResult;
}

// #116530#
SwUndoPageDescCreate::SwUndoPageDescCreate(const SwPageDesc * pNew,
                                           SwDoc * _pDoc)
    : SwUndo(UNDO_CREATE_PAGEDESC), pDesc(pNew), aNew(*pNew, _pDoc),
      pDoc(_pDoc)
{
    ASSERT(0 != pDoc, "no document?");
}

SwUndoPageDescCreate::~SwUndoPageDescCreate()
{
}

void SwUndoPageDescCreate::UndoImpl(::sw::UndoRedoContext &)
{
    // -> #116530#
    if (pDesc)
    {
        aNew = *pDesc;
        pDesc = NULL;
    }
    // <- #116530#

    pDoc->DelPageDesc(aNew.GetName(), sal_True);
}

void SwUndoPageDescCreate::DoImpl()
{
    SwPageDesc aPageDesc = aNew;
    pDoc->MakePageDesc(aNew.GetName(), &aPageDesc, sal_False, sal_True); // #116530#
}

void SwUndoPageDescCreate::RedoImpl(::sw::UndoRedoContext &)
{
    DoImpl();
}

void SwUndoPageDescCreate::RepeatImpl(::sw::RepeatContext &)
{
    ::sw::UndoGuard const undoGuard(pDoc->GetIDocumentUndoRedo());
    DoImpl();
}

SwRewriter SwUndoPageDescCreate::GetRewriter() const
{
    SwRewriter aResult;

    if (pDesc)
        aResult.AddRule(UNDO_ARG1, pDesc->GetName());
    else
        aResult.AddRule(UNDO_ARG1, aNew.GetName());


    return aResult;
}

SwUndoPageDescDelete::SwUndoPageDescDelete(const SwPageDesc & _aOld,
                                           SwDoc * _pDoc)
    : SwUndo(UNDO_DELETE_PAGEDESC), aOld(_aOld, _pDoc), pDoc(_pDoc)
{
    ASSERT(0 != pDoc, "no document?");
}

SwUndoPageDescDelete::~SwUndoPageDescDelete()
{
}

void SwUndoPageDescDelete::UndoImpl(::sw::UndoRedoContext &)
{
    SwPageDesc aPageDesc = aOld;
    pDoc->MakePageDesc(aOld.GetName(), &aPageDesc, sal_False, sal_True); // #116530#
}

void SwUndoPageDescDelete::DoImpl()
{
    pDoc->DelPageDesc(aOld.GetName(), sal_True); // #116530#
}

void SwUndoPageDescDelete::RedoImpl(::sw::UndoRedoContext &)
{
    DoImpl();
}

void SwUndoPageDescDelete::RepeatImpl(::sw::RepeatContext &)
{
    ::sw::UndoGuard const undoGuard(pDoc->GetIDocumentUndoRedo());
    DoImpl();
}

SwRewriter SwUndoPageDescDelete::GetRewriter() const
{
    SwRewriter aResult;

    aResult.AddRule(UNDO_ARG1, aOld.GetName());

    return aResult;
}
