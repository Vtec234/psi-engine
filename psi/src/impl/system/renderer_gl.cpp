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
		: _tasks(tasks)
		, _serv(serv)
		, _mrt_buf(std::vector<psi_gl::FramebufferRenderTargetCreationInfo>(), 2, 2) {}

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

		auto const& win = _serv.window_service();
		new (&_mrt_buf) psi_gl::MultipleRenderTargetFramebuffer(targets, win.width(), win.height());
	}

	enum class TextureUnit : GLuint {
		MRT_POSITION = 1,
		MRT_NORMAL = 2,
		MRT_ALBEDO = 3,
		MRT_REFLECTIVENESS_ROUGHNESS = 4,
		TEX_NORMAL = 5,
		TEX_ALBEDO = 6,
		TEX_REFLECTIVENESS_ROUGHNESS = 7,
	};

	void create_tex_samplers() {
		psi_gl::SamplerSettings set = {
			gl::LINEAR,
			gl::LINEAR_MIPMAP_LINEAR,
			gl::REPEAT,
			gl::REPEAT,
			16.0f,
		};

		psi_gl::create_sampler_at_texture_unit(set, GLuint(TextureUnit::TEX_NORMAL));
		psi_gl::create_sampler_at_texture_unit(set, GLuint(TextureUnit::TEX_ALBEDO));
		psi_gl::create_sampler_at_texture_unit(set, GLuint(TextureUnit::TEX_REFLECTIVENESS_ROUGHNESS));

		// frame G-Buffer samplers should filter from nearest pixel instead of linearly from several
		set.mag_filter = gl::NEAREST;
		set.min_filter = gl::NEAREST;

		psi_gl::create_sampler_at_texture_unit(set, GLuint(TextureUnit::MRT_POSITION));
		psi_gl::create_sampler_at_texture_unit(set, GLuint(TextureUnit::MRT_NORMAL));
		psi_gl::create_sampler_at_texture_unit(set, GLuint(TextureUnit::MRT_ALBEDO));
		psi_gl::create_sampler_at_texture_unit(set, GLuint(TextureUnit::MRT_REFLECTIVENESS_ROUGHNESS));

	}

	void register_input_handlers() {
		_serv.window_service().register_keyboard_input_callback(
			[this] (psi_serv::KeyboardInput k, psi_serv::InputAction a) {
				if (a == psi_serv::InputAction::PRESSED && k == psi_serv::KeyboardInput::B) {
					_serv.window_service().set_mouse_block(!_mouse_blocked);
					_mouse_blocked = !_mouse_blocked;
				}
			}
		);
	}

	void gl_3d_state_setup() {
		// enable face culling, counter-clockwise face is front
		gl::Enable(gl::CULL_FACE);
		gl::CullFace(gl::BACK);
		gl::FrontFace(gl::CCW);

		// ensure MSAA is enabled
		gl::Enable(gl::MULTISAMPLE);

		// enable depth test
		gl::Enable(gl::DEPTH_TEST);
		gl::DepthMask(true);
		gl::DepthFunc(gl::LEQUAL);
		gl::DepthRange(0.0f, 1.0f);

		// clear color has to black for position and normal vector buffers
		gl::ClearDepth(1.0f);
		gl::ClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	}

	void on_scene_loaded(psi_scene::ISceneDirectAccess& acc) override {
		register_input_handlers();

		gl_3d_state_setup();
		create_mrt_framebuffer();
		create_tex_samplers();

		std::hash<std::string> hash;
		_serv.resource_service().request_resource(hash(u8"deferred_geometry"), hash(u8"shader"), u8"glsl/deferred_geometry");
		{
			auto pass_first = *_serv.resource_service().retrieve_resource(hash(u8"deferred_geometry"));
			_compiled_shaders[u8"deferred_gbuffer"] = psi_gl::compile_glsl_source(boost::any_cast<psi_gl::GLSLSource>(pass_first->resource()));
		}

		_serv.resource_service().request_resource(hash(u8"deferred_quad"), hash(u8"shader"), u8"glsl/quad");
		{
			auto pass_second = *_serv.resource_service().retrieve_resource(hash(u8"deferred_quad"));
			_compiled_shaders[u8"deferred_quad"] = psi_gl::compile_glsl_source(boost::any_cast<psi_gl::GLSLSource>(pass_second->resource()));
		}

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

				_serv.resource_service().request_resource(hash(model->mesh_name.data()), hash(u8"mesh"), model->mesh_name.data());
				for (auto const& tex : textures) {
					_serv.resource_service().request_resource(hash(tex), hash(u8"texture"), tex);
				}

				// upload mesh to GL
				auto msh = *_serv.resource_service().retrieve_resource(hash(model->mesh_name.data()));
				psi_gl::MeshBuffer buf(boost::any_cast<psi_rndr::MeshData>(msh->resource()));
				_uploaded_meshes.emplace(model->mesh_name.data(), buf);

				// upload textures to GL
				for (auto const& tex : textures) {
					auto data = boost::any_cast<psi_rndr::TextureData>((*_serv.resource_service().retrieve_resource(hash(tex)))->resource());
					_uploaded_textures[tex] = psi_gl::upload_tex(data);
				}
			}
		}




		//  -- TEST --
		{
			_serv.resource_service().request_resource(hash(u8"meshes/cone_flat"), hash(u8"mesh"), u8"meshes/cone_flat");

			std::array<std::string, 3> textures = {{
				u8"textures/default",
				u8"textures/default_normal",
				u8"textures/default",
			}};

			for (auto const& tex : textures) {
				_serv.resource_service().request_resource(hash(tex), hash(u8"texture"), tex);
			}

			auto msh = *_serv.resource_service().retrieve_resource(hash(u8"meshes/cone_flat"));
			psi_gl::MeshBuffer buf(boost::any_cast<psi_rndr::MeshData>(msh->resource()));
			_uploaded_meshes.emplace(u8"meshes/cone_flat", buf);

			for (auto const& tex : textures) {
				auto data = boost::any_cast<psi_rndr::TextureData>((*_serv.resource_service().retrieve_resource(hash(tex)))->resource());
				_uploaded_textures[tex] = psi_gl::upload_tex(data);
			}
		}
		// -- END TEST --
	}

	void deferred_gbuffer_pass(psi_scene::ISceneDirectAccess& acc) {
		_mrt_buf.bind();

		gl::Clear(gl::COLOR_BUFFER_BIT | gl::DEPTH_BUFFER_BIT);

		_clip.adjust_aspect_ratio(_serv.window_service().aspect_ratio());

		auto const& sh = _compiled_shaders[u8"deferred_gbuffer"];
		gl::UseProgram(sh.handle);

		size_t entity_count = acc.component_count(psi_scene::component_type_entity_info.id);
		for (size_t i_ent = 0; i_ent < entity_count; ++i_ent) {
			auto ent = boost::any_cast<psi_scene::ComponentEntity const*>(acc.read_component(psi_scene::component_type_entity_info.id, i_ent));
			if (ent->model != psi_scene::NO_COMPONENT && ent->transform != psi_scene::NO_COMPONENT) {
				auto model = boost::any_cast<psi_scene::ComponentModel const*>(acc.read_component(psi_scene::component_type_model_info.id, ent->model));
				auto trans = boost::any_cast<psi_scene::ComponentTransform const*>(acc.read_component(psi_scene::component_type_model_info.id, ent->transform));

				gl::UniformMatrix4fv(sh.unifs.at(psi_gl::UniformMapping::LOCAL_TO_WORLD), 1, false, nullptr);
				gl::UniformMatrix4fv(sh.unifs.at(psi_gl::UniformMapping::LOCAL_TO_CLIP), 1, false, nullptr);

				gl::Uniform1i(sh.unifs.at(psi_gl::UniformMapping::ALBEDO_TEXTURE_SAMPLER), GLint(TextureUnit::TEX_ALBEDO));
				gl::Uniform1i(sh.unifs.at(psi_gl::UniformMapping::NORMAL_TEXTURE_SAMPLER), GLint(TextureUnit::TEX_NORMAL));
				gl::Uniform1i(sh.unifs.at(psi_gl::UniformMapping::REFLECTIVENESS_ROUGHNESS_TEXTURE_SAMPLER), GLint(TextureUnit::TEX_REFLECTIVENESS_ROUGHNESS));
			}
		}

		Eigen::Matrix4f unity;
		unity << 1, 0, 0, 0,
		         0, 1, 0, 0,
				 0, 0, 1, 0,
				 0, 0, 0, 1;
		gl::UniformMatrix4fv(sh.unifs.at(psi_gl::UniformMapping::LOCAL_TO_WORLD), 1, false, unity.data());
		Eigen::Matrix4f local_to_clip = _clip.to_clip() * _cam.world_to_local();
		gl::UniformMatrix4fv(sh.unifs.at(psi_gl::UniformMapping::LOCAL_TO_CLIP), 1, false, local_to_clip.data());

		gl::ActiveTexture(gl::TEXTURE0 + GLint(TextureUnit::TEX_ALBEDO));
		gl::BindTexture(gl::TEXTURE_2D, _uploaded_textures.at(u8"textures/default"));
		gl::ActiveTexture(gl::TEXTURE0 + GLint(TextureUnit::TEX_REFLECTIVENESS_ROUGHNESS));
		gl::BindTexture(gl::TEXTURE_2D, _uploaded_textures.at(u8"textures/default"));
		gl::ActiveTexture(gl::TEXTURE0 + GLint(TextureUnit::TEX_NORMAL));
		gl::BindTexture(gl::TEXTURE_2D, _uploaded_textures.at(u8"textures/default_normal"));
		gl::Uniform1i(sh.unifs.at(psi_gl::UniformMapping::ALBEDO_TEXTURE_SAMPLER), GLint(TextureUnit::TEX_ALBEDO));
		gl::Uniform1i(sh.unifs.at(psi_gl::UniformMapping::NORMAL_TEXTURE_SAMPLER), GLint(TextureUnit::TEX_NORMAL));
		gl::Uniform1i(sh.unifs.at(psi_gl::UniformMapping::REFLECTIVENESS_ROUGHNESS_TEXTURE_SAMPLER), GLint(TextureUnit::TEX_REFLECTIVENESS_ROUGHNESS));

		_uploaded_meshes.at(u8"meshes/cone_flat").draw(gl::TRIANGLES);

		_mrt_buf.unbind();
	}

	void deferred_lighting_pass() {
		gl::Clear(gl::COLOR_BUFFER_BIT | gl::DEPTH_BUFFER_BIT);

		gl::UseProgram(_compiled_shaders[u8"deferred_quad"].handle);

		gl::ActiveTexture(gl::TEXTURE0 + GLint(TextureUnit::MRT_POSITION));
		gl::BindTexture(gl::TEXTURE_2D, _mrt_buf.texture_target_handle(0));
		gl::ActiveTexture(gl::TEXTURE0 + GLint(TextureUnit::MRT_NORMAL));
		gl::BindTexture(gl::TEXTURE_2D, _mrt_buf.texture_target_handle(1));
		gl::ActiveTexture(gl::TEXTURE0 + GLint(TextureUnit::MRT_ALBEDO));
		gl::BindTexture(gl::TEXTURE_2D, _mrt_buf.texture_target_handle(2));
		gl::ActiveTexture(gl::TEXTURE0 + GLint(TextureUnit::MRT_REFLECTIVENESS_ROUGHNESS));
		gl::BindTexture(gl::TEXTURE_2D, _mrt_buf.texture_target_handle(3));

		gl::Uniform1i(_compiled_shaders[u8"deferred_quad"].unifs.at(psi_gl::UniformMapping::POSITION_FRAME_TEXTURE_SAMPLER), GLint(TextureUnit::MRT_POSITION));
		gl::Uniform1i(_compiled_shaders[u8"deferred_quad"].unifs.at(psi_gl::UniformMapping::NORMAL_FRAME_TEXTURE_SAMPLER), GLint(TextureUnit::MRT_NORMAL));
		gl::Uniform1i(_compiled_shaders[u8"deferred_quad"].unifs.at(psi_gl::UniformMapping::ALBEDO_FRAME_TEXTURE_SAMPLER), GLint(TextureUnit::MRT_ALBEDO));
		gl::Uniform1i(_compiled_shaders[u8"deferred_quad"].unifs.at(psi_gl::UniformMapping::REFL_ROUGH_FRAME_TEXTURE_SAMPLER), GLint(TextureUnit::MRT_REFLECTIVENESS_ROUGHNESS));

		gl::DrawArrays(gl::TRIANGLE_STRIP, 0, 4);
	}

	void handle_input() {
		auto mouse = _serv.window_service().mouse_pos();
		_mouse_prev_x = _mouse_x;
		_mouse_prev_y = _mouse_y;
		_mouse_x = mouse.first;
		_mouse_y = mouse.second;

		auto keys = _serv.window_service().active_keyboard_inputs();
		for (auto k : keys) {
			if (k == psi_serv::KeyboardInput::W)
				_cam.translate_in_local({0.0f, 0.0f, -0.2f});
			if (k == psi_serv::KeyboardInput::S)
				_cam.translate_in_local({0.0f, 0.0f, +0.2f});
			if (k == psi_serv::KeyboardInput::A)
				_cam.translate_in_local({-0.2f, 0.0f, 0.0f});
			if (k == psi_serv::KeyboardInput::D)
				_cam.translate_in_local({+0.2f, 0.0f, 0.0f});
			if (k == psi_serv::KeyboardInput::Q)
				_cam.translate_in_local({0.0f, -0.2f, 0.0f});
			if (k == psi_serv::KeyboardInput::E)
				_cam.translate_in_local({0.0f, +0.2f, 0.0f});
		}

		if (_mouse_blocked) {
			_cam.rotate_in_local({-1.0f, 0.0f, 0.0f}, (_mouse_y - _mouse_prev_y)/50.0f);
			_cam.rotate_in_world({0.0f, -1.0f, 0.0f}, (_mouse_x - _mouse_prev_x)/50.0f);
		}
	}

	void on_scene_update(psi_scene::ISceneDirectAccess& acc) override {
		// TODO handle changes in scene

		auto width = _serv.window_service().width();
	 	auto height = _serv.window_service().height();

		if (_frame_width != width || _frame_height != height) {
			_frame_width = width;
			_frame_height = height;
			create_mrt_framebuffer();
			gl::Viewport(0, 0, width, height);
		}

		handle_input();

		deferred_gbuffer_pass(acc);

		deferred_lighting_pass();
	}

	void on_scene_save(psi_scene::ISceneDirectAccess&, void* /*replace_with_save_file*/) override {}

	void on_scene_shutdown(psi_scene::ISceneDirectAccess&) override {}

private:
	psi_thread::TaskManager const& _tasks;
	psi_serv::ServiceManager const& _serv;

	psi_rndr::IsometricTransform _cam;
	psi_rndr::ClipMatrix _clip;

	std::unordered_map<std::string, GLuint> _uploaded_textures;
	std::unordered_map<std::string, psi_gl::MeshBuffer> _uploaded_meshes;
	std::unordered_map<std::string, psi_gl::Shader> _compiled_shaders;

	psi_gl::MultipleRenderTargetFramebuffer _mrt_buf;

	double _mouse_x;
	double _mouse_y;
	double _mouse_prev_x;
	double _mouse_prev_y;

	uint32_t _frame_width;
	uint32_t _frame_height;

	bool _mouse_blocked = true;
};

std::unique_ptr<psi_sys::ISystem> psi_sys::start_gl_renderer(psi_thread::TaskManager const& tasks, psi_serv::ServiceManager const& serv) {
	return std::make_unique<SystemGLRenderer>(tasks, serv);
}
