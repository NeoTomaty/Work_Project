#pragma once
#include <SimpleMath.h>
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "Material.h"
#include "Texture.h"

class Polygon2D {
	IndexBuffer				m_IndexBuffer;
	VertexBuffer<VERTEX_3D>	m_VertexBuffer;
	Shader					m_Shader;
	Material				m_Material;
	Texture					m_Texture;

	DirectX::SimpleMath::Vector3 m_center = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f);
	float m_lengthx = 500.0f;
	float m_lengthy = 500.0f;

public:
	void Init();
	void Update();
	void Draw();
	void Dispose();
};