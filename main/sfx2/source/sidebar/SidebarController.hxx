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

#ifndef SFX_SIDEBAR_CONTROLLER_HXX
#define SFX_SIDEBAR_CONTROLLER_HXX

#include "ResourceManager.hxx"
#include "sfx2/sidebar/EnumContext.hxx"
#include "AsynchronousCall.hxx"
#include "TabBar.hxx"

#include <vcl/menu.hxx>

#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/ui/XContextChangeEventListener.hpp>
#include <com/sun/star/ui/XUIElement.hpp>

#include <boost/noncopyable.hpp>
#include <cppuhelper/compbase2.hxx>
#include <cppuhelper/basemutex.hxx>

namespace css = ::com::sun::star;
namespace cssu = ::com::sun::star::uno;

namespace
{
    typedef ::cppu::WeakComponentImplHelper2 <
        css::ui::XContextChangeEventListener,
        css::beans::XPropertyChangeListener
        > SidebarControllerInterfaceBase;
}


class DockingWindow;

namespace sfx2 { namespace sidebar {

class ContentPanelDescriptor;
class Deck;
class DeckConfiguration;
class DeckDescriptor;
class Panel;
class TabBar;
class TabBarConfiguration;

class SidebarController
    : private ::boost::noncopyable,
      private ::cppu::BaseMutex,
      public SidebarControllerInterfaceBase
{
public:
    SidebarController(
        DockingWindow* pParentWindow,
        const cssu::Reference<css::frame::XFrame>& rxFrame);
    virtual ~SidebarController (void);

    // ui::XContextChangeEventListener
    virtual void SAL_CALL notifyContextChangeEvent (const css::ui::ContextChangeEventObject& rEvent)
        throw(cssu::RuntimeException);

    // XEventListener
    virtual void SAL_CALL disposing (const css::lang::EventObject& rEventObject)
        throw(cssu::RuntimeException);

    // beans::XPropertyChangeListener
    virtual void SAL_CALL propertyChange (const css::beans::PropertyChangeEvent& rEvent)
        throw(cssu::RuntimeException);
    
    void NotifyResize (void);

    void SwitchToDeck (
        const ::rtl::OUString& rsDeckId);

private:
    ::boost::shared_ptr<DeckConfiguration> mpCurrentConfiguration;
    DockingWindow* mpParentWindow;
    TabBar* mpTabBar;
    cssu::Reference<css::frame::XFrame> mxFrame;
    EnumContext maCurrentContext;
    ::rtl::OUString msCurrentDeckId;
    AsynchronousCall maPropertyChangeForwarder;
    
    DECL_LINK(WindowEventHandler, VclWindowEvent*);
    void UpdateConfigurations (const EnumContext& rContext);
    cssu::Reference<css::ui::XUIElement> CreateUIElement (
        const cssu::Reference<css::awt::XWindowPeer>& rxWindow,
        const ::rtl::OUString& rsImplementationURL,
        Panel* pPanel) const;
    Panel* CreatePanel (
        const ::rtl::OUString& rsPanelId,
        Deck* pDeck) const;
    void SwitchToDeck (
        const DeckDescriptor& rDeckDescriptor,
        const EnumContext& rContext);
    void ShowPopupMenu (
        const Rectangle& rButtonBox,
        const ::std::vector<TabBar::DeckMenuData>& rDeckSelectionData,
        const ::std::vector<TabBar::DeckMenuData>& rDeckShowData) const;
    ::boost::shared_ptr<PopupMenu> CreatePopupMenu (
        const ::std::vector<TabBar::DeckMenuData>& rDeckSelectionData,
        const ::std::vector<TabBar::DeckMenuData>& rDeckShowData) const;
    DECL_LINK(OnMenuItemSelected, Menu*);
    void BroadcastPropertyChange (void);

    virtual void SAL_CALL disposing (void);
};


} } // end of namespace sfx2::sidebar

#endif
