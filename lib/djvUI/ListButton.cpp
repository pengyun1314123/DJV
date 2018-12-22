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

#include <djvUI/ListButton.h>

#include <djvUI/Label.h>
#include <djvUI/RowLayout.h>
#include <djvUI/StackLayout.h>

#include <djvAV/Render2DSystem.h>

//#pragma optimize("", off)

using namespace djv::Core;

namespace djv
{
    namespace UI
    {
        namespace Button
        {
            struct List::Private
            {
                std::shared_ptr<Label> label;
                std::shared_ptr<Layout::Stack> layout;
            };

            void List::_init(const std::string& text, Context * context)
            {
                IButton::_init(context);

                setClassName("Gp::UI::Button::List");

                _p->label = Label::create(text, context);
                _p->label->setMargin(Style::MetricsRole::Margin);
                _p->label->setVisible(!text.empty());

                _p->layout = Layout::Stack::create(context);
                _p->layout->addWidget(_p->label);
                _p->layout->setParent(shared_from_this());
            }

            List::List() :
                _p(new Private)
            {}

            List::~List()
            {}

            std::shared_ptr<List> List::create(Context * context)
            {
                auto out = std::shared_ptr<List>(new List);
                out->_init(std::string(), context);
                return out;
            }

            std::shared_ptr<List> List::create(const std::string& text, Context * context)
            {
                auto out = std::shared_ptr<List>(new List);
                out->_init(text, context);
                return out;
            }

            const std::string& List::getText() const
            {
                return _p->label->getText();
            }

            void List::setText(const std::string& value)
            {
                _p->label->setText(value);
                _p->label->setVisible(!value.empty());
            }

            TextHAlign List::getTextHAlign() const
            {
                return _p->label->getTextHAlign();
            }

            TextVAlign List::getTextVAlign() const
            {
                return _p->label->getTextVAlign();
            }

            void List::setTextHAlign(TextHAlign value)
            {
                _p->label->setTextHAlign(value);
            }

            void List::setTextVAlign(TextVAlign value)
            {
                _p->label->setTextVAlign(value);
            }

            Style::ColorRole List::getTextColorRole() const
            {
                return _p->label->getTextColorRole();
            }

            void List::setTextColorRole(Style::ColorRole value)
            {
                _p->label->setTextColorRole(value);
            }

            const std::string & List::getFontFace() const
            {
                return _p->label->getFontFace();
            }

            Style::MetricsRole List::getFontSizeRole() const
            {
                return _p->label->getFontSizeRole();
            }

            void List::setFontFace(const std::string & value)
            {
                _p->label->setFontFace(value);
            }

            void List::setFontSizeRole(Style::MetricsRole value)
            {
                _p->label->setFontSizeRole(value);
            }

            float List::getHeightForWidth(float value) const
            {
                return _p->layout->getHeightForWidth(value);
            }

            void List::updateEvent(Event::Update& event)
            {
                IButton::updateEvent(event);

                _p->label->setTextColorRole(_isToggled() ? Style::ColorRole::CheckedForeground : Style::ColorRole::Foreground);
            }

            void List::preLayoutEvent(Event::PreLayout& event)
            {
                _setMinimumSize(_p->layout->getMinimumSize());
            }

            void List::layoutEvent(Event::Layout&)
            {
                _p->layout->setGeometry(getGeometry());
            }

        } // namespace Button
    } // namespace UI
} // namespace Gp
