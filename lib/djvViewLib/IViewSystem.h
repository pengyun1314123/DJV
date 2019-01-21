//------------------------------------------------------------------------------
// Copyright (c) 2018 Darby Johnston
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions, and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions, and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the names of the copyright holders nor the names of any
//   contributors may be used to endorse or promote products derived from this
//   software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//------------------------------------------------------------------------------

#pragma once

#include <djvCore/ISystem.h>

#include <map>

namespace djv
{
    namespace UI
    {
        class Action;
        class Menu;
        class Widget;
    
    } // namespace UI

    namespace ViewLib
    {
        class IToolWidget;
        class Media;
        
        struct NewMenu
        {
            std::shared_ptr<UI::Menu> menu;
            std::string sortKey;
        };

        struct NewToolWidget
        {
            std::shared_ptr<IToolWidget> widget;
            std::string sortKey;
            glm::vec2 pos = glm::vec2(0.f, 0.f);
        };

        struct NewSettingsWidget
        {
            std::shared_ptr<UI::Widget> widget;
            std::string name;
            std::string sortKey;
        };

        class IViewSystem : public Core::ISystem
        {
            DJV_NON_COPYABLE(IViewSystem);

        protected:
            void _init(const std::string & name, Core::Context *);
            IViewSystem();

        public:
            ~IViewSystem() override;

            virtual std::map<std::string, std::shared_ptr<UI::Action> > getActions();

            virtual NewMenu createMenu();
            virtual NewMenu createContextMenu() { return createMenu(); }
            virtual NewToolWidget createToolWidget();
            virtual std::vector<NewSettingsWidget> createSettingsWidgets();

            virtual void setCurrentMedia(const std::shared_ptr<Media> &);

        private:
            DJV_PRIVATE();
        };

    } // namespace ViewLib
} // namespace djv

