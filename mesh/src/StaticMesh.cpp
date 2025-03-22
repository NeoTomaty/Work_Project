#include	"StaticMesh.h"
#include	"AssimpPerse.h"

void StaticMesh::Load(std::string filename)
{
	std::vector<AssimpPerse::SUBSET> subsets{};							// �T�u�Z�b�g���
	std::vector<std::vector<AssimpPerse::VERTEX>> vertices{};			// ���_�f�[�^�i���b�V���P�ʁj
	std::vector<std::vector<unsigned int>> indices{};					// �C���f�b�N�X�f�[�^�i���b�V���P�ʁj
	std::vector<AssimpPerse::MATERIAL> materials{};						// �}�e���A��
	std::vector<std::unique_ptr<Texture>> embededtextures{};			// �����e�N�X�`���Q

	// assimp���g�p���ă��f���f�[�^���擾
	AssimpPerse::GetModelData(filename);

	subsets = AssimpPerse::GetSubsets();								// �T�u�Z�b�g���擾
	vertices = AssimpPerse::GetVertices();								// ���_�f�[�^�i���b�V���P�ʁj
	indices = AssimpPerse::GetIndices();								// �C���f�b�N�X�f�[�^�i���b�V���P�ʁj
	materials = AssimpPerse::GetMaterials();							// �}�e���A�����擾

	// ���_�f�[�^�쐬
	int meshidx = 0;

	for (const auto& mv : vertices)
	{
		for (auto& v : mv)
		{
			VERTEX_3D vertex{};
			vertex.Position = DirectX::SimpleMath::Vector3(v.pos.x, v.pos.y, v.pos.z);
			vertex.Normal = DirectX::SimpleMath::Vector3(v.normal.x, v.normal.y, v.normal.z);
			vertex.TexCoord = DirectX::SimpleMath::Vector2(v.texcoord.x, v.texcoord.y);
			vertex.Diffuse = DirectX::SimpleMath::Color(v.color.r, v.color.g, v.color.b, v.color.a);
			vertex.Tangent = DirectX::SimpleMath::Vector3(v.tangent.x, v.tangent.y, v.tangent.z);

			m_vertices.emplace_back(vertex);
		}
	}

	// �C���f�b�N�X�f�[�^�쐬
	for (const auto& mi : indices)
	{
		for (auto& index : mi)
		{
			m_indices.emplace_back(index);
		}
	}

	// �T�u�Z�b�g�f�[�^�쐬
	for (const auto& sub : subsets)
	{
		SUBSET subset{};
		subset.VertexBase = sub.VertexBase;
		subset.VertexNum = sub.VertexNum;
		subset.IndexBase = sub.IndexBase;
		subset.IndexNum = sub.IndexNum;
		subset.MtrlName = sub.mtrlname;
		subset.MaterialIdx = sub.materialindex;					//	�}�e���A���z��̃C���f�b�N�X
		m_subsets.emplace_back(subset);
	}

	// �}�e���A���f�[�^�쐬(�\���̂��߂�)
	for (const auto& m : materials)
	{
		MATERIAL material{};
		material.Ambient = DirectX::SimpleMath::Color(m.Ambient.r, m.Ambient.g, m.Ambient.b, m.Ambient.a);
		material.Diffuse = DirectX::SimpleMath::Color(m.Diffuse.r, m.Diffuse.g, m.Diffuse.b, m.Diffuse.a);
		material.Specular = DirectX::SimpleMath::Color(m.Specular.r, m.Specular.g, m.Specular.b, m.Specular.a);
		material.Emission = DirectX::SimpleMath::Color(m.Emission.r, m.Emission.g, m.Emission.b, m.Emission.a);
		material.Shiness = m.Shiness;

		m_materials.emplace_back(material);
	}
}