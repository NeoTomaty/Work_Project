#pragma once
#include	<simplemath.h>
#include	<random>
#include	"Mesh.h"

class PlaneMesh : public Mesh {
public:
	struct FACE {
		unsigned int idx[3];
	};

	void Init(int divx, int divy,
		float width, float height,
		DirectX::SimpleMath::Color color,
		DirectX::SimpleMath::Vector3 normal,
		DirectX::SimpleMath::Vector3 tangent,
		bool xzflag = false,
		bool cwflag = true);

	void CreateVertex();
	void CreateVertexXZ();

	void CreateIndexCW();						// ���v���ɃC���f�b�N�X�𐶐�
	void CreateIndexCCW();						// �����v���ɃC���f�b�N�X�𐶐�		

	// ��
	float GetWidth();
	// ����
	float GetHeight();
	int GetDivX();
	int GetDivY();

	// �w�肵��3�p�`�ԍ��̎O�p�`���\�����钸�_�C���f�b�N�X���擾
	PlaneMesh::FACE GetTriangle(int triangleno);

	// ���Ԗڂ̎l�p�`����������
	int GetSquareNo(DirectX::SimpleMath::Vector3 pos);

private:
	unsigned int m_divX = 1;
	unsigned int m_divY = 1;

	float  m_height = 100.0f;
	float  m_width = 100.0f;

	DirectX::SimpleMath::Color m_color;
	DirectX::SimpleMath::Vector3 m_normal;
	DirectX::SimpleMath::Vector3 m_tangent;

};