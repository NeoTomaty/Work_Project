#include	"PlaneMesh.h"
#include	<iostream>

void PlaneMesh::Init(
	int divx, int divy,
	float width, float height,
	DirectX::SimpleMath::Color color,
	DirectX::SimpleMath::Vector3 normal,
	DirectX::SimpleMath::Vector3 tangent,
	bool xzflag,
	bool cwflag) 
{
	// �T�C�Y�Z�b�g�i���ƍ����j�iXY���ʁj
	m_width = width;
	m_height = height;

	// ������
	m_divY = divy;
	m_divX = divx;

	// �@���x�N�g��
	m_normal = normal;

	// ���_�J���[
	m_color = color;

	// �ڃx�N�g��
	m_tangent = tangent;


	if (xzflag) {
		// ���_�f�[�^����
		CreateVertexXZ();
	}
	else {
		// ���_�f�[�^����
		CreateVertex();
	}

	// �C���f�b�N�X�f�[�^����
	if (cwflag) {
		CreateIndexCW();
	}
	else {
		CreateIndexCCW();
	}
}

void PlaneMesh::CreateVertex() {
	// ���_�f�[�^�N���A
	m_vertices.clear();

	for (unsigned int y = 0; y <= m_divY; y++) {
		VERTEX_3D	vtx{};

		for (unsigned int x = 0; x <= m_divX; x++) {

			// ���_���W�Z�b�g
			vtx.Position.x = -m_width / 2.0f + x * m_width / m_divX;
			vtx.Position.y = -m_height / 2.0f + y * m_height / m_divY;
			vtx.Position.z = 0.0f;

			// �@���x�N�g���Z�b�g
			vtx.Normal = m_normal;				// �@�����Z�b�g

			vtx.Diffuse = m_color;					// �J���[�l�Z�b�g

			// �J��Ԃ��ɑΉ��i�e�N�X�`���́j
			float texu = 1.0f * m_divX;
			float texv = 1.0f * m_divY;
			vtx.TexCoord.x = (texu * x / m_divX);
//			vtx.TexCoord.y = (texv * y / m_divY);			// �e�N�X�`���㉺�������܃o�O���C��
			vtx.TexCoord.y = texv-(texv * y / m_divY);

			// �ڃx�N�g�����Z�b�g
			vtx.Tangent = m_tangent;


			m_vertices.emplace_back(vtx);		// ���_�f�[�^�Z�b�g
		}
	}
}

void PlaneMesh::CreateVertexXZ() {
	// ���_�f�[�^�N���A
	m_vertices.clear();

	for (unsigned int y = 0; y <= m_divY; y++) {
		VERTEX_3D	vtx{};

		for (unsigned int x = 0; x <= m_divX; x++) {

			// ���_���W�Z�b�g
			vtx.Position.x = -m_width / 2.0f + x * m_width / m_divX;
			vtx.Position.y = 0.0f;
			vtx.Position.z = -m_height / 2.0f + y * m_height / m_divY;

			// �@���x�N�g���Z�b�g
			vtx.Normal = m_normal;				// �@�����Z�b�g

			vtx.Diffuse = m_color;					// �J���[�l�Z�b�g

			// �J��Ԃ��ɑΉ��i�e�N�X�`���́j
			float texu = 1.0f * m_divX;
			float texv = 1.0f * m_divY;
			vtx.TexCoord.x = (texu * x / m_divX);
			vtx.TexCoord.y = (texv * y / m_divY);


			m_vertices.emplace_back(vtx);		// ���_�f�[�^�Z�b�g
		}
	}
}

void PlaneMesh::CreateIndexCW() {
	// �C���f�b�N�X�f�[�^�N���A
	m_indices.clear();

	// �C���f�b�N�X����
	for (unsigned int y = 0; y < m_divY; y++) {
		for (unsigned int x = 0; x < m_divX; x++) {
			int count = (m_divX + 1) * y + x;		// �������W�̃C���f�b�N�X

			// ������
			{
				FACE face{};

				face.idx[0] = count;						// ����
				face.idx[1] = count + 1 + (m_divX + 1);		// �E��
				face.idx[2] = count + 1;					// �E

				m_indices.emplace_back(face.idx[0]);
				m_indices.emplace_back(face.idx[1]);
				m_indices.emplace_back(face.idx[2]);

			}

			// �㔼��
			{
				FACE face{};
				face.idx[0] = count;						// ����
				face.idx[1] = count + (m_divX + 1);			// ��
				face.idx[2] = count + (m_divX + 1) + 1;		// �E��

				m_indices.emplace_back(face.idx[0]);
				m_indices.emplace_back(face.idx[1]);
				m_indices.emplace_back(face.idx[2]);

			}
		}
	}
}

void PlaneMesh::CreateIndexCCW() {
	// �C���f�b�N�X�f�[�^�N���A
	m_indices.clear();

	// �C���f�b�N�X����
	for (unsigned int y = 0; y < m_divY; y++) {
		for (unsigned int x = 0; x < m_divX; x++) {
			int count = (m_divX + 1) * y + x;		// �������W�̃C���f�b�N�X

			// ������
			{
				FACE face{};

				face.idx[0] = count;						// ����
				face.idx[1] = count + 1 + (m_divX + 1);		// �E��
				face.idx[2] = count + 1;					// �E

				m_indices.emplace_back(face.idx[0]);
				m_indices.emplace_back(face.idx[2]);
				m_indices.emplace_back(face.idx[1]);

			}

			// �㔼��
			{
				FACE face{};
				face.idx[0] = count;						// ����
				face.idx[1] = count + (m_divX + 1);			// ��
				face.idx[2] = count + (m_divX + 1) + 1;		// �E��

				m_indices.emplace_back(face.idx[0]);
				m_indices.emplace_back(face.idx[2]);
				m_indices.emplace_back(face.idx[1]);

			}
		}
	}
}

// ��
float PlaneMesh::GetWidth() {
	return m_width;
}

// ����
float PlaneMesh::GetHeight() {
	return m_height;
}

int PlaneMesh::GetDivX() {
	return m_divX;
}

int PlaneMesh::GetDivY() {
	return m_divY;
}

// �w�肵��3�p�`�ԍ��̎O�p�`�C���f�b�N�X���擾
PlaneMesh::FACE PlaneMesh::GetTriangle(int triangleno) {

	FACE face;
	face.idx[0] = m_indices[triangleno * 3];
	face.idx[1] = m_indices[triangleno * 3 + 1];
	face.idx[2] = m_indices[triangleno * 3 + 2];
	return face;

}

// ���Ԗڂ̎l�p�`����������
int PlaneMesh::GetSquareNo(DirectX::SimpleMath::Vector3 pos)
{
	// �����b�V����XZ���ʂ��x�[�X�ō쐬���Ă���
	double x = pos.x;
	double y = pos.z;

	// ���ʂ̕�
	double planewidth = m_width;

	// ���ʂ̍���
	double planeheight = m_height;

	// �}�b�v�`�b�v�T�C�Y�i��Βl�Ōv�Z�j
	double mapchipwidth = fabs(planewidth / m_divX);
	double mapchipheight = fabs(planeheight / m_divY);

	// �������_�̑��΍��W�ɕϊ�
	double relativex = x + (planewidth / 2.0);
	double relativey = y + (planeheight / 2.0);

	// �����牽�Ԗڂ��H
	unsigned int mapchipx = static_cast<unsigned int>(relativex / mapchipwidth);

	// �����牽�Ԗڂ��H
	unsigned int mapchipy = static_cast<unsigned int>(relativey / mapchipheight);

	// �������O�ԖڂƂ����ꍇ�̏��Ԃ��v�Z
	int squareno;
	squareno = mapchipy * m_divX + mapchipx;

	if (squareno < 0) {
		squareno = 0;
	}
	else {
		if (squareno > static_cast<int>(m_divX) * static_cast<int>(m_divY) - 1) {
			squareno = m_divX * m_divY - 1;
		}
	}

	return squareno;
}
