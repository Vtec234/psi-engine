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
		, m_serv(serv)
		, m_mrt_buf(std::vector<psi_gl::FramebufferRenderTargetCreationInfo>(), 2, 2) {}

	uint64_t required_components() const override {
		return psi_scene::component_type_entity_info.id | psi_scene::component_type_model_info.id | psi_scene::component_type_transform_info.id;
	}

	void create_mrt_framebuffer() {
		std::vector<psi_gl::FramebufferRenderTargetCreationInfo> targets = {
			{ true, true, gl::RGB16F, },
			{ true, true, gl::RGB16F, },
			{ true, true, gl::RGBA, },
			{ true, true, gl::RGBA, },
			{ false, false, gl::DEPTH_COMPONENT, },
		};

		auto const& win = m_serv.window_service();
		new (&m_mrt_buf) psi_gl::MultipleRenderTargetFramebuffer(targets, win.width(), win.height());
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
		gl::ClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		gl::ClearDepth(1.0f);

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
		create_mrt_framebuffer();
		m_frame_width = m_serv.window_service().width();
		m_frame_height = m_serv.window_service().height();

		std::hash<std::string> hash;
		m_serv.resource_service().request_resource(hash(u8"deferred_geometry"), hash(u8"shader"), u8"glsl/deferred_geometry");
		auto src = *m_serv.resource_service().retrieve_resource(hash(u8"deferred_geometry"));
		m_compiled_shaders[u8"deferred_gbuffer"] = psi_gl::compile_glsl_source(boost::any_cast<psi_gl::GLSLSource>(src->resource()));

		m_serv.resource_service().request_resource(hash(u8"deferred_quad"), hash(u8"shader"), u8"glsl/quad");
		auto src2 = *m_serv.resource_service().retrieve_resource(hash(u8"deferred_quad"));
		m_compiled_shaders[u8"deferred_quad"] = psi_gl::compile_glsl_source(boost::any_cast<psi_gl::GLSLSource>(src2->resource()));

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
					m_uploaded_textures[tex] = psi_gl::upload_tex(data);
				}
			}
		}

		// TEST
		{
			m_serv.resource_service().request_resource(hash(u8"meshes/cone_flat"), hash(u8"mesh"), u8"meshes/cone_flat");

			std::array<std::string, 3> textures = {{
				u8"textures/default",
				u8"textures/default_normal",
				u8"textures/default",
			}};

			for (auto const& tex : textures) {
				m_serv.resource_service().request_resource(hash(tex), hash(u8"texture"), tex);
			}

			auto msh = *m_serv.resource_service().retrieve_resource(hash(u8"meshes/cone_flat"));
			psi_gl::MeshBuffer buf(boost::any_cast<psi_rndr::MeshData>(msh->resource()));
			m_uploaded_meshes.emplace(u8"meshes/cone_flat", buf);

			for (auto const& tex : textures) {
				auto data = boost::any_cast<psi_rndr::TextureData>((*m_serv.resource_service().retrieve_resource(hash(tex)))->resource());
				m_uploaded_textures[tex] = psi_gl::upload_tex(data);
			}
		}
	}

	void on_scene_update(psi_scene::ISceneDirectAccess& acc) override {
		// TODO handle changes in scene

		auto const& win = m_serv.window_service();
		if (m_frame_width != win.width() || m_frame_height != win.height()) {
			m_frame_width = win.width();
			m_frame_height = win.height();
			create_mrt_framebuffer();
			gl::Viewport(0, 0, m_frame_width, m_frame_height);
		}

		// TEST
		{
			auto mouse = m_serv.window_service().mouse_pos();
			m_mouse_prev_x = m_mouse_x;
			m_mouse_prev_y = m_mouse_y;
			m_mouse_x = mouse.first;
			m_mouse_y = mouse.second;

			auto keys = m_serv.window_service().active_keyboard_inputs();
			for (auto k : keys) {
				if (k == psi_serv::KeyboardInput::W)
					m_cam.moveZ(+0.2f);
				if (k == psi_serv::KeyboardInput::S)
					m_cam.moveZ(-0.2f);
				if (k == psi_serv::KeyboardInput::A)
					m_cam.moveX(-0.2f);
				if (k == psi_serv::KeyboardInput::D)
					m_cam.moveX(+0.2f);
				if (k == psi_serv::KeyboardInput::Q)
					m_cam.moveY(-0.2f);
				if (k == psi_serv::KeyboardInput::E)
					m_cam.moveY(+0.2f);
			}
			m_serv.window_service().register_keyboard_input_callback(
				[this] (psi_serv::KeyboardInput k, psi_serv::InputAction a) {
					if (a == psi_serv::InputAction::PRESSED && k == psi_serv::KeyboardInput::B) {
						m_serv.window_service().set_mouse_block(!m_mouse_blocked);
						m_mouse_blocked = !m_mouse_blocked;
					}
				}
			);
		}
		if (m_mouse_blocked) {
			psi_log::debug("renderer_gl")
					<< m_cam.cameraPosition().x
					<< " "
					<< m_cam.cameraPosition().y
					<< " "\
					<< m_cam.cameraPosition().z
					<< " "
					<< m_cam.rotatePitch((m_mouse_y - m_mouse_prev_y)/50.0f)
					<< " "
					<< m_cam.rotateYaw((m_mouse_x - m_mouse_prev_x)/50.0f)
					<< "\n";
		}

		m_mrt_buf.bind();

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

				gl::UniformMatrix4fv(sh.unifs.at(psi_gl::UniformMapping::LOCAL_TO_WORLD), 1, false, nullptr);
				gl::UniformMatrix4fv(sh.unifs.at(psi_gl::UniformMapping::LOCAL_TO_CLIP), 1, false, nullptr);

				gl::Uniform1i(sh.unifs.at(psi_gl::UniformMapping::ALBEDO_TEXTURE_SAMPLER), GLint(psi_gl::TextureUnit::ALBEDO));
				gl::Uniform1i(sh.unifs.at(psi_gl::UniformMapping::NORMAL_TEXTURE_SAMPLER), GLint(psi_gl::TextureUnit::NORM_MAP));
				gl::Uniform1i(sh.unifs.at(psi_gl::UniformMapping::REFLECTIVENESS_ROUGHNESS_TEXTURE_SAMPLER), GLint(psi_gl::TextureUnit::REFL_ROUGH));

			}
		}

		psi_scene::ComponentEntity ent;
		ent.transform = 0;
		ent.model = 0;
		ent.experiences_causality = true;

		psi_scene::ComponentTransform tr;
		tr.pos = {{ 0.0f, -10.0f, 0.0f }};
		tr.scale = {{ 1.0f, 1.0f, 1.0f }};
		tr.orientation = {{ 1.0f, 0.0f, 0.0f, 0.0f }};

		std::array<float, 16> unity =
		{{
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, -10.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f,
		}};
		gl::UniformMatrix4fv(sh.unifs.at(psi_gl::UniformMapping::LOCAL_TO_WORLD), 1, false, unity.data());
		gl::UniformMatrix4fv(sh.unifs.at(psi_gl::UniformMapping::LOCAL_TO_CLIP), 1, false, &(m_cam.cameraToClipMat4() * m_cam.worldToCameraMat4())[0][0]);

		gl::ActiveTexture(gl::TEXTURE0 + GLint(psi_gl::TextureUnit::ALBEDO));
		gl::BindTexture(gl::TEXTURE_2D, m_uploaded_textures.at(u8"textures/default"));
		gl::ActiveTexture(gl::TEXTURE0 + GLint(psi_gl::TextureUnit::REFL_ROUGH));
		gl::BindTexture(gl::TEXTURE_2D, m_uploaded_textures.at(u8"textures/default"));
		gl::ActiveTexture(gl::TEXTURE0 + GLint(psi_gl::TextureUnit::NORM_MAP));
		gl::BindTexture(gl::TEXTURE_2D, m_uploaded_textures.at(u8"textures/default_normal"));
		gl::Uniform1i(sh.unifs.at(psi_gl::UniformMapping::ALBEDO_TEXTURE_SAMPLER), GLint(psi_gl::TextureUnit::ALBEDO));
		gl::Uniform1i(sh.unifs.at(psi_gl::UniformMapping::NORMAL_TEXTURE_SAMPLER), GLint(psi_gl::TextureUnit::NORM_MAP));
		gl::Uniform1i(sh.unifs.at(psi_gl::UniformMapping::REFLECTIVENESS_ROUGHNESS_TEXTURE_SAMPLER), GLint(psi_gl::TextureUnit::REFL_ROUGH));

		m_uploaded_meshes.at(u8"meshes/cone_flat").draw(gl::TRIANGLES);


		// -- SECOND PASS --
		m_mrt_buf.unbind();
		gl::Clear(gl::COLOR_BUFFER_BIT | gl::DEPTH_BUFFER_BIT);

		gl::UseProgram(m_compiled_shaders[u8"deferred_quad"].handle);

		gl::ActiveTexture(gl::TEXTURE0 + GLint(psi_gl::TextureUnit::POS_FRAME));
		gl::BindTexture(gl::TEXTURE_2D, m_mrt_buf.texture_target_handle(0));
		gl::ActiveTexture(gl::TEXTURE0 + GLint(psi_gl::TextureUnit::NORM_FRAME));
		gl::BindTexture(gl::TEXTURE_2D, m_mrt_buf.texture_target_handle(1));
		gl::ActiveTexture(gl::TEXTURE0 + GLint(psi_gl::TextureUnit::ALBEDO_FRAME));
		gl::BindTexture(gl::TEXTURE_2D, m_mrt_buf.texture_target_handle(2));
		gl::ActiveTexture(gl::TEXTURE0 + GLint(psi_gl::TextureUnit::REFL_ROUGH_FRAME));
		gl::BindTexture(gl::TEXTURE_2D, m_mrt_buf.texture_target_handle(3));

		gl::Uniform1i(m_compiled_shaders[u8"deferred_quad"].unifs.at(psi_gl::UniformMapping::POSITION_FRAME_TEXTURE_SAMPLER), GLint(psi_gl::TextureUnit::POS_FRAME));
		gl::Uniform1i(m_compiled_shaders[u8"deferred_quad"].unifs.at(psi_gl::UniformMapping::NORMAL_FRAME_TEXTURE_SAMPLER), GLint(psi_gl::TextureUnit::NORM_FRAME));
		gl::Uniform1i(m_compiled_shaders[u8"deferred_quad"].unifs.at(psi_gl::UniformMapping::ALBEDO_FRAME_TEXTURE_SAMPLER), GLint(psi_gl::TextureUnit::ALBEDO_FRAME));
		gl::Uniform1i(m_compiled_shaders[u8"deferred_quad"].unifs.at(psi_gl::UniformMapping::REFL_ROUGH_FRAME_TEXTURE_SAMPLER), GLint(psi_gl::TextureUnit::REFL_ROUGH_FRAME));

		gl::DrawArrays(gl::TRIANGLE_STRIP, 0, 4);
	}

	void on_scene_save(psi_scene::ISceneDirectAccess&, void* /*replace_with_save_file*/) override {}

	void on_scene_shutdown(psi_scene::ISceneDirectAccess&) override {}

private:
	psi_thread::TaskManager const& m_tasks;
	psi_serv::ServiceManager const& m_serv;

	psi_rndr::Camera m_cam;

	std::unordered_map<std::string, GLuint> m_uploaded_textures;
	std::unordered_map<std::string, psi_gl::MeshBuffer> m_uploaded_meshes;
	std::unordered_map<std::string, psi_gl::Shader> m_compiled_shaders;

	psi_gl::MultipleRenderTargetFramebuffer m_mrt_buf;

	double m_mouse_x;
	double m_mouse_y;
	double m_mouse_prev_x;
	double m_mouse_prev_y;

	uint32_t m_frame_width;
	uint32_t m_frame_height;

	bool m_mouse_blocked = true;
};

std::unique_ptr<psi_sys::ISystem> psi_sys::start_gl_renderer(psi_thread::TaskManager const& tasks, psi_serv::ServiceManager const& serv) {
	return std::make_unique<SystemGLRenderer>(tasks, serv);
}
