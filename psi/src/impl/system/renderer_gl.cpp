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


class SystemGLRenderer : public psi_sys::ISystem {
public:
	SystemGLRenderer(psi_thread::TaskManager const& tasks, psi_serv::ServiceManager const& serv)
		: m_tasks(tasks)
		, m_serv(serv) {}

	uint64_t required_components() const override {
		// yay hardcoded values
		return psi_scene::component_type_entity_info.id | psi_scene::component_type_model_info.id | psi_scene::component_type_transform_info.id;
	}

	void on_scene_loaded(psi_scene::ISceneDirectAccess& acc) override {
		// enable face culling, counter-clockwise face is front
		gl::Enable(gl::CULL_FACE);
		gl::CullFace(gl::BACK);
		gl::FrontFace(gl::CCW);

		// ensure MSAA is enabled
		gl::Enable(gl::MULTISAMPLE);

		// convert linear shader output to sRGB in framebuffer
		gl::Enable(gl::FRAMEBUFFER_SRGB);

		// enable depth test
		gl::Enable(gl::DEPTH_TEST);
		gl::DepthMask(GLboolean(true));
		gl::DepthFunc(gl::LEQUAL);
		gl::DepthRange(0.0f, 1.0f);

		// enable depth clamping [0;1]
		//gl::Enable(gl::DEPTH_CLAMP);

		// setup color buffer clear value
		gl::ClearColor(1.0f, 0.5f, 0.5f, 1.0f);

		// setup depth buffer clear value
		gl::ClearDepth(1.0f);

		size_t entity_count = acc.component_count(psi_scene::component_type_entity_info.id);
		for (size_t i_ent = 0; i_ent < entity_count; ++i_ent) {
			auto ent = boost::any_cast<psi_scene::ComponentEntity const*>(acc.read_component(psi_scene::component_type_entity_info.id, i_ent));
			if (ent->model != psi_scene::NO_COMPONENT) {
				auto model = boost::any_cast<psi_scene::ComponentModel const*>(acc.read_component(psi_scene::component_type_model_info.id, ent->model));
				m_serv.resource_service().request_resource(std::hash<std::string>()(model->mesh_name.data()), std::hash<std::string>()(u8"mesh"), model->mesh_name.data());
				m_serv.resource_service().request_resource(std::hash<std::string>()(model->mat_name.data()), std::hash<std::string>()(u8"material"), model->mat_name.data());
			}
		}

		psi_gl::SamplerSettings set = {
			gl::LINEAR,
			gl::LINEAR_MIPMAP_LINEAR,
			gl::REPEAT,
			gl::REPEAT,
			16.0f
		};

		psi_gl::create_sampler_at_texture_unit(set, psi_gl::TextureUnit::DIFFUSE);
		psi_gl::create_sampler_at_texture_unit(set, psi_gl::TextureUnit::SPECULAR);
		psi_gl::create_sampler_at_texture_unit(set, psi_gl::TextureUnit::NORMAL);
		psi_gl::create_sampler_at_texture_unit(set, psi_gl::TextureUnit::CUBE);

		/* setup ambient light
		GLScene3D* scene = static_cast<GLScene3D*>(m_scene.get());
		scene->m_ambientLight.color = glm::vec3(0.0001f, 0.0001f, 0.0001f);
		scene->m_directionalLight.color = glm::vec3(1.0f, 1.0f, 1.0f);
		scene->m_directionalLight.dir_world = glm::vec3(0.0f, -1.0f, 0.0f);
		m_shaderPool.requestElement("shaders/bounding",
			[&] ()->ShaderSource* {
				return m_shaderSourceProvider->requestResource("shaders/bounding");
			}
		);

		m_shaderPool.requestElement("shaders/skybox",
			[&] ()->ShaderSource* {
				return m_shaderSourceProvider->requestResource("shaders/skybox");
			}
		);

		m_cubeTexPool.requestElement("textures/sky_debug",
			[&] ()->gli::texture* {
				return m_textureProvider->requestResource("textures/sky_debug");
			}
		);
		*/
	}

	void on_scene_update(psi_scene::ISceneDirectAccess& acc) override {
		gl::Clear(gl::COLOR_BUFFER_BIT | gl::DEPTH_BUFFER_BIT);

		m_cam.adjustAspectRatio(m_serv.window_service().aspect_ratio());


		size_t entity_count = acc.component_count(psi_scene::component_type_entity_info.id);
		for (size_t i_ent = 0; i_ent < entity_count; ++i_ent) {
			auto ent = boost::any_cast<psi_scene::ComponentEntity const*>(acc.read_component(psi_scene::component_type_entity_info.id, i_ent));
			if (ent->model != psi_scene::NO_COMPONENT && ent->transform != psi_scene::NO_COMPONENT) {
				auto model = boost::any_cast<psi_scene::ComponentModel const*>(acc.read_component(psi_scene::component_type_model_info.id, ent->model));
				auto trans = boost::any_cast<psi_scene::ComponentTransform const*>(acc.read_component(psi_scene::component_type_model_info.id, ent->transform));

				// bind deferred 4-FBO shader

				//auto const& mat = m_registered_materials[model->mat_name.data()];
				//mat.diffuse.bind_to_tex_unit(psi_gl::TextureUnit::DIFFUSE);
				////mat.specular.bind_to_tex_unit(psi_gl::TextureUnit::SPECULAR);
				//mat.normal.bind_to_tex_unit(psi_gl::TextureUnit::NORMAL);
				//mat.gloss.bind_to_tex_unit(psi_gl::TextureUnit::GLOSS);

				// submit uniforms..

				//m_registered_meshes[model->mesh_name.data()].draw(gl::TRIANGLES);


			}
		}

		// do lighting n' shit
		// how to deal with per-model lighting shaders n' shit?
	}

	void on_scene_save(psi_scene::ISceneDirectAccess&, void* replace_with_save_file) override {}

	void on_scene_shutdown(psi_scene::ISceneDirectAccess&) override {}

private:
	psi_thread::TaskManager const& m_tasks;
	psi_serv::ServiceManager const& m_serv;

	psi_rndr::Camera m_cam;

	//std::unordered_map<std::string, psi_gl::Shader> m_registered_shaders;
	//std::unordered_map<std::string, psi_gl::MeshBuffer> m_registered_meshes;
	//std::unordered_map<std::string, psi_gl::Material> m_registered_materials;
};

std::unique_ptr<psi_sys::ISystem> psi_sys::start_gl_renderer(psi_thread::TaskManager const& tasks, psi_serv::ServiceManager const& serv) {
	return std::make_unique<SystemGLRenderer>(tasks, serv);
}