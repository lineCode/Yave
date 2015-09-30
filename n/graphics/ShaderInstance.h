/*******************************
Copyright (C) 2013-2015 gregoire ANGERAND

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
**********************************/

#ifndef N_GRAPHICS_SHADERINSTANCE_H
#define N_GRAPHICS_SHADERINSTANCE_H

#include "Shader.h"
#include "ShaderProgram.h"

namespace n {
namespace graphics {

class ShaderLinkingException : public std::exception
{
	public:
		virtual const char *what() const throw() override {
			return m.toChar();
		}

	private:
		friend class ShaderInstance;

		ShaderLinkingException(const core::String &msg) : std::exception(), m(msg) {
		}

		const core::String m;
};

class ShaderInstance : core::NonCopyable
{
	public:
		typedef gl::GLint UniformAddr;

	private:
		struct UniformInfo
		{
			UniformAddr addr;
			uint size;
		};

	public:
		ShaderInstance(Shader<FragmentShader> *frag, Shader<VertexShader> *vert, Shader<GeometryShader> *geom = 0);
		ShaderInstance(Shader<FragmentShader> *frag, ShaderProgram::StandardVertexShader vert, Shader<GeometryShader> *geom = 0);
		~ShaderInstance();

		void bind();
		void rebind();
		void unbind();

		static const ShaderInstance *getCurrent();
		static void validateState();

		void setValue(UniformAddr addr, const int *a, uint count) const;
		void setValue(UniformAddr addr, const uint *a, uint count) const;
		void setValue(UniformAddr addr, const float *f, uint count) const;
		void setValue(UniformAddr addr, const math::Vec2i *v, uint count) const;
		void setValue(UniformAddr addr, const math::Vec3i *v, uint count) const;
		void setValue(UniformAddr addr, const math::Vec2 *v, uint count) const;
		void setValue(UniformAddr addr, const math::Vec3 *v, uint count) const;
		void setValue(UniformAddr addr, const math::Vec4 *v, uint count) const;
		void setValue(UniformAddr addr, const math::Matrix2<float> *m, uint count) const;
		void setValue(UniformAddr addr, const math::Matrix3<float> *m, uint count) const;
		void setValue(UniformAddr addr, const math::Matrix4<float> *m, uint count) const;
		void setValue(UniformAddr slot, const Texture &t, TextureSampler sampler = TextureSampler::Default) const;

		template<typename T>
		void setValue(UniformAddr addr, const T &t) const {
			setValue(addr, &t, 1);
		}

		void setValue(UniformAddr addr, const double &t) const {
			setValue(addr, float(t));
		}

		template<typename T, typename... Args>
		void setValue(const core::String &name, const T &t, Args... args) const {
			UniformAddr i = getAddr(name);
			if(i != UniformAddr(GL_INVALID_INDEX)) {
				setValue(i, t, args...);
			}
		}

		template<typename T, typename... Args>
		void setValue(ShaderValue v, const T &t, Args... args) const {
			setValue(ShaderValueName[v], t, args...);
		}

		UniformAddr getAddr(const core::String &name) const;
		UniformInfo getInfo(const core::String &name) const;


	private:
		friend class ShaderProgram;
		friend class GLContext;

		void bindStandards() const;
		void bindTextures() const;

		void compile();
		void getUniforms();
		UniformAddr computeStandardIndex(const core::String &name);

		gl::GLuint handle;

		uint samplerCount;
		internal::TextureBinding *bindings;
		UniformAddr standards[SVMax];
		core::Hash<core::String, UniformInfo> uniformsInfo;


		ShaderBase *bases[3];


		static ShaderInstance *current;
};

typedef ShaderInstance ShaderCombinaison;


}
}

#endif // SHADERINSTANCE_H
