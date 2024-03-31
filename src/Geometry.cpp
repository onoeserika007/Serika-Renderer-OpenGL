#include "Geometry.h"

Geometry::Geometry() :size_(0), geometry_type_(Mesh) {
	//glGenVertexArrays(1, &VAO);
}

 std::vector<std::string> Geometry::getAttributeNameList() {
	std::vector <std::string> ret;
	for (auto [k, v] : data_map_) {
		ret.push_back(k);
	}
	return ret;
}

 BufferAttribute& Geometry::getBufferData(const std::string& attr) {
	return data_map_[attr];
}

 void Geometry::setAttribute(const std::string& attr, BufferAttribute& data, bool isVertex) {
	//if(attr != "position" && size_ && size_ != data.size())
	if (isVertex) {
		size_ = data.size();
	}

	data_map_[attr] = data;
}

 void Geometry::setIndex(const std::vector<unsigned>& indices) {
	geometry_type_ = Mesh_Indexed;
	indices_.assign(indices.begin(), indices.end());
	//glBindVertexArray(VAO);
	//glBindVertexArray(NULL);
}

 void Geometry::setupPipeline(Renderer& renderer)
 {
	 renderer.setupGeometry(*this);
 }

 bool Geometry::isMesh() {
	return geometry_type_ == Mesh;
}

 bool Geometry::isMeshIndexed() {
	return geometry_type_ == Mesh_Indexed;
}

 GLuint Geometry::getVeticesNum() {
	return size_;
}

 GLuint Geometry::getIndicesNum() {
	return indices_.size();
}

 const unsigned* Geometry::getIndicesRawData()
 {
	 return indices_.data();
 }

 unsigned Geometry::getEBO()
 {
	 return EBO;
 }

 void Geometry::setEBO(unsigned id)
 {
	 EBO = id;
 }

