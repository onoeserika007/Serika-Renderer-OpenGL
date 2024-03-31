#pragma once
#include "BufferAttribute.h"
#include <string>
#include "Utils//utils.h"
#include "glad/glad.h"
#include <unordered_map>
#include <iostream>
#include <utility>

class Renderer;
class Geometry {

	enum Type {
		Mesh,
		Mesh_Indexed
	};

	size_t size_ = 0;
	std::unordered_map<std::string, BufferAttribute> data_map_;
	std::vector<unsigned> indices_;
	unsigned EBO = 0;
	Type geometry_type_ = Mesh;


public:
	Geometry();

	std::vector<std::string> getAttributeNameList();

	BufferAttribute& getBufferData(const std::string& attr);

	void setAttribute(const std::string& attr, BufferAttribute& data,  bool isVertex = false);

	void setIndex(const std::vector<unsigned>& indices);

	void setupPipeline(Renderer& renderer);

	bool isMesh();

	bool isMeshIndexed();

	unsigned getVeticesNum();

	unsigned getIndicesNum();

	const unsigned* getIndicesRawData();

	unsigned getEBO();

	void setEBO(unsigned id);
};