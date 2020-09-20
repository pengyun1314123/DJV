// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2004-2020 Darby Johnston
// All rights reserved.

#pragma once

#include <djvUI/Widget.h>

#include <djvImage/Pixel.h>

namespace djv
{
    namespace Image
    {
        class Color;

    } // Image

    namespace UI
    {
        //! This class provides a popup widget for choosing a color type.
        class ColorTypeWidget : public Widget
        {
            DJV_NON_COPYABLE(ColorTypeWidget);

        protected:
            void _init(const std::shared_ptr<System::Context>&);
            ColorTypeWidget();

        public:
            ~ColorTypeWidget() override;

            static std::shared_ptr<ColorTypeWidget> create(const std::shared_ptr<System::Context>&);

            Image::Type getType() const;

            void setType(Image::Type);
            void setTypeCallback(const std::function<void(Image::Type)> &);

        protected:
            void _preLayoutEvent(System::Event::PreLayout&) override;
            void _layoutEvent(System::Event::Layout&) override;

            void _initEvent(System::Event::Init &) override;

        private:
            void _widgetUpdate();

            DJV_PRIVATE();
        };

        //! This class provides slider widgets for choosing a color.
        //!
        //! \todo Implement HSV.
        class ColorSliders : public Widget
        {
            DJV_NON_COPYABLE(ColorSliders);

        protected:
            void _init(const std::shared_ptr<System::Context>&);
            ColorSliders();

        public:
            ~ColorSliders() override;

            static std::shared_ptr<ColorSliders> create(const std::shared_ptr<System::Context>&);

            const Image::Color & getColor() const;

            void setColor(const Image::Color &);
            void setColorCallback(const std::function<void(const Image::Color &)> &);
            
            bool hasHSV() const;

            void setHSV(bool);

        protected:
            void _preLayoutEvent(System::Event::PreLayout&) override;
            void _layoutEvent(System::Event::Layout&) override;

            void _initEvent(System::Event::Init &) override;

        private:
            void _widgetUpdate();
            void _colorUpdate();
            void _textUpdate();

            DJV_PRIVATE();
        };

        //! This class provides a color picker widget.
        class ColorPicker : public Widget
        {
            DJV_NON_COPYABLE(ColorPicker);

        protected:
            void _init(const std::shared_ptr<System::Context>&);
            ColorPicker();

        public:
            ~ColorPicker() override;

            static std::shared_ptr<ColorPicker> create(const std::shared_ptr<System::Context>&);

            const Image::Color& getColor() const;

            void setColor(const Image::Color&);
            void setColorCallback(const std::function<void(const Image::Color&)>&);

        protected:
            void _preLayoutEvent(System::Event::PreLayout&) override;
            void _layoutEvent(System::Event::Layout&) override;

        private:
            void _colorUpdate();

            DJV_PRIVATE();
        };

        //! This class provides a color swatch with a color picker.
        class ColorPickerSwatch : public Widget
        {
            DJV_NON_COPYABLE(ColorPickerSwatch);

        protected:
            void _init(const std::shared_ptr<System::Context>&);
            ColorPickerSwatch();

        public:
            ~ColorPickerSwatch() override;

            static std::shared_ptr<ColorPickerSwatch> create(const std::shared_ptr<System::Context>&);

            const Image::Color& getColor() const;

            void setColor(const Image::Color&);
            void setColorCallback(const std::function<void(const Image::Color&)>&);

            MetricsRole getSwatchSizeRole() const;

            void setSwatchSizeRole(MetricsRole);

            void open();
            void close();

        protected:
            void _preLayoutEvent(System::Event::PreLayout&) override;
            void _layoutEvent(System::Event::Layout&) override;

        private:
            void _colorUpdate();

            DJV_PRIVATE();
        };

    } // namespace UI
} // namespace djv

