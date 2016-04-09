/*
 * Copyright (C) 2015 Wojciech Nawrocki
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <cstdlib>
#include <string>
#include <vector>
#include <cstdint>
#include <regex>
#include <iostream>
#include <fstream>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;
#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <openssl/md5.h>

#include <resource.hpp>


static Assimp::Importer importer;


/// Tries to load the specified mesh file
/// and save a converted version to the specified output directory.
/// @throw if anything goes wrong during the process
void load_and_serialize_mesh(fs::path const& file, fs::path const& out_dir) {
	std::ifstream in_stream(file.string());
	if (in_stream.fail()) {
		// exit and declare error if file can't be found
		in_stream.close();
		throw std::runtime_error("File is invalid or doesn't exist.");
	}

	in_stream.close();

	aiScene const* scene = importer.ReadFile(file.string(), aiProcessPreset_TargetRealtime_MaxQuality);

	if (!scene)
		throw std::runtime_error(std::string("File couldn't be imported. Assimp error:\n") + importer.GetErrorString());

	if (!scene->HasMeshes())
		throw std::runtime_error("File doesn't contain any meshes.");

	// import all meshes
	for (size_t i_mesh = 0; i_mesh < scene->mNumMeshes; i_mesh++) {
		// pointer to current mesh
		aiMesh* mesh = scene->mMeshes[i_mesh];

		// check if there is necessary data in this mesh
		if (!mesh->HasPositions()) {
			std::cout << std::string("Mesh ") + mesh->mName.C_Str() + " does not have vertex positions.\n";
			continue;
		}

		if (!mesh->HasNormals()) {
			std::cout << std::string("Mesh ") + mesh->mName.C_Str() + " does not have vertex normals.\n";
			continue;
		}

		if (!mesh->HasTangentsAndBitangents()) {
			std::cout << std::string("Mesh ") + mesh->mName.C_Str() + " does not have vertex tangents.\n";
			continue;
		}

		if (!mesh->HasFaces()) {
			std::cout << std::string("Mesh ") + mesh->mName.C_Str() + " does not have faces.\n";
			continue;
		}

		psi_rndr::MeshData out_mesh;

		bool has_tex_coords = mesh->HasTextureCoords(0);

		for (size_t i_vert = 0; i_vert < mesh->mNumVertices; i_vert++) {
			// save vertex data to model vector
			aiVector3D const* const pos = &(mesh->mVertices[i_vert]);

			aiVector3D const* const norm = &(mesh->mNormals[i_vert]);

			aiVector3D const* const tangent = &(mesh->mTangents[i_vert]);

			float U = 0.0f, V = 0.0f;
			if (has_tex_coords) {
				aiVector3D const* const tex_coord = &(mesh->mTextureCoords[0][i_vert]);
				U = tex_coord->x;
				V = tex_coord->y;
			}

			psi_rndr::VertexData vert;
			vert.pos = {{pos->x, pos->y, pos->z}};
			vert.norm = {{norm->x, norm->y, norm->z}};
			vert.uv = {{U, V}};
			vert.tan = {{tangent->x, tangent->y, tangent->z}};

			out_mesh.vertices.push_back(vert);
		}

		for (size_t i_face = 0; i_face < mesh->mNumFaces; i_face++) {
			const aiFace* const face = &(mesh->mFaces[i_face]);

			// save all indices to model vector
			for (size_t i_index = 0; i_index < face->mNumIndices; i_index++)
				out_mesh.indices.push_back(face->mIndices[i_index]);
		}

		// TODO
		//mesh->calcBoundingBox();
		//mesh->calcTangents();

		// create file
		fs::path out_file = out_dir;
		out_file += file.filename().replace_extension().string() + "_" + mesh->mName.C_Str() + ".msh";

		std::ofstream out_stream(out_file.string());
		if (!out_stream.good())
			throw std::runtime_error("Could not write to file " + out_file.string() + ".");

		// store in Psi Mesh format
		MD5_CTX md5;
		std::array<unsigned char, 16> md5_digest;

		uint64_t verts = out_mesh.vertices.size();
		uint64_t inds = out_mesh.indices.size();

		MD5_Init(&md5);

		out_stream.write((char*)(&verts), sizeof(uint64_t));
		MD5_Update(&md5, &verts, sizeof(uint64_t));
		out_stream.write((char*)(&inds), sizeof(uint64_t));
		MD5_Update(&md5, &inds, sizeof(uint64_t));
		out_stream.write((char*)(out_mesh.max_pos.data()), 3 * sizeof(float));
		MD5_Update(&md5, out_mesh.max_pos.data(), 3 * sizeof(float));
		out_stream.write((char*)(out_mesh.min_pos.data()), 3 * sizeof(float));
		MD5_Update(&md5, out_mesh.min_pos.data(), 3 * sizeof(float));

		MD5_Final(md5_digest.data(), &md5);
		out_stream.write((char*)(md5_digest.data()), 16 * sizeof(unsigned char));

		MD5_Init(&md5);

		for (auto const& vert : out_mesh.vertices) {
			out_stream.write((char*)(&vert), sizeof(psi_rndr::VertexData));
			MD5_Update(&md5, &vert, sizeof(psi_rndr::VertexData));
		}
		for (auto const& ind : out_mesh.indices) {
			out_stream.write((char*)(&ind), sizeof(uint32_t));
			MD5_Update(&md5, &ind, sizeof(uint32_t));
		}

		MD5_Final(md5_digest.data(), &md5);
		out_stream.write((char*)(md5_digest.data()), 16 * sizeof(unsigned char));

		out_stream.close();
	}
}

struct Environment {
	std::vector<fs::path> meshes;
	fs::path out_dir;
};

constexpr int NO_EXIT = 1337;
/// Parses the given command line arguments into the specified environment.
/// @return exit code if program should exit, NO_EXIT otherwise
int parse_command_line(int argc, char** argv, Environment& env) {
	po::options_description options("Options");
	options.add_options()
		("help,h", "Display this help message and quit.")
		("version,v", "Display the program version and quit.")
		("input-directory,i", po::value<fs::path>()->default_value("./"), "Set the directory from which files are read.")
		("output-directory,o", po::value<fs::path>()->default_value("./"), "Set the directory to which files are saved.")
		("add-mesh,m", po::value<std::vector<std::string>>(), "Add a mesh file to be converted.")
	;

	po::variables_map vm;
	try {
		po::store(po::parse_command_line(argc, argv, options), vm);
		po::notify(vm);
	}
	catch (std::exception const& e) {
		std::cout
			<< "Invalid option:\n"
			<< e.what() << std::endl;
		return EXIT_FAILURE;
	}

	if (vm.count("help")) {
		std::cout
			<< "----- Psi Mesh Converter 1.0.0 -----\n"
			<< "Copyright (C) 2016 Wojciech Nawrocki\n"
			<< "Serializes meshes from common interchange formats\n"
			<< "to binary formats used by the Psi Engine.\n"
			<< "\n"
			<< options << std::endl;
		return EXIT_SUCCESS;
	}

	if (vm.count("version")) {
		std::cout << "1.0.0" << std::endl;
		return EXIT_SUCCESS;
	}

	auto in_dir = vm["input-directory"].as<fs::path>();
	if (!fs::is_directory(in_dir)) {
		std::cout << "Invalid input directory " << in_dir << std::endl;
		return EXIT_FAILURE;
	}

	env.out_dir = vm["output-directory"].as<fs::path>();
	if (!fs::is_directory(env.out_dir)) {
		std::cout << "Invalid output directory " << env.out_dir << std::endl;
		return EXIT_FAILURE;
	}

	if (!vm.count("add-mesh")) {
		std::cout << "Please specify at least one mesh file." << std::endl;
		return EXIT_FAILURE;
	}

	auto mesh_files = vm["add-mesh"].as<std::vector<std::string>>();
	fs::path path;
	for (auto const& s : mesh_files) {
		path = in_dir;
		path += s;
		if (!fs::exists(path)) {
			std::cout << "Specified mesh file " << path << " does not exist." << std::endl;
			return EXIT_FAILURE;
		}
		env.meshes.push_back(path);
	}

	return NO_EXIT;
}

int main(int argc, char** argv) {
	Environment env;
	auto code = parse_command_line(argc, argv, env);
	if (code != NO_EXIT)
		return code;

	for (auto const& file : env.meshes) {
		try {
			std::cout
				<< "Parsing mesh file "
				<< file
				<< ".." << std::endl;
			load_and_serialize_mesh(file, env.out_dir);
		}
		catch (std::exception const& e) {
			std::cout
				<< "An error occurred while parsing mesh file "
				<< file
				<< ":\n"
				<< e.what() << std::endl;
			return EXIT_FAILURE;
		}
	}

	std::cout << "Conversion completed successfully. Thank you for your patience." << std::endl;
	return EXIT_SUCCESS;
}
