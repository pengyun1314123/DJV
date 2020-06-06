// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2004-2020 Darby Johnston
// All rights reserved.

#include <djvViewApp/MemoryCacheWidget.h>

#include <djvViewApp/FileSettings.h>
#include <djvViewApp/FileSystem.h>

#include <djvUI/CheckBox.h>
#include <djvUI/IntSlider.h>
#include <djvUI/Label.h>
#include <djvUI/RowLayout.h>
#include <djvUI/SettingsSystem.h>

#include <djvCore/Context.h>
#include <djvCore/OS.h>

using namespace djv::Core;

namespace djv
{
    namespace ViewApp
    {
        struct MemoryCacheWidget::Private
        {
            float percentageUsed = 0.F;

            std::shared_ptr<UI::Label> titleLabel;
            std::shared_ptr<UI::CheckBox> enabledCheckBox;
            std::shared_ptr<UI::IntSlider> maxGBSlider;
            std::shared_ptr<UI::Label> maxGBLabel;
            std::shared_ptr<UI::Label> percentageLabel;
            std::shared_ptr<UI::Label> percentageLabel2;
            std::shared_ptr<UI::VerticalLayout> layout;

            std::shared_ptr<ValueObserver<bool> > enabledObserver;
            std::shared_ptr<ValueObserver<int> > maxGBObserver;
            std::shared_ptr<ValueObserver<float> > percentageObserver;
        };

        void MemoryCacheWidget::_init(const std::shared_ptr<Core::Context>& context)
        {
            Widget::_init(context);
            DJV_PRIVATE_PTR();
            setClassName("djv::ViewApp::MemoryCacheWidget");

            p.titleLabel = UI::Label::create(context);
            p.titleLabel->setTextHAlign(UI::TextHAlign::Left);
            p.titleLabel->setMargin(UI::MetricsRole::MarginSmall);
            p.titleLabel->setBackgroundRole(UI::ColorRole::Trough);

            p.enabledCheckBox = UI::CheckBox::create(context);

            p.maxGBSlider = UI::IntSlider::create(context);
            p.maxGBSlider->setRange(IntRange(1, OS::getRAMSize() / Memory::gigabyte));
            p.maxGBLabel = UI::Label::create(context);
            p.maxGBLabel->setTextHAlign(UI::TextHAlign::Left);

            p.percentageLabel = UI::Label::create(context);
            p.percentageLabel->setTextHAlign(UI::TextHAlign::Left);
            p.percentageLabel2 = UI::Label::create(context);
            p.percentageLabel2->setFontFamily(AV::Font::familyMono);

            p.layout = UI::VerticalLayout::create(context);
            p.layout->setSpacing(UI::MetricsRole::None);
            p.layout->addChild(p.titleLabel);
            p.layout->addSeparator();
            auto vLayout = UI::VerticalLayout::create(context);
            vLayout->setSpacing(UI::MetricsRole::None);
            vLayout->addChild(p.enabledCheckBox);
            auto hLayout = UI::HorizontalLayout::create(context);
            hLayout->setMargin(UI::MetricsRole::MarginSmall);
            hLayout->setSpacing(UI::MetricsRole::SpacingSmall);
            hLayout->addChild(p.maxGBSlider);
            hLayout->setStretch(p.maxGBSlider, UI::RowStretch::Expand);
            hLayout->addChild(p.maxGBLabel);
            vLayout->addChild(hLayout);
            hLayout = UI::HorizontalLayout::create(context);
            hLayout->setMargin(UI::MetricsRole::MarginSmall);
            hLayout->setSpacing(UI::MetricsRole::SpacingSmall);
            hLayout->addChild(p.percentageLabel);
            hLayout->addChild(p.percentageLabel2);
            vLayout->addChild(hLayout);
            p.layout->addChild(vLayout);
            addChild(p.layout);

            auto contextWeak = std::weak_ptr<Context>(context);
            p.enabledCheckBox->setCheckedCallback(
                [contextWeak](bool value)
                {
                    if (auto context = contextWeak.lock())
                    {
                        auto settingsSystem = context->getSystemT<UI::Settings::System>();
                        if (auto fileSettings = settingsSystem->getSettingsT<FileSettings>())
                        {
                            fileSettings->setCacheEnabled(value);
                        }
                    }
                });
            p.maxGBSlider->setValueCallback(
                [contextWeak](int value)
                {
                    if (auto context = contextWeak.lock())
                    {
                        auto settingsSystem = context->getSystemT<UI::Settings::System>();
                        if (auto fileSettings = settingsSystem->getSettingsT<FileSettings>())
                        {
                            fileSettings->setCacheMaxGB(value);
                        }
                    }
                });

            auto weak = std::weak_ptr<MemoryCacheWidget>(
                std::dynamic_pointer_cast<MemoryCacheWidget>(shared_from_this()));
            auto settingsSystem = context->getSystemT<UI::Settings::System>();
            if (auto fileSettings = settingsSystem->getSettingsT<FileSettings>())
            {
                p.enabledObserver = ValueObserver<bool>::create(
                    fileSettings->observeCacheEnabled(),
                    [weak](bool value)
                    {
                        if (auto widget = weak.lock())
                        {
                            widget->_p->enabledCheckBox->setChecked(value);
                        }
                    });

                p.maxGBObserver = ValueObserver<int>::create(
                    fileSettings->observeCacheMaxGB(),
                    [weak](int value)
                    {
                        if (auto widget = weak.lock())
                        {
                            widget->_p->maxGBSlider->setValue(value);
                        }
                    });
            }

            if (auto fileSystem = context->getSystemT<FileSystem>())
            {
                p.percentageObserver = ValueObserver<float>::create(
                    fileSystem->observeCachePercentage(),
                    [weak](float value)
                    {
                        if (auto widget = weak.lock())
                        {
                            widget->_p->percentageUsed = value;
                            widget->_widgetUpdate();
                        }
                    });
            }
        }

        MemoryCacheWidget::MemoryCacheWidget() :
            _p(new Private)
        {}

        MemoryCacheWidget::~MemoryCacheWidget()
        {}

        std::shared_ptr<MemoryCacheWidget> MemoryCacheWidget::create(const std::shared_ptr<Core::Context>& context)
        {
            auto out = std::shared_ptr<MemoryCacheWidget>(new MemoryCacheWidget);
            out->_init(context);
            return out;
        }

        void MemoryCacheWidget::_preLayoutEvent(Event::PreLayout&)
        {
            const auto& style = _getStyle();
            _setMinimumSize(_p->layout->getMinimumSize() + getMargin().getSize(style));
        }
        
        void MemoryCacheWidget::_layoutEvent(Event::Layout&)
        {
            const auto& style = _getStyle();
            _p->layout->setGeometry(getMargin().bbox(getGeometry(), style));
        }

        void MemoryCacheWidget::_initEvent(Event::Init & event)
        {
            Widget::_initEvent(event);
            if (event.getData().text)
            {
                _widgetUpdate();
            }
        }

        void MemoryCacheWidget::_widgetUpdate()
        {
            DJV_PRIVATE_PTR();
            p.titleLabel->setText(_getText(DJV_TEXT("memory_cache")));
            p.enabledCheckBox->setText(_getText(DJV_TEXT("memory_cache_enable")));
            {
                std::stringstream ss;
                ss << Memory::Unit::GB;
                p.maxGBLabel->setText(_getText(ss.str()));
            }
            p.percentageLabel->setText(_getText(DJV_TEXT("memory_cache_used")) + ":");
            {
                std::stringstream ss;
                ss << static_cast<int>(p.percentageUsed) << "%";
                p.percentageLabel2->setText(ss.str());
            }
        }

    } // namespace ViewApp
} // namespace djv

