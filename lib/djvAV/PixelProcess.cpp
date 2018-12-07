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

#include <djvAV/PixelProcess.h>

#include <djvAV/OpenGLMesh.h>
#include <djvAV/OpenGLOffscreenBuffer.h>
#include <djvAV/OpenGLShader.h>
#include <djvAV/OpenGLTexture.h>
#include <djvAV/Shader.h>
#include <djvAV/Shape.h>
#include <djvAV/System.h>
#include <djvAV/TriangleMesh.h>

#include <djvCore/Context.h>

#include <glm/gtc/matrix_transform.hpp>

namespace djv
{
    namespace AV
    {
	    namespace Pixel
	    {
            struct Convert::Private
            {
                std::shared_ptr<OpenGL::OffscreenBuffer> offscreenBuffer;
                std::shared_ptr<AV::OpenGL::Texture> texture;
                std::shared_ptr<AV::OpenGL::VBO> vbo;
                std::shared_ptr<AV::OpenGL::VAO> vao;
                std::shared_ptr<AV::OpenGL::Shader> shader;
            };

            void Convert::_init(const std::shared_ptr<Core::Context> & context)
            {
                AV::Shape::Square square;
                AV::TriangleMesh mesh;
                square.triangulate(mesh);
                _p->vbo = AV::OpenGL::VBO::create(2, 3, AV::OpenGL::VBOType::Pos3_F32_UV_U16_Normal_U10);
                _p->vbo->copy(AV::OpenGL::VBO::convert(mesh, _p->vbo->getType()));
                _p->vao = AV::OpenGL::VAO::create(_p->vbo->getType(), _p->vbo->getID());

                _p->shader = AV::OpenGL::Shader::create(AV::Shader::create(
                    context->getResourcePath(Core::ResourcePath::ShadersDirectory, "djvAVPixelConvertVertex.glsl"),
                    context->getResourcePath(Core::ResourcePath::ShadersDirectory, "djvAVPixelConvertFragment.glsl")));
            }

            Convert::Convert() :
                _p(new Private)
            {}

            Convert::~Convert()
            {}

            std::shared_ptr<Convert> Convert::create(const std::shared_ptr<Core::Context> & context)
            {
                auto out = std::shared_ptr<Convert>(new Convert);
                out->_init(context);
                return out;
            }

            std::shared_ptr<Data> Convert::process(const std::shared_ptr<Data> & data, const glm::ivec2 & size, Type type)
            {
                const auto info = Info(size, type);
                auto out = Data::create(info);

                if (!_p->offscreenBuffer || (_p->offscreenBuffer && info != _p->offscreenBuffer->getInfo()))
                {
                    _p->offscreenBuffer = OpenGL::OffscreenBuffer::create(info);
                }
                const OpenGL::OffscreenBufferBinding binding(_p->offscreenBuffer);

                if (!_p->texture || (_p->texture && data->getInfo() != _p->texture->getInfo()))
                {
                    _p->texture = OpenGL::Texture::create(data->getInfo());
                }
                _p->texture->bind();
                _p->texture->copy(*data);

                _p->shader->bind();
                _p->shader->setUniform("textureSampler", 0);
                glm::mat4x4 modelMatrix(1);
                modelMatrix = glm::rotate(modelMatrix, Core::Math::deg2rad(-90.f), glm::vec3(1.f, 0.f, 0.f));
                modelMatrix = glm::scale(modelMatrix, glm::vec3(size.x, 0.f, size.y));
                modelMatrix = glm::translate(modelMatrix, glm::vec3(.5f, 0.f, .5f));
                glm::mat4x4 viewMatrix(1);
                glm::mat4x4 projectionMatrix(1);
                projectionMatrix = glm::ortho(
                    0.f,
                    static_cast<float>(size.x),
                    0.f,
                    static_cast<float>(size.y),
                    -1.f,
                    1.f);
                _p->shader->setUniform("transform.mvp", projectionMatrix * viewMatrix * modelMatrix);

                auto glFuncs = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_3_Core>();
                glFuncs->glViewport(0, 0, size.x, size.y);
                glFuncs->glClearColor(0.f, 0.f, 0.f, 0.f);
                glFuncs->glClear(GL_COLOR_BUFFER_BIT);
                glFuncs->glActiveTexture(GL_TEXTURE0);

                _p->vao->bind();
                _p->vao->draw(0, 6);

                glFuncs->glPixelStorei(GL_PACK_ALIGNMENT, 1);
                glFuncs->glReadPixels(
                    0, 0, size.x, size.y,
                    info.getGLFormat(),
                    info.getGLType(),
                    out->getData());

                return out;
            }

	    } // namespace Keycode
	} // namespace AV
} // namespace djv
