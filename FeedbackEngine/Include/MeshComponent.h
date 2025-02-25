#pragma once
#include "Prerequisites.h"
#include "DeviceContext.h"

class 
MeshComponent {
public:
	MeshComponent() : m_numVertex(0), m_numIndex(0) {}
	~MeshComponent() = default;
public:
	std::string m_name;
	std::vector<SimpleVertex> m_vertex;
	std::vector<unsigned int> m_index;
	int m_numVertex;
	int m_numIndex;
};
