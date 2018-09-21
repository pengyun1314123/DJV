//------------------------------------------------------------------------------
// Copyright (c) 2004-2015 Darby Johnston
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions, and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions, and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
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

#include <djvGraphicsTest/ImageTagsTest.h>

#include <djvGraphics/ImageTags.h>

#include <djvCore/Assert.h>
#include <djvCore/Debug.h>

#include <QStringList>

using namespace djv::Core;
using namespace djv::Graphics;

namespace djv
{
    namespace GraphicsTest
    {
        void ImageTagsTest::run(int &, char **)
        {
            DJV_DEBUG("ImageTagsTest::run");
            ctors();
            members();
            operators();
        }

        void ImageTagsTest::ctors()
        {
            DJV_DEBUG("ImageTagsTest::ctors");
            {
                const Graphics::ImageTags tags;
                DJV_ASSERT(0 == tags.count());
            }
            {
                Graphics::ImageTags tmp;
                tmp.add("key", "value");
                const Graphics::ImageTags tags(tmp);
                DJV_ASSERT(1 == tags.count());
                DJV_ASSERT("key" == tags.keys()[0]);
                DJV_ASSERT("value" == tags.values()[0]);
            }
        }

        void ImageTagsTest::members()
        {
            DJV_DEBUG("ImageTagsTest::members");
            {
                Graphics::ImageTags tmp;
                tmp.add("key", "value");
                Graphics::ImageTags tags;
                tags.add(tmp);
                DJV_ASSERT(1 == tags.count());
                DJV_ASSERT("key" == tags.keys()[0]);
                DJV_ASSERT("value" == tags.values()[0]);
                DJV_ASSERT("value" == tags.tag("key"));
                DJV_ASSERT(!tags.tag("none").length());
                DJV_ASSERT(tags.isValid("key"));
                DJV_ASSERT(!tags.isValid("none"));
                tags.add("key", "value 2");
                DJV_ASSERT(1 == tags.count());
                DJV_ASSERT("value 2" == tags.tag("key"));
                tags.clear();
                DJV_ASSERT(0 == tags.count());
            }
            {
                DJV_DEBUG_PRINT(Graphics::ImageTags::tagLabels());
            }
        }

        void ImageTagsTest::operators()
        {
            DJV_DEBUG("ImageTagsTest::operators");
            {
                Graphics::ImageTags tags;
                tags["key"] = "value";
                DJV_ASSERT("value" == tags["key"]);
                const Graphics::ImageTags & tmp = tags;
                DJV_ASSERT("value" == tmp["key"]);
                tags["key 2"];
                DJV_ASSERT(2 == tags.count());
            }
            {
                Graphics::ImageTags a, b, c;
                a["key"] = "value";
                b["key"] = "value";
                c["key"] = "value 2";
                DJV_ASSERT(a == b);
                DJV_ASSERT(a != c);
                DJV_ASSERT(a != Graphics::ImageTags());
            }
            {
                Graphics::ImageTags tags;
                tags["key"] = "value";
                tags["key 2"] = "value 2";
                DJV_DEBUG_PRINT(tags);
            }
        }

    } // namespace GraphicsTest
} // namespace djv