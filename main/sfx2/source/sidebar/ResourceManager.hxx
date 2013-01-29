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

#ifndef SFX_SIDEBAR_RESOURCE_MANAGER_HXX
#define SFX_SIDEBAR_RESOURCE_MANAGER_HXX

#include "DeckDescriptor.hxx"
#include "PanelDescriptor.hxx"
#include "sfx2/sidebar/EnumContext.hxx"
#include <unotools/confignode.hxx>
#include <com/sun/star/frame/XFrame.hpp>
#include <set>
#include <boost/shared_ptr.hpp>


namespace css = ::com::sun::star;
namespace cssu = ::com::sun::star::uno;

namespace sfx2 { namespace sidebar {

/** Read the content of the Sidebar.xcu file and provide access
    methods so that the sidebar can easily decide which content panels
    to display for a certain context.
*/
class ResourceManager
{
public:
    static ResourceManager& Instance (void);

    const DeckDescriptor* GetBestMatchingDeck (
        const EnumContext& rContext,
        const cssu::Reference<css::frame::XFrame>& rxFrame);

    const DeckDescriptor* GetDeckDescriptor (
        const ::rtl::OUString& rsDeckId) const;
    const PanelDescriptor* GetPanelDescriptor (
        const ::rtl::OUString& rsPanelId) const;

    /** Excluded or include a deck from being displayed in the tab
        bar.
        Note that this value is not persistent.
        The flag can not be set directly at a DeckDescriptor object
        because the ResourceManager gives access to to them only
        read-only.
    */
    void SetIsDeckEnabled (
        const ::rtl::OUString& rsDeckId,
        const bool bIsEnabled);

    typedef ::std::vector<rtl::OUString> IdContainer;

    const IdContainer& GetMatchingDecks (
        IdContainer& rDeckDescriptors,
        const EnumContext& rContext,
        const cssu::Reference<css::frame::XFrame>& rxFrame);

    const IdContainer& GetMatchingPanels (
        IdContainer& rPanelDescriptors,
        const EnumContext& rContext,
        const ::rtl::OUString& rsDeckId,
        const cssu::Reference<css::frame::XFrame>& rxFrame);

    static ::rtl::OUString GetModuleName (
        const cssu::Reference<css::frame::XFrame>& rxFrame);

private:
    ResourceManager (void);
    ~ResourceManager (void);
    class Deleter;
    friend class Deleter;

    typedef ::std::vector<DeckDescriptor> DeckContainer;
    DeckContainer maDecks;
    typedef ::std::vector<PanelDescriptor> PanelContainer;
    PanelContainer maPanels;
    mutable ::std::set<rtl::OUString> maProcessedApplications;

    void ReadDeckList (void);
    void ReadPanelList (void);
    void ReadContextList (
        const ::utl::OConfigurationNode& rNode,
        ::std::vector<EnumContext>& rContextContainer) const;
    void ReadLegacyAddons (
        const cssu::Reference<css::frame::XFrame>& rxFrame);
    ::utl::OConfigurationTreeRoot GetLegacyAddonRootNode (
        const ::rtl::OUString& rsModuleName) const;
    void GetToolPanelNodeNames (
        ::std::vector<rtl::OUString>& rMatchingNames,
        const ::utl::OConfigurationTreeRoot aRoot) const;
};


} } // end of namespace sfx2::sidebar

#endif
