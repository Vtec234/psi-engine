/*
 * Copyright (C) 2015-2016 Wojciech Nawrocki
 *
 * This file is part of Psi Engine.
 *
 * Psi Engine is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Psi Engine is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Psi Engine. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "renderer_gl.hpp"

#include <string>
#include <codecvt>
#include <locale>

#include "../rendering/gl/gl.hpp"
#include "../rendering/gl/helper.hpp"
#include "../../scene/access.hpp"
#include "../../scene/components.hpp"
#include "../scene/default_components.hpp"
#include "../rendering/camera.hpp"
#include "../../log/log.hpp"


class SystemGLRenderer : public psi_sys::ISystem {
public:
	SystemGLRenderer(psi_thread::TaskManager const& tasks, psi_serv::ServiceManager const& serv)
		: m_tasks(tasks)
		, m_serv(serv) {}

	uint64_t required_components() const override {
		return psi_scene::component_type_entity_info.id | psi_scene::component_type_model_info.id | psi_scene::component_type_transform_info.id;
	}

	void gl_state_setup() {
		// enable face culling, counter-clockwise face is front
		gl::Enable(gl::CULL_FACE);
		gl::CullFace(gl::BACK);
		gl::FrontFace(gl::CCW);

		// ensure MSAA is enabled
		// TODO does MSAA work with deferred?
		gl::Enable(gl::MULTISAMPLE);

		// enable depth test
		gl::Enable(gl::DEPTH_TEST);
		gl::DepthMask(true);
		gl::DepthFunc(gl::LEQUAL);
		gl::DepthRange(0.0f, 1.0f);

		// setup color & depth buffer clear values
		gl::ClearColor(1.0f, 0.5f, 0.5f, 1.0f);
		gl::ClearDepth(1.0f);

		// setup MRT framebuffer and texture
		gl::GenFramebuffers(1, &m_mrt_framebuffer);
		gl::BindFramebuffer(gl::FRAMEBUFFER, m_mrt_framebuffer);

		// position vectors
		gl::GenTextures(1, &m_pos_frame_tex);
		gl::BindTexture(gl::TEXTURE_2D, m_pos_frame_tex);
		gl::TexImage2D(gl::TEXTURE_2D, 0, gl::RGB16F, m_serv.window_service().width(), m_serv.window_service().height(), 0, gl::RGB, gl::FLOAT, 0);
		gl::FramebufferTexture2D(gl::FRAMEBUFFER, psi_gl::POS_ATTACHMENT, gl::TEXTURE_2D, m_pos_frame_tex, 0);

		// normal vectors
		gl::GenTextures(1, &m_norm_frame_tex);
		gl::BindTexture(gl::TEXTURE_2D, m_norm_frame_tex);
		gl::TexImage2D(gl::TEXTURE_2D, 0, gl::RGB16F, m_serv.window_service().width(), m_serv.window_service().height(), 0, gl::RGB, gl::FLOAT, 0);
		gl::FramebufferTexture2D(gl::FRAMEBUFFER, psi_gl::NORM_ATTACHMENT, gl::TEXTURE_2D, m_norm_frame_tex, 0);

		// albedo
		gl::GenTextures(1, &m_albedo_frame_tex);
		gl::BindTexture(gl::TEXTURE_2D, m_albedo_frame_tex);
		gl::TexImage2D(gl::TEXTURE_2D, 0, gl::RGBA, m_serv.window_service().width(), m_serv.window_service().height(), 0, gl::RGBA, gl::UNSIGNED_BYTE, 0);
		gl::FramebufferTexture2D(gl::FRAMEBUFFER, psi_gl::ALBEDO_ATTACHMENT, gl::TEXTURE_2D, m_albedo_frame_tex, 0);

		// reflectiveness (coloured metalicness) in RGB, roughness in A
		gl::GenTextures(1, &m_refl_rough_frame_tex);
		gl::BindTexture(gl::TEXTURE_2D, m_refl_rough_frame_tex);
		gl::TexImage2D(gl::TEXTURE_2D, 0, gl::RGBA, m_serv.window_service().width(), m_serv.window_service().height(), 0, gl::RGBA, gl::UNSIGNED_BYTE, 0);
		gl::FramebufferTexture2D(gl::FRAMEBUFFER, psi_gl::RR_ATTACHMENT, gl::TEXTURE_2D, m_refl_rough_frame_tex, 0);

		// initialize samplers at texture units
		psi_gl::SamplerSettings set = {
			gl::LINEAR,
			gl::LINEAR_MIPMAP_LINEAR,
			gl::REPEAT,
			gl::REPEAT,
			16.0f
		};

		psi_gl::create_sampler_at_texture_unit(set, psi_gl::TextureUnit::NORM_MAP);
		psi_gl::create_sampler_at_texture_unit(set, psi_gl::TextureUnit::ALBEDO);
		psi_gl::create_sampler_at_texture_unit(set, psi_gl::TextureUnit::REFL_ROUGH);

		// frame G-Buffer samplers should filter from nearest pixel instead of linearly from several
		set.mag_filter = gl::NEAREST;
		set.min_filter = gl::NEAREST;

		psi_gl::create_sampler_at_texture_unit(set, psi_gl::TextureUnit::POS_FRAME);
		psi_gl::create_sampler_at_texture_unit(set, psi_gl::TextureUnit::NORM_FRAME);
		psi_gl::create_sampler_at_texture_unit(set, psi_gl::TextureUnit::ALBEDO_FRAME);
		psi_gl::create_sampler_at_texture_unit(set, psi_gl::TextureUnit::REFL_ROUGH_FRAME);
	}

	void gl_state_cleanup() {
		// TODO
	}

	void on_scene_loaded(psi_scene::ISceneDirectAccess& acc) override {
		gl_state_setup();

		std::hash<std::string> hash;
		m_serv.resource_service().request_resource(hash(u8"deferred_geometry"), hash(u8"shader"), u8"glsl/deferred_geometry");
		auto src = *m_serv.resource_service().retrieve_resource(hash(u8"deferred_geometry"));
		m_compiled_shaders[u8"deferred_gbuffer"] = psi_gl::compile_glsl_source(boost::any_cast<psi_gl::GLSLSource>(src->resource()));

		size_t entity_count = acc.component_count(psi_scene::component_type_entity_info.id);
		for (size_t i_ent = 0; i_ent < entity_count; ++i_ent) {
			auto ent = boost::any_cast<psi_scene::ComponentEntity const*>(acc.read_component(psi_scene::component_type_entity_info.id, i_ent));
			if (ent->model != psi_scene::NO_COMPONENT) {
				auto model = boost::any_cast<psi_scene::ComponentModel const*>(acc.read_component(psi_scene::component_type_model_info.id, ent->model));
				std::array<std::string, 3> textures = {{
					model->albedo_tex.data(),
					model->normal_tex.data(),
					model->reflectiveness_roughness_tex.data()
				}};

				m_serv.resource_service().request_resource(hash(model->mesh_name.data()), hash(u8"mesh"), model->mesh_name.data());
				for (auto const& tex : textures) {
					m_serv.resource_service().request_resource(hash(tex), hash(u8"texture"), tex);
				}

				// upload mesh to GL
				auto msh = *m_serv.resource_service().retrieve_resource(hash(model->mesh_name.data()));
				psi_gl::MeshBuffer buf(boost::any_cast<psi_rndr::MeshData>(msh->resource()));
				m_uploaded_meshes.emplace(model->mesh_name.data(), buf);

				// upload textures to GL
				for (auto const& tex : textures) {
					auto data = boost::any_cast<psi_rndr::TextureData>((*m_serv.resource_service().retrieve_resource(hash(tex)))->resource());
					m_uploaded_textures[model->albedo_tex.data()] = psi_gl::upload_tex(data);
				}
			}
		}

	}

	void on_scene_update(psi_scene::ISceneDirectAccess& acc) override {
		// TODO handle changes in scene

		gl::BindFramebuffer(gl::FRAMEBUFFER, m_mrt_framebuffer);

		// specify all framebuffer attachments to be rendered to
		GLuint attachments[4] = { psi_gl::POS_ATTACHMENT, psi_gl::NORM_ATTACHMENT, psi_gl::ALBEDO_ATTACHMENT, psi_gl::RR_ATTACHMENT, };
		gl::DrawBuffers(4, attachments);

		gl::Clear(gl::COLOR_BUFFER_BIT | gl::DEPTH_BUFFER_BIT);

		m_cam.adjustAspectRatio(m_serv.window_service().aspect_ratio());

		auto const& sh = m_compiled_shaders[u8"deferred_gbuffer"];
		gl::UseProgram(sh.handle);

		size_t entity_count = acc.component_count(psi_scene::component_type_entity_info.id);
		for (size_t i_ent = 0; i_ent < entity_count; ++i_ent) {
			auto ent = boost::any_cast<psi_scene::ComponentEntity const*>(acc.read_component(psi_scene::component_type_entity_info.id, i_ent));
			if (ent->model != psi_scene::NO_COMPONENT && ent->transform != psi_scene::NO_COMPONENT) {
				auto model = boost::any_cast<psi_scene::ComponentModel const*>(acc.read_component(psi_scene::component_type_model_info.id, ent->model));
				auto trans = boost::any_cast<psi_scene::ComponentTransform const*>(acc.read_component(psi_scene::component_type_model_info.id, ent->transform));

				gl::UniformMatrix3fv(sh.unifs.at(psi_gl::UniformMapping::LOCAL_TO_WORLD), 1, false, nullptr);
				gl::UniformMatrix3fv(sh.unifs.at(psi_gl::UniformMapping::LOCAL_TO_CLIP), 1, false, nullptr);

				gl::Uniform1i(sh.unifs.at(psi_gl::UniformMapping::ALBEDO_TEXTURE_SAMPLER), GLint(psi_gl::TextureUnit::ALBEDO));
				gl::Uniform1i(sh.unifs.at(psi_gl::UniformMapping::NORMAL_TEXTURE_SAMPLER), GLint(psi_gl::TextureUnit::NORM_MAP));
				gl::Uniform1i(sh.unifs.at(psi_gl::UniformMapping::REFLECTIVENESS_ROUGHNESS_TEXTURE_SAMPLER), GLint(psi_gl::TextureUnit::REFL_ROUGH));
			}
		}
	}

	void on_scene_save(psi_scene::ISceneDirectAccess&, void* replace_with_save_file) override {}

	void on_scene_shutdown(psi_scene::ISceneDirectAccess&) override {}

private:
	psi_thread::TaskManager const& m_tasks;
	psi_serv::ServiceManager const& m_serv;

	psi_rndr::Camera m_cam;

	std::unordered_map<std::string, GLuint> m_uploaded_textures;
	std::unordered_map<std::string, psi_gl::MeshBuffer> m_uploaded_meshes;
	std::unordered_map<std::string, psi_gl::Shader> m_compiled_shaders;

	GLuint m_mrt_framebuffer;
	GLuint m_pos_frame_tex;
	GLuint m_norm_frame_tex;
	GLuint m_albedo_frame_tex;
	GLuint m_refl_rough_frame_tex;
};

std::unique_ptr<psi_sys::ISystem> psi_sys::start_gl_renderer(psi_thread::TaskManager const& tasks, psi_serv::ServiceManager const& serv) {
	return std::make_unique<SystemGLRenderer>(tasks, serv);
}
