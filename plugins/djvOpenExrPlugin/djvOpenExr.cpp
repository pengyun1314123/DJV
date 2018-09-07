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

#include <djvOpenExr.h>

#include <djvAssert.h>
#include <djvError.h>
#include <djvListUtil.h>
#include <djvTime.h>

#include <ImfFramesPerSecond.h>
#include <ImfStandardAttributes.h>
#include <ImfThreading.h>

#include <QCoreApplication>
#include <QSet>

//------------------------------------------------------------------------------
// djvOpenExr::Channel
//------------------------------------------------------------------------------

djvOpenExr::Channel::Channel(
    const QString &    name,
    djvPixel::TYPE     type,
    const glm::ivec2 & sampling) :
    name    (name),
    type    (type),
    sampling(sampling)
{}

djvOpenExr::Layer::Layer(
    const QVector<Channel> & channels,
    bool                     luminanceChroma) :
    channels       (channels),
    luminanceChroma(luminanceChroma)
{
    QStringList names;
    for (int i = 0; i < channels.count(); ++i)
    {
        names += channels[i].name;
    }
    name = layerName(names);
}

//------------------------------------------------------------------------------
// djvOpenExr::Options
//------------------------------------------------------------------------------

djvOpenExr::Options::Options() :
    threadsEnable      (true),
    threadCount        (4),
    inputColorProfile  (djvOpenExr::COLOR_PROFILE_GAMMA),
    inputGamma         (2.2f),
    channels           (djvOpenExr::CHANNELS_GROUP_KNOWN),
    compression        (djvOpenExr::COMPRESSION_NONE)
#if OPENEXR_VERSION_HEX >= 0x02020000
    ,
    dwaCompressionLevel(45.f)
#endif // OPENEXR_VERSION_HEX
{}

//------------------------------------------------------------------------------
// djvOpenExr
//------------------------------------------------------------------------------

const QString djvOpenExr::staticName = "OpenEXR";

const QStringList & djvOpenExr::colorProfileLabels()
{
    static const QStringList data = QStringList() <<
        qApp->translate("djvOpenExr", "None") <<
        qApp->translate("djvOpenExr", "Gamma") <<
        qApp->translate("djvOpenExr", "Exposure");
    DJV_ASSERT(data.count() == COLOR_PROFILE_COUNT);
    return data;
}

const QStringList & djvOpenExr::compressionLabels()
{
    static const QStringList data = QStringList() <<
        qApp->translate("djvOpenExr", "None") <<
        qApp->translate("djvOpenExr", "RLE") <<
        qApp->translate("djvOpenExr", "ZIPS") <<
        qApp->translate("djvOpenExr", "ZIP") <<
        qApp->translate("djvOpenExr", "PIZ") <<
        qApp->translate("djvOpenExr", "PXR24") <<
        qApp->translate("djvOpenExr", "B44") <<
        qApp->translate("djvOpenExr", "B44A")
#if OPENEXR_VERSION_HEX >= 0x02020000
        <<
        qApp->translate("djvOpenExr", "DWAA") <<
        qApp->translate("djvOpenExr", "DWAB")
#endif // OPENEXR_VERSION_HEX
        ;
    DJV_ASSERT(data.count() == COMPRESSION_COUNT);
    return data;
}

const QStringList & djvOpenExr::channelsLabels()
{
    static const QStringList data = QStringList() <<
        qApp->translate("djvOpenExr", "None") <<
        qApp->translate("djvOpenExr", "Known") <<
        qApp->translate("djvOpenExr", "All");
    DJV_ASSERT(data.count() == CHANNELS_COUNT);
    return data;
}

const QStringList & djvOpenExr::tagLabels()
{
    static const QStringList data = QStringList() <<
        qApp->translate("djvOpenExr", "Longitude") <<
        qApp->translate("djvOpenExr", "Latitude") <<
        qApp->translate("djvOpenExr", "Altitude") <<
        qApp->translate("djvOpenExr", "Focus") <<
        qApp->translate("djvOpenExr", "Exposure") <<
        qApp->translate("djvOpenExr", "Aperture") <<
        qApp->translate("djvOpenExr", "ISO Speed") <<
        qApp->translate("djvOpenExr", "Chromaticities") <<
        qApp->translate("djvOpenExr", "White Luminance") <<
        qApp->translate("djvOpenExr", "XDensity");

    DJV_ASSERT(data.count() == TAG_COUNT);
    return data;
}

QString djvOpenExr::layerName(const QStringList & in)
{
    QString out;

    // Split into a prefix and suffix.
    QSet<QString> prefixSet;
    QStringList   suffixes;
    for (int i = 0; i < in.count(); ++i)
    {
        const QString & name = in[i];
        const int index = name.lastIndexOf('.');
        if (index != -1 && index != 0 && index != name.length() - 1)
        {
            prefixSet += name.mid(0, index);
            suffixes += name.mid(index + 1, name.length() - index - 1);
        }
        else
        {
            prefixSet += name;
        }
    }

    // Join pieces.
    QList<QString> list = prefixSet.toList();
    out = QStringList(list).join(",");
    if (suffixes.count())
    {
        out += "." + suffixes.join(",");
    }
    
    return out;
}

Imf::ChannelList djvOpenExr::defaultLayer(const Imf::ChannelList & in)
{
    Imf::ChannelList out;
    for (
        Imf::ChannelList::ConstIterator i = in.begin();
        i != in.end();
        ++i)
    {
        const QString tmp = i.name();
        const int index = tmp.indexOf('.');
        if (index != -1)
        {
            if (index != 0 || index != tmp.length() - 1)
            {
                continue;
            }
        }
        out.insert(i.name(), i.channel());
    }
    return out;
}

const Imf::Channel * djvOpenExr::find(
    const Imf::ChannelList & in,
    QString &                channel)
{
    //DJV_DEBUG("djvOpenExr::find");
    //DJV_DEBUG_PRINT("channel = " << channel);
    const QString channelLower = channel.toLower();
    for (
        Imf::ChannelList::ConstIterator i = in.begin();
        i != in.end();
        ++i)
    {
        const QString & inName = i.name();
        const int index = inName.lastIndexOf('.');
        const QString tmp =
            (index != -1) ?
            inName.mid(index + 1, inName.length() - index - 1) :
            inName;
        if (channelLower == tmp.toLower())
        {
            //DJV_DEBUG_PRINT("found = " << inName);
            channel = inName;
            return &i.channel();
        }
    }
    return 0;
}

namespace
{
QVector<djvOpenExr::Layer> _layer(
    const Imf::ChannelList & in,
    djvOpenExr::CHANNELS     channels)
{
    //DJV_DEBUG("_layer");
    QVector<djvOpenExr::Layer> out;
    QVector<const Imf::Channel *> reserved;
    if (channels != djvOpenExr::CHANNELS_GROUP_NONE)
    {
        // Look for known channel configurations then convert the remainder.

        // RGB / RGBA.
        QString rName = "r";
        QString gName = "g";
        QString bName = "b";
        QString aName = "a";
        const Imf::Channel * r = djvOpenExr::find (in, rName);
        const Imf::Channel * g = djvOpenExr::find (in, gName);
        const Imf::Channel * b = djvOpenExr::find (in, bName);
        const Imf::Channel * a = djvOpenExr::find (in, aName);
        if (r && g && b && a &&
            compare(QVector<Imf::Channel>() << *r << *g << *b << *a))
        {
            out += djvOpenExr::Layer(QVector<djvOpenExr::Channel>() <<
                djvOpenExr::imfToChannel(rName, *r) <<
                djvOpenExr::imfToChannel(gName, *g) <<
                djvOpenExr::imfToChannel(bName, *b) <<
                djvOpenExr::imfToChannel(aName, *a));
            //DJV_DEBUG_PRINT("rgba = " << out[out.count() - 1].name);
            reserved += QVector<const Imf::Channel *>() <<
                r << g << b << a;
        }
        else if (
            r && g && b &&
            compare(QVector<Imf::Channel>() << *r << *g << *b))
        {
            out += djvOpenExr::Layer(QVector<djvOpenExr::Channel>() <<
                djvOpenExr::imfToChannel(rName, *r) <<
                djvOpenExr::imfToChannel(gName, *g) <<
                djvOpenExr::imfToChannel(bName, *b));
            //DJV_DEBUG_PRINT("rgb = " << out[out.count() - 1].name);
            reserved += QVector<const Imf::Channel *>() <<
                r << g << b;
        }

        // Luminance, XYZ.
        QString yName  = "y";
        QString ryName = "ry";
        QString byName = "by";
        QString xName  = "x";
        QString zName  = "z";
        const Imf::Channel * y  = djvOpenExr::find (in, yName);
        const Imf::Channel * ry = djvOpenExr::find (in, ryName);
        const Imf::Channel * by = djvOpenExr::find (in, byName);
        const Imf::Channel * x  = djvOpenExr::find (in, xName);
        const Imf::Channel * z  = djvOpenExr::find (in, zName);
        if (y && a && compare(QVector<Imf::Channel>() << *y << *a))
        {
            out += djvOpenExr::Layer(QVector<djvOpenExr::Channel>() <<
                djvOpenExr::imfToChannel(yName, *y) <<
                djvOpenExr::imfToChannel(aName, *a));
            //DJV_DEBUG_PRINT("ya = " << out[out.count() - 1].name);
            reserved += QVector<const Imf::Channel *>() <<
                y << a;
        }
        else if (y && ry && by &&
            1 ==  y->xSampling &&
            1 ==  y->ySampling &&
            2 == ry->xSampling &&
            2 == ry->ySampling &&
            2 == by->xSampling &&
            2 == by->ySampling)
        {
            out += djvOpenExr::Layer(QVector<djvOpenExr::Channel>() <<
                djvOpenExr::imfToChannel(yName, *y) <<
                djvOpenExr::imfToChannel(ryName, *ry) <<
                djvOpenExr::imfToChannel(byName, *by),
                true);
            //DJV_DEBUG_PRINT("yc = " << out[out.count() - 1].name);
            reserved += QVector<const Imf::Channel *>() <<
                y << ry << by;
        }
        else if (
            x && y && z &&
            compare(QVector<Imf::Channel>() << *x << *y << *z))
        {
            out += djvOpenExr::Layer(QVector<djvOpenExr::Channel>() <<
                djvOpenExr::imfToChannel(xName, *x) <<
                djvOpenExr::imfToChannel(yName, *y) <<
                djvOpenExr::imfToChannel(zName, *z));
            //DJV_DEBUG_PRINT("xyz = " << out[out.count() - 1].name);
            reserved += QVector<const Imf::Channel *>() <<
                x << y << z;
        }
        else if (
            x && y &&
            compare(QVector<Imf::Channel>() << *x << *y))
        {
            out += djvOpenExr::Layer(QVector<djvOpenExr::Channel>() <<
                djvOpenExr::imfToChannel(xName, *x) <<
                djvOpenExr::imfToChannel(yName, *y));
            //DJV_DEBUG_PRINT("xy = " << out[out.count() - 1].name);
            reserved += QVector<const Imf::Channel *>() <<
                x << y;
        }
        else if (x)
        {
            out += djvOpenExr::Layer(QVector<djvOpenExr::Channel>() <<
                djvOpenExr::imfToChannel(xName, *x));
            //DJV_DEBUG_PRINT("x = " << out[out.count() - 1].name);
            reserved += QVector<const Imf::Channel *>() <<
                x;
        }
        else if (y)
        {
            out += djvOpenExr::Layer(QVector<djvOpenExr::Channel>() <<
                djvOpenExr::imfToChannel(yName, *y));
            //DJV_DEBUG_PRINT("y = " << out[out.count() - 1].name);
            reserved += QVector<const Imf::Channel *>() <<
                y;
        }
        else if (z)
        {
            out += djvOpenExr::Layer(QVector<djvOpenExr::Channel>() <<
                djvOpenExr::imfToChannel(zName, *z));
            //DJV_DEBUG_PRINT("z = " << out[out.count() - 1].name);
            reserved += QVector<const Imf::Channel *>() <<
                z;
        }

        // Colored mattes.
        QString arName = "ar";
        QString agName = "ag";
        QString abName = "ab";
        const Imf::Channel * ar = djvOpenExr::find (in, arName);
        const Imf::Channel * ag = djvOpenExr::find (in, agName);
        const Imf::Channel * ab = djvOpenExr::find (in, abName);
        if (ar && ag && ab &&
            compare(QVector<Imf::Channel>() << *ar << *ag << *ab))
        {
            out += djvOpenExr::Layer(QVector<djvOpenExr::Channel>() <<
                djvOpenExr::imfToChannel(arName, *ar) <<
                djvOpenExr::imfToChannel(agName, *ag) <<
                djvOpenExr::imfToChannel(abName, *ab));
            //DJV_DEBUG_PRINT("matte = " << out[out.count() - 1].name);
            reserved += QVector<const Imf::Channel *>() <<
                ar << ag << ab;
        }
    }

    // Convert the remainder.
    //DJV_DEBUG_PRINT("reserved list = " << reserved.count());
    for (
        Imf::ChannelList::ConstIterator i = in.begin();
        i != in.end();)
    {
        QVector<djvOpenExr::Channel> list;

        // Add the first channel.
        const QString & name = i.name();
        const Imf::Channel & channel = i.channel();
        ++i;
        if (reserved.contains(&channel))
        {
            continue;
        }
        list += djvOpenExr::imfToChannel(name, channel);
        if (djvOpenExr::CHANNELS_GROUP_ALL == channels)
        {
            // Group as many additional channels as possible.
            for (
                ;
                i != in.end() &&
                i.channel() == channel &&
                reserved.contains(&i.channel());
                ++i)
            {
                list += djvOpenExr::imfToChannel(i.name(), i.channel());
            }
        }

        // Add the layer.
        out += djvOpenExr::Layer(list);
        //DJV_DEBUG_PRINT("layer = " << out[out.count() - 1].name);
    }

    return out;
}

} // namespace

QVector<djvOpenExr::Layer> djvOpenExr::layer(
    const Imf::ChannelList & in,
    CHANNELS                 channels)
{
    //DJV_DEBUG("layer");

    QVector<Layer> out;

    // Default layer.
    out += _layer(defaultLayer(in), channels);

    // Additional layers.
    std::set<std::string> layers;
    in.layers(layers);

    for (
        std::set<std::string>::const_iterator i = layers.begin();
        i != layers.end();
        ++i)
    {
        Imf::ChannelList list;
        Imf::ChannelList::ConstIterator f, l;
        in.channelsInLayer(*i, f, l);
        for (Imf::ChannelList::ConstIterator j = f; j != l; ++j)
        {
            list.insert(j.name(), j.channel());
        }
        out += _layer(list, channels);
    }
    //for (int i = 0; i < out.count(); ++i)
    //    DJV_DEBUG_PRINT("layer[" << i << "] = " << out[i].name);

    return out;
}

void djvOpenExr::loadTags(const Imf::Header & in, djvImageIoInfo & out)
{
    const QStringList & openexrTags = tagLabels();
    const QStringList & tags = djvImageTags::tagLabels();
    if (hasOwner(in))
    {
        out.tags[tags[djvImageTags::CREATOR]] = ownerAttribute(in).value().c_str();
    }
    if (hasComments(in))
    {
        out.tags[tags[djvImageTags::DESCRIPTION]] =
            commentsAttribute(in).value().c_str();
    }
    if (hasCapDate(in))
    {
        out.tags[tags[djvImageTags::TIME]] = capDateAttribute(in).value().c_str();
    }
    if (hasUtcOffset(in))
        out.tags[tags[djvImageTags::UTC_OFFSET]] =
            QString::number(utcOffsetAttribute(in).value());
    if (hasLongitude(in))
        out.tags[openexrTags[TAG_LONGITUDE]] =
            QString::number(longitudeAttribute(in).value());
    if (hasLatitude(in))
        out.tags[openexrTags[TAG_LATITUDE]] =
            QString::number(latitudeAttribute(in).value());
    if (hasAltitude(in))
        out.tags[openexrTags[TAG_ALTITUDE]] =
            QString::number(altitudeAttribute(in).value());
    if (hasFocus(in))
        out.tags[openexrTags[TAG_FOCUS]] =
            QString::number(focusAttribute(in).value());
    if (hasExpTime(in))
        out.tags[openexrTags[TAG_EXPOSURE]] =
            QString::number(expTimeAttribute(in).value());
    if (hasAperture(in))
        out.tags[openexrTags[TAG_APERTURE]] =
            QString::number(apertureAttribute(in).value());
    if (hasIsoSpeed(in))
        out.tags[openexrTags[TAG_ISO_SPEED]] =
            QString::number(isoSpeedAttribute(in).value());
    if (hasChromaticities(in))
    {
        const Imf::Chromaticities data = chromaticitiesAttribute(in).value();
        out.tags[openexrTags[TAG_CHROMATICITIES]] = (QStringList() <<
            QString::number(data.red.x) <<
            QString::number(data.red.y) <<
            QString::number(data.green.x) <<
            QString::number(data.green.y) <<
            QString::number(data.blue.x) <<
            QString::number(data.blue.y) <<
            QString::number(data.white.x) <<
            QString::number(data.white.y)).join(" ");
    }
    if (hasWhiteLuminance(in))
        out.tags[openexrTags[TAG_WHITE_LUMINANCE]] =
            QString::number(whiteLuminanceAttribute(in).value());
    if (hasXDensity(in))
        out.tags[openexrTags[TAG_X_DENSITY]] =
            QString::number(xDensityAttribute(in).value());
    if (hasKeyCode(in))
    {
        const Imf::KeyCode data = keyCodeAttribute(in).value();
        out.tags[tags[djvImageTags::KEYCODE]] =
            djvTime::keycodeToString(
                data.filmMfcCode(),
                data.filmType(),
                data.prefix(),
                data.count(),
                data.perfOffset());
    }
    if (hasTimeCode(in))
        out.tags[tags[djvImageTags::TIMECODE]] = djvTime::timecodeToString(
                timeCodeAttribute(in).value().timeAndFlags());
    if (hasFramesPerSecond(in))
    {
        const Imf::Rational data = framesPerSecondAttribute(in).value();
        out.sequence.speed = djvSpeed(data.n, data.d);
    }
}

void djvOpenExr::saveTags(const djvImageIoInfo & in, Imf::Header & out)
{
    const QStringList & openexrTags = tagLabels();
    const QStringList & tags = djvImageTags::tagLabels();
    QString tmp = in.tags[tags[djvImageTags::CREATOR]];
    if (tmp.length())
    {
        addOwner(out, tmp.toLatin1().data());
    }
    tmp = in.tags[tags[djvImageTags::DESCRIPTION]];
    if (tmp.length())
    {
        addComments(out, tmp.toLatin1().data());
    }
    tmp = in.tags[tags[djvImageTags::TIME]];
    if (tmp.length())
    {
        addCapDate(out, tmp.toLatin1().data());
    }
    tmp = in.tags[tags[djvImageTags::UTC_OFFSET]];
    if (tmp.length())
    {
        addUtcOffset(out, tmp.toFloat());
    }
    tmp = in.tags[openexrTags[TAG_LONGITUDE]];
    if (tmp.length())
    {
        addLongitude(out, tmp.toFloat());
    }
    tmp = in.tags[openexrTags[TAG_LATITUDE]];
    if (tmp.length())
    {
        addLatitude(out, tmp.toFloat());
    }
    tmp = in.tags[openexrTags[TAG_ALTITUDE]];
    if (tmp.length())
    {
        addAltitude(out, tmp.toFloat());
    }
    tmp = in.tags[openexrTags[TAG_FOCUS]];
    if (tmp.length())
    {
        addFocus(out, tmp.toFloat());
    }
    tmp = in.tags[openexrTags[TAG_EXPOSURE]];
    if (tmp.length())
    {
        addExpTime(out, tmp.toFloat());
    }
    tmp = in.tags[openexrTags[TAG_APERTURE]];
    if (tmp.length())
    {
        addAperture(out, tmp.toFloat());
    }
    tmp = in.tags[openexrTags[TAG_ISO_SPEED]];
    if (tmp.length())
    {
        addIsoSpeed(out, tmp.toFloat());
    }
    tmp = in.tags[openexrTags[TAG_CHROMATICITIES]];
    if (tmp.length())
    {
        const QStringList list = tmp.split(' ', QString::SkipEmptyParts);
        if (8 == list.count())
        {
            addChromaticities(
                out,
                Imf::Chromaticities(
                    Imath::V2f(list[0].toFloat(), list[1].toFloat()),
                    Imath::V2f(list[2].toFloat(), list[3].toFloat()),
                    Imath::V2f(list[4].toFloat(), list[5].toFloat()),
                    Imath::V2f(list[6].toFloat(), list[7].toFloat())));
        }
    }
    tmp = in.tags[openexrTags[TAG_WHITE_LUMINANCE]];
    if (tmp.length())
        addWhiteLuminance(out, tmp.toFloat());
    tmp = in.tags[openexrTags[TAG_X_DENSITY]];
    if (tmp.length())
    {
        addXDensity(out, tmp.toFloat());
    }
    tmp = in.tags[tags[djvImageTags::KEYCODE]];
    if (tmp.length())
    {
        int id = 0, type = 0, prefix = 0, count = 0, offset = 0;
        djvTime::stringToKeycode(tmp, id, type, prefix, count, offset);
        addKeyCode(out, Imf::KeyCode(id, type, prefix, count, offset));
    }
    tmp = in.tags[tags[djvImageTags::TIMECODE]];
    if (tmp.length())
    {
        addTimeCode(out, djvTime::stringToTimecode(tmp));
    }
    addFramesPerSecond(
        out,
        Imf::Rational(in.sequence.speed.scale(), in.sequence.speed.duration()));
}

djvBox2i djvOpenExr::imfToBox(const Imath::Box2i & in)
{
    return djvBox2i(
        glm::ivec2(in.min.x, in.min.y),
        glm::ivec2(in.max.x, in.max.y) - glm::ivec2(in.min.x, in.min.y) + 1);
}

Imf::PixelType djvOpenExr::pixelTypeToImf(djvPixel::TYPE in)
{
    switch (in)
    {
        case djvPixel::U8:
        case djvPixel::U10:
        case djvPixel::U16:
        case djvPixel::F16: return Imf::HALF;
        default: break;
    }
    return Imf::FLOAT;
}

djvPixel::TYPE djvOpenExr::imfToPixelType(Imf::PixelType in)
{
    switch (in)
    {
        case Imf::HALF: return djvPixel::F16;
        default: break;
    }
    return djvPixel::F32;
}

djvOpenExr::Channel djvOpenExr::imfToChannel(
    const QString &      name,
    const Imf::Channel & channel)
{
    return Channel(
        name,
        imfToPixelType(channel.type),
        glm::ivec2(channel.xSampling, channel.ySampling));
}

const QStringList & djvOpenExr::optionsLabels()
{
    static const QStringList data = QStringList() <<
        qApp->translate("djvOpenExr", "Threads Enable") <<
        qApp->translate("djvOpenExr", "Thread Count") <<
        qApp->translate("djvOpenExr", "Input Color Profile") <<
        qApp->translate("djvOpenExr", "Input Gamma") <<
        qApp->translate("djvOpenExr", "Input Exposure") <<
        qApp->translate("djvOpenExr", "Channels") <<
        qApp->translate("djvOpenExr", "Compression")
#if OPENEXR_VERSION_HEX >= 0x02020000
        <<
        qApp->translate("djvOpenExr", "DWA Compression Level");
#endif // OPENEXR_VERSION_HEX
        ;
    DJV_ASSERT(data.count() == OPTIONS_COUNT);
    return data;
}

_DJV_STRING_OPERATOR_LABEL(djvOpenExr::COLOR_PROFILE,
    djvOpenExr::colorProfileLabels())
_DJV_STRING_OPERATOR_LABEL(djvOpenExr::COMPRESSION,
    djvOpenExr::compressionLabels())
_DJV_STRING_OPERATOR_LABEL(djvOpenExr::CHANNELS,
    djvOpenExr::channelsLabels())

bool compare(const QVector<Imf::Channel> & in)
{
    for (int i = 1; i < in.count(); ++i)
    {
        if (! (in[0] == in[i]))
        {
            return false;
        }
    }
    return true;
}