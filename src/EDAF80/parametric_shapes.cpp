#include "parametric_shapes.hpp"
#include "core/Log.h"

#include <glm/glm.hpp>

#include <array>
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

bonobo::mesh_data
parametric_shapes::createQuad(float const width, float const height,
                              unsigned int const horizontal_split_count,
                              unsigned int const vertical_split_count)
{
	auto const vertices = std::array<glm::vec3, 4>{
		glm::vec3(0.0f,  0.0f,   0.0f),
		glm::vec3(width, 0.0f,   0.0f),
		glm::vec3(width, height, 0.0f),
		glm::vec3(0.0f,  height, 0.0f)
	};

	auto const index_sets = std::array<glm::uvec3, 2>{
		glm::uvec3(0u, 1u, 2u),
		glm::uvec3(0u, 2u, 3u)
	};

	bonobo::mesh_data data;

	if (horizontal_split_count > 0u || vertical_split_count > 0u)
	{
		LogError("parametric_shapes::createQuad() does not support tesselation.");
		return data;
	}

	//
	// NOTE:
	//
	// Only the values preceeded by a `\todo` tag should be changed, the
	// other ones are correct!
	//

	// Create a Vertex Array Object: it will remember where we stored the
	// data on the GPU, and  which part corresponds to the vertices, which
	// one for the normals, etc.
	//
	// The following function will create new Vertex Arrays, and pass their
	// name in the given array (second argument). Since we only need one,
	// pass a pointer to `data.vao`.
	glGenVertexArrays(1, /*! \todo fill me */&data.vao);

	// To be able to store information, the Vertex Array has to be bound
	// first.
	glBindVertexArray(/*! \todo bind the previously generated Vertex Array */data.vao);

	// To store the data, we need to allocate buffers on the GPU. Let's
	// allocate a first one for the vertices.
	//
	// The following function's syntax is similar to `glGenVertexArray()`:
	// it will create multiple OpenGL objects, in this case buffers, and
	// return their names in an array. Have the buffer's name stored into
	// `data.bo`.
	glGenBuffers(1, /*! \todo fill me */&data.bo);

	// Similar to the Vertex Array, we need to bind it first before storing
	// anything in it. The data stored in it can be interpreted in
	// different ways. Here, we will say that it is just a simple 1D-array
	// and therefore bind the buffer to the corresponding target.
	glBindBuffer(GL_ARRAY_BUFFER, /*! \todo bind the previously generated Buffer */data.bo);

	glBufferData(GL_ARRAY_BUFFER, /*! \todo how many bytes should the buffer contain? */sizeof(vertices),
	             /* where is the data stored on the CPU? */vertices.data(),
	             /* inform OpenGL that the data is modified once, but used often */GL_STATIC_DRAW);

	// Vertices have been just stored into a buffer, but we still need to
	// tell Vertex Array where to find them, and how to interpret the data
	// within that buffer.
	//
	// You will see shaders in more detail in lab 3, but for now they are
	// just pieces of code running on the GPU and responsible for moving
	// all the vertices to clip space, and assigning a colour to each pixel
	// covered by geometry.
	// Those shaders have inputs, some of them are the data we just stored
	// in a buffer object. We need to tell the Vertex Array which inputs
	// are enabled, and this is done by the following line of code, which
	// enables the input for vertices:
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::vertices));

	// Once an input is enabled, we need to explain where the data comes
	// from, and how it interpret it. When calling the following function,
	// the Vertex Array will automatically use the current buffer bound to
	// GL_ARRAY_BUFFER as its source for the data. How to interpret it is
	// specified below:
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::vertices),
	                      /*! \todo how many components do our vertices have? */3,
	                      /* what is the type of each component? */GL_FLOAT,
	                      /* should it automatically normalise the values stored */GL_FALSE,
	                      /* once all components of a vertex have been read, how far away (in bytes) is the next vertex? */0,
	                      /* how far away (in bytes) from the start of the buffer is the first vertex? */reinterpret_cast<GLvoid const*>(0x0));

	// Now, let's allocate a second one for the indices.
	//
	// Have the buffer's name stored into `data.ibo`.
	glGenBuffers(1, /*! \todo fill me */&data.ibo);

	// We still want a 1D-array, but this time it should be a 1D-array of
	// elements, aka. indices!
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, /*! \todo bind the previously generated Buffer */data.ibo);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, /*! \todo how many bytes should the buffer contain? */sizeof(index_sets),
	             /* where is the data stored on the CPU? */index_sets.data(),
	             /* inform OpenGL that the data is modified once, but used often */GL_STATIC_DRAW);

	data.indices_nb = /*! \todo how many indices do we have? */index_sets.size() * 3u;

	// All the data has been recorded, we can unbind them.
	glBindVertexArray(0u);
	glBindBuffer(GL_ARRAY_BUFFER, 0u);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0u);

	return data;
}

// longitude: describe west and east
// latitude: describe north and south
bonobo::mesh_data
parametric_shapes::createSphere(float const radius,
                                unsigned int const longitude_split_count,
                                unsigned int const latitude_split_count)
{
	// compute the how many vertices do we need from the longitude and latitude
	// maybe we can use unordered_set to store the vertices

	// it's hard to understand why +1 here, but you can understand by the following set up
	const auto latitude_edge_count = latitude_split_count + 1u;
	const auto longitude_edge_count = longitude_split_count + 1u;
	const auto latitude_vertices_nb = latitude_split_count + 2u; // we take the top and the bottom vertices as a part of latitude every time
	const auto longitude_vertices_nb = longitude_edge_count + 1u;
	const auto vertices_nb = latitude_vertices_nb * longitude_vertices_nb;

	// construct the properties of the vertices
	auto vertices_pos = std::vector<glm::vec3>(vertices_nb);
	auto vertices_normal = std::vector<glm::vec3>(vertices_nb);
	auto vertices_tangent = std::vector<glm::vec3>(vertices_nb);
	auto vertices_binormal = std::vector<glm::vec3>(vertices_nb);
	auto vertices_texCoord = std::vector<glm::vec3>(vertices_nb);

	// compute the offset and size of each properties for set up the vertex array attributes
	// 1. position
	const auto vertices_pos_offset = 0;
	const auto vertices_pos_size = vertices_pos.size() * sizeof(glm::vec3);
	// 2. normal
	const auto vertices_normal_offset = vertices_pos_size;
	const auto vertices_normal_size = vertices_normal.size() * sizeof(glm::vec3);
	// 3. tangent
	const auto vertices_tangent_offset = vertices_normal_offset + vertices_normal_size;
	const auto vertices_tangent_size = vertices_tangent.size() * sizeof(glm::vec3);
	// 4. binormal
	const auto vertices_binormal_offset = vertices_tangent_offset + vertices_tangent_size;
	const auto vertices_binormal_size = vertices_binormal.size() * sizeof(glm::vec3);
	// 5. texture coordinates
	const auto vertices_texCoord_offset = vertices_binormal_offset + vertices_binormal_size;
	const auto vertices_texCoord_size = vertices_texCoord.size() * sizeof(glm::vec3);
	// 6. size in total
	const auto vbo_size =
		vertices_pos_size +
		vertices_normal_size +
		vertices_tangent_size +
		vertices_binormal_size +
		vertices_texCoord_size;


	// need to be unsigned int, otherwise the indices can not pass to the gpu correctly
	auto indices = std::vector<glm::uvec3>(2 * (latitude_vertices_nb-1) * (longitude_vertices_nb-1));

	// compute the theta and phi
	const auto d_theta = glm::two_pi<float>() / static_cast<float>(longitude_edge_count);
	const auto d_phi = glm::pi<float>() / static_cast<float>(latitude_edge_count);

	// compute the position of the vertices
	// we start at the top vertex of the sphere
	float phi = 0.0f;
	auto index = 0u;
	for (unsigned int i = 0; i < latitude_vertices_nb; i++) {
		const float cos_phi = std::cos(phi);
		const float sin_phi = std::sin(phi);

		float theta = 0.0f;
		for (unsigned int j = 0; j < longitude_vertices_nb; j++) {
			const float cos_theta = std::cos(theta);
			const float sin_theta = std::sin(theta);

			// compute the position
			vertices_pos[index] = glm::vec3(
				radius * sin_theta * sin_phi,
				-radius * cos_phi,
				radius * cos_theta * sin_phi
			);

			// tangent
			vertices_tangent[index] = glm::vec3(
				cos_theta,
				0,
				-sin_theta
			);

			// binormal
			vertices_binormal[index] = glm::vec3(
				sin_theta * cos_phi,
				sin_phi,
				cos_theta * cos_phi
			);

			// normal
			//vertices_normal[index] = glm::cross(vertices_tangent[index], vertices_binormal[index]);
			// dont forget to normalized
			vertices_normal[index] = glm::normalize(glm::cross(vertices_tangent[index], vertices_binormal[index]));

			// texture coordinates
			vertices_texCoord[index] = glm::vec3(
				static_cast<float>(j) / static_cast<float>(longitude_vertices_nb),
				static_cast<float>(i) / static_cast<float>(latitude_vertices_nb),
				.0f
			);

			theta += d_theta;
			++index;
		}

		phi += d_phi;
	}

	// compute the indices
	index = 0u;
	for (auto c = 0u; c < latitude_edge_count; c++) {
		for (auto r = 0u; r < longitude_edge_count; r++) {
			// the first tri of the quad
			indices[index] = glm::uvec3(
				c * longitude_vertices_nb + r,
				(c + 1) * longitude_vertices_nb + (r+1),
				(c + 1) * longitude_vertices_nb + r
			);
			//indices[index] = glm::uvec3(
			//	longitude_vertices_nb * (c + 0u) + (r + 0u),
			//	longitude_vertices_nb * (c + 0u) + (r + 1u),
			//	longitude_vertices_nb * (c + 1u) + (r + 1u)
			//);
			++index;

			// the second tri of the quad
			indices[index] = glm::uvec3(
				c * longitude_vertices_nb + r,
				c * longitude_vertices_nb + (r+1),
				(c + 1) * longitude_vertices_nb + (r+1)
			);
			//indices[index] = glm::uvec3(
			//	longitude_vertices_nb * (c + 0u) + (r + 0u),
			//	longitude_vertices_nb * (c + 1u) + (r + 1u),
			//	longitude_vertices_nb * (c + 1u) + (r + 0u)
			//);
			++index;
		}
	}

	bonobo::mesh_data data;
	glGenVertexArrays(1, &data.vao);
	assert(data.vao != 0u);
	glBindVertexArray(data.vao);

	glGenBuffers(1, &data.bo);
	assert(data.bo != 0u);
	glBindBuffer(GL_ARRAY_BUFFER, data.bo);
	// pre-allocated the memory
	glBufferData(GL_ARRAY_BUFFER, vbo_size, nullptr, GL_STATIC_DRAW);

	// specify the attributes in the memory
	glBufferSubData(GL_ARRAY_BUFFER, vertices_pos_offset, vertices_pos_size, vertices_pos.data());
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::vertices));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::vertices), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(0x0));

	glBufferSubData(GL_ARRAY_BUFFER, vertices_normal_offset, vertices_normal_size, static_cast<GLvoid const*>(vertices_normal.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::normals));
	// dont forget to specify the correct offset
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::normals), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(vertices_normal_offset));

	glBufferSubData(GL_ARRAY_BUFFER, vertices_texCoord_offset, vertices_texCoord_size, static_cast<GLvoid const*>(vertices_texCoord.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::texcoords));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::texcoords), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(vertices_texCoord_offset));

	glBufferSubData(GL_ARRAY_BUFFER, vertices_tangent_offset, vertices_tangent_size, static_cast<GLvoid const*>(vertices_tangent.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::tangents));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::tangents), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(vertices_tangent_offset));

	glBufferSubData(GL_ARRAY_BUFFER, vertices_binormal_offset, vertices_binormal_size, static_cast<GLvoid const*>(vertices_binormal.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::binormals));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::binormals), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(vertices_binormal_offset));


	glBindBuffer(GL_ARRAY_BUFFER, 0u);

	data.indices_nb = static_cast<GLsizei>(indices.size() * 3u);
	glGenBuffers(1, &data.ibo);
	assert(data.ibo != 0u);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(glm::uvec3), indices.data(), GL_STATIC_DRAW);

	glBindVertexArray(0u);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0u);

	return data;
}

bonobo::mesh_data
parametric_shapes::createTorus(float const major_radius,
                               float const minor_radius,
                               unsigned int const major_split_count,
                               unsigned int const minor_split_count)
{
	const auto major_edge_count = major_split_count + 1u;
	const auto minor_edge_count = minor_split_count + 1u;
	const auto major_vert_nb = major_edge_count + 2u; // we take the top and the bottom vertices as a part of latitude every time
	const auto minor_vert_nb = minor_edge_count + 1u;
	const auto vertices_nb = major_vert_nb * minor_vert_nb;

	// construct the properties of the vertices
	auto vertices_pos = std::vector<glm::vec3>(vertices_nb);
	auto vertices_normal = std::vector<glm::vec3>(vertices_nb);
	auto vertices_tangent = std::vector<glm::vec3>(vertices_nb);
	auto vertices_binormal = std::vector<glm::vec3>(vertices_nb);
	auto vertices_texCoord = std::vector<glm::vec3>(vertices_nb);

	// compute the offset and size of each properties for set up the vertex array attributes
	// 1. position
	const auto vertices_pos_offset = 0;
	const auto vertices_pos_size = vertices_pos.size() * sizeof(glm::vec3);
	// 2. normal
	const auto vertices_normal_offset = vertices_pos_size;
	const auto vertices_normal_size = vertices_normal.size() * sizeof(glm::vec3);
	// 3. tangent
	const auto vertices_tangent_offset = vertices_normal_offset + vertices_normal_size;
	const auto vertices_tangent_size = vertices_tangent.size() * sizeof(glm::vec3);
	// 4. binormal
	const auto vertices_binormal_offset = vertices_tangent_offset + vertices_tangent_size;
	const auto vertices_binormal_size = vertices_binormal.size() * sizeof(glm::vec3);
	// 5. texture coordinates
	const auto vertices_texCoord_offset = vertices_binormal_offset + vertices_binormal_size;
	const auto vertices_texCoord_size = vertices_texCoord.size() * sizeof(glm::vec3);
	// 6. size in total
	const auto vbo_size =
		vertices_pos_size +
		vertices_normal_size +
		vertices_tangent_size +
		vertices_binormal_size +
		vertices_texCoord_size;


	// need to be unsigned int, otherwise the indices can not pass to the gpu correctly
	auto indices = std::vector<glm::uvec3>(2 * (major_vert_nb-1) * (minor_vert_nb-1));

	// compute the theta and phi
	const auto d_theta = glm::two_pi<float>() / static_cast<float>(major_edge_count);
	const auto d_phi = glm::two_pi<float>() / static_cast<float>(minor_edge_count);

	// compute the position of the vertices
	// we start at the top vertex of the sphere
	/**
	* Major is vertical
	* Minor is horizontal
	*/
	float theta = 0.0f;
	auto index = 0u;
	for (unsigned int i = 0; i < major_vert_nb; i++) {
		const float cos_theta = std::cos(theta);
		const float sin_theta = std::sin(theta);

		float phi = 0.0f;
		for (unsigned int j = 0; j < minor_vert_nb; j++) {
			const float cos_phi = std::cos(phi);
			const float sin_phi = std::sin(phi);

			// compute the position
			vertices_pos[index] = glm::vec3(
				(major_radius + minor_radius * cos_theta) * cos_phi,
				-minor_radius * sin_theta,
				(major_radius + minor_radius * cos_theta) * sin_phi
			);

			// tangent
			vertices_tangent[index] = glm::vec3(
				-minor_radius * sin_theta * cos_phi,
				-minor_radius * sin_theta * sin_phi,
				-minor_radius * cos_theta
			);

			// binormal
			vertices_binormal[index] = glm::vec3(
				-(major_radius + minor_radius * cos_theta) * sin_phi,
				(major_radius + minor_radius * cos_theta) * cos_phi,
				0
			);

			// normal
			//vertices_normal[index] = glm::cross(vertices_tangent[index], vertices_binormal[index]);
			// dont forget to normalized
			//vertices_normal[index] = glm::cross(vertices_binormal[index], vertices_tangent[index]);
			vertices_normal[index] = glm::cross(vertices_tangent[index], vertices_binormal[index]);

			// texture coordinates
			vertices_texCoord[index] = glm::vec3(
				static_cast<float>(i) / static_cast<float>(major_edge_count),
				static_cast<float>(j) / static_cast<float>(minor_edge_count),
				.0f
			);

			phi += d_phi;
			++index;
		}

		theta += d_theta;
	}

	// compute the indices
	index = 0u;
	for (auto c = 0u; c < major_edge_count; c++) {
		for (auto r = 0u; r < minor_edge_count; r++) {
			// the first tri of the quad
			indices[index] = glm::uvec3(
				c * minor_vert_nb + r,
				(c + 1) * minor_vert_nb + (r+1),
				(c + 1) * minor_vert_nb + r
			);
			//indices[index] = glm::uvec3(
			//	longitude_vertices_nb * (c + 0u) + (r + 0u),
			//	longitude_vertices_nb * (c + 0u) + (r + 1u),
			//	longitude_vertices_nb * (c + 1u) + (r + 1u)
			//);
			++index;

			// the second tri of the quad
			indices[index] = glm::uvec3(
				c * minor_vert_nb + r,
				c * minor_vert_nb + (r+1),
				(c + 1) * minor_vert_nb + (r+1)
			);
			//indices[index] = glm::uvec3(
			//	longitude_vertices_nb * (c + 0u) + (r + 0u),
			//	longitude_vertices_nb * (c + 1u) + (r + 1u),
			//	longitude_vertices_nb * (c + 1u) + (r + 0u)
			//);
			++index;
		}
	}

	bonobo::mesh_data data;
	glGenVertexArrays(1, &data.vao);
	assert(data.vao != 0u);
	glBindVertexArray(data.vao);

	glGenBuffers(1, &data.bo);
	assert(data.bo != 0u);
	glBindBuffer(GL_ARRAY_BUFFER, data.bo);
	// pre-allocated the memory
	glBufferData(GL_ARRAY_BUFFER, vbo_size, nullptr, GL_STATIC_DRAW);

	// specify the attributes in the memory
	glBufferSubData(GL_ARRAY_BUFFER, vertices_pos_offset, vertices_pos_size, vertices_pos.data());
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::vertices));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::vertices), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(0x0));

	glBufferSubData(GL_ARRAY_BUFFER, vertices_normal_offset, vertices_normal_size, static_cast<GLvoid const*>(vertices_normal.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::normals));
	// dont forget to specify the correct offset
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::normals), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(vertices_normal_offset));

	glBufferSubData(GL_ARRAY_BUFFER, vertices_texCoord_offset, vertices_texCoord_size, static_cast<GLvoid const*>(vertices_texCoord.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::texcoords));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::texcoords), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(vertices_texCoord_offset));

	glBufferSubData(GL_ARRAY_BUFFER, vertices_tangent_offset, vertices_tangent_size, static_cast<GLvoid const*>(vertices_tangent.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::tangents));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::tangents), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(vertices_tangent_offset));

	glBufferSubData(GL_ARRAY_BUFFER, vertices_binormal_offset, vertices_binormal_size, static_cast<GLvoid const*>(vertices_binormal.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::binormals));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::binormals), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(vertices_binormal_offset));


	glBindBuffer(GL_ARRAY_BUFFER, 0u);

	data.indices_nb = static_cast<GLsizei>(indices.size() * 3u);
	glGenBuffers(1, &data.ibo);
	assert(data.ibo != 0u);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(glm::uvec3), indices.data(), GL_STATIC_DRAW);

	glBindVertexArray(0u);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0u);

	return data;
}

bonobo::mesh_data
parametric_shapes::createCircleRing(float const radius,
                                    float const spread_length,
                                    unsigned int const circle_split_count,
                                    unsigned int const spread_split_count)
{
	auto const circle_slice_edges_count = circle_split_count + 1u;
	auto const spread_slice_edges_count = spread_split_count + 1u;
	auto const circle_slice_vertices_count = circle_slice_edges_count + 1u;
	auto const spread_slice_vertices_count = spread_slice_edges_count + 1u;
	auto const vertices_nb = circle_slice_vertices_count * spread_slice_vertices_count;

	auto vertices  = std::vector<glm::vec3>(vertices_nb);
	auto normals   = std::vector<glm::vec3>(vertices_nb);
	auto texcoords = std::vector<glm::vec3>(vertices_nb);
	auto tangents  = std::vector<glm::vec3>(vertices_nb);
	auto binormals = std::vector<glm::vec3>(vertices_nb);

	float const spread_start = radius - 0.5f * spread_length;
	float const d_theta = glm::two_pi<float>() / (static_cast<float>(circle_slice_edges_count));
	float const d_spread = spread_length / (static_cast<float>(spread_slice_edges_count));

	// generate vertices iteratively
	size_t index = 0u;
	float theta = 0.0f;
	for (unsigned int i = 0u; i < circle_slice_vertices_count; ++i) {
		float const cos_theta = std::cos(theta);
		float const sin_theta = std::sin(theta);

		float distance_to_centre = spread_start;
		for (unsigned int j = 0u; j < spread_slice_vertices_count; ++j) {
			// vertex
			vertices[index] = glm::vec3(distance_to_centre * cos_theta,
			                            distance_to_centre * sin_theta,
			                            0.0f);

			// texture coordinates
			texcoords[index] = glm::vec3(static_cast<float>(j) / (static_cast<float>(spread_slice_vertices_count)),
			                             static_cast<float>(i) / (static_cast<float>(circle_slice_vertices_count)),
			                             0.0f);

			// tangent
			auto const t = glm::vec3(cos_theta, sin_theta, 0.0f);
			tangents[index] = t;

			// binormal
			auto const b = glm::vec3(-sin_theta, cos_theta, 0.0f);
			binormals[index] = b;

			// normal
			auto const n = glm::cross(t, b);
			normals[index] = n;

			distance_to_centre += d_spread;
			++index;
		}

		theta += d_theta;
	}

	// create index array
	auto index_sets = std::vector<glm::uvec3>(2u * circle_slice_edges_count * spread_slice_edges_count);

	// generate indices iteratively
	index = 0u;
	for (unsigned int i = 0u; i < circle_slice_edges_count; ++i)
	{
		for (unsigned int j = 0u; j < spread_slice_edges_count; ++j)
		{
			index_sets[index] = glm::uvec3(spread_slice_vertices_count * (i + 0u) + (j + 0u),
			                               spread_slice_vertices_count * (i + 0u) + (j + 1u),
			                               spread_slice_vertices_count * (i + 1u) + (j + 1u));
			++index;

			index_sets[index] = glm::uvec3(spread_slice_vertices_count * (i + 0u) + (j + 0u),
			                               spread_slice_vertices_count * (i + 1u) + (j + 1u),
			                               spread_slice_vertices_count * (i + 1u) + (j + 0u));
			++index;
		}
	}

	bonobo::mesh_data data;
	glGenVertexArrays(1, &data.vao);
	assert(data.vao != 0u);
	glBindVertexArray(data.vao);

	auto const vertices_offset = 0u;
	auto const vertices_size = static_cast<GLsizeiptr>(vertices.size() * sizeof(glm::vec3));
	auto const normals_offset = vertices_size;
	auto const normals_size = static_cast<GLsizeiptr>(normals.size() * sizeof(glm::vec3));
	auto const texcoords_offset = normals_offset + normals_size;
	auto const texcoords_size = static_cast<GLsizeiptr>(texcoords.size() * sizeof(glm::vec3));
	auto const tangents_offset = texcoords_offset + texcoords_size;
	auto const tangents_size = static_cast<GLsizeiptr>(tangents.size() * sizeof(glm::vec3));
	auto const binormals_offset = tangents_offset + tangents_size;
	auto const binormals_size = static_cast<GLsizeiptr>(binormals.size() * sizeof(glm::vec3));
	auto const bo_size = static_cast<GLsizeiptr>(vertices_size
	                                            +normals_size
	                                            +texcoords_size
	                                            +tangents_size
	                                            +binormals_size
	                                            );
	glGenBuffers(1, &data.bo);
	assert(data.bo != 0u);
	glBindBuffer(GL_ARRAY_BUFFER, data.bo);
	//glBufferData(GL_ARRAY_BUFFER, bo_size, nullptr, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, bo_size, nullptr, GL_STATIC_DRAW);

	glBufferSubData(GL_ARRAY_BUFFER, vertices_offset, vertices_size, static_cast<GLvoid const*>(vertices.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::vertices));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::vertices), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(0x0));

	glBufferSubData(GL_ARRAY_BUFFER, normals_offset, normals_size, static_cast<GLvoid const*>(normals.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::normals));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::normals), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(normals_offset));

	glBufferSubData(GL_ARRAY_BUFFER, texcoords_offset, texcoords_size, static_cast<GLvoid const*>(texcoords.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::texcoords));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::texcoords), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(texcoords_offset));

	glBufferSubData(GL_ARRAY_BUFFER, tangents_offset, tangents_size, static_cast<GLvoid const*>(tangents.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::tangents));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::tangents), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(tangents_offset));

	glBufferSubData(GL_ARRAY_BUFFER, binormals_offset, binormals_size, static_cast<GLvoid const*>(binormals.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::binormals));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::binormals), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(binormals_offset));

	glBindBuffer(GL_ARRAY_BUFFER, 0u);

	data.indices_nb = static_cast<GLsizei>(index_sets.size() * 3u);
	glGenBuffers(1, &data.ibo);
	assert(data.ibo != 0u);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(index_sets.size() * sizeof(glm::uvec3)), reinterpret_cast<GLvoid const*>(index_sets.data()), GL_STATIC_DRAW);

	glBindVertexArray(0u);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0u);

	return data;
}
